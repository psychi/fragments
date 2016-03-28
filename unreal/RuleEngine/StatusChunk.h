// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RuleEngine::_private::TStatusChunk
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#include <vector>
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
/// @copybrief Psyque::RuleEngine::_private::TReservoir::FStatusChunk
/// @tparam TemplateBitBlock    @copydoc TStatusChunk::FBitBlock
/// @tparam TemplateBitPosition @copydoc TStatusChunk::FBitPosition
/// @tparam TemplateBitWidth    @copydoc TStatusChunk::FBitWidth
/// @tparam TemplateAllocator   @copydoc TStatusChunk::FAllocator
template<
	typename TemplateBitBlock,
	typename TemplateBitPosition,
	typename TemplateBitWidth,
	typename TemplateAllocator>
class Psyque::RuleEngine::_private::TStatusChunk
{
	private: using This = TStatusChunk; ///< @copydoc TReservoir::This

	//-------------------------------------------------------------------------
	/// @brief ビット列の単位を表す型。
	public: using FBitBlock = TemplateBitBlock;
	static_assert(
		std::is_unsigned<TemplateBitBlock>::value,
		"TemplateBitBlock is not unsigned integer type.");
	/// @brief ビット列のビット位置を表す型。
	public: using FBitPosition = TemplateBitPosition;
	static_assert(
		std::is_unsigned<TemplateBitPosition>::value,
		"TemplateBitPosition is not unsigned integer type.");
	/// @brief ビット列のビット幅を表す型。
	public: using FBitWidth = TemplateBitWidth;
	static_assert(
		std::is_unsigned<TemplateBitWidth>::value,
		"TemplateBitWidth is not unsigned integer type.");
	/// @brief コンテナに用いるメモリ割当子の型。
	public: using FAllocator = TemplateAllocator;
	/// @copydoc This::BitBlocks
	public: using FBitBlockArray = std::vector<
		typename This::FBitBlock, typename This::FAllocator>;
	/// @copybrief This::EmptyBitsets
	/// @details
	/// - first は、空きビット領域のビット幅。
	/// - second は、空きビット領域のビット位置。
	public: using FEmptyBitsetArray = std::vector<
		std::pair<typename This::FBitWidth, typename This::FBitPosition>,
		typename This::FAllocator>;
	public: enum: typename This::FBitPosition
	{
		/// @brief 無効なビット位置。
		INVALID_BIT_POSITION = ~static_cast<typename This::FBitPosition>(0),
	};
	public: enum: typename This::FBitWidth
	{
		/// @brief ビット列単位のビット数。
		BLOCK_BIT_WIDTH = static_cast<typename This::FBitWidth>(
			sizeof(typename This::FBitBlock) * CHAR_BIT),
	};
	static_assert(
		// This::BLOCK_BIT_WIDTH が This::FBitWidth に収まるのを確認する。
		This::BLOCK_BIT_WIDTH < (
			1 << (sizeof(This::FBitWidth) * CHAR_BIT - 1)),
		"This::BLOCK_BIT_WIDTH is overflow.");

	//-------------------------------------------------------------------------
	/// @brief 空のビット領域チャンクを構築する。
	public: explicit TStatusChunk(
		/// [in] コンテナが使うメモリ割当子の初期値。
		typename This::FAllocator const& InAllocator):
	BitBlocks(InAllocator),
	EmptyBitsets(InAllocator)
	{}

#ifdef PSYQUE_NO_STD_DEFAULTED_FUNCTION
	/// @brief ムーブ構築子。
	public: TStatusChunk(
		/// [in,out] ムーブ元となるインスタンス。
		This&& OutSource):
	BitBlocks(MoveTemp(OutSource.BitBlocks)),
	EmptyBitsets(MoveTemp(OutSource.EmptyBitsets))
	{}

	/// @brief ムーブ代入演算子。
	/// @return *this
	public: This& operator=(
		/// [in,out] ムーブ元となるインスタンス。
		This&& OutSource)
	{
		this->BitBlocks = MoveTemp(OutSource.BitBlocks);
		this->EmptyBitsets = MoveTemp(OutSource.EmptyBitsets);
		return *this;
	}
#endif // defined(PSYQUE_NO_STD_DEFAULTED_FUNCTION)

	/// @brief 状態値を格納するビット領域を生成する。
	/// @return
	/// 生成したビット領域のビット位置。
	/// 失敗した場合は This::INVALID_BIT_POSITION を返す。
	public: typename This::FBitPosition AllocateBitset(
		/// [in] 生成するビット領域のビット数。
		typename This::FBitWidth const InBitWidth)
	{
		// 状態値を格納できるビット領域を、空きビット領域から取得する。
		auto const local_empty_field(
			std::lower_bound(
				this->EmptyBitsets.begin(),
				this->EmptyBitsets.end(),
				typename This::FEmptyBitsetArray::value_type(
					InBitWidth, 0)));
		return local_empty_field != this->EmptyBitsets.end()?
			// 既存の空き領域を再利用する。
			this->reuse_empty_field(InBitWidth, local_empty_field):
			// 適切な空き領域がないので、新たな領域を追加する。
			this->AddBitset(InBitWidth);
	}

	/// @brief ビット領域の値を取得する。
	/// @return ビット領域の値。該当するビット領域がない場合は~0を返す。
	public: typename This::FBitBlock GetBitset(
		/// [in] 値を取得するビット領域のビット位置。
		std::size_t const InBitPosition,
		/// [in] 値を取得するビット領域のビット幅。
		std::size_t const InBitWidth)
	const PSYQUE_NOEXCEPT
	{
		if (This::BLOCK_BIT_WIDTH < InBitWidth)
		{
			check(false);
			return ~static_cast<typename This::FBitBlock>(0);
		}
		auto const LocalBlockIndex(
			InBitPosition / This::BLOCK_BIT_WIDTH);
		if (this->BitBlocks.size() <= LocalBlockIndex)
		{
			check(false);
			return ~static_cast<typename This::FBitBlock>(0);
		}

		// ビット列ブロックでのビット位置を決定し、値を取り出す。
		return Psyque::GetBitset(
			this->BitBlocks.at(LocalBlockIndex),
			InBitPosition - LocalBlockIndex * This::BLOCK_BIT_WIDTH,
			InBitWidth);
	}

	/// @brief ビット領域に値を設定する。
	/// @retval 正 元とは異なる値を設定した。
	/// @retval 0  元と同じ値を設定した。
	/// @retval 負 失敗。値を設定できなかった。
	public: int8 SetBitset(
		/// [in] 値を設定するビット領域のビット位置。
		std::size_t const InBitPosition,
		/// [in] 値を設定するビット領域のビット幅。
		std::size_t const InBitWidth,
		/// [in] ビット領域に設定する値。
		typename This::FBitBlock const& InValue)
	PSYQUE_NOEXCEPT
	{
		if (Psyque::ShiftRightBitwise(InValue, InBitWidth) != 0)
		{
			return -1;
		}
		auto const LocalBlockIndex(
			InBitPosition / This::BLOCK_BIT_WIDTH);
		if (this->BitBlocks.size() <= LocalBlockIndex)
		{
			check(false);
			return -1;
		}

		// ビット列単位でのビット位置を決定し、値を埋め込む。
		auto& LocalBlock(this->BitBlocks.at(LocalBlockIndex));
		auto const local_last_block(LocalBlock);
		LocalBlock = Psyque::SetBitset(
			LocalBlock,
			InBitPosition - LocalBlockIndex * This::BLOCK_BIT_WIDTH,
			InBitWidth,
			InValue);
		return local_last_block != LocalBlock;
	}

	//-------------------------------------------------------------------------
	/// @brief 空きビット領域を再利用する。
	/// @return 再利用したビット領域のビット位置。
	private: typename This::FBitPosition reuse_empty_field(
		/// [in] 再利用したい領域のビット幅。
		typename This::FBitPosition const InBitWidth,
		/// [in] 再利用する空きビット領域のプロパティを指す反復子。
		typename This::FEmptyBitsetArray::iterator const in_empty_field)
	{
		// 既存の空き領域を再利用する。
		auto const LocalEmptyPosition(in_empty_field->second);
		auto const local_empty_width(in_empty_field->first);
		check(0 < local_empty_width);

		// 再利用する空き領域を削除する。
		this->EmptyBitsets.erase(in_empty_field);
		if (InBitWidth < local_empty_width)
		{
			// 余りを空き領域として追加する。
			This::add_empty_field(
				this->EmptyBitsets,
				LocalEmptyPosition + InBitWidth,
				local_empty_width - InBitWidth);
		}
		return LocalEmptyPosition;
	}

	/// @brief 状態値に使うビット領域を追加する。
	/// @return 追加したビット領域のビット位置。
	/// 失敗した場合は This::INVALID_BIT_POSITION を返す。
	private: typename This::FBitPosition AddBitset(
		/// [in] 追加するビット領域のビット幅。
		typename This::FBitWidth const InBitWidth)
	{
		if (InBitWidth <= 0 || This::BLOCK_BIT_WIDTH < InBitWidth)
		{
			check(false);
			return This::INVALID_BIT_POSITION;
		}

		// 新たにビット列を追加する。
		auto const local_position(
			this->BitBlocks.size() * This::BLOCK_BIT_WIDTH);
		if (This::INVALID_BIT_POSITION <= local_position)
		{
			// ビット位置の最大値を超過した。
			check(false);
			return This::INVALID_BIT_POSITION;
		}
		auto const local_add_block_width(
			(InBitWidth + This::BLOCK_BIT_WIDTH - 1)
			/ This::BLOCK_BIT_WIDTH);
		this->BitBlocks.insert(
			this->BitBlocks.end(), local_add_block_width, 0);

		// 余りを空きビット領域に追加する。
		auto const local_add_width(
			local_add_block_width * This::BLOCK_BIT_WIDTH);
		if (InBitWidth < local_add_width)
		{
			This::add_empty_field(
				this->EmptyBitsets,
				local_position + InBitWidth,
				local_add_width - InBitWidth);
		}
		return static_cast<typename This::FBitPosition>(local_position);
	}

	/// @brief 空きビット領域を追加する。
	private: static void add_empty_field(
		/// [in,out] 空きビット領域情報を追加するコンテナ。
		typename This::FEmptyBitsetArray& OutEmptyBitsets,
		/// [in] 追加する空きビット領域のビット位置。
		std::size_t const InBitPosition,
		/// [in] 追加する空きビット領域のビット幅。
		std::size_t const InBitWidth)
	{
		using FEmptyBitset = typename This::FEmptyBitsetArray::value_type;
		FEmptyBitset const local_empty_field(
			static_cast<typename FEmptyBitset::first_type>(InBitWidth),
			static_cast<typename FEmptyBitset::second_type>(InBitPosition));
		if (InBitPosition == local_empty_field.second
			&& InBitWidth == local_empty_field.first)
		{
			OutEmptyBitsets.insert(
				std::lower_bound(
					OutEmptyBitsets.begin(),
					OutEmptyBitsets.end(),
					local_empty_field),
				local_empty_field);
		}
		else
		{
			check(false);
		}
	}

	//-------------------------------------------------------------------------
	/// @brief ビット列のコンテナ。
	public: typename This::FBitBlockArray BitBlocks;
	/// @brief 空きビット領域情報のコンテナ。
	public: typename This::FEmptyBitsetArray EmptyBitsets;

}; // class Psyque::RuleEngine::_private::TStatusChunk

// vim: set noexpandtab:
