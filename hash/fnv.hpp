/// @file
/// @brief FNV形式のハッシュ関数。
/// @details
/// 以下のウェブページを参考に実装。
/// http://www.radiumsoftware.com/0605.html#060526
/// @author Hillco Psychi (https://twitter.com/psychi)
#ifndef PSYQ_HASH_FNV_HPP_
#define PSYQ_HASH_FNV_HPP_

#include "./proxy.hpp"
#include <cstdint>

namespace psyq
{
    namespace hash
    {
        /// @cond
        template<typename, typename> class string_fnv1_32;
        template<typename, typename> class string_fnv1_64;
        template<typename, typename> class string_fnv1a_32;
        template<typename, typename> class string_fnv1a_64;
        /// @endcond

        namespace _private
        {
            /// @cond
            template<typename, typename> class fnv;
            class fnv1_accumulator;
            class fnv1a_accumulator;
            /// @endcond
            /// @brief FNVハッシュ関数の型特性。宣言のみ。
            template<typename> class fnv_traits;

        } // namespace _private
    } // namespace hash
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief FNV形式のバイト列ハッシュ関数オブジェクト。
/// @tparam template_fnv_accumulator @copydoc psyq::hash::_private::fnv::accumulator
/// @tparam template_result          @copydoc psyq::hash::_private::fnv_traits<std::uint32_t>::result_type
template<typename template_fnv_accumulator, typename template_result>
class psyq::hash::_private::fnv
{
    /// @copydoc psyq::string::view::this_type
    private: typedef fnv this_type;

    //-------------------------------------------------------------------------
    /// @brief 委譲先となるFNVハッシュ式。
    public: typedef template_fnv_accumulator accumulator;
    /// @brief FNVハッシュ関数の型特性。
    public: typedef
        psyq::hash::_private::fnv_traits<template_result>
        traits_type;

    /// @copydoc psyq::hash::_private::murmur3a::hash
    public: typename this_type::traits_type::result_type operator()(
        /// [in] バイト列の先頭要素を指すポインタ。
        unsigned char const* const in_bytes,
        /// [in] バイト列のバイト数。
        std::size_t const in_length,
        /// [in] ハッシュ関数のシード値。
        typename this_type::traits_type::result_type const& in_seed)
    const
    {
        auto const local_end(in_bytes + in_length);
        auto local_hash(in_seed);
        for (auto i(in_bytes); i < local_end; ++i)
        {
            local_hash = this_type::accumulator::accumulate(
                *i,
                local_hash,
                static_cast<decltype(local_hash)>(this_type::traits_type::PRIME));
        }
        return local_hash;
    }

}; // class psyq::hash::_private::fnv

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// 32ビットFNVハッシュ関数の型特性。
template<> class psyq::hash::_private::fnv_traits<std::uint32_t>
{
    /// @brief ハッシュ関数の戻り値。
    public: typedef std::uint32_t result_type;
    public: enum: result_type
    {
        SEED = 0x811C9DC5, ///< シード値。
        PRIME = 0x1000193, ///< FNVハッシュ素数。
    };

}; // class psyq::hash::_private::fnv_traits<std::uint32_t>

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// 64ビットFNVハッシュ関数の型特性。
template<> class psyq::hash::_private::fnv_traits<std::uint64_t>
{
    /// @brief ハッシュ関数の戻り値。
    public: typedef std::uint64_t result_type;
    public: enum: result_type
    {
        SEED = 0xCBF29CE484222325ull, ///< シード値。
        PRIME = 0x100000001B3ull,     ///< FNVハッシュ素数。
    };
}; // class psyq::hash::_private::fnv_traits<std::uint64_t>

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief FNV-1形式のハッシュ計算式。
class psyq::hash::_private::fnv1_accumulator
{
    public: template<typename template_value_type>
    static template_value_type accumulate(
        /// [in] バイト列の値。
        unsigned char const in_byte,
        /// [in] FNVハッシュ計算式のシード値。
        template_value_type const in_seed,
        /// [in] FNVハッシュ素数。
        template_value_type const in_prime)
    PSYQ_NOEXCEPT
    {
        return (in_seed * in_prime) ^ in_byte;
    }

}; // class psyq::hash::_private::fnv1_accumulator

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief FNV-1a形式のハッシュ計算式。
class psyq::hash::_private::fnv1a_accumulator
{
    public: template<typename template_value_type>
    static template_value_type accumulate(
        /// [in] バイト列の値。
        unsigned char const in_byte,
        /// [in] FNVハッシュ計算式のシード値。
        template_value_type const in_seed,
        /// [in] FNVハッシュ素数。
        template_value_type const in_prime)
    PSYQ_NOEXCEPT
    {
        return (in_seed ^ in_byte) * in_prime;
    }

}; // class psyq::hash::_private::fnv1a_accumulator

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    namespace hash
    {
        namespace _private
        {
            /// @brief 32ビットFNV-1形式のバイト列ハッシュ関数オブジェクト。
            typedef fnv<fnv1_accumulator, std::uint32_t> fnv1_32;
            /// @brief 64ビットFNV-1形式のバイト列ハッシュ関数オブジェクト。
            typedef fnv<fnv1_accumulator, std::uint64_t> fnv1_64;
            /// @brief 32ビットFNV-1a形式のバイト列ハッシュ関数オブジェクト。
            typedef fnv<fnv1a_accumulator, std::uint32_t> fnv1a_32;
            /// @brief 64ビットFNV-1a形式のバイト列ハッシュ関数オブジェクト。
            typedef fnv<fnv1a_accumulator, std::uint64_t> fnv1a_64;

            /// @brief 32ビットFNV-1形式の配列ハッシュ関数オブジェクト。
            typedef
                array_seeding_proxy<
                    fnv1_32,
                    fnv1_32::traits_type::result_type,
                    fnv1_32::traits_type::SEED>
                array_fnv1_32;
            /// @brief 64ビットFNV-1形式の配列ハッシュ関数オブジェクト。
            typedef
                array_seeding_proxy<
                    fnv1_64,
                    fnv1_64::traits_type::result_type,
                    fnv1_64::traits_type::SEED>
                array_fnv1_64;
            /// @brief 32ビットFNV-1a形式の配列ハッシュ関数オブジェクト。
            typedef
                array_seeding_proxy<
                    fnv1a_32,
                    fnv1a_32::traits_type::result_type,
                    fnv1a_32::traits_type::SEED>
                array_fnv1a_32;
            /// @brief 64ビットFNV-1a形式の配列ハッシュ関数オブジェクト。
            typedef
                array_seeding_proxy<
                    fnv1a_64,
                    fnv1a_64::traits_type::result_type,
                    fnv1a_64::traits_type::SEED>
                array_fnv1a_64;

        } // namespace _private
    } // namespace hash
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 32ビットFNV-1形式の文字列ハッシュ関数オブジェクト。
/// @tparam template_string @copydoc _private::string_proxy::argument_type
/// @tparam template_result @copydoc primitive_bits::result_type
template<typename template_string, typename template_result = std::size_t>
class psyq::hash::string_fnv1_32:
public psyq::hash::_private::string_proxy<
    psyq::hash::_private::array_fnv1_32, template_result, template_string>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef string_fnv1_32 this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef
        psyq::hash::_private::string_proxy<
            psyq::hash::_private::array_fnv1_32,
            template_result,
            template_string>
        base_type;

    /// @brief 文字列ハッシュ関数オブジェクトを構築する。
    public: string_fnv1_32():
    base_type(typename base_type::hasher(typename base_type::hasher::hasher()))
    {}

}; // class psyq::hash::string_fnv1_32

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 64ビットFNV-1形式の文字列ハッシュ関数オブジェクト。
/// @tparam template_string @copydoc _private::string_proxy::argument_type
/// @tparam template_result @copydoc primitive_bits::result_type
template<typename template_string, typename template_result = std::size_t>
class psyq::hash::string_fnv1_64:
public psyq::hash::_private::string_proxy<
    psyq::hash::_private::array_fnv1_64, template_result, template_string>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef string_fnv1_64 this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef
        psyq::hash::_private::string_proxy<
            psyq::hash::_private::array_fnv1_64,
            template_result,
            template_string>
        base_type;

    /// @brief 文字列ハッシュ関数オブジェクトを構築する。
    public: string_fnv1_64():
    base_type(typename base_type::hasher(typename base_type::hasher::hasher()))
    {}

}; // class psyq::hash::string_fnv1_64

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 32ビットFNV-1a形式の文字列ハッシュ関数オブジェクト。
/// @tparam template_string @copydoc _private::string_proxy::argument_type
/// @tparam template_result @copydoc primitive_bits::result_type
template<typename template_string, typename template_result = std::size_t>
class psyq::hash::string_fnv1a_32:
public psyq::hash::_private::string_proxy<
    psyq::hash::_private::array_fnv1a_32, template_result, template_string>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef string_fnv1a_32 this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef
        psyq::hash::_private::string_proxy<
            psyq::hash::_private::array_fnv1a_32,
            template_result,
            template_string>
        base_type;

    /// @brief 文字列ハッシュ関数オブジェクトを構築する。
    public: string_fnv1a_32():
    base_type(typename base_type::hasher(typename base_type::hasher::hasher()))
    {}

}; // class psyq::hash::string_fnv1a_32

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 64ビットFNV-1a形式の文字列ハッシュ関数オブジェクト。
/// @tparam template_string @copydoc _private::string_proxy::argument_type
/// @tparam template_result @copydoc primitive_bits::result_type
template<typename template_string, typename template_result = std::size_t>
class psyq::hash::string_fnv1a_64:
public psyq::hash::_private::string_proxy<
    psyq::hash::_private::array_fnv1a_64, template_result, template_string>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef string_fnv1a_64 this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef
        psyq::hash::_private::string_proxy<
            psyq::hash::_private::array_fnv1a_64,
            template_result,
            template_string>
        base_type;

    /// @brief 文字列ハッシュ関数オブジェクトを構築する。
    public: string_fnv1a_64():
    base_type(typename base_type::hasher(typename base_type::hasher::hasher()))
    {}

}; // class psyq::hash::fvn1a_string_hash64

#endif // !defined(PSYQ_HASH_FNV_HPP_)
// vim: set expandtab:
