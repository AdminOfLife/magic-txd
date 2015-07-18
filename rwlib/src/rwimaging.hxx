// Internal header for the imaging components and environment.
namespace rw
{

struct imagingLayerTraversal
{
    // We do want to support compression traversal with this struct.
    // That is why this looks like a mipmap layer, but not really.
    uint32 layerWidth, layerHeight;
    uint32 mipWidth, mipHeight;
    void *texelSource;  // always newly allocated.
    uint32 dataSize;

    eRasterFormat rasterFormat;
    uint32 depth;
    uint32 rowAlignment;
    eColorOrdering colorOrder;
    ePaletteType paletteType;
    void *paletteData;
    uint32 paletteSize;
    eCompressionType compressionType;
    
    bool hasAlpha;  // only valid for deserialization, if capabilities say so.
};

// Interface for various image formats that this library should support.
struct imagingFormatExtension abstract
{
    // Method to verify whether a stream should be compatible with this format.
    // This method does not have to completely verify the integrity of the stream, so that deserialization may fail anyway.
    virtual bool IsStreamCompatible( Interface *engineInterface, Stream *inputStream ) const = 0;

    // Method that reports storage capabilities of this format.
    // What it can store is directly what it can take, nothing more.
    virtual void GetStorageCapabilities( pixelCapabilities& capsOut ) const = 0;

    // Pull and fetch methods.
    virtual void DeserializeImage( Interface *engineInterface, Stream *inputStream, imagingLayerTraversal& outputPixels ) const = 0;
    virtual void SerializeImage( Interface *engineInterface, Stream *outputStream, const imagingLayerTraversal& inputPixels ) const = 0;
};

// Function to register new imaging formats.
bool RegisterImagingFormat( Interface *engineInterface, const char *formatName, const char *defaultExt, imagingFormatExtension *intf );
bool UnregisterImagingFormat( Interface *engineInterface, imagingFormatExtension *intf );

// Helper functions.
inline void checkAhead( Stream *stream, int64 count )
{
    // Check whether we have those bytes.
    int64 curPos = stream->tell();
    int64 streamSize = stream->size();

    int64 availableBytes = ( streamSize - curPos );

    if ( availableBytes < count )
    {
        throw RwException( "stream does not have required bytes" );
    }
}

inline void putc_stream( rw::Stream *theStream, char val, size_t count )
{
    for ( size_t n = 0; n < count; n++ )
    {
        theStream->write( &val, 1 );
    }
}

inline void skipAvailable( Stream *stream, int64 skipCount )
{
    // Check availability.
    checkAhead( stream, skipCount );

    // We are okay. Just skip ahead.
    stream->skip( skipCount );
}

}