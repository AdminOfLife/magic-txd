// Virtual image importing helper for a standardized way to load images.

#pragma once

enum eImportExpectation
{
    IMPORTE_IMAGE,
    IMPORTE_TEXCHUNK
};

// Image import method manager.
struct imageImportMethods abstract
{
    struct loadActionResult
    {
        rw::Raster *texRaster;
        rw::TextureBase *texHandle; // NULL if load-result does not include texture handle
    };

    inline imageImportMethods( void )
    {
        // Register all import methods.
        // Those represent the kind of texture data that is accepted by Magic.TXD!
        this->RegisterImportMethod( "image", &imageImportMethods::impMeth_loadImage, IMPORTE_IMAGE );
        this->RegisterImportMethod( "tex chunks", &imageImportMethods::impMeth_loadTexChunk, IMPORTE_TEXCHUNK );
    }

    bool LoadImage( rw::Stream *stream, eImportExpectation img_exp, loadActionResult& action_result ) const;

    typedef bool (imageImportMethods::* importMethod_t)( rw::Stream *stream, loadActionResult& action_result ) const;

    void RegisterImportMethod( const char *name, importMethod_t meth, eImportExpectation expImp );

protected:
    virtual void OnWarning( std::string&& msg ) const = 0;
    virtual void OnError( std::string&& msg ) const = 0;

    // We need to be able to create a special raster.
    virtual rw::Raster* MakeRaster( void ) const = 0;

    // Default image loading methods.
    bool impMeth_loadImage( rw::Stream *stream, loadActionResult& action_result ) const;
    bool impMeth_loadTexChunk( rw::Stream *stream, loadActionResult& action_result ) const;

private:
    struct meth_reg
    {
        eImportExpectation img_exp;
        importMethod_t cb;
        const char *name;
    };

    std::vector <meth_reg> methods;
};