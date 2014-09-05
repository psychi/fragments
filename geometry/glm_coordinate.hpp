/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief OpenGL Mathematics にテンプレート特殊化した座標系。
 */
#ifndef PSYQ_GEOMETRY_GLM_COORDINATE_HPP_
#define PSYQ_GEOMETRY_GLM_COORDINATE_HPP_

#include <glm/gtx/simd_vec4.hpp>
//#include "psyq/geometry/coordinate.hpp"

/// @cond
namespace psyq
{
    namespace geometry
    {
        template<typename, unsigned> class glm_coordinate;
    } // namespace geometry
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief glm::tvec2 にテンプレート特殊化した幾何ベクトル型特性。
    @ingroup psyq_geometry_vector_traits
 */
template<typename template_element, glm::precision template_precision>
class psyq::geometry::vector_traits<
    glm::detail::tvec2<template_element, template_precision>>
{
    /// 幾何ベクトルの型。
    public: typedef glm::detail::tvec2<template_element, template_precision>
        type;
    /// 幾何ベクトルが持つ成分の型。
    public: typedef template_element element;

    public: enum: unsigned
    {
        /// 幾何ベクトルが持つ成分の数。
        size = 2,
    };
};

/** @brief glm::tvec3 にテンプレート特殊化した幾何ベクトル型特性。
    @ingroup psyq_geometry_vector_traits
 */
template<typename template_element, glm::precision template_precision>
class psyq::geometry::vector_traits<
    glm::detail::tvec3<template_element, template_precision>>
{
    /// 幾何ベクトルの型。
    public: typedef glm::detail::tvec3<template_element, template_precision>
        type;
    /// 幾何ベクトルが持つ成分の型。
    public: typedef template_element element;

    public: enum: unsigned
    {
        /// 幾何ベクトルが持つ成分の数。
        size = 3,
    };
};

/** @brief glm::tvec4 にテンプレート特殊化した幾何ベクトル型特性。
    @ingroup psyq_geometry_vector_traits
 */
template<typename template_element, glm::precision template_precision>
class psyq::geometry::vector_traits<
    glm::detail::tvec4<template_element, template_precision>>
{
    /// 幾何ベクトルの型。
    public: typedef glm::detail::tvec4<template_element, template_precision>
        type;
    /// 幾何ベクトルが持つ成分の型。
    public: typedef template_element element;

    public: enum: unsigned
    {
        /// 幾何ベクトルが持つ成分の数。
        size = 4,
    };
};

/** @brief glm::simdVec4 にテンプレート特殊化した幾何ベクトル型特性。
    @ingroup psyq_geometry_vector_traits
 */
template<>
class psyq::geometry::vector_traits<glm::simdVec4>
{
    /// 幾何ベクトルの型。
    public: typedef glm::simdVec4 type;
    /// 幾何ベクトルが持つ成分の型。
    public: typedef glm::f32 element;

    public: enum: unsigned
    {
        /// 幾何ベクトルが持つ成分の数。
        size = 4,
    };
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief OpenGL Mathematics のベクトルを使う座標系のベクトル処理。
    @ingroup psyq_geometry_coordinate
 */
template<typename template_vector, unsigned template_dimension>
class psyq::geometry::glm_coordinate:
public psyq::geometry::coordinate_traits<template_vector, template_dimension>
{
    /// thisが指す値の型。
    private: typedef glm_coordinate this_type;

    /// this_type の基底型。
    public: typedef psyq::geometry::coordinate_traits<vector, dimension>
        base_type;
    static_assert(
        template_dimension <= 4, "'base_type::dimension' is greater than 4.");

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
    private: static typename base_type::element const& vector_at(
        typename glm::simdVec4 const& in_vector,
        unsigned const in_index)
    {
        PSYQ_ASSERT(in_index < 4);
        return in_vector.Data.m128_f32[in_index];
    }

    private: template<typename template_glm_vec>
    static typename base_type::element const& vector_at(
        typename template_glm_vec const& in_vector,
        unsigned const in_index)
    {
        return in_vector[in_index];
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
        //PSYQ_ASSERT(this_type::check_unused_elements(in_left, in_right));
        //return glm::all(glm::lessThanEqual(in_left, in_right));
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 座標の演算
    //@{
    /// @copydoc psyq::geometry::coordinate::dot_product()
    public: static typename base_type::element dot_product(
        typename base_type::vector const& in_left,
        typename base_type::vector const& in_right)
    {
        PSYQ_ASSERT(this_type::check_unused_elements(in_left, in_right));
        return glm::dot(in_left, in_right);
    }

    /// @copydoc psyq::geometry::coordinate::cross_product()
    public: static typename base_type::vector cross_product(
        typename base_type::vector const& in_left,
        typename base_type::vector const& in_right)
    {
        static_assert(
            base_type::dimension == 3, "'base_type::dimension' is not 3.");
        PSYQ_ASSERT(this_type::check_unused_elements(in_left, in_right));
        return glm::cross(in_left, in_right);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 座標の大きさ
    //@{
    /// @copydoc psyq::geometry::coordinate::compute_length()
    public: static typename base_type::element compute_length(
        typename base_type::vector const& in_vector)
    {
        PSYQ_ASSERT(this_type::check_unused_elements(in_vector));
        return glm::length(in_vector);
    }

    /** @brief 正規化した幾何ベクトルを算出する。
        @return 正規化した幾何ベクトル。
        @param[in] in_vector 元となる幾何ベクトル。
        @note
            座標を表す幾何ベクトルの構築後に
            座標が使ってない幾何ベクトルの成分が変更されていた場合、
            正しい動作を保証できない。
     */
    public: static typename base_type::vector arrange_length(
        typename base_type::vector const& in_vector)
    {
        PSYQ_ASSERT(this_type::check_unused_elements(in_vector));
        return glm::normalize(in_vector);
    }

    /// @copydoc psyq::geometry::coordinate::arrange_length()
    public: static typename base_type::vector arrange_length(
        typename base_type::vector const& in_vector,
        typename base_type::element const in_length)
    {
        return this_type::arrange_length(in_vector) * in_length;
    }
    //@}
    //-------------------------------------------------------------------------
    private: static bool check_unused_elements(
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

    private: static bool check_unused_elements(
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

}; // class psyq::geometry::glm_coordinate

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief glm::tvec2 を使う座標系のテンプレート特殊化。
    @ingroup psyq_geometry_coordinate
 */
template<
    typename template_element,
    glm::precision template_precision,
    unsigned template_dimension>
class psyq::geometry::coordinate<
    glm::detail::tvec2<template_element, template_precision>,
    template_dimension>:
public psyq::geometry::glm_coordinate<
    glm::detail::tvec2<template_element, template_precision>,
    template_dimension>
{
    /// thisが指す値の型。
    private: typedef coordinate this_type;

    /// this_type の基底型。
    public: typedef psyq::geometry::glm_coordinate<vector, dimension>
        base_type;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief glm::tvec3 を使う座標系のテンプレート特殊化。
    @ingroup psyq_geometry_coordinate
 */
template<
    typename template_element,
    glm::precision template_precision,
    unsigned template_dimension>
class psyq::geometry::coordinate<
    glm::detail::tvec3<template_element, template_precision>,
    template_dimension>:
public psyq::geometry::glm_coordinate<
    glm::detail::tvec3<template_element, template_precision>,
    template_dimension>
{
    /// thisが指す値の型。
    private: typedef coordinate this_type;

    /// this_type の基底型。
    public: typedef psyq::geometry::glm_coordinate<vector, dimension>
        base_type;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief glm::tvec4 を使う座標系のテンプレート特殊化。
    @ingroup psyq_geometry_coordinate
 */
template<
    typename template_element,
    glm::precision template_precision,
    unsigned template_dimension>
class psyq::geometry::coordinate<
    glm::detail::tvec4<template_element, template_precision>,
    template_dimension>:
public psyq::geometry::glm_coordinate<
    glm::detail::tvec4<template_element, template_precision>,
    template_dimension>
{
    /// thisが指す値の型。
    private: typedef coordinate this_type;

    /// this_type の基底型。
    public: typedef psyq::geometry::glm_coordinate<vector, dimension>
        base_type;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief glm::simdVec4 を使う座標系のテンプレート特殊化。
    @ingroup psyq_geometry_coordinate
 */
template<unsigned template_dimension>
class psyq::geometry::coordinate<glm::simdVec4, template_dimension>:
public psyq::geometry::glm_coordinate<glm::simdVec4, template_dimension>
{
    /// thisが指す値の型。
    private: typedef coordinate this_type;

    /// this_type の基底型。
    public: typedef psyq::geometry::coordinate_traits<vector, dimension>
        base_type;

}; // class psyq::geometry::coordinate

#endif // !defined(PSYQ_GEOMETRY_GLM_COORDINATE_HPP_)
