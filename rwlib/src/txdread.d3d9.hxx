#include "txdread.d3d.hxx"

#define PLATFORM_D3D9   9

namespace rw
{

inline bool getD3DFormatFromRasterType(eRasterFormat paletteRasterType, ePaletteType paletteType, eColorOrdering colorOrder, uint32 itemDepth, D3DFORMAT& d3dFormat)
{
    bool hasFormat = false;

    if ( paletteType != PALETTE_NONE )
    {
        if ( itemDepth == 8 )
        {
            if (colorOrder == COLOR_RGBA)
            {
                d3dFormat = D3DFMT_P8;

                hasFormat = true;
            }
        }
    }
    else
    {
        if ( paletteRasterType == RASTER_1555 )
        {
            if ( itemDepth == 16 )
            {
                if (colorOrder == COLOR_BGRA)
                {
                    d3dFormat = D3DFMT_A1R5G5B5;

                    hasFormat = true;
                }
            }
        }
        else if ( paletteRasterType == RASTER_565 )
        {
            if ( itemDepth == 16 )
            {
                if (colorOrder == COLOR_BGRA)
                {
                    d3dFormat = D3DFMT_R5G6B5;

                    hasFormat = true;
                }
            }
        }
        else if ( paletteRasterType == RASTER_4444 )
        {
            if ( itemDepth == 16 )
            {
                if (colorOrder == COLOR_BGRA)
                {
                    d3dFormat = D3DFMT_A4R4G4B4;

                    hasFormat = true;
                }
            }
        }
        else if ( paletteRasterType == RASTER_LUM8 )
        {
            if ( itemDepth == 8 )
            {
                d3dFormat = D3DFMT_L8;

                hasFormat = true;
            }
        }
        else if ( paletteRasterType == RASTER_8888 )
        {
            if ( itemDepth == 32 )
            {
                if (colorOrder == COLOR_BGRA)
                {
                    d3dFormat = D3DFMT_A8R8G8B8;

                    hasFormat = true;
                }
                else if (colorOrder == COLOR_RGBA)
                {
                    d3dFormat = D3DFMT_A8B8G8R8;

                    hasFormat = true;
                }
            }
        }
        else if ( paletteRasterType == RASTER_888 )
        {
            if (colorOrder == COLOR_BGRA)
            {
                if ( itemDepth == 32 )
                {
                    d3dFormat = D3DFMT_X8R8G8B8;

                    hasFormat = true;
                }
                else if ( itemDepth == 24 )
                {
                    d3dFormat = D3DFMT_R8G8B8;

                    hasFormat = true;
                }
            }
            else if (colorOrder == COLOR_RGBA)
            {
                if ( itemDepth == 32 )
                {
                    d3dFormat = D3DFMT_X8B8G8R8;

                    hasFormat = true;
                }
            }
        }
        else if ( paletteRasterType == RASTER_555 )
        {
            if ( itemDepth == 16 )
            {
                if (colorOrder == COLOR_BGRA)
                {
                    d3dFormat = D3DFMT_X1R5G5B5;

                    hasFormat = true;
                }
            }
        }
    }

    return hasFormat;
}

struct NativeTextureD3D9 : public d3dpublic::d3dNativeTextureInterface
{
    Interface *engineInterface;

    LibraryVersion texVersion;

    inline NativeTextureD3D9( Interface *engineInterface )
    {
        // Initialize the texture object.
        this->engineInterface = engineInterface;
        this->texVersion = engineInterface->GetVersion();
        this->palette = NULL;
        this->paletteSize = 0;
        this->paletteType = PALETTE_NONE;
        this->rasterFormat = RASTER_8888;
        this->depth = 0;
        this->isCubeTexture = false;
        this->autoMipmaps = false;
        this->d3dFormat = D3DFMT_A8R8G8B8;
        this->d3dRasterFormatLink = false;
        this->anonymousFormatLink = NULL;
        this->dxtCompression = 0;
        this->rasterType = 4;
        this->hasAlpha = true;
        this->colorOrdering = COLOR_BGRA;
    }

    inline NativeTextureD3D9( const NativeTextureD3D9& right )
    {
        Interface *engineInterface = right.engineInterface;

        this->engineInterface = engineInterface;
        this->texVersion = right.texVersion;

        // Copy palette information.
        {
	        if (right.palette)
            {
                uint32 palRasterDepth = Bitmap::getRasterFormatDepth(right.rasterFormat);

                size_t wholeDataSize = getRasterDataSize( right.paletteSize, palRasterDepth );

		        this->palette = engineInterface->PixelAllocate( wholeDataSize );

		        memcpy(this->palette, right.palette, wholeDataSize);
	        }
            else
            {
		        this->palette = NULL;
	        }

            this->paletteSize = right.paletteSize;
            this->paletteType = right.paletteType;
        }

        // Copy image texel information.
        {
            copyMipmapLayers( engineInterface, right.mipmaps, this->mipmaps );

            this->rasterFormat = right.rasterFormat;
            this->depth = right.depth;
        }

        this->isCubeTexture =       right.isCubeTexture;
        this->autoMipmaps =         right.autoMipmaps;
        this->d3dFormat =           right.d3dFormat;
        this->d3dRasterFormatLink = right.d3dRasterFormatLink;
        this->anonymousFormatLink = right.anonymousFormatLink;
        this->dxtCompression =      right.dxtCompression;
        this->rasterType =          right.rasterType;
        this->hasAlpha =            right.hasAlpha;
        this->colorOrdering =       right.colorOrdering;
    }

    inline void clearTexelData( void )
    {
        if ( this->palette )
        {
	        this->engineInterface->PixelFree( palette );

	        palette = NULL;
        }

        deleteMipmapLayers( this->engineInterface, this->mipmaps );
    }

    inline ~NativeTextureD3D9( void )
    {
        this->clearTexelData();
    }

    // Implement the public API.

    bool GetD3DFormat( DWORD& d3dFormat ) const
    {
        d3dFormat = (DWORD)this->d3dFormat;
        return true;
    }

    // PUBLIC API END

public:
    typedef genmip::mipmapLayer mipmapLayer;

    eRasterFormat rasterFormat;

    uint32 depth;

	std::vector <mipmapLayer> mipmaps;

	void *palette;
	uint32 paletteSize;

    ePaletteType paletteType;

	// PC/XBOX
    bool isCubeTexture;
    bool autoMipmaps;

    D3DFORMAT d3dFormat;
    uint32 dxtCompression;
    uint32 rasterType;

    bool d3dRasterFormatLink;

    d3dpublic::nativeTextureFormatHandler *anonymousFormatLink;

    inline bool IsRWCompatible( void ) const
    {
        // This function returns whether we can push our data to the RW implementation.
        // We cannot push anything to RW that we have no idea about how it actually looks like.
        return ( this->d3dRasterFormatLink == true || this->dxtCompression != 0 );
    }

    bool hasAlpha;

    eColorOrdering colorOrdering;
};

struct d3d9NativeTextureTypeProvider : public texNativeTypeProvider, d3dpublic::d3dNativeTextureDriverInterface
{
    void ConstructTexture( Interface *engineInterface, void *objMem, size_t memSize )
    {
        new (objMem) NativeTextureD3D9( engineInterface );
    }

    void CopyConstructTexture( Interface *engineInterface, void *objMem, const void *srcObjMem, size_t memSize )
    {
        new (objMem) NativeTextureD3D9( *(const NativeTextureD3D9*)srcObjMem );
    }
    
    void DestroyTexture( Interface *engineInterface, void *objMem, size_t memSize )
    {
        ( *(NativeTextureD3D9*)objMem ).~NativeTextureD3D9();
    }

    eTexNativeCompatibility IsCompatibleTextureBlock( BlockProvider& inputProvider ) const;

    void SerializeTexture( TextureBase *theTexture, PlatformTexture *nativeTex, BlockProvider& outputProvider ) const;
    void DeserializeTexture( TextureBase *theTexture, PlatformTexture *nativeTex, BlockProvider& inputProvider ) const;

    void GetPixelCapabilities( pixelCapabilities& capsOut ) const
    {
        capsOut.supportsDXT1 = true;
        capsOut.supportsDXT2 = true;
        capsOut.supportsDXT3 = true;
        capsOut.supportsDXT4 = true;
        capsOut.supportsDXT5 = true;
        capsOut.supportsPalette = true;
    }

    void GetStorageCapabilities( storageCapabilities& storeCaps ) const
    {
        storeCaps.pixelCaps.supportsDXT1 = true;
        storeCaps.pixelCaps.supportsDXT2 = true;
        storeCaps.pixelCaps.supportsDXT3 = true;
        storeCaps.pixelCaps.supportsDXT4 = true;
        storeCaps.pixelCaps.supportsDXT5 = true;
        storeCaps.pixelCaps.supportsPalette = true;

        storeCaps.isCompressedFormat = false;
    }

    void GetPixelDataFromTexture( Interface *engineInterface, void *objMem, pixelDataTraversal& pixelsOut );
    void SetPixelDataToTexture( Interface *engineInterface, void *objMem, const pixelDataTraversal& pixelsIn, acquireFeedback_t& feedbackOut );
    void UnsetPixelDataFromTexture( Interface *engineInterface, void *objMem, bool deallocate );

    void SetTextureVersion( Interface *engineInterface, void *objMem, LibraryVersion version )
    {
        NativeTextureD3D9 *nativeTex = (NativeTextureD3D9*)objMem;

        nativeTex->texVersion = version;
    }

    LibraryVersion GetTextureVersion( const void *objMem )
    {
        const NativeTextureD3D9 *nativeTex = (const NativeTextureD3D9*)objMem;

        return nativeTex->texVersion;
    }

    bool GetMipmapLayer( Interface *engineInterface, void *objMem, uint32 mipIndex, rawMipmapLayer& layerOut );
    bool AddMipmapLayer( Interface *engineInterface, void *objMem, const rawMipmapLayer& layerIn, acquireFeedback_t& feedbackOut );
    void ClearMipmaps( Interface *engineInterface, void *objMem );

    void* GetNativeInterface( void *objMem )
    {
        NativeTextureD3D9 *nativeTex = (NativeTextureD3D9*)objMem;

        // The native interface is part of the texture.
        d3dpublic::d3dNativeTextureInterface *nativeAPI = nativeTex;

        return (void*)nativeAPI;
    }

    void* GetDriverNativeInterface( void ) const
    {
        d3dpublic::d3dNativeTextureDriverInterface *nativeDriver = (d3dpublic::d3dNativeTextureDriverInterface*)this;

        // We do export a public driver API.
        // It is the most direct way to address the D3D9 native texture environment.
        return nativeDriver;
    }

    void GetTextureInfo( Interface *engineInterface, void *objMem, nativeTextureBatchedInfo& infoOut );
    void GetTextureFormatString( Interface *engineInterface, void *objMem, char *buf, size_t bufLen, size_t& lengthOut ) const;

    ePaletteType GetTexturePaletteType( const void *objMem )
    {
        const NativeTextureD3D9 *nativeTex = (const NativeTextureD3D9*)objMem;

        return nativeTex->paletteType;
    }

    bool IsTextureCompressed( const void *objMem )
    {
        const NativeTextureD3D9 *nativeTex = (const NativeTextureD3D9*)objMem;

        return ( nativeTex->dxtCompression != 0 );
    }

    bool DoesTextureHaveAlpha( const void *objMem )
    {
        const NativeTextureD3D9 *nativeTex = (const NativeTextureD3D9*)objMem;

        return nativeTex->hasAlpha;
    }

    uint32 GetDriverIdentifier( void *objMem ) const
    {
        // We are the Direct3D 9 driver.
        return 2;
    }

    // PUBLIC API begin

    d3dpublic::nativeTextureFormatHandler* GetFormatHandler( D3DFORMAT format ) const
    {
        for ( nativeFormatExtensions_t::const_iterator iter = this->formatExtensions.cbegin(); iter != this->formatExtensions.cend(); iter++ )
        {
            const nativeFormatExtension& ext = *iter;

            if ( ext.theFormat == format )
            {
                return ext.handler;
            }
        }

        return NULL;
    }

    bool RegisterFormatHandler( DWORD format, d3dpublic::nativeTextureFormatHandler *handler );
    bool UnregisterFormatHandler( DWORD format );

    struct nativeFormatExtension
    {
        D3DFORMAT theFormat;
        d3dpublic::nativeTextureFormatHandler *handler;
    };

    typedef std::list <nativeFormatExtension> nativeFormatExtensions_t;

    nativeFormatExtensions_t formatExtensions;

    // PUBLIC API end

    inline void Initialize( Interface *engineInterface )
    {
        RegisterNativeTextureType( engineInterface, "Direct3D9", this, sizeof( NativeTextureD3D9 ) );
    }

    inline void Shutdown( Interface *engineInterface )
    {
        UnregisterNativeTextureType( engineInterface, "Direct3D9" );
    }
};

namespace d3d9
{

#pragma pack(1)
struct textureMetaHeaderStructGeneric
{
    uint32 platformDescriptor;

    rw::texFormatInfo texFormat;
    
    char name[32];
    char maskName[32];

    uint32 rasterFormat;
    D3DFORMAT d3dFormat;

    uint16 width;
    uint16 height;
    uint8 depth;
    uint8 mipmapCount;
    uint8 rasterType : 3;
    uint8 pad1 : 5;

    uint8 hasAlpha : 1;
    uint8 isCubeTexture : 1;
    uint8 autoMipMaps : 1;
    uint8 isNotRwCompatible : 1;
    uint8 pad2 : 4;
};
#pragma pack()

};

}