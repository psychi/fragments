/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief OpenGL Mathematics glm::vecにテンプレート特殊化した幾何ベクトル関数群。
 */
#ifndef PSYQ_GEOMETRY_GLM_VEC_HPP_
#define PSYQ_GEOMETRY_GLM_VEC_HPP_

#include <glm/glm.hpp> // OpenGL Mathematics

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

#endif // !defined(PSYQ_GEOMETRY_GLM_VEC_HPP_)
