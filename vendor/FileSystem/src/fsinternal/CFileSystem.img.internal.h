/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        FileSystem/src/CFileSystem.img.internal.h
*  PURPOSE:     IMG R* Games archive management
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#ifndef _FILESYSTEM_IMG_ROCKSTAR_MANAGEMENT_INTERNAL_
#define _FILESYSTEM_IMG_ROCKSTAR_MANAGEMENT_INTERNAL_

#include <map>

// Implement the GTAIII/GTAVC XBOX IMG archive compression.
struct xboxIMGCompression : public CIMGArchiveCompressionHandler
{
                xboxIMGCompression( void );
                ~xboxIMGCompression( void );

    bool        IsStreamCompressed( CFile *stream ) const;

    bool        Decompress( CFile *input, CFile *output );
    bool        Compress( CFile *input, CFile *output );

    struct simpleWorkBuffer
    {
        inline simpleWorkBuffer( void )
        {
            this->bufferSize = 0;
            this->buffer = NULL;
        }

        inline ~simpleWorkBuffer( void )
        {
            if ( void *ptr = this->buffer )
            {
                free( ptr );

                this->buffer = NULL;
            }
        }

        inline bool MinimumSize( size_t minSize )
        {
            bool hasChanged = false;

            void *bufPtr = this->buffer;
            size_t bufSize = this->bufferSize;

            if ( bufPtr == NULL || bufSize < minSize )
            {
                void *newBufPtr = NULL;

                if ( bufPtr == NULL )
                {
                    newBufPtr = malloc( minSize );
                }
                else
                {
                    newBufPtr = realloc( bufPtr, minSize );
                }

                if ( newBufPtr != bufPtr )
                {
                    this->buffer = newBufPtr;

                    hasChanged = true;
                }

                if ( minSize != bufSize )
                {
                    this->bufferSize = minSize;

                    hasChanged = true;
                }
            }

            return hasChanged;
        }

        inline void Grow( size_t growBy )
        {
            size_t newSize = ( this->bufferSize + growBy );

            this->buffer = realloc( this->buffer, newSize );

            this->bufferSize = newSize;
        }

        inline void* GetPointer( void )
        {
            return this->buffer;
        }

        inline size_t GetSize( void ) const
        {
            return bufferSize;
        }

        inline bool IsReady( void ) const
        {
            return ( this->buffer != NULL );
        }

        inline void Release( void )
        {
            // Zero us out.
            this->buffer = NULL;
            this->bufferSize = 0;
        }

    private:
        size_t bufferSize;
        void *buffer;
    };

    simpleWorkBuffer decompressBuffer;

    size_t      compressionMaximumBlockSize;
};

// IMG extension struct.
struct imgExtension
{
    static fileSystemFactory_t::pluginOffset_t _imgPluginOffset;

    static imgExtension*        Get( CFileSystem *sys )
    {
        if ( _imgPluginOffset != fileSystemFactory_t::INVALID_PLUGIN_OFFSET )
        {
            return fileSystemFactory_t::RESOLVE_STRUCT <imgExtension> ( (CFileSystemNative*)sys, _imgPluginOffset );
        }
        return NULL;
    }

    void                        Initialize      ( CFileSystemNative *sys );
    void                        Shutdown        ( CFileSystemNative *sys );

    inline void operator = ( const imgExtension& right )
    {
        assert( 0 );
    }

    CIMGArchiveTranslatorHandle*    NewArchive  ( CFileTranslator *srcRoot, const char *srcPath, eIMGArchiveVersion version, bool isLiveMode = false );
    CIMGArchiveTranslatorHandle*    OpenArchive ( CFileTranslator *srcRoot, const char *srcPath, bool writeAccess, bool isLiveMode = false );

    CIMGArchiveTranslatorHandle*    NewArchive  ( CFileTranslator *srcRoot, const wchar_t *srcPath, eIMGArchiveVersion version, bool isLiveMode = false );
    CIMGArchiveTranslatorHandle*    OpenArchive ( CFileTranslator *srcRoot, const wchar_t *srcPath, bool writeAccess, bool isLiveMode = false );

    // Private extension methods.
    CFileTranslator*            GetTempRoot     ( void );

    // Extension members.
    // ... for managing temporary files (OS dependent).
    CRepository                 repo;
};

#include <sys/stat.h>

#include "CFileSystem.vfs.h"

// Global IMG management definitions.
#define IMG_BLOCK_SIZE          2048

#pragma warning(push)
#pragma warning(disable:4250)

class CIMGArchiveTranslator : public CSystemPathTranslator, public CFileTranslatorWideWrap, public CIMGArchiveTranslatorHandle
{
    friend class CFileSystem;
    friend struct imgExtension;
public:
                    CIMGArchiveTranslator( imgExtension& imgExt, CFile *contentFile, CFile *registryFile, eIMGArchiveVersion theVersion, bool isLiveMode );
                    ~CIMGArchiveTranslator( void );

    bool            CreateDir( const char *path ) override final;
    CFile*          Open( const char *path, const char *mode, eFileOpenFlags flags ) override final;
    bool            Exists( const char *path ) const override final;
    bool            Delete( const char *path ) override final;
    bool            Copy( const char *src, const char *dst ) override final;
    bool            Rename( const char *src, const char *dst ) override final;
    size_t          Size( const char *path ) const override final;
    bool            Stat( const char *path, struct stat *stats ) const override final;

protected:
    bool            OnConfirmDirectoryChange( const dirTree& tree ) override final;

public:
    void            ScanDirectory( const char *directory, const char *wildcard, bool recurse,
                        pathCallback_t dirCallback,
                        pathCallback_t fileCallback,
                        void *userdata ) const override final;

    void            GetDirectories( const char *path, const char *wildcard, bool recurse, std::vector <filePath>& output ) const override final;
    void            GetFiles( const char *path, const char *wildcard, bool recurse, std::vector <filePath>& output ) const override final;

    void            Save( void ) override final;

    void            SetCompressionHandler( CIMGArchiveCompressionHandler *handler ) override final;

    eIMGArchiveVersion  GetVersion( void ) const override final     { return m_version; }
    
    // Members.
    imgExtension&   m_imgExtension;
    CFile*          m_contentFile;
    CFile*          m_registryFile;
    eIMGArchiveVersion  m_version;

    bool isLiveMode;    // if true then the archive is editable directly without saving.

    CIMGArchiveCompressionHandler*  m_compressionHandler;

protected: 
    // Allocator of space on the IMG file.
    typedef InfiniteCollisionlessBlockAllocator <size_t> fileAddrAlloc_t;

    struct fileMetaData
    {
        VirtualFileSystem::fileInterface *fileNode;

        inline void SetInterface( VirtualFileSystem::fileInterface *intf )
        {
            fileNode = intf;
        }

        inline void Reset( void )
        {
            return;
        }

        inline bool IsLocked( void ) const
        {
            return ( this->lockCount != 0 );
        }

        inline fileMetaData( void )
        {
            this->blockOffset = 0;
            this->resourceSize = 0;
            this->resourceName[0] = 0;
            this->isAllocated = false;
            this->isExtracted = false;
            this->hasCompressed = false;
            this->lockCount = 0;
        }

        inline ~fileMetaData( void )
        {
            assert( this->lockCount == 0 );
            assert( this->isAllocated == false );   // make sure we are removed from the allocation table.
        }   

        inline void AddLock( void )
        {
            this->lockCount++;
        }

        inline void RemoveLock( void )
        {
            this->lockCount--;
        }

        size_t blockOffset;
        size_t resourceSize;
        char resourceName[24 + 1];
        bool isAllocated;
        fileAddrAlloc_t::block_t allocBlock;

        typedef sliceOfData <size_t> imgBlockSlice_t;

        inline imgBlockSlice_t GetDataSlice( void ) const
        {
            return imgBlockSlice_t( this->blockOffset, this->resourceSize );
        }

        bool isExtracted;
        bool hasCompressed;     // temporary parameter during archive building.

        unsigned long lockCount;

        CIMGArchiveTranslator *translator;

        // Virtual methods.
        inline fsOffsetNumber_t GetSize( void ) const
        {
            fsOffsetNumber_t resourceSize = 0;

            if ( this->isExtracted )
            {
                const filePath& ourPath = this->fileNode->GetRelativePath();

                CFileTranslator *fileRoot = translator->GetUnpackRoot();

                if ( fileRoot )
                {
                    resourceSize = fileRoot->Size( ourPath );
                }
            }
            else
            {
                resourceSize = ( (fsOffsetNumber_t)this->resourceSize * IMG_BLOCK_SIZE );
            }

            return resourceSize;
        }

        bool OnFileCopy( const dirTree& tree, const filePath& newName ) const;
        bool OnFileRename( const dirTree& tree, const filePath& newName );
        void OnFileDelete( void );

        inline void GetANSITimes( time_t& mtime, time_t& atime, time_t& ctime ) const
        {
            // There is no time info saved that is related to IMG archive entries.
            return;
        }

        inline void GetDeviceIdentifier( dev_t& deviceIdx ) const
        {
            // Return some funny device id.
            deviceIdx = 0x41;
        }

        inline void CopyAttributesTo( fileMetaData& dstEntry ) const
        {
            if ( this->translator->isLiveMode )
            {
                // TODO.
            }
            else
            {
                dstEntry.isExtracted = this->isExtracted;

                if ( this->isExtracted )
                {
                    // There are no attributes to copy, for now.
                }
                else
                {
                    // The file is located inside of the container.
                    // We create a "hard link" to it.
                    dstEntry.blockOffset = this->blockOffset;
                    dstEntry.resourceSize = this->resourceSize;
                    dstEntry.isAllocated = this->isAllocated;

                    // NOTE: this is JUST an optimization.
                    assert( this->translator->isLiveMode == false );
                }
            }
        }
    };

    struct directoryMetaData
    {
        VirtualFileSystem::directoryInterface *dirNode;

        inline void SetInterface( VirtualFileSystem::directoryInterface *intf )
        {
            dirNode = intf;
        }

        inline void Reset( void )
        {
            return;
        }
    };

    // Virtual filesystem implementation.
    typedef CVirtualFileSystem <CIMGArchiveTranslator, directoryMetaData, fileMetaData> vfs_t;

    vfs_t m_virtualFS;

    typedef vfs_t::fsActiveEntry fsActiveEntry;
    typedef vfs_t::file file;
    typedef vfs_t::directory directory;

    typedef vfs_t::fileList fileList;

public:
    // Node meta-data initializators.
    inline void InitializeFileMeta( fileMetaData& meta )
    {
        meta.translator = this;
    }

    inline void ShutdownFileMeta( fileMetaData& meta )
    {
        meta.translator = NULL;
    }

    inline void InitializeDirectoryMeta( directoryMetaData& meta )
    {

    }

    inline void ShutdownDirectoryMeta( directoryMetaData& meta )
    {

    }

    // Special VFS functions.
    CFile* OpenNativeFileStream( file *fsObject, eFileMode openMode, unsigned int access );

    // Extraction helper for data still inside the IMG archive.
    bool RequiresExtraction( CFile *stream );
    bool ExtractStream( CFile *input, CFile *output, file *theFile );

protected:
    // Public stream base-class for IMG archive streams.
    struct streamBase : public CFile
    {
        inline streamBase( CIMGArchiveTranslator *translator, file *theFile, filePath thePath, unsigned int accessMode )
        {
            this->m_translator = translator;
            this->m_info = theFile;
            this->m_path = thePath;
            this->m_accessMode = accessMode;

            // Increase the lock count.
            theFile->metaData.AddLock();
        }

        inline ~streamBase( void )
        {
            // Decrease the lock count.
            m_info->metaData.RemoveLock();
        }

        inline const filePath& GetPath( void ) const
        {
            return m_path;
        }

        bool IsReadable( void ) const
        {
            return ( m_accessMode & FILE_ACCESS_READ ) != 0;
        }

        bool IsWriteable( void ) const
        {
            return ( m_accessMode & FILE_ACCESS_WRITE ) != 0;
        }

    protected:
        CIMGArchiveTranslator *m_translator;
        file *m_info;
        filePath m_path;
        unsigned int m_accessMode;
    };

    // Slice of file stream.
    typedef sliceOfData <fsOffsetNumber_t> fileStreamSlice_t;

    // File stream inside of the IMG archive (read-only).
    struct dataSectorStream : public streamBase
    {
        inline dataSectorStream( CIMGArchiveTranslator *translator, file *theFile, filePath thePath, unsigned int accessMode );
        inline ~dataSectorStream( void );

        inline fsOffsetNumber_t _getoffset( void ) const;
        inline fsOffsetNumber_t _getsize( void ) const;
        inline void TargetSourceFile( void );

        size_t Read( void *buffer, size_t sElement, size_t iNumElements ) override;
        size_t Write( const void *buffer, size_t sElement, size_t iNumElements ) override;

        int Seek( long iOffset, int iType ) override;
        int SeekNative( fsOffsetNumber_t iOffset, int iType ) override;

        long Tell( void ) const override;
        fsOffsetNumber_t TellNative( void ) const override;
        bool IsEOF( void ) const override;

        bool Stat( struct stat *stats ) const override;
        void PushStat( const struct stat *stats ) override;

        void SetSeekEnd( void ) override;

        size_t GetSize( void ) const override;
        fsOffsetNumber_t GetSizeNative( void ) const override;

        void Flush( void ) override;

        fsOffsetNumber_t m_currentSeek;
    };

    // Root of this translator, so it can dump files for rebuilding.
    CFileTranslator *m_fileRoot;
    CFileTranslator *m_unpackRoot;
    CFileTranslator *m_compressRoot;

    CFileTranslator*    GetFileRoot( void );
    CFileTranslator*    GetUnpackRoot( void );
    CFileTranslator*    GetCompressRoot( void );

    // File stream using cached on-disk versions of the files.
    struct dataCachedStream : public streamBase
    {
        inline dataCachedStream( CIMGArchiveTranslator *translator, file *theFile, filePath thePath, unsigned int accessMode, CFile *rawStream );
        inline ~dataCachedStream( void );

        size_t Read( void *buffer, size_t sElement, size_t iNumElements ) override;
        size_t Write( const void *buffer, size_t sElement, size_t iNumElements ) override;

        int Seek( long iOffset, int iType ) override;
        int SeekNative( fsOffsetNumber_t iOffset, int iType ) override;

        long Tell( void ) const override;
        fsOffsetNumber_t TellNative( void ) const override;
        bool IsEOF( void ) const override;

        bool Stat( struct stat *stats ) const override;
        void PushStat( const struct stat *stats ) override;

        void SetSeekEnd( void ) override;

        size_t GetSize( void ) const override;
        fsOffsetNumber_t GetSizeNative( void ) const override;

        void Flush( void ) override;

        CFile *m_rawStream;
    };

    // Methods for heavy-lifting of file entries.
    bool ReallocateFileEntry( file *fileEntry, fsOffsetNumber_t fileSize );

    // Files sorted in allocation-order.
    fileAddrAlloc_t fileAddressAlloc;   // used for write-mode block alignment.

    // Private members.
    struct headerGenPresence
    {
        size_t numOfFiles;
    };

    void            GenerateFileHeaderStructure( directory& baseDir, headerGenPresence& genOut );

    struct archiveGenPresence
    {
        size_t currentBlockCount;
    };

    void            GenerateArchiveStructure( directory& baseDir, archiveGenPresence& genOut );
    void            WriteFileHeaders( CFile *targetStream, directory& baseDir );
    void            WriteFiles( CFile *targetStream, directory& baseDir );

public:
    bool            ReadArchive();
};

#pragma warning(pop)

#endif //_FILESYSTEM_IMG_ROCKSTAR_MANAGEMENT_INTERNAL_