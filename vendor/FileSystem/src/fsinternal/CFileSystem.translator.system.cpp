/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        FileSystem/src/fsinternal/CFileSystem.translator.system.cpp
*  PURPOSE:     FileSystem translator that represents directory links
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/
#include <StdInc.h>
#include <sys/stat.h>

#ifdef __linux__
#include <utime.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#endif //__linux__

// Include the internal definitions.
#include "CFileSystem.internal.h"

// Sub modules.
#include "CFileSystem.platform.h"
#include "CFileSystem.translator.system.h"
#include "CFileSystem.stream.raw.h"
#include "CFileSystem.platformutils.hxx"

// Include common fs utilitites.
#include "../CFileSystem.utils.hxx"

// Include native utilities for platforms.
#include "CFileSystem.internal.nativeimpl.hxx"

/*===================================================
    File_IsDirectoryAbsolute

    Arguments:
        pPath - Absolute path pointing to an OS filesystem entry.
    Purpose:
        Checks the given path and returns true if it points
        to a directory, false if a file or no entry was found
        at the path.
===================================================*/
bool File_IsDirectoryAbsolute( const char *pPath )
{
#ifdef _WIN32
    DWORD dwAttributes = GetFileAttributesA(pPath);

    if (dwAttributes == INVALID_FILE_ATTRIBUTES)
        return false;

    return (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
#elif defined(__linux__)
    struct stat dirInfo;

    if ( stat( pPath, &dirInfo ) != 0 )
        return false;

    return ( dirInfo.st_mode & S_IFDIR ) != 0;
#else
    return false;
#endif
}

bool File_IsDirectoryAbsoluteW( const wchar_t *pPath )
{
#ifdef _WIN32
    DWORD dwAttributes = GetFileAttributesW(pPath);

    if (dwAttributes == INVALID_FILE_ATTRIBUTES)
        return false;

    return (dwAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
#elif defined(__linux__)
    struct stat dirInfo;

    if ( stat( pPath, &dirInfo ) != 0 )
        return false;

    return ( dirInfo.st_mode & S_IFDIR ) != 0;
#else
    return false;
#endif
}

/*=======================================
    CSystemFileTranslator

    Default file translator
=======================================*/

CSystemFileTranslator::~CSystemFileTranslator( void )
{
#ifdef _WIN32
    if ( m_curDirHandle )
        CloseHandle( m_curDirHandle );

    CloseHandle( m_rootHandle );
#elif defined(__linux__)
    if ( m_curDirHandle )
        closedir( m_curDirHandle );

    closedir( m_rootHandle );
#endif //OS DEPENDANT CODE
}

bool CSystemFileTranslator::_CreateDirTree( const dirTree& tree )
{
    dirTree::const_iterator iter;
    filePath path = m_root;

    for ( iter = tree.begin(); iter != tree.end(); ++iter )
    {
        path += *iter;
        path += '/';

        bool success = _File_CreateDirectory( path );

        if ( !success )
            return false;
    }

    return true;
}

template <typename charType>
bool CSystemFileTranslator::GenCreateDir( const charType *path )
{
    dirTree tree;
    bool file;

    if ( !GetRelativePathTreeFromRoot( path, tree, file ) )
        return false;

    if ( file )
        tree.pop_back();

    return _CreateDirTree( tree );
}

bool CSystemFileTranslator::CreateDir( const char *path )       { return GenCreateDir( path ); }
bool CSystemFileTranslator::CreateDir( const wchar_t *path )    { return GenCreateDir( path ); }

template <typename charType>
CFile* CSystemFileTranslator::GenOpen( const charType *path, const charType *mode, eFileOpenFlags flags )
{
    CFile *outFile = NULL;

    dirTree tree;
    filePath output = m_root;
    CRawFile *pFile;
    unsigned int frm_dwAccess = 0;
    eFileMode frm_dwCreate = eFileMode::UNKNOWN;
    bool file;

    if ( !GetRelativePathTreeFromRoot( path, tree, file ) )
        return NULL;

    // We can only open files!
    if ( !file )
        return NULL;

    _File_OutputPathTree( tree, true, output );

    if ( !_File_ParseMode( *this, path, mode, frm_dwAccess, frm_dwCreate ) )
        return NULL;

    // Creation requires the dir tree!
    if ( frm_dwCreate == eFileMode::CREATE )
    {
        tree.pop_back();

        bool dirSuccess = _CreateDirTree( tree );

        if ( !dirSuccess )
            return NULL;
    }

#ifdef _WIN32
    // Translate to native OS access and create mode.
    DWORD win32AccessMode = 0;

    if ( frm_dwAccess == FILE_ACCESS_READ )
    {
        win32AccessMode |= GENERIC_READ;
    }
    if ( frm_dwAccess == FILE_ACCESS_WRITE )
    {
        win32AccessMode |= GENERIC_WRITE;
    }

    DWORD win32CreateMode = 0;

    if ( frm_dwCreate == eFileMode::OPEN )
    {
        win32CreateMode = OPEN_EXISTING;
    }
    else if ( frm_dwCreate == eFileMode::CREATE )
    {
        win32CreateMode = CREATE_ALWAYS;
    }

    DWORD flagAttr = 0;

    if ( flags & FILE_FLAG_TEMPORARY )
        flagAttr |= FILE_FLAG_DELETE_ON_CLOSE | FILE_ATTRIBUTE_TEMPORARY;

    if ( flags & FILE_FLAG_UNBUFFERED )
        flagAttr |= FILE_FLAG_NO_BUFFERING | FILE_FLAG_WRITE_THROUGH;

    HANDLE sysHandle = INVALID_HANDLE_VALUE;

    DWORD win32ShareMode = FILE_SHARE_READ;

    if ( ( flags & FILE_FLAG_WRITESHARE ) != 0 )
    {
        win32ShareMode |= FILE_SHARE_WRITE;
    }

    if ( const char *sysPath = output.c_str() )
    {
        sysHandle = CreateFileA( sysPath, win32AccessMode, win32ShareMode, NULL, win32CreateMode, flagAttr, NULL );
    }
    else if ( const wchar_t *sysPath = output.w_str() )
    {
        sysHandle = CreateFileW( sysPath, win32AccessMode, win32ShareMode, NULL, win32CreateMode, flagAttr, NULL );
    }

    if ( sysHandle == INVALID_HANDLE_VALUE )
        return NULL;

    pFile = new CRawFile( output );
    pFile->m_file = sysHandle;
#elif defined(__linux__)
    const char *openMode;

    // TODO: support flags parameter.

    if ( dwCreate == FILE_MODE_CREATE )
    {
        if ( dwAccess & FILE_ACCESS_READ )
            openMode = "w+";
        else
            openMode = "w";
    }
    else if ( dwCreate == FILE_MODE_OPEN )
    {
        if ( dwAccess & FILE_ACCESS_WRITE )
            openMode = "r+";
        else
            openMode = "r";
    }
    else
        return NULL;

    FILE *filePtr = fopen( output.c_str(), openMode );

    if ( !filePtr )
        return NULL;

    pFile = new CRawFile( output );
    pFile->m_file = filePtr;
#else
    return NULL;
#endif //OS DEPENDANT CODE

    // Write shared file properties.
    pFile->m_access = frm_dwAccess;

    if ( *mode == 'a' )
        pFile->Seek( 0, SEEK_END );

    outFile = pFile;

    // TODO: improve the buffering implementation, so it does not fail in write-only mode.

    // If required, wrap the file into a buffered stream.
    if ( false ) // todo: add a property that decides this?
    {
        outFile = new CBufferedStreamWrap( pFile, true );
    }

    return outFile;
}

CFile* CSystemFileTranslator::Open( const char *path, const char *mode, eFileOpenFlags flags )            { return GenOpen( path, mode, flags ); }
CFile* CSystemFileTranslator::Open( const wchar_t *path, const wchar_t *mode, eFileOpenFlags flags )      { return GenOpen( path, mode, flags ); }

inline bool _File_Stat( const filePath& path, struct stat& statOut )
{
    int iStat = -1;

    if ( const char *sysPath = path.c_str() )
    {
        iStat = stat( sysPath, &statOut );
    }
    else if ( const wchar_t *sysPath = path.w_str() )
    {
        struct _stat tmp;

        iStat = _wstat( sysPath, &tmp );

        if ( iStat == 0 )
        {
            // Backwards convert.
            statOut.st_dev = tmp.st_dev;
            statOut.st_ino = tmp.st_ino;
            statOut.st_mode = tmp.st_mode;
            statOut.st_nlink = tmp.st_nlink;
            statOut.st_uid = tmp.st_uid;
            statOut.st_gid = tmp.st_gid;
            statOut.st_rdev = tmp.st_rdev;
            statOut.st_size = tmp.st_size;
            statOut.st_atime = (decltype( statOut.st_atime ))tmp.st_atime;
            statOut.st_mtime = (decltype( statOut.st_mtime ))tmp.st_mtime;
            statOut.st_ctime = (decltype( statOut.st_ctime ))tmp.st_ctime;
        }
    }

    return ( iStat == 0 );
}

template <typename charType>
bool CSystemFileTranslator::GenExists( const charType *path ) const
{
    filePath output;

    if ( !GetFullPath( path, true, output ) )
        return false;

#if 0
    // The C API cannot cope with trailing slashes
    size_t outSize = output.size();

    if ( outSize && output.compareCharAt( '/', --outSize ) )
    {
        output.resize( outSize );
    }
#endif
    
    struct stat tmp;

    return _File_Stat( output, tmp );
}

bool CSystemFileTranslator::Exists( const char *path ) const        { return GenExists( path ); }
bool CSystemFileTranslator::Exists( const wchar_t *path ) const     { return GenExists( path ); }

inline bool _deleteFile( const char *path )
{
#ifdef _WIN32
    return DeleteFileA( path ) != FALSE;
#elif defined(__linux__)
    return unlink( path ) == 0;
#else
    return false;
#endif //OS DEPENDANT CODE
}

inline bool _deleteFile( const wchar_t *path )
{
#ifdef _WIN32
    return DeleteFileW( path ) != FALSE;
#elif defined(__linux__)
    return unlink( path ) == 0;
#else
    return false;
#endif //OS DEPENDANT CODE
}

inline bool _deleteFileCallback_gen( const filePath& path )
{
    bool deletionSuccess = false;

    if ( const char *sysPath = path.c_str() )
    {
        deletionSuccess = _deleteFile( sysPath );
    }
    else if ( const wchar_t *sysPath = path.w_str() )
    {
        deletionSuccess = _deleteFile( sysPath );
    }

    return deletionSuccess;
}

static void _deleteFileCallback( const filePath& path, void *ud )
{
    bool deletionSuccess = _deleteFileCallback_gen( path );

    if ( !deletionSuccess )
    {
        __debugbreak();
    }
}

inline bool _deleteDir( const char *path )
{
#ifdef _WIN32
    return RemoveDirectoryA( path ) != FALSE;
#elif defined(__linux__)
    return rmdir( path ) == 0;
#else
    return false;
#endif //OS DEPENDANT CODE
}

inline bool _deleteDir( const wchar_t *path )
{
#ifdef _WIN32
    return RemoveDirectoryW( path ) != FALSE;
#elif defined(__linux__)
    return rmdir( path ) == 0;
#else
    return false;
#endif //OS DEPENDANT CODE
}

static void _deleteDirCallback( const filePath& path, void *ud );

inline bool _deleteDirCallback_gen( const filePath& path, CSystemFileTranslator *sysRoot )
{
    bool deletionSuccess = false;

    if ( const char *sysPath = path.c_str() )
    {
        sysRoot->ScanDirectory( sysPath, "*", false, _deleteDirCallback, _deleteFileCallback, sysRoot );

        deletionSuccess = _deleteDir( sysPath );
    }
    else if ( const wchar_t *sysPath = path.w_str() )
    {
        sysRoot->ScanDirectory( sysPath, L"*", false, _deleteDirCallback, _deleteFileCallback, sysRoot );

        deletionSuccess = _deleteDir( sysPath );
    }

    return deletionSuccess;
}

static void _deleteDirCallback( const filePath& path, void *ud )
{
    // Delete all subdirectories too.
    CSystemFileTranslator *sysRoot = (CSystemFileTranslator*)ud;

    bool deletionSuccess = _deleteDirCallback_gen( path, sysRoot );

    if ( !deletionSuccess )
    {
        __debugbreak();
    }
}

template <typename charType>
bool CSystemFileTranslator::GenDelete( const charType *path )
{
    filePath output;

    if ( !GetFullPath( path, true, output ) )
        return false;

    if ( FileSystem::IsPathDirectory( output ) )
    {
        bool isDirectory = false;

        if ( const char *sysPath = output.c_str() )
        {
            isDirectory = File_IsDirectoryAbsolute( sysPath );
        }
        else if ( const wchar_t *sysPath = output.w_str() )
        {
            isDirectory = File_IsDirectoryAbsoluteW( sysPath );
        }

        if ( !isDirectory )
            return false;

        // Remove all files and directories inside
        return _deleteDirCallback_gen( output, this );
    }

    return _deleteFileCallback_gen( output );
}

bool CSystemFileTranslator::Delete( const char *path )      { return GenDelete( path ); }
bool CSystemFileTranslator::Delete( const wchar_t *path )   { return GenDelete( path ); }

inline bool _File_Copy( const char *src, const char *dst )
{
#ifdef _WIN32
    return CopyFileA( src, dst, false ) != FALSE;
#elif defined(__linux__)
    int iReadFile = open( src, O_RDONLY, 0 );

    if ( iReadFile == -1 )
        return false;

    int iWriteFile = open( dst, O_CREAT | O_WRONLY | O_ASYNC, FILE_ACCESS_FLAG );

    if ( iWriteFile == -1 )
        return false;

    struct stat read_info;
    if ( fstat( iReadFile, &read_info ) != 0 )
    {
        close( iReadFile );
        close( iWriteFile );
        return false;
    }

    sendfile( iWriteFile, iReadFile, NULL, read_info.st_size );

    close( iReadFile );
    close( iWriteFile );
    return true;
#else
    return false;
#endif //OS DEPENDANT CODE
}

inline bool _File_Copy( const wchar_t *src, const wchar_t *dst )
{
#ifdef _WIN32
    return CopyFileW( src, dst, false ) != FALSE;
#elif defined(__linux__)
    int iReadFile = open( src, O_RDONLY, 0 );

    if ( iReadFile == -1 )
        return false;

    int iWriteFile = open( dst, O_CREAT | O_WRONLY | O_ASYNC, FILE_ACCESS_FLAG );

    if ( iWriteFile == -1 )
        return false;

    struct stat read_info;
    if ( fstat( iReadFile, &read_info ) != 0 )
    {
        close( iReadFile );
        close( iWriteFile );
        return false;
    }

    sendfile( iWriteFile, iReadFile, NULL, read_info.st_size );

    close( iReadFile );
    close( iWriteFile );
    return true;
#else
    return false;
#endif //OS DEPENDANT CODE
}

template <typename charType>
bool CSystemFileTranslator::GenCopy( const charType *src, const charType *dst )
{
    filePath source;
    filePath target;
    dirTree dstTree;
    bool file;

    if ( !GetFullPath( src, true, source ) || !GetRelativePathTreeFromRoot( dst, dstTree, file ) || !file )
        return false;

    // We always start from root
    target = m_root;

    _File_OutputPathTree( dstTree, true, target );

    // Make sure dir exists
    dstTree.pop_back();
    bool dirSuccess = _CreateDirTree( dstTree );

    if ( !dirSuccess )
        return false;

    // Copy data using quick kernel calls.
    if ( const wchar_t *sysSrcPath = source.w_str() )
    {
        target.convert_unicode();

        return _File_Copy( sysSrcPath, target.w_str() );
    }
    else if ( const wchar_t *sysDstPath = target.w_str() )
    {
        source.convert_unicode();

        return _File_Copy( source.w_str(), sysDstPath );
    }

    return _File_Copy( source.c_str(), target.c_str() );
}

bool CSystemFileTranslator::Copy( const char *src, const char *dst )        { return GenCopy( src, dst ); }
bool CSystemFileTranslator::Copy( const wchar_t *src, const wchar_t *dst )  { return GenCopy( src, dst ); }

inline bool _File_Rename( const char *src, const char *dst )
{
#ifdef _WIN32
    return MoveFileA( src, dst ) != FALSE;
#elif defined(__linux__)
    return rename( src, dst ) == 0;
#else
    return false;
#endif //OS DEPENDANT CODE
}

inline bool _File_Rename( const wchar_t *src, const wchar_t *dst )
{
#ifdef _WIN32
    return MoveFileW( src, dst ) != FALSE;
#elif defined(__linux__)
    return rename( src, dst ) == 0;
#else
    return false;
#endif //OS DEPENDANT CODE
}

template <typename charType>
bool CSystemFileTranslator::GenRename( const charType *src, const charType *dst )
{
    filePath source;
    filePath target;
    dirTree dstTree;
    bool file;

    if ( !GetFullPath( src, true, source ) || !GetRelativePathTreeFromRoot( dst, dstTree, file ) || !file )
        return false;
    
    // We always start from root
    target = m_root;

    _File_OutputPathTree( dstTree, true, target );

    // Make sure dir exists
    dstTree.pop_back();
    bool dirSuccess = _CreateDirTree( dstTree );

    if ( !dirSuccess )
        return false;

    if ( const wchar_t *sysSrcPath = source.w_str() )
    {
        target.convert_unicode();

        return _File_Rename( sysSrcPath, target.w_str() );
    }
    else if ( const wchar_t *sysDstPath = target.w_str() )
    {
        source.convert_unicode();

        return _File_Rename( source.w_str(), sysDstPath );
    }

    return _File_Rename( source.c_str(), target.c_str() );
}
    
bool CSystemFileTranslator::Rename( const char *src, const char *dst )          { return GenRename( src, dst ); }
bool CSystemFileTranslator::Rename( const wchar_t *src, const wchar_t *dst )    { return GenRename( src, dst ); }

template <typename charType>
bool CSystemFileTranslator::GenStat( const charType *path, struct stat *stats ) const
{
    filePath output;

    if ( !GetFullPath( path, true, output ) )
        return false;

    return _File_Stat( output, *stats );
}

bool CSystemFileTranslator::Stat( const char *path, struct stat *stats ) const      { return GenStat( path, stats ); }
bool CSystemFileTranslator::Stat( const wchar_t *path, struct stat *stats ) const   { return GenStat( path, stats ); }

template <typename charType>
size_t CSystemFileTranslator::GenSize( const charType *path ) const
{
    struct stat fstats;

    if ( !Stat( path, &fstats ) )
        return 0;

    return fstats.st_size;
}

size_t CSystemFileTranslator::Size( const char *path ) const      { return GenSize( path ); }
size_t CSystemFileTranslator::Size( const wchar_t *path ) const   { return GenSize( path ); }

// Handle absolute paths.
template <typename charType, typename procType>
AINLINE bool CSystemFileTranslator::GenProcessFullPath( const charType *path, dirTree& tree, bool& file, bool& success, procType proc ) const
{
#ifdef _WIN32
    filePath uncPart;
    const charType *uncEnd;
#endif

    if ( _File_IsAbsolutePath( path ) )
    {
#ifdef _WIN32
        if ( this->m_pathType != ROOTPATH_DISK || m_root.compareCharAt( path[0], 0 ) == false )
        {
            success = false;   // drive mismatch
        }
        else
        {
            success = proc.ParseAbsolute( path + 3, tree, file );
        }
#else
        success = proc.parseAbsolute( path + 1, tree, file );
#endif //OS DEPENDANT CODE

        return true;
    }
#ifdef _WIN32
    else if ( _File_IsUNCPath( path, uncEnd, uncPart ) )
    {
        // Make sure UNC descriptors match.
        if ( this->m_pathType != ROOTPATH_UNC || uncPart.equals( this->m_unc, false ) == false )
        {
            success = false;    // wrong UNC.
        }
        else
        {
            success = proc.ParseAbsolute( uncEnd, tree, file );
        }
        
        return true;
    }
#endif

    return false;
}

template <typename charType>
bool CSystemFileTranslator::GenOnGetRelativePathTreeFromRoot( const charType *path, dirTree& tree, bool& file, bool& success ) const
{
    struct relativePathTreeFromRootProc
    {
        AINLINE relativePathTreeFromRootProc( const CSystemFileTranslator *trans )
        {
            this->trans = trans;
        }

        AINLINE bool ParseAbsolute( const charType *path, dirTree& tree, bool& file ) const
        {
            return _File_ParseRelativeTree( path, trans->m_rootTree, tree, file );
        }

        const CSystemFileTranslator *trans;
    };

    return GenProcessFullPath( path, tree, file, success, relativePathTreeFromRootProc( this ) );
}

bool CSystemFileTranslator::OnGetRelativePathTreeFromRoot( const char *path, dirTree& tree, bool& file, bool& success ) const       { return GenOnGetRelativePathTreeFromRoot( path, tree, file, success ); }
bool CSystemFileTranslator::OnGetRelativePathTreeFromRoot( const wchar_t *path, dirTree& tree, bool& file, bool& success ) const    { return GenOnGetRelativePathTreeFromRoot( path, tree, file, success ); }

template <typename charType>
bool CSystemFileTranslator::GenOnGetRelativePathTree( const charType *path, dirTree& tree, bool& file, bool& success ) const
{
    struct relativePathTreeProc
    {
        AINLINE relativePathTreeProc( const CSystemFileTranslator *trans )
        {
            this->trans = trans;
        }

        AINLINE bool ParseAbsolute( const charType *path, dirTree& tree, bool& file ) const
        {
            return _File_ParseRelativeTreeDeriviate( path, trans->m_rootTree, trans->m_curDirTree, tree, file );
        }

        const CSystemFileTranslator *trans;
    };

    return GenProcessFullPath( path, tree, file, success, relativePathTreeProc( this ) );
}

bool CSystemFileTranslator::OnGetRelativePathTree( const char *path, dirTree& tree, bool& file, bool& success ) const       { return GenOnGetRelativePathTree( path, tree, file, success ); }
bool CSystemFileTranslator::OnGetRelativePathTree( const wchar_t *path, dirTree& tree, bool& file, bool& success ) const    { return GenOnGetRelativePathTree( path, tree, file, success ); }

template <typename charType>
bool CSystemFileTranslator::GenOnGetFullPathTree( const charType *path, dirTree& tree, bool& file, bool& success ) const
{
    struct fullPathTreeProc
    {
        AINLINE fullPathTreeProc( const CSystemFileTranslator *trans )
        {
            this->trans = trans;
        }

        AINLINE bool ParseAbsolute( const charType *path, dirTree& tree, bool& file ) const
        {
            tree = trans->m_rootTree;

            return _File_ParseRelativeTree( path, trans->m_rootTree, tree, file );
        }

        const CSystemFileTranslator *trans;
    };

    return GenProcessFullPath( path, tree, file, success, fullPathTreeProc( this ) );
}

bool CSystemFileTranslator::OnGetFullPathTree( const char *path, dirTree& tree, bool& file, bool& success ) const       { return GenOnGetFullPathTree( path, tree, file, success ); }
bool CSystemFileTranslator::OnGetFullPathTree( const wchar_t *path, dirTree& tree, bool& file, bool& success ) const    { return GenOnGetFullPathTree( path, tree, file, success ); }

void CSystemFileTranslator::OnGetFullPath( filePath& curAbsPath ) const
{
#ifdef _WIN32
    eRootPathType pathType = this->m_pathType;

    if ( pathType == ROOTPATH_UNC )
    {
        const filePath uncPrefix = filePath( "//" ) + m_unc + filePath( "/" );

        curAbsPath.insert( 0, uncPrefix, uncPrefix.size() );
    }
    else if ( pathType == ROOTPATH_DISK )
    {
        curAbsPath.insert( 0, m_root, 3 );
    }
    else
    {
        assert( 0 );
    }
#else
    curAbsPath.insert( 0, "/", 1 );
#endif //_WIN32
}

bool CSystemFileTranslator::OnConfirmDirectoryChange( const dirTree& tree )
{
    filePath absPath = m_root;
    _File_OutputPathTree( tree, false, absPath );

#ifdef _WIN32
    HANDLE dir = _FileWin32_OpenDirectoryHandle( absPath );

    if ( dir == INVALID_HANDLE_VALUE )
        return false;

    if ( m_curDirHandle )
        CloseHandle( m_curDirHandle );

    m_curDirHandle = dir;
#elif defined(__linux__)
    DIR *dir = opendir( absPath.c_str() );

    if ( dir == NULL )
        return false;

    if ( m_curDirHandle )
        closedir( m_curDirHandle );

    m_curDirHandle = dir;
#else
    if ( !File_IsDirectoryAbsolute( absPath.c_str() ) )
        return false;
#endif //OS DEPENDANT CODE

    return true;
}

template <typename charType>
inline const charType* GetAnyWildcardSelector( void )
{
    static_assert( "invalid character type" );
}

template <>
inline const char* GetAnyWildcardSelector <char> ( void )
{
    return "*";
}

template <>
inline const wchar_t* GetAnyWildcardSelector <wchar_t> ( void )
{
    return L"*";
}

template <typename charType>
inline void copystr( charType *dst, const charType *src, size_t max )
{
    static_assert( false, "invalid string type for copy" );
}

template <>
inline void copystr( char *dst, const char *src, size_t max )
{
    strncpy( dst, src, max );
}

template <>
inline void copystr( wchar_t *dst, const wchar_t *src, size_t max )
{
    wcsncpy( dst, src, max );
}

template <typename charType>
struct FINDDATA_ENV
{
    //
};

template <>
struct FINDDATA_ENV <char>
{
    typedef WIN32_FIND_DATAA cont_type;

    inline static HANDLE FindFirst( const filePath& path, cont_type *out )
    {
        if ( const char *sysPath = path.c_str() )
        {
            return FindFirstFileA( sysPath, out );
        }

        std::string ansiPath = path.convert_ansi();

        return FindFirstFileA( ansiPath.c_str(), out );
    }

    inline static BOOL FindNext( HANDLE hfind, cont_type *out )
    {
        return FindNextFileA( hfind, out );
    }
};

template <>
struct FINDDATA_ENV <wchar_t>
{
    typedef WIN32_FIND_DATAW cont_type;

    inline static HANDLE FindFirst( const filePath& path, cont_type *out )
    {
        if ( const wchar_t *sysPath = path.w_str() )
        {
            return FindFirstFileW( sysPath, out );
        }

        std::wstring widePath = path.convert_unicode();

        return FindFirstFileW( widePath.c_str(), out );
    }

    inline static BOOL FindNext( HANDLE hfind, cont_type *out )
    {
        return FindNextFileW( hfind, out );
    }
};

template <typename find_prov, typename pattern_env_type>
AINLINE void Win32ScanDirectoryNative(
    filePath absDirPath,
    const pattern_env_type& patternEnv, typename pattern_env_type::filePattern_t *pattern,
    bool recurse,
    pathCallback_t dirCallback, pathCallback_t fileCallback,
    void *userdata
)
{
    // Create the query string to send to Windows.
    filePath query = absDirPath;
    query += GetAnyWildcardSelector <char> ();

    find_prov::cont_type    finddata;

    //first search for files only
    if ( fileCallback )
    {
        HANDLE handle = find_prov::FindFirst( query, &finddata );

        if ( handle != INVALID_HANDLE_VALUE )
        {
            try
            {
                do
                {
                    if ( finddata.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_TEMPORARY | FILE_ATTRIBUTE_DIRECTORY) )
                        continue;

                    // Match the pattern ourselves.
                    if ( patternEnv.MatchPattern( finddata.cFileName, pattern ) )
                    {
                        filePath filename = absDirPath;
                        filename += finddata.cFileName;

                        fileCallback( filename, userdata );
                    }
                } while ( find_prov::FindNext(handle, &finddata) );
            }
            catch( ... )
            {
                FindClose( handle );

                throw;
            }

            FindClose( handle );
        }
    }

    if ( dirCallback || recurse )
    {
        //next search for subdirectories only
        HANDLE handle = find_prov::FindFirst( query, &finddata );

        if ( handle != INVALID_HANDLE_VALUE )
        {
            try
            {
                do
                {
                    if ( finddata.dwFileAttributes & (FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_SYSTEM | FILE_ATTRIBUTE_TEMPORARY) )
                        continue;

                    if ( !(finddata.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) )
                        continue;

                    // Optimization :)
                    if ( _File_IgnoreDirectoryScanEntry( finddata.cFileName ) )
                        continue;

                    filePath target = absDirPath;
                    target += finddata.cFileName;
                    target += '/';

                    if ( dirCallback )
                    {
                        _File_OnDirectoryFound( patternEnv, pattern, finddata.cFileName, target, dirCallback, userdata );
                    }

                    if ( recurse )
                    {
                        Win32ScanDirectoryNative <find_prov> (
                            std::move( target ),
                            patternEnv, pattern,
                            true, dirCallback, fileCallback,
                            userdata
                        );
                    }

                } while ( find_prov::FindNext(handle, &finddata) );
            }
            catch( ... )
            {
                FindClose( handle );

                throw;
            }

            FindClose( handle );
        }
    }
}

template <typename charType>
void CSystemFileTranslator::GenScanDirectory( const charType *directory, const charType *wildcard, bool recurse,
                                              pathCallback_t dirCallback,
                                              pathCallback_t fileCallback,
                                              void *userdata ) const
{
    filePath            output;
    charType		    wcard[256];

    if ( !GetFullPath( directory, false, output ) )
        return;

    if ( !wildcard )
    {
        wcard[0] = GetAnyWildcardSelector <charType> ()[ 0 ];
        wcard[1] = 0;
    }
    else
    {
        copystr( wcard, wildcard, 255 );
        wcard[255] = 0;
    }

#ifdef _WIN32
    typedef FINDDATA_ENV <wchar_t> find_prov;

    PathPatternEnv <wchar_t> patternEnv( true );

    PathPatternEnv <wchar_t>::filePattern_t *pattern = patternEnv.CreatePattern( wildcard );

    try
    {
        Win32ScanDirectoryNative <find_prov> (
            std::move( output ),
            patternEnv, pattern,
            recurse,
            dirCallback, fileCallback,
            userdata
        );
    }
    catch( ... )
    {
        // Callbacks may throw exceptions
        patternEnv.DestroyPattern( pattern );

        throw;
    }

    patternEnv.DestroyPattern( pattern );

#elif defined(__linux__)
    DIR *findDir = opendir( output.c_str() );

    if ( !findDir )
        return;

    filePattern_t *pattern = _File_CreatePattern( wildcard );

    try
    {
        //first search for files only
        if ( fileCallback )
        {
            while ( struct dirent *entry = readdir( findDir ) )
            {
                filePath path = output;
                path += entry->d_name;

                struct stat entry_stats;

                if ( stat( path.c_str(), &entry_stats ) == 0 )
                {
                    if ( !( S_ISDIR( entry_stats.st_mode ) ) && _File_MatchPattern( entry->d_name, pattern ) )
                    {
                        fileCallback( path.c_str(), userdata );
                    }
                }
            }
        }

        rewinddir( findDir );

        if ( dirCallback || recurse )
        {
            //next search for subdirectories only
            while ( struct dirent *entry = readdir( findDir ) )
            {
                const char *name = entry->d_name;

                if ( _File_IgnoreDirectoryScanEntry( name ) )
                    continue;

                filePath path = output;
                path += name;
                path += '/';

                struct stat entry_info;

                if ( stat( path.c_str(), &entry_info ) == 0 && S_ISDIR( entry_info.st_mode ) )
                {
                    if ( dirCallback )
                    {
                        _File_OnDirectoryFound( pattern, entry->d_name, path, dirCallback, userdata );
                    }

                    // TODO: this can be optimized by reusing the pattern structure.
                    if ( recurse )
                        ScanDirectory( path.c_str(), wcard, recurse, dirCallback, fileCallback, userdata );
                }
            }
        }
    }
    catch( ... )
    {
        // Callbacks may throw exceptions
        _File_DestroyPattern( pattern );

        closedir( findDir );
        throw;
    }

    _File_DestroyPattern( pattern );

    closedir( findDir );
#endif //OS DEPENDANT CODE
}

void CSystemFileTranslator::ScanDirectory( const char *directory, const char *wildcard, bool recurse,
                                           pathCallback_t dirCallback,
                                           pathCallback_t fileCallback,
                                           void *userdata ) const
{
    return GenScanDirectory( directory, wildcard, recurse, dirCallback, fileCallback, userdata );
}

void CSystemFileTranslator::ScanDirectory( const wchar_t *directory, const wchar_t *wildcard, bool recurse,
                                           pathCallback_t dirCallback,
                                           pathCallback_t fileCallback,
                                           void *userdata ) const
{
    return GenScanDirectory( directory, wildcard, recurse, dirCallback, fileCallback, userdata );
}

static void _scanFindCallback( const filePath& path, std::vector <filePath> *output )
{
    output->push_back( path );
}

template <typename charType>
void CSystemFileTranslator::GenGetDirectories( const charType *path, const charType *wildcard, bool recurse, std::vector <filePath>& output ) const
{
    ScanDirectory( path, wildcard, recurse, (pathCallback_t)_scanFindCallback, NULL, &output );
}

void CSystemFileTranslator::GetDirectories( const char *path, const char *wildcard, bool recurse, std::vector <filePath>& output ) const
{
    return GenGetDirectories( path, wildcard, recurse, output );
}
void CSystemFileTranslator::GetDirectories( const wchar_t *path, const wchar_t *wildcard, bool recurse, std::vector <filePath>& output ) const
{
    return GenGetDirectories( path, wildcard, recurse, output );
}

template <typename charType>
void CSystemFileTranslator::GenGetFiles( const charType *path, const charType *wildcard, bool recurse, std::vector <filePath>& output ) const
{
    ScanDirectory( path, wildcard, recurse, NULL, (pathCallback_t)_scanFindCallback, &output );
}

void CSystemFileTranslator::GetFiles( const char *path, const char *wildcard, bool recurse, std::vector <filePath>& output ) const
{
    return GenGetFiles( path, wildcard, recurse, output );
}
void CSystemFileTranslator::GetFiles( const wchar_t *path, const wchar_t *wildcard, bool recurse, std::vector <filePath>& output ) const
{
    return GenGetFiles( path, wildcard, recurse, output );
}