// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RuleEngine::_private::TStatusProperty
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#include "../BitAlgorithm.h"

/// @cond
namespace Psyque
{
	namespace RuleEngine
	{
		namespace _private
		{
			template<typename, typename, typename> class TStatusProperty;

		} // namespace _private
	} // namespace RuleEngine
} // namespace Psyque
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 状態値が格納されているビット領域を指す型。
/// @tparam TemplateChunkKey    @copydoc TStatusProperty::ChunkKeyType
/// @tparam TemplateBitPosition @copydoc TStatusProperty::BitPositionType
/// @tparam TemplateFormat      @copydoc TStatusProperty::FormatType
template<
	typename TemplateChunkKey,
	typename TemplateBitPosition,
	typename TemplateFormat>
class Psyque::RuleEngine::_private::TStatusProperty
{
	/// @brief this が指す値の型。
	using This = TStatusProperty;

	//-------------------------------------------------------------------------
	public:
	/// @brief 状態値が格納されているビット領域チャンクの識別値を表す型。
	using ChunkKeyType = TemplateChunkKey;
	/// @brief 状態値が格納されているビット位置を表す型。
	using BitPositionType = TemplateBitPosition;
	static_assert(
		std::is_integral<TemplateBitPosition>::value,
		"TemplateBitPosition is not integer type.");
	/// @brief 状態値のビット構成を表す型。
	using FormatType = TemplateFormat;
	static_assert(
		std::is_integral<TemplateFormat>::value,
		"'TemplateFormat' is not integer type.");

	//-------------------------------------------------------------------------
	public:
	/// @brief 状態値プロパティを構築する。
	PSYQUE_CONSTEXPR TStatusProperty(
		/// [in] This::ChunkKey の初期値。
		typename This::ChunkKeyType const InChunkKey,
		/// [in] This::BitPosition の初期値。
		typename This::BitPositionType const InBitPosition,
		/// [in] This::Format の初期値。
		typename This::FormatType const InFormat)
	PSYQUE_NOEXCEPT:
	ChunkKey(InChunkKey),
	BitPosition(InBitPosition),
	Format(InFormat),
	bHasTransited(true)
	{}

	//-------------------------------------------------------------------------
	/// @name 状態値のプロパティ
	/// @{
	public:
	/// @brief 状態値のプロパティが空か判定する。
	/// @retval true  *this は空。
	/// @retval false *this は空ではない。
	PSYQUE_CONSTEXPR bool IsEmpty() const PSYQUE_NOEXCEPT
	{
		return this->Format == 0;
	}

	/// @brief 状態値が格納されているビット列チャンクの識別値を取得する。
	/// @return @copydoc This::ChunkKey
	PSYQUE_CONSTEXPR typename This::ChunkKeyType const GetChunkKey()
	const PSYQUE_NOEXCEPT
	{
		return this->ChunkKey;
	}

	/// @brief 状態値のビット位置を取得する。
	/// @return 状態値のビット位置。
	PSYQUE_CONSTEXPR typename This::BitPositionType GetBitPosition()
	const PSYQUE_NOEXCEPT
	{
		return this->BitPosition;
	}

	/// @brief 状態値のビット構成を取得する。
	/// @return 状態値のビット構成。
	PSYQUE_CONSTEXPR typename This::FormatType GetFormat() const PSYQUE_NOEXCEPT
	{
		return this->Format;
	}

	/// @brief 状態変化フラグを取得する。
	/// @return 状態変化フラグ。
	PSYQUE_CONSTEXPR bool HasTransited() const PSYQUE_NOEXCEPT
	{
		return this->bHasTransited;
	}

	void Transit(bool const InTransition) PSYQUE_NOEXCEPT
	{
		this->bHasTransited = InTransition;
	}
	/// @}
	//-------------------------------------------------------------------------
	private:
	/// @brief 状態値が格納されているビット領域チャンクの識別値。
	typename This::ChunkKeyType ChunkKey;
	/// @brief 状態値が格納されているビット領域のビット位置。
	typename This::BitPositionType BitPosition;
	/// @brief 状態値のビット構成。
	typename This::FormatType Format;
	/// @brief 状態変化フラグ。
	bool bHasTransited;

}; // class Psyque::RuleEngine::_private::TStatusProperty

// vim: set noexpandtab:
