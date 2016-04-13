// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RulesEngine::TStatusBuilder
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#include "../String/NumericParser.h"

/// @cond
namespace Psyque
{
	namespace RulesEngine
	{
		class TStatusBuilder;
	} // namespace RulesEngine
} // namespace Psyque
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief UDataTable から状態値を構築する関数オブジェクト。
/// @details  TDriver::ExtendChunk の引数として使う。
class Psyque::RulesEngine::TStatusBuilder
{
	/// @brief this が指す値の型。
	private: using ThisClass = TStatusBuilder;

	//-------------------------------------------------------------------------
	/// @brief 中間表現を解析して状態値を構築し、状態貯蔵器へ登録する。
	/// @return 登録した状態値の数。
	public: template<
		typename TemplateReservoir,
		typename TemplateHasher,
		typename TemplateIntermediation>
	uint32 operator()(
		/// [in,out] 状態値を登録する TDriver::FReservoir 。
		TemplateReservoir& OutReservoir,
		/// [in,out] 文字列からハッシュ値を作る TDriver::FHasher インスタンス。
		TemplateHasher const& InHashFunction,
		/// [in] 状態値を登録するチャンクの名前ハッシュ値。
		typename TemplateReservoir::FChunkKey const InChunkKey,
		/// [in] 解析する中間表現。
		TemplateIntermediation const& InIntermediation)
	const
	{
		return ThisClass::RegisterStatuses(
			OutReservoir, InHashFunction, InChunkKey, InIntermediation);
	}

	/// @brief UDataTable を解析して状態値を構築し、状態貯蔵器へ登録する。
	/// @return 登録した状態値の数。
	public: template<typename TemplateReservoir, typename TemplateHasher>
	static uint32 RegisterStatuses(
		/// [in,out] 状態値を登録する TDriver::FReservoir インスタンス。
		TemplateReservoir& OutReservoir,
		/// [in] 文字列からハッシュ値を作る TDriver::FHasher インスタンス。
		TemplateHasher const& InHashFunction,
		/// [in] 状態値を登録するチャンクの名前ハッシュ値。
		typename TemplateReservoir::FChunkKey const InChunkKey,
		/// [in] 登録する状態値のもととなる
		/// FPsyqueRulesStatusTableRow で構成される UDataTable 。
		UDataTable const& InStatusTable)
	{
		FString const LocalContextName(
			TEXT("PsyqueRulesPlugin/StatusBuilder::RegisterStatuses"));
		auto const LocalRowNames(InStatusTable.GetRowNames());
		uint32 LocalCount(0);
		for (auto& LocalRowName: LocalRowNames)
		{
			auto const LocalRow(
				InStatusTable.FindRow<FPsyqueRulesStatusTableRow>(
					LocalRowName, LocalContextName));
			LocalCount += LocalRow != nullptr
				&& ThisClass::RegisterStatus(
					OutReservoir,
					InHashFunction,
					InChunkKey,
					InHashFunction(LocalRowName),
					*LocalRow);
		}
		return LocalCount;
	}

	/// @brief JSONを解析して状態値を構築し、状態貯蔵器へ登録する。
	/// @return 登録した状態値の数。
	/// @todo 実装途中
	public: template<typename TemplateReservoir, typename TemplateHasher>
	static uint32 RegisterStatuses(
		/// [in,out] 状態値を登録する TDriver::FReservoir インスタンス。
		TemplateReservoir& OutReservoir,
		/// [in] 文字列からハッシュ値を作る TDriver::FHasher インスタンス。
		TemplateHasher const& InHashFunction,
		/// [in] 状態値を登録するチャンクの名前ハッシュ値。
		typename TemplateReservoir::FChunkKey const InChunkKey,
		/// [in] 登録する状態値が記述されているJSON値の配列。
		TArray<TSharedPtr<FJsonValue>> const& InJsonArray)
	{
		uint32 LocalCount(0);
		for (auto const& LocalJsonValue: InJsonArray)
		{
			// 下位要素が要素数2以上の配列か判定する。
			auto const LocalRow(LocalJsonValue.Get());
			if (LocalRow == nullptr || LocalRow->Type != EJson::Array)
			{
				check(LocalRow != nullptr);
				continue;
			}
			auto const& LocalColumns(LocalRow->AsArray());
			if (LocalColumns.Num() < 2)
			{
				continue;
			}

			// 下位配列の要素#0が状態値の名前文字列として取り出す。
			auto const LocalStatusKey(LocalColumns[0].Get());
			if (LocalStatusKey == nullptr
				|| LocalStatusKey->Type != EJson::String)
			{
				check(LocalStatusKey != nullptr);
				continue;
			}

			// 下位配列の要素#1を状態値の初期値として取り出す。
			auto const LocalStatusValue(LocalColumns[1].Get());
			if (LocalStatusValue != nullptr)
			{
				LocalCount += ThisClass::RegisterStatus(
					OutReservoir,
					InHashFunction,
					InChunkKey,
					LocalStatusKey->AsString(),
					*LocalStatusValue,
					LocalColumns.Num() < 3? nullptr: LocalColumns[2].Get());
			}
			else {check(false);}
		}
		return LocalCount;
	}

	//-------------------------------------------------------------------------
	/// @brief データテーブル行を解析して状態値を構築し、状態貯蔵器へ登録する。
	private: template<typename TemplateReservoir, typename TemplateHasher>
	static bool RegisterStatus(
		/// [in,out] 状態値を登録する TDriver::FReservoir インスタンス。
		TemplateReservoir& OutReservoir,
		/// [in] 文字列からハッシュ値を作る TDriver::FHasher インスタンス。
		TemplateHasher const& InHashFunction,
		/// [in] 状態値を登録するチャンクの名前ハッシュ値。
		typename TemplateReservoir::FChunkKey const InChunkKey,
		/// [in] 状態値の名前ハッシュ値。
		typename TemplateReservoir::FStatusKey const InStatusKey,
		/// [in] 状態値のもととなるデータテーブル行。
		FPsyqueRulesStatusTableRow const& InStatus)
	{
		int32 LocalParseEnd;
		Psyque::String::FNumericParser const LocalNumber(
			&LocalParseEnd, InStatus.InitialValue);
		if (LocalParseEnd != InStatus.InitialValue.Len())
		{
			UE_LOG(
				LogPsyqueRulesEngine,
				Warning,
				TEXT(
					"TStatusBuilder::RegisterStatus is failed."
					"\n\t'%s'[%d] is invalid in status key '%s'."),
				*InStatus.InitialValue,
				LocalParseEnd,
				*_private::_find_key_string(InStatusKey));
			return false;
		}

		// 論理型の状態値を登録する。
		auto const LocalBool(LocalNumber.GetBool());
		if (LocalBool != nullptr)
		{
			return OutReservoir.RegisterStatus(
				InChunkKey, InStatusKey, *LocalBool);
		}
		else if (0 < InStatus.BitWidth)
		{
			// 符号なし整数型の状態値を登録する。
			auto const LocalUnsigned(LocalNumber.GetUnsigned());
			if (LocalUnsigned != nullptr)
			{
				return OutReservoir.RegisterStatus(
					InChunkKey,
					InStatusKey,
					*LocalUnsigned,
					static_cast<uint32>(InStatus.BitWidth));
			}
		}
		else if (InStatus.BitWidth < 0)
		{
			// 符号あり整数型の状態値を登録する。
			auto const LocalUnsigned(LocalNumber.GetUnsigned());
			if (LocalUnsigned != nullptr)
			{
				if (MAX_int64 < *LocalUnsigned)
				{
					UE_LOG(
						LogPsyqueRulesEngine,
						Warning,
						TEXT(
							"TStatusBuilder::RegisterStatus is failed."
							"\n\tSigned status '%s' = %llu"
							" is greater than MAX_int64."),
						*_private::_find_key_string(InStatusKey),
						*LocalUnsigned);
					return false;
				}
				return OutReservoir.RegisterStatus(
					InChunkKey,
					InStatusKey,
					static_cast<int64>(*LocalUnsigned),
					static_cast<uint32>(-InStatus.BitWidth));
			}
			auto const LocalNegative(LocalNumber.GetNegative());
			if (LocalNegative != nullptr)
			{
				return OutReservoir.RegisterStatus(
					InChunkKey,
					InStatusKey,
					*LocalNegative,
					static_cast<uint32>(-InStatus.BitWidth));
			}
		}
		else
		{
			// 浮動小数点数型の状態値を登録する。
			using FFloat = typename TemplateReservoir::FStatusValue::FFloat;
			auto const LocalFloat(LocalNumber.GetFloat());
			if (LocalFloat != nullptr)
			{
				return OutReservoir.RegisterStatus(
					InChunkKey,
					InStatusKey,
					static_cast<FFloat>(*LocalFloat));
			}
			auto const LocalUnsigned(LocalNumber.GetUnsigned());
			if (LocalUnsigned != nullptr)
			{
				return OutReservoir.RegisterStatus(
					InChunkKey,
					InStatusKey,
					static_cast<FFloat>(*LocalUnsigned));
			}
			auto const LocalNegative(LocalNumber.GetUnsigned());
			if (LocalNegative != nullptr)
			{
				return OutReservoir.RegisterStatus(
					InChunkKey,
					InStatusKey,
					static_cast<FFloat>(*LocalNegative));
			}
		}
		UE_LOG(
			LogPsyqueRulesEngine,
			Warning,
			TEXT(
				"TStatusBuilder::RegisterStatus is failed."
				"\n\tInStatus.InitialValue '%s' is empty in status key '%s'."),
			*InStatus.InitialValue,
			*_private::_find_key_string(InStatusKey));
		return false;
	}

	/// @brief JSONを解析して状態値を構築し、状態貯蔵器へ登録する。
	private: template<typename TemplateReservoir, typename TemplateHasher>
	static bool RegisterStatus(
		/// [in,out] 状態値を登録する TDriver::FReservoir インスタンス。
		TemplateReservoir& OutReservoir,
		/// [in] 文字列からハッシュ値を作る TDriver::FHasher インスタンス。
		TemplateHasher const& InHashFunction,
		/// [in] 状態値を登録するチャンクの識別値。
		typename TemplateReservoir::FChunkKey const InChunkKey,
		/// [in] 状態値の識別値に使う文字列。
		FString const& InStatusKey,
		/// [in] 状態値の初期値を持つJSON値。
		FJsonValue const& InStatusValue,
		/// [in] 状態値のビット幅を持つJSON値。
		FJsonValue const* const InStatusBitWidth)
	{
		switch (InStatusValue.Type)
		{
			// 論理型の状態値を登録する。
			case EJson::Boolean:
			return OutReservoir.RegisterStatus(
				InChunkKey,
				InHashFunction(FName(*InStatusKey)),
				InStatusValue.AsBool());
			break;

			case EJson::Number:
			// ビット幅があるなら、整数型の状態値を登録する。
			if (InStatusBitWidth != nullptr)
			{
				// 状態値のビット幅を取り出す。
				int64 LocalBitWidth;
				if (!ThisClass::ExtractInteger(LocalBitWidth, *InStatusBitWidth))
				{
					return false;
				}

				// 初期値となる整数を取り出す。
				int64 LocalInteger;
				if (!ThisClass::ExtractInteger(LocalInteger, InStatusValue))
				{
					return false;
				}
				auto const LocalStatusKey(InHashFunction(FName(*InStatusKey)));
				return LocalBitWidth < 0?
					OutReservoir.RegisterStatus(
						InChunkKey,
						LocalStatusKey,
						LocalInteger,
						static_cast<uint32>(-LocalBitWidth)):
					OutReservoir.RegisterStatus(
						InChunkKey,
						LocalStatusKey,
						static_cast<uint64>(LocalInteger),
						static_cast<uint32>(LocalBitWidth));
			}

			// 浮動小数点数型の状態値を登録する。
			return OutReservoir.RegisterStatus(
				InChunkKey,
				InHashFunction(FName(*InStatusKey)),
				InStatusValue.AsNumber());

			/// @todo 状態値の初期値を、文字列から数値へ変換できるようにしたい。
			case EJson::String:
			check(false);
			break;

			default: break;
		}
		return false;
	}

	/// @brief JSON値から整数値を取り出す。
	private: static bool ExtractInteger(
		/// [out] 取り出した整数値の格納先。
		int64& OutInteger,
		/// [in] 整数値を取り出すJSON値を指すポインタ。
		FJsonValue const& InJsonValue)
	{
		if (InJsonValue.Type == EJson::Number)
		{
			auto const LocalFloat(InJsonValue.AsNumber());
			OutInteger = static_cast<int64>(LocalFloat);
			if (LocalFloat == OutInteger)
			{
				return true;
			}
		}
		return false;
	}

}; // class Psyque::RulesEngine::TStatusBuilder

// vim: set noexpandtab:
