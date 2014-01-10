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
 */
#ifndef PSYQ_BIT_ALGORITHM_HPP_
#define PSYQ_BIT_ALGORITHM_HPP_
#include <cstdint>

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
    bool get_bit(
        template_bits const in_bits,
        std::size_t   const in_position)
    {
        return in_position < sizeof(int) * 8?
            psyq::get_bit_fast(in_bits, in_position): in_bits < 0;
    }

    /** @brief 指定された位置のビット値を取得する。
        @param[in] in_bits ビット集合として扱う整数値。
        @param[in] in_position 取得するビットの位置。
        @return
            指定された位置のビット値。
            ただし in_position がsizeof(int)以上だった場合、未定義。
     */
    template<typename template_bits>
    bool get_bit_fast(
        template_bits const in_bits,
        std::size_t   const in_position)
    {
        return ((in_bits >> in_position) & 1) != 0;
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
    template_bits set_bit(
        template_bits const in_bits,
        std::size_t   const in_position)
    {
        return in_position < sizeof(in_bits) * 8?
            psyq::set_bit_fast(in_bits, in_position): in_bits;
    }

    /** @brief 指定された位置にビット値として1を設定する。
        @param[in] in_bits     ビット集合として扱う整数値。
        @param[in] in_position 設定するビットの位置。
        @return
            指定されたビット位置に1を設定した整数値。
            ただし in_position がsizeof(int)以上だった場合、未定義。
     */
    template<typename template_bits>
    template_bits set_bit_fast(
        template_bits const in_bits,
        std::size_t   const in_position)
    {
        return static_cast<template_bits>((1 << in_position) | in_bits);
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
    template_bits set_bit(
        template_bits const in_bits,
        std::size_t   const in_position,
        bool          const in_value)
    {
        return in_position < sizeof(in_bits) * 8?
            psyq::set_bit_fast(in_bits, in_position, in_value): in_bits;
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
    template_bits set_bit_fast(
        template_bits const in_bits,
        std::size_t   const in_position,
        bool          const in_value)
    {
        return psyq::reset_bit_fast(in_bits, in_position)
            | (in_value << in_position);
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
    template_bits reset_bit(
        template_bits const in_bits,
        std::size_t   const in_position)
    {
        return in_position < sizeof(in_bits) * 8?
            psyq::reset_bit_fast(in_bits, in_position): in_bits;
    }

    /** @brief 指定された位置にビット値として0を設定する。
        @param[in] in_bits     ビット集合として扱う整数値。
        @param[in] in_position 設定するビットの位置。
        @return
            指定されたビット位置に0を設定した整数値。
            ただし in_position がsizeof(int)以上だった場合、未定義。
     */
    template<typename template_bits>
    template_bits reset_bit_fast(
        template_bits const in_bits,
        std::size_t   const in_position)
    {
        return static_cast<template_bits>(~(1 << in_position) & in_bits);
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
    template_bits flip_bit(
        template_bits const in_bits,
        std::size_t   const in_position)
    {
        return in_position < sizeof(in_bits) * 8?
            psyq::flip_bit_fast(in_bits, in_position): in_bits;
    }

    /** @brief 指定された位置のビット値を反転する。
        @param[in] in_bits     ビット集合として扱う整数値。
        @param[in] in_position 反転するビットの位置。
        @return
            指定されたビット位置の値を反転した整数値。
            ただし in_position がsizeof(int)以上だった場合、未定義。
     */
    template<typename template_bits>
    template_bits flip_bit_fast(
        template_bits const in_bits,
        std::size_t   const in_position)
    {
        return static_cast<template_bits>((1 << in_position) ^ in_bits);
    }

    //-------------------------------------------------------------------------
    namespace internal
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
        inline std::size_t count_1bits_by_logical(std::uint8_t const in_bits)
        {
            unsigned local_bits(in_bits);
            local_bits = (local_bits & 0x55) + ((local_bits >> 1) & 0x55);
            local_bits = (local_bits & 0x33) + ((local_bits >> 2) & 0x33);
            local_bits = (local_bits & 0x0f) + ((local_bits >> 4) & 0x0f);
            return local_bits;
        }

        /// @copydoc count_1bits_by_logical()
        inline std::size_t count_1bits_by_logical(std::uint16_t const in_bits)
        {
            unsigned local_bits(in_bits);
            local_bits = (local_bits & 0x5555) + ((local_bits >> 1) & 0x5555);
            local_bits = (local_bits & 0x3333) + ((local_bits >> 2) & 0x3333);
            local_bits = (local_bits & 0x0f0f) + ((local_bits >> 4) & 0x0f0f);
            local_bits = (local_bits & 0x00ff) + ((local_bits >> 8) & 0x00ff);
            return local_bits;
        }

        /// @copydoc count_1bits_by_logical()
        inline std::size_t count_1bits_by_logical(std::uint32_t const in_bits)
        {
            unsigned local_bits(in_bits);
            local_bits = (local_bits & 0x55555555)
                + ((local_bits >> 1) & 0x55555555);
            local_bits = (local_bits & 0x33333333)
                + ((local_bits >> 2) & 0x33333333);
            local_bits = (local_bits & 0x0f0f0f0f)
                + ((local_bits >> 4) & 0x0f0f0f0f);
            local_bits = (local_bits & 0x00ff00ff)
                + ((local_bits >> 8) & 0x00ff00ff);
            local_bits = (local_bits & 0x0000ffff)
                + ((local_bits >>16) & 0x0000ffff);
            return local_bits;
        }

        /// @copydoc count_1bits_by_logical()
        inline std::size_t count_1bits_by_logical(std::uint64_t const in_bits)
        {
            auto local_bits(in_bits);
            local_bits = (local_bits & 0x5555555555555555)
                + ((local_bits >> 1) & 0x5555555555555555);
            local_bits = (local_bits & 0x3333333333333333)
                + ((local_bits >> 2) & 0x3333333333333333);
            local_bits = (local_bits & 0x0f0f0f0f0f0f0f0f)
                + ((local_bits >> 4) & 0x0f0f0f0f0f0f0f0f);
            local_bits = (local_bits & 0x00ff00ff00ff00ff)
                + ((local_bits >> 8) & 0x00ff00ff00ff00ff);
            local_bits = (local_bits & 0x0000ffff0000ffff)
                + ((local_bits >>16) & 0x0000ffff0000ffff);
            local_bits = (local_bits & 0x00000000ffffffff)
                + ((local_bits >>32) & 0x00000000ffffffff);
            return static_cast<std::size_t>(local_bits);
        }

        //---------------------------------------------------------------------
        /** @brief 無符号整数で、1になっているビットを数える。
            @param[in] in_bits ビットを数える無符号整数の値。
            @return 1になってるビットの数。
         */
        template<typename template_bits>
        std::size_t count_1bits_of_uint(template_bits const in_bits)
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
            return psyq::internal::count_1bits_by_logical(in_bits);
#endif
        }

        /** @brief 64ビット無符号整数で、1になっているビットを数える。
            @param[in] in_bits ビットを数える無符号整数の値。
            @return 1になってるビットの数。
         */
        template<> std::size_t count_1bits_of_uint(std::uint64_t const in_bits)
        {
#if PSYQ_BIT_ALGORITHM_INTRINSIC_SIZE < 64
            // 上位32ビットと下位32ビットに分ける。
            auto const local_high_count(
                psyq::internal::count_1bits_of_uint(
                    static_cast<std::uint32_t>(in_bits >> 32)));
            auto const local_low_count(
                psyq::internal::count_1bits_of_uint(
                    static_cast<std::uint32_t>(in_bits)));
            return local_high_count + local_low_count;
#elif defined(PSYQ_BIT_ALGORITHM_FOR_MSC)
            return __popcnt64(in_bits);
#elif defined(PSYQ_BIT_ALGORITHM_FOR_GNUC) && ULONG_MAX == 0xffffffffffffffff
            return __builtin_popcountl(in_bits);
#elif defined(PSYQ_BIT_ALGORITHM_FOR_GNUC) && ULLONG_MAX == 0xffffffffffffffff
            return __builtin_popcountll(in_bits);
#else
            return psyq::internal::count_1bits_by_logical(in_bits);
#endif
        }

        //---------------------------------------------------------------------
        /** @brief 浮動小数点実数のビット値を取得する。

            strict-aliasing ruleに抵触しないように、unionを使って取得する。
            http://homepage1.nifty.com/herumi/diary/0911.html#10

            @param[in] in_float ビット値を取得するする浮動小数点実数。
            @return 浮動小数点実数のビット値。
         */
        inline uint32_t get_float_bit_value(float const in_float)
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
            http://tlsf.baisoku.org/

            @param[in] in_bits ビットを数える無符号整数の値。
            @return 最上位ビットから0が連続する数。
         */
        inline std::size_t count_leading_0bits_by_logical(
            std::uint8_t const in_bits)
        {
            unsigned local_bits(in_bits);
            unsigned local_fls(sizeof(in_bits) * 8);
            if (local_bits == 0)
            {
                --local_fls;
            }
            if ((local_bits & 0xf0) == 0)
            {
                local_bits <<= 4;
                local_fls -= 4;
            }
            if ((local_bits & 0xc0) == 0)
            {
                local_bits <<= 2;
                local_fls -= 2;
            }
            if ((local_bits & 0x80) == 0)
            {
                local_bits <<= 1;
                local_fls -= 1;
            }
            return sizeof(in_bits) * 8 - local_fls;
        }

        /// @copydoc count_leading_0bits_by_logical()
        inline std::size_t count_leading_0bits_by_logical(
            std::uint16_t const in_bits)
        {
            unsigned local_bits(in_bits);
            unsigned local_fls(sizeof(in_bits) * 8);
            if (local_bits == 0)
            {
                --local_fls;
            }
            if ((local_bits & 0xff00) == 0)
            {
                local_bits <<= 8;
                local_fls -= 8;
            }
            if ((local_bits & 0xf000) == 0)
            {
                local_bits <<= 4;
                local_fls -= 4;
            }
            if ((local_bits & 0xc000) == 0)
            {
                local_bits <<= 2;
                local_fls -= 2;
            }
            if ((local_bits & 0x8000) == 0)
            {
                local_bits <<= 1;
                local_fls -= 1;
            }
            return sizeof(in_bits) * 8 - local_fls;
        }

        /// @copydoc count_leading_0bits_by_logical()
        inline std::size_t count_leading_0bits_by_logical(
            std::uint32_t const in_bits)
        {
            unsigned local_bits(in_bits);
            unsigned local_fls(sizeof(in_bits) * 8);
            if (local_bits == 0)
            {
                --local_fls;
            }
            if ((local_bits & 0xffff0000) == 0)
            {
                local_bits <<= 16;
                local_fls -= 16;
            }
            if ((local_bits & 0xff000000) == 0)
            {
                local_bits <<= 8;
                local_fls -= 8;
            }
            if ((local_bits & 0xf0000000) == 0)
            {
                local_bits <<= 4;
                local_fls -= 4;
            }
            if ((local_bits & 0xc0000000) == 0)
            {
                local_bits <<= 2;
                local_fls -= 2;
            }
            if ((local_bits & 0x80000000) == 0)
            {
                local_bits <<= 1;
                local_fls -= 1;
            }
            return sizeof(in_bits) * 8 - local_fls;
        }

        /// @copydoc count_leading_0bits_by_logical()
        inline std::size_t count_leading_0bits_by_logical(
            std::uint64_t in_bits)
        {
            auto local_bits(in_bits);
            unsigned local_fls(sizeof(in_bits) * 8);
            if (local_bits == 0)
            {
                --local_fls;
            }
            if ((local_bits & 0xffffffff00000000) == 0)
            {
                local_bits <<= 32;
                local_fls -= 32;
            }
            if ((local_bits & 0xffff000000000000) == 0)
            {
                local_bits <<= 16;
                local_fls -= 16;
            }
            if ((local_bits & 0xff00000000000000) == 0)
            {
                local_bits <<= 8;
                local_fls -= 8;
            }
            if ((local_bits & 0xf000000000000000) == 0)
            {
                local_bits <<= 4;
                local_fls -= 4;
            }
            if ((local_bits & 0xc000000000000000) == 0)
            {
                local_bits <<= 2;
                local_fls -= 2;
            }
            if ((local_bits & 0x8000000000000000) == 0)
            {
                local_bits <<= 1;
                local_fls -= 1;
            }
            return sizeof(in_bits) * 8 - local_fls;
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
                sizeof(in_bits) * 8 < FLT_MANT_DIG,
                "Bit size of 'in_bits' must be less than FLT_MANT_DIG.");
            return sizeof(in_bits) * 8 + (1 - FLT_MIN_EXP) - (
                psyq::internal::get_float_bit_value(in_bits + 0.5f)
                >> (FLT_MANT_DIG - 1));
        }

        /// @copydoc count_leading_0bits_by_float()
        template<>
        std::size_t count_leading_0bits_by_float(std::uint32_t const in_bits)
        {
            static_assert(
                // 浮動小数点の基数は、2であること。
                FLT_RADIX == 2, "Floating point radix must be 2.");
            static_assert(
                // in_bits のビット数は、DBL_MANT_DIG未満であること。
                sizeof(in_bits) * 8 < DBL_MANT_DIG,
                "Bit size of 'in_bits' must be less than DBL_MANT_DIG.");
            return sizeof(in_bits) * 8 + (1 - DBL_MIN_EXP) - (
                psyq::internal::get_float_bit_value(in_bits + 0.5)
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
                BIT_SIZE = sizeof(in_bits) * 8,
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
            return psyq::internal::count_leading_0bits_by_float(in_bits);
#else
            return psyq::internal::count_leading_0bits_by_logical(in_bits);
#endif
        }

        /** @brief 64ビット無符号整数の最上位ビットから、0が連続する数を数える。
            @param[in] in_bits ビットを数える整数の値。
            @return 最上位ビットから0が連続する数。
         */
        template<>
        std::size_t count_leading_0bits_of_uint(std::uint64_t const in_bits)
        {
#if PSYQ_BIT_ALGORITHM_INTRINSIC_SIZE < 64
#   if defined(PSYQ_COUNT_LEADING_0BITS_BY_FLOAT)
            if ((in_bits >> (DBL_MANT_DIG - 1)) == 0)
            {
                // 浮動小数点を利用し、最上位ビットから0が連続する数を数える。
                return sizeof(in_bits) * 8 + (1 - DBL_MIN_EXP) - (
                    psyq::internal::get_float_bit_value(in_bits + 0.5)
                    >> (DBL_MANT_DIG - 1));
            }
            else
            {
                static_assert(
                    // DBL_MANT_DIGは、48より大きいこと。
                    48 < DBL_MANT_DIG,
                    "DBL_MANT_DIG must be greater than 48.");
                return psyq::internal::count_leading_0bits_by_float(
                    static_cast<std::uint16_t>(in_bits >> 48));
            }
#   elif 1
            // 上位32ビットと下位32ビットに分ける。
            auto const local_high_bits(
                static_cast<std::uint32_t>(in_bits >> 32));
            if (local_high_bits != 0)
            {
                return psyq::internal::count_leading_0bits_by_logical(
                    local_high_bits);
            }
            return 32 + psyq::internal::count_leading_0bits_by_logical(
                static_cast<std::uint32_t>(in_bits));
#   else
            // 64ビットのまま処理する。
            return psyq::internal::count_leading_0bits_by_logical(in_bits);
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
#elif defined(PSYQ_BIT_ALGORITHM_FOR_GNUC) && ULONG_MAX == 0xffffffffffffffff
            return in_bits != 0? __builtin_clzl(in_bits): 64;
#elif defined(PSYQ_BIT_ALGORITHM_FOR_GNUC) && ULLONG_MAX == 0xffffffffffffffff
            return in_bits != 0? __builtin_clzll(in_bits): 64;
#else
            return psyq::internal::count_leading_0bits_by_logical(in_bits);
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
            return psyq::internal::count_1bits_by_logical(
                static_cast<template_bits>(((~in_bits + 1) & in_bits) - 1));
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
                return sizeof(in_bits) * 8;
            }
            unsigned long local_index;
            BitScanForward(&local_index, in_bits);
            return local_index;
#elif defined(PSYQ_BIT_ALGORITHM_FOR_GNUC)
            return in_bits != 0? __builtin_ctz(in_bits): sizeof(in_bits) * 8;
#else
            return psyq::internal::count_trailing_0bits_by_logical(in_bits);
#endif
        }

        /** @brief 64ビット無符号整数の最下位ビットから、0が連続する数を数える。
            @param[in] in_bits ビットを数える整数の値。
            @return 最下位ビットから、0が連続する数。
         */
        template<>
        std::size_t count_trailing_0bits_of_uint(std::uint64_t const in_bits)
        {
#if PSYQ_BIT_ALGORITHM_INTRINSIC_SIZE < 64
            // 上位32ビットと下位32ビットに分ける。
            auto const local_low_count(
                psyq::internal::count_trailing_0bits_of_uint(
                    static_cast<std::uint32_t>(in_bits)));
            if (local_low_count < 32)
            {
                return local_low_count;
            }
            return 32 + psyq::internal::count_trailing_0bits_of_uint(
                static_cast<std::uint32_t>(in_bits >> 32));
#elif defined(PSYQ_BIT_ALGORITHM_FOR_MSC) && define(BitScanForward64)
            if (in_bits == 0)
            {
                return 64;
            }
            unsigned long local_index;
            BitScanForward64(&local_index, in_bits);
            return local_index;
#elif defined(PSYQ_BIT_ALGORITHM_FOR_GNUC) && ULONG_MAX == 0xffffffffffffffff
            return in_bits != 0? __builtin_ctzl(in_bits): 64;
#elif defined(PSYQ_BIT_ALGORITHM_FOR_GNUC) && ULLONG_MAX == 0xffffffffffffffff
            return in_bits != 0? __builtin_ctzll(in_bits): 64;
#else
            return psyq::internal::count_trailing_0bits_by_logical(in_bits);
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
        typedef typename psyq::internal::make_std_uint<template_bits>::type
            std_uint;
        static_assert(
            // template_bits は、64ビット以下の整数型であること。
            std::is_unsigned<std_uint>::value,
            "'template_bits' must be integer type of 64bits or less.");
        return psyq::internal::count_1bits_of_uint(
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
        typedef typename psyq::internal::make_std_uint<template_bits>::type
            std_uint;
        static_assert(
            // template_bits は、64ビット以下の整数型であること。
            std::is_unsigned<std_uint>::value,
            "'template_bits' must be integer type of 64bits or less.");
        return psyq::internal::count_leading_0bits_of_uint(
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
        typedef typename psyq::internal::make_std_uint<template_bits>::type
            std_uint;
        static_assert(
            // template_bits は、64ビット以下の整数型であること。
            std::is_unsigned<std_uint>::value,
            "'template_bits' must be integer type of 64bits or less.");
        return psyq::internal::count_trailing_0bits_of_uint(
            static_cast<std_uint>(in_bits));
    }

    //-------------------------------------------------------------------------
    /// 単体テストに使う。この名前空間をuserが直接accessするのは禁止。
    namespace test
    {
        template<typename template_value> void count_1bits()
        {
            template_value local_bits(0);
            PSYQ_ASSERT(psyq::count_1bits(local_bits) == 0);
            for (unsigned i(0); i < sizeof(template_value) * 8; ++i)
            {
                local_bits = (local_bits << 1) | 1;
                PSYQ_ASSERT(i + 1 == psyq::count_1bits(local_bits));
            }
        }

        inline void count_1bits()
        {
            psyq::test::count_1bits<char>();
            psyq::test::count_1bits<short>();
            psyq::test::count_1bits<int>();
            psyq::test::count_1bits<long>();
#if ULLONG_MAX <= 0xffffffffffffffff
            psyq::test::count_1bits<long long>();
#endif
        }

        template<typename template_value> void count_leading_0bits()
        {
            PSYQ_ASSERT(
                psyq::count_leading_0bits(template_value(0))
                == sizeof(template_value) * 8);
            for (unsigned i(0); i < sizeof(template_value) * 8; ++i)
            {
                auto const local_clz(
                    psyq::count_leading_0bits(
                        template_value(template_value(1) << i)));
                PSYQ_ASSERT(local_clz + i == sizeof(template_value) * 8 - 1);
            }
        }

        inline void count_leading_0bits()
        {
            psyq::test::count_leading_0bits<char>();
            psyq::test::count_leading_0bits<short>();
            psyq::test::count_leading_0bits<int>();
            psyq::test::count_leading_0bits<long>();
#if ULLONG_MAX <= 0xffffffffffffffff
            psyq::test::count_leading_0bits<long long>();
#endif
        }

        template<typename template_value> void count_trailing_0bits()
        {
            PSYQ_ASSERT(
                psyq::count_trailing_0bits(template_value(0))
                == sizeof(template_value) * 8);
            for (unsigned i(0); i < sizeof(template_value) * 8; ++i)
            {
                auto const local_ctz(
                    psyq::count_trailing_0bits(
                        template_value(template_value(1) << i)));
                PSYQ_ASSERT(local_ctz == i);
            }
        }

        inline void count_trailing_0bits()
        {
            psyq::test::count_trailing_0bits<char>();
            psyq::test::count_trailing_0bits<short>();
            psyq::test::count_trailing_0bits<int>();
            psyq::test::count_trailing_0bits<long>();
#if ULLONG_MAX <= 0xffffffffffffffff
            psyq::test::count_trailing_0bits<long long>();
#endif
        }
    }
}

#endif // PSYQ_BIT_ALGORITHM_HPP_
