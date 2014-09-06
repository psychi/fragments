/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief DirectX D3DX にテンプレート特殊化した座標系。
 */
#ifndef PSYQ_GEOMETRY_D3DX_COORDINATE_HPP_
#define PSYQ_GEOMETRY_D3DX_COORDINATE_HPP_

#include <d3dx9math.h>
//#include "psyq/geometry/coordinate.hpp"

/// @cond
namespace psyq
{
    namespace geometry
    {
        template<typename, unsigned> class d3dx_coordinate;
    } // namespace geometry
} // namespace psyq
/// @endcond

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

/** @brief D3DXVECTOR2 にテンプレート特殊化した幾何ベクトル型特性。
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

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief DirectX D3DX のベクトルを使う座標系のベクトル処理。
    @ingroup psyq_geometry_coordinate
 */
template<typename template_vector, unsigned template_dimension>
class psyq::geometry::d3dx_coordinate:
public psyq::geometry::coordinate_traits<template_vector, template_dimension>
{
    /// thisが指す値の型。
    private: typedef d3dx_coordinate this_type;

    /// this_type の基底型。
    public: typedef psyq::geometry::coordinate_traits<vector, dimension>
        base_type;
    static_assert(
        template_dimension <= 4, "'base_type::dimension' is greater than 4.");

    /// 最小座標と最大座標を要素とするAABB。
    public: typedef psyq::geometry::aabb<this_type> aabb;

    //-------------------------------------------------------------------------
    /// @name 座標の成分
    //@{
    /// @copydoc psyq::geometry::coordinate::get_element()
    public: static typename base_type::element get_element(
        typename base_type::vector const& in_vector,
        unsigned const in_index)
    {
        PSYQ_ASSERT(in_index < this_type::dimension);
        return this_type::vector_at(in_vector, in_index);
    }

    /// @copydoc psyq::geometry::coordinate::set_element()
    public: static typename base_type::element set_element(
        typename base_type::vector& io_vector,
        unsigned const in_index,
        typename base_type::element const in_value)
    {
        PSYQ_ASSERT(in_index < this_type::dimension);
        auto& local_element(
            const_cast<typename base_type::element&>(
                this_type::vector_at(io_vector, in_index)));
        local_element = in_value;
        return in_value;
    }
    //@}
    private: template<typename template_glm_vec>
    static typename base_type::element const& vector_at(
        typename template_glm_vec const& in_vector,
        unsigned const in_index)
    {
        PSYQ_ASSERT(in_index < this_type::vector_traits::size);
        return *(((FLOAT const*)in_vector) + in_index);
    }

    //-------------------------------------------------------------------------
    /// @name 座標の比較
    //@{
    /// @copydoc psyq::geometry::coordinate::less_than()
    public: static bool less_than(
        typename base_type::vector const& in_left,
        typename base_type::vector const& in_right)
    {
        return !this_type::less_than_equal(in_right, in_left);
    }

    /// @copydoc psyq::geometry::coordinate::less_equal()
    public: static bool less_than_equal(
        typename base_type::vector const& in_left,
        typename base_type::vector const& in_right)
    {
        for (unsigned i(0); i < this_type::dimension; ++i)
        {
            auto const local_left(this_type::get_element(in_left, i));
            auto const local_right(this_type::get_element(in_right, i));
            if (local_right < local_left)
            {
                return false;
            }
        }
        return true;
    }
    //@}
    //-------------------------------------------------------------------------
    protected: static bool check_unused_elements(
        typename base_type::vector const& in_vector)
    {
        return (base_type::vector_traits::size <= 0
                || 0 < base_type::dimension
                || this_type::vector_at(in_vector, 0) == 0)
            && (base_type::vector_traits::size <= 1
                || 1 < base_type::dimension
                || this_type::vector_at(in_vector, 1) == 0)
            && (base_type::vector_traits::size <= 2
                || 2 < base_type::dimension
                || this_type::vector_at(in_vector, 2) == 0)
            && (base_type::vector_traits::size <= 3
                || 3 < base_type::dimension
                || this_type::vector_at(in_vector, 3) == 0);
    }

    protected: static bool check_unused_elements(
        typename base_type::vector const& in_vector_0,
        typename base_type::vector const& in_vector_1)
    {
        return (base_type::vector_traits::size <= 0
                || 0 < base_type::dimension
                || this_type::vector_at(in_vector_0, 0) == 0
                || this_type::vector_at(in_vector_1, 0) == 0)
            && (base_type::vector_traits::size <= 1
                || 1 < base_type::dimension
                || this_type::vector_at(in_vector_0, 1) == 0
                || this_type::vector_at(in_vector_1, 1) == 0)
            && (base_type::vector_traits::size <= 2
                || 2 < base_type::dimension
                || this_type::vector_at(in_vector_0, 2) == 0
                || this_type::vector_at(in_vector_1, 2) == 0)
            && (base_type::vector_traits::size <= 3
                || 3 < base_type::dimension
                || this_type::vector_at(in_vector_0, 3) == 0
                || this_type::vector_at(in_vector_1, 3) == 0);
    }

}; // class psyq::geometry::d3dx_coordinate

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief D3DXVECTOR2 を使う座標系のテンプレート特殊化。
    @ingroup psyq_geometry_coordinate
 */
template<unsigned template_dimension>
class psyq::geometry::coordinate<D3DXVECTOR2, template_dimension>:
public psyq::geometry::d3dx_coordinate<D3DXVECTOR2, template_dimension>
{
    /// thisが指す値の型。
    private: typedef coordinate this_type;

    /// this_type の基底型。
    public: typedef psyq::geometry::d3dx_coordinate<vector, dimension>
        base_type;

    //-------------------------------------------------------------------------
    /// @name 座標の演算
    //@{
    /// @copydoc psyq::geometry::coordinate::dot_product()
    public: static typename base_type::element dot_product(
        typename base_type::vector const& in_left,
        typename base_type::vector const& in_right)
    {
        PSYQ_ASSERT(this_type::check_unused_elements(in_left, in_right));
        return D3DXVec2Dot(&in_left, &in_right);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 座標の大きさ
    //@{
    /// @copydoc psyq::geometry::coordinate::compute_length()
    public: static typename base_type::element compute_length(
        typename base_type::vector const& in_vector)
    {
        PSYQ_ASSERT(base_type::check_unused_elements(in_vector));
        return D3DXVec2Length(&in_vector);
    }

    /// @copydoc psyq::geometry::coordinate::normalize_length()
    public: static typename base_type::vector normalize_length(
        typename base_type::vector const& in_vector)
    {
        PSYQ_ASSERT(this_type::check_unused_elements(in_vector));
        typename base_type::vector local_vector;
        D3DXVec2Normalize(&local_vector, &in_vector);
        return local_vector;
    }
    //@}

}; // class psyq::geometry::coordinate

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief D3DXVECTOR3 を使う座標系のテンプレート特殊化。
    @ingroup psyq_geometry_coordinate
 */
template<unsigned template_dimension>
class psyq::geometry::coordinate<D3DXVECTOR3, template_dimension>:
public psyq::geometry::d3dx_coordinate<D3DXVECTOR3, template_dimension>
{
    /// thisが指す値の型。
    private: typedef coordinate this_type;

    /// this_type の基底型。
    public: typedef psyq::geometry::d3dx_coordinate<vector, dimension>
        base_type;

    //-------------------------------------------------------------------------
    /// @name 座標の演算
    //@{
    /// @copydoc psyq::geometry::coordinate::dot_product()
    public: static typename base_type::element dot_product(
        typename base_type::vector const& in_left,
        typename base_type::vector const& in_right)
    {
        PSYQ_ASSERT(this_type::check_unused_elements(in_left, in_right));
        return D3DXVec3Dot(&in_left, &in_right);
    }

    /// @copydoc psyq::geometry::coordinate::cross_product()
    public: static typename base_type::vector cross_product(
        typename base_type::vector const& in_left,
        typename base_type::vector const& in_right)
    {
        static_assert(
            base_type::dimension == 3, "'base_type::dimension' is not 3.");
        PSYQ_ASSERT(this_type::check_unused_elements(in_left, in_right));
        typename base_type::vector local_cross;
        D3DXVec3Cross(&local_cross, &in_left, &in_right);
        return local_cross;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 座標の大きさ
    //@{
    /// @copydoc psyq::geometry::coordinate::compute_length()
    public: static typename base_type::element compute_length(
        typename base_type::vector const& in_vector)
    {
        PSYQ_ASSERT(base_type::check_unused_elements(in_vector));
        return D3DXVec3Length(&in_vector);
    }

    /// @copydoc psyq::geometry::coordinate::normalize_length()
    public: static typename base_type::vector normalize_length(
        typename base_type::vector const& in_vector)
    {
        PSYQ_ASSERT(this_type::check_unused_elements(in_vector));
        typename base_type::vector local_vector;
        D3DXVec3Normalize(&local_vector, &in_vector);
        return local_vector;
    }
    //@}

}; // class psyq::geometry::coordinate

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief D3DXVECTOR4 を使う座標系のテンプレート特殊化。
    @ingroup psyq_geometry_coordinate
 */
template<unsigned template_dimension>
class psyq::geometry::coordinate<D3DXVECTOR4, template_dimension>:
public psyq::geometry::d3dx_coordinate<D3DXVECTOR4, template_dimension>
{
    /// thisが指す値の型。
    private: typedef coordinate this_type;

    /// this_type の基底型。
    public: typedef psyq::geometry::d3dx_coordinate<vector, dimension>
        base_type;

    //-------------------------------------------------------------------------
    /// @name 座標の演算
    //@{
    /// @copydoc psyq::geometry::coordinate::dot_product()
    public: static typename base_type::element dot_product(
        typename base_type::vector const& in_left,
        typename base_type::vector const& in_right)
    {
        PSYQ_ASSERT(this_type::check_unused_elements(in_left, in_right));
        return D3DXVec4Dot(&in_left, &in_right);
    }

    /** @brief 3つの4次元幾何ベクトルの外積を算出する。
        @return 3つの4次元幾何ベクトルの外積。
        @param[in] in_left   外積の左辺となる3次元幾何ベクトル。
        @param[in] in_middle 外積の中辺となる3次元幾何ベクトル。
        @param[in] in_right  外積の右辺となる3次元幾何ベクトル。
        @note
            座標を表す幾何ベクトルの構築後に
            座標が使ってない幾何ベクトルの成分が変更されていた場合、
            正しい動作を保証できない。
     */
    public: static typename base_type::vector cross_product(
        typename base_type::vector const& in_left,
        typename base_type::vector const& in_middle,
        typename base_type::vector const& in_right)
    {
        static_assert(
            base_type::dimension == 4, "'base_type::dimension' is not 4.");
        typename base_type::vector local_cross;
        D3DXVec4Cross(&local_cross, &in_left, &in_middle, &in_right);
        return local_cross;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 座標の大きさ
    //@{
    /// @copydoc psyq::geometry::coordinate::compute_length()
    public: static typename base_type::element compute_length(
        typename base_type::vector const& in_vector)
    {
        PSYQ_ASSERT(base_type::check_unused_elements(in_vector));
        return D3DXVec4Length(&in_vector);
    }

    /// @copydoc psyq::geometry::coordinate::normalize_length()
    public: static typename base_type::vector normalize_length(
        typename base_type::vector const& in_vector)
    {
        PSYQ_ASSERT(this_type::check_unused_elements(in_vector));
        typename base_type::vector local_vector;
        D3DXVec4Normalize(&local_vector, &in_vector);
        return local_vector;
    }
    //@}

}; // class psyq::geometry::coordinate

#endif // !defined(PSYQ_GEOMETRY_D3DX_COORDINATE_HPP_)
