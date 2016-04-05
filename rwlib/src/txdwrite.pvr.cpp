#include "StdInc.h"

#ifdef RWLIB_INCLUDE_NATIVETEX_POWERVR_MOBILE

#include "pixelformat.hxx"

#include "txdread.d3d.hxx"
#include "txdread.pvr.hxx"

#include "streamutil.hxx"

#include "txdread.miputil.hxx"

namespace rw
{

eTexNativeCompatibility pvrNativeTextureTypeProvider::IsCompatibleTextureBlock( BlockProvider& inputProvider ) const
{
    eTexNativeCompatibility texCompat = RWTEXCOMPAT_NONE;

    BlockProvider texNativeImageBlock( &inputProvider );

    texNativeImageBlock.EnterContext();

    try
    {
        if ( texNativeImageBlock.getBlockID() == CHUNK_STRUCT )
        {
            // Here we can check the platform descriptor, since we know it is unique.
            uint32 platformDescriptor = texNativeImageBlock.readUInt32();

            if ( platformDescriptor == PLATFORM_PVR )
            {
                texCompat = RWTEXCOMPAT_ABSOLUTE;
            }
        }
    }
    catch( ... )
    {
        texNativeImageBlock.LeaveContext();

        throw;
    }

    texNativeImageBlock.LeaveContext();

    return texCompat;
}

void pvrNativeTextureTypeProvider::SerializeTexture( TextureBase *theTexture, PlatformTexture *nativeTex, BlockProvider& outputProvider ) const
{
    Interface *engineInterface = theTexture->engineInterface;

    // Cast the texture to our native type.
    NativeTexturePVR *platformTex = (NativeTexturePVR*)nativeTex;

    size_t mipmapCount = platformTex->mipmaps.size();

    if ( mipmapCount == 0 )
    {
        throw RwException( "attempt to write PowerVR native texture which has no mipmap layers" );
    }

	// Struct
	{
		BlockProvider texImageDataChunk( &outputProvider );
        
        texImageDataChunk.EnterContext();

        try
        {
            // Write the header with meta information.
            pvr::textureMetaHeaderGeneric metaHeader;
            metaHeader.platformDescriptor = PLATFORM_PVR;
            metaHeader.formatInfo.set( *theTexture );
            
            memset( metaHeader.pad1, 0, sizeof(metaHeader.pad1) );

            // Correctly write the name strings (for safety).
            // Even though we can read those name fields with zero-termination safety,
            // the engines are not guarranteed to do so.
            // Also, print a warning if the name is changed this way.
            writeStringIntoBufferSafe( engineInterface, theTexture->GetName(), metaHeader.name, sizeof( metaHeader.name ), theTexture->GetName(), "name" );
            writeStringIntoBufferSafe( engineInterface, theTexture->GetMaskName(), metaHeader.maskName, sizeof( metaHeader.maskName ), theTexture->GetName(), "mask name" );

            metaHeader.mipmapCount = (uint8)mipmapCount;
            metaHeader.unk1 = platformTex->unk1;
            metaHeader.hasAlpha = platformTex->hasAlpha;
            metaHeader.pad2 = 0;

            metaHeader.width = platformTex->mipmaps[ 0 ].layerWidth;
            metaHeader.height = platformTex->mipmaps[ 0 ].layerHeight;

            metaHeader.internalFormat = platformTex->internalFormat;
            
            // Calculate the image data section size.
            uint32 imageDataSectionSize = 0;

            for ( size_t n = 0; n < mipmapCount; n++ )
            {
                uint32 mipDataSize = platformTex->mipmaps[ n ].dataSize;

                imageDataSectionSize += mipDataSize;
                imageDataSectionSize += sizeof( uint32 );
            }

            metaHeader.imageDataStreamSize = imageDataSectionSize;
            metaHeader.unk8 = platformTex->unk8;

            // Write the meta header.
            texImageDataChunk.write( &metaHeader, sizeof(metaHeader) );

            // Write the mipmap data sizes.
            for ( size_t n = 0; n < mipmapCount; n++ )
            {
                uint32 mipDataSize = platformTex->mipmaps[ n ].dataSize;

                texImageDataChunk.writeUInt32( mipDataSize );
            }

            // Write the picture data now.
            for ( size_t n = 0; n < mipmapCount; n++ )
            {
                NativeTexturePVR::mipmapLayer& mipLayer = platformTex->mipmaps[ n ];

                uint32 mipDataSize = mipLayer.dataSize;

                texImageDataChunk.write( mipLayer.texels, mipDataSize );
            }
        }
        catch( ... )
        {
            texImageDataChunk.LeaveContext();

            throw;
        }

        texImageDataChunk.LeaveContext();
	}

    // Write the extensions.
    engineInterface->SerializeExtensions( theTexture, outputProvider );
}

void pvrNativeTextureTypeProvider::DecompressPVRMipmap(
    Interface *engineInterface,
    uint32 mipWidth, uint32 mipHeight, uint32 layerWidth, uint32 layerHeight, const void *srcTexels,
    eRasterFormat pvrRasterFormat, uint32 pvrDepth, eColorOrdering pvrColorOrder,
    eRasterFormat targetRasterFormat, uint32 targetDepth, uint32 targetRowAlignment, eColorOrdering targetColorOrder,
    PVRPixelType pvrSrcPixelType, PVRPixelType pvrDstPixelType,
    void*& dstTexelsOut, uint32& dstDataSizeOut
)
{
    // Create a PVR texture.
    PVRTextureHeader pvrHeader = PVRTextureHeaderCreate( PVRPixelTypeGetID( pvrSrcPixelType ), mipHeight, mipWidth );

    if ( !pvrHeader )
    {
        throw RwException( "failed to create PVRTexLib texture header" );
    }

    try
    {
        PVRTexture pvrSourceTexture = PVRTextureCreate( pvrHeader, srcTexels );

        if ( !pvrSourceTexture )
        {
            throw RwException( "failed to create PVRTexLib texture" );
        }
        
        try
        {
            // Decompress it.
            bool transcodeSuccess =
                PVRTranscode( pvrSourceTexture, pvrDstPixelType, ePVRTVarTypeUnsignedByteNorm, ePVRTCSpacelRGB );

            if ( transcodeSuccess == false )
            {
                throw RwException( "failed to decompress PVRTC compressed data using PVRTexLib" );
            }

            // Create a new raw texture of the layer dimensions.
            uint32 dstRowSize = getRasterDataRowSize( layerWidth, targetDepth, targetRowAlignment );

            uint32 dstDataSize = getRasterDataSizeByRowSize( dstRowSize, layerHeight );

            uint32 pvrWidth = mipWidth;
            uint32 pvrHeight = mipHeight;

            uint32 pvrRowSize = getRasterDataRowSize( pvrWidth, pvrDepth, getPVRToolTextureDataRowAlignment() );

            const void *srcTexelPtr = PVRTextureGetDataPtr( pvrSourceTexture );

            // Allocate new texels.
            void *dstTexels = engineInterface->PixelAllocate( dstDataSize );

            if ( !dstTexels )
            {
                throw RwException( "failed to allocate destination surface for decompressed PowerVR native texture data" );
            }

            try
            {
                colorModelDispatcher <const void> fetchDispatch( pvrRasterFormat, pvrColorOrder, pvrDepth, NULL, 0, PALETTE_NONE );
                colorModelDispatcher <void> putDispatch( targetRasterFormat, targetColorOrder, targetDepth, NULL, 0, PALETTE_NONE );

                copyTexelDataBounded(
                    srcTexelPtr, dstTexels,
                    fetchDispatch, putDispatch,
                    pvrWidth, pvrHeight,
                    layerWidth, layerHeight,
                    0, 0,
                    0, 0,
                    pvrRowSize, dstRowSize
                );
            }
            catch( ... )
            {
                // If anything went wrong in the pixel fetching, we free our data.
                engineInterface->PixelFree( dstTexels );

                throw;
            }

            // Give things to the runtime.
            dstTexelsOut = dstTexels;
            dstDataSizeOut = dstDataSize;
        }
        catch( ... )
        {
            PVRTextureDelete( pvrSourceTexture );

            throw;
        }

        PVRTextureDelete( pvrSourceTexture );
    }
    catch( ... )
    {
        PVRTextureHeaderDelete( pvrHeader );

        throw;
    }

    PVRTextureHeaderDelete( pvrHeader );
}

inline void getPVRTargetRasterFormat( ePVRInternalFormat internalFormat, eRasterFormat& targetRasterFormat, uint32& targetDepth, eColorOrdering& targetColorOrder )
{
    if ( internalFormat == GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG ||
         internalFormat == GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG )
    {
        targetRasterFormat = RASTER_888;
        targetDepth = 32;
        targetColorOrder = COLOR_RGBA;
    }
    else if ( internalFormat == GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG ||
              internalFormat == GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG )
    {
        targetRasterFormat = RASTER_8888;
        targetDepth = 32;
        targetColorOrder = COLOR_RGBA;
    }
    else
    {
        throw RwException( "failed to determine raster format for PVR texture decompression" );
    }
}

void pvrNativeTextureTypeProvider::GetPixelDataFromTexture( Interface *engineInterface, void *objMem, pixelDataTraversal& pixelsOut )
{
    NativeTexturePVR *platformTex = (NativeTexturePVR*)objMem;

    ePVRInternalFormat internalFormat = platformTex->internalFormat;

    // Decide to what raster format we should decode to.
    eRasterFormat targetRasterFormat;
    uint32 targetDepth;
    eColorOrdering targetColorOrder;

    uint32 targetRowAlignment = getPVRExportTextureDataRowAlignment();

    getPVRTargetRasterFormat( internalFormat, targetRasterFormat, targetDepth, targetColorOrder );

    // Decompress the PVR texture and put it into framework format.
    size_t mipmapCount = platformTex->mipmaps.size();

    pixelsOut.mipmaps.resize( mipmapCount );
    {
        // Create source of the pixel type descriptor.
        PVRPixelType pvrSrcPixelType = PVRGetCachedPixelType( internalFormat );

        if ( !pvrSrcPixelType )
        {
            throw RwException( "failed to decompress PVRTC due to unknown internalFormat" );
        }

        // We need a pixel type for the decompressed format.
        PVRPixelType pvrDstPixelType = this->pvrPixelType_rgba8888;

        for ( size_t n = 0; n < mipmapCount; n++ )
        {
            // Get parameters of this mipmap layer.
            const NativeTexturePVR::mipmapLayer& mipLayer = platformTex->mipmaps[ n ];

            uint32 mipWidth = mipLayer.width;
            uint32 mipHeight = mipLayer.height;

            uint32 layerWidth = mipLayer.layerWidth;
            uint32 layerHeight = mipLayer.layerHeight;

            void *srcTexels = mipLayer.texels;

            // Decompress the mipmap.
            void *dstTexels = NULL;
            uint32 dstDataSize = 0;

            DecompressPVRMipmap(
                engineInterface,
                mipWidth, mipHeight, layerWidth, layerHeight, srcTexels,
                RASTER_8888, 32, COLOR_RGBA,
                targetRasterFormat, targetDepth, targetRowAlignment, targetColorOrder,
                pvrSrcPixelType, pvrDstPixelType,
                dstTexels, dstDataSize
            );

            // Get the new texels into the virtual mipmap texture.
            pixelDataTraversal::mipmapResource newLayer;

            // The raw dimensions match the layer dimensions, because we output in a non-compressed format.
            newLayer.width = layerWidth;
            newLayer.height = layerHeight;

            newLayer.layerWidth = layerWidth;
            newLayer.layerHeight = layerHeight;

            newLayer.texels = dstTexels;
            newLayer.dataSize = dstDataSize;

            // Store it into the runtime struct.
            pixelsOut.mipmaps[ n ] = newLayer;
        }

        // We have successfully copied the mipmap data to the pixelsOut struct!
    }

    // Copy over general raster properties.
    pixelsOut.rasterFormat = targetRasterFormat;
    pixelsOut.depth = targetDepth;
    pixelsOut.rowAlignment = targetRowAlignment;
    pixelsOut.colorOrder = targetColorOrder;
    pixelsOut.paletteType = PALETTE_NONE;
    pixelsOut.paletteData = NULL;
    pixelsOut.paletteSize = 0;
    
    // We always output in a non-compressed format.
    pixelsOut.compressionType = RWCOMPRESS_NONE;

    // Move over advanced properties.
    pixelsOut.hasAlpha = platformTex->hasAlpha;
    pixelsOut.cubeTexture = false;
    pixelsOut.autoMipmaps = false;    // really? what about the other fields?

    pixelsOut.rasterType = 4;   // PowerVR does only store bitmap textures.

    // Since we decompress, we always have newly allocated pixel data.
    pixelsOut.isNewlyAllocated = true;
}

void pvrNativeTextureTypeProvider::CompressMipmapToPVR(
    Interface *engineInterface,
    uint32 mipWidth, uint32 mipHeight, const void *srcTexels,
    eRasterFormat srcRasterFormat, uint32 srcDepth, uint32 srcRowAlignment, eColorOrdering srcColorOrder, ePaletteType srcPaletteType, const void *srcPaletteData, uint32 srcPaletteSize,
    eRasterFormat pvrRasterFormat, uint32 pvrDepth, eColorOrdering pvrColorOrder,
    PVRPixelType pvrSrcPixelType, PVRPixelType pvrDstPixelType,
    uint32 pvrBlockWidth, uint32 pvrBlockHeight,
    uint32 pvrBlockDepth,
    uint32& widthOut, uint32& heightOut,
    void*& dstTexelsOut, uint32& dstDataSizeOut
)
{
    // Create a PVR texture.
    uint32 srcRowSize = getRasterDataRowSize( mipWidth, srcDepth, srcRowAlignment );

    // We need to determine dimensions that the PVR texture has to use.
    uint32 pvrTexWidth = ALIGN_SIZE( mipWidth, pvrBlockWidth );
    uint32 pvrTexHeight = ALIGN_SIZE( mipHeight, pvrBlockHeight );

    uint32 pvrRowSize = getRasterDataRowSize( pvrTexWidth, pvrDepth, getPVRToolTextureDataRowAlignment() );

    PVRTextureHeader pvrHeader = PVRTextureHeaderCreate( PVRPixelTypeGetID( pvrSrcPixelType ), pvrTexHeight, pvrTexWidth );

    if ( !pvrHeader )
    {
        throw RwException( "failed to create PVRTexLib texture header" );
    }

    try
    {
        // Copy stuff into the PVR texture properly.
        PVRTexture pvrTexture = PVRTextureCreate( pvrHeader );

        if ( !pvrTexture )
        {
            throw RwException( "failed to create PVRTexLib texture handle" );
        }

        try
        {
            // Process the colors into the PowerVR texture.
            {
                void *pvrDstBuf = PVRTextureGetDataPtr( pvrTexture );

                colorModelDispatcher <const void> fetchDispatch( srcRasterFormat, srcColorOrder, srcDepth, srcPaletteData, srcPaletteSize, srcPaletteType );
                colorModelDispatcher <void> putDispatch( pvrRasterFormat, pvrColorOrder, pvrDepth, NULL, 0, PALETTE_NONE );

                copyTexelDataBounded(
                    srcTexels, pvrDstBuf,
                    fetchDispatch, putDispatch,
                    mipWidth, mipHeight,
                    pvrTexWidth, pvrTexHeight,
                    0, 0,
                    0, 0,
                    srcRowSize, pvrRowSize
                );
            }

            // Transcode it.
            bool transcodeSuccess =
                PVRTranscode( pvrTexture, pvrDstPixelType, ePVRTVarTypeUnsignedByteNorm, ePVRTCSpacelRGB );

            if ( transcodeSuccess == false )
            {
                throw RwException( "failed to compress texture data to PVRTC using PVRTexLib" );
            }

            // Copy the PowerVR pixels into a local array.
            uint32 dstDataSize = getPackedRasterDataSize(pvrTexWidth * pvrTexHeight, pvrBlockDepth);

            PVRTextureHeaderCheckDataSize( pvrTexture, dstDataSize );

            void *dstTexels = engineInterface->PixelAllocate( dstDataSize );

            memcpy( dstTexels, PVRTextureGetDataPtr( pvrTexture ), dstDataSize );

            // Give parameters to the runtime.
            widthOut = pvrTexWidth;
            heightOut = pvrTexHeight;

            dstTexelsOut = dstTexels;
            dstDataSizeOut = dstDataSize;
        }
        catch( ... )
        {
            PVRTextureDelete( pvrTexture );

            throw;
        }

        PVRTextureDelete( pvrTexture );
    }
    catch( ... )
    {
        PVRTextureHeaderDelete( pvrHeader );

        throw;
    }

    PVRTextureHeaderDelete( pvrHeader );
}

void pvrNativeTextureTypeProvider::SetPixelDataToTexture( Interface *engineInterface, void *objMem, const pixelDataTraversal& pixelsIn, acquireFeedback_t& feedbackOut )
{
    // Allocate a new texture.
    NativeTexturePVR *pvrTex = (NativeTexturePVR*)objMem;

    // We can only accept raw bitmaps here.
    assert( pixelsIn.compressionType == RWCOMPRESS_NONE );

    // Verify some mipmap dimension rules.
    {
        nativeTextureSizeRules sizeRules;
        getPVRNativeTextureSizeRules( sizeRules );

        if ( !sizeRules.verifyPixelData( pixelsIn ) )
        {
            throw RwException( "invalid mipmap dimensions in PowerVR native texture pixel acquisition" );
        }
    }

    // Give it common parameters.
    bool hasAlpha = pixelsIn.hasAlpha;

    pvrTex->hasAlpha = hasAlpha;

    // Copy over compressed texels.
    eRasterFormat srcRasterFormat = pixelsIn.rasterFormat;
    eColorOrdering srcColorOrder = pixelsIn.colorOrder;
    uint32 srcDepth = pixelsIn.depth;
    uint32 srcRowAlignment = pixelsIn.rowAlignment;

    ePaletteType srcPaletteType = pixelsIn.paletteType;
    const void *paletteData = pixelsIn.paletteData;
    uint32 paletteSize = pixelsIn.paletteSize;

    // Determine the internal format we are going to compress to.
    ePVRInternalFormat internalFormat = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;

    const pixelDataTraversal::mipmapResource& mainMipLayer = pixelsIn.mipmaps[ 0 ];

    bool canBeCompressedHigh = ( mainMipLayer.layerWidth * mainMipLayer.layerHeight ) >= ( 100 * 100 );

    if ( hasAlpha )
    {
        if ( canBeCompressedHigh )
        {
            internalFormat = GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
        }
        else
        {
            internalFormat = GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
        }
    }
    else
    {
        if ( canBeCompressedHigh )
        {
            internalFormat = GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
        }
        else
        {
            internalFormat = GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
        }
    }

    size_t mipmapCount = pixelsIn.mipmaps.size();

    {
        // Determine the source pixel format.
        PVRPixelType pvrSrcPixelType = this->pvrPixelType_rgba8888;

        // Transform the internal format into a pvrtexlib parameter.
        PVRPixelType pvrDstPixelType = PVRGetCachedPixelType( internalFormat );

        if ( !pvrDstPixelType )
        {
            throw RwException( "failed to compress PVRTC due to unknown internalFormat" );
        }

        // Determine the block dimensions of the PVR destination texture.
        uint32 pvrBlockWidth, pvrBlockHeight;

        uint32 pvrDepth = getDepthByPVRFormat( internalFormat );

        bool gotDimms = getPVRCompressionBlockDimensions( pvrDepth, pvrBlockWidth, pvrBlockHeight );

        if ( !gotDimms )
        {
            throw RwException( "failed to get PVR block compression dimensions in PVR native texture texel acquisition" );
        }

        // Pre-allocate the mipmap array.
        pvrTex->mipmaps.resize( mipmapCount );

        for ( uint32 n = 0; n < mipmapCount; n++ )
        {
            // Get parameters of this mipmap layer.
            const pixelDataTraversal::mipmapResource& mipLayer = pixelsIn.mipmaps[ n ];

            // Create a PVR texture.
            uint32 mipWidth = mipLayer.width;
            uint32 mipHeight = mipLayer.height;

            uint32 layerWidth = mipLayer.layerWidth;
            uint32 layerHeight = mipLayer.layerHeight;

            const void *srcTexels = mipLayer.texels;

            // Compress stuff.
            uint32 compressedWidth, compressedHeight;

            void *dstTexels = NULL;
            uint32 dstDataSize = 0;

            CompressMipmapToPVR(
                engineInterface,
                mipWidth, mipHeight, srcTexels,
                srcRasterFormat, srcDepth, srcRowAlignment, srcColorOrder, srcPaletteType, paletteData, paletteSize,
                RASTER_8888, 32, COLOR_RGBA,
                pvrSrcPixelType, pvrDstPixelType,
                pvrBlockWidth, pvrBlockHeight,
                pvrDepth,
                compressedWidth, compressedHeight,
                dstTexels, dstDataSize
            );

            // Put the result into a new mipmap layer.
            NativeTexturePVR::mipmapLayer newLayer;

            newLayer.width = compressedWidth;
            newLayer.height = compressedHeight;

            newLayer.layerWidth = layerWidth;
            newLayer.layerHeight = layerHeight;

            newLayer.texels = dstTexels;
            newLayer.dataSize = dstDataSize;

            // Put the new layer.
            pvrTex->mipmaps[ n ] = newLayer;
        }
    }

    // Store more advanced properties.
    pvrTex->internalFormat = internalFormat;

    // Since we always compress pixels, we cannot directly acquire.
    feedbackOut.hasDirectlyAcquired = false;
}

void pvrNativeTextureTypeProvider::UnsetPixelDataFromTexture( Interface *engineInterface, void *objMem, bool deallocate )
{
    NativeTexturePVR *nativeTex = (NativeTexturePVR*)objMem;

    if ( deallocate )
    {
        // Delete all pixel data.
        deleteMipmapLayers( engineInterface, nativeTex->mipmaps );
    }

    // Clear mipmap data.
    nativeTex->mipmaps.clear();
}

struct pvrMipmapManager
{
    pvrNativeTextureTypeProvider *typeProv;
    NativeTexturePVR *nativeTex;

    inline pvrMipmapManager( pvrNativeTextureTypeProvider *typeProv, NativeTexturePVR *nativeTex )
    {
        this->typeProv = typeProv;
        this->nativeTex = nativeTex;
    }

    inline void GetLayerDimensions(
        const NativeTexturePVR::mipmapLayer& mipLayer,
        uint32& layerWidth, uint32& layerHeight
    )
    {
        layerWidth = mipLayer.layerWidth;
        layerHeight = mipLayer.layerHeight;
    }

    inline void GetSizeRules( nativeTextureSizeRules& rulesOut ) const
    {
        getPVRNativeTextureSizeRules( rulesOut );
    }

    inline void Deinternalize(
        Interface *engineInterface,
        const NativeTexturePVR::mipmapLayer& mipLayer,
        uint32& widthOut, uint32& heightOut, uint32& layerWidthOut, uint32& layerHeightOut,
        eRasterFormat& dstRasterFormat, eColorOrdering& dstColorOrder, uint32& dstDepth,
        uint32& dstRowAlignment,
        ePaletteType& dstPaletteType, void*& dstPaletteData, uint32& dstPaletteSize,
        eCompressionType& dstCompressionType, bool& hasAlpha,
        void*& dstTexelsOut, uint32& dstDataSizeOut,
        bool& isNewlyAllocatedOut, bool& isPaletteNewlyAllocated
    )
    {
        ePVRInternalFormat internalFormat = nativeTex->internalFormat;

        uint32 mipWidth = mipLayer.width;
        uint32 mipHeight = mipLayer.height;

        uint32 layerWidth = mipLayer.layerWidth;
        uint32 layerHeight = mipLayer.layerHeight;

        const void *srcTexels = mipLayer.texels;

        // Decide to what raster format we should decode to.
        eRasterFormat targetRasterFormat;
        uint32 targetDepth;
        eColorOrdering targetColorOrder;

        uint32 targetRowAlignment = getPVRExportTextureDataRowAlignment();

        getPVRTargetRasterFormat( internalFormat, targetRasterFormat, targetDepth, targetColorOrder );

        // Create source of the pixel type descriptor.
        pvrNativeTextureTypeProvider::PVRPixelType pvrSrcPixelType = typeProv->PVRGetCachedPixelType( internalFormat );

        if ( !pvrSrcPixelType )
        {
            throw RwException( "failed to decompress PVRTC due to unknown internalFormat" );
        }

        // We need a pixel type for the decompressed format.
        pvrNativeTextureTypeProvider::PVRPixelType pvrDstPixelType = typeProv->pvrPixelType_rgba8888;

        // Do the decompression.
        void *dstTexels = NULL;
        uint32 dstDataSize = 0;

        typeProv->DecompressPVRMipmap(
            engineInterface,
            mipWidth, mipHeight, layerWidth, layerHeight, srcTexels,
            RASTER_8888, 32, COLOR_RGBA,
            targetRasterFormat, targetDepth, targetRowAlignment, targetColorOrder,
            pvrSrcPixelType, pvrDstPixelType,
            dstTexels, dstDataSize
        );

        // Give values to the runtime.
        widthOut = layerWidth;
        heightOut = layerHeight;

        layerWidthOut = layerWidth;
        layerHeightOut = layerHeight;

        dstRasterFormat = targetRasterFormat;
        dstDepth = targetDepth;
        dstRowAlignment = targetRowAlignment;
        dstColorOrder = targetColorOrder;

        dstPaletteType = PALETTE_NONE;
        dstPaletteData = NULL;
        dstPaletteSize = 0;

        dstCompressionType = RWCOMPRESS_NONE;

        hasAlpha = nativeTex->hasAlpha;

        dstTexelsOut = dstTexels;
        dstDataSizeOut = dstDataSize;

        isNewlyAllocatedOut = true;
        isPaletteNewlyAllocated = false;
    }

    inline void Internalize(
        Interface *engineInterface,
        NativeTexturePVR::mipmapLayer& mipLayer,
        uint32 width, uint32 height, uint32 layerWidth, uint32 layerHeight, void *srcTexels, uint32 dataSize,
        eRasterFormat rasterFormat, eColorOrdering colorOrder, uint32 depth,
        uint32 rowAlignment,
        ePaletteType paletteType, void *paletteData, uint32 paletteSize,
        eCompressionType compressionType, bool hasAlpha,
        bool& hasDirectlyAcquiredOut
    )
    {
        // We want to compress the input and insert it into our texture.
        ePVRInternalFormat internalFormat = nativeTex->internalFormat;

        // If the input is not in raw bitmap format, convert it to raw format.
        bool srcTexelsNewlyAllocated = false;

        if ( compressionType != RWCOMPRESS_NONE )
        {
            eRasterFormat targetRasterFormat = RASTER_8888;
            uint32 targetDepth = 32;
            eColorOrdering targetColorOrder = COLOR_RGBA;
            
            uint32 targetRowAlignment = 4;  // good measure.

            bool hasChanged =
                ConvertMipmapLayerNative(
                    engineInterface,
                    width, height, layerWidth, layerHeight, srcTexels, dataSize,
                    rasterFormat, depth, rowAlignment, colorOrder, paletteType, paletteData, paletteSize, compressionType,
                    targetRasterFormat, targetDepth, targetRowAlignment, targetColorOrder, PALETTE_NONE, NULL, 0, RWCOMPRESS_NONE,
                    false,
                    width, height,
                    srcTexels, dataSize
                );

            if ( hasChanged == false )
            {
                throw RwException( "failed to decompress in PVR native texture mipmap manager" );
            }

            // We are now in raw format.
            compressionType = RWCOMPRESS_NONE;

            rasterFormat = targetRasterFormat;
            depth = targetDepth;
            rowAlignment = targetRowAlignment;
            colorOrder = targetColorOrder;

            paletteType = PALETTE_NONE;
            paletteData = NULL;
            paletteSize = 0;

            srcTexelsNewlyAllocated = true;
        }

        // Determine the source pixel format.
        pvrNativeTextureTypeProvider::PVRPixelType pvrSrcPixelType = typeProv->pvrPixelType_rgba8888;

        // Transform the internal format into a pvrtexlib parameter.
        pvrNativeTextureTypeProvider::PVRPixelType pvrDstPixelType = typeProv->PVRGetCachedPixelType( internalFormat );

        if ( !pvrDstPixelType )
        {
            throw RwException( "failed to compress PVRTC due to unknown internalFormat" );
        }

        // Determine the block dimensions of the PVR destination texture.
        uint32 pvrBlockWidth, pvrBlockHeight;

        uint32 pvrDepth = getDepthByPVRFormat( internalFormat );

        bool gotDimms = getPVRCompressionBlockDimensions( pvrDepth, pvrBlockWidth, pvrBlockHeight );

        if ( gotDimms )
        {
            throw RwException( "failed to get PVR native texture block compression dimensions in mipmap texel acquisition" );
        }

        // Do the compression.
        uint32 compressedWidth, compressedHeight;

        void *dstTexels = NULL;
        uint32 dstDataSize = 0;

        typeProv->CompressMipmapToPVR(
            engineInterface,
            width, height, srcTexels,
            rasterFormat, depth, rowAlignment, colorOrder, paletteType, paletteData, paletteSize,
            RASTER_8888, 32, COLOR_RGBA,
            pvrSrcPixelType, pvrDstPixelType,
            pvrBlockWidth, pvrBlockHeight,
            pvrDepth,
            compressedWidth, compressedHeight,
            dstTexels, dstDataSize
        );

        // Free temporary copies of srcTexels.
        if ( srcTexelsNewlyAllocated )
        {
            engineInterface->PixelFree( srcTexels );
        }

        // Store the texels.
        mipLayer.width = compressedWidth;
        mipLayer.height = compressedHeight;

        mipLayer.layerWidth = layerWidth;
        mipLayer.layerHeight = layerHeight;

        mipLayer.texels = dstTexels;
        mipLayer.dataSize = dstDataSize;

        // We have compressed texels, so no direct acquisition.
        hasDirectlyAcquiredOut = false;
    }
};

bool pvrNativeTextureTypeProvider::GetMipmapLayer( Interface *engineInterface, void *objMem, uint32 mipIndex, rawMipmapLayer& layerOut )
{
    NativeTexturePVR *nativeTex = (NativeTexturePVR*)objMem;

    pvrMipmapManager mipMan( this, nativeTex );

    return
        virtualGetMipmapLayer <NativeTexturePVR::mipmapLayer> (
            engineInterface, mipMan,
            mipIndex,
            nativeTex->mipmaps, layerOut
        );
}

bool pvrNativeTextureTypeProvider::AddMipmapLayer( Interface *engineInterface, void *objMem, const rawMipmapLayer& layerIn, acquireFeedback_t& feedbackOut )
{
    NativeTexturePVR *nativeTex = (NativeTexturePVR*)objMem;

    pvrMipmapManager mipMan( this, nativeTex );

    return
        virtualAddMipmapLayer <NativeTexturePVR::mipmapLayer> (
            engineInterface, mipMan,
            nativeTex->mipmaps, layerIn,
            feedbackOut
        );
}

void pvrNativeTextureTypeProvider::ClearMipmaps( Interface *engineInterface, void *objMem )
{
    NativeTexturePVR *nativeTex = (NativeTexturePVR*)objMem;

    virtualClearMipmaps <NativeTexturePVR::mipmapLayer> ( engineInterface, nativeTex->mipmaps );
}

void pvrNativeTextureTypeProvider::GetTextureInfo( Interface *engineInterface, void *objMem, nativeTextureBatchedInfo& infoOut )
{
    NativeTexturePVR *nativeTex = (NativeTexturePVR*)objMem;

    size_t mipmapCount = nativeTex->mipmaps.size();

    infoOut.mipmapCount = (uint32)mipmapCount;

    uint32 baseWidth = 0;
    uint32 baseHeight = 0;

    if ( mipmapCount > 0 )
    {
        baseWidth = nativeTex->mipmaps[ 0 ].layerWidth;
        baseHeight = nativeTex->mipmaps[ 0 ].layerHeight;
    }

    infoOut.baseWidth = baseWidth;
    infoOut.baseHeight = baseHeight;
}

void pvrNativeTextureTypeProvider::GetTextureFormatString( Interface *engineInterface, void *objMem, char *buf, size_t bufLen, size_t& lengthOut ) const
{
    NativeTexturePVR *nativeTex = (NativeTexturePVR*)objMem;

    // Return a format string based on the internalFormat parameter.
    std::string formatString( "PVR " );

    ePVRInternalFormat internalFormat = nativeTex->internalFormat;

    if ( internalFormat == ePVRInternalFormat::GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG )
    {
        formatString += "RGB 2bit";
    }
    else if ( internalFormat == ePVRInternalFormat::GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG )
    {
        formatString += "RGBA 2bit";
    }
    else if ( internalFormat == ePVRInternalFormat::GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG )
    {
        formatString += "RGB 4bit";
    }
    else if ( internalFormat == ePVRInternalFormat::GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG )
    {
        formatString += "RGBA 4bit";
    }

    if ( buf )
    {
        strncpy( buf, formatString.c_str(), bufLen );
    }

    lengthOut = formatString.size();
}

};

#endif //RWLIB_INCLUDE_NATIVETEX_POWERVR_MOBILE