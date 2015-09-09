#include "txdread.nativetex.hxx"

#define PS2_FOURCC 0x00325350 /* "PS2\0" */

namespace rw
{

inline uint32 getPS2TextureDataRowAlignment( void )
{
    // For compatibility reasons, we say that swizzled mipmap data has a row alignment of 1.
    // It should not matter for any of the operations we do.
    return 1;
}

inline uint32 getPS2ExportTextureDataRowAlignment( void )
{
    // This row alignment should be a framework friendly size.
    // To make things most-compatible with Direct3D, a size of 4 is recommended.
    return 4;
}

inline uint32 getPS2RasterDataRowSize( uint32 mipWidth, uint32 depth )
{
    return getRasterDataRowSize( mipWidth, depth, getPS2TextureDataRowAlignment() );
}

struct ps2MipmapTransmissionData
{
    uint16 destX, destY;
};

struct ps2GSRegisters
{
    typedef unsigned long long ps2reg_t;

    struct TEX0_REG
    {
        inline TEX0_REG( void )
        {
            *(ps2reg_t*)this = 0L;
        }

        inline TEX0_REG( ps2reg_t val )
        {
            *(ps2reg_t*)this = val;
        }

        ps2reg_t textureBasePointer : 14;
        ps2reg_t textureBufferWidth : 6;
        ps2reg_t pixelStorageFormat : 6;
        ps2reg_t textureWidthLog2 : 4;
        ps2reg_t textureHeightLog2 : 4;
        ps2reg_t texColorComponent : 1;
        ps2reg_t texFunction : 2;
        ps2reg_t clutBufferBase : 14;
        ps2reg_t clutStorageFmt : 4;
        ps2reg_t clutMode : 1;
        ps2reg_t clutEntryOffset : 5;
        ps2reg_t clutLoadControl : 3;

        inline bool operator ==( const TEX0_REG& right ) const
        {
            return ( *(ps2reg_t*)this == *(ps2reg_t*)&right );
        }

        inline bool operator !=( const TEX0_REG& right ) const
        {
            return !( *this == right );
        }

        inline operator ps2reg_t ( void ) const
        {
            return *(ps2reg_t*)this;
        }
    };

    struct TEX1_REG
    {
        inline TEX1_REG( void )
        {
            *(ps2reg_t*)this = 0L;
        }

        inline TEX1_REG( ps2reg_t val )
        {
            *(ps2reg_t*)this = val;
        }

        ps2reg_t lodCalculationModel : 1;
        ps2reg_t unknown2 : 1;
        ps2reg_t maximumMIPLevel : 3;
        ps2reg_t mmag : 1;
        ps2reg_t mmin : 3;
        ps2reg_t mtba : 1;
        ps2reg_t unknown : 1;
        ps2reg_t unused2 : 8;
        ps2reg_t lodParamL : 2;
        ps2reg_t unused3 : 11;
        ps2reg_t lodParamK : 12;

        inline bool operator ==( const TEX1_REG& right ) const
        {
#if 0
            return
                this->lodCalculationModel == right.lodCalculationModel &&
                this->maximumMIPLevel == right.maximumMIPLevel &&
                this->mmag == right.mmag &&
                this->mmin == right.mmin &&
                this->mtba == right.mtba &&
                this->lodParamL == right.lodParamL &&
                this->lodParamK == right.lodParamK &&
                this->unknown == right.unknown &&
                this->unknown2 == right.unknown2;
#else
            return ( *(ps2reg_t*)this == *(ps2reg_t*)&right );
#endif
        }

        inline bool operator !=( const TEX1_REG& right ) const
        {
            return !( *this == right );
        }

        inline operator ps2reg_t ( void ) const
        {
            return *(ps2reg_t*)this;
        }
    };

    struct MIPTBP1_REG
    {
        inline MIPTBP1_REG( void )
        {
            *(ps2reg_t*)this = 0L;
        }

        inline MIPTBP1_REG( ps2reg_t val )
        {
            *(ps2reg_t*)this = val;
        }

        ps2reg_t textureBasePointer1 : 14;
        ps2reg_t textureBufferWidth1 : 6;
        ps2reg_t textureBasePointer2 : 14;
        ps2reg_t textureBufferWidth2 : 6;
        ps2reg_t textureBasePointer3 : 14;
        ps2reg_t textureBufferWidth3 : 6;

        inline bool operator ==( const MIPTBP1_REG& right ) const
        {
#if 0
            return
                this->textureBasePointer1 == right.textureBasePointer1 &&
                this->textureBufferWidth1 == right.textureBufferWidth1 &&
                this->textureBasePointer2 == right.textureBasePointer2 &&
                this->textureBufferWidth2 == right.textureBufferWidth2 &&
                this->textureBasePointer3 == right.textureBasePointer3 &&
                this->textureBufferWidth3 == right.textureBufferWidth3;
#else
            return ( *(ps2reg_t*)this == *(ps2reg_t*)&right );
#endif
        }

        inline bool operator !=( const MIPTBP1_REG& right ) const
        {
            return !( *this == right );
        }

        inline operator ps2reg_t ( void ) const
        {
            return *(ps2reg_t*)this;
        }
    };

    struct MIPTBP2_REG
    {
        inline MIPTBP2_REG( void )
        {
            *(ps2reg_t*)this = 0L;
        }

        inline MIPTBP2_REG( ps2reg_t val )
        {
            *(ps2reg_t*)this = val;
        }

        ps2reg_t textureBasePointer4 : 14;
        ps2reg_t textureBufferWidth4 : 6;
        ps2reg_t textureBasePointer5 : 14;
        ps2reg_t textureBufferWidth5 : 6;
        ps2reg_t textureBasePointer6 : 14;
        ps2reg_t textureBufferWidth6 : 6;

        inline bool operator ==( const MIPTBP2_REG& right ) const
        {
#if 0
            return
                this->textureBasePointer4 == right.textureBasePointer4 &&
                this->textureBufferWidth4 == right.textureBufferWidth4 &&
                this->textureBasePointer5 == right.textureBasePointer5 &&
                this->textureBufferWidth5 == right.textureBufferWidth5 &&
                this->textureBasePointer6 == right.textureBasePointer6 &&
                this->textureBufferWidth6 == right.textureBufferWidth6;
#else
            return ( *(ps2reg_t*)this == *(ps2reg_t*)&right );
#endif
        }

        inline bool operator !=( const MIPTBP2_REG& right ) const
        {
            return !( *this == right );
        }

        inline operator ps2reg_t ( void ) const
        {
            return *(ps2reg_t*)this;
        }
    };

    struct TRXPOS_REG
    {
        inline TRXPOS_REG( void )
        {
            this->qword = 0L;
        }

        inline TRXPOS_REG( ps2reg_t val )
        {
            this->qword = val;
        }

        union
        {
            struct
            {
                ps2reg_t ssax : 11;
                ps2reg_t pad1 : 5;
                ps2reg_t ssay : 11;
                ps2reg_t pad2 : 5;
                ps2reg_t dsax : 11;
                ps2reg_t pad3 : 5;
                ps2reg_t dsay : 11;
                ps2reg_t dir : 2;
            };
            struct
            {
                ps2reg_t qword;
            };
        };

        inline operator ps2reg_t ( void ) const
        {
            return *(ps2reg_t*)this;
        }
    };

    struct TRXREG_REG
    {
        inline TRXREG_REG( void )
        {
            this->qword = 0L;
        }

        inline TRXREG_REG( ps2reg_t val )
        {
            this->qword = val;
        }

        union
        {
            struct
            {
                ps2reg_t transmissionAreaWidth : 12;
                ps2reg_t pad1 : 20;
                ps2reg_t transmissionAreaHeight : 12;
            };
            struct
            {
                ps2reg_t qword;
            };
        };

        inline operator ps2reg_t ( void ) const
        {
            return *(ps2reg_t*)this;
        }
    };

    struct TRXDIR_REG
    {
        inline TRXDIR_REG( void )
        {
            this->qword = 0L;
        }

        inline TRXDIR_REG( ps2reg_t val )
        {
            this->qword = val;
        }

        union
        {
            struct
            {
                ps2reg_t xdir : 2;
            };
            struct
            {
                ps2reg_t qword;
            };
        };

        inline operator ps2reg_t ( void ) const
        {
            return *(ps2reg_t*)this;
        }
    };

    TEX0_REG tex0;
    TEX1_REG tex1;

    MIPTBP1_REG miptbp1;
    MIPTBP2_REG miptbp2;
};

struct textureMetaDataHeader
{
    endian::little_endian <uint32> width;
    endian::little_endian <uint32> height;
    endian::little_endian <uint32> depth;
    endian::little_endian <uint32> rasterFormat;

    endian::little_endian <ps2GSRegisters::ps2reg_t> tex0;
    endian::little_endian <ps2GSRegisters::ps2reg_t> tex1;

    endian::little_endian <ps2GSRegisters::ps2reg_t> miptbp1;
    endian::little_endian <ps2GSRegisters::ps2reg_t> miptbp2;

    endian::little_endian <uint32> dataSize;	    // texels + header
    endian::little_endian <uint32> paletteDataSize; // palette + header + unknowns

    endian::little_endian <uint32> combinedGPUDataSize;

    // constant (Sky mipmap val)
    // see http://www.gtamodding.com/wiki/Sky_Mipmap_Val_%28RW_Section%29
    endian::little_endian <uint32> skyMipmapVal;
};

enum eGSRegister
{
    GIF_REG_PRIM,
    GIF_REG_RGBAQ,
    GIF_REG_ST,
    GIF_REG_UV,
    GIF_REG_XYZF2,
    GIF_REG_XYZ2,
    GIF_REG_TEX0_1,
    GIF_REG_TEX0_2,
    GIF_REG_CLAMP_1,
    GIF_REG_CLAMP_2,
    GIF_REG_FOG,
    GIF_REG_XYZF3 = 0x0C,
    GIF_REG_XYZ3,

    GIF_REG_TEX1_1 = 0x14,
    GIF_REG_TEX1_2,
    GIF_REG_TEX2_1,
    GIF_REG_TEX2_2,
    GIF_REG_XYOFFSET_1,
    GIF_REG_XYOFFSET_2,
    GIF_REG_PRMODECONT,
    GIF_REG_PRMODE,
    GIF_REG_TEXCLUT,

    GIF_REG_SCANMSK = 0x22,

    GIF_REG_MIPTBP1_1 = 0x34,
    GIF_REG_MIPTBP1_2,
    GIF_REG_MIPTBP2_1,
    GIF_REG_MIPTBP2_2,

    GIF_REG_TEXA = 0x3B,

    GIF_REG_FOGCOL = 0x3D,

    GIF_REG_TEXFLUSH = 0x3F,
    GIF_REG_SCISSOR_1,
    GIF_REG_SCISSOR_2,
    GIF_REG_ALPHA_1,
    GIF_REG_ALPHA_2,
    GIF_REG_DIMX,
    GIF_REG_DTHE,
    GIF_REG_COLCLAMP,
    GIF_REG_TEST_1,
    GIF_REG_TEST_2,
    GIF_REG_PABE,
    GIF_REG_FBA_1,
    GIF_REG_FBA_2,
    GIF_REG_FRAME_1,
    GIF_REG_FRAME_2,
    GIF_REG_ZBUF_1,
    GIF_REG_ZBUF_2,
    GIF_REG_BITBLTBUF,

    GIF_REG_TRXPOS,
    GIF_REG_TRXREG,
    GIF_REG_TRXDIR,

    GIF_REG_HWREG,

    GIF_REG_SIGNAL,
    GIF_REG_FINISH,
    GIF_REG_LABEL
};

enum eMemoryLayoutType
{
    PSMCT32 = 0,
    PSMCT24,
    PSMCT16,

    PSMCT16S = 10,

    PSMT8 = 19,
    PSMT4,

    PSMT8H = 27,
    PSMT4HL = 36,
    PSMT4HH = 44,

    PSMZ32 = 48,
    PSMZ24,
    PSMZ16,

    PSMZ16S = 58
};

enum eFormatEncodingType
{
    FORMAT_UNKNOWN,
    FORMAT_IDTEX4,
    FORMAT_IDTEX8,
    FORMAT_IDTEX8_COMPRESSED,
    FORMAT_TEX16,
    FORMAT_TEX32
};

inline bool isPackOperation( eFormatEncodingType srcFormat, eFormatEncodingType dstFormat )
{
    if ( srcFormat == dstFormat )
        return false;

    if ( srcFormat == FORMAT_IDTEX4 )
    {
        if ( dstFormat == FORMAT_IDTEX8 ||
             dstFormat == FORMAT_IDTEX8_COMPRESSED ||
             dstFormat == FORMAT_TEX16 ||
             dstFormat == FORMAT_TEX32 )
        {
            return true;
        }
    }
    else if ( srcFormat == FORMAT_IDTEX8 )
    {
        if ( dstFormat == FORMAT_TEX16 ||
             dstFormat == FORMAT_TEX32 )
        {
            return true;
        }
        else if ( dstFormat == FORMAT_IDTEX4 ||
                  dstFormat == FORMAT_IDTEX8_COMPRESSED )
        {
            return false;
        }
    }
    else if ( srcFormat == FORMAT_IDTEX8_COMPRESSED )
    {
        if ( dstFormat == FORMAT_TEX16 ||
             dstFormat == FORMAT_TEX32 )
        {
            return true;
        }
        else if ( dstFormat == FORMAT_IDTEX4 ||
                  dstFormat == FORMAT_IDTEX8_COMPRESSED )
        {
            return false;
        }
    }
    else if ( srcFormat == FORMAT_TEX16 )
    {
        if ( dstFormat == FORMAT_IDTEX4 ||
             dstFormat == FORMAT_IDTEX8 ||
             dstFormat == FORMAT_IDTEX8_COMPRESSED )
        {
            return false;
        }
        else if ( dstFormat == FORMAT_TEX32 )
        {
            return true;
        }
    }
    else if ( srcFormat == FORMAT_TEX32 )
    {
        if ( dstFormat == FORMAT_IDTEX4 ||
             dstFormat == FORMAT_IDTEX8 ||
             dstFormat == FORMAT_IDTEX8_COMPRESSED ||
             dstFormat == FORMAT_TEX16 )
        {
            return false;
        }
    }

    // If anything reaches this, then we have an unhandled situation.
    return false;
}

inline static bool getMemoryLayoutFromTexelFormat(eFormatEncodingType encodingType, eMemoryLayoutType& memLayout)
{
    eMemoryLayoutType theLayout;

    if ( encodingType == FORMAT_IDTEX4 || encodingType == FORMAT_IDTEX8_COMPRESSED )
    {
        theLayout = PSMT4;
    }
    else if ( encodingType == FORMAT_IDTEX8 )
    {
        theLayout = PSMT8;
    }
    else if ( encodingType == FORMAT_TEX16 )
    {
        theLayout = PSMCT16S;
    }
    else if ( encodingType == FORMAT_TEX32 )
    {
        theLayout = PSMCT32;
    }
    else
    {
        return false;
    }

    memLayout = theLayout;

    return true;
}

struct GIFtag
{
    union
    {
        struct
        {
            unsigned long long nloop : 15;
            unsigned long long eop : 1;
            unsigned long long pad1 : 30;
            unsigned long long pre : 1;
            unsigned long long prim : 11;
            unsigned long long flg : 2;
            unsigned long long nreg : 4;
        };
        unsigned long long props;
    };

    unsigned long long regs;

    uint32 getRegisterID(uint32 i) const
    {
        assert(i < 16);

        unsigned long long shiftPos = i * 4;

        return ( this->regs & ( 0xF << shiftPos ) ) >> shiftPos;
    }

    void setRegisterID(uint32 i, uint32 regContent)
    {
        assert(i < 16);

        unsigned long long shiftPos = i * 4;

        this->regs &= ~( 0xF << shiftPos );

        this->regs |= regContent >> shiftPos;
    }
};

struct GIFtag_serialized
{
    endian::little_endian <unsigned long long> props;
    endian::little_endian <unsigned long long> regs;

    inline operator GIFtag ( void ) const
    {
        GIFtag result;

        result.props = this->props;
        result.regs = this->regs;

        return result;
    }

    inline GIFtag_serialized& operator = ( const GIFtag& right )
    {
        this->props = right.props;
        this->regs = right.regs;

        return *this;
    }
};

struct regID_struct
{
    unsigned long long regID : 8;
    unsigned long long pad1 : 56;

    inline regID_struct( void )
    {
        // Do nothing.
    }

    inline regID_struct( unsigned long long item )
    {
        *(unsigned long long*)this = item;
    }

    inline void operator = ( unsigned long long right )
    {
        *(unsigned long long*)this = right;
    }

    inline unsigned long long toNumber( void ) const
    {
        return *(unsigned long long*)this;
    }
};

inline uint32 getFormatEncodingDepth(eFormatEncodingType encodingType)
{
    uint32 depth = 0;

    if (encodingType == FORMAT_IDTEX4 || encodingType == FORMAT_IDTEX8_COMPRESSED)
    {
        depth = 4;
    }
    else if (encodingType == FORMAT_IDTEX8)
    {
        depth = 8;
    }
    else if (encodingType == FORMAT_TEX16)
    {
        depth = 16;
    }
    else if (encodingType == FORMAT_TEX32)
    {
        depth = 32;
    }

    return depth;
}

struct NativeTexturePS2
{
    Interface *engineInterface;

    LibraryVersion texVersion;

    inline NativeTexturePS2( Interface *engineInterface )
    {
        // Initialize the texture object.
        this->engineInterface = engineInterface;
        this->texVersion = engineInterface->GetVersion();
        this->rasterFormat = RASTER_DEFAULT;
        this->depth = 0;
        this->paletteType = PALETTE_NONE;
        this->autoMipmaps = false;
        this->requiresHeaders = true;
        this->hasSwizzle = false;
        this->skyMipMapVal = 4032;
        this->recommendedBufferBasePointer = 0;
        this->swizzleEncodingType = FORMAT_UNKNOWN;
        this->paletteSwizzleEncodingType = FORMAT_UNKNOWN;

        // Set default values for PS2 GS parameters.
        gsParams.maxMIPLevel = 7;
        gsParams.mtba = 0;
        gsParams.textureFunction = 0;       // MODULATE
        gsParams.lodCalculationModel = 0;   // LOD using formula
        gsParams.mmag = 0;                  // NEAREST
        gsParams.mmin = 0;                  // NEAREST
        gsParams.lodParamL = 0;
        gsParams.lodParamK = 0;

        // Whatever those are.
        gsParams.gsTEX1Unknown1 = 0;
        gsParams.gsTEX1Unknown2 = 0;

        // Texture raster by default.
        this->rasterType = 4;

        this->hasAlpha = true;
        this->colorOrdering = COLOR_RGBA;   // PlayStation 2 textures are always RGBA ordered.
    }

    inline NativeTexturePS2( const NativeTexturePS2& right )
    {
        Interface *engineInterface = right.engineInterface;

        this->engineInterface = engineInterface;
        this->texVersion = right.texVersion;

        // Copy palette information.
        this->paletteTex.CopyTexture( engineInterface, right.paletteTex );
        this->paletteType = right.paletteType;

        // Copy image texel information.
        this->rasterFormat = right.rasterFormat;
        this->depth = right.depth;

        {
            size_t mipmapCount = right.mipmaps.size();

            this->mipmaps.resize( mipmapCount );

            for ( size_t n = 0; n < mipmapCount; n++ )
            {
                GSMipmap& thisLayer = this->mipmaps[ n ];

                const GSMipmap& srcLayer = right.mipmaps[ n ];

                thisLayer.CopyMipmap( engineInterface, srcLayer );
            }
        }
        
        // Copy PS2 data.
        this->autoMipmaps = right.autoMipmaps;
        this->requiresHeaders = right.requiresHeaders;
        this->hasSwizzle = right.hasSwizzle;
        this->skyMipMapVal = right.skyMipMapVal;
        this->gsParams = right.gsParams;
        this->recommendedBufferBasePointer = right.recommendedBufferBasePointer;
        this->swizzleEncodingType = right.swizzleEncodingType;
        this->paletteSwizzleEncodingType = right.paletteSwizzleEncodingType;

        this->rasterType = right.rasterType;

        this->hasAlpha = right.hasAlpha;
        this->colorOrdering = right.colorOrdering;
    }

    inline void clearImageData( void )
    {
        Interface *engineInterface = this->engineInterface;

        // Free all mipmaps.
        size_t mipmapCount = this->mipmaps.size();

        for ( size_t n = 0; n < mipmapCount; n++ )
        {
            GSMipmap& mipLayer = this->mipmaps[ n ];

            mipLayer.FreeTexels( engineInterface );
        }

        // Free the palette texture.
        this->paletteTex.FreeTexels( engineInterface );
    }

    inline ~NativeTexturePS2( void )
    {
        this->clearImageData();
    }

    struct GSTexture
    {
        inline GSTexture( void )
        {
            this->dataSize = 0;
            this->texels = NULL;

            this->swizzleWidth = 0;
            this->swizzleHeight = 0;
        }

        inline GSTexture( const GSTexture& right )
        {
            *this = right;
        }

        inline void FreeTexels( Interface *engineInterface )
        {
            if ( void *texels = this->texels )
            {
                engineInterface->PixelFree( texels );

                this->texels = NULL;
            }
        }

        inline void DetachTexels( void )
        {
            this->texels = NULL;
        }

    public:
        inline void CopyTexture( Interface *engineInterface, const GSTexture& right )
        {
            // Copy over image data.
            void *newTexels = NULL;

            uint32 dataSize = right.dataSize;

            if ( dataSize != 0 )
            {
                const void *srcTexels = right.texels;

                newTexels = engineInterface->PixelAllocate( dataSize );

                memcpy( newTexels, srcTexels, dataSize );
            }
            this->texels = newTexels;
            this->dataSize = dataSize;

            // Copy registers.
            this->storedRegs = right.storedRegs;

            // Copy over encoding properties.
            this->swizzleWidth = right.swizzleWidth;
            this->swizzleHeight = right.swizzleHeight;
        }

        inline void setGSRegister(eGSRegister regID, unsigned long long regContent)
        {
            // Try to find an existing entry with this register.
            size_t numRegs = this->storedRegs.size();

            bool hasReplacedReg = false;

            for ( size_t n = 0; n < numRegs; n++ )
            {
                GSRegInfo& regInfo = this->storedRegs[ n ];

                if ( regInfo.regID == regID )
                {
                    regInfo.content = regContent;
                    
                    hasReplacedReg = true;
                    break;
                }
            }

            if ( !hasReplacedReg )
            {
                GSRegInfo newRegInfo;
                newRegInfo.regID = regID;
                newRegInfo.content = regContent;

                this->storedRegs.push_back( newRegInfo );
            }
        }

        uint32 getDataSize( eFormatEncodingType swizzleEncodingType ) const
        {
            // Since the texture dimension are power of two, this is actually correct.
            // The PlayStation 2 does not use the row alignment concept anyway.
            // Instead it has a special memory pattern that must be upkept.
            uint32 encodedTexItems = ( this->swizzleWidth * this->swizzleHeight );

            uint32 encodingDepth = getFormatEncodingDepth(swizzleEncodingType);

			return ( ALIGN_SIZE( encodedTexItems * encodingDepth, 8u ) / 8 );
        }

        uint32 getStreamSize( bool requiresHeaders ) const
        {
            uint32 streamSize = 0;

            if ( requiresHeaders )
            {
                streamSize += (uint32)(
                    sizeof(GIFtag_serialized) +
                    this->storedRegs.size() * ( sizeof(uint64) * 2 ) +
                    sizeof(GIFtag_serialized)
                );
            }

            streamSize += this->dataSize;

            return streamSize;
        }

        uint32 readGIFPacket(Interface *engineInterface, BlockProvider& inputProvider, bool hasHeaders, bool& corruptedHeaders_out);
        uint32 writeGIFPacket(Interface *engineInterface, BlockProvider& outputProvider, bool requiresHeaders) const;

        // Members.
        uint32 dataSize;
        void *texels;           // holds either indices or color values

        uint32 swizzleWidth, swizzleHeight;

        struct GSRegInfo
        {
            eGSRegister regID;
            unsigned long long content;
        };

        typedef std::vector <GSRegInfo> regInfoList_t;

        regInfoList_t storedRegs;
    };

    struct GSMipmap : public GSTexture
    {
        inline GSMipmap( void )
        {
            this->width = 0;
            this->height = 0;
        }

        inline ~GSMipmap( void )
        {
            return;
        }

        inline GSMipmap( const GSMipmap& right )
        {
            *this = right;
        }

    public:
        inline void CopyMipmap( Interface *engineInterface, const GSMipmap& right )
        {
            // Copy general attributes.
            this->width = right.width;
            this->height = right.height;

            // Copy texture stuff.
            this->CopyTexture( engineInterface, right );
        }

        uint32 width, height;   // store width & height for each mipmap
    };

    // mipmaps are GSTextures.
    std::vector <GSMipmap> mipmaps;

    eRasterFormat rasterFormat;

    uint32 depth;

    GSTexture paletteTex;

    ePaletteType paletteType;

    uint32 recommendedBufferBasePointer;

    // Encoding type of all mipmaps.
    eFormatEncodingType swizzleEncodingType;
    eFormatEncodingType paletteSwizzleEncodingType;

    bool requiresHeaders;
    bool hasSwizzle;
    bool autoMipmaps;

    bool hasAlpha;

    uint32 skyMipMapVal;

    uint8 rasterType;

    eColorOrdering colorOrdering;

    struct gsParams_t
    {
        // Unique PS2 configuration.
        uint8 maxMIPLevel;
        uint8 mtba;
        uint8 textureFunction;
        uint8 lodCalculationModel;
        uint8 mmag;
        uint8 mmin;
        uint8 lodParamL;
        uint8 lodParamK;
        
        // Some undocumented PS2 flags.
        uint8 gsTEX1Unknown1;
        uint8 gsTEX1Unknown2;
    };
    gsParams_t gsParams;

    eFormatEncodingType getHardwareRequiredEncoding(LibraryVersion version) const;

private:
    bool allocateTextureMemoryNative(
        uint32 mipmapBasePointer[], uint32 mipmapBufferWidth[], uint32 mipmapMemorySize[], ps2MipmapTransmissionData mipmapTransData[], uint32 maxMipmaps,
        eMemoryLayoutType& pixelMemLayoutTypeOut,
        uint32& clutBasePointer, uint32& clutMemSize, ps2MipmapTransmissionData& clutTransData,
        uint32& maxBuffHeight
    ) const;

public:
    bool allocateTextureMemory(
        uint32 mipmapBasePointer[], uint32 mipmapBufferWidth[], uint32 mipmapMemorySize[], ps2MipmapTransmissionData mipmapTransData[], uint32 maxMipmaps,
        eMemoryLayoutType& pixelMemLayoutTypeOut,
        uint32& clutBasePointer, uint32& clutMemSize, ps2MipmapTransmissionData& clutTransData
    ) const;

    uint32 calculateGPUDataSize(
        const uint32 mipmapBasePointer[], const uint32 mipmapMemorySize[], uint32 maxMipmaps,
        eMemoryLayoutType memLayoutType,
        uint32 clutBasePointer, uint32 clutMemSize
    ) const;

    bool generatePS2GPUData(
        LibraryVersion gameVersion,
        ps2GSRegisters& gpuData,
        const uint32 mipmapBasePointer[], const uint32 mipmapBufferWidth[], const uint32 mipmapMemorySize[], uint32 maxMipmaps,
        eMemoryLayoutType memLayoutType,
        uint32 clutBasePointer
    ) const;

    void getOptimalGSParameters(gsParams_t& paramsOut) const;

    // Call this function whenever special properties of the PS2 texture have changed.
    void UpdateStructure( Interface *engineInterface );

    bool getDebugBitmap( Bitmap& bmpOut ) const;
};

inline void getPS2NativeTextureSizeRules( nativeTextureSizeRules& rulesOut )
{
    rulesOut.powerOfTwo = true;
    rulesOut.squared = false;
}

struct ps2NativeTextureTypeProvider : public texNativeTypeProvider
{
    void ConstructTexture( Interface *engineInterface, void *objMem, size_t memSize )
    {
        new (objMem) NativeTexturePS2( engineInterface );
    }

    void CopyConstructTexture( Interface *engineInterface, void *objMem, const void *srcObjMem, size_t memSize )
    {
        new (objMem) NativeTexturePS2( *(const NativeTexturePS2*)srcObjMem );
    }
    
    void DestroyTexture( Interface *engineInterface, void *objMem, size_t memSize )
    {
        ( *(NativeTexturePS2*)objMem ).~NativeTexturePS2();
    }

    eTexNativeCompatibility IsCompatibleTextureBlock( BlockProvider& inputProvider ) const;

    void SerializeTexture( TextureBase *theTexture, PlatformTexture *nativeTex, BlockProvider& outputProvider ) const;
    void DeserializeTexture( TextureBase *theTexture, PlatformTexture *nativeTex, BlockProvider& inputProvider ) const;

    void GetPixelCapabilities( pixelCapabilities& capsOut ) const
    {
        capsOut.supportsDXT1 = false;
        capsOut.supportsDXT2 = false;
        capsOut.supportsDXT3 = false;
        capsOut.supportsDXT4 = false;
        capsOut.supportsDXT5 = false;
        capsOut.supportsPalette = true;
    }

    void GetStorageCapabilities( storageCapabilities& storeCaps ) const
    {
        storeCaps.pixelCaps.supportsDXT1 = false;
        storeCaps.pixelCaps.supportsDXT2 = false;
        storeCaps.pixelCaps.supportsDXT3 = false;
        storeCaps.pixelCaps.supportsDXT4 = false;
        storeCaps.pixelCaps.supportsDXT5 = false;
        storeCaps.pixelCaps.supportsPalette = true;

        storeCaps.isCompressedFormat = false;
    }

    void GetPixelDataFromTexture( Interface *engineInterface, void *objMem, pixelDataTraversal& pixelsOut );
    void SetPixelDataToTexture( Interface *engineInterface, void *objMem, const pixelDataTraversal& pixelsIn, acquireFeedback_t& feedbackOut );
    void UnsetPixelDataFromTexture( Interface *engineInterface, void *objMem, bool deallocate );

    void SetTextureVersion( Interface *engineInterface, void *objMem, LibraryVersion version )
    {
        // TODO: track version changes and update contents.
        // the swizzling changes depending on library version.

        NativeTexturePS2 *nativeTex = (NativeTexturePS2*)objMem;

        nativeTex->texVersion = version;

        nativeTex->UpdateStructure( engineInterface );
    }

    LibraryVersion GetTextureVersion( const void *objMem )
    {
        const NativeTexturePS2 *nativeTex = (const NativeTexturePS2*)objMem;

        return nativeTex->texVersion;
    }

    bool GetMipmapLayer( Interface *engineInterface, void *objMem, uint32 mipIndex, rawMipmapLayer& layerOut );
    bool AddMipmapLayer( Interface *engineInterface, void *objMem, const rawMipmapLayer& layerIn, acquireFeedback_t& feedbackOut );
    void ClearMipmaps( Interface *engineInterface, void *objMem );

    void GetTextureInfo( Interface *engineInterface, void *objMem, nativeTextureBatchedInfo& infoOut );
    void GetTextureFormatString( Interface *engineInterface, void *objMem, char *buf, size_t bufLen, size_t& lengthOut ) const;

    eRasterFormat GetTextureRasterFormat( const void *objMem ) override
    {
        const NativeTexturePS2 *nativeTex = (const NativeTexturePS2*)objMem;

        return nativeTex->rasterFormat;
    }

    ePaletteType GetTexturePaletteType( const void *objMem ) override
    {
        const NativeTexturePS2 *nativeTex = (const NativeTexturePS2*)objMem;

        return nativeTex->paletteType;
    }

    bool IsTextureCompressed( const void *objMem ) override
    {
        return false;
    }

    eCompressionType GetTextureCompressionFormat( const void *objMem ) override
    {
        return RWCOMPRESS_NONE;
    }

    bool DoesTextureHaveAlpha( const void *objMem ) override
    {
        // TODO: this needs a rewrite. PS2 textures do not cache the alpha flag.

        const NativeTexturePS2 *nativeTex = (const NativeTexturePS2*)objMem;

        return nativeTex->hasAlpha;
    }

    uint32 GetTextureDataRowAlignment( void ) const override
    {
        // This is kind of a tricky one. I believe that PlayStation 2 native textures do not use
        // any row alignment. I could be wrong tho. We are safe if we decide for 4 byte alignment.
        // Just report back to us if there is any issue. :-)
        return 4;
    }

    void GetTextureSizeRules( const void *objMem, nativeTextureSizeRules& rulesOut ) const override
    {
        // The PlayStation 2 native texture does not change size rules, thankfully.
        getPS2NativeTextureSizeRules( rulesOut );
    }

    uint32 GetDriverIdentifier( void *objMem ) const override
    {
        // Always the generic PlayStation 2 driver.
        return 6;
    }

    inline void Initialize( Interface *engineInterface )
    {
        RegisterNativeTextureType( engineInterface, "PlayStation2", this, sizeof( NativeTexturePS2 ) );
    }

    inline void Shutdown( Interface *engineInterface )
    {
        UnregisterNativeTextureType( engineInterface, "PlayStation2" );
    }
};

inline eFormatEncodingType getFormatEncodingFromRasterFormat(eRasterFormat rasterFormat, ePaletteType paletteType)
{
    eFormatEncodingType encodingFormat = FORMAT_UNKNOWN;

    if ( paletteType != PALETTE_NONE )
    {
        if (paletteType == PALETTE_4BIT)
        {
            encodingFormat = FORMAT_IDTEX8_COMPRESSED;
        }
        else if (paletteType == PALETTE_8BIT)
        {
            encodingFormat = FORMAT_IDTEX8;
        }
        else
        {
            throw RwException( "invalid palette type in PS2 swizzle format detection" );
        }
    }
    else
    {
        if (rasterFormat == RASTER_LUM)
        {
            // We assume that we are 8bit LUM here.
            encodingFormat = FORMAT_IDTEX8;
        }
        else if (rasterFormat == RASTER_1555 || rasterFormat == RASTER_565 || rasterFormat == RASTER_4444 ||
                 rasterFormat == RASTER_16 || rasterFormat == RASTER_555)
        {
            encodingFormat = FORMAT_TEX16;
        }
        else if (rasterFormat == RASTER_8888 || rasterFormat == RASTER_888 || rasterFormat == RASTER_32)
        {
            encodingFormat = FORMAT_TEX32;
        }
    }

    return encodingFormat;
}

inline eFormatEncodingType getFormatEncodingFromMemoryLayout(eMemoryLayoutType memLayout)
{
    eFormatEncodingType encodingFormat = FORMAT_UNKNOWN;

    if (memLayout == PSMT4)
    {
        encodingFormat = FORMAT_IDTEX8_COMPRESSED;
    }
    else if (memLayout == PSMT8)
    {
        encodingFormat = FORMAT_IDTEX8;
    }
    else if (memLayout == PSMCT16 || memLayout == PSMCT16S)
    {
        encodingFormat = FORMAT_TEX16;
    }
    else if (memLayout == PSMCT32)
    {
        encodingFormat = FORMAT_TEX32;
    }

    return encodingFormat;
}

inline void getPaletteTextureDimensions(ePaletteType paletteType, LibraryVersion version, uint32& width, uint32& height)
{
    if (paletteType == PALETTE_4BIT)
    {
        if (version.rwLibMinor <= 1)
        {
            width = 8;
            height = 2;
        }
        else
        {
            width = 8;
            height = 3;
        }
    }
    else if (paletteType == PALETTE_8BIT)
    {
        width = 16;
        height = 16;
    }
    else
    {
        assert( 0 );
    }
}

inline void genpalettetexeldata(
    LibraryVersion gameVersion,
    void *paletteData, eRasterFormat rasterFormat, ePaletteType paletteType, uint32 itemCount,
    void*& texelData, uint32& texelDataSize, uint32& palWidth, uint32& palHeight
)
{
    // Select a compatible texture size for our palette data.
    uint32 texelWidth = 0;
    uint32 texelHeight = 0;

    getPaletteTextureDimensions(paletteType, gameVersion, texelWidth, texelHeight);

    // Allocate texture memory.
    uint32 texelItemCount = ( texelWidth * texelHeight );

    // Calculate the data size.
    uint32 palDepth = Bitmap::getRasterFormatDepth(rasterFormat);

    assert( itemCount != 0 );
    assert( texelItemCount != 0 );

    uint32 srcDataSize = getPaletteDataSize( itemCount, palDepth );
    uint32 dstDataSize = getPaletteDataSize( texelItemCount, palDepth );

    assert( srcDataSize != 0 );
    assert( dstDataSize != 0 );

    void *newTexelData = NULL;

    if ( srcDataSize != dstDataSize )
    {
        newTexelData = new uint8[ dstDataSize ];

        // Write the new memory.
        memcpy(newTexelData, paletteData, std::min(srcDataSize, dstDataSize));
        
        if (dstDataSize > srcDataSize)
        {
            // Zero out the rest.
            memset((char*)newTexelData + srcDataSize, 0, (dstDataSize - srcDataSize));
        }
    }
    else
    {
        newTexelData = paletteData;
    }

    // Give parameters to the runtime.
    texelData = newTexelData;
    texelDataSize = dstDataSize;
    
    palWidth = texelWidth;
    palHeight = texelHeight;
}

}