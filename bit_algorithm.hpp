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

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
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

	/** @brief 浮動小数点実数をビット集合に変換する。
	    @param[in] in_float 変換する浮動小数点実数。
	    @return 浮動小数点実数のビット集合。
	 */
	inline uint32_t get_float_bits(
		float const in_float)
	{
		return *reinterpret_cast<std::uint32_t const*>(&in_float);
	}

	/// @copydoc get_float_bits()
	inline std::uint64_t get_float_bits(
		double const in_float)
	{
		return *reinterpret_cast<std::uint64_t const*>(&in_float);
	}

	/** @brief 値が0以外のビット値の数を数える。

	    以下のウェブページを参考にした。
	    http://www.nminoru.jp/~nminoru/programming/bitcount.html
	    @param[in] in_bits  数えるビット集合。
	    @return 値が0以外のビット値の数。
	 */
	template<typename template_bits>
	std::size_t count_bit1(
		template_bits const in_bits)
	{
		std::uint32_t local_bits(in_bits);
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

	/// @copydoc count_bit1()
	template<>
	std::size_t count_bit1(
		std::uint64_t const in_bits)
	{
		auto local_bits(in_bits);
		local_bits = (local_bits & 0x5555555555555555LL)
			+ ((local_bits >> 1) & 0x5555555555555555LL);
		local_bits = (local_bits & 0x3333333333333333LL)
			+ ((local_bits >> 2) & 0x3333333333333333LL);
		local_bits = (local_bits & 0x0f0f0f0f0f0f0f0fLL)
			+ ((local_bits >> 4) & 0x0f0f0f0f0f0f0f0fLL);
		local_bits = (local_bits & 0x00ff00ff00ff00ffLL)
			+ ((local_bits >> 8) & 0x00ff00ff00ff00ffLL);
		local_bits = (local_bits & 0x0000ffff0000ffffLL)
			+ ((local_bits >>16) & 0x0000ffff0000ffffLL);
		local_bits = (local_bits & 0x00000000ffffffffLL)
			+ ((local_bits >>32) & 0x00000000ffffffffLL);
		return local_bits;
	}

	/// @copydoc count_bit1()
	template<>
	std::size_t count_bit1(
		std::int64_t const in_bits)
	{
		return psyq::count_bit1(static_cast<std::uint64_t>(in_bits));
	}

	/** @brief 最下位ビットから0が連続する数を数える。

	    以下のウェブページを参考にした。
	    http://www.nminoru.jp/~nminoru/programming/bitcount.html
	    @param[in] in_bits  数えるビット集合。
	    @return 最下位ビットから0が連続する数。
	 */
	template<typename template_bits>
	std::size_t count_training_bit0(
		template_bits const in_bits)
	{
		return psyq::count_bit1((in_bits & (-in_bits)) - 1);
	}

	/** @brief 23ビット整数で、最上位ビットから0が連続する数を数える。

	    以下のウェブページを参考にした。
	    http://www.nminoru.jp/~nminoru/programming/bitcount.html
	    @param[in] in_bits  数えるビット集合。
	    @return 最上位ビットから0が連続する数。
	 */
	template<typename template_bits>
	std::size_t count_leading_bit0_23bits(
		template_bits const in_bits)
	{
		float const local_float(in_bits + 0.5f);
		return 149 - (psyq::get_float_bits(local_float) >> 23);
	}

	/** @brief 32ビット整数で、最上位ビットから0が連続する数を数える。

	    以下のウェブページを参考にした。
	    http://www.nminoru.jp/~nminoru/programming/bitcount.html
	    @param[in] in_bits  数えるビット集合。
	    @return 最上位ビットから0が連続する数。
	 */
	inline std::size_t count_leading_bit0_32bits(
		std::uint32_t const in_bits)
	{
		auto local_bits(in_bits);
		local_bits = local_bits | (local_bits >>  1);
		local_bits = local_bits | (local_bits >>  2);
		local_bits = local_bits | (local_bits >>  4);
		local_bits = local_bits | (local_bits >>  8);
		local_bits = local_bits | (local_bits >> 16);
		return psyq::count_bit1(~local_bits);
	}

	/** @brief 52ビット整数で、最上位ビットから0が連続する数を数える。

	    以下のウェブページを参考にした。
	    http://www.nminoru.jp/~nminoru/programming/bitcount.html
	    @param[in] in_bits  数えるビット集合。
	    @return 最上位ビットから0が連続する数。
	 */
	template<typename template_bits>
	std::size_t count_leading_bit0_52bits(
		template_bits const in_bits)
	{
		double const local_float(in_bits + 0.5);
		return 1074 - (psyq::get_float_bits(local_float) >> 52);
	}

	/** @brief 64ビット整数で、最上位ビットから0が連続する数を数える。

	    以下のウェブページを参考にした。
	    http://www.nminoru.jp/~nminoru/programming/bitcount.html
	    @param[in] in_bits  数えるビット集合。
	    @return 最上位ビットから0が連続する数。
	 */
	inline std::size_t count_leading_bit0_64bits(
		std::uint64_t const in_bits)
	{
		auto local_bits(in_bits);
		local_bits = local_bits | (local_bits >>  1);
		local_bits = local_bits | (local_bits >>  2);
		local_bits = local_bits | (local_bits >>  4);
		local_bits = local_bits | (local_bits >>  8);
		local_bits = local_bits | (local_bits >> 16);
		local_bits = local_bits | (local_bits >> 32);
		return psyq::count_bit1(~local_bits);
	}

	/** @brief 最上位ビットから0が連続する数を数える。
	    @param[in] in_bits  数えるビット集合。
	    @return 最上位ビットから0が連続する数。
	 */
	template<typename template_bits>
	std::size_t count_leading_bit0(
		template_bits const in_bits)
	{
		return psyq::count_leading_bit0_23bits(in_bits)
			+ sizeof(template_bits) * 8 - 23;
	}

	/// @copydoc count_leading_bit0()
	template<>
	static std::size_t count_leading_bit0(
		std::uint32_t const in_bits)
	{
		return psyq::count_leading_bit0_52bits(in_bits) - 20;
	}

	/// @copydoc count_leading_bit0()
	template<>
	std::size_t count_leading_bit0(
		std::int32_t const in_bits)
	{
		return psyq::count_leading_bit0(static_cast<std::uint32_t>(in_bits));
	}

	/// @copydoc count_leading_bit0_()
	template<>
	std::size_t count_leading_bit0(
		std::uint64_t const in_bits)
	{
		if (in_bits < (std::uint64_t(1) << 52))
		{
			return psyq::count_leading_bit0_52bits(in_bits) + 12;
		}
		else
		{
			return psyq::count_leading_bit0_64bits(in_bits);
		}
	}

	/// @copydoc count_leading_bit0_()
	template<>
	std::size_t count_leading_bit0(
		std::int64_t const in_bits)
	{
		return psyq::count_leading_bit0(static_cast<std::uint64_t>(in_bits));
	}

} // namespace psyq

#endif // PSYQ_BIT_ALGORITHM_HPP_
