/// @file
/// @brief @copybrief psyq::hash::primitive_bits
/// @author Hillco Psychi (https://twitter.com/psychi)
#ifndef PSYQ_HASH_PRIMITIVE_BITS_HPP_
#define PSYQ_HASH_PRIMITIVE_BITS_HPP_

#include "../bit_algorithm.hpp"

namespace psyq
{
    namespace hash
    {
        /// @cond
        template<typename, typename> class primitive_bits;
        /// @endcond
    }
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 組み込み型のビット表現をそのまま使うハッシュ関数オブジェクト。
/// @tparam template_value  @copydoc primitive_bits::argument_type
/// @tparam template_result @copydoc primitive_bits::result_type
template<typename template_value, typename template_result = std::size_t>
class psyq::hash::primitive_bits
{
    /// @copydoc psyq::string::view::this_type
    private: typedef primitive_bits this_type;

    //-------------------------------------------------------------------------
    /// @brief ハッシュ関数の引数となるキー。
    /// @details
    /// 以下の組み込み型が使える。
    /// - ポインタ型。
    /// - 列挙型。
    /// - 整数型。
    /// - 浮動小数点数型。
    public: typedef template_value argument_type;
    static_assert(
        std::is_pointer<template_value>::value
        || std::is_enum<template_value>::value
        || std::is_integral<template_value>::value
        || std::is_floating_point<template_value>::value,
        "'template_value' is invalid type.");

    /// @brief ハッシュ関数の戻り値。
    /// @details
    /// template_result が std::size_t へ暗黙に型変換できるなら、
    /// this_type は std::hash のインタフェイスと互換性を持つ。
    public: typedef template_result result_type;

    //-------------------------------------------------------------------------
    /// @brief キーの型の種別。
    private: enum kind: unsigned char
    {
        kind_POINTER, ///< ポインタ型。
        kind_INTEGER, ///< 整数型。
        kind_FLOAT,   ///< 浮動小数点数型。
    };

    //-------------------------------------------------------------------------
    /// @brief キーに対応するハッシュ値を取得する。
    /// @param[in] in_key キー。
    /// @return in_key に対応するハッシュ値。
    public: typename this_type::result_type operator()(
        typename this_type::argument_type const in_key)
    const PSYQ_NOEXCEPT
    {
        return this_type::get_bits(
            in_key,
            std::integral_constant<
                typename this_type::kind,
                std::is_pointer<typename this_type::argument_type>::value?
                    kind_POINTER:
                    std::is_floating_point<typename this_type::argument_type>::value?
                        kind_FLOAT: kind_INTEGER>());
    }

    //-------------------------------------------------------------------------
    /// @brief ポインタ値をそのままハッシュ値として使う。
    private: template<typename template_pointer>
    static typename this_type::result_type get_bits(
        /// [in] in_pointer ポインタ。
        template_pointer const in_pointer,
        std::integral_constant<typename this_type::kind, this_type::kind_POINTER> const&)
    PSYQ_NOEXCEPT
    {
        return reinterpret_cast<typename this_type::result_type>(in_pointer);
    }

    /// @brief 整数値をそのままハッシュ値として使う。
    private: template<typename template_integer>
    static typename this_type::result_type get_bits(
        /// [in] in_integer 整数。
        template_integer const in_integer,
        std::integral_constant<typename this_type::kind, this_type::kind_INTEGER> const&)
    PSYQ_NOEXCEPT
    {
        return static_cast<typename this_type::result_type>(in_integer);
    }

    /// @brief 浮動小数点数のビット表現をハッシュ値として使う。
    private: template<typename template_float>
    static typename this_type::result_type get_bits(
        /// [in] 浮動小数点数。
        template_float const in_float,
        std::integral_constant<typename this_type::kind, this_type::kind_FLOAT> const&)
    PSYQ_NOEXCEPT
    {
        psyq::float_bit_field<template_float> const local_value(in_float);
        return static_cast<typename this_type::result_type>(local_value.bit_field_);
    }

}; // class psyq::hash::primitive_bits

#endif // !defined(PSYQ_HASH_PRIMITIVE_BITS_HPP_)
// vim: set expandtab:
