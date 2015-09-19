/*****************************************************************************
*
*  PROJECT:     Multi Theft Auto v1.2
*  LICENSE:     See LICENSE in the top level directory
*  FILE:        FileSystem/src/CFileSystem.img.cpp
*  PURPOSE:     IMG R* Games archive management
*
*  Multi Theft Auto is available from http://www.multitheftauto.com/
*
*****************************************************************************/

#include <StdInc.h>

// Include internal (private) definitions.
#include "fsinternal/CFileSystem.internal.h"
#include "fsinternal/CFileSystem.img.internal.h"

extern CFileSystem *fileSystem;

#include "CFileSystem.Utils.hxx"


void imgExtension::Initialize( CFileSystemNative *sys )
{
    return;
}

void imgExtension::Shutdown( CFileSystemNative *sys )
{
    return;
}

template <typename charType>
inline const charType* GetReadWriteMode( bool isNew )
{
    static_assert( false, "invalid character type" );
}

template <>
inline const char* GetReadWriteMode <char> ( bool isNew )
{
    return ( isNew ? "wb" : "rb+" );
}

template <>
inline const wchar_t* GetReadWriteMode <wchar_t> ( bool isNew )
{
    return ( isNew ? L"wb" : L"rb+" );
}

template <typename charType>
inline CFile* OpenSeperateIMGRegistryFile( CFileTranslator *srcRoot, const charType *imgFilePath, bool isNew )
{
    CFile *registryFile = NULL;

    filePath dirOfArchive;
    filePath extention;

    filePath nameItem = FileSystem::GetFileNameItem( imgFilePath, false, &dirOfArchive, &extention );

    if ( nameItem.size() != 0 )
    {
        filePath regFilePath = dirOfArchive + nameItem + ".DIR";

        // Open a seperate registry file.
        if ( const char *sysPath = regFilePath.c_str() )
        {
            registryFile = srcRoot->Open( sysPath, GetReadWriteMode <char> ( isNew ) );
        }
        else if ( const wchar_t *sysPath = regFilePath.w_str() )
        {
            registryFile = srcRoot->Open( sysPath, GetReadWriteMode <wchar_t> ( isNew ) );
        }
    }

    return registryFile;
}

template <typename charType>
static inline CIMGArchiveTranslatorHandle* GenNewArchive( imgExtension *env, CFileTranslator *srcRoot, const charType *srcPath, eIMGArchiveVersion version )
{
    // Create an archive depending on version.
    CIMGArchiveTranslator *resultArchive = NULL;
    {
        CFile *contentFile = NULL;
        CFile *registryFile = NULL;

        if ( version == IMG_VERSION_1 )
        {
            // Just open the content file.
            contentFile = srcRoot->Open( srcPath, GetReadWriteMode <charType> ( true ) );

            // We need to create a seperate registry file.
            registryFile = OpenSeperateIMGRegistryFile( srcRoot, srcPath, true );
        }
        else if ( version == IMG_VERSION_2 )
        {
            // Just create a content file.
            contentFile = srcRoot->Open( srcPath, GetReadWriteMode <charType> ( true ) );

            registryFile = contentFile;
        }

        if ( contentFile && registryFile )
        {
            resultArchive = new CIMGArchiveTranslator( *env, contentFile, registryFile, version );
        }

        if ( !resultArchive )
        {
            if ( contentFile )
            {
                delete contentFile;
            }

            if ( registryFile && registryFile != contentFile )
            {
                delete registryFile;
            }
        }
    }
    return resultArchive;
}

CIMGArchiveTranslatorHandle* imgExtension::NewArchive( CFileTranslator *srcRoot, const char *srcPath, eIMGArchiveVersion version )
{ return GenNewArchive( this, srcRoot, srcPath, version ); }
CIMGArchiveTranslatorHandle* imgExtension::NewArchive( CFileTranslator *srcRoot, const wchar_t *srcPath, eIMGArchiveVersion version )
{ return GenNewArchive( this, srcRoot, srcPath, version ); }

template <typename charType>
static inline CIMGArchiveTranslatorHandle* GenOpenArchive( imgExtension *env, CFileTranslator *srcRoot, const charType *srcPath )
{
    CIMGArchiveTranslatorHandle *transOut = NULL;
        
    bool hasValidArchive = false;
    eIMGArchiveVersion theVersion;

    CFile *contentFile = srcRoot->Open( srcPath, GetReadWriteMode <charType> ( false ) );

    if ( !contentFile )
    {
        return NULL;
    }

    bool hasUniqueRegistryFile = false;
    CFile *registryFile = NULL;

    // Check for version 2.
    struct mainHeader
    {
        union
        {
            unsigned char version[4];
            fsUInt_t checksum;
        };
    };

    mainHeader imgHeader;

    bool hasReadMainHeader = contentFile->ReadStruct( imgHeader );

    if ( hasReadMainHeader && imgHeader.checksum == '2REV' )
    {
        hasValidArchive = true;
        theVersion = IMG_VERSION_2;

        registryFile = contentFile;
    }

    if ( !hasValidArchive )
    {
        // Check for version 1.
        hasUniqueRegistryFile = true;

        registryFile = OpenSeperateIMGRegistryFile( srcRoot, srcPath, false );
        
        if ( registryFile )
        {
            hasValidArchive = true;
            theVersion = IMG_VERSION_1;
        }
    }

    if ( hasValidArchive )
    {
        CIMGArchiveTranslator *translator = new CIMGArchiveTranslator( *env, contentFile, registryFile, theVersion );

        if ( translator )
        {
            bool loadingSuccess = translator->ReadArchive();

            if ( loadingSuccess )
            {
                transOut = translator;
            }
            else
            {
                delete translator;

                contentFile = NULL;
                registryFile = NULL;
            }
        }
    }

    if ( !transOut )
    {
        if ( contentFile )
        {
            delete contentFile;

            contentFile = NULL;
        }

        if ( hasUniqueRegistryFile && registryFile )
        {
            delete registryFile;

            registryFile = NULL;
        }
    }

    return transOut;
}

CIMGArchiveTranslatorHandle* imgExtension::OpenArchive( CFileTranslator *srcRoot, const char *srcPath )
{ return GenOpenArchive( this, srcRoot, srcPath ); }
CIMGArchiveTranslatorHandle* imgExtension::OpenArchive( CFileTranslator *srcRoot, const wchar_t *srcPath )
{ return GenOpenArchive( this, srcRoot, srcPath ); }

CFileTranslator* imgExtension::GetTempRoot( void )
{
    return repo.GetTranslator();
}

template <typename charType>
static inline CIMGArchiveTranslatorHandle* GenOpenIMGArchive( CFileSystem *sys, CFileTranslator *srcRoot, const charType *srcPath )
{
    imgExtension *imgExt = imgExtension::Get( sys );

    if ( imgExt )
    {
        return imgExt->OpenArchive( srcRoot, srcPath );
    }
    return NULL;
}

CIMGArchiveTranslatorHandle* CFileSystem::OpenIMGArchive( CFileTranslator *srcRoot, const char *srcPath )
{ return GenOpenIMGArchive( this, srcRoot, srcPath ); }
CIMGArchiveTranslatorHandle* CFileSystem::OpenIMGArchive( CFileTranslator *srcRoot, const wchar_t *srcPath )
{ return GenOpenIMGArchive( this, srcRoot, srcPath ); }

template <typename charType>
static inline CIMGArchiveTranslatorHandle* GenCreateIMGArchive( CFileSystem *sys, CFileTranslator *srcRoot, const charType *srcPath, eIMGArchiveVersion version )
{
    imgExtension *imgExt = imgExtension::Get( sys );

    if ( imgExt )
    {
        return imgExt->NewArchive( srcRoot, srcPath, version );
    }
    return NULL;
}

CIMGArchiveTranslatorHandle* CFileSystem::CreateIMGArchive( CFileTranslator *srcRoot, const char *srcPath, eIMGArchiveVersion version )
{ return GenCreateIMGArchive( this, srcRoot, srcPath, version ); }
CIMGArchiveTranslatorHandle* CFileSystem::CreateIMGArchive( CFileTranslator *srcRoot, const wchar_t *srcPath, eIMGArchiveVersion version )
{ return GenCreateIMGArchive( this, srcRoot, srcPath, version ); }

template <typename charType>
static inline CIMGArchiveTranslatorHandle* GenOpenCompressedIMGArchive( CFileSystem *sys, CFileTranslator *srcRoot, const charType *srcPath )
{
    CIMGArchiveTranslatorHandle *archiveHandle = GenOpenIMGArchive( sys, srcRoot, srcPath );

    if ( archiveHandle )
    {
        imgExtension *imgExt = imgExtension::Get( sys );

        if ( imgExt )
        {
            // Set the xbox compression handler.
            archiveHandle->SetCompressionHandler( &imgExt->xboxCompressionHandler );
        }
    }

    return archiveHandle;
}

CIMGArchiveTranslatorHandle* CFileSystem::OpenCompressedIMGArchive( CFileTranslator *srcRoot, const char *srcPath )
{ return GenOpenCompressedIMGArchive( this, srcRoot, srcPath ); }
CIMGArchiveTranslatorHandle* CFileSystem::OpenCompressedIMGArchive( CFileTranslator *srcRoot, const wchar_t *srcPath )
{ return GenOpenCompressedIMGArchive( this, srcRoot, srcPath ); }

template <typename charType>
static inline CIMGArchiveTranslatorHandle* GenCreateCompressedIMGArchive( CFileSystem *sys, CFileTranslator *srcRoot, const charType *srcPath, eIMGArchiveVersion version )
{
    CIMGArchiveTranslatorHandle *archiveHandle = GenCreateIMGArchive( sys, srcRoot, srcPath, version );

    if ( archiveHandle )
    {
        imgExtension *imgExt = imgExtension::Get( sys );

        if ( imgExt )
        {
            // Set the xbox compression handler.
            archiveHandle->SetCompressionHandler( &imgExt->xboxCompressionHandler );
        }
    }

    return archiveHandle;
}

CIMGArchiveTranslatorHandle* CFileSystem::CreateCompressedIMGArchive( CFileTranslator *srcRoot, const char *srcPath, eIMGArchiveVersion version )
{ return GenCreateCompressedIMGArchive( this, srcRoot, srcPath, version ); }
CIMGArchiveTranslatorHandle* CFileSystem::CreateCompressedIMGArchive( CFileTranslator *srcRoot, const wchar_t *srcPath, eIMGArchiveVersion version )
{ return GenCreateCompressedIMGArchive( this, srcRoot, srcPath, version ); }

fileSystemFactory_t::pluginOffset_t imgExtension::_imgPluginOffset = fileSystemFactory_t::INVALID_PLUGIN_OFFSET;

void CFileSystemNative::RegisterIMGDriver( void )
{
    imgExtension::_imgPluginOffset =
        _fileSysFactory.RegisterDependantStructPlugin <imgExtension> ( fileSystemFactory_t::ANONYMOUS_PLUGIN_ID );
}

void CFileSystemNative::UnregisterIMGDriver( void )
{
    if ( imgExtension::_imgPluginOffset != fileSystemFactory_t::INVALID_PLUGIN_OFFSET )
    {
        _fileSysFactory.UnregisterPlugin( imgExtension::_imgPluginOffset );
    }
}