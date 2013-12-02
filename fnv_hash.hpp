/* Copyright (c) 2013, Hillco Psychi, All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
   OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
   ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief FNV-hash関数。

    以下のweb-pageを参考にして実装した。
    - http://www.radiumsoftware.com/0605.html#060526
    - http://d.hatena.ne.jp/jonosuke/20100406/p1
 */
#ifndef PSYQ_FNV_HASH_HPP_
#define PSYQ_FNV_HASH_HPP_
#include <cstdint>

namespace psyq
{
    /// この名前空間をuserが直接accessするのは禁止。
    namespace internal
    {
        /// @cond
        template<typename, typename> struct fnv_hash;
        struct fnv1_maker;
        struct fnv1a_maker;
        template<typename> struct fnv_traits;
        /// @endcond
    }

    /// 32bit FNV-1 hash関数object
    typedef psyq::internal::fnv_hash<
        psyq::internal::fnv1_maker, psyq::internal::fnv_traits<std::uint32_t>>
            fnv1_hash32;

    /// 64bit FNV-1 hash関数object
    typedef psyq::internal::fnv_hash<
        psyq::internal::fnv1_maker, psyq::internal::fnv_traits<std::uint64_t>>
            fnv1_hash64;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief FNV-hash
    @tparam template_hash_maker @copydoc fnv_hash::maker
    @tparam template_hash_traits @copydoc fnv_hash::traits
 */
template<typename template_hash_maker, typename template_hash_traits>
struct psyq::internal::fnv_hash
{
    /// thisの指す値の型。
    typedef psyq::internal::fnv_hash<
        template_hash_maker, template_hash_traits>
            self;

    //-------------------------------------------------------------------------
    /// FNV-hashを生成。
    typedef template_hash_maker maker;

    /// FNV-hashの型特性。
    typedef template_hash_traits traits_type;

    /// FNV-hash値の型。
    typedef typename self::traits_type::value_type value_type;

    //-------------------------------------------------------------------------
    /** @brief 文字列のhash値を生成。
        @tparam template_char_type 文字の型。
        @param[in] in_string NULL文字で終了する文字列の先頭位置。
        @param[in] in_offset FNV-hash開始値。
        @param[in] in_prime  FNV-hash素数。
        @return 文字列のhash値。
     */
    template<typename template_char_type>
    static typename self::value_type make(
        template_char_type const* const in_string,
        typename self::value_type const in_offset = self::traits_type::EMPTY,
        typename self::value_type const in_prime = self::traits_type::PRIME)
    {
        auto local_hash(in_offset);
        if (in_string != NULL)
        {
            for (template_char_type const* i(in_string); *i != 0; ++i)
            {
                local_hash = template_hash_maker::make(
                    i, i + 1, local_hash, in_prime);
            }
        }
        return local_hash;
    }

    /** @brief 配列のhash値を生成。
        @tparam template_value_type 配列の要素の型。
        @param[in] in_begin  配列の先頭位置。
        @param[in] in_end    配列の末尾位置。
        @param[in] in_offset FNV-hash開始値。
        @param[in] in_prime  FNV-hash素数。
     */
    template<typename template_value_type>
    static typename self::value_type make(
        template_value_type const* const in_begin,
        template_value_type const* const in_end,
        typename self::value_type const  in_offset = self::traits_type::EMPTY,
        typename self::value_type const  in_prime = self::traits_type::PRIME)
    {
        return template_hash_maker::make(
            in_begin, in_end, in_offset, in_prime);
    }

    /** @brief 配列のhash値を生成。
        @tparam template_iterator_type 配列の要素を指す反復子の型。
        @param[in] in_begin  文字列の先頭位置。
        @param[in] in_end    文字列の末尾位置。
        @param[in] in_offset FNV-hash開始値。
        @param[in] in_prime  FNV-hash素数。
     */
    template<typename template_iterator_type>
    static typename self::value_type make(
        template_iterator_type const&   in_begin,
        template_iterator_type const&   in_end,
        typename self::value_type const in_offset = self::traits_type::EMPTY,
        typename self::value_type const in_prime = self::traits_type::PRIME)
    {
        auto local_hash(in_offset);
        for (template_iterator_type i(in_begin); in_end != i; ++i)
        {
            local_hash = template_hash_maker::make(
                &(*i), &(*i) + 1, local_hash, in_prime);
        }
        return local_hash;
    }
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief FNV-1で、hash値を生成。
struct psyq::internal::fnv1_maker
{
    //-------------------------------------------------------------------------
    /** @brief byte配列のhash値を生成。
        @param[in] in_begin  byte配列の先頭位置。
        @param[in] in_end    byte配列の末尾位置。
        @param[in] in_offset hash開始値。
        @param[in] in_prime  FNV-hash素数。
     */
    public: template<typename template_value_type>
    static template_value_type make(
        void const* const          in_begin,
        void const* const          in_end,
        template_value_type const& in_offset,
        template_value_type const& in_prime)
    {
        auto local_hash(in_offset);
        for (auto i(static_cast<char const*>(in_begin)); i < in_end; ++i)
        {
            local_hash = (local_hash * in_prime) ^ *i;
        }
        return local_hash;
    }
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief FNV-1aで、hash値を生成。
struct psyq::internal::fnv1a_maker
{
    //-------------------------------------------------------------------------
    /** @brief byte配列のhash値を生成。
        @param[in] in_begin  byte配列の先頭位置。
        @param[in] in_end    byte配列の末尾位置。
        @param[in] in_offset FNV-hash開始値。
        @param[in] in_prime  FNV-hash素数。
     */
    public: template<typename template_value_type>
    static template_value_type make(
        void const* const          in_begin,
        void const* const          in_end,
        template_value_type const& in_offset,
        template_value_type const& in_prime)
    {
        auto local_hash(in_offset);
        for (auto i(static_cast<char const*>(in_begin)); i < in_end; ++i)
        {
            local_hash = (local_hash ^ *i) * in_prime;
        }
        return local_hash;
    }
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// 32bitのFNV-hash関数の型特性。
template<> struct psyq::internal::fnv_traits<std::uint32_t>
{
    typedef std::uint32_t value_type; ///< hash値の型。
    enum: value_type
    {
        EMPTY = 0x811c9dc5, ///< 空hash値。
        PRIME = 0x1000193,  ///< FNV-hash素数。
    };
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// 64bitのFNV-hash関数の型特性。
template<> struct psyq::internal::fnv_traits<std::uint64_t>
{
    typedef std::uint64_t value_type;  ///< hash値の型。
    enum: value_type
    {
        EMPTY = 0xcbf29ce484222325ULL, ///< 空hash値。
        PRIME = 0x100000001b3ULL,      ///< FNV-hash素数。
    };
};

#endif // !defined(PSYQ_FNV_HASH_HPP_)
