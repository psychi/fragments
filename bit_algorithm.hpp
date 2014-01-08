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
#   define PSYQ_BIT_ALGORITHM_FOR_GNUC
#elif defined(_MSC_VER)
#   if defined(_M_PPC)
#      define PSYQ_BIT_ALGORITHM_FOR_VC_PPC
#      include <ppcintrinsics.h>
#   elif defined(_M_ARM)
#      define PSYQ_BIT_ALGORITHM_FOR_VC_ARM
#   else
#      define PSYQ_BIT_ALGORITHM_FOR_VC
#   endif
#elif defined(__ARMCC_VERSION)
#   define PSYQ_BIT_ALGORITHM_FOR_ARMCC
#elif defined(__ghs__)
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
        @return 指定された位置のビット値。
     */
    template<typename template_bits>
    bool get_bit(
        template_bits const in_bits,
        std::size_t   const in_position)
    {
        return (in_bits >> in_position) & template_bits(1);
    }

    /** @brief 指定された位置にビット値として1を設定する。
        @param[in] in_bits     ビット集合として扱う整数値。
        @param[in] in_position 設定するビットの位置。
        @return 指定された位置にビット値を設定したビット集合。
     */
    template<typename template_bits>
    template_bits set_bit(
        template_bits const in_bits,
        std::size_t   const in_position)
    {
        return (template_bits(1) << in_position) | in_bits;
    }

    /** @brief 指定された位置にビット値を設定する。
        @param[in] in_bits     ビット集合として扱う整数値。
        @param[in] in_position 設定するビットの位置。
        @param[in] in_value    設定するビット値。
        @return 指定された位置にビット値を設定したビット集合。
     */
    template<typename template_bits>
    template_bits set_bit(
        template_bits const in_bits,
        std::size_t   const in_position,
        bool          const in_value)
    {
        return psyq::reset_bit(in_bits, in_position)
            | (in_value << in_position);
    }

    /** @brief 指定された位置にビット値として0を設定する。
        @param[in] in_bits     ビット集合として扱う整数値。
        @param[in] in_position 設定するビットの位置。
        @return 指定された位置にビット値を設定したビット集合。
     */
    template<typename template_bits>
    template_bits reset_bit(
        template_bits const in_bits,
        std::size_t   const in_position)
    {
        return ~(template_bits(1) << in_position) & in_bits;
    }

    /** @brief 指定された位置のビット値を反転する。
        @param[in] in_bits     ビット集合として扱う整数値。
        @param[in] in_position 反転するビットの位置。
        @return 指定された位置のビット値を反転したビット集合。
     */
    template<typename template_bits>
    template_bits flip_bit(
        template_bits const in_bits,
        std::size_t   const in_position)
    {
        return (template_bits(1) << in_position) ^ in_bits;
    }

    //-------------------------------------------------------------------------
    /** @brief 浮動小数点実数をビット集合に変換する。
        @param[in] in_float 変換する浮動小数点実数。
        @return 浮動小数点実数のビット集合。
     */
    inline uint32_t get_float_bits(float const in_float)
    {
        return *reinterpret_cast<std::uint32_t const*>(&in_float);
    }

    /// @copydoc get_float_bits()
    inline std::uint64_t get_float_bits(double const in_float)
    {
        return *reinterpret_cast<std::uint64_t const*>(&in_float);
    }

    //-------------------------------------------------------------------------
    /** @brief 値が0以外のビット値の数を数える。

        以下のウェブページを参考にした。
        http://www.nminoru.jp/~nminoru/programming/bitcount.html
        @param[in] in_bits  数えるビット集合。
        @return 値が0以外のビット値の数。
     */
    template<typename template_bits>
    std::size_t count_1bits(template_bits const in_bits)
    {
        return psyq::count_1bits(
            static_cast<typename std::make_unsigned<template_bits>::type>(
                in_bits));
    }

    /// @copydoc count_1bits()
    template<> std::size_t count_1bits(std::uint8_t const in_bits)
    {
        unsigned local_bits(in_bits);
        local_bits = (local_bits & 0x55) + ((local_bits >> 1) & 0x55);
        local_bits = (local_bits & 0x33) + ((local_bits >> 2) & 0x33);
        local_bits = (local_bits & 0x0f) + ((local_bits >> 4) & 0x0f);
        return local_bits;
    }

    /// @copydoc count_1bits()
    template<> std::size_t count_1bits(std::uint16_t const in_bits)
    {
        unsigned local_bits(in_bits);
        local_bits = (local_bits & 0x5555) + ((local_bits >> 1) & 0x5555);
        local_bits = (local_bits & 0x3333) + ((local_bits >> 2) & 0x3333);
        local_bits = (local_bits & 0x0f0f) + ((local_bits >> 4) & 0x0f0f);
        local_bits = (local_bits & 0x00ff) + ((local_bits >> 8) & 0x00ff);
        return local_bits;
    }

    /// @copydoc count_1bits()
    template<> std::size_t count_1bits(std::uint32_t const in_bits)
    {
        unsigned local_bits(in_bits);
        local_bits = (local_bits & 0x55555555) + ((local_bits >> 1) & 0x55555555);
        local_bits = (local_bits & 0x33333333) + ((local_bits >> 2) & 0x33333333);
        local_bits = (local_bits & 0x0f0f0f0f) + ((local_bits >> 4) & 0x0f0f0f0f);
        local_bits = (local_bits & 0x00ff00ff) + ((local_bits >> 8) & 0x00ff00ff);
        local_bits = (local_bits & 0x0000ffff) + ((local_bits >>16) & 0x0000ffff);
        return local_bits;
    }

    /// @copydoc count_1bits()
    template<> std::size_t count_1bits(std::uint64_t const in_bits)
    {
        auto local_bits(in_bits);
        local_bits = (local_bits & 0x5555555555555555ULL) + ((local_bits >> 1) & 0x5555555555555555ULL);
        local_bits = (local_bits & 0x3333333333333333ULL) + ((local_bits >> 2) & 0x3333333333333333ULL);
        local_bits = (local_bits & 0x0f0f0f0f0f0f0f0fULL) + ((local_bits >> 4) & 0x0f0f0f0f0f0f0f0fULL);
        local_bits = (local_bits & 0x00ff00ff00ff00ffULL) + ((local_bits >> 8) & 0x00ff00ff00ff00ffULL);
        local_bits = (local_bits & 0x0000ffff0000ffffULL) + ((local_bits >>16) & 0x0000ffff0000ffffULL);
        local_bits = (local_bits & 0x00000000ffffffffULL) + ((local_bits >>32) & 0x00000000ffffffffULL);
        return static_cast<std::size_t>(local_bits);
    }

    //-------------------------------------------------------------------------
    /** @brief 整数値の最下位ビットから、0が連続する数を数える。

        以下のウェブページを参考にした。
        http://www.nminoru.jp/~nminoru/programming/bitcount.html

        @param[in] in_bits 数える整数値。
        @return 最下位ビットから0が連続する数。
     */
    template<typename template_bits>
    std::size_t count_trailing_0bits(template_bits const in_bits)
    {
        return psyq::count_1bits((in_bits & (-in_bits)) - 1);
    }

    //-------------------------------------------------------------------------
    namespace internal
    {
        /** @brief 整数値の最上位ビットから、0が連続する数を数える。

            以下のウェブページを参考にした。
            http://www.nminoru.jp/~nminoru/programming/bitcount.html

            @param[in] in_bits 数える整数値。
            @return 最上位ビットから0が連続する数。
         */
        inline std::size_t count_leading_0bits(std::uint8_t const in_bits)
        {
            unsigned local_bits(in_bits);
            local_bits = local_bits | (local_bits >> 1);
            local_bits = local_bits | (local_bits >> 2);
            local_bits = local_bits | (local_bits >> 4);
            return psyq::count_1bits(static_cast<std::uint8_t>(~local_bits));
        }

        /// @copydoc count_leading_0bits()
        inline std::size_t count_leading_0bits(std::uint16_t const in_bits)
        {
            unsigned local_bits(in_bits);
            local_bits = local_bits | (local_bits >> 1);
            local_bits = local_bits | (local_bits >> 2);
            local_bits = local_bits | (local_bits >> 4);
            local_bits = local_bits | (local_bits >> 8);
            return psyq::count_1bits(static_cast<std::uint16_t>(~local_bits));
        }

        /// @copydoc count_leading_0bits()
        inline std::size_t count_leading_0bits(std::uint32_t const in_bits)
        {
            unsigned local_bits(in_bits);
            local_bits = local_bits | (local_bits >> 1);
            local_bits = local_bits | (local_bits >> 2);
            local_bits = local_bits | (local_bits >> 4);
            local_bits = local_bits | (local_bits >> 8);
            local_bits = local_bits | (local_bits >>16);
            return psyq::count_1bits(static_cast<std::uint32_t>(~local_bits));
        }

        /// @copydoc count_leading_0bits()
        inline std::size_t count_leading_0bits(std::uint64_t in_bits)
        {
            in_bits = in_bits | (in_bits >> 1);
            in_bits = in_bits | (in_bits >> 2);
            in_bits = in_bits | (in_bits >> 4);
            in_bits = in_bits | (in_bits >> 8);
            in_bits = in_bits | (in_bits >>16);
            in_bits = in_bits | (in_bits >>32);
            return psyq::count_1bits(~in_bits);
        }

#if defined(PSYQ_COUNT_LEADING_0BITS_BY_FLOAT)
        /** @brief 浮動小数点のビットフィールドを用いて、
                   整数値の最上位ビットから、0が連続する数を数える。

            以下のウェブページを参考にした。
            http://www.nminoru.jp/~nminoru/programming/bitcount.html

            @param[in] in_bits 数える整数値。
            @return 最上位ビットから0が連続する数。
         */
        template<typename template_bits>
        std::size_t count_leading_0bits_by_float(template_bits const in_bits)
        {
            static_assert(std::is_unsigned<template_bits>::value, "");
            static_assert(
                FLT_RADIX == 2 && sizeof(in_bits) * 8 < FLT_MANT_DIG, "");
            return sizeof(in_bits) * 8 + (1 - FLT_MIN_EXP) - (
                psyq::get_float_bits(in_bits + 0.5f) >> (FLT_MANT_DIG - 1));
        }

        /// @copydoc count_leading_0bits_by_float()
        template<>
        std::size_t count_leading_0bits_by_float(std::uint32_t const in_bits)
        {
            static_assert(
                FLT_RADIX == 2 && sizeof(in_bits) * 8 < DBL_MANT_DIG, "");
            return sizeof(in_bits) * 8 + (1 - DBL_MIN_EXP) - (
                psyq::get_float_bits(in_bits + 0.5) >> (DBL_MANT_DIG - 1));
        }
#endif

        /** @brief 無符号整数値の最上位ビットから、0が連続する数を数える。
            @param[in] in_bits 数える整数値。
            @return 最上位ビットから0が連続する数。
         */
        template<typename template_bits>
        std::size_t count_leading_0bits_in_unsigned(template_bits const in_bits)
        {
            static_assert(sizeof(in_bits) <= sizeof(std::uint32_t), "");
            static_assert(std::is_unsigned<template_bits>::value, "");
            enum: unsigned
            {
                BIT_WIDTH = sizeof(in_bits) * 8,
                WIDTH_DIFF = 32 - BIT_WIDTH,
            };
#if defined(PSYQ_BIT_ALGORITHM_FOR_VC)
            if (in_bits == 0)
            {
                return BIT_WIDTH;
            }
            unsigned long local_index;
            BitScanReverse(&local_index, in_bits);
            return BIT_WIDTH - 1 - local_index;
#elif defined(PSYQ_BIT_ALGORITHM_FOR_VC_PPC)
            return CountLeadingZeros(in_bits) - WIDTH_DIFF;
#elif defined(PSYQ_BIT_ALGORITHM_FOR_GNUC)
            return in_bits != 0? __builtin_clz(in_bits) - WIDTH_DIFF: BIT_WIDTH;
#elif defined(PSYQ_BIT_ALGORITHM_FOR_ARMCC)
            return in_bits != 0? __clz(in_bits) - WIDTH_DIFF: BIT_WIDTH;
#elif defined(PSYQ_BIT_ALGORITHM_FOR_GHS)
            return in_bits != 0? __CLZ(in_bits) - WIDTH_DIFF: BIT_WIDTH;
#elif defined(PSYQ_COUNT_LEADING_0BITS_BY_FLOAT)
            return psyq::internal::count_leading_0bits_by_float(in_bits);
#else
            return psyq::internal::count_leading_0bits(in_bits);
#endif
        }

        /// @copydoc count_leading_0bits_in_unsigned()
        template<>
        std::size_t count_leading_0bits_in_unsigned(std::uint64_t const in_bits)
        {
#if PSYQ_BIT_ALGORITHM_INTRINSIC_SIZE < 64
#   if defined(PSYQ_COUNT_LEADING_0BITS_BY_FLOAT)
            if ((in_bits >> (DBL_MANT_DIG - 1)) == 0)
            {
                /*  浮動小数点を利用し、最上位ビットから0が連続する数を数える。
                    以下のウェブページを参考にした。
                    http://www.nminoru.jp/~nminoru/programming/bitcount.html
                 */
                return sizeof(in_bits) * 8 + (1 - DBL_MIN_EXP) - (
                    psyq::get_float_bits(in_bits + 0.5) >> (DBL_MANT_DIG - 1));
            }
            else
            {
                static_assert(48 < DBL_MANT_DIG, "");
                return psyq::internal::count_leading_0bits_by_float(
                    static_cast<std::uint16_t>(in_bits >> 48));
            }
#   else
            auto const local_high_bits(static_cast<std::uint32_t>(in_bits >> 32));
            if (local_high_bits == 0)
            {
                return 32 + psyq::internal::count_leading_0bits(
                    static_cast<std::uint32_t>(in_bits));
            }
            else
            {
                return psyq::internal::count_leading_0bits(local_high_bits);
            }
#   endif
#elif defined(PSYQ_BIT_ALGORITHM_FOR_VC)
            if (in_bits == 0)
            {
                return 64;
            }
            unsigned long local_index;
            BitScanReverse64(&local_index, in_bits);
            return 63 - local_index;
#else
            return internal::count_leading_0bits(in_bits);
#endif
        }
    }

    //-------------------------------------------------------------------------
    /** @brief 整数値の最上位ビットから、0が連続する数を数える。

        @param[in] in_bits 数える整数値。
        @return 最上位ビットから0が連続する数。
     */
    template<typename template_bits>
    std::size_t count_leading_0bits(template_bits const in_bits)
    {
        return psyq::internal::count_leading_0bits_in_unsigned(
            static_cast<typename std::make_unsigned<template_bits>::type>(
                in_bits));
    }

} // namespace psyq

#endif // PSYQ_BIT_ALGORITHM_HPP_
