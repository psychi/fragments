/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief DirectX D3DX にテンプレート特殊化した幾何ベクトル演算。
 */
#ifndef PSYQ_GEOMETRY_D3DX_VECTOR_HPP_
#define PSYQ_GEOMETRY_D3DX_VECTOR_HPP_

#include <d3dx9math.h>
//#include "psyq/geometry/vector.hpp"

namespace psyq
{
namespace geometry
{
namespace vector
{
//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief D3DXVECTOR2 にテンプレート特殊化した幾何ベクトル型特性。
    @ingroup psyq_geometry_vector_traits
 */
template<> class traits<D3DXVECTOR2>
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

//-----------------------------------------------------------------------------
/// @name 幾何ベクトルの成分
//@{
/// @copydoc psyq::geometry::vector::at
inline psyq::geometry::vector::traits<D3DXVECTOR2>::element& at(
    D3DXVECTOR2& io_vector,
    unsigned const in_index)
{
    PSYQ_ASSERT(in_index < 2);
    return *(((FLOAT*)io_vector) + in_index);
}
//@}
//-----------------------------------------------------------------------------
/// @name 幾何ベクトルの演算
//@{
/// @copydoc psyq::geometry::vector::dot
inline psyq::geometry::vector::traits<D3DXVECTOR2>::element dot(
    D3DXVECTOR2 const& in_left,
    D3DXVECTOR2 const& in_right)
{
    return D3DXVec2Dot(&in_left, &in_right);
}
//@}
//-------------------------------------------------------------------------
/// @name 幾何ベクトルの大きさ
//@{
/// @copydoc psyq::geometry::vector::length
inline psyq::geometry::vector::traits<D3DXVECTOR2>::element length(
    D3DXVECTOR2 const& in_vector)
{
    return D3DXVec2Length(&in_vector);
}

/// @copydoc psyq::geometry::vector::normalize
inline D3DXVECTOR2 normalize(D3DXVECTOR2 const& in_vector)
{
    D3DXVECTOR2 local_vector;
    D3DXVec2Normalize(&local_vector, &in_vector);
    return local_vector;
}
//@}
//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief D3DXVECTOR3 にテンプレート特殊化した幾何ベクトル型特性。
    @ingroup psyq_geometry_vector_traits
 */
template<> class traits<D3DXVECTOR3>
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

//-----------------------------------------------------------------------------
/// @name 幾何ベクトルの成分
//@{
/// @copydoc psyq::geometry::vector::at
inline psyq::geometry::vector::traits<D3DXVECTOR3>::element& at(
    D3DXVECTOR3& io_vector,
    unsigned const in_index)
{
    PSYQ_ASSERT(in_index < 3);
    return *(((FLOAT*)io_vector) + in_index);
}
//@}
//-----------------------------------------------------------------------------
/// @name 幾何ベクトルの演算
//@{
/// @copydoc psyq::geometry::vector::dot
inline psyq::geometry::vector::traits<D3DXVECTOR3>::element dot(
    D3DXVECTOR3 const& in_left,
    D3DXVECTOR3 const& in_right)
{
    return D3DXVec3Dot(&in_left, &in_right);
}

/// @copydoc psyq::geometry::vector::cross_3d
inline D3DXVECTOR3 cross_3d(
    D3DXVECTOR3 const& in_left,
    D3DXVECTOR3 const& in_right)
{
    D3DXVECTOR3 local_cross;
    D3DXVec3Cross(&local_cross, &in_left, &in_right);
    return local_cross;
}
//@}
//-------------------------------------------------------------------------
/// @name 幾何ベクトルの大きさ
//@{
/// @copydoc psyq::geometry::vector::length
inline psyq::geometry::vector::traits<D3DXVECTOR3>::element length(
    D3DXVECTOR3 const& in_vector)
{
    return D3DXVec3Length(&in_vector);
}

/// @copydoc psyq::geometry::vector::normalize
inline D3DXVECTOR3 normalize(D3DXVECTOR3 const& in_vector)
{
    D3DXVECTOR3 local_vector;
    D3DXVec3Normalize(&local_vector, &in_vector);
    return local_vector;
}
//@}
//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief D3DXVECTOR4 にテンプレート特殊化した幾何ベクトル型特性。
    @ingroup psyq_geometry_vector_traits
 */
template<> class traits<D3DXVECTOR4>
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

//-----------------------------------------------------------------------------
/// @name 幾何ベクトルの成分
//@{
/// @copydoc psyq::geometry::vector::at
inline psyq::geometry::vector::traits<D3DXVECTOR4>::element& at(
    D3DXVECTOR4& io_vector,
    unsigned const in_index)
{
    PSYQ_ASSERT(in_index < 4);
    return *(((FLOAT*)io_vector) + in_index);
}
//@}
//-----------------------------------------------------------------------------
/// @name 幾何ベクトルの演算
//@{
/// @copydoc psyq::geometry::vector::dot
inline psyq::geometry::vector::traits<D3DXVECTOR4>::element dot(
    D3DXVECTOR4 const& in_left,
    D3DXVECTOR4 const& in_right)
{
    return D3DXVec4Dot(&in_left, &in_right);
}

/// @copydoc psyq::geometry::vector::cross_4d
inline D3DXVECTOR4 cross_4d(
    D3DXVECTOR4 const& in_left,
    D3DXVECTOR4 const& in_middle,
    D3DXVECTOR4 const& in_right)
{
    D3DXVECTOR4 local_cross;
    D3DXVec4Cross(&local_cross, &in_left, &in_middle, &in_right);
    return local_cross;
}
//@}
//-------------------------------------------------------------------------
/// @name 幾何ベクトルの大きさ
//@{
/// @copydoc psyq::geometry::vector::length
inline psyq::geometry::vector::traits<D3DXVECTOR4>::element length(
    D3DXVECTOR4 const& in_vector)
{
    return D3DXVec4Length(&in_vector);
}

/// @copydoc psyq::geometry::vector::normalize
inline D3DXVECTOR4 normalize(D3DXVECTOR4 const& in_vector)
{
    D3DXVECTOR4 local_vector;
    D3DXVec4Normalize(&local_vector, &in_vector);
    return local_vector;
}
//@}
} // namespace vector
} // namespace geometry
} // namespace psyq

#endif // !defined(PSYQ_GEOMETRY_D3DX_VECTOR_HPP_)
