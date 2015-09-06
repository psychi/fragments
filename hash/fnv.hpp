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
        template<typename> class fnv1_string_hash32;
        template<typename> class fnv1_string_hash64;
        template<typename> class fnv1a_string_hash32;
        template<typename> class fnv1a_string_hash64;

        namespace _private
        {
            template<typename, typename> class fnv_hash;
            /// @brief FNVハッシュ関数の型特性。宣言のみ。
            template<typename> class fnv_traits;
            class fnv1_accumulator;
            class fnv1a_accumulator;

        } // namespace _private
    } // namespace hash
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief FNVハッシュ関数オブジェクト。
/// @tparam template_fnv_accumulator @copydoc psyq::hash::_private::fnv_hash::accumulator
/// @tparam template_result          @copydoc psyq::hash::_private::fnv_traits<std::uint32_t>::result_type
template<typename template_fnv_accumulator, typename template_result>
class psyq::hash::_private::fnv_hash
{
    /// @copydoc psyq::string::view::this_type
    private: typedef fnv_hash this_type;

    //-------------------------------------------------------------------------
    /// @brief 委譲先となるFNVハッシュ関数。
    public: typedef template_fnv_accumulator accumulator;
    /// @brief FNVハッシュ関数の型特性。
    public: typedef
        psyq::hash::_private::fnv_traits<template_result>
        traits_type;

    /// @copydoc psyq::hash::_private::murmur3_hash32::hash
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

}; // class psyq::hash::_private::fnv_hash

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
/// @brief FNV-1形式のハッシュ関数。
class psyq::hash::_private::fnv1_accumulator
{
    public: template<typename template_value_type>
    static template_value_type accumulate(
        /// [in] バイト列の値。
        unsigned char const in_byte,
        /// [in] ハッシュ関数のシード値。
        template_value_type const in_seed,
        /// [in] FNV-1ハッシュ素数。
        template_value_type const in_prime)
    PSYQ_NOEXCEPT
    {
        return (in_seed * in_prime) ^ in_byte;
    }

}; // class psyq::hash::_private::fnv1_accumulator

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief FNV-1a形式のハッシュ関数。
class psyq::hash::_private::fnv1a_accumulator
{
    public: template<typename template_value_type>
    static template_value_type accumulate(
        /// [in] バイト列の値。
        unsigned char const in_byte,
        /// [in] ハッシュ関数のシード値。
        template_value_type const in_seed,
        /// [in] FNV-1ハッシュ素数。
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
            /// @brief FNV-1形式の32ビットハッシュ関数オブジェクト。
            typedef fnv_hash<fnv1_accumulator, std::uint32_t> fnv1_hash32;
            /// @brief FNV-1形式の64ビットハッシュ関数オブジェクト。
            typedef fnv_hash<fnv1_accumulator, std::uint64_t> fnv1_hash64;
            /// @brief FNV-1a形式の32ビットハッシュ関数オブジェクト。
            typedef fnv_hash<fnv1a_accumulator, std::uint32_t> fnv1a_hash32;
            /// @brief FNV-1a形式の64ビットハッシュ関数オブジェクト。
            typedef fnv_hash<fnv1a_accumulator, std::uint64_t> fnv1a_hash64;

            /// @brief FNV-1形式の32ビットハッシュ関数代理オブジェクト。
            typedef
                seeding_bytes_hash_proxy<
                    fnv1_hash32,
                    fnv1_hash32::traits_type::result_type,
                    fnv1_hash32::traits_type::SEED>
                fnv1_hash32_proxy;
            /// @brief FNV-1形式の64ビットハッシュ関数代理オブジェクト。
            typedef
                seeding_bytes_hash_proxy<
                    fnv1_hash64,
                    fnv1_hash64::traits_type::result_type,
                    fnv1_hash64::traits_type::SEED>
                fnv1_hash64_proxy;
            /// @brief FNV-1a形式のビットハッシュ関数代理オブジェクト。
            typedef
                seeding_bytes_hash_proxy<
                    fnv1a_hash32,
                    fnv1a_hash32::traits_type::result_type,
                    fnv1a_hash32::traits_type::SEED>
                fnv1a_hash32_proxy;
            /// @brief FNV-1a形式のビットハッシュ関数代理オブジェクト。
            typedef
                seeding_bytes_hash_proxy<
                    fnv1a_hash64,
                    fnv1a_hash64::traits_type::result_type,
                    fnv1a_hash64::traits_type::SEED>
                fnv1a_hash64_proxy;

        } // namespace _private
    } // namespace hash
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief FNV-1形式の文字列32ビットハッシュ関数オブジェクト。
/// @tparam template_string @copydoc psyq::hash::_private::string_hash_proxy::argument_type
template<typename template_string>
class psyq::hash::fnv1_string_hash32:
public psyq::hash::_private::string_hash_proxy<
    psyq::hash::_private::fnv1_hash32_proxy, template_string>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef fnv1_string_hash32 this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef
        psyq::hash::_private::string_hash_proxy<
            psyq::hash::_private::fnv1_hash32_proxy, template_string>
        base_type;

    /// @brief 文字列ハッシュ関数オブジェクトを構築する。
    public: fnv1_string_hash32():
    base_type(typename base_type::hasher(typename base_type::hasher::hasher()))
    {}

}; // class psyq::hash::fnv1_string_hash32

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief FNV-1形式の文字列64ビットハッシュ関数オブジェクト。
/// @tparam template_string @copydoc psyq::hash::_private::string_hash_proxy::argument_type
template<typename template_string>
class psyq::hash::fnv1_string_hash64:
public psyq::hash::_private::string_hash_proxy<
    psyq::hash::_private::fnv1_hash64_proxy, template_string>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef fnv1_string_hash64 this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef
        psyq::hash::_private::string_hash_proxy<
            psyq::hash::_private::fnv1_hash64_proxy, template_string>
        base_type;

    /// @brief 文字列ハッシュ関数オブジェクトを構築する。
    public: fnv1_string_hash64():
    base_type(typename base_type::hasher(typename base_type::hasher::hasher()))
    {}

}; // class psyq::hash::fnv1_string_hash64

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief FNV-1a形式の文字列32ビットハッシュ関数オブジェクト。
/// @tparam template_string @copydoc psyq::hash::_private::string_hash_proxy::argument_type
template<typename template_string>
class psyq::hash::fnv1a_string_hash32:
public psyq::hash::_private::string_hash_proxy<
    psyq::hash::_private::fnv1a_hash32_proxy, template_string>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef fnv1a_string_hash32 this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef
        psyq::hash::_private::string_hash_proxy<
            psyq::hash::_private::fnv1a_hash32_proxy, template_string>
        base_type;

    /// @brief 文字列ハッシュ関数オブジェクトを構築する。
    public: fnv1a_string_hash32():
    base_type(typename base_type::hasher(typename base_type::hasher::hasher()))
    {}

}; // class psyq::hash::fnv1a_string_hash32

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief FNV-1a形式の文字列64ビットハッシュ関数オブジェクト。
/// @tparam template_string @copydoc psyq::hash::_private::string_hash_proxy::argument_type
template<typename template_string>
class psyq::hash::fnv1a_string_hash64:
public psyq::hash::_private::string_hash_proxy<
    psyq::hash::_private::fnv1a_hash64_proxy, template_string>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef fnv1a_string_hash64 this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef
        psyq::hash::_private::string_hash_proxy<
            psyq::hash::_private::fnv1a_hash64_proxy, template_string>
        base_type;

    /// @brief 文字列ハッシュ関数オブジェクトを構築する。
    public: fnv1a_string_hash64():
    base_type(typename base_type::hasher(typename base_type::hasher::hasher()))
    {}

}; // class psyq::hash::fvn1a_string_hash64

#endif // !defined(PSYQ_HASH_FNV_HPP_)
// vim: set expandtab:
