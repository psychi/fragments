/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief OpenGL Mathematics (http://glm.g-truc.net)
           にテンプレート特殊化した幾何ベクトル演算。
 */
#ifndef PSYQ_GEOMETRY_GLM_VECTOR_HPP_
#define PSYQ_GEOMETRY_GLM_VECTOR_HPP_

#include <glm/gtx/simd_vec4.hpp>
//#include "psyq/geometry/vector.hpp"

/// @cond
namespace psyq
{
    namespace geometry
    {
        template<typename> class glm_vector_processor;
    } // namespace geometry
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief glm::tvec2 にテンプレート特殊化した幾何ベクトル型特性。
    @ingroup psyq_geometry_vector
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
    @ingroup psyq_geometry_vector
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
    @ingroup psyq_geometry_vector
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

#if GLM_ARCH != GLM_ARCH_PURE
/** @brief glm::simdVec4 にテンプレート特殊化した幾何ベクトル型特性。
    @ingroup psyq_geometry_vector
 */
template<> class psyq::geometry::vector_traits<glm::simdVec4>
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
#endif // GLM_ARCH != GLM_ARCH_PURE

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<typename template_vector>
class psyq::geometry::glm_vector_processor:
public psyq::geometry::generic_vector_processor<template_vector>
{
    /// thisが指す値の型。
    private: typedef glm_vector_processor this_type;

    /// this_type の基底型。
    public: typedef psyq::geometry::generic_vector_processor<template_vector>
        base_type;

    //-------------------------------------------------------------------------
    /// @name 幾何ベクトルの成分
    //@{
    /// @copydoc base_type::at
    public: static typename base_type::traits::element& at(
        typename base_type::traits::type& io_vector,
        unsigned const in_index)
    {
        return io_vector[in_index];
    }

    /// @copydoc base_type::const_at
    public: static typename base_type::traits::element const& const_at(
        typename base_type::traits::type const& in_vector,
        unsigned const in_index)
    {
        return in_vector[in_index];
    }
    //@}

}; // class psyq::geometry::glm_vector_processor


//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<typename template_element, glm::precision template_precision>
class psyq::geometry::vector_processor<
    glm::detail::tvec3<template_element, template_precision>>:
public psyq::geometry::glm_vector_processor<
    glm::detail::tvec3<template_element, template_precision>>
{
    /// thisが指す値の型。
    private: typedef vector_processor this_type;

    /// this_type の基底型。
    public: psyq::geometry::glm_vector_processor<
        glm::detail::tvec3<template_element, template_precision>>
            base_type;

}; // class psyq::geometry::vector_processor

#if 0
//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
//-----------------------------------------------------------------------------
/// @copydoc psyq::geometry::vector::at
template<>
inline typename psyq::geometry::vector_traits<glm::vec2>::element&
psyq::geometry::vector::at<glm::vec2>(
    glm::vec2& io_vector,
    unsigned const in_index)
{
    return io_vector[in_index];
}

//-----------------------------------------------------------------------------
/// @copydoc psyq::geometry::vector::dot
template<>
inline typename psyq::geometry::vector_traits<glm::vec2>::element
psyq::geometry::vector::dot<glm::vec2>(
    glm::vec2 const& in_left,
    glm::vec2 const& in_right)
{
    return glm::dot(in_left, in_right);
}

//-------------------------------------------------------------------------
/// @copydoc psyq::geometry::vector::length
template<>
inline typename psyq::geometry::vector_traits<glm::vec2>::element
psyq::geometry::vector::length<glm::vec2>(
    glm::vec2 const& in_vector)
{
    return glm::length(in_vector);
}

/// @copydoc psyq::geometry::vector::normalize
template<>
inline glm::vec2 psyq::geometry::vector::normalize<glm::vec2>(
    glm::vec2 const& in_vector)
{
    return glm::normalize(in_vector);
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
//-----------------------------------------------------------------------------
/// @copydoc psyq::geometry::vector::at
template<>
inline typename psyq::geometry::vector_traits<glm::vec3>::element&
psyq::geometry::vector::at<glm::vec3>(
    glm::vec3& io_vector,
    unsigned const in_index)
{
    return io_vector[in_index];
}

//-----------------------------------------------------------------------------
/// @copydoc psyq::geometry::vector::dot
template<>
inline typename psyq::geometry::vector_traits<glm::vec3>::element
psyq::geometry::vector::dot<glm::vec3>(
    glm::vec3 const& in_left,
    glm::vec3 const& in_right)
{
    return glm::dot(in_left, in_right);
}

/// @copydoc psyq::geometry::vector::cross_3d
template<>
inline glm::vec3 psyq::geometry::vector::cross_3d<glm::vec3>(
    glm::vec3 const& in_left,
    glm::vec3 const& in_right)
{
    return glm::cross(in_left, in_right);
}

//-------------------------------------------------------------------------
/// @copydoc psyq::geometry::vector::length
template<>
inline typename psyq::geometry::vector_traits<glm::vec3>::element
psyq::geometry::vector::length<glm::vec3>(
    glm::vec3 const& in_vector)
{
    return glm::length(in_vector);
}

/// @copydoc psyq::geometry::vector::normalize
template<>
inline glm::vec3 psyq::geometry::vector::normalize<glm::vec3>(
    glm::vec3 const& in_vector)
{
    return glm::normalize(in_vector);
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
//-----------------------------------------------------------------------------
/// @copydoc psyq::geometry::vector::at
template<>
inline typename psyq::geometry::vector_traits<glm::vec4>::element&
psyq::geometry::vector::at<glm::vec4>(
    glm::vec4& io_vector,
    unsigned const in_index)
{
    return io_vector[in_index];
}

//-----------------------------------------------------------------------------
/// @copydoc psyq::geometry::vector::dot
template<>
inline typename psyq::geometry::vector_traits<glm::vec4>::element
psyq::geometry::vector::dot<glm::vec4>(
    glm::vec4 const& in_left,
    glm::vec4 const& in_right)
{
    return glm::dot(in_left, in_right);
}

//-------------------------------------------------------------------------
/// @copydoc psyq::geometry::vector::length
template<>
inline typename psyq::geometry::vector_traits<glm::vec4>::element
psyq::geometry::vector::length<glm::vec4>(
    glm::vec4 const& in_vector)
{
    return glm::length(in_vector);
}

/// @copydoc psyq::geometry::vector::normalize
template<>
inline glm::vec4 psyq::geometry::vector::normalize<glm::vec4>(
    glm::vec4 const& in_vector)
{
    return glm::normalize(in_vector);
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
//-----------------------------------------------------------------------------
/// @copydoc psyq::geometry::vector::at
template<>
inline glm::f32& psyq::geometry::vector::at<glm::simdVec4>(
    glm::simdVec4& io_vector,
    unsigned const in_index)
{
    typedef psyq::geometry::vector_traits<glm::simdVec4> vector_traits;
    PSYQ_ASSERT(in_index < vector_traits::size);
    return io_vector.Data.m128_f32[in_index];
}

//-----------------------------------------------------------------------------
/// @copydoc psyq::geometry::vector::dot
template<>
inline typename psyq::geometry::vector_traits<glm::simdVec4>::element
psyq::geometry::vector::dot<glm::simdVec4>(
    glm::simdVec4 const& in_left,
    glm::simdVec4 const& in_right)
{
    return glm::dot(in_left, in_right);
}

/// @copydoc psyq::geometry::vector::cross_3d
template<>
inline glm::simdVec4 psyq::geometry::vector::cross_3d<glm::simdVec4>(
    glm::simdVec4 const& in_left,
    glm::simdVec4 const& in_right)
{
    return glm::cross(in_left, in_right);
}

//-------------------------------------------------------------------------
/// @copydoc psyq::geometry::vector::length
template<>
inline typename psyq::geometry::vector_traits<glm::simdVec4>::element
psyq::geometry::vector::length<glm::simdVec4>(
    glm::simdVec4 const& in_vector)
{
    return glm::length(in_vector);
}

/// @copydoc psyq::geometry::vector::normalize
template<>
inline glm::simdVec4 psyq::geometry::vector::normalize<glm::simdVec4>(
    glm::simdVec4 const& in_vector)
{
    return glm::normalize(in_vector);
}
#endif

#endif // !defined(PSYQ_GEOMETRY_GLM_VECTOR_HPP_)
