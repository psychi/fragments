// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RulesEngine::TStatusBuilder
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

/// @cond
namespace Psyque
{
	namespace String
	{
		class FNumericParser;
	}
	namespace RulesEngine
	{
		class TStatusBuilder;
	} // namespace RulesEngine
} // namespace Psyque
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class Psyque::String::FNumericParser
{
	private: using ThisClass = FNumericParser;

	public: enum class EKind: uint8
	{
		Empty,
		Bool,
		Unsigned,
		Negative,
		Float,
	};

	public: template<typename TemplateChar>
	TemplateChar const* Parse(
		TemplateChar const* const InStringBegin,
		TemplateChar const* const InStringEnd)
	{
		// 先頭と末尾の空白文字を取り除く。
		auto const LocalStringBegin(
			ThisClass::TrimFront(InStringBegin, InStringEnd));
		auto const LocalStringEnd(
			ThisClass::TrimFront(InStringBegin, InStringEnd));
		if (LocalStringEnd <= LocalStringBegin)
		{
			return nullptr;
		}

		// 符号を決定する。
		auto LocalCharIterator(LocalStringBegin);
		int8 LocalSign(1);
		switch (*LocalCharIterator)
		{
			case '-': LocalSign = -1;
			// case '+' に続く。

			case '+':
			++LocalCharIterator;
			if (LocalStringEnd <= LocalCharIterator)
			{
				return nullptr;
			}
			break;

			case 't':
			case 'T':
			return nullptr;

			case 'f':
			case 'F':
			return nullptr;

			default: break;
		}

		// 基数を決定する。
		uint8 LocalRadix;
		if (*LocalCharIterator == '.' || (
			'1' <= *LocalCharIterator && *LocalCharIterator <= '9'))
		{
			LocalRadix = 10;
		}
		else if (*LocalCharIterator != '0')
		{
			return nullptr;
		}
		else
		{
			++LocalCharIterator;
			if (LocalStringEnd <= LocalCharIterator)
			{
				this->SetUnsigned(0);
				return LocalCharIterator;
			}

			switch (*LocalCharIterator)
			{
				case 'b': LocalRadix =  2; break;
				case 'x': LocalRadix = 16; break;

				default:
				if (std::isdigit(*LocalCharIterator))
				{
					LocalRadix = 8;
					break;
				}
				return nullptr;
			}
		}

		/// @todo 未実装。
		return nullptr;
	}

	public: void SetUnsigned(uint64 const InUnsigned)
	{
		this->Kind = ThisClass::EKind::Unsigned;
		this->Unsigned = InUnsigned;
	}

	public: template<typename TemplateChar>
	static TemplateChar const* TrimFront(
		TemplateChar const* const InStringBegin,
		TemplateChar const* const InStringEnd)
	{
		if (InStringBegin < InStringEnd)
		{
			check(InStringBegin != nullptr);
			for (auto i(InStringBegin); i < InStringEnd; ++i)
			{
				if (!std::isspace(*i))
				{
					return i;
				}
			}
		}
		return InStringEnd;
	}

	public: template<typename TemplateChar>
	static TemplateChar const* TrimBack(
		TemplateChar const* const InStringBegin,
		TemplateChar const* const InStringEnd)
	{
		if (InStringBegin < InStringEnd)
		{
			check(InStringBegin != nullptr);
			for (auto i(InStringEnd - 1); InStringBegin <= i; --i)
			{
				if (!std::isspace(*i))
				{
					return i + 1;
				}
			}
		}
		return InStringBegin;
	}

	private:
	union
	{
		double Float;
		uint64 Unsigned;
		bool Bool;
	};
	ThisClass::EKind Kind;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 文字列表から状態値を構築する関数オブジェクト。
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
	std::size_t operator()(
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
	static std::size_t RegisterStatuses(
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
		std::size_t LocalCount(0);
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
	public: template<typename TemplateReservoir, typename TemplateHasher>
	static std::size_t RegisterStatuses(
		/// [in,out] 状態値を登録する TDriver::FReservoir インスタンス。
		TemplateReservoir& OutReservoir,
		/// [in] 文字列からハッシュ値を作る TDriver::FHasher インスタンス。
		TemplateHasher const& InHashFunction,
		/// [in] 状態値を登録するチャンクの名前ハッシュ値。
		typename TemplateReservoir::FChunkKey const InChunkKey,
		/// [in] 登録する状態値が記述されているJSON値の配列。
		TArray<TSharedPtr<FJsonValue>> const& InJsonArray)
	{
		std::size_t LocalCount(0);
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
		auto const LocalKleene(Psyque::ParseKleene(InStatus.InitialValue));
		if (LocalKleene != EPsyqueKleene::TernaryUnknown)
		{
			return OutReservoir.RegisterStatus(
				InChunkKey,
				InStatusKey,
				LocalKleene != EPsyqueKleene::TernaryFalse);
		}

		/// @todo 未実装。
		auto const LocalFloat(FCString::Atof(*InStatus.InitialValue));
		auto const LocalInteger(FCString::Atoi(*InStatus.InitialValue));
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
						static_cast<std::size_t>(-LocalBitWidth)):
					OutReservoir.RegisterStatus(
						InChunkKey,
						LocalStatusKey,
						static_cast<uint64>(LocalInteger),
						static_cast<std::size_t>(LocalBitWidth));
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
