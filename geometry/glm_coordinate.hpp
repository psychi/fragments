/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief OpenGL Mathematicsにテンプレート特殊化した座標系。
 */
#ifndef PSYQ_GEOMETRY_GLM_COORDINATE_HPP_
#define PSYQ_GEOMETRY_GLM_COORDINATE_HPP_

#include <glm/gtx/simd_vec4.hpp>
//#include "psyq/geometry/coordinate.hpp"

namespace psyq
{
    namespace geometry
    {
        template<typename, unsigned> class glm_coordinate;
        template<typename> class glm_coordinate_2d;
        template<typename> class glm_coordinate_3d;
    } // namespace geometry
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<typename template_element, glm::precision P>
class psyq::geometry::vector_traits<glm::detail::tvec2<template_element, P>>
{
    public: typedef glm::detail::tvec2<template_element, P> type;
    public: typedef template_element element;
    public: enum: unsigned { size = 2 };
};

template<typename template_element, glm::precision P>
class psyq::geometry::vector_traits<glm::detail::tvec3<template_element, P>>
{
    public: typedef glm::detail::tvec3<template_element, P> type;
    public: typedef template_element element;
    public: enum: unsigned { size = 3 };
};

template<typename template_element, glm::precision P>
class psyq::geometry::vector_traits<glm::detail::tvec4<template_element, P>>
{
    public: typedef glm::detail::tvec4<template_element, P> type;
    public: typedef template_element element;
    public: enum: unsigned { size = 4 };
};

template<>
class psyq::geometry::vector_traits<glm::simdVec4>
{
    public: typedef glm::simdVec4 type;
    public: typedef glm::f32 element;
    public: enum: unsigned { size = 4 };
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief OpenGL Mathematics ベクトルを座標として使う座標系の基底型。
 */
template<typename template_vector_traits, unsigned template_dimension>
class psyq::geometry::glm_coordinate:
    public psyq::geometry::coordinate<template_vector_traits, template_dimension>
{
    /// thisが指す値の型。
    private: typedef glm_coordinate this_type;

    /// this_type の基底型。
    public: typedef psyq::geometry::coordinate<
        template_vector_traits, template_dimension>
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
/** @brief glm::simdVec4 を座標として使う座標系。
 */
template<unsigned template_dimension>
class psyq::geometry::glm_coordinate<psyq::geometry::vector_traits<glm::simdVec4>, template_dimension>:
    public psyq::geometry::coordinate<psyq::geometry::vector_traits<glm::simdVec4>, template_dimension>
{
    /// thisが指す値の型。
    private: typedef glm_coordinate this_type;

    /// this_type の基底型。
    public: typedef psyq::geometry::coordinate
        <psyq::geometry::vector_traits<glm::simdVec4>, template_dimension>
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
        PSYQ_ASSERT(2 < base_type::dimension || in_left.Data.m128_f32[2] == 0 || in_right.Data.m128_f32[2] == 0);
        PSYQ_ASSERT(3 < base_type::dimension || in_left.Data.m128_f32[3] == 0 || in_right.Data.m128_f32[3] == 0);
        return glm::dot(in_left, in_right);
    }

    /// @copydoc psyq::geometry::coordinate::cross_product()
    public: static typename base_type::vector cross_product(
        typename base_type::vector const& in_left,
        typename base_type::vector const& in_right)
    {
        static_assert(2 < base_type::dimension, "");
        PSYQ_ASSERT(3 < base_type::dimension || in_left.Data.m128_f32[3] == 0 || in_right.Data.m128_f32[3] == 0);
        return glm::cross(in_left, in_right);
    }

    //-------------------------------------------------------------------------
    /// @copydoc psyq::geometry::coordinate::compute_length()
    public: static typename base_type::element compute_length(
        typename base_type::vector const& in_vector)
    {
        PSYQ_ASSERT(2 < base_type::dimension || in_vector.Data.m128_f32[2] == 0);
        PSYQ_ASSERT(3 < base_type::dimension || in_vector.Data.m128_f32[3] == 0);
        return glm::length(in_vector);
    }

    /// @copydoc psyq::geometry::coordinate::arrange_length()
    public: static typename base_type::vector arrange_length(
        typename base_type::vector const& in_vector,
        typename base_type::element const in_length)
    {
        PSYQ_ASSERT(2 < base_type::dimension || in_vector.Data.m128_f32[2] == 0);
        PSYQ_ASSERT(3 < base_type::dimension || in_vector.Data.m128_f32[3] == 0);
        return glm::normalize(in_vector) * in_length;
    }

}; // class psyq::geometry::glm_coordinate

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief OpenGL Mathematics ベクトルを座標として使う2次元座標系。
 */
template<typename template_vector_traits>
class psyq::geometry::glm_coordinate_2d:
    public psyq::geometry::glm_coordinate<template_vector_traits, 2>
{
    /// thisが指す値の型。
    private: typedef glm_coordinate_2d this_type;

    /// this_type の基底型。
    public: typedef psyq::geometry::glm_coordinate<template_vector_traits, 2>
        base_type;

    //-------------------------------------------------------------------------
    /// @copydoc psyq::geometry::coordinate_2d::make(element, element)
    public: static typename base_type::vector make(
        typename base_type::element const in_element_0,
        typename base_type::element const in_element_1)
    {
        return psyq::geometry::_private::vector_maker
            <typename base_type::vector, base_type::vector_traits::size>
                ::make(in_element_0, in_element_1);
    }

    /// @copydoc psyq::geometry::coordinate_2d::make(element)
    public: static typename base_type::vector make(
        typename base_type::element const in_element)
    {
        return this_type::make(in_element, in_element);
    }

    /// @copydoc psyq::geometry::coordinate_2d::make(template_container const&)
    public: template<typename template_container>
    static typename this_type::vector make(
        template_container const& in_container)
    {
        return this_type::make(in_container[0], in_container[1]);
    }

}; // class psyq::geometry::glm_coordinate_2d

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief OpenGL Mathematics ベクトルを座標として使う3次元座標系。
 */
template<typename template_vector_traits>
class psyq::geometry::glm_coordinate_3d:
    public psyq::geometry::glm_coordinate<template_vector_traits, 3>
{
    /// thisが指す値の型。
    private: typedef glm_coordinate_3d this_type;

    /// this_type の基底型。
    public: typedef psyq::geometry::glm_coordinate<template_vector_traits, 3>
        base_type;

    //-------------------------------------------------------------------------
    /// @copydoc psyq::geometry::coordinate_3d::make(element, element, element)
    public: static typename base_type::vector make(
        typename base_type::element const in_element_0,
        typename base_type::element const in_element_1,
        typename base_type::element const in_element_2)
    {
        return psyq::geometry::_private::vector_maker
            <typename base_type::vector, base_type::vector_traits::size>
                ::make(in_element_0, in_element_1, in_element_2);
    }

    /// @copydoc psyq::geometry::coordinate_3d::make(element)
    public: static typename base_type::vector make(
        typename base_type::element const in_element)
    {
        return this_type::make(in_element, in_element, in_element);
    }

    /// @copydoc psyq::geometry::coordinate_3d::make(template_container const&)
    public: template<typename template_container>
    static typename this_type::vector make(
        template_container const& in_container)
    {
        return this_type::make(
            in_container[0], in_container[1], in_container[2]);
    }

}; // class psyq::geometry::coordinate_3d

#endif // !defined(PSYQ_GEOMETRY_GLM_COORDINATE_HPP_)
