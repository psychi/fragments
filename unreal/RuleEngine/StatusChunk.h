// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RuleEngine::_private::TStatusChunk
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#include <utility>
#include "Containers/Array.h"
#include "../BitAlgorithm.h"

namespace Psyque
{
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

	/// @cond
	namespace RuleEngine
	{
		namespace _private
		{
			template<typename, typename, typename, typename> class TStatusChunk;
		} // namespace _private
	} // namespace RuleEngine
	/// @endcond
} // namespace Psyque

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 状態値を格納するビット領域のコンテナ。
/// @tparam TemplateBitBlock    @copydoc TStatusChunk::BitBlock
/// @tparam TemplateBitPosition @copydoc TStatusChunk::BitPosition
/// @tparam TemplateBitWidth    @copydoc TStatusChunk::BitWidth
/// @tparam TemplateAllocator   @copydoc TStatusChunk::Allocator
template<
	typename TemplateBitBlock,
	typename TemplateBitPosition,
	typename TemplateBitWidth,
	typename TemplateAllocator>
class Psyque::RuleEngine::_private::TStatusChunk
{
	/// @brief thisが指す値の型。
	using This = TStatusChunk;

	//-------------------------------------------------------------------------
	public:
	/// @brief ビット列の単位を表す型。
	using BitBlock = TemplateBitBlock;
	static_assert(
		std::is_unsigned<TemplateBitBlock>::value,
		"TemplateBitBlock is not unsigned integer type.");
	/// @brief ビット列のビット位置を表す型。
	using BitPosition = TemplateBitPosition;
	static_assert(
		std::is_unsigned<TemplateBitPosition>::value,
		"TemplateBitPosition is not unsigned integer type.");
	/// @brief ビット列のビット幅を表す型。
	using BitWidth = TemplateBitWidth;
	static_assert(
		std::is_unsigned<TemplateBitWidth>::value,
		"TemplateBitWidth is not unsigned integer type.");
	/// @brief コンテナに用いる、 FHeapAllocator 互換のメモリ割当子の型 。
	using Allocator = TemplateAllocator;
	/// @copydoc This::BitBlocks
	using BitBlockArray = TArray<TemplateBitBlock, TemplateAllocator>;
	/// @copybrief This::EmptyBitsets
	/// @details
	/// - first は、空きビット領域のビット幅。
	/// - second は、空きビット領域のビット位置。
	using EmptyBitsetArray = TArray<
		std::pair<TemplateBitWidth, TemplateBitPosition>, TemplateAllocator>;
	enum: TemplateBitPosition
	{
		/// @brief 無効なビット位置。
		INVALID_BIT_POSITION = ~static_cast<TemplateBitPosition>(0),
	};
	enum: TemplateBitWidth
	{
		/// @brief ビット列単位のビット数。
		BLOCK_BIT_WIDTH = static_cast<TemplateBitWidth>(
			sizeof(TemplateBitBlock) * CHAR_BIT),
	};
	static_assert(
		// This::BLOCK_BIT_WIDTH が This::BitWidth に収まるのを確認する。
		This::BLOCK_BIT_WIDTH < (
			1 << (sizeof(This::BitWidth) * CHAR_BIT - 1)),
		"This::BLOCK_BIT_WIDTH is overflow.");

	//-------------------------------------------------------------------------
	public:
	/// @brief 空のビット領域チャンクを構築する。
	TStatusChunk() {}

	/// @brief 状態値を格納するビット領域を生成する。
	/// @return 生成したビット領域のビット位置。
	/// 失敗した場合は This::INVALID_BIT_POSITION を返す。
	TemplateBitPosition AddBitset(
		/// [in] 生成するビット領域のビット数。
		TemplateBitWidth const InBitWidth)
	{
		// 状態値を格納できるビット領域を、空きビット領域から取得する。
		auto const LocalEmptyIndex(
			Psyque::_private::LowerBound(
				this->EmptyBitsets,
				typename This::EmptyBitsetArray::ElementType(InBitWidth, 0)));
		return LocalEmptyIndex < this->EmptyBitsets.Num()?
			// 既存の空き領域を再利用する。
			this->ReallocateBitset(InBitWidth, LocalEmptyIndex):
			// 適切な空き領域がないので、新たな領域を追加する。
			this->AllocateBitset(InBitWidth);
	}

	/// @brief ビット領域の値を取得する。
	/// @return ビット領域の値。該当するビット領域がない場合は~0を返す。
	TemplateBitBlock GetBitset(
		/// [in] 値を取得するビット領域のビット位置。
		TemplateBitPosition const InBitPosition,
		/// [in] 値を取得するビット領域のビット幅。
		TemplateBitWidth const InBitWidth)
	const PSYQUE_NOEXCEPT
	{
		if (This::BLOCK_BIT_WIDTH < InBitWidth)
		{
			check(false);
			return ~static_cast<TemplateBitBlock>(0);
		}
		auto const LocalBlockIndex(InBitPosition / This::BLOCK_BIT_WIDTH);
		if (this->GetBlockCount() <= LocalBlockIndex)
		{
			check(false);
			return ~static_cast<TemplateBitBlock>(0);
		}

		// ビット列ブロックでのビット位置を決定し、値を取り出す。
		return Psyque::GetBitset(
			this->BitBlocks[LocalBlockIndex],
			InBitPosition - LocalBlockIndex * This::BLOCK_BIT_WIDTH,
			InBitWidth);
	}

	/// @brief ビット領域に値を設定する。
	/// @retval 正 元とは異なる値を設定した。
	/// @retval 0  元と同じ値を設定した。
	/// @retval 負 失敗。値を設定できなかった。
	int8 SetBitset(
		/// [in] 値を設定するビット領域のビット位置。
		TemplateBitPosition const InBitPosition,
		/// [in] 値を設定するビット領域のビット幅。
		TemplateBitWidth const InBitWidth,
		/// [in] ビット領域に設定する値。
		TemplateBitBlock const InValue)
	PSYQUE_NOEXCEPT
	{
		if (Psyque::ShiftRightBitwise(InValue, InBitWidth) != 0)
		{
			return -1;
		}
		auto const LocalBlockIndex(InBitPosition / This::BLOCK_BIT_WIDTH);
		if (this->GetBlockCount() <= LocalBlockIndex)
		{
			check(false);
			return -1;
		}

		// ビット列単位でのビット位置を決定し、値を埋め込む。
		auto& LocalBlock(this->BitBlocks[LocalBlockIndex]);
		auto const LocalLastBlock(LocalBlock);
		LocalBlock = Psyque::SetBitset(
			LocalBlock,
			InBitPosition - LocalBlockIndex * This::BLOCK_BIT_WIDTH,
			InBitWidth,
			InValue);
		return LocalLastBlock != LocalBlock;
	}

	//-------------------------------------------------------------------------
	private:
	uint32 GetBlockCount() const
	{
		return static_cast<uint32>(this->BitBlocks.Num());
	}

	/// @brief 空きビット領域を再利用する。
	/// @return 再利用したビット領域のビット位置。
	TemplateBitPosition ReallocateBitset(
		/// [in] 再利用したい領域のビット幅。
		TemplateBitPosition const InBitWidth,
		/// [in] 再利用する空きビット領域を指すインデクス番号。
		int32 const InEmptyIndex)
	{
		// 既存の空き領域を再利用する。
		auto const& LocalEmptyBitset(this->EmptyBitsets[InEmptyIndex]);
		auto const LocalEmptyPosition(LocalEmptyBitset.second);
		auto const LocalEmptyWidth(LocalEmptyBitset.first);
		check(0 < LocalEmptyWidth);

		// 再利用する空き領域を削除する。
		this->EmptyBitsets.RemoveAt(InEmptyIndex, 1, false);
		if (InBitWidth < LocalEmptyWidth)
		{
			// 余りを空き領域として追加する。
			This::AddEmptyBitset(
				this->EmptyBitsets,
				LocalEmptyPosition + InBitWidth,
				LocalEmptyWidth - InBitWidth);
		}
		return LocalEmptyPosition;
	}

	/// @brief 状態値に使うビット領域を追加する。
	/// @return 追加したビット領域のビット位置。
	/// 失敗した場合は This::INVALID_BIT_POSITION を返す。
	TemplateBitPosition AllocateBitset(
		/// [in] 追加するビット領域のビット幅。
		TemplateBitWidth const InBitWidth)
	{
		if (InBitWidth <= 0 || This::BLOCK_BIT_WIDTH < InBitWidth)
		{
			check(false);
			return This::INVALID_BIT_POSITION;
		}

		// 新たにビット列を追加する。
		auto const LocalBitPosition(
			this->GetBlockCount() * This::BLOCK_BIT_WIDTH);
		if (This::INVALID_BIT_POSITION <= LocalBitPosition)
		{
			// ビット位置の最大値を超過した。
			check(false);
			return This::INVALID_BIT_POSITION;
		}
		auto const LocalAddBlockCount(
			(InBitWidth + This::BLOCK_BIT_WIDTH - 1) / This::BLOCK_BIT_WIDTH);
		this->BitBlocks.AddZeroed(LocalAddBlockCount);

		// 余りを空きビット領域に追加する。
		auto const LocalEmptyWidth(
			LocalAddBlockCount * This::BLOCK_BIT_WIDTH - InBitWidth);
		if (0 < LocalEmptyWidth)
		{
			This::AddEmptyBitset(
				this->EmptyBitsets,
				LocalBitPosition + InBitWidth,
				LocalEmptyWidth);
		}
		return static_cast<TemplateBitPosition>(LocalBitPosition);
	}

	/// @brief 空きビット領域を追加する。
	static void AddEmptyBitset(
		/// [in,out] 空きビット領域情報を追加するコンテナ。
		typename This::EmptyBitsetArray& OutEmptyBitsets,
		/// [in] 追加する空きビット領域のビット位置。
		TemplateBitPosition const InBitPosition,
		/// [in] 追加する空きビット領域のビット幅。
		std::size_t const InBitWidth)
	{
		using EmptyBitset = typename This::EmptyBitsetArray::ElementType;
		EmptyBitset const LocalEmptyBitset(
			static_cast<typename EmptyBitset::first_type>(InBitWidth),
			static_cast<typename EmptyBitset::second_type>(InBitPosition));
		if (InBitPosition == LocalEmptyBitset.second
			&& InBitWidth == LocalEmptyBitset.first)
		{
			OutEmptyBitsets.Insert(
				LocalEmptyBitset,
				Psyque::_private::LowerBound(
					OutEmptyBitsets, LocalEmptyBitset));
		}
		else {check(false);}
	}

	//-------------------------------------------------------------------------
	public:
	/// @brief ビット列のコンテナ。
	typename This::BitBlockArray BitBlocks;
	/// @brief 空きビット領域情報のコンテナ。
	typename This::EmptyBitsetArray EmptyBitsets;

}; // class Psyque::RuleEngine::_private::TStatusChunk

// vim: set noexpandtab:
