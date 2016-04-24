// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RulesEngine::_private::TStatusChunk
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#include <vector>
#include "../BitAlgorithm.h"

/// @cond
namespace Psyque
{
	namespace RulesEngine
	{
		namespace _private
		{
			template<typename, typename, typename, typename> class TStatusChunk;
		} // namespace _private
	} // namespace RulesEngine
} // namespace Psyque
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @copybrief Psyque::RulesEngine::_private::TReservoir::FStatusChunk
/// @tparam TemplateBitBlock    @copydoc TStatusChunk::FBitBlock
/// @tparam TemplateBitPosition @copydoc TStatusChunk::FBitPosition
/// @tparam TemplateBitWidth    @copydoc TStatusChunk::FBitWidth
/// @tparam TemplateAllocator   @copydoc TStatusChunk::FAllocator
template<
	typename TemplateBitBlock,
	typename TemplateBitPosition,
	typename TemplateBitWidth,
	typename TemplateAllocator>
class Psyque::RulesEngine::_private::TStatusChunk
{
	private: using ThisClass = TStatusChunk; ///< @copydoc TReservoir::ThisClass

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
	/// @copydoc ThisClass::BitBlocks
	public: using FBitBlockArray = std::vector<
		typename ThisClass::FBitBlock, typename ThisClass::FAllocator>;
	/// @copybrief ThisClass::EmptyBitsets
	/// @details
	/// - first は、空きビット領域のビット幅。
	/// - second は、空きビット領域のビット位置。
	public: using FEmptyBitsetArray = std::vector<
		std::pair<typename ThisClass::FBitWidth, typename ThisClass::FBitPosition>,
		typename ThisClass::FAllocator>;
	public: enum: typename ThisClass::FBitPosition
	{
		/// @brief 無効なビット位置。
		INVALID_BIT_POSITION = ~static_cast<typename ThisClass::FBitPosition>(0),
	};
	public: enum: typename ThisClass::FBitWidth
	{
		/// @brief ビット列単位のビット数。
		BLOCK_BIT_WIDTH = static_cast<typename ThisClass::FBitWidth>(
			sizeof(typename ThisClass::FBitBlock) * CHAR_BIT),
	};
	static_assert(
		// ThisClass::BLOCK_BIT_WIDTH が ThisClass::FBitWidth に収まるのを確認する。
		ThisClass::BLOCK_BIT_WIDTH < (
			1 << (sizeof(ThisClass::FBitWidth) * CHAR_BIT - 1)),
		"ThisClass::BLOCK_BIT_WIDTH is overflow.");

	//-------------------------------------------------------------------------
	/// @brief 空のビット領域チャンクを構築する。
	public: explicit TStatusChunk(
		/// [in] コンテナが使うメモリ割当子の初期値。
		typename ThisClass::FAllocator const& InAllocator):
	BitBlocks(InAllocator),
	EmptyBitsets(InAllocator)
	{}

#ifdef PSYQUE_NO_STD_DEFAULTED_FUNCTION
	/// @brief ムーブ構築子。
	public: TStatusChunk(
		/// [in,out] ムーブ元となるインスタンス。
		ThisClass&& OutSource):
	BitBlocks(MoveTemp(OutSource.BitBlocks)),
	EmptyBitsets(MoveTemp(OutSource.EmptyBitsets))
	{}

	/// @brief ムーブ代入演算子。
	/// @return *this
	public: ThisClass& operator=(
		/// [in,out] ムーブ元となるインスタンス。
		ThisClass&& OutSource)
	{
		this->BitBlocks = MoveTemp(OutSource.BitBlocks);
		this->EmptyBitsets = MoveTemp(OutSource.EmptyBitsets);
		return *this;
	}
#endif // defined(PSYQUE_NO_STD_DEFAULTED_FUNCTION)

	/// @brief 状態値を格納するビット領域を生成する。
	/// @return
	/// 生成したビット領域のビット位置。
	/// 失敗した場合は ThisClass::INVALID_BIT_POSITION を返す。
	public: typename ThisClass::FBitPosition AllocateBitset(
		/// [in] 生成するビット領域のビット数。
		typename ThisClass::FBitWidth const InBitWidth)
	{
		// 状態値を格納できるビット領域を、空きビット領域から取得する。
		auto const LocalEmptyBitset(
			std::lower_bound(
				this->EmptyBitsets.begin(),
				this->EmptyBitsets.end(),
				typename ThisClass::FEmptyBitsetArray::value_type(
					InBitWidth, 0)));
		return LocalEmptyBitset != this->EmptyBitsets.end()?
			// 既存の空き領域を再利用する。
			this->RecycleBitset(InBitWidth, LocalEmptyBitset):
			// 適切な空き領域がないので、新たな領域を追加する。
			this->AddBitset(InBitWidth);
	}

	/// @brief ビット領域の値を取得する。
	/// @return ビット領域の値。該当するビット領域がない場合は~0を返す。
	public: typename ThisClass::FBitBlock GetBitset(
		/// [in] 値を取得するビット領域のビット位置。
		std::size_t const InBitPosition,
		/// [in] 値を取得するビット領域のビット幅。
		std::size_t const InBitWidth)
	const PSYQUE_NOEXCEPT
	{
		if (ThisClass::BLOCK_BIT_WIDTH < InBitWidth)
		{
			check(false);
			return ~static_cast<typename ThisClass::FBitBlock>(0);
		}
		auto const LocalBlockIndex(
			InBitPosition / ThisClass::BLOCK_BIT_WIDTH);
		if (this->BitBlocks.size() <= LocalBlockIndex)
		{
			check(false);
			return ~static_cast<typename ThisClass::FBitBlock>(0);
		}

		// ビット列ブロックでのビット位置を決定し、値を取り出す。
		return Psyque::GetBitset(
			this->BitBlocks.at(LocalBlockIndex),
			InBitPosition - LocalBlockIndex * ThisClass::BLOCK_BIT_WIDTH,
			InBitWidth);
	}

	/// @brief ビット領域に値を設定する。
	/// @retval 正 元とは異なる値を設定した。
	/// @retval 0  元と同じ値を設定した。
	/// @retval 負 失敗。値を設定できなかった。
	public: EPsyqueKleene SetBitset(
		/// [in] 値を設定するビット領域のビット位置。
		std::size_t const InBitPosition,
		/// [in] 値を設定するビット領域のビット幅。
		std::size_t const InBitWidth,
		/// [in] ビット領域に設定する値。
		typename ThisClass::FBitBlock const& InValue)
	PSYQUE_NOEXCEPT
	{
		if (Psyque::ShiftRightBitwise(InValue, InBitWidth) != 0)
		{
			return EPsyqueKleene::Unknown;
		}
		auto const LocalBlockIndex(
			InBitPosition / ThisClass::BLOCK_BIT_WIDTH);
		if (this->BitBlocks.size() <= LocalBlockIndex)
		{
			check(false);
			return EPsyqueKleene::Unknown;
		}

		// ビット列単位でのビット位置を決定し、値を埋め込む。
		auto& LocalBlock(this->BitBlocks.at(LocalBlockIndex));
		auto const LocalLastBlock(LocalBlock);
		LocalBlock = Psyque::SetBitset(
			LocalBlock,
			InBitPosition - LocalBlockIndex * ThisClass::BLOCK_BIT_WIDTH,
			InBitWidth,
			InValue);
		return static_cast<EPsyqueKleene>(LocalLastBlock != LocalBlock);
	}

	//-------------------------------------------------------------------------
	/// @brief 空きビット領域を再利用する。
	/// @return 再利用したビット領域のビット位置。
	private: typename ThisClass::FBitPosition RecycleBitset(
		/// [in] 再利用したい領域のビット幅。
		typename ThisClass::FBitPosition const InBitWidth,
		/// [in] 再利用する空きビット領域のプロパティを指す反復子。
		typename ThisClass::FEmptyBitsetArray::iterator const InEmptyBitset)
	{
		// 既存の空き領域を再利用する。
		auto const LocalEmptyPosition(InEmptyBitset->second);
		auto const LocalEmptyBitWidth(InEmptyBitset->first);
		check(0 < LocalEmptyBitWidth);

		// 再利用する空き領域を削除する。
		this->EmptyBitsets.erase(InEmptyBitset);
		if (InBitWidth < LocalEmptyBitWidth)
		{
			// 余りを空き領域として追加する。
			ThisClass::AddEmptyBitset(
				this->EmptyBitsets,
				LocalEmptyPosition + InBitWidth,
				LocalEmptyBitWidth - InBitWidth);
		}
		return LocalEmptyPosition;
	}

	/// @brief 状態値に使うビット領域を追加する。
	/// @return 追加したビット領域のビット位置。
	/// 失敗した場合は ThisClass::INVALID_BIT_POSITION を返す。
	private: typename ThisClass::FBitPosition AddBitset(
		/// [in] 追加するビット領域のビット幅。
		typename ThisClass::FBitWidth const InBitWidth)
	{
		if (InBitWidth <= 0 || ThisClass::BLOCK_BIT_WIDTH < InBitWidth)
		{
			check(false);
			return ThisClass::INVALID_BIT_POSITION;
		}

		// 新たにビット列を追加する。
		auto const LocalBitPosition(
			this->BitBlocks.size() * ThisClass::BLOCK_BIT_WIDTH);
		if (ThisClass::INVALID_BIT_POSITION <= LocalBitPosition)
		{
			// ビット位置の最大値を超過した。
			check(false);
			return ThisClass::INVALID_BIT_POSITION;
		}
		auto const LocalAddBlockCount(
			(InBitWidth + ThisClass::BLOCK_BIT_WIDTH - 1) / ThisClass::BLOCK_BIT_WIDTH);
		this->BitBlocks.insert(this->BitBlocks.end(), LocalAddBlockCount, 0);

		// 余りを空きビット領域に追加する。
		auto const LocalAddBitWidth(
			LocalAddBlockCount * ThisClass::BLOCK_BIT_WIDTH);
		if (InBitWidth < LocalAddBitWidth)
		{
			ThisClass::AddEmptyBitset(
				this->EmptyBitsets,
				LocalBitPosition + InBitWidth,
				LocalAddBitWidth - InBitWidth);
		}
		return static_cast<typename ThisClass::FBitPosition>(LocalBitPosition);
	}

	/// @brief 空きビット領域を追加する。
	private: static void AddEmptyBitset(
		/// [in,out] 空きビット領域情報を追加するコンテナ。
		typename ThisClass::FEmptyBitsetArray& OutEmptyBitsets,
		/// [in] 追加する空きビット領域のビット位置。
		std::size_t const InBitPosition,
		/// [in] 追加する空きビット領域のビット幅。
		std::size_t const InBitWidth)
	{
		using FEmptyBitset = typename ThisClass::FEmptyBitsetArray::value_type;
		FEmptyBitset const LocalEmptyBitset(
			static_cast<typename FEmptyBitset::first_type>(InBitWidth),
			static_cast<typename FEmptyBitset::second_type>(InBitPosition));
		if (InBitPosition == LocalEmptyBitset.second
			&& InBitWidth == LocalEmptyBitset.first)
		{
			OutEmptyBitsets.insert(
				std::lower_bound(
					OutEmptyBitsets.begin(),
					OutEmptyBitsets.end(),
					LocalEmptyBitset),
				LocalEmptyBitset);
		}
		else
		{
			check(false);
		}
	}

	//-------------------------------------------------------------------------
	/// @brief ビット列のコンテナ。
	public: typename ThisClass::FBitBlockArray BitBlocks;
	/// @brief 空きビット領域情報のコンテナ。
	public: typename ThisClass::FEmptyBitsetArray EmptyBitsets;

}; // class Psyque::RulesEngine::_private::TStatusChunk

// vim: set noexpandtab:
