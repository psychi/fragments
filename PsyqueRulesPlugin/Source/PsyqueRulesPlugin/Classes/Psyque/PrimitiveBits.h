// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::Hash::TPrimitiveBits
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#include "./BitAlgorithm.h"

namespace Psyque
{
	namespace Hash
	{
		/// @cond
		template<typename, typename> class TPrimitiveBits;
		/// @endcond
	}

	namespace _private
	{
		/// @brief ソート済配列で、
		/// 指定された要素以上の値が現れる最初のインデクス番号を取得する。
		template<
			typename TemplateElement,
			typename TemplateAllocator,
			typename TemplateKey,
			typename TemplateCompare>
		int32 LowerBound(
			/// [in] 検索する配列。
			TArray<TemplateElement, TemplateAllocator> const& InArray,
			/// [in] 検索する範囲の先頭インデクス番号。
			int32 const InBegin,
			/// [in] 検索する範囲の末尾インデクス番号。
			int32 const InEnd,
			/// [in] 検索する値。
			TemplateKey const& InKey,
			/// [in] 検索に使う比較関数オブジェクト。
			TemplateCompare const& InCompare)
		{
			auto const LocalArrayNum(InArray.Num());
			if (InEnd <= InBegin)
			{
				check(
					0 <= InBegin
					&& 0 <= InEnd
					&& InBegin == InEnd
					&& InEnd <= LocalArrayNum);
				return InEnd;
			}
			if (InBegin < 0 || LocalArrayNum <= InBegin)
			{
				check(false);
				return InEnd;
			}
			auto const LocalFront(&InArray[0]);
			auto const LocalPosition(
				std::lower_bound(
					LocalFront + InBegin,
					LocalFront + InEnd,
					InKey,
					InCompare));
			return LocalPosition - LocalFront;
		}
		/// @copydoc LowerBound
		template<
			typename TemplateElement,
			typename TemplateAllocator,
			typename TemplateKey>
		int32 LowerBound(
			TArray<TemplateElement, TemplateAllocator> const& InArray,
			int32 const InBegin,
			int32 const InEnd,
			TemplateKey const& InKey)
		{
			return LowerBound(
				InArray, InBegin, InEnd, InKey,
				[](
					TemplateElement const& InLeft,
					TemplateElement const& InRight)
				{
					return InLeft < InRight;
				});
		}
		/// @copydoc LowerBound
		template<
			typename TemplateElement,
			typename TemplateAllocator,
			typename TemplateKey,
			typename TemplateCompare>
		int32 LowerBound(
			TArray<TemplateElement, TemplateAllocator> const& InArray,
			TemplateKey const& InKey,
			TemplateCompare const& InCompare)
		{
			return LowerBound(InArray, 0, InArray.Num(), InKey, InCompare);
		}
		/// @copydoc LowerBound
		template<
			typename TemplateElement,
			typename TemplateAllocator,
			typename TemplateKey>
		int32 LowerBound(
			TArray<TemplateElement, TemplateAllocator> const& InArray,
			TemplateKey const& InKey)
		{
			return LowerBound(InArray, 0, InArray.Num(), InKey);
		}
	}
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 組み込み型のビット表現をそのまま使うハッシュ関数オブジェクト。
/// @tparam TemplateValue  @copydoc TPrimitiveBits::argument_type
/// @tparam template_result @copydoc TPrimitiveBits::result_type
template<typename TemplateValue, typename template_result = std::size_t>
class Psyque::Hash::TPrimitiveBits
{
	private: using ThisClass = TPrimitiveBits;

	//-------------------------------------------------------------------------
	/// @brief ハッシュ関数の引数となるキー。
	/// @details
	/// 以下の組み込み型が使える。
	/// - ポインタ型。
	/// - 列挙型。
	/// - 整数型。
	/// - 浮動小数点数型。
	public: typedef TemplateValue argument_type;
	static_assert(
		std::is_pointer<TemplateValue>::value
		|| std::is_enum<TemplateValue>::value
		|| std::is_integral<TemplateValue>::value
		|| std::is_floating_point<TemplateValue>::value,
		"'TemplateValue' is invalid type.");

	/// @brief ハッシュ関数の戻り値。
	/// @details
	/// template_result が std::size_t へ暗黙に型変換できるなら、
	/// ThisClass は std::Hash のインタフェイスと互換性を持つ。
	public: typedef template_result result_type;

	//-------------------------------------------------------------------------
	/// @brief キーの型の種別。
	private: enum class EKind: unsigned char
	{
		POINTER, ///< ポインタ型。
		INTEGER, ///< 整数型。
		Float,	 ///< 浮動小数点数型。
	};

	//-------------------------------------------------------------------------
	/// @brief キーに対応するハッシュ値を取得する。
	/// @param[in] InKey キー。
	/// @return InKey に対応するハッシュ値。
	public: typename ThisClass::result_type operator()(
		typename ThisClass::argument_type const InKey)
	const PSYQUE_NOEXCEPT
	{
		return ThisClass::GetBits(
			InKey,
			std::integral_constant<
				typename ThisClass::EKind,
				std::is_pointer<typename ThisClass::argument_type>::value?
					EKind::POINTER:
					std::is_floating_point<typename ThisClass::argument_type>::value?
						EKind::Float: EKind::INTEGER>());
	}

	//-------------------------------------------------------------------------
	/// @brief ポインタ値をそのままハッシュ値として使う。
	private: template<typename template_pointer>
	static typename ThisClass::result_type GetBits(
		/// [in] InPointer ポインタ。
		template_pointer const InPointer,
		std::integral_constant<typename ThisClass::EKind, ThisClass::EKind::POINTER> const&)
	PSYQUE_NOEXCEPT
	{
		return reinterpret_cast<typename ThisClass::result_type>(InPointer);
	}

	/// @brief 整数値をそのままハッシュ値として使う。
	private: template<typename TemplateInteger>
	static typename ThisClass::result_type GetBits(
		/// [in] InInteger 整数。
		TemplateInteger const InInteger,
		std::integral_constant<typename ThisClass::EKind, ThisClass::EKind::INTEGER> const&)
	PSYQUE_NOEXCEPT
	{
		return static_cast<typename ThisClass::result_type>(InInteger);
	}

	/// @brief 浮動小数点数のビット表現をハッシュ値として使う。
	private: template<typename TemplateFloat>
	static typename ThisClass::result_type GetBits(
		/// [in] 浮動小数点数。
		TemplateFloat const InFloat,
		std::integral_constant<typename ThisClass::EKind, ThisClass::EKind::Float> const&)
	PSYQUE_NOEXCEPT
	{
		Psyque::FloatBitset<TemplateFloat> const LocalValue(InFloat);
		return static_cast<typename ThisClass::result_type>(LocalValue.Bitset);
	}

}; // class Psyque::Hash::TPrimitiveBits

// vim: set noexpandtab:
