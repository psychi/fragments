// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief ビット操作のための関数群。
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#include <algorithm>
#include <climits>
#include <type_traits>
#include <array>
#include "GenericPlatform/GenericPlatform.h"
#include "./Assert.h"

/// @cond
#if defined(__alpha__) || defined(__ia64__) || defined(__x86_64__) || defined(_WIN64) || defined(__LP64__) || defined(__LLP64__)
#	define PSYQUE_BIT_ALGORITHM_INTRINSIC_SIZE 64
#else
#	define PSYQUE_BIT_ALGORITHM_INTRINSIC_SIZE 32
#endif

#if defined(FLT_RADIX) && FLT_RADIX == 2
#	define PSYQUE_COUNT_LEADING_0BITS_BY_FLOAT
#endif

#if defined(__GNUC__) && (3 < __GNUC__ || (__GNUC__ == 3 && 4 <= __GNUC_MINOR__)) && defined(__GNUC_PATCHLEVEL__)
//	'gcc 3.4' and above have builtin support, specialized for architecture.
//	Some compilers masquerade as gcc; patchlevel test filters them out.
#	define PSYQUE_BIT_ALGORITHM_FOR_GNUC
#elif defined(_MSC_VER)
#	define PSYQUE_BIT_ALGORITHM_FOR_MSC
#	if defined(_M_PPC)
#		include <ppcintrinsics.h>
#	endif
#elif defined(__ARMCC_VERSION)
//	RealView Compilation Tools for ARM.
#	define PSYQUE_BIT_ALGORITHM_FOR_ARMCC
#elif defined(__ghs__)
//	Green Hills support for PowerPC.
#	define PSYQUE_BIT_ALGORITHM_FOR_GHS
#	include <ppc_ghs.h>
#endif
/// @endcond

/// @brief バイト値をビッグエンディアン形式で合成する。
#define PSYQUE_BIG_ENDIAN_4BYTES(define_type, define_0, define_1, define_2, define_3)\
	static_cast<define_type>(\
		static_cast<define_type>(static_cast<unsigned char>(define_0))\
		| (static_cast<define_type>(static_cast<unsigned char>(define_1)) << CHAR_BIT)\
		| (static_cast<define_type>(static_cast<unsigned char>(define_2)) << (CHAR_BIT * 2))\
		| (static_cast<define_type>(static_cast<unsigned char>(define_3)) << (CHAR_BIT * 3)))

/// @brief Unreal Engine 4 で動作する、ビデオゲーム開発のためのライブラリ。
namespace Psyque
{
	template<typename> union TFloatBitset;

	/// @brief Psyque の管理者以外は、直接アクセス禁止。
	namespace _private
	{
		class FTrailing0Bits;
	} // namespace _private
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 浮動小数点数とビット列の変換に使う共用体。
/// @note strict-aliasing ruleに抵触しないために、共用体を使う。
/// http://homepage1.nifty.com/herumi/diary/0911.html#10
template<typename TemplateFloat>
union Psyque::TFloatBitset
{
	private:
	using ThisClass = TFloatBitset;

	public:
	/// @brief 浮動小数点数の型。
	using FFloat = TemplateFloat;
	static_assert(
		std::is_floating_point<TemplateFloat>::value,
		"'TemplateFloat' is not float type.");

	/// @brief 符号なし整数型をビット列として扱う。
	using FBitset = typename std::conditional<
		std::is_same<TemplateFloat, float>::value, uint32, uint64>::type;
	static_assert(
		sizeof(TemplateFloat) == sizeof(FBitset),
		"sizeof(TemplateFloat) is not equal sizeof(FBitset).");

	explicit TFloatBitset(typename ThisClass::FFloat const InFloat)
	PSYQUE_NOEXCEPT
	{
		this->Float = InFloat;
	}

	explicit TFloatBitset(typename ThisClass::FBitset const InBitset)
	PSYQUE_NOEXCEPT
	{
		this->Bitset = InBitset;
	}

	typename ThisClass::FFloat Float;   ///< 浮動小数点数の値。
	typename ThisClass::FBitset Bitset; ///< 浮動小数点数のビット列。

}; // union Psyque::TFloatBitset

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 無符号64ビット整数の最下位ビットから、0が連続する数を数える。
/// @note 以下のウェブページを参考にした。
/// http://d.hatena.ne.jp/siokoshou/20090704#p1
class Psyque::_private::FTrailing0Bits
{
	using ThisClass = FTrailing0Bits;

	enum: uint64 {HASH = 0x03F566ED27179461ul};

	public:
	FTrailing0Bits()
	{
		uint64 LocalHash(ThisClass::HASH);
		for (uint8 i(0); i < this->Counts.size(); ++i)
		{
			this->Counts[LocalHash >> 58] = i;
			LocalHash <<= 1;
		}
	}

	/// @brief 整数の最下位ビットから、0が連続する数を数える。
	/// @return InValue の最下位ビットから、0が連続する数。
	template<typename TemplateValue>
	uint8 Count(
		/// [in] 対象となる整数。
		TemplateValue const InValue)
	const
	{
		return InValue != 0?
			this->Counts[ThisClass::ComputeIndex(InValue)]:
			sizeof(TemplateValue) * CHAR_BIT;
	}

	private:
	static unsigned ComputeIndex(int64 const InValue)
	{
		return static_cast<unsigned>(
			(static_cast<uint64>(InValue & -InValue) * ThisClass::HASH) >> 58);
	}

	private:
	std::array<uint8, 64> Counts;

}; // class Psyque::_private::FTrailing0Bits

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace Psyque
{
	//-------------------------------------------------------------------------
	namespace _private
	{
		template<typename TemplateBits>
		bool PSYQUE_CONSTEXPR IsValidBitShift(SIZE_T const InBitShift)
		PSYQUE_NOEXCEPT
		{
			return InBitShift < sizeof(TemplateBits) * CHAR_BIT;
		}

		template<typename TemplateBits>
		bool PSYQUE_CONSTEXPR IsValidBitWidth(SIZE_T const InBitWidth)
		PSYQUE_NOEXCEPT
		{
			return InBitWidth <= sizeof(TemplateBits) * CHAR_BIT;
		}

		template<typename TemplateBits>
		bool PSYQUE_CONSTEXPR IsValidBitWidth(
			SIZE_T const InBitPosition,
			SIZE_T const InBitWidth)
		PSYQUE_NOEXCEPT
		{
			return IsValidBitWidth<TemplateBits>(InBitPosition)
				&& IsValidBitWidth<TemplateBits>(InBitWidth)
				&& IsValidBitWidth<TemplateBits>(InBitPosition + InBitWidth);
		}
	}

	//-------------------------------------------------------------------------
	/// @brief 符号あり整数の絶対値を取得する。
	/// @details 条件分岐の代わりにビット演算を使い、整数の絶対値を算出する。
	/// @return InValue の絶対値。
	template<typename TemplateInteger>
	TemplateInteger AbsInteger(
		 /// [in] 絶対値を求める符号あり整数。
		TemplateInteger const InValue)
	PSYQUE_NOEXCEPT
	{
		static_assert(
			std::is_integral<TemplateInteger>::value
			&& std::is_signed<TemplateInteger>::value,
			"'TemplateInteger' is not signed integer type.");
		auto const LocalSignBitPosition(
			CHAR_BIT * sizeof(TemplateInteger) - 1);
		auto const LocalMask(
			-static_cast<TemplateInteger>(
				1 & (InValue >> LocalSignBitPosition)));
		return (InValue ^ LocalMask) - LocalMask;
	}

	//-------------------------------------------------------------------------
	/// @name ビットシフト演算
	/// @{

	/// @brief 整数を左ビットシフトする。
	/// @return 左ビットシフトした値。
	template<typename TemplateBits>
	TemplateBits PSYQUE_CONSTEXPR ShiftLeftBitwise(
		/// [in] ビットシフトする値。
		TemplateBits const InBits,
		/// [in] シフトするビット数。
		SIZE_T const InShift)
	PSYQUE_NOEXCEPT
	{
		return static_cast<TemplateBits>(
			Psyque::_private::IsValidBitShift<TemplateBits>(InShift)?
				InBits << InShift: 0);
	}

	/// @brief 整数を左ビットシフトする。
	/// @note ビット数以上のビットシフト演算は、
	/// C言語の仕様として未定義の動作となる。
	/// http://hexadrive.sblo.jp/article/56575654.html
	/// @return 左ビットシフトした値。
	template<typename TemplateBits>
	TemplateBits PSYQUE_CONSTEXPR ShiftLeftBitwiseFast(
		/// [in] ビットシフトする値。
		TemplateBits const InBits,
		/// [in] シフトするビット数。
		SIZE_T const InShift)
	PSYQUE_NOEXCEPT
	{
		return static_cast<TemplateBits>(
			PSYQUE_ASSERT(
				Psyque::_private::IsValidBitShift<TemplateBits>(InShift)),
			InBits << InShift);
	}

	/// @brief 整数を右ビットシフトする。
	/// @return 右ビットシフトした値。
	template<typename TemplateBits>
	TemplateBits PSYQUE_CONSTEXPR ShiftRightBitwise(
		/// [in] ビットシフトする値。
		TemplateBits const InBits,
		/// [in] シフトするビット数。
		SIZE_T const InShift)
	PSYQUE_NOEXCEPT
	{
		return std::is_unsigned<TemplateBits>::value?
			static_cast<TemplateBits>(
				Psyque::_private::IsValidBitShift<TemplateBits>(InShift)?
					InBits >> InShift: 0):
			static_cast<TemplateBits>(
				InBits >> (std::min<SIZE_T>)(
					InShift, sizeof(InBits) * CHAR_BIT - 1));
	}

	/// @brief 整数を右ビットシフトする。
	/// @note bit数以上のbit-shift演算は、C言語の仕様として未定義の動作となる。
	/// http://hexadrive.sblo.jp/article/56575654.html
	/// @return 右ビットシフトした値。
	template<typename TemplateBits>
	TemplateBits PSYQUE_CONSTEXPR ShiftRightBitwiseFast(
		/// [in] ビットシフトする値。
		TemplateBits const InBits,
		/// [in] シフトするビット数。
		SIZE_T const InShift)
	PSYQUE_NOEXCEPT
	{
		return static_cast<TemplateBits>(
			PSYQUE_ASSERT(
				Psyque::_private::IsValidBitShift<TemplateBits>(InShift)),
			InBits >> InShift);
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 1ビット単位の操作
	/// @{

	/// @brief 指定された位置にビット値として0を設定する。
	/// @return 指定されたビット位置に0を設定した整数値。
	/// ただし InPosition がsizeof(int)以上だった場合、
	/// InBits をそのまま返す。
	template<typename TemplateBits>
	TemplateBits PSYQUE_CONSTEXPR ResetBit(
		/// [in] ビット集合として扱う整数値。
		TemplateBits const InBits,
		/// [in] 設定するビットの位置。
		SIZE_T const InPosition)
	PSYQUE_NOEXCEPT
	{
		return ~Psyque::ShiftLeftBitwise(TemplateBits(1), InPosition) & InBits;
	}

	/// @brief 指定された位置にビット値として0を設定する。
	/// @return
	/// 指定されたビット位置に0を設定した整数値。
	/// ただし InPosition が sizeof(int) 以上だった場合、未定義。
	template<typename TemplateBits>
	TemplateBits PSYQUE_CONSTEXPR ResetBitFast(
		/// [in] ビット集合として扱う整数値。
		TemplateBits const InBits,
		/// [in] 設定するビットの位置。
		SIZE_T const InPosition)
	PSYQUE_NOEXCEPT
	{
		return ~Psyque::ShiftLeftBitwiseFast(TemplateBits(1), InPosition)
			& InBits;
	}

	/// @brief 指定された位置にビット値として1を設定する。
	/// @return
	/// 指定されたビット位置に1を設定した整数値。
	/// ただし InPosition が sizeof(int) 以上だった場合、
	/// InBits をそのまま返す。
	template<typename TemplateBits>
	TemplateBits PSYQUE_CONSTEXPR SetBit(
		/// [in] ビット集合として扱う整数値。
		TemplateBits const InBits,
		/// [in] 設定するビットの位置。
		SIZE_T const InPosition)
	PSYQUE_NOEXCEPT
	{
		return Psyque::ShiftLeftBitwise(TemplateBits(1), InPosition) | InBits;
	}

	/// @brief 指定された位置にビット値として1を設定する。
	/// @return
	/// 指定されたビット位置に1を設定した整数値。
	/// ただし InPosition が sizeof(int) 以上だった場合、未定義。
	template<typename TemplateBits>
	TemplateBits PSYQUE_CONSTEXPR SetBitFast(
		/// [in] ビット集合として扱う整数値。
		TemplateBits const InBits,
		/// [in] 設定するビットの位置。
		SIZE_T const InPosition)
	PSYQUE_NOEXCEPT
	{
		return Psyque::ShiftLeftBitwiseFast(TemplateBits(1), InPosition)
			| InBits;
	}

	/// @brief 指定された位置にビット値を設定する。
	/// @return
	/// 指定されたビット位置に InValue を設定した整数値。
	/// ただし InPosition が sizeof(int) 以上だった場合、
	/// InBits をそのまま返す。
	template<typename TemplateBits>
	TemplateBits PSYQUE_CONSTEXPR SetBit(
		/// [in] ビット集合として扱う整数値。
		TemplateBits const InBits,
		/// [in] 設定するビットの位置。
		SIZE_T const InPosition,
		/// [in] 設定するビット値。
		bool const InValue)
	PSYQUE_NOEXCEPT
	{
		return Psyque::ResetBit(InBits, InPosition)
			| Psyque::ShiftLeftBitwise<TemplateBits>(InValue, InPosition);
	}

	/// @brief 指定された位置にビット値を設定する。
	/// @return
	/// 指定されたビット位置に InValue を設定した整数値。
	/// ただし InPosition が sizeof(int) 以上だった場合、未定義。
	template<typename TemplateBits>
	TemplateBits PSYQUE_CONSTEXPR SetBitFast(
		/// [in] ビット集合として扱う整数値。
		TemplateBits const InBits,
		/// [in] 設定するビットの位置。
		SIZE_T const InPosition,
		/// [in] 設定するビット値。
		bool const InValue)
	PSYQUE_NOEXCEPT
	{
		return Psyque::ResetBitFast(InBits, InPosition)
			| Psyque::ShiftLeftBitwiseFast<TemplateBits>(InValue, InPosition);
	}

	/// @brief 指定された位置のビット値を反転する。
	/// @return
	/// 指定されたビット位置の値を反転した整数値。
	/// ただし InPosition がsizeof(int)以上だった場合、
	/// InBits をそのまま返す。
	template<typename TemplateBits>
	TemplateBits PSYQUE_CONSTEXPR FlipBit(
		/// [in] ビット集合として扱う整数値。
		TemplateBits const InBits,
		/// [in] 反転するビットの位置。
		SIZE_T const InPosition)
	PSYQUE_NOEXCEPT
	{
		return Psyque::ShiftLeftBitwise(TemplateBits(1), InPosition) ^ InBits;
	}

	/// @brief 指定された位置のビット値を反転する。
	/// @return
	/// 指定されたビット位置の値を反転した整数値。
	/// ただし InPosition が sizeof(int) 以上だった場合、未定義。
	template<typename TemplateBits>
	TemplateBits PSYQUE_CONSTEXPR FlipBitFast(
		/// [in] ビット集合として扱う整数値。
		TemplateBits const InBits,
		/// [in] 反転するビットの位置。
		SIZE_T const InPosition)
	PSYQUE_NOEXCEPT
	{
		return Psyque::ShiftLeftBitwiseFast(TemplateBits(1), InPosition)
			^ InBits;
	}
	/// @}
	//-------------------------------------------------------------------------
	namespace _private
	{
		/// @brief 組み込み整数型から、同じ大きさのuint*型に変換する。
		/// @tparam TemplateType 元となる型。
		template<typename TemplateType> struct TMakeUint
		{
			/// @brief TemplateType型から変換した、std::uint*_t型。
			/// @details 変換できない場合は、void型となる。
			using Type =
				typename std::conditional<
					!std::is_integral<TemplateType>::value,
					void,
				typename std::conditional<
					sizeof(TemplateType) == sizeof(uint8),
					uint8,
				typename std::conditional<
					sizeof(TemplateType) == sizeof(uint16),
					uint16,
				typename std::conditional<
					sizeof(TemplateType) == sizeof(uint32),
					uint32,
				typename std::conditional<
					sizeof(TemplateType) == sizeof(uint64),
					uint64,
					void
				>::type>::type>::type>::type>::type;
		};

		//---------------------------------------------------------------------
		/// @brief 無符号整数で、1になっているビットを数える。
		/// @details 以下のウェブページを参考にした。
		/// http://www.nminoru.jp/~nminoru/programming/bitcount.html
		/// @return 1になっているビットの数。
		inline SIZE_T Count1BitsByTable(
			/// [in] ビットを数える無符号整数の値。
			uint8 const InBits)
		PSYQUE_NOEXCEPT
		{
			static uint8 const BITS_COUNT_TABLE[256] =
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
			return BITS_COUNT_TABLE[InBits];
		}

		/// @copydoc Count1BitsByTable()
		inline SIZE_T Count1BitsByTable(uint16 const InBits) PSYQUE_NOEXCEPT
		{
			return Count1BitsByTable(static_cast<uint8>(InBits))
				+ Count1BitsByTable(static_cast<uint8>(InBits >> CHAR_BIT));
		}

		/// @copydoc Count1BitsByTable()
		inline SIZE_T Count1BitsByTable(uint32 const InBits) PSYQUE_NOEXCEPT
		{
			return Count1BitsByTable(static_cast<uint16>(InBits))
				+ Count1BitsByTable(static_cast<uint16>(InBits >> 16));
		}

		/// @copydoc Count1BitsByTable()
		inline SIZE_T Count1BitsByTable(uint64 const InBits) PSYQUE_NOEXCEPT
		{
			return Count1BitsByTable(static_cast<uint32>(InBits))
				+ Count1BitsByTable(static_cast<uint32>(InBits >> 32));
		}

		//---------------------------------------------------------------------
		/// @brief 無符号整数で、1になっているビットを数える。
		/// @details 以下のウェブページを参考にした。
		/// http://www.nminoru.jp/~nminoru/programming/bitcount.html
		/// @return 1になっているビットの数。
		inline SIZE_T Count1BitsByLogical(
			/// [in] ビットを数える無符号整数の値。
			uint8 const InBits)
		PSYQUE_NOEXCEPT
		{
			unsigned LocalBits(InBits);
			LocalBits = (LocalBits & 0x55) + ((LocalBits >> 1) & 0x55);
			LocalBits = (LocalBits & 0x33) + ((LocalBits >> 2) & 0x33);
			LocalBits = (LocalBits & 0x0F) + ((LocalBits >> 4) & 0x0F);
			return LocalBits;
		}

		/// @copydoc Count1BitsByLogical()
		inline SIZE_T Count1BitsByLogical(uint16 const InBits) PSYQUE_NOEXCEPT
		{
			unsigned LocalBits(InBits);
			LocalBits = (LocalBits & 0x5555) + ((LocalBits >> 1) & 0x5555);
			LocalBits = (LocalBits & 0x3333) + ((LocalBits >> 2) & 0x3333);
			LocalBits = (LocalBits & 0x0F0F) + ((LocalBits >> 4) & 0x0F0F);
			LocalBits = (LocalBits & 0x00FF) + ((LocalBits >> 8) & 0x00FF);
			return LocalBits;
		}

		/// @copydoc Count1BitsByLogical()
		inline SIZE_T Count1BitsByLogical(uint32 const InBits) PSYQUE_NOEXCEPT
		{
			unsigned LocalBits(InBits);
			LocalBits = (LocalBits & 0x55555555)
				+ ((LocalBits >> 1) & 0x55555555);
			LocalBits = (LocalBits & 0x33333333)
				+ ((LocalBits >> 2) & 0x33333333);
			LocalBits = (LocalBits & 0x0F0F0F0F)
				+ ((LocalBits >> 4) & 0x0F0F0F0F);
			LocalBits = (LocalBits & 0x00FF00FF)
				+ ((LocalBits >> 8) & 0x00FF00FF);
			LocalBits = (LocalBits & 0x0000FFFF)
				+ ((LocalBits >>16) & 0x0000FFFF);
			return LocalBits;
		}

		/// @copydoc Count1BitsByLogical()
		inline SIZE_T Count1BitsByLogical(uint64 const InBits) PSYQUE_NOEXCEPT
		{
			auto LocalBits(InBits);
			LocalBits = (LocalBits & 0x5555555555555555)
				+ ((LocalBits >> 1) & 0x5555555555555555);
			LocalBits = (LocalBits & 0x3333333333333333)
				+ ((LocalBits >> 2) & 0x3333333333333333);
			LocalBits = (LocalBits & 0x0F0F0F0F0F0F0F0F)
				+ ((LocalBits >> 4) & 0x0F0F0F0F0F0F0F0F);
			LocalBits = (LocalBits & 0x00FF00FF00FF00FF)
				+ ((LocalBits >> 8) & 0x00FF00FF00FF00FF);
			LocalBits = (LocalBits & 0x0000FFFF0000FFFF)
				+ ((LocalBits >>16) & 0x0000FFFF0000FFFF);
			LocalBits = (LocalBits & 0x00000000FFFFFFFF)
				+ ((LocalBits >>32) & 0x00000000FFFFFFFF);
			return static_cast<SIZE_T>(LocalBits);
		}

		//---------------------------------------------------------------------
		/// @brief 無符号整数で、1になっているビットを数える。
		/// @return 1になってるビットの数。
		template<typename TemplateBits>
		SIZE_T Count1BitsOfUint(
			/// [in] ビットを数える無符号整数の値。
			TemplateBits const InBits)
		PSYQUE_NOEXCEPT
		{
			static_assert(
				// InBitsのビット数は、32以下であること。
				sizeof(InBits) <= sizeof(uint32),
				"Bit size of 'InBits' must be less than or equal to 32.");
			static_assert(
				// InBits は、無符号整数型であること。
				std::is_unsigned<TemplateBits>::value,
				"'InBits' must be unsigned integer type.");
#if defined(PSYQUE_BIT_ALGORITHM_FOR_MSC)
			return __popcnt(InBits);
#elif defined(PSYQUE_BIT_ALGORITHM_FOR_GNUC)
			return __builtin_popcount(InBits);
#else
			return Psyque::_private::Count1BitsByTable(InBits);
#endif
		}

		/// @brief 64ビット無符号整数で、1になっているビットを数える。
		/// @return 1になってるビットの数。
		template<> inline SIZE_T Count1BitsOfUint(
			/// [in] ビットを数える無符号整数の値。
			uint64 const InBits)
		PSYQUE_NOEXCEPT
		{
#if PSYQUE_BIT_ALGORITHM_INTRINSIC_SIZE < 64
			// 上位32ビットと下位32ビットに分ける。
			auto const LocalHighCount(
				Psyque::_private::Count1BitsOfUint(
					static_cast<uint32>(InBits >> 32)));
			auto const LocalLowCount(
				Psyque::_private::Count1BitsOfUint(
					static_cast<uint32>(InBits)));
			return LocalHighCount + LocalLowCount;
#elif defined(PSYQUE_BIT_ALGORITHM_FOR_MSC)
			return __popcnt64(InBits);
#elif defined(PSYQUE_BIT_ALGORITHM_FOR_GNUC) && ULONG_MAX == 0xFFFFFFFFFFFFFFFF
			return __builtin_popcountl(InBits);
#elif defined(PSYQUE_BIT_ALGORITHM_FOR_GNUC) && ULLONG_MAX == 0xFFFFFFFFFFFFFFFF
			return __builtin_popcountll(InBits);
#else
			return Psyque::_private::Count1BitsByTable(InBits);
#endif
		}

		//---------------------------------------------------------------------
		/// @brief 無符号整数の最上位ビットから、0が連続する数を数える。
		/// @details 以下のウェブページを参考にした。
		/// http://www.nminoru.jp/~nminoru/programming/bitcount.html
		/// @return 最上位ビットから0が連続する数。
		inline SIZE_T CountLeading0BitsByLogical(
			/// [in] ビットを数える無符号整数の値。
			uint8 const InBits)
		PSYQUE_NOEXCEPT
		{
			unsigned LocalBits(InBits);
			LocalBits = LocalBits | (LocalBits >> 1);
			LocalBits = LocalBits | (LocalBits >> 2);
			LocalBits = LocalBits | (LocalBits >> 4);
			return Psyque::_private::Count1BitsOfUint(
				static_cast<uint8>(~LocalBits));
		}

		/// @copydoc CountLeading0BitsByLogical()
		inline SIZE_T CountLeading0BitsByLogical(
			uint16 const InBits)
		PSYQUE_NOEXCEPT
		{
			unsigned LocalBits(InBits);
			LocalBits = LocalBits | (LocalBits >> 1);
			LocalBits = LocalBits | (LocalBits >> 2);
			LocalBits = LocalBits | (LocalBits >> 4);
			LocalBits = LocalBits | (LocalBits >> 8);
			return Psyque::_private::Count1BitsOfUint(
				static_cast<uint16>(~LocalBits));
		}

		/// @copydoc CountLeading0BitsByLogical()
		inline SIZE_T CountLeading0BitsByLogical(
			uint32 const InBits)
		PSYQUE_NOEXCEPT
		{
			unsigned LocalBits(InBits);
			LocalBits = LocalBits | (LocalBits >> 1);
			LocalBits = LocalBits | (LocalBits >> 2);
			LocalBits = LocalBits | (LocalBits >> 4);
			LocalBits = LocalBits | (LocalBits >> 8);
			LocalBits = LocalBits | (LocalBits >>16);
			return Psyque::_private::Count1BitsOfUint(
				static_cast<uint32>(~LocalBits));
		}

		/// @copydoc CountLeading0BitsByLogical()
		inline SIZE_T CountLeading0BitsByLogical(
			uint64 InBits)
		PSYQUE_NOEXCEPT
		{
			auto LocalBits(InBits);
			LocalBits = LocalBits | (LocalBits >> 1);
			LocalBits = LocalBits | (LocalBits >> 2);
			LocalBits = LocalBits | (LocalBits >> 4);
			LocalBits = LocalBits | (LocalBits >> 8);
			LocalBits = LocalBits | (LocalBits >>16);
			LocalBits = LocalBits | (LocalBits >>32);
			return Psyque::_private::Count1BitsOfUint(~LocalBits);
		}

		//---------------------------------------------------------------------
		/// @brief 浮動小数点のビットパターンを使って、
		/// 無符号整数の最上位ビットから0が連続する数を数える。
		/// @details 以下のウェブページを参考にした。
		/// http://www.nminoru.jp/~nminoru/programming/bitcount.html
		/// @return 最上位ビットから0が連続する数。
		template<typename TemplateBits>
		SIZE_T CountLeading0BitsByFloat(
			/// [in] ビットを数える整数の値。
			TemplateBits const InBits)
		{
			static_assert(
				// InBits は、無符号整数型であること。
				std::is_unsigned<TemplateBits>::value,
				"'InBits' must be unsigned integer type.");
			static_assert(
				// 浮動小数点の基数は、2であること。
				FLT_RADIX == 2, "Floating point radix must be 2.");
			static_assert(
				// InBits のビット数は、FLT_MANT_DIG未満であること。
				sizeof(InBits) * CHAR_BIT < FLT_MANT_DIG,
				"Bit size of 'InBits' must be less than FLT_MANT_DIG.");
			return sizeof(InBits) * CHAR_BIT
				+ (1 - FLT_MIN_EXP) - (
					Psyque::TFloatBitset<float>(InBits + 0.5f).Bitset
					>> (FLT_MANT_DIG - 1));
		}

		/// @copydoc CountLeading0BitsByFloat()
		template<> inline SIZE_T CountLeading0BitsByFloat(uint32 const InBits)
		{
			static_assert(
				// 浮動小数点の基数は、2であること。
				FLT_RADIX == 2, "Floating point radix must be 2.");
			static_assert(
				// InBits のビット数は、DBL_MANT_DIG未満であること。
				sizeof(InBits) * CHAR_BIT < DBL_MANT_DIG,
				"Bit size of 'InBits' must be less than DBL_MANT_DIG.");
			return sizeof(InBits) * CHAR_BIT
				+ (1 - DBL_MIN_EXP) - (
					Psyque::TFloatBitset<double>(InBits + 0.5).Bitset
					>> (DBL_MANT_DIG - 1));
		}

		//---------------------------------------------------------------------
		/// @brief 無符号整数の最上位ビットから、0が連続する数を数える。
		/// @return 最上位ビットから0が連続する数。
		template<typename TemplateBits>
		SIZE_T CountLeading0BitsOfUint(
			/// [in] ビットを数える整数の値。
			TemplateBits const InBits)
		{
			static_assert(
				// InBitsのビット数は、32以下であること。
				sizeof(InBits) <= sizeof(uint32),
				"Bit size of 'InBits' must be less than or equal to 32.");
			static_assert(
				// InBits は、無符号整数型であること。
				std::is_unsigned<TemplateBits>::value,
				"'InBits' must be unsigned integer type.");
			enum: unsigned
			{
				BIT_SIZE = sizeof(InBits) * CHAR_BIT,
				SIZE_DIFF = 32 - BIT_SIZE,
			};
#if defined(PSYQUE_BIT_ALGORITHM_FOR_MSC) && defined(BitScanReverse)
			if (InBits == 0)
			{
				return BIT_SIZE;
			}
			unsigned long LocalIndex;
			BitScanReverse(&LocalIndex, InBits);
			return BIT_SIZE - 1 - LocalIndex;
#elif defined(PSYQUE_BIT_ALGORITHM_FOR_MSC) && defined(CountLeadingZeros)
			return CountLeadingZeros(InBits) - SIZE_DIFF;
#elif defined(PSYQUE_BIT_ALGORITHM_FOR_GNUC)
			return InBits != 0? __builtin_clz(InBits) - SIZE_DIFF: BIT_SIZE;
#elif defined(PSYQUE_BIT_ALGORITHM_FOR_ARMCC)
			/// @note ARMのclzは、0判定がなくてもいい気がする。
			return InBits != 0? __clz(InBits) - SIZE_DIFF: BIT_SIZE;
#elif defined(PSYQUE_BIT_ALGORITHM_FOR_GHS)
			return InBits != 0? __CLZ(InBits) - SIZE_DIFF: BIT_SIZE;
#elif defined(PSYQUE_COUNT_LEADING_0BITS_BY_FLOAT)
			return Psyque::_private::CountLeading0BitsByFloat(InBits);
#else
			return Psyque::_private::CountLeading0BitsByLogical(InBits);
#endif
		}

		/// @brief 64ビット無符号整数の最上位ビットから、0が連続する数を数える。
		/// @return 最上位ビットから0が連続する数。
		template<> inline SIZE_T CountLeading0BitsOfUint(
			/// [in] ビットを数える整数の値。
			uint64 const InBits)
		{
#if PSYQUE_BIT_ALGORITHM_INTRINSIC_SIZE < 64
#	if defined(PSYQUE_COUNT_LEADING_0BITS_BY_FLOAT)
			if ((InBits >> (DBL_MANT_DIG - 1)) == 0)
			{
				// 浮動小数点を利用し、最上位ビットから0が連続する数を数える。
				return sizeof(InBits) * CHAR_BIT
					+ (1 - DBL_MIN_EXP) - (
						Psyque::TFloatBitset<double>(InBits + 0.5).Bitset
						>> (DBL_MANT_DIG - 1));
			}
			else
			{
				static_assert(
					// DBL_MANT_DIGは、48より大きいこと。
					48 < DBL_MANT_DIG,
					"DBL_MANT_DIG must be greater than 48.");
				return Psyque::_private::CountLeading0BitsByFloat(
					static_cast<uint16>(InBits >> 48));
			}
#	elif 1
			// 上位32ビットと下位32ビットに分ける。
			auto const local_high_bits(
				static_cast<uint32>(InBits >> 32));
			if (local_high_bits != 0)
			{
				return Psyque::_private::CountLeading0BitsByLogical(
					local_high_bits);
			}
			return 32 + Psyque::_private::CountLeading0BitsByLogical(
				static_cast<uint32>(InBits));
#	else
			// 64ビットのまま処理する。
			return Psyque::_private::CountLeading0BitsByLogical(InBits);
#	endif
#elif defined(PSYQUE_BIT_ALGORITHM_FOR_MSC) && defined(BitScanReverse64)
			if (InBits == 0)
			{
				return 64;
			}
			unsigned long LocalIndex;
			BitScanReverse64(&LocalIndex, InBits);
			return 63 - LocalIndex;
#elif defined(PSYQUE_BIT_ALGORITHM_FOR_MSC) && defined(CountLeadingZeros64)
			return CountLeadingZeros64(InBits);
#elif defined(PSYQUE_BIT_ALGORITHM_FOR_GNUC) && ULONG_MAX == 0xFFFFFFFFFFFFFFFF
			return InBits != 0? __builtin_clzl(InBits): 64;
#elif defined(PSYQUE_BIT_ALGORITHM_FOR_GNUC) && ULLONG_MAX == 0xFFFFFFFFFFFFFFFF
			return InBits != 0? __builtin_clzll(InBits): 64;
#else
			return Psyque::_private::CountLeading0BitsByLogical(InBits);
#endif
		}

		//---------------------------------------------------------------------
		/// @brief 無符号整数の最下位ビットから、0が連続する数を数える。
		/// @return 最下位ビットから、0が連続する数。
		template<typename TemplateBits>
		SIZE_T CountTrailing0BitsByLogical(
			/// [in] 数える整数の値。
			TemplateBits const InBits)
		{
#if 1
			static Psyque::_private::FTrailing0Bits const StaticTrailing0Bits;
			return StaticTrailing0Bits.Count(InBits);
#else
			/// @note 以下のウェブページを参考にした。
			/// http://www.nminoru.jp/~nminoru/programming/bitcount.html
			return Psyque::_private::Count1BitsOfUint(
				static_cast<TemplateBits>((~InBits) & (InBits - 1)));
#endif
		}

		/// @brief 無符号整数の最下位ビットから、0が連続する数を数える。
		/// @return 最下位ビットから、0が連続する数。
		template<typename TemplateBits>
		SIZE_T CountTrailing0BitsOfUint(
			/// [in] ビットを数える整数の値。
			TemplateBits const InBits)
		{
			static_assert(
				// InBitsのビット数は、32以下であること。
				sizeof(InBits) <= sizeof(uint32),
				"Bit size of 'InBits' must be less than or equal to 32.");
			static_assert(
				// InBits は、無符号整数型であること。
				std::is_unsigned<TemplateBits>::value,
				"'InBits' must be unsigned integer type.");
#if defined(PSYQUE_BIT_ALGORITHM_FOR_MSC) && defined(BitScanForward)
			if (InBits == 0)
			{
				return sizeof(InBits) * CHAR_BIT;
			}
			unsigned long LocalIndex;
			BitScanForward(&LocalIndex, InBits);
			return LocalIndex;
#elif defined(PSYQUE_BIT_ALGORITHM_FOR_GNUC)
			return InBits != 0?
				__builtin_ctz(InBits): sizeof(InBits) * CHAR_BIT;
#else
			return Psyque::_private::CountTrailing0BitsByLogical(InBits);
#endif
		}

		/// @brief 64ビット無符号整数の最下位ビットから、0が連続する数を数える。
		/// @return 最下位ビットから、0が連続する数。
		template<> inline SIZE_T CountTrailing0BitsOfUint(
			/// [in] ビットを数える整数の値。
			uint64 const InBits)
		{
#if PSYQUE_BIT_ALGORITHM_INTRINSIC_SIZE < 64
			// 上位32ビットと下位32ビットに分ける。
			auto const LocalLowCount(
				Psyque::_private::CountTrailing0BitsOfUint(
					static_cast<uint32>(InBits)));
			if (LocalLowCount < 32)
			{
				return LocalLowCount;
			}
			return 32 + Psyque::_private::CountTrailing0BitsOfUint(
				static_cast<uint32>(InBits >> 32));
#elif defined(PSYQUE_BIT_ALGORITHM_FOR_MSC) && defined(BitScanForward64)
			if (InBits == 0)
			{
				return 64;
			}
			unsigned long LocalIndex;
			BitScanForward64(&LocalIndex, InBits);
			return LocalIndex;
#elif defined(PSYQUE_BIT_ALGORITHM_FOR_GNUC) && ULONG_MAX == 0xFFFFFFFFFFFFFFFF
			return InBits != 0? __builtin_ctzl(InBits): 64;
#elif defined(PSYQUE_BIT_ALGORITHM_FOR_GNUC) && ULLONG_MAX == 0xFFFFFFFFFFFFFFFF
			return InBits != 0? __builtin_ctzll(InBits): 64;
#else
			return Psyque::_private::CountTrailing0BitsByLogical(InBits);
#endif
		}
	}

	//-------------------------------------------------------------------------
	/// @name 1ビット単位の参照
	/// @{

	/// @brief 指定された位置のビット値を取得する。
	/// @return
	/// 指定された位置のビット値。
	/// ただし InPosition が sizeof(int) 以上だった場合…
	/// - InBits が有符号整数型なら、符号ビットを返す。
	/// - InBits が無符号整数型なら、falseを返す。
	template<typename TemplateBits>
	bool PSYQUE_CONSTEXPR Is1Bit(
		/// [in] ビット集合として扱う整数値。
		TemplateBits const InBits,
		/// [in] 取得するビットの位置。
		SIZE_T const InPosition)
	PSYQUE_NOEXCEPT
	{
		return (Psyque::ShiftRightBitwise(InBits, InPosition) & 1) != 0;
	}

	/// @brief 指定された位置のビット値を取得する。
	/// @return 指定された位置のビット値。
	/// ただし InPosition が sizeof(int) 以上だった場合、未定義。
	template<typename TemplateBits>
	bool PSYQUE_CONSTEXPR Is1BitFast(
		/// [in] ビット集合として扱う整数値。
		TemplateBits const InBits,
		/// [in] 取得するビットの位置。
		SIZE_T const InPosition)
	PSYQUE_NOEXCEPT
	{
		return (Psyque::ShiftRightBitwiseFast(InBits, InPosition) & 1) != 0;
	}

	/// @brief 整数で、1になっているビットを数える。
	/// @return 1になっているビットの数。
	template<typename TemplateBits>
	SIZE_T Count1Bits(
		/// [in] ビットを数える整数の値。
		TemplateBits const InBits)
	{
		using Uint = typename Psyque::_private::TMakeUint<TemplateBits>::Type;
		static_assert(
			// TemplateBits は、64ビット以下の整数型であること。
			std::is_unsigned<Uint>::value,
			"'TemplateBits' must be integer type of 64bits or less.");
		return Psyque::_private::Count1BitsOfUint(static_cast<Uint>(InBits));
	}

	/// @brief 整数の最上位ビットから、0が連続する数を数える。
	/// @return 最上位ビットから0が連続する数。
	template<typename TemplateBits>
	SIZE_T CountLeading0Bits(
		/// [in] ビットを数える整数の値。
		TemplateBits const InBits)
	{
		using Uint = typename Psyque::_private::TMakeUint<TemplateBits>::Type;
		static_assert(
			// TemplateBits は、64ビット以下の整数型であること。
			std::is_unsigned<Uint>::value,
			"'TemplateBits' must be integer type of 64bits or less.");
		return Psyque::_private::CountLeading0BitsOfUint(
			static_cast<Uint>(InBits));
	}

	/// @brief 整数の最下位ビットから、0が連続する数を数える。
	/// @return 最下位ビットから0が連続する数。
	template<typename TemplateBits>
	SIZE_T CountTrailing0Bits(
		///[in] ビットを数える整数の値。
		TemplateBits const InBits)
	{
		using Uint = typename Psyque::_private::TMakeUint<TemplateBits>::Type;
		static_assert(
			// TemplateBits は、64ビット以下の整数型であること。
			std::is_unsigned<Uint>::value,
			"'TemplateBits' must be integer type of 64bits or less.");
		return Psyque::_private::CountTrailing0BitsOfUint(
			static_cast<Uint>(InBits));
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name ビット範囲の操作
	/// @{

	/// @brief ビットマスクを作る。
	/// @return ビット幅が InBitWidth のビットマスク。
	template<typename TemplateBits>
	TemplateBits PSYQUE_CONSTEXPR MakeBitMask(
		/// [in] ビットマスクのビット幅。
		SIZE_T const InBitWidth)
	{
		return ~Psyque::ShiftLeftBitwise(~TemplateBits(0), InBitWidth);
	}

	/// @brief 指定されたビット範囲を取得する。
	/// @return ビット範囲。
	template<typename TemplateBits>
	TemplateBits PSYQUE_CONSTEXPR EmbossBitField(
		/// [in] ビット集合として扱う整数値。
		TemplateBits const InBits,
		/// [in] 取得するビット範囲のビット位置。
		SIZE_T const InBitPosition,
		/// [in] 取得するビット範囲のビット幅。
		SIZE_T const InBitWidth)
	{
		return (
			PSYQUE_ASSERT(
				Psyque::_private::IsValidBitWidth<TemplateBits>(
					InBitPosition, InBitWidth)),
			Psyque::ShiftLeftBitwiseFast(
				Psyque::MakeBitMask<TemplateBits>(InBitWidth), InBitPosition)
			& InBits);
	}

	/// @brief 指定されたビット範囲の値を取得する。
	/// @return ビット範囲の値。
	template<typename TemplateBits>
	TemplateBits PSYQUE_CONSTEXPR GetBitset(
		/// [in] ビット集合として扱う整数値。
		TemplateBits const InBits,
		/// [in] 取得するビット範囲のビット位置。
		SIZE_T const InBitPosition,
		/// [in] 取得するビット範囲のビット幅。
		SIZE_T const InBitWidth)
	{
		return (
			PSYQUE_ASSERT(
				Psyque::_private::IsValidBitWidth<TemplateBits>(
					InBitPosition, InBitWidth)),
			Psyque::MakeBitMask<TemplateBits>(InBitWidth)
			& Psyque::ShiftRightBitwiseFast(InBits, InBitPosition));
	}

	/// @brief 指定されたビット範囲を0にする。
	/// @return 指定されたビット範囲を0にした整数値。
	template<typename TemplateBits>
	TemplateBits PSYQUE_CONSTEXPR ResetBitField(
		/// [in] InBits		 ビット集合として扱う整数値。
		TemplateBits const InBits,
		/// [in] 設定するビット範囲のビット位置。
		SIZE_T const InBitPosition,
		/// [in] 設定するビット範囲のビット幅。
		SIZE_T const InBitWidth)
	{
		return (
			PSYQUE_ASSERT(
				Psyque::_private::IsValidBitWidth<TemplateBits>(
					InBitPosition, InBitWidth)),
			InBits & ~Psyque::ShiftLeftBitwiseFast(
				Psyque::MakeBitMask<TemplateBits>(InBitWidth),
				InBitPosition));
	}

	/// @brief 指定されたビット範囲に値を埋め込む。
	/// @return 指定されたビット位置に InValue を埋め込んだ整数値。
	template<typename TemplateBits>
	TemplateBits PSYQUE_CONSTEXPR SetBitset(
		/// [in] ビット集合として扱う整数値。
		TemplateBits const InBits,
		/// [in] 埋め込むビット範囲のビット位置。
		SIZE_T const InBitPosition,
		/// [in] 埋め込むビット範囲のビット幅。
		SIZE_T const InBitWidth,
		/// [in] 埋め込む値。
		TemplateBits const InValue)
	PSYQUE_NOEXCEPT
	{
		return (
			PSYQUE_ASSERT(
				Psyque::ShiftRightBitwise(InValue, InBitWidth) == 0),
			Psyque::ResetBitField(InBits, InBitPosition, InBitWidth)
			| Psyque::ShiftLeftBitwiseFast(InValue, InBitPosition));
	}
	/// @}
} // namespace Psyque

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 単体テストに使う。この名前空間をユーザーが直接アクセスするのは禁止。
namespace PsyqueTest
{
	template<typename TemplateValue> void Count1Bits()
	{
		TemplateValue LocalBits(0);
		check(Psyque::Count1Bits(LocalBits) == 0);
		for (unsigned i(0); i < sizeof(TemplateValue) * CHAR_BIT; ++i)
		{
			LocalBits = Psyque::ShiftLeftBitwiseFast(LocalBits, 1) | 1;
			check(i + 1 == Psyque::Count1Bits(LocalBits));
		}
	}

	inline void Count1Bits()
	{
		PsyqueTest::Count1Bits<char>();
		PsyqueTest::Count1Bits<short>();
		PsyqueTest::Count1Bits<int>();
		PsyqueTest::Count1Bits<long>();
#if ULLONG_MAX <= 0xFFFFFFFFFFFFFFFF
		PsyqueTest::Count1Bits<long long>();
#endif
	}

	template<typename TemplateValue> void CountLeading0Bits()
	{
		check(
			Psyque::CountLeading0Bits(TemplateValue(0))
			== sizeof(TemplateValue) * CHAR_BIT);
		for (unsigned i(0); i < sizeof(TemplateValue) * CHAR_BIT; ++i)
		{
			auto const LocalClz(
				Psyque::CountLeading0Bits(
					Psyque::ShiftLeftBitwiseFast(TemplateValue(1), i)));
			check(LocalClz + i == sizeof(TemplateValue) * CHAR_BIT - 1);
		}
	}

	inline void CountLeading0Bits()
	{
		PsyqueTest::CountLeading0Bits<char>();
		PsyqueTest::CountLeading0Bits<short>();
		PsyqueTest::CountLeading0Bits<int>();
		PsyqueTest::CountLeading0Bits<long>();
#if ULLONG_MAX <= 0xFFFFFFFFFFFFFFFF
		PsyqueTest::CountLeading0Bits<long long>();
#endif
	}

	template<typename TemplateValue> void CountTrailing0Bits()
	{
		check(
			Psyque::CountTrailing0Bits(TemplateValue(0))
			== sizeof(TemplateValue) * CHAR_BIT);
		for (unsigned i(0); i < sizeof(TemplateValue) * CHAR_BIT; ++i)
		{
			auto const LocalCtz(
				Psyque::CountTrailing0Bits(
					Psyque::ShiftLeftBitwiseFast(TemplateValue(1), i)));
			check(LocalCtz == i);
		}
	}

	inline void CountTrailing0Bits()
	{
		PsyqueTest::CountTrailing0Bits<char>();
		PsyqueTest::CountTrailing0Bits<short>();
		PsyqueTest::CountTrailing0Bits<int>();
		PsyqueTest::CountTrailing0Bits<long>();
#if ULLONG_MAX <= 0xFFFFFFFFFFFFFFFF
		PsyqueTest::CountTrailing0Bits<long long>();
#endif
	}

	inline void BitAlgorithm()
	{
		PsyqueTest::Count1Bits();
		PsyqueTest::CountLeading0Bits();
		PsyqueTest::CountTrailing0Bits();
	}
}

// vim: set noexpandtab:
