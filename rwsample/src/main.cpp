#include <renderware.h>

// These structs are standardized.
// You can safely use them in streams!
#define DEG2RAD(x)  ( M_PI * x / 180 )
#define RAD2DEG(x)  ( x / M_PI * 180 )

using namespace rw;

template <typename numberType, size_t dimm>
struct SquareMatrix
{
    typedef Vector <numberType, dimm> vec_t;

    AINLINE SquareMatrix( void )
    {
        for ( size_t y = 0; y < dimm; y++ )
        {
            for ( size_t x = 0; x < dimm; x++ )
            {
                this->vecs[y][x] = (numberType)( x == y ? 1 : 0 );
            }
        }
    }
    AINLINE SquareMatrix( const SquareMatrix& right )
    {
        for ( size_t n = 0; n < dimm; n++ )
        {
            this->vecs[ n ] = right.vecs[ n ];
        }
    }
    AINLINE SquareMatrix( SquareMatrix&& right )
    {
        for ( size_t n = 0; n < dimm; n++ )
        {
            this->vecs[ n ] = std::move( right.vecs[ n ] );
        }
    }

    AINLINE SquareMatrix( const Vector <vec_t, dimm>& mat )
    {
        for ( size_t y = 0; y < dimm; y++ )
        {
            this->vecs[ y ] = mat[ y ];
        }
    }

    AINLINE SquareMatrix ( Vector <vec_t, dimm>&& mat )
    {
        for ( size_t y = 0; y < dimm; y++ )
        {
            this->vecs[ y ] = std::move( mat.elems[ y ] );
        }
    }

    AINLINE SquareMatrix( std::initializer_list <vec_t> items )
    {
        size_t y = 0;

        for ( vec_t vec : items )
        {
            this->vecs[ y ] = vec;

            y++;
        }
    }

    AINLINE SquareMatrix& operator = ( const SquareMatrix& right )
    {
        for ( size_t n = 0; n < dimm; n++ )
        {
            this->vecs[ n ] = right.vecs[ n ];
        }

        return *this;
    }
    AINLINE SquareMatrix& operator = ( SquareMatrix&& right )
    {
        for ( size_t n = 0; n < dimm; n++ )
        {
            this->vecs[ n ] = std::move( right.vecs[ n ] );
        }

        return *this;
    }

    // Addition.
    AINLINE SquareMatrix operator + ( const SquareMatrix& right ) const
    {
        SquareMatrix newMat = *this;

        for ( size_t y = 0; y < dimm; y++ )
        {
            newMat.vecs[ y ] += right.vecs[ y ];
        }

        return newMat;
    }

    AINLINE SquareMatrix& operator += ( const SquareMatrix& right )
    {
        for ( size_t y = 0; y < dimm; y++ )
        {
            this->vecs[ y ] += right.vecs[ y ];
        }

        return *this;
    }

    // By scalar.
    AINLINE SquareMatrix operator + ( const numberType& val ) const
    {
        SquareMatrix newMat = *this;

        for ( size_t y = 0; y < dimm; y++ )
        {
            newMat.vecs[ y ] += val;
        }

        return newMat;
    }

    AINLINE SquareMatrix& operator += ( const numberType& val )
    {
        for ( size_t y = 0; y < dimm; y++ )
        {
            this->vecs[ y ] += val;
        }

        return *this;
    }

    // Subtraction.
    AINLINE SquareMatrix operator - ( const SquareMatrix& right ) const
    {
        SquareMatrix newMat = *this;

        for ( size_t y = 0; y < dimm; y++ )
        {
            newMat.vecs[ y ] -= right.vecs[ y ];
        }

        return newMat;
    }

    AINLINE SquareMatrix& operator -= ( const SquareMatrix& right )
    {
        for ( size_t y = 0; y < dimm; y++ )
        {
            this->vecs[ y ] -= right.vecs[ y ];
        }

        return *this;
    }

    // By scalar.
    AINLINE SquareMatrix operator - ( const numberType& val ) const
    {
        SquareMatrix newMat = *this;

        for ( size_t y = 0; y < dimm; y++ )
        {
            newMat.vecs[ y ] -= val;
        }

        return newMat;
    }

    AINLINE SquareMatrix& operator -= ( const numberType& val )
    {
        for ( size_t y = 0; y < dimm; y++ )
        {
            this->vecs[ y ] -= val;
        }

        return *this;
    }

    // Multiplication by scalar.
    AINLINE SquareMatrix operator * ( const numberType& val ) const
    {
        SquareMatrix newMat = *this;

        for ( size_t y = 0; y < dimm; y++ )
        {
            newMat.vecs[ y ] *= val;
        }

        return newMat;
    }

    AINLINE SquareMatrix& operator *= ( const numberType& val )
    {
        for ( size_t y = 0; y < dimm; y++ )
        {
            this->vecs[ y ] *= val;
        }

        return *this;
    }

    // Division by scalar.
    AINLINE SquareMatrix operator / ( const numberType& val ) const
    {
        SquareMatrix newMat = *this;

        for ( size_t y = 0; y < dimm; y++ )
        {
            newMat.vecs[ y ] /= val;
        }

        return newMat;
    }

    AINLINE SquareMatrix& operator /= ( const numberType& val )
    {
        for ( size_t y = 0; y < dimm; y++ )
        {
            this->vecs[ y ] /= val;
        }

        return *this;
    }

private:
    AINLINE static void multiplyWith( SquareMatrix& newMat, const SquareMatrix& left, const SquareMatrix& right )
    {
        for ( size_t y_pos = 0; y_pos < dimm; y_pos++ )
        {
            for ( size_t x_pos = 0; x_pos < dimm; x_pos++ )
            {
                // Cross product of row and column vectors.
                numberType val = numberType();

                for ( size_t dimiter = 0; dimiter < dimm; dimiter++ )
                {
                    val += ( left[y_pos][dimiter] * right[dimiter][x_pos] );
                }

                newMat[ y_pos ][ x_pos ] = val;
            }
        }
    }

public:
    // Multiply matrices.
    AINLINE SquareMatrix operator * ( const SquareMatrix& right ) const
    {
        SquareMatrix newMat;

        multiplyWith( newMat, *this, right );

        return newMat;
    };

    AINLINE SquareMatrix& operator *= ( const SquareMatrix& right )
    {
        SquareMatrix oldmat = *this;

        multiplyWith( *this, oldmat, right );

        return *this;
    }

    // Transforming vectors.
    AINLINE vec_t operator * ( const vec_t& right ) const
    {
        vec_t newVec;

        for ( size_t n = 0; n < dimm; n++ )
        {
            newVec += ( this->vecs[ n ] * right[ n ] );
        }

        return newVec;
    }

    AINLINE vec_t operator * ( vec_t&& right ) const
    {
        vec_t newVec;

        for ( size_t n = 0; n < dimm; n++ )
        {
            newVec += ( this->vecs[ n ] * right[ n ] );
        }

        return newVec;
    }

    AINLINE vec_t& operator [] ( ptrdiff_t n )
    {
        return vecs[ n ];
    }

    AINLINE const vec_t& operator [] ( ptrdiff_t n ) const
    {
        return vecs[ n ];
    }

    // Matrix inversion.
    AINLINE numberType det( void ) const
    {
        using namespace rw::mview;

        return detcalc <numberType, SquareMatrix, dimm>::calc( *this );
    }

    AINLINE bool Invert( void )
    {
        double mydet = det();

        if ( mydet == 0 )
            return false;

        for ( size_t y = 0; y < dimm; y++ )
        {
            for ( size_t x = 0; x < dimm; x++ )
            {
                this->vecs[y][x] /= mydet;
            }
        }

        return true;
    }

#ifdef _SUPPORTS_CONSTEXPR
    // Euler trait.
    // SFINAE, bitch! It is a very ridiculous concept, but very great to have :)
    static constexpr bool canHaveEuler =
        ( typename std::is_same <float, numberType>::value || typename std::is_same <double, numberType>::value ) &&
        ( dimm == 3 || dimm == 4 );

    template <typename = std::enable_if <canHaveEuler>::type>
    inline void SetRotationRad( numberType x, numberType y, numberType z )
    {
        numberType ch = (numberType)cos( x );
        numberType sh = (numberType)sin( x );
        numberType cb = (numberType)cos( y );
        numberType sb = (numberType)sin( y );
        numberType ca = (numberType)cos( z );
        numberType sa = (numberType)sin( z );

        vec_t& vRight = this->vecs[0];
        vec_t& vFront = this->vecs[1];
        vec_t& vUp = this->vecs[2];

        vRight[0] = (numberType)( ca * cb );
        vRight[1] = (numberType)( -sa * cb );
        vRight[2] = (numberType)( sb );

        vFront[0] = (numberType)( ca * sb * sh + sa * ch );
        vFront[1] = (numberType)( ca * ch - sa * sb * sh );
        vFront[2] = (numberType)( -sh * cb );

        vUp[0] = (numberType)( sa * sh - ca * sb * ch );
        vUp[1] = (numberType)( sa * sb * ch + ca * sh );
        vUp[2] = (numberType)( ch * cb );
    }

    template <typename = std::enable_if <!canHaveEuler>::type>
    inline void SetRotationRad( numberType x, numberType y, numberType z, int a = 0 )
    {
        static_assert( false, "does not support euler rotation" );
    }

    template <typename = std::enable_if <canHaveEuler>::type>
    inline void SetRotation( numberType x, numberType y, numberType z )
    {
        SetRotationRad( (numberType)DEG2RAD( x ), (numberType)DEG2RAD( y ), (numberType)DEG2RAD( z ) );
    }

    template <typename = std::enable_if <!canHaveEuler>::type>
    inline void SetRotation( numberType x, numberType y, numberType z, int a = 0 )
    {
        static_assert( false, "does not support euler rotation" );
    }

    template <typename = std::enable_if <canHaveEuler>::type>
    inline void GetRotationRad( numberType& x, numberType& y, numberType& z ) const
    {
        const vec_t& vRight = this->vecs[0];
        const vec_t& vFront = this->vecs[1];
        const vec_t& vUp = this->vecs[2];

        if ( vRight[2] == 1 )
        {
            y = (numberType)( M_PI / 2 );

            x = 0;
            z = (numberType)atan2( vRight[0], vRight[1] );
        }
        else if ( vRight[2] == -1 )
        {
            y = -(numberType)( M_PI / 2 );

            x = -0;
            z = (numberType)atan2( vRight[0], vRight[1] );
        }
        else
        {
            y = asin( vRight[2] );

            x = (numberType)atan2( -vFront[2], vUp[2] );
            z = (numberType)atan2( -vRight[1], vRight[0] );
        }
    }

    template <typename = std::enable_if <!canHaveEuler>::type>
    inline void GetRotationRad( numberType& x, numberType& y, numberType& z, int a = 0 ) const
    {
        static_assert( false, "does not support euler rotation" );
    }

    template <typename = std::enable_if <canHaveEuler>::type>
    inline void GetRotation( numberType& x, numberType& y, numberType& z ) const
    {
        GetRotationRad( x, y, z );

        x = (numberType)RAD2DEG( x );
        y = (numberType)RAD2DEG( y );
        z = (numberType)RAD2DEG( z );
    }

    template <typename = std::enable_if <!canHaveEuler>::type>
    inline void GetRotation( numberType& x, numberType& y, numberType& z, int a = 0 ) const
    {
        static_assert( false, "does not support euler rotation" );
    }
#endif //_SUPPORTS_CONSTEXPR

private:
    vec_t vecs[ dimm ];
};

template <typename ...Args, typename valType = std::tuple_element <0, std::tuple <Args...>>::type>
AINLINE Vector <valType, sizeof...(Args)> makevec( Args... theArgs )
{
    Vector <valType, sizeof...(Args)> theVec;

    complex_assign_helper <valType, sizeof...(Args)>::
        constr_helper <0, Args...>::
            ConstrVecElem( &theVec[ 0 ], std::forward <Args> ( theArgs )... );

    return theVec;
}

__declspec(dllexport) double mydet;

namespace rw
{
    LibraryVersion app_version( void )
    {
        LibraryVersion ver;

        ver.rwLibMajor = 3;
        ver.rwLibMinor = 7;
        ver.rwRevMajor = 0;
        ver.rwRevMinor = 0;

        return ver;
    }

    static void window_closing_event_handler( RwObject *obj, event_t eventID, void *callbackData, void *ud )
    {
        // We request destruction of the window.
        obj->engineInterface->DeleteRwObject( obj );
    }

    int32 rwmain( Interface *engineInterface )
    {
        using namespace mview;

#if 0
        typedef SquareMatrix <float, 7> mat_t;

        mat_t myMat =
        {
            { 3, 7, 2, 8, 3, 5, 8 },
            { 1, 6, 2, 7, 9, 2, 3 },
            { 2, 6, 5, 3, 5, 4, 5 },
            { 9, 6, 1, 4, 1, 7, 2 },
            { 4, 8, 4, 2, 8, 1, 9 },
            { 4, 2, 6, 4, 3, 1, 1 },
            { 5, 2, 7, 4, 8, 3, 5 }
        };

        // det: -543876
#else
#if 0
        typedef SquareMatrix <float, 6> mat_t;

        mat_t myMat =
        {
            { 3, 7, 2, 8, 3, 5 },
            { 1, 6, 2, 7, 9, 2 },
            { 2, 6, 5, 3, 5, 4 },
            { 9, 6, 1, 4, 1, 7 },
            { 4, 8, 4, 2, 8, 1 },
            { 4, 2, 6, 4, 3, 1 }
        };

        // det: -43512
#else
        typedef SquareMatrix <double, 5> mat_t;

        mat_t myMat =
        {
            { 3, 7, 2, 8, 3 },
            { 1, 6, 2, 7, 9 },
            { 2, 6, 5, 3, 5 },
            { 9, 6, 1, 4, 1 },
            { 4, 8, 4, 2, 8 }
        };

        // det: 3790
#endif
#endif

        //double myDet = myMat.det();

        mat_t cloneMat = myMat;

        //cloneMat.Invert();

        mat_t identity = cloneMat * myMat;

        //::mydet = myDet;

        // Give information about the running application to the runtime.
        softwareMetaInfo metaInfo;
        metaInfo.applicationName = "RenderWare Sample";
        metaInfo.applicationVersion = "test";
        metaInfo.description = "A test application for the rwtools runtime";

        engineInterface->SetApplicationInfo( metaInfo );

        // Create a window and render into it.
        Window *rwWindow = MakeWindow( engineInterface, 640, 480 );

        if ( !rwWindow )
            return -1;

        // We hold an extra reference.
        AcquireObject( rwWindow );

        // Handle the window closing event.
        RegisterEventHandler( rwWindow, event_t::WINDOW_CLOSING, window_closing_event_handler );
        RegisterEventHandler( rwWindow, event_t::WINDOW_QUIT, window_closing_event_handler );

        // Show the window, since we have set it up by now.
        rwWindow->SetVisible( true );
        
        // Create the game renderer.
        Driver *d3dDriver = CreateDriver( engineInterface, "Direct3D12" );

        assert( d3dDriver != NULL );

        // Set up the game resources.
        DriverSwapChain *swapChain = d3dDriver->CreateSwapChain( rwWindow, 2 ); // we want to double-buffer.

        DrawingLayer2D *guiContext = CreateDrawingLayer2D( d3dDriver );

        // We have to get the ref count over here, because the swap chain increases the ref count as well.
        uint32 wndBaseRefCount = GetRefCount( rwWindow );

        // Execute the main loop
        while ( GetRefCount( rwWindow ) >= wndBaseRefCount )   // we wait until somebody requested to destroy the window.
        {
            // Draw the game scene.
            {
                // Set render states.
                guiContext->SetRenderState( DrawingLayer2D::RWSTATE_UTEXADDRESSMODE, RWTEXADDRESS_WRAP );
                guiContext->SetRenderState( DrawingLayer2D::RWSTATE_VTEXADDRESSMODE, RWTEXADDRESS_WRAP );
                guiContext->SetRenderState( DrawingLayer2D::RWSTATE_ZWRITEENABLE, true );
                guiContext->SetRenderState( DrawingLayer2D::RWSTATE_TEXFILTER, RWFILTER_POINT );
                guiContext->SetRenderState( DrawingLayer2D::RWSTATE_SRCBLEND, RWBLEND_ONE );
                guiContext->SetRenderState( DrawingLayer2D::RWSTATE_DSTBLEND, RWBLEND_ZERO );
                guiContext->SetRenderState( DrawingLayer2D::RWSTATE_ALPHABLENDENABLE, true );
                guiContext->SetRenderState( DrawingLayer2D::RWSTATE_ALPHAFUNC, RWCMP_ALWAYS );
                guiContext->SetRenderState( DrawingLayer2D::RWSTATE_ALPHAREF, 0 );

                // Execute draws.
                guiContext->Begin();

                guiContext->DrawRect( 50, 50, 100, 100 );
                guiContext->DrawRect( 60, 60, 80, 80 );
                guiContext->DrawLine( 10, 10, 290, 150 );

                guiContext->End();
            }

            // Give cycles to the window manager.
            // In the multi-threaded environment, this will effectively be a no-op.
            PulseWindowingSystem( engineInterface );

            // We want to give some cycles to the OS.
            // Otherwise our thread would starve everything.
            YieldExecution( 1 );
        }

        // Hide the window.
        // Do this because terminating resources takes some time and
        // the user already knows this application is terminating.
        rwWindow->SetVisible( false );

        // Destroy drawing contexts.
        DeleteDrawingLayer2D( guiContext );

        // Release the swap chain device resource.
        d3dDriver->DestroySwapChain( swapChain );

        // Terminate the driver.
        DestroyDriver( engineInterface, d3dDriver );

        // Release our window reference.
        // This will destroy it.
        engineInterface->DeleteRwObject( rwWindow );

        return 0;
    }
};

// We use the windows subsystem.
BOOL WINAPI WinMain( HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nCmdShow )
{
    // Redirect to RenderWare.
    return rw::frameworkEntryPoint_win32( hInst, hPrevInst, lpCmdLine, nCmdShow );
}