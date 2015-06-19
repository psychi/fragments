/*
Copyright (c) 2015, Hillco Psychi, All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met: 
ソースコード形式かバイナリ形式か、変更するかしないかを問わず、
以下の条件を満たす場合に限り、再頒布および使用が許可されます。

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer. 
   ソースコードを再頒布する場合、上記の著作権表示、本条件一覧、
   および下記の免責条項を含めること。
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution. 
   バイナリ形式で再頒布する場合、頒布物に付属のドキュメント等の資料に、
   上記の著作権表示、本条件一覧、および下記の免責条項を含めること。

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
本ソフトウェアは、著作権者およびコントリビューターによって
「現状のまま」提供されており、明示黙示を問わず、商業的な使用可能性、
および特定の目的に対する適合性に関する暗黙の保証も含め、
またそれに限定されない、いかなる保証もありません。
著作権者もコントリビューターも、事由のいかんを問わず、
損害発生の原因いかんを問わず、かつ責任の根拠が契約であるか厳格責任であるか
（過失その他の）不法行為であるかを問わず、
仮にそのような損害が発生する可能性を知らされていたとしても、
本ソフトウェアの使用によって発生した（代替品または代用サービスの調達、
使用の喪失、データの喪失、利益の喪失、業務の中断も含め、
またそれに限定されない）直接損害、間接損害、偶発的な損害、特別損害、
懲罰的損害、または結果損害について、一切責任を負わないものとします。
 */
/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief FNV形式のhash計算機。

    以下のweb-pageを参考に実装。
    http://www.radiumsoftware.com/0605.html#060526
 */
#ifndef PSYQ_FNV_HASH_HPP_
#define PSYQ_FNV_HASH_HPP_

#include <cstdint>
#include "./assert.hpp"

namespace psyq
{
    /// psyq 管理者以外が、この名前空間を直接accessするのは禁止。
    namespace _private
    {
        /// @cond
        template<typename, typename> struct fnv_hash;
        struct fnv1_algorithm;
        struct fnv1a_algorithm;
        template<typename> struct fnv_traits;
        /// @endcond
    }

    /// 32bit FNV-1形式のhash計算機。
    typedef psyq::_private::fnv_hash<
        psyq::_private::fnv1_algorithm, psyq::_private::fnv_traits<std::uint32_t>>
            fnv1_hash32;

    /// 64bit FNV-1形式のhash計算機。
    typedef psyq::_private::fnv_hash<
        psyq::_private::fnv1_algorithm, psyq::_private::fnv_traits<std::uint64_t>>
            fnv1_hash64;

    /// 32bit FNV-1a形式のhash計算機。
    typedef psyq::_private::fnv_hash<
        psyq::_private::fnv1a_algorithm, psyq::_private::fnv_traits<std::uint32_t>>
            fnv1a_hash32;

    /// 64bit FNV-1a形式のhash計算機。
    typedef psyq::_private::fnv_hash<
        psyq::_private::fnv1a_algorithm, psyq::_private::fnv_traits<std::uint64_t>>
            fnv1a_hash64;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief FNV-hash計算機。
    @tparam template_hash_algorithm  @copydoc fnv_hash::algorithm
    @tparam template_hash_traits @copydoc fnv_hash::traits_type
 */
template<typename template_hash_algorithm, typename template_hash_traits>
struct psyq::_private::fnv_hash
{
    /// thisの指す値の型。
    typedef psyq::_private::fnv_hash<
        template_hash_algorithm, template_hash_traits>
            this_type;

    //-------------------------------------------------------------------------
    /// FNV-hashの算法。
    typedef template_hash_algorithm algorithm;

    /// FNV-hashの型特性。
    typedef template_hash_traits traits_type;

    /// FNV-hash値の型。
    typedef typename this_type::traits_type::value_type value_type;

    //-------------------------------------------------------------------------
    /** @brief 文字列のhash値を算出する。
        @tparam template_char_type 文字の型。
        @param[in] in_string null文字で終了する文字列の先頭位置。
        @param[in] in_offset FNV-hash開始値。
        @param[in] in_prime  FNV-hash素数。
        @return 文字列のhash値。
     */
    template<typename template_char_type>
    static typename this_type::value_type compute(
        template_char_type const* const in_string,
        typename this_type::value_type const in_offset = this_type::traits_type::EMPTY,
        typename this_type::value_type const in_prime = this_type::traits_type::PRIME)
    PSYQ_NOEXCEPT
    {
        auto local_hash(in_offset);
        if (in_string != nullptr)
        {
            for (template_char_type const* i(in_string); *i != 0; ++i)
            {
                local_hash = template_hash_algorithm::compute(
                    i, i + 1, local_hash, in_prime);
            }
        }
        return local_hash;
    }

    /** @brief バイト列のhash値を算出する。
        @tparam template_value_type 配列の要素の型。
        @param[in] in_begin  配列の先頭位置。
        @param[in] in_end    配列の末尾位置。
        @param[in] in_offset FNV-hash開始値。
        @param[in] in_prime  FNV-hash素数。
     */
    static typename this_type::value_type compute(
        void const* const in_begin,
        void const* const in_end,
        typename this_type::value_type const in_offset = this_type::traits_type::EMPTY,
        typename this_type::value_type const in_prime = this_type::traits_type::PRIME)
    PSYQ_NOEXCEPT
    {
        return template_hash_algorithm::compute(
            in_begin, in_end, in_offset, in_prime);
    }

    /** @brief containerのhash値を算出する。
        @tparam template_iterator_type containerの要素を指す反復子の型。
        @param[in] in_begin  containerの先頭位置。
        @param[in] in_end    containerの末尾位置。
        @param[in] in_offset FNV-hash開始値。
        @param[in] in_prime  FNV-hash素数。
     */
    template<typename template_iterator_type>
    static typename this_type::value_type compute(
        template_iterator_type const& in_begin,
        template_iterator_type const& in_end,
        typename this_type::value_type const in_offset = this_type::traits_type::EMPTY,
        typename this_type::value_type const in_prime = this_type::traits_type::PRIME)
    PSYQ_NOEXCEPT
    {
        auto local_hash(in_offset);
        for (template_iterator_type i(in_begin); in_end != i; ++i)
        {
            local_hash = template_hash_algorithm::compute(
                &(*i), &(*i) + 1, local_hash, in_prime);
        }
        return local_hash;
    }
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief FNV-1形式のhash算法。
struct psyq::_private::fnv1_algorithm
{
    //-------------------------------------------------------------------------
    /** @brief byte配列のhash値を算出する。
        @param[in] in_begin  byte配列の先頭位置。
        @param[in] in_end    byte配列の末尾位置。
        @param[in] in_offset hash開始値。
        @param[in] in_prime  FNV-hash素数。
        @return FNV-1形式のhash値。
     */
    public: template<typename template_value_type>
    static template_value_type compute(
        void const* const          in_begin,
        void const* const          in_end,
        template_value_type const& in_offset,
        template_value_type const& in_prime)
    PSYQ_NOEXCEPT
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
/// @brief FNV-1a形式のhash算法。
struct psyq::_private::fnv1a_algorithm
{
    //-------------------------------------------------------------------------
    /** @brief byte配列のhash値を算出する。
        @param[in] in_begin  byte配列の先頭位置。
        @param[in] in_end    byte配列の末尾位置。
        @param[in] in_offset FNV-hash開始値。
        @param[in] in_prime  FNV-hash素数。
        @return FNV-1a形式のhash値。
     */
    public: template<typename template_value_type>
    static template_value_type compute(
        void const* const          in_begin,
        void const* const          in_end,
        template_value_type const& in_offset,
        template_value_type const& in_prime)
    PSYQ_NOEXCEPT
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
template<> struct psyq::_private::fnv_traits<std::uint32_t>
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
template<> struct psyq::_private::fnv_traits<std::uint64_t>
{
    typedef std::uint64_t value_type;  ///< hash値の型。
    enum: value_type
    {
        EMPTY = 0xcbf29ce484222325ULL, ///< 空hash値。
        PRIME = 0x100000001b3ULL,      ///< FNV-hash素数。
    };
};

#endif // !defined(PSYQ_FNV_HASH_HPP_)
