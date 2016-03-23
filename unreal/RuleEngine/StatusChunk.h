// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RuleEngine::_private::TStatusChunk
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#include <utility>
#include "Containers/Array.h"
#include "../BitAlgorithm.h"

/// @cond
namespace Psyque
{
	namespace RuleEngine
	{
		namespace _private
		{
			template<typename, typename, typename, typename> class TStatusChunk;
		} // namespace _private
	} // namespace RuleEngine
} // namespace Psyque
/// @endcond

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
	typedef TStatusChunk This;

	//-------------------------------------------------------------------------
	public:
	/// @brief ビット列の単位を表す型。
	typedef TemplateBitBlock BitBlock;
	static_assert(
		std::is_unsigned<TemplateBitBlock>::value,
		"TemplateBitBlock is not unsigned integer type.");
	/// @brief ビット列のビット位置を表す型。
	typedef TemplateBitPosition BitPosition;
	static_assert(
		std::is_unsigned<TemplateBitPosition>::value,
		"TemplateBitPosition is not unsigned integer type.");
	/// @brief ビット列のビット幅を表す型。
	typedef TemplateBitWidth BitWidth;
	static_assert(
		std::is_unsigned<TemplateBitWidth>::value,
		"TemplateBitWidth is not unsigned integer type.");
	/// @brief コンテナに用いる、 FHeapAllocator 互換のメモリ割当子の型 。
	typedef TemplateAllocator Allocator;
	/// @copydoc This::BitBlocks
	typedef
		TArray<typename This::BitBlock, typename This::Allocator>
		BitBlockArray;
	/// @copybrief This::EmptyFields
	/// @details
	/// - first は、空きビット領域のビット幅。
	/// - second は、空きビット領域のビット位置。
	typedef
		TArray<
			std::pair<typename This::BitWidth, typename This::BitPosition>,
			typename This::Allocator>
		EmptyFieldArray;
	enum: typename This::BitPosition
	{
		/// @brief 無効なビット位置。
		INVALID_BIT_POSITION = ~static_cast<typename This::BitPosition>(0),
	};
	enum: typename This::BitWidth
	{
		/// @brief ビット列単位のビット数。
		BLOCK_BIT_WIDTH = static_cast<typename This::BitWidth>(
			sizeof(typename This::BitBlock) * CHAR_BIT),
	};
	static_assert(
		// This::BLOCK_BIT_WIDTH が This::BitWidth に収まるのを確認する。
		This::BLOCK_BIT_WIDTH < (
			1 << (sizeof(This::BitWidth) * CHAR_BIT - 1)),
		"This::BLOCK_BIT_WIDTH is overflow.");

	//-------------------------------------------------------------------------
	public:
	/// @brief 空のビット領域チャンクを構築する。
	TStatusChunk(
		int32 const InBlockCount,
		int32 const InEmptyCount)
	{
		this->BitBlocks.Reserve(InBlockCount);
		this->EmptyFields.Reserve(InFieldCount);
	}

	/// @brief 状態値を格納するビット領域を生成する。
	/// @return
	///   生成したビット領域のビット位置。
	///   失敗した場合は This::INVALID_BIT_POSITION を返す。
	typename This::BitPosition AddBitField(
		/// [in] 生成するビット領域のビット数。
		typename This::BitWidth const InBitWidth)
	{
		// 状態値を格納できるビット領域を、空きビット領域から取得する。
		auto const LocalEmptyEnd(end(this->EmptyFields));
		auto const LocalEmptyPosition(
			std::lower_bound(
				begin(this->EmptyFields),
				LocalEmptyEnd,
				typename This::EmptyFieldArray::ElementType(InBitWidth, 0)));
		return LocalEmptyEnd != LocalEmptyPosition?
			// 既存の空き領域を再利用する。
			this->ReallocateBitField(InBitWidth, LocalEmptyPosition):
			// 適切な空き領域がないので、新たな領域を追加する。
			this->AllocateBitField(InBitWidth);
	}

	/// @brief ビット領域の値を取得する。
	/// @return ビット領域の値。該当するビット領域がない場合は~0を返す。
	typename This::BitBlock GetBitField(
		/// [in] 値を取得するビット領域のビット位置。
		typename This::BitPosition const InBitPosition,
		/// [in] 値を取得するビット領域のビット幅。
		typename This::BitWidth const InBitWidth)
	const PSYQUE_NOEXCEPT
	{
		if (This::BLOCK_BIT_WIDTH < InBitWidth)
		{
			check(false);
			return ~static_cast<typename This::BitBlock>(0);
		}
		auto const LocalBlockIndex(InBitPosition / This::BLOCK_BIT_WIDTH);
		if (this->GetBlockCount() <= LocalBlockIndex)
		{
			check(false);
			return ~static_cast<typename This::BitBlock>(0);
		}

		// ビット列ブロックでのビット位置を決定し、値を取り出す。
		return Psyque::GetBitField(
			this->BitBlocks[LocalBlockIndex],
			InBitPosition - LocalBlockIndex * This::BLOCK_BIT_WIDTH,
			InBitWidth);
	}

	/// @brief ビット領域に値を設定する。
	/// @retval 正 元とは異なる値を設定した。
	/// @retval 0  元と同じ値を設定した。
	/// @retval 負 失敗。値を設定できなかった。
	int8 SetBitField(
		/// [in] 値を設定するビット領域のビット位置。
		typename This::BitPosition const InBitPosition,
		/// [in] 値を設定するビット領域のビット幅。
		typename This::BitWidth const InBitWidth,
		/// [in] ビット領域に設定する値。
		typename This::BitBlock const InValue)
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
		LocalBlock = Psyque::SetBitField(
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
	typename This::BitPosition ReallocateBitField(
		/// [in] 再利用したい領域のビット幅。
		typename This::BitPosition const InBitWidth,
		/// [in] 再利用する空きビット領域のプロパティを指す反復子。
		typename This::EmptyFieldArray::TIterator const& InEmptyField)
	{
		// 既存の空き領域を再利用する。
		auto const LocalEmptyPosition(InEmptyField->second);
		auto const LocalEmptyWidth(InEmptyField->first);
		check(0 < LocalEmptyWidth);

		// 再利用する空き領域を削除する。
		this->EmptyFields.RemoveAt(InEmptyField.GetIndex(), 1, false);
		if (InBitWidth < LocalEmptyWidth)
		{
			// 余りを空き領域として追加する。
			This::AddEmptyField(
				this->EmptyFields,
				LocalEmptyPosition + InBitWidth,
				LocalEmptyWidth - InBitWidth);
		}
		return LocalEmptyPosition;
	}

	/// @brief 状態値に使うビット領域を追加する。
	/// @return 追加したビット領域のビット位置。
	/// 失敗した場合は This::INVALID_BIT_POSITION を返す。
	typename This::BitPosition AllocateBitField(
		/// [in] 追加するビット領域のビット幅。
		typename This::BitWidth const InBitWidth)
	{
		if (InBitWidth <= 0 || This::BLOCK_BIT_WIDTH < InBitWidth)
		{
			check(false);
			return This::INVALID_BIT_POSITION;
		}

		// 新たにビット列を追加する。
		auto const LocalPosition(this->GetBlockCount() * This::BLOCK_BIT_WIDTH);
		if (This::INVALID_BIT_POSITION <= LocalPosition)
		{
			// ビット位置の最大値を超過した。
			check(false);
			return This::INVALID_BIT_POSITION;
		}
		auto const LocalAddBlockWidth(
			(InBitWidth + This::BLOCK_BIT_WIDTH - 1) / This::BLOCK_BIT_WIDTH);
		this->BitBlocks.Emplace(LocalAddBlockWidth, 0);

		// 余りを空きビット領域に追加する。
		auto const LocalAddWidth(LocalAddBlockWidth * This::BLOCK_BIT_WIDTH);
		if (InBitWidth < LocalAddWidth)
		{
			This::AddEmptyField(
				this->EmptyFields,
				LocalPosition + InBitWidth,
				LocalAddWidth - InBitWidth);
		}
		return static_cast<typename This::BitPosition>(LocalPosition);
	}

	/// @brief 空きビット領域を追加する。
	static void AddEmptyField(
		/// [in,out] 空きビット領域情報を追加するコンテナ。
		typename This::EmptyFieldArray& OutEmptyFields,
		/// [in] 追加する空きビット領域のビット位置。
		typename This::BitPosition const InBitPosition,
		/// [in] 追加する空きビット領域のビット幅。
		std::size_t const InBitWidth)
	{
		typedef typename This::EmptyFieldArray::ElementType EmptyField;
		EmptyField const LocalEmptyField(
			static_cast<typename EmptyField::first_type>(InBitWidth),
			static_cast<typename FmptyField::second_type>(InBitPosition));
		if (InBitPosition == LocalEmptyField.second
			&& InBitWidth == LocalEmptyField.first)
		{
			auto const LocalPosition(
				std::lower_bound(
					begin(OutEmptyFields),
					end(OutEmptyFields),
					LocalEmptyField));
			OutEmptyFields.Insert(LocalPosition.GetIndex(), LocalEmptyField);
		}
		else
		{
			check(false);
		}
	}

	//-------------------------------------------------------------------------
	public:
	/// @brief ビット列のコンテナ。
	typename This::BitBlockArray BitBlocks;
	/// @brief 空きビット領域情報のコンテナ。
	typename This::EmptyFieldArray EmptyFields;

}; // class Psyque::RuleEngine::_private::TStatusChunk

// vim: set noexpandtab:
