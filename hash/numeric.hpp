/// @file
/// @brief @copybrief psyq::hash::numeric_hash
/// @author Hillco Psychi (https://twitter.com/psychi)
#ifndef PSYQ_HASH_NUMERIC_HPP_
#define PSYQ_HASH_NUMERIC_HPP_

#include "../bit_algorithm.hpp"

namespace psyq
{
    namespace hash
    {
        template<typename, typename> class numeric_hash;
    }
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 数値のバイト列を使うハッシュ関数オブジェクト。
/// @tparam template_number @copydoc numeric_hash::argument_type
/// @tparam template_result @copydoc numeric_hash::result_type
template<typename template_number, typename template_result = std::size_t>
class psyq::hash::numeric_hash
{
    /// @copydoc psyq::string::view::this_type
    private: typedef numeric_hash this_type;

    //-------------------------------------------------------------------------
    /// @brief ハッシュ関数の引数となる数値の型。
    public: typedef template_number argument_type;
    static_assert(
        std::is_arithmetic<template_number>::value,
        "'template_number' is not arithmetic type.");

    /// @brief ハッシュ関数の戻り値の型。
    /// @details
    /// std::size_t に暗黙の型変換が可能なら、
    /// this_type は std::hash の上位互換性を持つ。
    public: typedef template_result result_type;

    //-------------------------------------------------------------------------
    /// @brief 数値のハッシュ値を取得する。
    /// @param[in] in_number 数値。
    /// @return in_integer
    public: typename this_type::result_type operator()(
        typename this_type::argument_type const in_number)
    const PSYQ_NOEXCEPT
    {
        return this_type::get_bit_field(
            in_number,
            std::is_floating_point<typename this_type::argument_type>());
    }

    //-------------------------------------------------------------------------
    /// @brief 浮動小数点数からバイト列を取り出し、ハッシュ値として使う。
    private: template<typename template_value>
    static typename this_type::result_type get_bit_field(
         /// [in] 浮動小数点数。
         template_value const in_float,
         std::true_type const&)
    PSYQ_NOEXCEPT
    {
        psyq::float_bit_field<template_value> local_value;
        local_value.float_ = in_float;
        return static_cast<typename this_type::result_type>(local_value.bit_field_);
    }

    /// @brief 整数をそのままハッシュ値として使う。
    private: template<typename template_value>
    static typename this_type::result_type get_bit_field(
        /// [in] in_integer 整数。
        template_value const in_integer,
        std::false_type const&)
    PSYQ_NOEXCEPT
    {
        static_assert(
            std::is_integral<template_value>::value,
            "'template_value' is not integral type.");
        return static_cast<typename this_type::result_type>(in_integer);
    }

}; // class psyq::hash::numeric_hash

#endif // !defined(PSYQ_HASH_NUMERIC_HPP_)
// vim: set expandtab:
