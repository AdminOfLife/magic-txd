/*
    RenderWare math definitions.
    Every good engine needs stable math, so we provide you with it!
*/

// These structs are standardized.
// You can safely use them in streams!

#ifndef _RENDERWARE_MATH_GLOBALS_
#define _RENDERWARE_MATH_GLOBALS_

#define DEG2RAD(x)  ( M_PI * x / 180 )
#define RAD2DEG(x)  ( x / M_PI * 180 )

#pragma warning(push)
#pragma warning(disable:4520)

// TODO: add support for more compilers.
#undef _SUPPORTS_CONSTEXPR

#if defined(_MSC_VER)
#if _MSC_VER >= 1900
#define _SUPPORTS_CONSTEXPR
#endif
#else
// Unknown compiler.
#define _SUPPORTS_CONSTEXPR
#endif

#define FASTMATH __declspec(noalias)

template <typename numberType, size_t dimm>
struct complex_assign_helper
{
    template <typename valType>
    AINLINE static void AssignStuff( numberType *elems, size_t n, valType&& theVal )
    {
        elems[ n ] = (numberType)theVal;
    }

    template <size_t n, typename curVal, typename... Args>
    struct constr_helper
    {
        AINLINE static void ConstrVecElem( numberType *elems, curVal&& theVal, Args... leftArgs )
        {
            AssignStuff( elems, n, theVal );

            constr_helper <n + 1, Args...>::ConstrVecElem( elems, std::forward <Args> ( leftArgs )... );
        }
    };

    template <size_t n>
    struct constr_helper_ex
    {
        AINLINE static void ConstrVecElem( numberType *elems )
        {
            elems[ n ] = numberType();

            constr_helper_ex <n + 1>::ConstrVecElem( elems );
        }
    };

    template <>
    struct constr_helper_ex <dimm>
    {
        AINLINE static void ConstrVecElem( numberType *elems )
        {
            // Nothing.
        }
    };

    template <size_t n, typename curVal>
    struct constr_helper <n, curVal>
    {
        AINLINE static void ConstrVecElem( numberType *elems, curVal&& theVal )
        {
            AssignStuff( elems, n, theVal );

            constr_helper_ex <n + 1>::ConstrVecElem( elems );
        }
    };
};

template <typename numberType, size_t dimm>
struct Vector
{
    AINLINE FASTMATH Vector( void )
    {
        for ( size_t n = 0; n < dimm; n++ )
        {
            this->elems[ n ] = numberType();
        }
    }

    template <typename... Args,
        typename = typename std::enable_if<
            ( sizeof...(Args) <= dimm && sizeof...(Args) != 0 )
        >::type>
    AINLINE FASTMATH explicit Vector( Args&&... theStuff )
    {
        complex_assign_helper <numberType, dimm>::
            constr_helper <0, Args...>::
                ConstrVecElem( this->elems, std::forward <Args> ( theStuff )... );
    }

    AINLINE FASTMATH Vector( std::initializer_list <numberType> args )
    {
        size_t n = 0;

        for ( numberType iter : args )
        {
            this->elems[ n ] = iter;

            n++;
        }
    }

    AINLINE FASTMATH Vector( const Vector& right )
    {
        for ( size_t n = 0; n < dimm; n++ )
        {
            this->elems[ n ] = right.elems[ n ];
        }
    }
    
    AINLINE FASTMATH Vector( Vector&& right )
    {
        for ( size_t n = 0; n < dimm; n++ )
        {
            this->elems[ n ] = std::move( right.elems[ n ] );
        }
    }

    AINLINE FASTMATH Vector& operator = ( const Vector& right )
    {
        for ( size_t n = 0; n < dimm; n++ )
        {
            this->elems[ n ] = right.elems[ n ];
        }

        return *this;
    }

    AINLINE FASTMATH Vector& operator = ( Vector&& right )
    {
        for ( size_t n = 0; n < dimm; n++ )
        {
            this->elems[ n ] = std::move( right.elems[ n ] );
        }

        return *this;
    }

    // Basic vector math.
    AINLINE FASTMATH Vector operator + ( const Vector& right ) const
    {
        Vector newVec = *this;

        for ( size_t n = 0; n < dimm; n++ )
        {
            newVec.elems[ n ] += right.elems[ n ];
        }

        return newVec;
    }

    AINLINE FASTMATH Vector& operator += ( const Vector& right )
    {
        for ( size_t n = 0; n < dimm; n++ )
        {
            this->elems[ n ] += right.elems[ n ];
        }

        return *this;
    }

    AINLINE FASTMATH Vector operator * ( numberType&& val ) const
    {
        Vector newVec = *this;

        for ( size_t n = 0; n < dimm; n++ )
        {
            newVec.elems[ n ] *= val;
        }

        return newVec;
    }

    AINLINE FASTMATH Vector& operator *= ( numberType&& val )
    {
        for ( size_t n = 0; n < dimm; n++ )
        {
            this->elems[ n ] *= val;
        }

        return *this;
    }

    AINLINE FASTMATH Vector operator / ( numberType&& val ) const
    {
        Vector newVec = *this;

        for ( size_t n = 0; n < dimm; n++ )
        {
            newVec.elems[ n ] /= val;
        }

        return newVec;
    }

    AINLINE FASTMATH bool operator == ( const Vector& right ) const
    {
        for ( size_t n = 0; n < dimm; n++ )
        {
            if ( this->elems[ n ] != right.elems[ n ] )
            {
                return false;
            }
        }

        return true;
    }
    AINLINE FASTMATH bool operator != ( const Vector& right ) const
    {
        return !( *this == right );
    }

    AINLINE FASTMATH numberType& operator [] ( ptrdiff_t n )
    {
        return elems[ n ];
    }

    AINLINE FASTMATH const numberType& operator [] ( ptrdiff_t n ) const
    {
        return elems[ n ];
    }

private:
    numberType elems[ dimm ];
};

// Special vectorized versions.
template <>
struct Vector <float, 4>
{
    AINLINE Vector( void ) : data() {}
    AINLINE Vector( const Vector& right ) : data( right.data )  {}
    AINLINE Vector( Vector&& right ) : data( std::move( right.data ) )  {}

    AINLINE Vector( float x, float y, float z, float w = 1.0f )
    {
        struct __declspec( align( 16 ) )
        {
            union
            {
                struct
                {
                    float a, b, c, d;
                };
                __m128 ssereg;
            };
        } tmp;

        tmp.a = x;
        tmp.b = y;
        tmp.c = z;
        tmp.d = w;

        this->data = tmp.ssereg;
    }

    AINLINE Vector& operator = ( const Vector& right )
    {
        this->data = right.data;

        return *this;
    }
    AINLINE Vector& operator = ( Vector&& right )
    {
        this->data = std::move( right.data );

        return *this;
    }

    // Addition.
    AINLINE Vector operator + ( const Vector& right ) const { Vector newVec = *this; newVec.data = _mm_add_ps( newVec.data, right.data ); return newVec; }
    AINLINE Vector operator + ( Vector&& right ) const      { Vector newVec = right; newVec.data = _mm_add_ps( newVec.data, this->data ); return newVec; }
    AINLINE Vector& operator += ( const Vector& right )     { this->data = _mm_add_ps( this->data, right.data ); return *this; }
    AINLINE Vector& operator += ( Vector&& right )          { this->data = _mm_add_ps( this->data, right.data ); return *this; }

private:
    AINLINE static __m128 loadExpanded( float val )
    { __m128 tmp = _mm_load_ss( &val ); tmp = _mm_shuffle_ps( tmp, tmp, _MM_SHUFFLE( 0, 0, 0, 0 ) ); return tmp; }

public:
    // By scalar.
    AINLINE Vector operator + ( float val ) const   { Vector newVec = *this; __m128 tmp = loadExpanded( val ); newVec.data = _mm_add_ps( newVec.data, tmp ); return newVec; }
    AINLINE Vector operator += ( float val )        { __m128 tmp = loadExpanded( val ); this->data = _mm_add_ps( this->data, tmp ); return *this; }

    // Subtraction.
    AINLINE Vector operator - ( const Vector& right ) const { Vector newVec = *this; newVec.data = _mm_sub_ps( newVec.data, right.data ); return newVec; }
    AINLINE Vector operator - ( Vector&& right ) const      { Vector newVec = right; newVec.data = _mm_sub_ps( newVec.data, this->data ); return newVec; }
    AINLINE Vector& operator -= ( const Vector& right )     { this->data = _mm_sub_ps( this->data, right.data ); return *this; }
    AINLINE Vector& operator -= ( Vector&& right )          { this->data = _mm_sub_ps( this->data, right.data ); return *this; }

    // By scalar.
    AINLINE Vector operator - ( float val ) const   { Vector newVec = *this; __m128 tmp = loadExpanded( val ); newVec.data = _mm_sub_ps( newVec.data, tmp ); return newVec; }
    AINLINE Vector operator -= ( float val )        { __m128 tmp = loadExpanded( val ); this->data = _mm_sub_ps( this->data, tmp ); return *this; }

    // Multiplication by scalar.
    AINLINE Vector operator * ( float val ) const   { Vector newVec = *this; __m128 tmp = loadExpanded( val ); newVec.data = _mm_mul_ps( newVec.data, tmp ); return newVec; }
    AINLINE Vector operator *= ( float val )        { __m128 tmp = loadExpanded( val ); this->data = _mm_mul_ps( this->data, tmp ); return *this; }

    // Division by scalar.
    AINLINE Vector operator / ( float val ) const   { Vector newVec = *this; __m128 tmp = loadExpanded( val ); newVec.data = _mm_div_ps( newVec.data, tmp ); return newVec; }
    AINLINE Vector operator /= ( float val )        { __m128 tmp = loadExpanded( val ); this->data = _mm_div_ps( this->data, tmp ); return *this; }

    AINLINE bool operator == ( const Vector& right ) const
    {
        __m128 cmpRes1 = _mm_cmpneq_ps( this->data, right.data );

        bool isSame = ( _mm_testz_ps( cmpRes1, cmpRes1 ) != 0 );

        return isSame;
    }
    AINLINE bool operator != ( const Vector& right ) const
    {
        return !( *this == right );
    }

    AINLINE float operator [] ( ptrdiff_t n ) const
    {
        struct __declspec( align( 16 ) )
        {
            union
            {
                __m128 ssereg;
                float f[4];
            };
        } tmp;

        tmp.ssereg = this->data;

        return tmp.f[ n ];
    }

    AINLINE float& operator [] ( ptrdiff_t n )
    {
        return *( (float*)this + n );
    }

private:
    __m128 data;
};

template <>
struct Vector <double, 2>
{
    AINLINE Vector( void ) : data() {}
    AINLINE Vector( const Vector& right ) : data( right.data )  {}
    AINLINE Vector( Vector&& right ) : data( std::move( right.data ) )  {}

    AINLINE Vector( double x, double y )
    {
        struct __declspec( align( 16 ) )
        {
            union
            {
                struct
                {
                    double a, b;
                };
                __m128d ssereg;
            };
        } tmp;

        tmp.a = x;
        tmp.b = y;

        this->data = tmp.ssereg;
    }

    AINLINE Vector& operator = ( const Vector& right )
    {
        this->data = right.data;

        return *this;
    }
    AINLINE Vector& operator = ( Vector&& right )
    {
        this->data = std::move( right.data );

        return *this;
    }

    // Addition.
    AINLINE Vector operator + ( const Vector& right ) const { Vector newVec = *this; newVec.data = _mm_add_pd( newVec.data, right.data ); return newVec; }
    AINLINE Vector operator + ( Vector&& right ) const      { Vector newVec = right; newVec.data = _mm_add_pd( newVec.data, this->data ); return newVec; }
    AINLINE Vector& operator += ( const Vector& right )     { this->data = _mm_add_pd( this->data, right.data ); return *this; }
    AINLINE Vector& operator += ( Vector&& right )          { this->data = _mm_add_pd( this->data, right.data ); return *this; }

private:
    AINLINE static __m128d loadExpanded( double val )
    { __m128d tmp = _mm_load_sd( &val ); tmp = _mm_shuffle_pd( tmp, tmp, _MM_SHUFFLE2(0, 0) ); return tmp; }

public:
    // By scalar.
    AINLINE Vector operator + ( double val ) const  { Vector newVec = *this; __m128d tmp = loadExpanded( val ); newVec.data = _mm_add_pd( newVec.data, tmp ); return newVec; }
    AINLINE Vector operator += ( double val )       { __m128d tmp = loadExpanded( val ); this->data = _mm_add_pd( this->data, tmp ); return *this; }

    // Subtraction.
    AINLINE Vector operator - ( const Vector& right ) const { Vector newVec = *this; newVec.data = _mm_sub_pd( newVec.data, right.data ); return newVec; }
    AINLINE Vector operator - ( Vector&& right ) const      { Vector newVec = right; newVec.data = _mm_sub_pd( newVec.data, this->data ); return newVec; }
    AINLINE Vector& operator -= ( const Vector& right )     { this->data = _mm_sub_pd( this->data, right.data ); return *this; }
    AINLINE Vector& operator -= ( Vector&& right )          { this->data = _mm_sub_pd( this->data, right.data ); return *this; }

    // By scalar.
    AINLINE Vector operator - ( double val ) const  { Vector newVec = *this; __m128d tmp = loadExpanded( val ); newVec.data = _mm_sub_pd( newVec.data, tmp ); return newVec; }
    AINLINE Vector operator -= ( double val )       { __m128d tmp = loadExpanded( val ); this->data = _mm_sub_pd( this->data, tmp ); return *this; }

    // Multiplication by scalar.
    AINLINE Vector operator * ( double val ) const  { Vector newVec = *this; __m128d tmp = loadExpanded( val ); newVec.data = _mm_mul_pd( newVec.data, tmp ); return newVec; }
    AINLINE Vector operator *= ( double val )       { __m128d tmp = loadExpanded( val ); this->data = _mm_mul_pd( this->data, tmp ); return *this; }

    // Division by scalar.
    AINLINE Vector operator / ( double val ) const  { Vector newVec = *this; __m128d tmp = loadExpanded( val ); newVec.data = _mm_div_pd( newVec.data, tmp ); return newVec; }
    AINLINE Vector operator /= ( double val )       { __m128d tmp = loadExpanded( val ); this->data = _mm_div_pd( this->data, tmp ); return *this; }

    AINLINE bool operator == ( const Vector& right ) const
    {
        __m128d cmpRes1 = _mm_cmpneq_pd( this->data, right.data );

        bool isSame = ( _mm_testz_pd( cmpRes1, cmpRes1 ) != 0 );

        return isSame;
    }
    AINLINE bool operator != ( const Vector& right ) const
    {
        return !( *this == right );
    }

    AINLINE double operator [] ( ptrdiff_t n ) const
    {
        struct __declspec( align( 16 ) )
        {
            union
            {
                __m128d ssereg;
                double d[2];
            };
        } tmp;

        tmp.ssereg = this->data;

        return tmp.d[ n ];
    }

    AINLINE double& operator [] ( ptrdiff_t n )
    {
        return *( (double*)this + n );
    }

private:
    __m128d data;
};

template <>
struct Vector <double, 4>
{
    AINLINE Vector( void ) : data1(), data2() {}
    AINLINE Vector( const Vector& right ) : data1( right.data1 ), data2( right.data2 )  {}
    AINLINE Vector( Vector&& right ) : data1( std::move( right.data1 ) ), data2( std::move( right.data2 ) )  {}

    AINLINE Vector( double x, double y, double z, double w = 1.0 )
    {
        struct __declspec( align( 16 ) )
        {
            union
            {
                struct
                {
                    double a, b, c, d;
                };
                struct
                {
                    __m128d ssereg1;
                    __m128d ssereg2;
                };
            };
        } tmp;

        tmp.a = x;
        tmp.b = y;
        tmp.c = z;
        tmp.d = w;

        this->data1 = tmp.ssereg1;
        this->data2 = tmp.ssereg2;
    }

    AINLINE Vector& operator = ( const Vector& right )
    {
        this->data1 = right.data1;
        this->data2 = right.data2;

        return *this;
    }
    AINLINE Vector& operator = ( Vector&& right )
    {
        this->data1 = std::move( right.data1 );
        this->data2 = std::move( right.data2 );

        return *this;
    }

    // Addition.
    AINLINE Vector operator + ( const Vector& right ) const
    { Vector newVec = *this; newVec.data1 = _mm_add_pd( newVec.data1, right.data1 ); newVec.data2 = _mm_add_pd( newVec.data2, right.data2 ); return newVec; }
    AINLINE Vector operator + ( Vector&& right ) const
    { Vector newVec = right; newVec.data1 = _mm_add_pd( newVec.data1, this->data1 ); newVec.data2 = _mm_add_pd( newVec.data2, this->data2 ); return newVec; }
    AINLINE Vector& operator += ( const Vector& right )
    { this->data1 = _mm_add_pd( this->data1, right.data1 ); this->data2 = _mm_add_pd( this->data2, right.data2 ); return *this; }
    AINLINE Vector& operator += ( Vector&& right )
    { this->data1 = _mm_add_pd( this->data1, right.data1 ); this->data2 = _mm_add_pd( this->data2, right.data2 ); return *this; }

private:
    AINLINE static __m128d loadExpanded( double val )
    { __m128d tmp = _mm_load_sd( &val ); tmp = _mm_shuffle_pd( tmp, tmp, _MM_SHUFFLE2(0, 0) ); return tmp; }

public:
    // By scalar.
    AINLINE Vector operator + ( double val ) const
    { Vector newVec = *this; __m128d tmp = loadExpanded( val ); newVec.data1 = _mm_add_pd( newVec.data1, tmp ); newVec.data2 = _mm_add_pd( newVec.data2, tmp ); return newVec; }
    AINLINE Vector operator += ( double val )
    { __m128d tmp = loadExpanded( val ); this->data1 = _mm_add_pd( this->data1, tmp ); this->data2 = _mm_add_pd( this->data2, tmp ); return *this; }

    // Subtraction.
    AINLINE Vector operator - ( const Vector& right ) const
    { Vector newVec = *this; newVec.data1 = _mm_sub_pd( newVec.data1, right.data1 ); newVec.data2 = _mm_sub_pd( newVec.data2, right.data2 ); return newVec; }
    AINLINE Vector operator - ( Vector&& right ) const
    { Vector newVec = right; newVec.data1 = _mm_sub_pd( newVec.data1, this->data1 ); newVec.data2 = _mm_sub_pd( newVec.data2, this->data2 ); return newVec; }
    AINLINE Vector& operator -= ( const Vector& right )
    { this->data1 = _mm_sub_pd( this->data1, right.data1 ); this->data2 = _mm_sub_pd( this->data2, right.data2 ); return *this; }
    AINLINE Vector& operator -= ( Vector&& right )
    { this->data1 = _mm_sub_pd( this->data1, right.data1 ); this->data2 = _mm_sub_pd( this->data2, right.data2 ); return *this; }

    // By scalar.
    AINLINE Vector operator - ( double val ) const
    { Vector newVec = *this; __m128d tmp = loadExpanded( val ); newVec.data1 = _mm_sub_pd( newVec.data1, tmp ); newVec.data2 = _mm_sub_pd( newVec.data2, tmp ); return newVec; }
    AINLINE Vector operator -= ( double val )
    { __m128d tmp = loadExpanded( val ); this->data1 = _mm_sub_pd( this->data1, tmp ); this->data2 = _mm_sub_pd( this->data2, tmp ); return *this; }

    // Multiplication by scalar.
    AINLINE Vector operator * ( double val ) const
    { Vector newVec = *this; __m128d tmp = loadExpanded( val ); newVec.data1 = _mm_mul_pd( newVec.data1, tmp ); newVec.data2 = _mm_mul_pd( newVec.data2, tmp ); return newVec; }
    AINLINE Vector operator *= ( double val )
    { __m128d tmp = loadExpanded( val ); this->data1 = _mm_mul_pd( this->data1, tmp ); this->data2 = _mm_mul_pd( this->data2, tmp ); return *this; }

    // Division by scalar.
    AINLINE Vector operator / ( double val ) const
    { Vector newVec = *this; __m128d tmp = loadExpanded( val ); newVec.data1 = _mm_div_pd( newVec.data1, tmp ); newVec.data2 = _mm_div_pd( newVec.data2, tmp ); return newVec; }
    AINLINE Vector operator /= ( double val )
    { __m128d tmp = loadExpanded( val ); this->data1 = _mm_div_pd( this->data1, tmp ); this->data2 = _mm_div_pd( this->data2, tmp ); return *this; }

    AINLINE bool operator == ( const Vector& right ) const
    {
        __m128d cmpRes1 = _mm_cmpneq_pd( this->data1, right.data1 );
        __m128d cmpRes2 = _mm_cmpneq_pd( this->data2, right.data2 );

        bool isSame = ( _mm_testz_pd( cmpRes1, cmpRes1 ) != 0 && _mm_testz_pd( cmpRes2, cmpRes2 ) != 0 );

        return isSame;
    }
    AINLINE bool operator != ( const Vector& right ) const
    {
        return !( *this == right );
    }

    AINLINE double operator [] ( ptrdiff_t n ) const
    {
        struct __declspec( align( 16 ) )
        {
            union
            {
                struct
                {
                    __m128d ssereg1;
                    __m128d ssereg2;
                };
                double d[4];
            };
        } tmp;

        tmp.ssereg1 = this->data1;
        tmp.ssereg2 = this->data2;

        return tmp.d[ n ];
    }

    AINLINE double& operator [] ( ptrdiff_t n )
    {
        return *( (double*)this + n );
    }

private:
    __m128d data1;
    __m128d data2;
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

// Include the matrix view submodule.
#include "renderware.math.matview.h"

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

#ifdef _SUPPORTS_CONSTEXPR
    // Matrix inversion.
    AINLINE numberType det( void ) const
    {
        using namespace rw::mview;

        return detcalc <numberType, SquareMatrix, dimm>::calc( *this );
    }

    template <size_t y_iter>
    struct inverse_matrix_y_iterator
    {
        template <size_t x_iter>
        struct inverse_matrix_x_iterator
        {
            AINLINE static void CalculateNative( const SquareMatrix& srcMat, SquareMatrix& matOut )
            {
                numberType resultDet =
                    rw::mview::detcalc <numberType, SquareMatrix, dimm - 1, rw::mview::matview_skip_y <y_iter>, rw::mview::matview_skip_x <x_iter>>::calc( srcMat );

                if ( ( ( y_iter % 2 ) ^ ( x_iter % 2 ) ) == 0 )
                {
                    matOut[x_iter][y_iter] = resultDet;
                }
                else
                {
                    matOut[x_iter][y_iter] = -resultDet;
                }
            }

            template <typename = std::enable_if <(x_iter != dimm)>::type>
            AINLINE static void Calculate( const SquareMatrix& srcMat, SquareMatrix& matOut )
            {
                CalculateNative( srcMat, matOut );

                inverse_matrix_x_iterator <x_iter + 1>::Calculate( srcMat, matOut );
            }

            template <typename = std::enable_if <(x_iter == dimm)>::type>
            AINLINE static void Calculate( const SquareMatrix& srcMat, SquareMatrix& matOut, int a = 0 )
            {
                return;
            }
        };

        template <typename = std::enable_if <(y_iter != dimm)>::type>
        AINLINE static void Calculate( const SquareMatrix& srcMat, SquareMatrix& matOut )
        {
            inverse_matrix_x_iterator <0>::Calculate( srcMat, matOut );

            inverse_matrix_y_iterator <y_iter + 1>::Calculate( srcMat, matOut );
        }

        template <typename = std::enable_if <(y_iter == dimm)>::type>
        AINLINE static void Calculate( const SquareMatrix& srcMat, SquareMatrix& matOut, int a = 0 )
        {
            return;
        }
    };

    // Makes no sense to always_inline this piece of code.
    // You know what I mean.
    bool Invert( void )
    {
        SquareMatrix cofactor_matrix;

        inverse_matrix_y_iterator <0>::Calculate( *this, cofactor_matrix );

        // Quickly calculate the determinant.
        numberType theDet = 0;

        for ( size_t n = 0; n < dimm; n++ )
        {
            theDet += ( (*this)[ 0 ][ n ] * cofactor_matrix[ n ][ 0 ] );
        }

        if ( theDet == 0 )
            return false;

        // Inverse matrix time!
        for ( size_t y = 0; y < dimm; y++ )
        {
            for ( size_t x = 0; x < dimm; x++ )
            {
                (*this)[ y ][ x ] = cofactor_matrix[ y ][ x ] / theDet;
            }
        }

        return true;
    }
#endif //_SUPPORTS_CONSTEXPR

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

#pragma warning(pop)

#endif //_RENDERWARE_MATH_GLOBALS_