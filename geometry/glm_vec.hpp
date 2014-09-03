/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief OpenGL Mathematicsにテンプレート特殊化した幾何ベクトル関数群。
 */
#ifndef PSYQ_GEOMETRY_GLM_VEC_HPP_
#define PSYQ_GEOMETRY_GLM_VEC_HPP_

#include <glm/gtx/simd_vec4.hpp>

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @copydoc psyq::geometry::get_vector_element()
template<>
glm::vec2::value_type
psyq::geometry::get_vector_element<2, glm::vec2::value_type, glm::vec2>(
    glm::vec2 const& in_vector,
    unsigned const in_index)
{
    return in_vector[in_index];
}

/// @copydoc psyq::geometry::set_vector_element()
template<>
glm::vec2::value_type
psyq::geometry::set_vector_element<2, glm::vec2::value_type, glm::vec2>(
    glm::vec2& io_vector,
    unsigned const in_index,
    glm::vec2::value_type const in_value)
{
    io_vector[in_index] = in_value;
    return in_value;
}

/// @copydoc psyq::geometry::dot_product_vector()
template<>
glm::vec2::value_type
psyq::geometry::dot_product_vector<2, glm::vec2::value_type, glm::vec2>(
    glm::vec2 const& in_left,
    glm::vec2 const& in_right)
{
    return glm::dot(in_left, in_right);
}

/// @copydoc psyq::geometry::compute_vector_length()
template<>
glm::vec2::value_type
psyq::geometry::compute_vector_length<2, glm::vec2::value_type, glm::vec2>(
    glm::vec2 const& in_vector)
{
    return glm::length(in_vector);
}

/// @copydoc psyq::geometry::normalize_vector()
template<>
glm::vec2
psyq::geometry::normalize_vector<2, glm::vec2::value_type, glm::vec2>(
    glm::vec2 const& in_vector)
{
    return glm::normalize(in_vector);
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @copydoc psyq::geometry::get_vector_element()
template<>
glm::vec3::value_type
psyq::geometry::get_vector_element<3, glm::vec3::value_type, glm::vec3>(
    glm::vec3 const& in_vector,
    unsigned const in_index)
{
    return in_vector[in_index];
}

/// @copydoc psyq::geometry::set_vector_element()
template<>
glm::vec3::value_type
psyq::geometry::set_vector_element<3, glm::vec3::value_type, glm::vec3>(
    glm::vec3& io_vector,
    unsigned const in_index,
    glm::vec3::value_type const in_value)
{
    io_vector[in_index] = in_value;
    return in_value;
}

/// @copydoc psyq::geometry::dot_product_vector()
template<>
glm::vec3::value_type
psyq::geometry::dot_product_vector<3, glm::vec3::value_type, glm::vec3>(
    glm::vec3 const& in_left,
    glm::vec3 const& in_right)
{
    return glm::dot(in_left, in_right);
}

/// @copydoc psyq::geometry::cross_product_vector()
template<>
glm::vec3
psyq::geometry::cross_product_vector<glm::vec3::value_type, glm::vec3>(
    glm::vec3 const& in_left,
    glm::vec3 const& in_right)
{
    return glm::cross(in_left, in_right);
}

/// @copydoc psyq::geometry::compute_vector_length()
template<>
glm::vec3::value_type
psyq::geometry::compute_vector_length<3, glm::vec3::value_type, glm::vec3>(
    glm::vec3 const& in_vector)
{
    return glm::length(in_vector);
}

/// @copydoc psyq::geometry::normalize_vector()
template<>
glm::vec3
psyq::geometry::normalize_vector<3, glm::vec3::value_type, glm::vec3>(
    glm::vec3 const& in_vector)
{
    return glm::normalize(in_vector);
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @copydoc psyq::geometry::make_vector(template_element, template_element)
template<>
glm::simdVec4
psyq::geometry::make_vector<2, glm::simdVec4, glm::f32>(
    glm::f32 const in_element_0,
    glm::f32 const in_element_1)
{
    return glm::simdVec4(in_element_0, in_element_1, 0, 0);
}

/// @copydoc psyq::geometry::get_vector_element()
template<>
glm::f32
psyq::geometry::get_vector_element<2, glm::f32, glm::simdVec4>(
    glm::simdVec4 const& in_vector,
    unsigned const in_index)
{
    PSYQ_ASSERT(in_index < 2);
    return in_vector.Data.m128_f32[in_index];
}

/// @copydoc psyq::geometry::set_vector_element()
template<>
glm::f32
psyq::geometry::set_vector_element<2, glm::f32, glm::simdVec4>(
    glm::simdVec4& io_vector,
    unsigned const in_index,
    glm::f32 const in_value)
{
    PSYQ_ASSERT(in_index < 2);
    io_vector.Data.m128_f32[in_index] = in_value;
    return in_value;
}

/// @copydoc psyq::geometry::dot_product_vector()
template<>
glm::f32
psyq::geometry::dot_product_vector<2, glm::f32, glm::simdVec4>(
    glm::simdVec4 const& in_left,
    glm::simdVec4 const& in_right)
{
    PSYQ_ASSERT(in_left.Data.m128_f32[2] == 0);
    PSYQ_ASSERT(in_left.Data.m128_f32[3] == 0);
    PSYQ_ASSERT(in_right.Data.m128_f32[2] == 0);
    PSYQ_ASSERT(in_right.Data.m128_f32[3] == 0);
    return glm::dot(in_left, in_right);
}

/// @copydoc psyq::geometry::compute_vector_length()
template<>
glm::f32
psyq::geometry::compute_vector_length<2, glm::f32, glm::simdVec4>(
    glm::simdVec4 const& in_vector)
{
    PSYQ_ASSERT(in_vector.Data.m128_f32[2] == 0);
    PSYQ_ASSERT(in_vector.Data.m128_f32[3] == 0);
    return glm::length(in_vector);
}

/// @copydoc psyq::geometry::normalize_vector()
template<>
glm::simdVec4
psyq::geometry::normalize_vector<2, glm::f32, glm::simdVec4>(
    glm::simdVec4 const& in_vector)
{
    PSYQ_ASSERT(in_vector.Data.m128_f32[2] == 0);
    PSYQ_ASSERT(in_vector.Data.m128_f32[3] == 0);
    return glm::normalize(in_vector);
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @copydoc psyq::geometry::make_vector(template_element, template_element, template_element)
template<>
glm::simdVec4
psyq::geometry::make_vector<3, glm::simdVec4, glm::f32>(
    glm::f32 const in_element_0,
    glm::f32 const in_element_1,
    glm::f32 const in_element_2)
{
    return glm::simdVec4(in_element_0, in_element_1, in_element_2, 0);
}

/// @copydoc psyq::geometry::get_vector_element()
template<>
glm::f32
psyq::geometry::get_vector_element<3, glm::f32, glm::simdVec4>(
    glm::simdVec4 const& in_vector,
    unsigned const in_index)
{
    PSYQ_ASSERT(in_index < 3);
    return in_vector.Data.m128_f32[in_index];
}

/// @copydoc psyq::geometry::set_vector_element()
template<>
glm::f32
psyq::geometry::set_vector_element<3, glm::f32, glm::simdVec4>(
    glm::simdVec4& io_vector,
    unsigned const in_index,
    glm::f32 const in_value)
{
    PSYQ_ASSERT(in_index < 3);
    io_vector.Data.m128_f32[in_index] = in_value;
    return in_value;
}

/// @copydoc psyq::geometry::dot_product_vector()
template<>
glm::f32
psyq::geometry::dot_product_vector<3, glm::f32, glm::simdVec4>(
    glm::simdVec4 const& in_left,
    glm::simdVec4 const& in_right)
{
    PSYQ_ASSERT(in_left.Data.m128_f32[3] == 0);
    PSYQ_ASSERT(in_right.Data.m128_f32[3] == 0);
    return glm::dot(in_left, in_right);
}

/// @copydoc psyq::geometry::cross_product_vector()
template<>
glm::simdVec4
psyq::geometry::cross_product_vector<glm::f32, glm::simdVec4>(
    glm::simdVec4 const& in_left,
    glm::simdVec4 const& in_right)
{
    return glm::cross(in_left, in_right);
}

/// @copydoc psyq::geometry::compute_vector_length()
template<>
glm::f32
psyq::geometry::compute_vector_length<3, glm::f32, glm::simdVec4>(
    glm::simdVec4 const& in_vector)
{
    PSYQ_ASSERT(in_vector.Data.m128_f32[3] == 0);
    return glm::length(in_vector);
}

/// @copydoc psyq::geometry::normalize_vector()
template<>
glm::simdVec4
psyq::geometry::normalize_vector<3, glm::f32, glm::simdVec4>(
    glm::simdVec4 const& in_vector)
{
    PSYQ_ASSERT(in_vector.Data.m128_f32[3] == 0);
    return glm::normalize(in_vector);
}

#endif // !defined(PSYQ_GEOMETRY_GLM_VEC_HPP_)
