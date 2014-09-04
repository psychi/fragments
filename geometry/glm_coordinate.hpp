/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief OpenGL Mathematicsにテンプレート特殊化した座標系。
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
    /// 幾何ベクトルが持つ成分の数。
    public: enum: unsigned { size = 2 };
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
    /// 幾何ベクトルが持つ成分の数。
    public: enum: unsigned { size = 3 };
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
    /// 幾何ベクトルが持つ成分の数。
    public: enum: unsigned { size = 4 };
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
    /// 幾何ベクトルが持つ成分の数。
    public: enum: unsigned { size = 4 };
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

    //-------------------------------------------------------------------------
    /// @copydoc psyq::geometry::coordinate::get_element()
    public: static typename base_type::element get_element(
        typename base_type::vector const& in_vector,
        unsigned const in_index)
    {
        return in_vector[in_index];
    }

    /// @copydoc psyq::geometry::coordinate::set_element()
    public: static typename base_type::element set_element(
        typename base_type::vector& io_vector,
        unsigned const in_index,
        typename base_type::element const in_value)
    {
        io_vector[in_index] = in_value;
        return in_value;
    }

    //-------------------------------------------------------------------------
    /// @copydoc psyq::geometry::coordinate::dot_product()
    public: static typename base_type::element dot_product(
        typename base_type::vector const& in_left,
        typename base_type::vector const& in_right)
    {
        return glm::dot(in_left, in_right);
    }

    /// @copydoc psyq::geometry::coordinate::cross_product()
    public: static typename base_type::vector cross_product(
        typename base_type::vector const& in_left,
        typename base_type::vector const& in_right)
    {
        return glm::cross(in_left, in_right);
    }

    //-------------------------------------------------------------------------
    /// @copydoc psyq::geometry::coordinate::compute_length()
    public: static typename base_type::element compute_length(
        typename base_type::vector const& in_vector)
    {
        return glm::length(in_vector);
    }

    /// @copydoc psyq::geometry::coordinate::arrange_length()
    public: static typename base_type::vector arrange_length(
        typename base_type::vector const& in_vector,
        typename base_type::element const in_length)
    {
        return glm::normalize(in_vector) * in_length;
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
public psyq::geometry::coordinate_traits<glm::simdVec4, template_dimension>
{
    /// thisが指す値の型。
    private: typedef coordinate this_type;

    /// this_type の基底型。
    public: typedef psyq::geometry::coordinate_traits<vector, dimension>
        base_type;

    //-------------------------------------------------------------------------
    /// @copydoc psyq::geometry::coordinate::get_element()
    public: static typename base_type::element get_element(
        typename base_type::vector const& in_vector,
        unsigned const in_index)
    {
        PSYQ_ASSERT(in_index < base_type::dimension);
        return in_vector.Data.m128_f32[in_index];
    }

    /// @copydoc psyq::geometry::coordinate::set_element()
    public: static typename base_type::element set_element(
        typename base_type::vector& io_vector,
        unsigned const in_index,
        typename base_type::element const in_value)
    {
        PSYQ_ASSERT(in_index < base_type::dimension);
        io_vector.Data.m128_f32[in_index] = in_value;
        return in_value;
    }

    //-------------------------------------------------------------------------
    /// @copydoc psyq::geometry::coordinate::dot_product()
    public: static typename base_type::element dot_product(
        typename base_type::vector const& in_left,
        typename base_type::vector const& in_right)
    {
        PSYQ_ASSERT(
            2 < base_type::dimension ||
            in_left.Data.m128_f32[2] == 0 ||
            in_right.Data.m128_f32[2] == 0);
        PSYQ_ASSERT(
            3 < base_type::dimension ||
            in_left.Data.m128_f32[3] == 0 ||
            in_right.Data.m128_f32[3] == 0);
        return glm::dot(in_left, in_right);
    }

    /// @copydoc psyq::geometry::coordinate::cross_product()
    public: static typename base_type::vector cross_product(
        typename base_type::vector const& in_left,
        typename base_type::vector const& in_right)
    {
        static_assert(2 < base_type::dimension, "");
        PSYQ_ASSERT(
            3 < base_type::dimension ||
            in_left.Data.m128_f32[3] == 0 ||
            in_right.Data.m128_f32[3] == 0);
        return glm::cross(in_left, in_right);
    }

    //-------------------------------------------------------------------------
    /// @copydoc psyq::geometry::coordinate::compute_length()
    public: static typename base_type::element compute_length(
        typename base_type::vector const& in_vector)
    {
        PSYQ_ASSERT(
            2 < base_type::dimension || in_vector.Data.m128_f32[2] == 0);
        PSYQ_ASSERT(
            3 < base_type::dimension || in_vector.Data.m128_f32[3] == 0);
        return glm::length(in_vector);
    }

    /// @copydoc psyq::geometry::coordinate::arrange_length()
    public: static typename base_type::vector arrange_length(
        typename base_type::vector const& in_vector,
        typename base_type::element const in_length)
    {
        PSYQ_ASSERT(
            2 < base_type::dimension || in_vector.Data.m128_f32[2] == 0);
        PSYQ_ASSERT(
            3 < base_type::dimension || in_vector.Data.m128_f32[3] == 0);
        return glm::normalize(in_vector) * in_length;
    }

}; // class psyq::geometry::coordinate

#endif // !defined(PSYQ_GEOMETRY_GLM_COORDINATE_HPP_)
