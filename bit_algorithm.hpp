/* Copyright (c) 2013, Hillco Psychi, All rights reserved.

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
    @brief ビット操作のための関数群。
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_BIT_ALGORITHM_HPP_
#define PSYQ_BIT_ALGORITHM_HPP_

#include <algorithm>
#include <cstdint>
#include "./assert.hpp"

#if defined(__alpha__) || defined(__ia64__) || defined(__x86_64__) || defined(_WIN64) || defined(__LP64__) || defined(__LLP64__)
#   define PSYQ_BIT_ALGORITHM_INTRINSIC_SIZE 64
#else
#   define PSYQ_BIT_ALGORITHM_INTRINSIC_SIZE 32
#endif

#if defined(FLT_RADIX) && FLT_RADIX == 2
#   define PSYQ_COUNT_LEADING_0BITS_BY_FLOAT
#endif

#if defined(__GNUC__) && (3 < __GNUC__ || (__GNUC__ == 3 && 4 <= __GNUC_MINOR__)) && defined(__GNUC_PATCHLEVEL__)
//  'gcc 3.4' and above have builtin support, specialized for architecture.
//  Some compilers masquerade as gcc; patchlevel test filters them out.
#   define PSYQ_BIT_ALGORITHM_FOR_GNUC
#elif defined(_MSC_VER)
#   define PSYQ_BIT_ALGORITHM_FOR_MSC
#   if defined(_M_PPC)
#      include <ppcintrinsics.h>
#   endif
#elif defined(__ARMCC_VERSION)
//  RealView Compilation Tools for ARM.
#   define PSYQ_BIT_ALGORITHM_FOR_ARMCC
#elif defined(__ghs__)
//  Green Hills support for PowerPC.
#   define PSYQ_BIT_ALGORITHM_FOR_GHS
#   include <ppc_ghs.h>
#endif

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    enum: std::uint8_t
    {
        /// @brief char値1つあたりのビット幅。
        CHAR_BIT_WIDTH = 8,
    };
    static_assert(
        char(1 << (CHAR_BIT_WIDTH - 1)) < 0,
        "'CHAR_BIT_WIDTH' is not bit width of char.");

    //-------------------------------------------------------------------------
    namespace _private
    {
        template<typename template_bits>
        bool is_valid_bit_shift(std::size_t const in_bit_shift) PSYQ_NOEXCEPT
        {
            return in_bit_shift < sizeof(template_bits) * psyq::CHAR_BIT_WIDTH;
        }

        template<typename template_bits>
        bool is_valid_bit_width(std::size_t const in_bit_width) PSYQ_NOEXCEPT
        {
            return in_bit_width <= sizeof(template_bits) * psyq::CHAR_BIT_WIDTH;
        }

        template<typename template_bits>
        bool is_valid_bit_width(
            std::size_t const in_bit_position,
            std::size_t const in_bit_width)
        PSYQ_NOEXCEPT
        {
            return is_valid_bit_width<template_bits>(in_bit_position)
                && is_valid_bit_width<template_bits>(in_bit_width)
                && is_valid_bit_width<template_bits>(
                    in_bit_position + in_bit_width);
        }
    }

    //-------------------------------------------------------------------------
    /** @brief 符号あり整数の絶対値を取得する。

        条件分岐の代わりにビット演算を使い、整数の絶対値を算出する。

        @param[in] in_value 絶対値を求める符号あり整数。
        @return in_value の絶対値。
     */
    template<typename template_integer>
    template_integer abs_integer(template_integer const in_value)
    PSYQ_NOEXCEPT
    {
        static_assert(
            std::is_integral<template_integer>::value
            && std::is_signed<template_integer>::value,
            "'template_integer' is not signed integer type.");
        auto const local_sign_bit_position(
            psyq::CHAR_BIT_WIDTH * sizeof(template_integer) - 1);
        auto const local_mask(
            -static_cast<template_integer>(
                1 & (in_value >> local_sign_bit_position)));
        return (in_value ^ local_mask) - local_mask;
    }

    //-------------------------------------------------------------------------
    /** @brief 整数を左ビットシフトする。
        @param[in] in_bits  ビットシフトする値。
        @param[in] in_shift シフトするビット数。
        @return 左ビットシフトした値。
     */
    template<typename template_bits>
    PSYQ_CONSTEXPR template_bits shift_left_bitwise(
        template_bits const in_bits,
        std::size_t const in_shift)
    PSYQ_NOEXCEPT
    {
        return static_cast<template_bits>(
            psyq::_private::is_valid_bit_shift<template_bits>(in_shift)?
                in_bits << in_shift: 0);
    }

    /** @brief 整数を左ビットシフトする。
        @param[in] in_bits  ビットシフトする値。
        @param[in] in_shift シフトするビット数。
        @return 左ビットシフトした値。
        @note
            bit数以上のbit-shift演算は、C言語の仕様として未定義の動作となる。
            http://hexadrive.sblo.jp/article/56575654.html
     */
    template<typename template_bits>
    PSYQ_CONSTEXPR template_bits shift_left_bitwise_fast(
        template_bits const in_bits,
        std::size_t const in_shift)
    PSYQ_NOEXCEPT
    {
        return static_cast<template_bits>(
            PSYQ_ASSERT(
                psyq::_private::is_valid_bit_shift<template_bits>(in_shift)),
            in_bits << in_shift);
    }

    /** @brief 整数を右ビットシフトする。
        @param[in] in_bits  ビットシフトする値。
        @param[in] in_shift シフトするビット数。
        @return 右ビットシフトした値。
     */
    template<typename template_bits>
    PSYQ_CONSTEXPR template_bits shift_right_bitwise(
        template_bits const in_bits,
        std::size_t const in_shift)
    PSYQ_NOEXCEPT
    {
        return std::is_unsigned<template_bits>::value?
            static_cast<template_bits>(
                psyq::_private::is_valid_bit_shift<template_bits>(in_shift)?
                    in_bits >> in_shift: 0):
            static_cast<template_bits>(
                in_bits >> (std::min<std::size_t>)(
                    in_shift, sizeof(in_bits) * psyq::CHAR_BIT_WIDTH - 1));
    }

    /** @brief 整数を右ビットシフトする。
        @param[in] in_bits  ビットシフトする値。
        @param[in] in_shift シフトするビット数。
        @return 右ビットシフトした値。
        @note
            bit数以上のbit-shift演算は、C言語の仕様として未定義の動作となる。
            http://hexadrive.sblo.jp/article/56575654.html
     */
    template<typename template_bits>
    PSYQ_CONSTEXPR template_bits shift_right_bitwise_fast(
        template_bits const in_bits,
        std::size_t const in_shift)
    PSYQ_NOEXCEPT
    {
        return static_cast<template_bits>(
            PSYQ_ASSERT(
                psyq::_private::is_valid_bit_shift<template_bits>(in_shift)),
            in_bits >> in_shift);
    }

    //-------------------------------------------------------------------------
    /** @brief 指定された位置のビット値を取得する。
        @param[in] in_bits ビット集合として扱う整数値。
        @param[in] in_position 取得するビットの位置。
        @return
            指定された位置のビット値。
            ただし in_position がsizeof(int)以上だった場合
            - in_bits が有符号整数型なら、符号ビットを返す。
            - in_bits が無符号整数型なら、falseを返す。
     */
    template<typename template_bits>
    PSYQ_CONSTEXPR bool get_bit(
        template_bits const in_bits,
        std::size_t const in_position)
    PSYQ_NOEXCEPT
    {
        return (psyq::shift_right_bitwise(in_bits, in_position) & 1) != 0;
    }

    /** @brief 指定された位置のビット値を取得する。
        @param[in] in_bits ビット集合として扱う整数値。
        @param[in] in_position 取得するビットの位置。
        @return
            指定された位置のビット値。
            ただし in_position がsizeof(int)以上だった場合、未定義。
     */
    template<typename template_bits>
    PSYQ_CONSTEXPR bool get_bit_fast(
        template_bits const in_bits,
        std::size_t const in_position)
    PSYQ_NOEXCEPT
    {
        return (psyq::shift_right_bitwise_fast(in_bits, in_position) & 1) != 0;
    }

    /** @brief 指定された位置にビット値として0を設定する。
        @param[in] in_bits     ビット集合として扱う整数値。
        @param[in] in_position 設定するビットの位置。
        @return
            指定されたビット位置に0を設定した整数値。
            ただし in_position がsizeof(int)以上だった場合、
            in_bits をそのまま返す。
     */
    template<typename template_bits>
    PSYQ_CONSTEXPR template_bits reset_bit(
        template_bits const in_bits,
        std::size_t const in_position)
    PSYQ_NOEXCEPT
    {
        return ~psyq::shift_left_bitwise(template_bits(1), in_position)
            & in_bits;
    }

    /** @brief 指定された位置にビット値として0を設定する。
        @param[in] in_bits     ビット集合として扱う整数値。
        @param[in] in_position 設定するビットの位置。
        @return
            指定されたビット位置に0を設定した整数値。
            ただし in_position がsizeof(int)以上だった場合、未定義。
     */
    template<typename template_bits>
    PSYQ_CONSTEXPR template_bits reset_bit_fast(
        template_bits const in_bits,
        std::size_t const in_position)
    PSYQ_NOEXCEPT
    {
        return ~psyq::shift_left_bitwise_fast(template_bits(1), in_position)
            & in_bits;
    }

    /** @brief 指定された位置にビット値として1を設定する。
        @param[in] in_bits     ビット集合として扱う整数値。
        @param[in] in_position 設定するビットの位置。
        @return
            指定されたビット位置に1を設定した整数値。
            ただし in_position がsizeof(int)以上だった場合、
            in_bits をそのまま返す。
     */
    template<typename template_bits>
    PSYQ_CONSTEXPR template_bits set_bit(
        template_bits const in_bits,
        std::size_t const in_position)
    PSYQ_NOEXCEPT
    {
        return psyq::shift_left_bitwise(template_bits(1), in_position)
            | in_bits;
    }

    /** @brief 指定された位置にビット値として1を設定する。
        @param[in] in_bits     ビット集合として扱う整数値。
        @param[in] in_position 設定するビットの位置。
        @return
            指定されたビット位置に1を設定した整数値。
            ただし in_position がsizeof(int)以上だった場合、未定義。
     */
    template<typename template_bits>
    PSYQ_CONSTEXPR template_bits set_bit_fast(
        template_bits const in_bits,
        std::size_t const in_position)
    PSYQ_NOEXCEPT
    {
        return psyq::shift_left_bitwise_fast(template_bits(1), in_position)
            | in_bits;
    }

    /** @brief 指定された位置にビット値を設定する。
        @param[in] in_bits     ビット集合として扱う整数値。
        @param[in] in_position 設定するビットの位置。
        @param[in] in_value    設定するビット値。
        @return
            指定されたビット位置に in_value を設定した整数値。
            ただし in_position がsizeof(int)以上だった場合、
            in_bits をそのまま返す。
     */
    template<typename template_bits>
    PSYQ_CONSTEXPR template_bits set_bit(
        template_bits const in_bits,
        std::size_t const in_position,
        bool const in_value)
    PSYQ_NOEXCEPT
    {
        return psyq::reset_bit(in_bits, in_position)
            | psyq::shift_left_bitwise<template_bits>(in_value, in_position);
    }

    /** @brief 指定された位置にビット値を設定する。
        @param[in] in_bits     ビット集合として扱う整数値。
        @param[in] in_position 設定するビットの位置。
        @param[in] in_value    設定するビット値。
        @return
            指定されたビット位置に in_value を設定した整数値。
            ただし in_position がsizeof(int)以上だった場合、未定義。
     */
    template<typename template_bits>
    PSYQ_CONSTEXPR template_bits set_bit_fast(
        template_bits const in_bits,
        std::size_t   const in_position,
        bool          const in_value)
    PSYQ_NOEXCEPT
    {
        return psyq::reset_bit_fast(in_bits, in_position)
            | psyq::shift_left_bitwise_fast<template_bits>(
                in_value, in_position);
    }

    /** @brief 指定された位置のビット値を反転する。
        @param[in] in_bits     ビット集合として扱う整数値。
        @param[in] in_position 反転するビットの位置。
        @return
            指定されたビット位置の値を反転した整数値。
            ただし in_position がsizeof(int)以上だった場合、
            in_bits をそのまま返す。
     */
    template<typename template_bits>
    PSYQ_CONSTEXPR template_bits flip_bit(
        template_bits const in_bits,
        std::size_t   const in_position)
    PSYQ_NOEXCEPT
    {
        return psyq::shift_left_bitwise(template_bits(1), in_position)
            ^ in_bits;
    }

    /** @brief 指定された位置のビット値を反転する。
        @param[in] in_bits     ビット集合として扱う整数値。
        @param[in] in_position 反転するビットの位置。
        @return
            指定されたビット位置の値を反転した整数値。
            ただし in_position がsizeof(int)以上だった場合、未定義。
     */
    template<typename template_bits>
    PSYQ_CONSTEXPR template_bits flip_bit_fast(
        template_bits const in_bits,
        std::size_t   const in_position)
    PSYQ_NOEXCEPT
    {
        return psyq::shift_left_bitwise_fast(template_bits(1), in_position)
            ^ in_bits;
    }

    //-------------------------------------------------------------------------
    template<typename template_bits>
    PSYQ_CONSTEXPR template_bits make_bit_mask(
        std::size_t const in_bit_width)
    {
        return ~psyq::shift_left_bitwise(~template_bits(0), in_bit_width);
    }

    /** @brief 指定されたビット範囲を取得する。
        @param[in] in_bits         ビット集合として扱う整数値。
        @param[in] in_bit_position 取得するビット範囲のビット位置。
        @param[in] in_bit_width    取得するビット範囲のビット幅。
        @return ビット範囲。
     */
    template<typename template_bits>
    PSYQ_CONSTEXPR template_bits emboss_bit_field(
        template_bits const in_bits,
        std::size_t const in_bit_position,
        std::size_t const in_bit_width)
    {
        return (
            PSYQ_ASSERT(
                psyq::_private::is_valid_bit_width<template_bits>(
                    in_bit_position, in_bit_width)),
            psyq::shift_left_bitwise_fast(
                psyq::make_bit_mask<template_bits>(in_bit_width),
                in_bit_position)
            & in_bits);
    }

    /** @brief 指定されたビット範囲の値を取得する。
        @param[in] in_bits         ビット集合として扱う整数値。
        @param[in] in_bit_position 取得するビット範囲のビット位置。
        @param[in] in_bit_width    取得するビット範囲のビット幅。
        @return ビット範囲の値。
     */
    template<typename template_bits>
    PSYQ_CONSTEXPR template_bits get_bit_field(
        template_bits const in_bits,
        std::size_t const in_bit_position,
        std::size_t const in_bit_width)
    {
        return (
            PSYQ_ASSERT(
                psyq::_private::is_valid_bit_width<template_bits>(
                    in_bit_position, in_bit_width)),
            psyq::make_bit_mask<template_bits>(in_bit_width)
            & psyq::shift_right_bitwise_fast(in_bits, in_bit_position));
    }

    /** @brief 指定されたビット範囲を0にする。
        @param[in] in_bits         ビット集合として扱う整数値。
        @param[in] in_bit_position 設定するビット範囲のビット位置。
        @param[in] in_bit_width    設定するビット範囲のビット幅。
        @return 指定されたビット範囲を0にした整数値。
     */
    template<typename template_bits>
    PSYQ_CONSTEXPR template_bits reset_bit_field(
        template_bits const in_bits,
        std::size_t const in_bit_position,
        std::size_t const in_bit_width)
    {
        return (
            PSYQ_ASSERT(
                psyq::_private::is_valid_bit_width<template_bits>(
                    in_bit_position, in_bit_width)),
            in_bits & ~psyq::shift_left_bitwise_fast(
                psyq::make_bit_mask<template_bits>(in_bit_width),
                in_bit_position));
    }

    /** @brief 指定されたビット範囲に値を埋め込む。
        @param[in] in_bits         ビット集合として扱う整数値。
        @param[in] in_bit_position 埋め込むビット範囲のビット位置。
        @param[in] in_bit_width    埋め込むビット範囲のビット幅。
        @param[in] in_value        埋め込む値。
        @return 指定されたビット位置に in_value を埋め込んだ整数値。
     */
    template<typename template_bits>
    PSYQ_CONSTEXPR template_bits set_bit_field(
        template_bits const in_bits,
        std::size_t   const in_bit_position,
        std::size_t   const in_bit_width,
        template_bits const in_value)
    PSYQ_NOEXCEPT
    {
        return (
            PSYQ_ASSERT(
                psyq::shift_right_bitwise(in_value, in_bit_width) == 0),
            psyq::reset_bit_field(in_bits, in_bit_position, in_bit_width)
            | psyq::shift_left_bitwise_fast(in_value, in_bit_position));
    }

    //-------------------------------------------------------------------------
    namespace _private
    {
        /** @brief 組み込み整数型から、同じ大きさのstd::uint*_t型に変換する。
            @tparam template_type 元となる型。
         */
        template<typename template_type> struct make_std_uint
        {
            /** @brief template_type型から変換した、std::uint*_t型。

                変換できない場合は、void型となる。
             */
            typedef
                typename std::conditional<
                    !std::is_integral<template_type>::value,
                    void,
                typename std::conditional<
                    sizeof(template_type) == sizeof(std::uint8_t),
                    std::uint8_t,
                typename std::conditional<
                    sizeof(template_type) == sizeof(std::uint16_t),
                    std::uint16_t,
                typename std::conditional<
                    sizeof(template_type) == sizeof(std::uint32_t),
                    std::uint32_t,
                typename std::conditional<
                    sizeof(template_type) == sizeof(std::uint64_t),
                    std::uint64_t,
                    void
                >::type>::type>::type>::type>::type
                    type;
        };

        //---------------------------------------------------------------------
        /** @brief 無符号整数で、1になっているビットを数える。

            以下のウェブページを参考にした。
            http://www.nminoru.jp/~nminoru/programming/bitcount.html

            @param[in] in_bits  ビットを数える無符号整数の値。
            @return 1になっているビットの数。
         */
        inline std::size_t count_1bits_by_table(std::uint8_t const in_bits)
        PSYQ_NOEXCEPT
        {
            static std::uint8_t const BITS_COUNT_TABLE[256] =
            {
                0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4,
                1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
                1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
                2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
                1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
                2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
                2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
                3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
                1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
                2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
                2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
                3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
                2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
                3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
                3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
                4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,
            };
            return BITS_COUNT_TABLE[in_bits];
        }

        /// @copydoc count_1bits_by_table()
        inline std::size_t count_1bits_by_table(std::uint16_t const in_bits)
        PSYQ_NOEXCEPT
        {
            return count_1bits_by_table(static_cast<std::uint8_t>(in_bits))
                + count_1bits_by_table(
                    static_cast<std::uint8_t>(in_bits >> psyq::CHAR_BIT_WIDTH));
        }

        /// @copydoc count_1bits_by_table()
        inline std::size_t count_1bits_by_table(std::uint32_t const in_bits)
        PSYQ_NOEXCEPT
        {
            return count_1bits_by_table(static_cast<std::uint16_t>(in_bits))
                + count_1bits_by_table(
                    static_cast<std::uint16_t>(in_bits >> 16));
        }

        /// @copydoc count_1bits_by_table()
        inline std::size_t count_1bits_by_table(std::uint64_t const in_bits)
        PSYQ_NOEXCEPT
        {
            return count_1bits_by_table(static_cast<std::uint32_t>(in_bits))
                + count_1bits_by_table(
                    static_cast<std::uint32_t>(in_bits >> 32));
        }

        //---------------------------------------------------------------------
        /** @brief 無符号整数で、1になっているビットを数える。

            以下のウェブページを参考にした。
            http://www.nminoru.jp/~nminoru/programming/bitcount.html

            @param[in] in_bits  ビットを数える無符号整数の値。
            @return 1になっているビットの数。
         */
        inline std::size_t count_1bits_by_logical(std::uint8_t const in_bits)
        PSYQ_NOEXCEPT
        {
            unsigned local_bits(in_bits);
            local_bits = (local_bits & 0x55) + ((local_bits >> 1) & 0x55);
            local_bits = (local_bits & 0x33) + ((local_bits >> 2) & 0x33);
            local_bits = (local_bits & 0x0F) + ((local_bits >> 4) & 0x0F);
            return local_bits;
        }

        /// @copydoc count_1bits_by_logical()
        inline std::size_t count_1bits_by_logical(std::uint16_t const in_bits)
        PSYQ_NOEXCEPT
        {
            unsigned local_bits(in_bits);
            local_bits = (local_bits & 0x5555) + ((local_bits >> 1) & 0x5555);
            local_bits = (local_bits & 0x3333) + ((local_bits >> 2) & 0x3333);
            local_bits = (local_bits & 0x0F0F) + ((local_bits >> 4) & 0x0F0F);
            local_bits = (local_bits & 0x00FF) + ((local_bits >> 8) & 0x00FF);
            return local_bits;
        }

        /// @copydoc count_1bits_by_logical()
        inline std::size_t count_1bits_by_logical(std::uint32_t const in_bits)
        PSYQ_NOEXCEPT
        {
            unsigned local_bits(in_bits);
            local_bits = (local_bits & 0x55555555)
                + ((local_bits >> 1) & 0x55555555);
            local_bits = (local_bits & 0x33333333)
                + ((local_bits >> 2) & 0x33333333);
            local_bits = (local_bits & 0x0F0F0F0F)
                + ((local_bits >> 4) & 0x0F0F0F0F);
            local_bits = (local_bits & 0x00FF00FF)
                + ((local_bits >> 8) & 0x00FF00FF);
            local_bits = (local_bits & 0x0000FFFF)
                + ((local_bits >>16) & 0x0000FFFF);
            return local_bits;
        }

        /// @copydoc count_1bits_by_logical()
        inline std::size_t count_1bits_by_logical(std::uint64_t const in_bits)
        PSYQ_NOEXCEPT
        {
            auto local_bits(in_bits);
            local_bits = (local_bits & 0x5555555555555555)
                + ((local_bits >> 1) & 0x5555555555555555);
            local_bits = (local_bits & 0x3333333333333333)
                + ((local_bits >> 2) & 0x3333333333333333);
            local_bits = (local_bits & 0x0F0F0F0F0F0F0F0F)
                + ((local_bits >> 4) & 0x0F0F0F0F0F0F0F0F);
            local_bits = (local_bits & 0x00FF00FF00FF00FF)
                + ((local_bits >> 8) & 0x00FF00FF00FF00FF);
            local_bits = (local_bits & 0x0000FFFF0000FFFF)
                + ((local_bits >>16) & 0x0000FFFF0000FFFF);
            local_bits = (local_bits & 0x00000000FFFFFFFF)
                + ((local_bits >>32) & 0x00000000FFFFFFFF);
            return static_cast<std::size_t>(local_bits);
        }

        //---------------------------------------------------------------------
        /** @brief 無符号整数で、1になっているビットを数える。
            @param[in] in_bits ビットを数える無符号整数の値。
            @return 1になってるビットの数。
         */
        template<typename template_bits>
        std::size_t count_1bits_of_uint(template_bits const in_bits)
        PSYQ_NOEXCEPT
        {
            static_assert(
                // in_bitsのビット数は、32以下であること。
                sizeof(in_bits) <= sizeof(std::uint32_t),
                "Bit size of 'in_bits' must be less than or equal to 32.");
            static_assert(
                // in_bits は、無符号整数型であること。
                std::is_unsigned<template_bits>::value,
                "'in_bits' must be unsigned integer type.");
#if defined(PSYQ_BIT_ALGORITHM_FOR_MSC)
            return __popcnt(in_bits);
#elif defined(PSYQ_BIT_ALGORITHM_FOR_GNUC)
            return __builtin_popcount(in_bits);
#else
            return psyq::_private::count_1bits_by_table(in_bits);
#endif
        }

        /** @brief 64ビット無符号整数で、1になっているビットを数える。
            @param[in] in_bits ビットを数える無符号整数の値。
            @return 1になってるビットの数。
         */
        template<> inline std::size_t count_1bits_of_uint(
            std::uint64_t const in_bits)
        PSYQ_NOEXCEPT
        {
#if PSYQ_BIT_ALGORITHM_INTRINSIC_SIZE < 64
            // 上位32ビットと下位32ビットに分ける。
            auto const local_high_count(
                psyq::_private::count_1bits_of_uint(
                    static_cast<std::uint32_t>(in_bits >> 32)));
            auto const local_low_count(
                psyq::_private::count_1bits_of_uint(
                    static_cast<std::uint32_t>(in_bits)));
            return local_high_count + local_low_count;
#elif defined(PSYQ_BIT_ALGORITHM_FOR_MSC)
            return __popcnt64(in_bits);
#elif defined(PSYQ_BIT_ALGORITHM_FOR_GNUC) && ULONG_MAX == 0xFFFFFFFFFFFFFFFF
            return __builtin_popcountl(in_bits);
#elif defined(PSYQ_BIT_ALGORITHM_FOR_GNUC) && ULLONG_MAX == 0xFFFFFFFFFFFFFFFF
            return __builtin_popcountll(in_bits);
#else
            return psyq::_private::count_1bits_by_table(in_bits);
#endif
        }

        //---------------------------------------------------------------------
        /** @brief 浮動小数点実数のビット値を取得する。

            strict-aliasing ruleに抵触しないように、unionを使って取得する。
            http://homepage1.nifty.com/herumi/diary/0911.html#10

            @param[in] in_float ビット値を取得するする浮動小数点実数。
            @return 浮動小数点実数のビット値。
         */
        inline uint32_t get_float_bit_value(float const in_float) PSYQ_NOEXCEPT
        {
            union
            {
                float         value;
                std::uint32_t bit_value;
            } local_float;
            local_float.value = in_float;
            return local_float.bit_value;
        }

        /// @copydoc get_float_bit_value()
        inline std::uint64_t get_float_bit_value(double const in_float)
        PSYQ_NOEXCEPT
        {
            union
            {
                double        value;
                std::uint64_t bit_value;
            } local_float;
            local_float.value = in_float;
            return local_float.bit_value;
        }

        //---------------------------------------------------------------------
        /** @brief 無符号整数の最上位ビットから、0が連続する数を数える。

            以下のウェブページを参考にした。
            http://www.nminoru.jp/~nminoru/programming/bitcount.html

            @param[in] in_bits ビットを数える無符号整数の値。
            @return 最上位ビットから0が連続する数。
         */
        inline std::size_t count_leading_0bits_by_logical(
            std::uint8_t const in_bits)
        PSYQ_NOEXCEPT
        {
            unsigned local_bits(in_bits);
            local_bits = local_bits | (local_bits >> 1);
            local_bits = local_bits | (local_bits >> 2);
            local_bits = local_bits | (local_bits >> 4);
            return psyq::_private::count_1bits_of_uint(
                static_cast<std::uint8_t>(~local_bits));
        }

        /// @copydoc count_leading_0bits_by_logical()
        inline std::size_t count_leading_0bits_by_logical(
            std::uint16_t const in_bits)
        PSYQ_NOEXCEPT
        {
            unsigned local_bits(in_bits);
            local_bits = local_bits | (local_bits >> 1);
            local_bits = local_bits | (local_bits >> 2);
            local_bits = local_bits | (local_bits >> 4);
            local_bits = local_bits | (local_bits >> 8);
            return psyq::_private::count_1bits_of_uint(
                static_cast<std::uint16_t>(~local_bits));
        }

        /// @copydoc count_leading_0bits_by_logical()
        inline std::size_t count_leading_0bits_by_logical(
            std::uint32_t const in_bits)
        PSYQ_NOEXCEPT
        {
            unsigned local_bits(in_bits);
            local_bits = local_bits | (local_bits >> 1);
            local_bits = local_bits | (local_bits >> 2);
            local_bits = local_bits | (local_bits >> 4);
            local_bits = local_bits | (local_bits >> 8);
            local_bits = local_bits | (local_bits >>16);
            return psyq::_private::count_1bits_of_uint(
                static_cast<std::uint32_t>(~local_bits));
        }

        /// @copydoc count_leading_0bits_by_logical()
        inline std::size_t count_leading_0bits_by_logical(
            std::uint64_t in_bits)
        PSYQ_NOEXCEPT
        {
            auto local_bits(in_bits);
            local_bits = local_bits | (local_bits >> 1);
            local_bits = local_bits | (local_bits >> 2);
            local_bits = local_bits | (local_bits >> 4);
            local_bits = local_bits | (local_bits >> 8);
            local_bits = local_bits | (local_bits >>16);
            local_bits = local_bits | (local_bits >>32);
            return psyq::_private::count_1bits_of_uint(~local_bits);
        }

        //---------------------------------------------------------------------
        /** @brief 浮動小数点のビットパターンを使って、
                   無符号整数の最上位ビットから、0が連続する数を数える。

            以下のウェブページを参考にした。
            http://www.nminoru.jp/~nminoru/programming/bitcount.html

            @param[in] in_bits ビットを数える整数の値。
            @return 最上位ビットから0が連続する数。
         */
        template<typename template_bits>
        std::size_t count_leading_0bits_by_float(template_bits const in_bits)
        {
            static_assert(
                // in_bits は、無符号整数型であること。
                std::is_unsigned<template_bits>::value,
                "'in_bits' must be unsigned integer type.");
            static_assert(
                // 浮動小数点の基数は、2であること。
                FLT_RADIX == 2, "Floating point radix must be 2.");
            static_assert(
                // in_bits のビット数は、FLT_MANT_DIG未満であること。
                sizeof(in_bits) * psyq::CHAR_BIT_WIDTH < FLT_MANT_DIG,
                "Bit size of 'in_bits' must be less than FLT_MANT_DIG.");
            return sizeof(in_bits) * psyq::CHAR_BIT_WIDTH + (1 - FLT_MIN_EXP) - (
                psyq::_private::get_float_bit_value(in_bits + 0.5f)
                >> (FLT_MANT_DIG - 1));
        }

        /// @copydoc count_leading_0bits_by_float()
        template<> inline std::size_t count_leading_0bits_by_float(
            std::uint32_t const in_bits)
        {
            static_assert(
                // 浮動小数点の基数は、2であること。
                FLT_RADIX == 2, "Floating point radix must be 2.");
            static_assert(
                // in_bits のビット数は、DBL_MANT_DIG未満であること。
                sizeof(in_bits) * psyq::CHAR_BIT_WIDTH < DBL_MANT_DIG,
                "Bit size of 'in_bits' must be less than DBL_MANT_DIG.");
            return sizeof(in_bits) * psyq::CHAR_BIT_WIDTH + (1 - DBL_MIN_EXP) - (
                psyq::_private::get_float_bit_value(in_bits + 0.5)
                >> (DBL_MANT_DIG - 1));
        }

        //---------------------------------------------------------------------
        /** @brief 無符号整数の最上位ビットから、0が連続する数を数える。
            @param[in] in_bits ビットを数える整数の値。
            @return 最上位ビットから0が連続する数。
         */
        template<typename template_bits>
        std::size_t count_leading_0bits_of_uint(template_bits const in_bits)
        {
            static_assert(
                // in_bitsのビット数は、32以下であること。
                sizeof(in_bits) <= sizeof(std::uint32_t),
                "Bit size of 'in_bits' must be less than or equal to 32.");
            static_assert(
                // in_bits は、無符号整数型であること。
                std::is_unsigned<template_bits>::value,
                "'in_bits' must be unsigned integer type.");
            enum: unsigned
            {
                BIT_SIZE = sizeof(in_bits) * psyq::CHAR_BIT_WIDTH,
                SIZE_DIFF = 32 - BIT_SIZE,
            };
#if defined(PSYQ_BIT_ALGORITHM_FOR_MSC) && defined(BitScanReverse)
            if (in_bits == 0)
            {
                return BIT_SIZE;
            }
            unsigned long local_index;
            BitScanReverse(&local_index, in_bits);
            return BIT_SIZE - 1 - local_index;
#elif defined(PSYQ_BIT_ALGORITHM_FOR_MSC) && defined(CountLeadingZeros)
            return CountLeadingZeros(in_bits) - SIZE_DIFF;
#elif defined(PSYQ_BIT_ALGORITHM_FOR_GNUC)
            return in_bits != 0? __builtin_clz(in_bits) - SIZE_DIFF: BIT_SIZE;
#elif defined(PSYQ_BIT_ALGORITHM_FOR_ARMCC)
            /// @note ARMのclzは、0判定がなくてもいい気がする。
            return in_bits != 0? __clz(in_bits) - SIZE_DIFF: BIT_SIZE;
#elif defined(PSYQ_BIT_ALGORITHM_FOR_GHS)
            return in_bits != 0? __CLZ(in_bits) - SIZE_DIFF: BIT_SIZE;
#elif defined(PSYQ_COUNT_LEADING_0BITS_BY_FLOAT)
            return psyq::_private::count_leading_0bits_by_float(in_bits);
#else
            return psyq::_private::count_leading_0bits_by_logical(in_bits);
#endif
        }

        /** @brief 64ビット無符号整数の最上位ビットから、0が連続する数を数える。
            @param[in] in_bits ビットを数える整数の値。
            @return 最上位ビットから0が連続する数。
         */
        template<> inline std::size_t count_leading_0bits_of_uint(
            std::uint64_t const in_bits)
        {
#if PSYQ_BIT_ALGORITHM_INTRINSIC_SIZE < 64
#   if defined(PSYQ_COUNT_LEADING_0BITS_BY_FLOAT)
            if ((in_bits >> (DBL_MANT_DIG - 1)) == 0)
            {
                // 浮動小数点を利用し、最上位ビットから0が連続する数を数える。
                return sizeof(in_bits) * psyq::CHAR_BIT_WIDTH + (1 - DBL_MIN_EXP) - (
                    psyq::_private::get_float_bit_value(in_bits + 0.5)
                    >> (DBL_MANT_DIG - 1));
            }
            else
            {
                static_assert(
                    // DBL_MANT_DIGは、48より大きいこと。
                    48 < DBL_MANT_DIG,
                    "DBL_MANT_DIG must be greater than 48.");
                return psyq::_private::count_leading_0bits_by_float(
                    static_cast<std::uint16_t>(in_bits >> 48));
            }
#   elif 1
            // 上位32ビットと下位32ビットに分ける。
            auto const local_high_bits(
                static_cast<std::uint32_t>(in_bits >> 32));
            if (local_high_bits != 0)
            {
                return psyq::_private::count_leading_0bits_by_logical(
                    local_high_bits);
            }
            return 32 + psyq::_private::count_leading_0bits_by_logical(
                static_cast<std::uint32_t>(in_bits));
#   else
            // 64ビットのまま処理する。
            return psyq::_private::count_leading_0bits_by_logical(in_bits);
#   endif
#elif defined(PSYQ_BIT_ALGORITHM_FOR_MSC) && defined(BitScanReverse64)
            if (in_bits == 0)
            {
                return 64;
            }
            unsigned long local_index;
            BitScanReverse64(&local_index, in_bits);
            return 63 - local_index;
#elif defined(PSYQ_BIT_ALGORITHM_FOR_MSC) && defined(CountLeadingZeros64)
            return CountLeadingZeros64(in_bits);
#elif defined(PSYQ_BIT_ALGORITHM_FOR_GNUC) && ULONG_MAX == 0xFFFFFFFFFFFFFFFF
            return in_bits != 0? __builtin_clzl(in_bits): 64;
#elif defined(PSYQ_BIT_ALGORITHM_FOR_GNUC) && ULLONG_MAX == 0xFFFFFFFFFFFFFFFF
            return in_bits != 0? __builtin_clzll(in_bits): 64;
#else
            return psyq::_private::count_leading_0bits_by_logical(in_bits);
#endif
        }

        //---------------------------------------------------------------------
        /** @brief 無符号整数の最下位ビットから、0が連続する数を数える。

            以下のウェブページを参考にした。
            http://www.nminoru.jp/~nminoru/programming/bitcount.html

            @param[in] in_bits 数える整数の値。
            @return 最下位ビットから、0が連続する数。
         */
        template<typename template_bits>
        std::size_t count_trailing_0bits_by_logical(template_bits const in_bits)
        {
            return psyq::_private::count_1bits_of_uint(
                static_cast<template_bits>((~in_bits) & (in_bits - 1)));
        }

        /** @brief 無符号整数の最下位ビットから、0が連続する数を数える。
            @param[in] in_bits ビットを数える整数の値。
            @return 最下位ビットから、0が連続する数。
         */
        template<typename template_bits>
        std::size_t count_trailing_0bits_of_uint(template_bits const in_bits)
        {
            static_assert(
                // in_bitsのビット数は、32以下であること。
                sizeof(in_bits) <= sizeof(std::uint32_t),
                "Bit size of 'in_bits' must be less than or equal to 32.");
            static_assert(
                // in_bits は、無符号整数型であること。
                std::is_unsigned<template_bits>::value,
                "'in_bits' must be unsigned integer type.");
#if defined(PSYQ_BIT_ALGORITHM_FOR_MSC) && defined(BitScanForward)
            if (in_bits == 0)
            {
                return sizeof(in_bits) * psyq::CHAR_BIT_WIDTH;
            }
            unsigned long local_index;
            BitScanForward(&local_index, in_bits);
            return local_index;
#elif defined(PSYQ_BIT_ALGORITHM_FOR_GNUC)
            return in_bits != 0?
                __builtin_ctz(in_bits): sizeof(in_bits) * psyq::CHAR_BIT_WIDTH;
#else
            return psyq::_private::count_trailing_0bits_by_logical(in_bits);
#endif
        }

        /** @brief 64ビット無符号整数の最下位ビットから、0が連続する数を数える。
            @param[in] in_bits ビットを数える整数の値。
            @return 最下位ビットから、0が連続する数。
         */
        template<> inline std::size_t count_trailing_0bits_of_uint(
            std::uint64_t const in_bits)
        {
#if PSYQ_BIT_ALGORITHM_INTRINSIC_SIZE < 64
            // 上位32ビットと下位32ビットに分ける。
            auto const local_low_count(
                psyq::_private::count_trailing_0bits_of_uint(
                    static_cast<std::uint32_t>(in_bits)));
            if (local_low_count < 32)
            {
                return local_low_count;
            }
            return 32 + psyq::_private::count_trailing_0bits_of_uint(
                static_cast<std::uint32_t>(in_bits >> 32));
#elif defined(PSYQ_BIT_ALGORITHM_FOR_MSC) && defined(BitScanForward64)
            if (in_bits == 0)
            {
                return 64;
            }
            unsigned long local_index;
            BitScanForward64(&local_index, in_bits);
            return local_index;
#elif defined(PSYQ_BIT_ALGORITHM_FOR_GNUC) && ULONG_MAX == 0xFFFFFFFFFFFFFFFF
            return in_bits != 0? __builtin_ctzl(in_bits): 64;
#elif defined(PSYQ_BIT_ALGORITHM_FOR_GNUC) && ULLONG_MAX == 0xFFFFFFFFFFFFFFFF
            return in_bits != 0? __builtin_ctzll(in_bits): 64;
#else
            return psyq::_private::count_trailing_0bits_by_logical(in_bits);
#endif
        }
    }

    //-------------------------------------------------------------------------
    /** @brief 整数で、1になっているビットを数える。
        @param[in] in_bits  ビットを数える整数の値。
        @return 1になっているビットの数。
     */
    template<typename template_bits>
    std::size_t count_1bits(template_bits const in_bits)
    {
        typedef typename psyq::_private::make_std_uint<template_bits>::type
            std_uint;
        static_assert(
            // template_bits は、64ビット以下の整数型であること。
            std::is_unsigned<std_uint>::value,
            "'template_bits' must be integer type of 64bits or less.");
        return psyq::_private::count_1bits_of_uint(
            static_cast<std_uint>(in_bits));
    }

    //-------------------------------------------------------------------------
    /** @brief 整数の最上位ビットから、0が連続する数を数える。
        @param[in] in_bits ビットを数える整数の値。
        @return 最上位ビットから0が連続する数。
     */
    template<typename template_bits>
    std::size_t count_leading_0bits(template_bits const in_bits)
    {
        typedef typename psyq::_private::make_std_uint<template_bits>::type
            std_uint;
        static_assert(
            // template_bits は、64ビット以下の整数型であること。
            std::is_unsigned<std_uint>::value,
            "'template_bits' must be integer type of 64bits or less.");
        return psyq::_private::count_leading_0bits_of_uint(
            static_cast<std_uint>(in_bits));
    }

    //-------------------------------------------------------------------------
    /** @brief 整数の最下位ビットから、0が連続する数を数える。
        @param[in] in_bits ビットを数える整数の値。
        @return 最下位ビットから0が連続する数。
     */
    template<typename template_bits>
    std::size_t count_trailing_0bits(template_bits const in_bits)
    {
        typedef typename psyq::_private::make_std_uint<template_bits>::type
            std_uint;
        static_assert(
            // template_bits は、64ビット以下の整数型であること。
            std::is_unsigned<std_uint>::value,
            "'template_bits' must be integer type of 64bits or less.");
        return psyq::_private::count_trailing_0bits_of_uint(
            static_cast<std_uint>(in_bits));
    }
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 単体テストに使う。この名前空間をuserが直接accessするのは禁止。
namespace psyq_test
{
    template<typename template_value> void count_1bits()
    {
        template_value local_bits(0);
        PSYQ_ASSERT(psyq::count_1bits(local_bits) == 0);
        for (unsigned i(0); i < sizeof(template_value) * psyq::CHAR_BIT_WIDTH; ++i)
        {
            local_bits = (local_bits << 1) | 1;
            PSYQ_ASSERT(i + 1 == psyq::count_1bits(local_bits));
        }
    }

    inline void count_1bits()
    {
        psyq_test::count_1bits<char>();
        psyq_test::count_1bits<short>();
        psyq_test::count_1bits<int>();
        psyq_test::count_1bits<long>();
#if ULLONG_MAX <= 0xFFFFFFFFFFFFFFFF
        psyq_test::count_1bits<long long>();
#endif
    }

    template<typename template_value> void count_leading_0bits()
    {
        PSYQ_ASSERT(
            psyq::count_leading_0bits(template_value(0))
            == sizeof(template_value) * psyq::CHAR_BIT_WIDTH);
        for (unsigned i(0); i < sizeof(template_value) * psyq::CHAR_BIT_WIDTH; ++i)
        {
            auto const local_clz(
                psyq::count_leading_0bits(
                    template_value(template_value(1) << i)));
            PSYQ_ASSERT(
                local_clz + i
                == sizeof(template_value) * psyq::CHAR_BIT_WIDTH - 1);
        }
    }

    inline void count_leading_0bits()
    {
        psyq_test::count_leading_0bits<char>();
        psyq_test::count_leading_0bits<short>();
        psyq_test::count_leading_0bits<int>();
        psyq_test::count_leading_0bits<long>();
#if ULLONG_MAX <= 0xFFFFFFFFFFFFFFFF
        psyq_test::count_leading_0bits<long long>();
#endif
    }

    template<typename template_value> void count_trailing_0bits()
    {
        PSYQ_ASSERT(
            psyq::count_trailing_0bits(template_value(0))
            == sizeof(template_value) * psyq::CHAR_BIT_WIDTH);
        for (unsigned i(0); i < sizeof(template_value) * psyq::CHAR_BIT_WIDTH; ++i)
        {
            auto const local_ctz(
                psyq::count_trailing_0bits(
                    template_value(template_value(1) << i)));
            PSYQ_ASSERT(local_ctz == i);
        }
    }

    inline void count_trailing_0bits()
    {
        psyq_test::count_trailing_0bits<char>();
        psyq_test::count_trailing_0bits<short>();
        psyq_test::count_trailing_0bits<int>();
        psyq_test::count_trailing_0bits<long>();
#if ULLONG_MAX <= 0xFFFFFFFFFFFFFFFF
        psyq_test::count_trailing_0bits<long long>();
#endif
    }
}

#endif // PSYQ_BIT_ALGORITHM_HPP_
// vim: set expandtab:
