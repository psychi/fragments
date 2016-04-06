// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RulesEngine::_private::TStatusProperty
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

/// @cond
namespace Psyque
{
	namespace RulesEngine
	{
		namespace _private
		{
			template<typename, typename, typename> class TStatusProperty;
		} // namespace _private
	} // namespace RulesEngine
} // namespace Psyque
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @copybrief Psyque::RulesEngine::_private::TReservoir::FStatusProperty
/// @tparam TemplateChunkKey    @copydoc TStatusProperty::FChunkKey
/// @tparam TemplateBitPosition @copydoc TStatusProperty::FBitPosition
/// @tparam TemplateBitFormat   @copydoc TStatusProperty::FBitFormat
template<
	typename TemplateChunkKey,
	typename TemplateBitPosition,
	typename TemplateBitFormat>
class Psyque::RulesEngine::_private::TStatusProperty
{
	private: using ThisClass = TStatusProperty; ///< @copydoc TReservoir::ThisClass

	//-------------------------------------------------------------------------
	/// @brief 状態値が格納されているビット領域チャンクの識別値を表す型。
	public: using FChunkKey = TemplateChunkKey;
	/// @brief 状態値が格納されているビット位置を表す型。
	public: using FBitPosition = TemplateBitPosition;
	static_assert(
		std::is_integral<TemplateBitPosition>::value,
		"TemplateBitPosition is not integer type.");
	/// @brief 状態値のビット構成を表す型。
	public: using FBitFormat = TemplateBitFormat;
	static_assert(
		std::is_integral<TemplateBitFormat>::value,
		"'TemplateBitFormat' is not integer type.");

	//-------------------------------------------------------------------------
	/// @brief 状態値プロパティを構築する。
	public: PSYQUE_CONSTEXPR TStatusProperty(
		/// [in] ThisClass::ChunkKey の初期値。
		typename ThisClass::FChunkKey const InChunkKey,
		/// [in] ThisClass::BitPosition の初期値。
		typename ThisClass::FBitPosition const InBitPosition,
		/// [in] ThisClass::BitFormat の初期値。
		typename ThisClass::FBitFormat const InBitFormat)
	PSYQUE_NOEXCEPT:
	ChunkKey(InChunkKey),
	BitPosition(InBitPosition),
	BitFormat(InBitFormat),
	Transition(true)
	{}

	//-------------------------------------------------------------------------
	/// @name 状態値のプロパティ
	/// @{

	/// @brief 状態値のプロパティが空か判定する。
	/// @retval true  *this は空。
	/// @retval false *this は空ではない。
	public: bool PSYQUE_CONSTEXPR IsEmpty() const PSYQUE_NOEXCEPT
	{
		return this->BitFormat == 0;
	}

	/// @brief 状態値が格納されているビット列チャンクの識別値を取得する。
	/// @return @copydoc ThisClass::ChunkKey
	public: typename ThisClass::FChunkKey const PSYQUE_CONSTEXPR GetChunkKey()
	const PSYQUE_NOEXCEPT
	{
		return this->ChunkKey;
	}

	/// @brief 状態値のビット位置を取得する。
	/// @return 状態値のビット位置。
	public: typename ThisClass::FBitPosition PSYQUE_CONSTEXPR GetBitPosition()
	const PSYQUE_NOEXCEPT
	{
		return this->BitPosition;
	}

	/// @brief 状態値のビット構成を取得する。
	/// @return 状態値のビット構成。
	public: typename ThisClass::FBitFormat PSYQUE_CONSTEXPR GetBitFormat()
	const PSYQUE_NOEXCEPT
	{
		return this->BitFormat;
	}

	/// @brief 状態変化フラグを取得する。
	/// @return 状態変化フラグ。
	public: bool PSYQUE_CONSTEXPR GetTransition() const PSYQUE_NOEXCEPT
	{
		return this->Transition;
	}

	public: void SetTransition(bool const InTransition) PSYQUE_NOEXCEPT
	{
		this->Transition = InTransition;
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @brief 状態値が格納されているビット領域チャンクの識別値。
	private: typename ThisClass::FChunkKey ChunkKey;
	/// @brief 状態値が格納されているビット領域のビット位置。
	private: typename ThisClass::FBitPosition BitPosition;
	/// @brief 状態値のビット構成。
	private: typename ThisClass::FBitFormat BitFormat;
	/// @brief 状態変化フラグ。
	private: bool Transition;

}; // class Psyque::RulesEngine::_private::TStatusProperty

// vim: set noexpandtab:
