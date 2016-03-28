// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RuleEngine::_private::TStatusProperty
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

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
/// @copybrief Psyque::RuleEngine::_private::TReservoir::FStatusProperty
/// @tparam TemplateChunkKey    @copydoc TStatusProperty::FChunkKey
/// @tparam TemplateBitPosition @copydoc TStatusProperty::FBitPosition
/// @tparam TemplateFormat      @copydoc TStatusProperty::FFormat
template<
	typename TemplateChunkKey,
	typename TemplateBitPosition,
	typename TemplateFormat>
class Psyque::RuleEngine::_private::TStatusProperty
{
	private: using This = TStatusProperty; ///< @copydoc TReservoir::This

	//-------------------------------------------------------------------------
	/// @brief 状態値が格納されているビット領域チャンクの識別値を表す型。
	public: using FChunkKey = TemplateChunkKey;
	/// @brief 状態値が格納されているビット位置を表す型。
	public: using FBitPosition = TemplateBitPosition;
	static_assert(
		std::is_integral<TemplateBitPosition>::value,
		"TemplateBitPosition is not integer type.");
	/// @brief 状態値のビット構成を表す型。
	public: using FFormat = TemplateFormat;
	static_assert(
		std::is_integral<TemplateFormat>::value,
		"'TemplateFormat' is not integer type.");

	//-------------------------------------------------------------------------
	/// @brief 状態値プロパティを構築する。
	public: PSYQUE_CONSTEXPR TStatusProperty(
		/// [in] This::ChunkKey の初期値。
		typename This::FChunkKey const InChunkKey,
		/// [in] This::BitPosition の初期値。
		typename This::FBitPosition const InBitPosition,
		/// [in] This::Format の初期値。
		typename This::FFormat const InFormat)
	PSYQUE_NOEXCEPT:
	ChunkKey(InChunkKey),
	BitPosition(InBitPosition),
	Format(InFormat),
	Transition(true)
	{}

	//-------------------------------------------------------------------------
	/// @name 状態値のプロパティ
	/// @{

	/// @brief 状態値のプロパティが空か判定する。
	/// @retval true  *this は空。
	/// @retval false *this は空ではない。
	public: PSYQUE_CONSTEXPR bool IsEmpty() const PSYQUE_NOEXCEPT
	{
		return this->Format == 0;
	}

	/// @brief 状態値が格納されているビット列チャンクの識別値を取得する。
	/// @return @copydoc This::ChunkKey
	public: PSYQUE_CONSTEXPR typename This::FChunkKey const GetChunkKey()
	const PSYQUE_NOEXCEPT
	{
		return this->ChunkKey;
	}

	/// @brief 状態値のビット位置を取得する。
	/// @return 状態値のビット位置。
	public: PSYQUE_CONSTEXPR typename This::FBitPosition GetBitPosition()
	const PSYQUE_NOEXCEPT
	{
		return this->BitPosition;
	}

	/// @brief 状態値のビット構成を取得する。
	/// @return 状態値のビット構成。
	public: PSYQUE_CONSTEXPR typename This::FFormat GetFormat()
	const PSYQUE_NOEXCEPT
	{
		return this->Format;
	}

	/// @brief 状態変化フラグを取得する。
	/// @return 状態変化フラグ。
	public: PSYQUE_CONSTEXPR bool GetTransition() const PSYQUE_NOEXCEPT
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
	private: typename This::FChunkKey ChunkKey;
	/// @brief 状態値が格納されているビット領域のビット位置。
	private: typename This::FBitPosition BitPosition;
	/// @brief 状態値のビット構成。
	private: typename This::FFormat Format;
	/// @brief 状態変化フラグ。
	private: bool Transition;

}; // class Psyque::RuleEngine::_private::TStatusProperty

// vim: set noexpandtab:
