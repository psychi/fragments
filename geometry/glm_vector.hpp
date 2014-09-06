/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief OpenGL Mathematics (http://glm.g-truc.net)
           にテンプレート特殊化した幾何ベクトル演算。
 */
#ifndef PSYQ_GEOMETRY_GLM_VECTOR_HPP_
#define PSYQ_GEOMETRY_GLM_VECTOR_HPP_

#include <glm/gtx/simd_vec4.hpp>
//#include "psyq/geometry/vector.hpp"

namespace psyq
{
namespace geometry
{
namespace vector
{

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief glm::tvec2 にテンプレート特殊化した幾何ベクトル型特性。
    @ingroup psyq_geometry_vector
 */
template<typename template_element, glm::precision template_precision>
class traits<glm::detail::tvec2<template_element, template_precision>>
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

//-----------------------------------------------------------------------------
/// @name 幾何ベクトルの成分
//@{
/// @copydoc psyq::geometry::vector::at
template<>
inline typename traits<glm::vec2>::element& at<glm::vec2>(
    glm::vec2& io_vector,
    unsigned const in_index)
{
    return io_vector[in_index];
}
//@}
//-----------------------------------------------------------------------------
/// @name 幾何ベクトルの演算
//@{
/// @copydoc psyq::geometry::vector::dot
template<>
inline typename traits<glm::vec2>::element dot<glm::vec2>(
    glm::vec2 const& in_left,
    glm::vec2 const& in_right)
{
    return glm::dot(in_left, in_right);
}
//@}
//-------------------------------------------------------------------------
/// @name 幾何ベクトルの大きさ
//@{
/// @copydoc psyq::geometry::vector::length
template<>
inline typename traits<glm::vec2>::element length<glm::vec2>(
    glm::vec2 const& in_vector)
{
    return glm::length(in_vector);
}

/// @copydoc psyq::geometry::vector::normalize
template<>
inline glm::vec2 normalize<glm::vec2>(glm::vec2 const& in_vector)
{
    return glm::normalize(in_vector);
}
//@}
//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief glm::tvec3 にテンプレート特殊化した幾何ベクトル型特性。
    @ingroup psyq_geometry_vector
 */
template<typename template_element, glm::precision template_precision>
class traits<glm::detail::tvec3<template_element, template_precision>>
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

//-----------------------------------------------------------------------------
/// @name 幾何ベクトルの成分
//@{
/// @copydoc psyq::geometry::vector::at
template<>
inline typename traits<glm::vec3>::element& at<glm::vec3>(
    glm::vec3& io_vector,
    unsigned const in_index)
{
    return io_vector[in_index];
}
//@}
//-----------------------------------------------------------------------------
/// @name 幾何ベクトルの演算
//@{
/// @copydoc psyq::geometry::vector::dot
template<>
inline typename traits<glm::vec3>::element dot<glm::vec3>(
    glm::vec3 const& in_left,
    glm::vec3 const& in_right)
{
    return glm::dot(in_left, in_right);
}

/// @copydoc psyq::geometry::vector::cross_3d
template<>
inline glm::vec3 cross_3d<glm::vec3>(
    glm::vec3 const& in_left,
    glm::vec3 const& in_right)
{
    return glm::cross(in_left, in_right);
}
//@}
//-------------------------------------------------------------------------
/// @name 幾何ベクトルの大きさ
//@{
/// @copydoc psyq::geometry::vector::length
template<>
inline typename traits<glm::vec3>::element length<glm::vec3>(
    glm::vec3 const& in_vector)
{
    return glm::length(in_vector);
}

/// @copydoc psyq::geometry::vector::normalize
template<>
inline glm::vec3 normalize<glm::vec3>(glm::vec3 const& in_vector)
{
    return glm::normalize(in_vector);
}
//@}
//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief glm::tvec4 にテンプレート特殊化した幾何ベクトル型特性。
    @ingroup psyq_geometry_vector
 */
template<typename template_element, glm::precision template_precision>
class traits<glm::detail::tvec4<template_element, template_precision>>
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

//-----------------------------------------------------------------------------
/// @name 幾何ベクトルの成分
//@{
/// @copydoc psyq::geometry::vector::at
template<>
inline typename traits<glm::vec4>::element& at<glm::vec4>(
    glm::vec4& io_vector,
    unsigned const in_index)
{
    return io_vector[in_index];
}
//@}
//-----------------------------------------------------------------------------
/// @name 幾何ベクトルの演算
//@{
/// @copydoc psyq::geometry::vector::dot
template<>
inline typename traits<glm::vec4>::element dot<glm::vec4>(
    glm::vec4 const& in_left,
    glm::vec4 const& in_right)
{
    return glm::dot(in_left, in_right);
}
//@}
//-------------------------------------------------------------------------
/// @name 幾何ベクトルの大きさ
//@{
/// @copydoc psyq::geometry::vector::length
template<>
inline typename traits<glm::vec4>::element length<glm::vec4>(
    glm::vec4 const& in_vector)
{
    return glm::length(in_vector);
}

/// @copydoc psyq::geometry::vector::normalize
template<>
inline glm::vec4 normalize<glm::vec4>(glm::vec4 const& in_vector)
{
    return glm::normalize(in_vector);
}
//@}
//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
#if GLM_ARCH != GLM_ARCH_PURE
/** @brief glm::simdVec4 にテンプレート特殊化した幾何ベクトル型特性。
    @ingroup psyq_geometry_vector
 */
template<>
class traits<glm::simdVec4>
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

//-----------------------------------------------------------------------------
/// @name 幾何ベクトルの成分
//@{
/// @copydoc psyq::geometry::vector::at
template<>
inline glm::f32& at<glm::simdVec4>(
    glm::simdVec4& io_vector,
    unsigned const in_index)
{
    typedef psyq::geometry::vector::traits<glm::simdVec4> vector_traits;
    PSYQ_ASSERT(in_index < vector_traits::size);
    return io_vector.Data.m128_f32[in_index];
}
//@}
//-----------------------------------------------------------------------------
/// @name 幾何ベクトルの演算
//@{
/// @copydoc psyq::geometry::vector::dot
template<>
inline typename traits<glm::simdVec4>::element dot<glm::simdVec4>(
    glm::simdVec4 const& in_left,
    glm::simdVec4 const& in_right)
{
    return glm::dot(in_left, in_right);
}

/// @copydoc psyq::geometry::vector::cross_3d
template<>
inline glm::simdVec4 cross_3d<glm::simdVec4>(
    glm::simdVec4 const& in_left,
    glm::simdVec4 const& in_right)
{
    return glm::cross(in_left, in_right);
}
//@}
//-------------------------------------------------------------------------
/// @name 幾何ベクトルの大きさ
//@{
/// @copydoc psyq::geometry::vector::length
template<>
inline typename traits<glm::simdVec4>::element length<glm::simdVec4>(
    glm::simdVec4 const& in_vector)
{
    return glm::length(in_vector);
}

/// @copydoc psyq::geometry::vector::normalize
template<>
inline glm::simdVec4 normalize<glm::simdVec4>(glm::simdVec4 const& in_vector)
{
    return glm::normalize(in_vector);
}
#endif // GLM_ARCH != GLM_ARCH_PURE
//@}
} // namespace vector
} // namespace geometry
} // namespace psyq

#endif // !defined(PSYQ_GEOMETRY_GLM_VECTOR_HPP_)
