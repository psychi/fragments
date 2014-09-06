/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief DirectX D3DX にテンプレート特殊化した幾何ベクトル演算。
 */
#ifndef PSYQ_GEOMETRY_D3DX_VECTOR_HPP_
#define PSYQ_GEOMETRY_D3DX_VECTOR_HPP_

#include <d3dx9math.h>
//#include "psyq/geometry/vector.hpp"

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief D3DXVECTOR2 にテンプレート特殊化した幾何ベクトル型特性。
    @ingroup psyq_geometry_vector_traits
 */
template<> class psyq::geometry::vector_traits<D3DXVECTOR2>
{
    /// 幾何ベクトルの型。
    public: typedef D3DXVECTOR2 type;
    /// 幾何ベクトルが持つ成分の型。
    public: typedef FLOAT element;

    public: enum: unsigned
    {
        /// 幾何ベクトルが持つ成分の数。
        size = 2,
    };
};

/** @brief D3DXVECTOR3 にテンプレート特殊化した幾何ベクトル型特性。
    @ingroup psyq_geometry_vector_traits
 */
template<> class psyq::geometry::vector_traits<D3DXVECTOR3>
{
    /// 幾何ベクトルの型。
    public: typedef D3DXVECTOR3 type;
    /// 幾何ベクトルが持つ成分の型。
    public: typedef FLOAT element;

    public: enum: unsigned
    {
        /// 幾何ベクトルが持つ成分の数。
        size = 3,
    };
};

/** @brief D3DXVECTOR4 にテンプレート特殊化した幾何ベクトル型特性。
    @ingroup psyq_geometry_vector_traits
 */
template<> class psyq::geometry::vector_traits<D3DXVECTOR4>
{
    /// 幾何ベクトルの型。
    public: typedef D3DXVECTOR4 type;
    /// 幾何ベクトルが持つ成分の型。
    public: typedef FLOAT element;

    public: enum: unsigned
    {
        /// 幾何ベクトルが持つ成分の数。
        size = 4,
    };
};

#if 0
//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq {
namespace geometry {
namespace vector {
//-----------------------------------------------------------------------------
/// @copydoc psyq::geometry::vector::at
inline FLOAT& at(
    D3DXVECTOR3& io_vector,
    unsigned const in_index)
{
    typedef psyq::geometry::vector_traits<D3DXVECTOR3> vector_traits;
    PSYQ_ASSERT(in_index < vector_traits::size);
    return *(((FLOAT*)io_vector) + in_index);
}

}}}

//-----------------------------------------------------------------------------
/// @copydoc psyq::geometry::vector::dot
template<>
typename psyq::geometry::vector_traits<D3DXVECTOR3>::element
psyq::geometry::vector::dot<D3DXVECTOR3>(
    D3DXVECTOR3 const& in_left,
    D3DXVECTOR3 const& in_right)
{
    return D3DXVec3Dot(&in_left, &in_right);
}

/// @copydoc psyq::geometry::vector::cross_3d
template<>
D3DXVECTOR3 psyq::geometry::vector::cross_3d<D3DXVECTOR3>(
    D3DXVECTOR3 const& in_left,
    D3DXVECTOR3 const& in_right)
{
    D3DXVECTOR3 local_cross;
    D3DXVec3Cross(&local_cross, &in_left, &in_right);
    return local_cross;
}

//-------------------------------------------------------------------------
/// @copydoc psyq::geometry::vector::length
template<>
typename psyq::geometry::vector_traits<D3DXVECTOR3>::element
psyq::geometry::vector::length<D3DXVECTOR3>(
    D3DXVECTOR3 const& in_vector)
{
    return D3DXVec3Length(&in_vector);
}

/// @copydoc psyq::geometry::vector::normalize
template<>
D3DXVECTOR3 psyq::geometry::vector::normalize<D3DXVECTOR3>(
    D3DXVECTOR3 const& in_vector)
{
    D3DXVECTOR3 local_vector;
    D3DXVec3Normalize(&local_vector, &in_vector);
    return local_vector;
}
/// @endcond
#endif

#endif // !defined(PSYQ_GEOMETRY_D3DX_VECTOR_HPP_)
