/// @file
/// @brief  @copybrief psyq::hash::numeric_hash
/// @author Hillco Psychi (https://twitter.com/psychi)
#ifndef PSYQ_HASH_NUMERIC_HPP_
#define PSYQ_HASH_NUMERIC_HPP_

#include "../bit_algorithm.hpp"

namespace psyq
{
    namespace hash
    {
        template<typename> class numeric_hash;
    }
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 数値のバイト列を使うハッシュ関数オブジェクト。
/// @tparam template_iterator @copydoc argument_type
template<typename template_number>
class psyq::hash::numeric_hash
{
    private: typedef numeric_hash this_type;

    //-------------------------------------------------------------------------
    /// @brief ハッシュ関数の引数となる数値の型。
    public: typedef template_number argument_type;
    static_assert(
        std::is_arithmetic<template_number>::value,
        "'template_number' is not arithmetic type.");

    /// @brief ハッシュ関数の戻り値の型。
    public: typedef std::size_t result_type;

    //-------------------------------------------------------------------------
    /// @brief 数値のハッシュ値を取得する。
    /// @param[in] in_number 数値。
    /// @return in_integer
    public: typename this_type::result_type operator()(
        typename this_type::argument_type const in_number)
    const PSYQ_NOEXCEPT
    {
        return this_type::get_bitfield(
            in_number,
            std::is_floating_point<typename this_type::argument_type>());
    }

    //-------------------------------------------------------------------------
    private: template<typename template_value>
    static typename this_type::result_type get_bitfield(
         template_value const in_number,
         std::true_type const&)
    PSYQ_NOEXCEPT
    {
        psyq::float_bit_field<template_value> local_value;
        local_value.float_ = in_number;
        return local_float.bit_field_;
    }

    private: template<typename template_value>
    static typename this_type::result_type get_bitfield(
         template_value const in_number,
         std::false_type const&)
    PSYQ_NOEXCEPT
    {
        return static_cast<typename this_type::result_type>(in_number);
    }

}; // class psyq::hash::numeric_hash

#endif // !defined(PSYQ_HASH_NUMERIC_HPP_)
// vim: set expandtab:
