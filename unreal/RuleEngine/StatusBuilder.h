// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RuleEngine::TStatusBuilder
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#include "Dom/JsonValue.h"
#include "Serialization/JsonSerializer.h"
//#include "../string/numeric_parser.h"

/// @brief 文字列表で、状態値の識別値が記述されている属性の名前。
/// @details
/// Psyque::RuleEngine::TStatusBuilder で解析する文字列表で、
/// Psyque::RuleEngine::TDriver::FReservoir::FStatusKey
/// の初期値として解析する属性の名前。
#ifndef PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_COLUMN_KEY
#define PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_COLUMN_KEY "KEY"
#endif // !defined(PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_COLUMN_KEY)

/// @brief 文字列表で、状態値の種別が記述されている属性の名前。
/// @details
/// Psyque::RuleEngine::TStatusBuilder で解析する文字列表で、
/// Psyque::RuleEngine::TDriver::FReservoir::FStatusValue::EKind
/// の初期値として解析する属性の名前。
#ifndef PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_COLUMN_KIND
#define PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_COLUMN_KIND "KIND"
#endif // !defined(PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_COLUMN_KIND)

/// @brief 文字列表で、状態値の初期値が記述されている属性の名前。
/// @details
/// Psyque::RuleEngine::TStatusBuilder で解析する文字列表で、
/// Psyque::RuleEngine::TDriver::FReservoir::FStatusValue
/// の初期値として解析する属性の名前。
#ifndef PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_COLUMN_VALUE
#define PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_COLUMN_VALUE "VALUE"
#endif // !defined(PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_COLUMN_VALUE)

/// @brief 文字列表で、状態値の種別が論理型と対応する文字列。
/// @details
/// Psyque::RuleEngine::TStatusBuilder で解析する文字列表で、
/// Psyque::RuleEngine::TDriver::FReservoir::FStatusValue::EKind::Bool
/// として解析する属性の名前。
#ifndef PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_KIND_BOOL
#define PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_KIND_BOOL "BOOL"
#endif // !defined(PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_KIND_BOOL)

/// @brief 文字列表で、状態値の種別が符号なし整数型と対応する文字列。
/// @details
/// Psyque::RuleEngine::TStatusBuilder で解析する文字列表で、
/// Psyque::RuleEngine::TDriver::FReservoir::FStatusValue::EKind::Unsigned
/// として解析する属性の名前。
#ifndef PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_KIND_UNSIGNED
#define PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_KIND_UNSIGNED "UNSIGNED"
#endif // !defined(PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_KIND_UNSIGNED)

/// @brief 文字列表で、状態値の種別が符号あり整数型と対応する文字列。
/// @details
/// Psyque::RuleEngine::TStatusBuilder で解析する文字列表で、
/// Psyque::RuleEngine::TDriver::FReservoir::FStatusValue::EKind::Signed
/// として解析する属性の名前。
#ifndef PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_KIND_SIGNED
#define PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_KIND_SIGNED "SIGNED"
#endif // !defined(PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_KIND_SIGNED)

/// @brief 文字列表で、状態値の種別が浮動小数点数型と対応する文字列。
/// @details
/// Psyque::RuleEngine::TStatusBuilder で解析する文字列表で、
/// Psyque::RuleEngine::TDriver::FReservoir::FStatusValue::EKind::Float
/// として解析する属性の名前。
#ifndef PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_KIND_FLOAT
#define PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_KIND_FLOAT "FLOAT"
#endif // !defined(PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_KIND_FLOAT)

/// 整数型のデフォルトのビット幅。
#ifndef PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_INTEGER_WIDTH_DEFAULT
#define PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_INTEGER_WIDTH_DEFAULT 8
#endif // !defined(PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_INTEGER_WIDTH_DEFAULT)

/// @cond
namespace Psyque
{
	namespace RuleEngine
	{
		class TStatusBuilder;
	} // namespace RuleEngine
} // namespace Psyque
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 文字列表から状態値を構築する関数オブジェクト。
/// @details  TDriver::ExtendChunk の引数として使う。
class Psyque::RuleEngine::TStatusBuilder
{
	/// @brief this が指す値の型。
	private: using This = TStatusBuilder;

	//-------------------------------------------------------------------------
	/// @brief 状態値の文字列表で使う属性。
	private: template<typename TemplateRelationTable>
	class FTableAttribute
	{
		public: explicit FTableAttribute(
			TemplateRelationTable const& InTable)
		PSYQUE_NOEXCEPT:
		Key(
			InTable.FindAttribute(
				PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_COLUMN_KEY)),
		Kind(
			InTable.FindAttribute(
				PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_COLUMN_KIND)),
		Value(
			InTable.FindAttribute(
				PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_COLUMN_VALUE))
		{}

		public: bool IsValid() const PSYQUE_NOEXCEPT
		{
			return 1 <= this->Key.second
				&& 1 <= this->Kind.second
				&& 1 <= this->Value.second;
		}

		/// @brief 文字列表で状態値の識別値が記述されている属性の列番号と列数。
		public: typename TemplateRelationTable::FAttribute Key;
		/// @brief 文字列表で状態値の種別が記述されている属性の列番号と列数。
		public: typename TemplateRelationTable::FAttribute Kind;
		/// @brief 文字列表で状態値の初期値が記述されている属性の列番号と列数。
		public: typename TemplateRelationTable::FAttribute Value;

	}; // class FTableAttribute

	//-------------------------------------------------------------------------
	/// @brief 中間表現を解析して状態値を構築し、状態貯蔵器へ登録する。
	/// @return 登録した状態値の数。
	public: template<
		typename TemplateReservoir,
		typename TemplateHasher,
		typename TemplateIntermediation>
	std::size_t operator()(
		/// [in,out] 状態値を登録する TDriver::FReservoir 。
		TemplateReservoir& OutReservoir,
		/// [in,out] 文字列からハッシュ値を作る TDriver::FHasher インスタンス。
		TemplateHasher const& InHashFunction,
		/// [in] 状態値を登録するチャンクの識別値。
		typename TemplateReservoir::FChunkKey const InChunkKey,
		/// [in] 解析する中間表現。
		TemplateIntermediation const& InIntermediation)
	const
	{
		return This::RegisterStatuses(
			OutReservoir, InHashFunction, InChunkKey, InIntermediation);
	}

	/// @brief JSONを解析して状態値を構築し、状態貯蔵器へ登録する。
	/// @return 登録した状態値の数。
	public: template<
		typename TemplateReservoir,
		typename TemplateHasher,
		typename TemplateChar>
	static std::size_t RegisterStatuses(
		/// [in,out] 状態値を登録する TDriver::FReservoir インスタンス。
		TemplateReservoir& OutReservoir,
		/// [in,out] 文字列からハッシュ値を作る TDriver::FHasher インスタンス。
		TemplateHasher const& InHashFunction,
		/// [in] 状態値を登録するチャンクの識別値。
		typename TemplateReservoir::FChunkKey const InChunkKey,
		/// [in] 状態値が記述されているJSON解析器。
		TSharedRef<TJsonReader<TemplateChar>> const& InJsonReader)
	{
		TSharedPtr<FJsonObject> LocalJsonObject;
		if (!FJsonSerializer::Deserialize(InJsonReader, LocalJsonObject)
			|| !LocalJsonObject.IsValid())
		{
			check(false);
			return false;
		}
		return true;
	}

	public: template<
		typename TemplateReservoir,
		typename TemplateHasher,
		typename TemplateRelationTable>
	static std::size_t Register_Statuses(
		/// [in,out] 状態値を登録する TDriver::FReservoir インスタンス。
		TemplateReservoir& OutReservoir,
		/// [in,out] 文字列からハッシュ値を作る TDriver::FHasher インスタンス。
		TemplateHasher const& InHashFunction,
		/// [in] 状態値を登録するチャンクの識別値。
		typename TemplateReservoir::FChunkKey const InChunkKey,
		/// [in] 状態値が記述されている Psyque::string::TRelationTable 。
		/// 空の場合は、状態値は登録されない。
		TemplateRelationTable const& InTable)
	{
		// 文字列表の属性を取得する。
		This::FTableAttribute<TemplateRelationTable> const LocalAttribute(
			InTable);
		if (!LocalAttribute.IsValid())
		{
			check(InTable.GetCells().empty());
			return 0;
		}

		// 文字列表を行ごとに解析し、状態値を登録する。
		auto const LocalRowCount(InTable.GetRowCount());
		std::size_t LocalRegisterCount(0);
		for (
			typename TemplateRelationTable::FNumber i(0);
			i < LocalRowCount;
			++i)
		{
			if (i != InTable.GetAttributeRow()
				&& This::RegisterStatus(
					OutReservoir,
					InHashFunction,
					InChunkKey,
					InTable,
					i,
					LocalAttribute))
			{
				++LocalRegisterCount;
			}
		}
		return LocalRegisterCount;
	}

	//-------------------------------------------------------------------------
	/// @brief 文字列表を解析して状態値を構築し、状態貯蔵器へ登録する。
	/// @retval true  成功。構築した状態値を状態貯蔵器へ登録した。
	/// @retval false 失敗。状態値は状態貯蔵器へ登録されなかった。
	private: template<
		typename TemplateReservoir,
		typename TemplateHasher,
		typename TemplateRelationTable>
	static bool RegisterStatus(
		/// [in,out] 状態値を登録する状態貯蔵器。
		TemplateReservoir& OutReservoir,
		/// [in,out] 文字列からハッシュ値を作る関数オブジェクト。
		TemplateHasher const& InHashFunction,
		/// [in] 状態値を登録するチャンクの識別値。
		typename TemplateReservoir::FChunkKey const InChunkKey,
		/// [in] 解析する Psyque::string::TRelationTable 。
		TemplateRelationTable const& InTable,
		/// [in] 解析する InTable の行番号。
		typename TemplateRelationTable::FNumber const InRowNumber,
		/// [in] InTable の属性。
		typename This::FTableAttribute<TemplateRelationTable> const& InAttribute)
	{
		// 状態値の識別値を取得する。
		auto const LocalStatusKey(
			InHashFunction(
				InTable.FindCell(InRowNumber, InAttribute.Key.first)));
		if (LocalStatusKey == InHashFunction(
				typename TemplateHasher::argument_type())
			|| 0 < OutReservoir.FindBitWidth(LocalStatusKey))
		{
			// 状態値の識別値が空だったか、重複していた。
			check(false);
			return false;
		}

		// 状態値の型と初期値を取得し、状態値を登録する。
		using FStringView = typename TemplateRelationTable::FString::FView;
		FStringView const LocalKindCell(
			InTable.FindCell(InRowNumber, InAttribute.Kind.first));
		FStringView const LocalValueCell(
			InTable.FindCell(InRowNumber, InAttribute.Value.first));
		if (LocalKindCell == PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_KIND_BOOL)
		{
			// 論理型の状態値を登録する。
			return This::RegisterStatus<bool>(
				OutReservoir, InChunkKey, LocalStatusKey, LocalValueCell);
		}
		if (LocalKindCell == PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_KIND_FLOAT)
		{
			// 浮動小数点数型の状態値を登録する。
			using FFloat = typename TemplateReservoir::FStatusValue::FFloat;
			return This::RegisterStatus<FFloat>(
				OutReservoir, InChunkKey, LocalStatusKey, LocalValueCell);
		}
		auto const local_unsigned_width(
			This::FetchIntegerWidth(
				LocalKindCell,
				FStringView(PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_KIND_UNSIGNED),
				PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_INTEGER_WIDTH_DEFAULT));
		if (0 < local_unsigned_width)
		{
			// 符号なし整数型の状態値を登録する。
			using FUnsigned =
				typename TemplateReservoir::FStatusValue::FUnsigned;
			return This::RegisterStatus<FUnsigned>(
				OutReservoir,
				InChunkKey,
				LocalStatusKey,
				LocalValueCell,
				local_unsigned_width);
		}
		auto const LocalSignedWidth(
			This::FetchIntegerWidth(
				LocalKindCell,
				FStringView(PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_KIND_SIGNED),
				PSYQUE_IF_THEN_ENGINE_STATUS_BUILDER_INTEGER_WIDTH_DEFAULT));
		if (0 < LocalSignedWidth)
		{
			// 符号あり整数型の状態値を登録する。
			using FSigned = typename TemplateReservoir::FStatusValue::FSigned;
			return This::RegisterStatus<FSigned>(
				OutReservoir,
				InChunkKey,
				LocalStatusKey,
				LocalValueCell,
				LocalSignedWidth);
		}

		// 適切な型が見つからなかった。
		check(false);
		return false;
	}

	/// @brief 文字列を解析して状態値を構築し、状態貯蔵器へ登録する。
	/// @retval true  成功。構築した状態値を状態貯蔵器へ登録した。
	/// @retval false 失敗。状態値は状態貯蔵器へ登録されなかった。
	/// @tparam TemplateValue 構築する状態値の型。
	private: template<
		typename TemplateValue,
		typename TemplateReservoir,
		typename TemplateString>
	static bool RegisterStatus(
		/// [in,out] 状態値を登録する状態貯蔵器。
		TemplateReservoir& OutReservoir,
		/// [in] 状態値を登録するチャンクの識別値。
		typename TemplateReservoir::FChunkKey const& InChunkKey,
		/// [in] 登録する状態値に対応する識別値。
		typename TemplateReservoir::FStatusKey const& InStatusKey,
		/// [in] 解析する状態値の文字列。
		TemplateString const& InValueCell)
	{
		Psyque::string::numeric_parser<TemplateValue> const LocalParser(
			InValueCell);
		return LocalParser.IsCompleted()?
			OutReservoir.RegisterStatus(
				InChunkKey, InStatusKey, LocalParser.GetValue()):
			(PSYQUE_ASSERT(false), false);
	}

	/// @brief 文字列を解析して整数型の状態値を構築し、状態貯蔵器へ登録する。
	/// @retval true  成功。構築した状態値を状態貯蔵器へ登録した。
	/// @retval false 失敗。状態値は状態貯蔵器へ登録されなかった。
	private: template<
		typename TemplateValue,
		typename TemplateReservoir,
		typename TemplateString>
	static bool RegisterStatus(
		/// [in,out] 状態値を登録する状態貯蔵器。
		TemplateReservoir& OutReservoir,
		/// [in] 状態値を登録するチャンクの識別値。
		typename TemplateReservoir::FChunkKey const& InChunkKey,
		/// [in] 登録する状態値に対応する識別値。
		typename TemplateReservoir::FStatusKey const& InStatusKey,
		/// [in] 解析する状態値の文字列。
		TemplateString const& InValueCell,
		/// [in] 状態値のビット幅。
		std::size_t const InBitWidth)
	{
		Psyque::string::numeric_parser<TemplateValue> const LocalParser(
			InValueCell);
		return LocalParser.IsCompleted()?
			OutReservoir.RegisterStatus(
				InChunkKey,
				InStatusKey,
				LocalParser.GetValue(),
				InBitWidth):
			(PSYQUE_ASSERT(false), false);
	}

	/// @brief 整数型のビット数を取得する。
	/// @return 整数型のビット数。失敗した場合は0を返す。
	private: template<typename TemplateString>
	static std::size_t FetchIntegerWidth(
		/// [in] セルの文字列。
		TemplateString const& InCell,
		/// [in] 整数型を表す文字列。
		TemplateString const& InKind,
		/// [in] ビット数がない場合のデフォルト値。
		std::size_t const InDefaultSize)
	{
		check(!InKind.empty());
		if (InCell.size() < InKind.size()
			|| InKind != InCell.substr(0, InKind.size()))
		{}
		else if (InKind.size() == InCell.size())
		{
			return InDefaultSize;
		}
		else if (
			InKind.size() + 2 <= InCell.size()
			&& InCell.at(InKind.size()) == '_')
		{
			Psyque::string::numeric_parser<std::size_t> const
				LocalParser(InCell.substr(InKind.size() + 1));
			if (LocalParser.IsCompleted())
			{
				return LocalParser.GetValue();
			}
		}
		return 0;
	}

}; // class Psyque::RuleEngine::TStatusBuilder

// vim: set noexpandtab:
