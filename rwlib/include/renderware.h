#ifndef _RENDERWARE_H_
#define _RENDERWARE_H_
#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

#include <iostream>
#include <vector>
#include <string>
#include <bitset>
#include <assert.h>
#include <algorithm>
#include <list>
#include <tuple>

#define _USE_MATH_DEFINES
#include <math.h>

// Include some special vendor libraries.
#include <CFileSystem.common.h>
#include <sdk/rwlist.hpp>
#include <sdk/Endian.h>

namespace rw
{

// Forward declaration.
struct Interface;

// Main memory allocator for everything, if I get around to standardizing it!
// TODO: improve this.
struct RwMemoryAllocator
{
public:
    inline RwMemoryAllocator( void )
    {
        return;
    }

    inline ~RwMemoryAllocator( void )
    {
        return;
    }
    
    inline void* Allocate( size_t memSize )
    {
        return new unsigned char[ memSize ];
    }

    inline void Free( void *memPtr, size_t memSize )
    {
        return delete [] memPtr;
    }
};

// Common types used in this library.
// These are defined against MSVC++ 32bit.
typedef char int8;                      // 1 byte
typedef short int16;                    // 2 bytes
typedef int int32;                      // 4 bytes
typedef long long int64;                // 8 bytes
typedef unsigned char uint8;            // 1 byte
typedef unsigned short uint16;          // 2 bytes
typedef unsigned int uint32;            // 4 bytes
typedef unsigned long long uint64;      // 8 bytes
typedef float float32;                  // 4 bytes

// These do not have to be exclusively defined, so be careful.
enum : uint32
{
    PLATFORM_OGL = 2,
    PLATFORM_PS2 = 4,
    PLATFORM_XBOX = 5
};

// An exclusively defined list of chunk ids.
enum CHUNK_TYPE
{
    CHUNK_NAOBJECT        = 0x0,
    CHUNK_STRUCT          = 0x1,
    CHUNK_STRING          = 0x2,
    CHUNK_EXTENSION       = 0x3,
    CHUNK_CAMERA          = 0x5,
    CHUNK_TEXTURE         = 0x6,
    CHUNK_MATERIAL        = 0x7,
    CHUNK_MATLIST         = 0x8,
    CHUNK_ATOMICSECT      = 0x9,
    CHUNK_PLANESECT       = 0xA,
    CHUNK_WORLD           = 0xB,
    CHUNK_SPLINE          = 0xC,
    CHUNK_MATRIX          = 0xD,
    CHUNK_FRAMELIST       = 0xE,
    CHUNK_GEOMETRY        = 0xF,
    CHUNK_CLUMP           = 0x10,
    CHUNK_LIGHT           = 0x12,
    CHUNK_UNICODESTRING   = 0x13,
    CHUNK_ATOMIC          = 0x14,
    CHUNK_TEXTURENATIVE   = 0x15,
    CHUNK_TEXDICTIONARY   = 0x16,
    CHUNK_ANIMDATABASE    = 0x17,
    CHUNK_IMAGE           = 0x18,
    CHUNK_SKINANIMATION   = 0x19,
    CHUNK_GEOMETRYLIST    = 0x1A,
    CHUNK_ANIMANIMATION   = 0x1B,
    CHUNK_HANIMANIMATION  = 0x1B,
    CHUNK_TEAM            = 0x1C,
    CHUNK_CROWD           = 0x1D,
    CHUNK_RIGHTTORENDER   = 0x1F,
    CHUNK_MTEFFECTNATIVE  = 0x20,
    CHUNK_MTEFFECTDICT    = 0x21,
    CHUNK_TEAMDICTIONARY  = 0x22,
    CHUNK_PITEXDICTIONARY = 0x23,
    CHUNK_TOC             = 0x24,
    CHUNK_PRTSTDGLOBALDATA = 0x25,
    CHUNK_ALTPIPE         = 0x26,
    CHUNK_PIPEDS          = 0x27,
    CHUNK_PATCHMESH       = 0x28,
    CHUNK_CHUNKGROUPSTART = 0x29,
    CHUNK_CHUNKGROUPEND   = 0x2A,
    CHUNK_UVANIMDICT      = 0x2B,
    CHUNK_COLLTREE        = 0x2C,
    CHUNK_ENVIRONMENT     = 0x2D,
    CHUNK_COREPLUGINIDMAX = 0x2E,

    CHUNK_MORPH           = 0x105,
    CHUNK_SKYMIPMAP       = 0x110,
    CHUNK_SKIN            = 0x116,
    CHUNK_PARTICLES       = 0x118,
    CHUNK_HANIM           = 0x11E,
    CHUNK_MATERIALEFFECTS = 0x120,
    CHUNK_ADCPLG          = 0x134,
    CHUNK_BINMESH         = 0x50E,
    CHUNK_NATIVEDATA      = 0x510,
    CHUNK_VERTEXFORMAT    = 0x510,

    CHUNK_PIPELINESET      = 0x253F2F3,
    CHUNK_SPECULARMAT      = 0x253F2F6,
    CHUNK_2DFX             = 0x253F2F8,
    CHUNK_NIGHTVERTEXCOLOR = 0x253F2F9,
    CHUNK_COLLISIONMODEL   = 0x253F2FA,
    CHUNK_REFLECTIONMAT    = 0x253F2FC,
    CHUNK_MESHEXTENSION    = 0x253F2FD,
    CHUNK_FRAME            = 0x253F2FE
};

enum
{
	RASTER_AUTOMIPMAP = 0x1000,
	RASTER_PAL8 = 0x2000,
	RASTER_PAL4 = 0x4000,
	RASTER_MIPMAP = 0x8000
};

#pragma warning(push)
#pragma warning(disable: 4996)

// Decoded RenderWare version management struct.
struct LibraryVersion
{
    inline LibraryVersion( void )
    {
        this->buildNumber = 0xFFFF;
        this->rwLibMajor = 3;
        this->rwLibMinor = 0;
        this->rwRevMajor = 0;
        this->rwRevMinor = 0;
    }

    uint16 buildNumber;
    
    union
    {
        uint32 version;
        struct
        {
            uint8 rwRevMinor;
            uint8 rwRevMajor;
            uint8 rwLibMinor;
            uint8 rwLibMajor;
        };
    };

    inline bool operator ==( const LibraryVersion& right ) const
    {
        return
            this->buildNumber == right.buildNumber &&
            this->version == right.version;
    }

    inline bool operator !=( const LibraryVersion& right ) const
    {
        return !( *this == right );
    }

    std::string toString(void) const
    {
        std::string returnVer;

        char numBuf[ 10 ];

        // Zero it.
        numBuf[ sizeof( numBuf ) - 1 ] = '\0';

        _snprintf( numBuf, sizeof( numBuf ) - 1, "%u", this->rwLibMajor );

        returnVer += numBuf;
        returnVer += ".";

        _snprintf( numBuf, sizeof( numBuf ) - 1, "%u", this->rwLibMinor );

        returnVer += numBuf;
        returnVer += ".";

        _snprintf( numBuf, sizeof( numBuf ) - 1, "%u", this->rwRevMajor );

        returnVer += numBuf;
        returnVer += ".";

        _snprintf( numBuf, sizeof( numBuf ) - 1, "%u", this->rwRevMinor );

        returnVer += numBuf;

        // If we have a valid build number, append it.
        if ( this->buildNumber != 0xFFFF )
        {
            _snprintf( numBuf, sizeof( numBuf ) - 1, "%u", this->buildNumber );

            returnVer += " (build: ";
            returnVer += numBuf;
            returnVer += ")";
        }

        return returnVer;
    }
};

#pragma warning(pop)

/*
 * DFFs
 */

enum
{ 
    FLAGS_TRISTRIP   = 0x01, 
    FLAGS_POSITIONS  = 0x02, 
    FLAGS_TEXTURED   = 0x04, 
    FLAGS_PRELIT     = 0x08, 
    FLAGS_NORMALS    = 0x10, 
    FLAGS_LIGHT      = 0x20, 
    FLAGS_MODULATEMATERIALCOLOR  = 0x40, 
    FLAGS_TEXTURED2  = 0x80
};

enum
{
	MATFX_BUMPMAP = 0x1,
	MATFX_ENVMAP = 0x2,
	MATFX_BUMPENVMAP = 0x3,
	MATFX_DUAL = 0x4,
	MATFX_UVTRANSFORM = 0x5,
	MATFX_DUALUVTRANSFORM = 0x6,
};

enum
{
	FACETYPE_STRIP = 0x1,
	FACETYPE_LIST = 0x0
};

namespace KnownVersions
{
    enum eGameVersion
    {
        GTA3,
        VC_PS2,
        VC_PC,
        SA,
        MANHUNT
    };

    LibraryVersion  getGameVersion( eGameVersion gameVer );
};

// Main RenderWare abstraction type.
struct RwObject abstract
{
    Interface *engineInterface;

    inline RwObject( Interface *engineInterface, void *construction_params );

    inline RwObject( const RwObject& right )
    {
        // Constructor that is called for cloning.
        this->engineInterface = right.engineInterface;
        this->objVersion = right.objVersion;
    }

    inline ~RwObject( void )
    {
        return;
    }

    inline Interface* GetEngine( void ) const
    {
        return this->engineInterface;
    }

    inline void SetEngineVersion( const LibraryVersion& version )
    {
        if ( version != this->objVersion )
        {
            // Notify the object about this change.
            this->OnVersionChange( this->objVersion, version );

            // Update our version.
            this->objVersion = version;
        }
    }

    inline const LibraryVersion& GetEngineVersion( void ) const
    {
        return this->objVersion;
    }

    virtual void OnVersionChange( const LibraryVersion& oldVer, const LibraryVersion& newVer )
    {
        return;
    }

protected:
    LibraryVersion objVersion;
};

// Special acquisition function using reference counting.
// Objects do not have to support reference counts, but if they do then there are special destruction semantics.
RwObject* AcquireObject( RwObject *obj );
void ReleaseObject( RwObject *obj );

uint32 GetRefCount( RwObject *obj );

struct Frame : public RwObject
{
    inline Frame( Interface *engineInterface, void *construction_params ) : RwObject( engineInterface, construction_params )
    {
        this->parent = -1;
        this->hasHAnim = false;
        this->hAnimUnknown1 = 0;
        this->hAnimBoneId = -1;
        this->hAnimBoneCount = 0;
        this->hAnimUnknown2 = 0;
        this->hAnimUnknown3 = 0;

	    for (int i = 0; i < 3; i++)
        {
		    position[i] = 0.0f;

		    for ( int j = 0; j < 3; j++ )
            {
			    rotationMatrix[ i*3 + j ] = (i == j) ? 1.0f : 0.0f;
            }
	    }
    }

	float32 rotationMatrix[9];
	float32 position[3];
	int32 parent;

	/* Extensions */

	/* node name */
	std::string name;

	/* hanim */
	bool hasHAnim;
	uint32 hAnimUnknown1;
	int32 hAnimBoneId;
	uint32 hAnimBoneCount;
	uint32 hAnimUnknown2;
	uint32 hAnimUnknown3;
	std::vector<int32> hAnimBoneIds;
	std::vector<uint32> hAnimBoneNumbers;
	std::vector<uint32> hAnimBoneTypes;

	/* functions */
	void readStruct(std::istream &dff);
	void readExtension(std::istream &dff);
	uint32 writeStruct(std::ostream &dff);
	uint32 writeExtension(std::ostream &dff);

	void dump(uint32 index, std::string ind = "");
};

// Main exception base class of this RenderWare framework.
// If you want to catch it, please make sure that you compile your project with the same compiler as this framework
// (a.k.a. ABI compatibility).
struct RwException
{
    inline RwException( const std::string& msg )
    {
        this->message = msg;
    }

    inline RwException( std::string&& msg )
    {
        this->message = msg;
    }

    std::string message;
};

#include "rwendian.h"
#include "renderware.math.h"
#include "renderware.stream.h"
#include "renderware.blockapi.h"
#include "renderware.txd.h"
#include "renderware.material.h"
#include "renderware.dff.h"
#include "renderware.imaging.h"
#include "renderware.file.h"
#include "renderware.events.h"
#include "renderware.windowing.h"
#include "renderware.threading.h"
#include "renderware.driver.h"
#include "renderware.drawing.h"

// Some pretty useful mappings.
typedef Vector <float, 2> RwV2d;
typedef Vector <float, 3> RwV3d;
typedef Vector <float, 4> RwV4d;

typedef SquareMatrix <float, 4> RwMatrix;

// Warning manager interface.
struct WarningManagerInterface abstract
{
    virtual void OnWarning( const std::string& message ) = 0;
};

// Software meta information provider struct.
struct softwareMetaInfo
{
    const char *applicationName;
    const char *applicationVersion;
    const char *description;
};

// Palettization configuration.
enum ePaletteRuntimeType
{
    PALRUNTIME_NATIVE,      // use the palettizer that is embedded into rwtools
    PALRUNTIME_PNGQUANT     // use the libimagequant vendor
};

// DXT compression configuration.
enum eDXTCompressionMethod
{
    DXTRUNTIME_NATIVE,      // prefer our own logic
    DXTRUNTIME_SQUISH       // prefer squish
};

struct Interface abstract
{
protected:
    Interface( void )
    {
        return;
    }

    ~Interface( void )
    {
        return;
    }

public:
    void                SetVersion              ( LibraryVersion version );
    LibraryVersion      GetVersion              ( void ) const;

    // Give details about the running application to RenderWare.
    void                SetApplicationInfo      ( const softwareMetaInfo& metaInfo );
    void                SetMetaDataTagging      ( bool enabled );
    bool                GetMetaDataTagging      ( void ) const;

    void                SetFileInterface        ( FileInterface *fileIntf );
    FileInterface*      GetFileInterface        ( void );

    bool                RegisterStream          ( const char *typeName, size_t memBufSize, customStreamInterface *streamInterface );
    Stream*             CreateStream            ( eBuiltinStreamType streamType, eStreamMode streamMode, streamConstructionParam_t *param );
    void                DeleteStream            ( Stream *theStream );

    // Serialization interface.
    void                SerializeBlock          ( RwObject *objectToStore, BlockProvider& outputProvider );
    void                Serialize               ( RwObject *objectToStore, Stream *outputStream );
    RwObject*           DeserializeBlock        ( BlockProvider& inputProvider );
    RwObject*           Deserialize             ( Stream *inputStream );

    // RwObject general API.
    RwObject*           ConstructRwObject       ( const char *typeName );       // construct any registered RwObject class
    RwObject*           CloneRwObject           ( const RwObject *srcObj );     // creates a new copy of an object
    void                DeleteRwObject          ( RwObject *obj );              // force kills an object (or decrements ref count)

    typedef std::vector <std::string> rwobjTypeNameList_t;

    void                GetObjectTypeNames      ( rwobjTypeNameList_t& listOut ) const;
    bool                IsObjectRegistered      ( const char *typeName ) const;
    const char*         GetObjectTypeName       ( const RwObject *rwObj ) const;

    void                SerializeExtensions     ( const RwObject *rwObj, BlockProvider& outputProvider );
    void                DeserializeExtensions   ( RwObject *rwObj, BlockProvider& inputProvider );

    // Memory management.
    void*               MemAllocate             ( size_t memSize );
    void                MemFree                 ( void *ptr );

    void*               PixelAllocate           ( size_t memSize );
    void                PixelFree               ( void *pixels );

    void                SetWarningManager       ( WarningManagerInterface *warningMan );
    WarningManagerInterface*    GetWarningManager( void ) const;

    void                SetWarningLevel         ( int level );
    int                 GetWarningLevel         ( void ) const;

    void                SetIgnoreSecureWarnings ( bool doIgnore );
    bool                GetIgnoreSecureWarnings ( void ) const;

    void                PushWarning             ( const std::string& message );

    bool                SetPaletteRuntime       ( ePaletteRuntimeType palRunType );
    ePaletteRuntimeType GetPaletteRuntime       ( void ) const;

    void                    SetDXTRuntime       ( eDXTCompressionMethod dxtRunType );
    eDXTCompressionMethod   GetDXTRuntime       ( void ) const;

    void                SetFixIncompatibleRasters   ( bool doFix );
    bool                GetFixIncompatibleRasters   ( void ) const;

    void                SetDXTPackedDecompression   ( bool packedDecompress );
    bool                GetDXTPackedDecompression   ( void ) const;

    void                SetIgnoreSerializationBlockRegions  ( bool doIgnore );
    bool                GetIgnoreSerializationBlockRegions  ( void ) const;
};

// To create a RenderWare interface, you have to go through a constructor.
Interface*  CreateEngine( LibraryVersion engine_version );
void        DeleteEngine( Interface *theEngine );

// Framework entry points.
#ifdef WIN32
BOOL WINAPI frameworkEntryPoint_win32( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow );
#endif

}

#endif
