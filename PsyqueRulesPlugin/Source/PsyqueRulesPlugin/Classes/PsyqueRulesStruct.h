// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief UPsyqueRulesEngine で使う、列挙型と構造体の定義。
/// @author Hillco Psychi (https://twitter.com/psychi)

#pragma once

#include <memory>
#include "PsyqueRulesStruct.generated.h"

/// @cond
DECLARE_LOG_CATEGORY_EXTERN(LogPsyqueRulesEngine, Log, All);
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
//-----------------------------------------------------------------------------
/// @brief Kleeneの3値論理による真理値。
/// @details http://goo.gl/0kyClO
UENUM(BlueprintType)
enum class EPsyqueKleene: uint8
{
	/// 偽。
	IsFalse = 0 UMETA(DisplayName="False"),

	/// 真。
	IsTrue = 1 UMETA(DisplayName="True"),

	/// 真か偽か不明。
	Unknown = 128 UMETA(DisplayName="Unknown")

}; // enum class EPsyqueKleene

static_assert(
	static_cast<int8>(EPsyqueKleene::IsFalse) == false
	&& static_cast<int8>(EPsyqueKleene::IsTrue) == true
	&& static_cast<int8>(EPsyqueKleene::Unknown) < 0,
	"");

//-----------------------------------------------------------------------------
/// @brief 条件式の論理項を結合する、論理演算子を表す列挙型。
/// @details Psyque::RulesEngine::TDriver::FEvaluator::FExpression::Logic
///   の型として使う。
UENUM(BlueprintType)
enum class EPsyqueRulesExpressionLogic: uint8
{
	/// 条件式の論理項を、論理積で結合する。
	And = 0 UMETA(DisplayName="and"),

	/// 条件式の論理項を、論理和で結合する。
	Or = 1 UMETA(DisplayName="or"),

}; // enum class EPsyqueRulesExpressionLogic

//-----------------------------------------------------------------------------
/// @brief 条件式の種類を表す列挙型。
/// @details Psyque::RulesEngine::TDriver::FEvaluator::FExpression::Kind
///   の型として使う。
UENUM(BlueprintType)
enum class EPsyqueRulesExpressionKind: uint8
{
	/// 状態値を比較する条件式。
	/// 1つの論理項は、3つの要素で構成される。
	/// - 左辺となる状態値の識別子。
	/// - 左辺と右辺を結合する比較演算子。
	/// - 右辺となる定数値。または右辺となる状態値の識別子。
	StatusComparison = 0 UMETA(DisplayName="StatusComparison"),

	/// 状態値の変化を検知する条件式。
	/// 1つの論理項は、1つの要素で構成される。
	/// - 変化を検知する状態値の識別子。
	StatusTransition = 1 UMETA(DisplayName="StatusTransition"),

	/// 複数の条件式を結合する条件式。
	/// 1つの論理項は、2つの要素で構成される。
	/// - 結合する条件式の識別子。
	/// - 条件となる真偽値。
	SubExpression = 2 UMETA(DisplayName="SubExpression"),

}; // enum class EPsyqueRulesExpressionKind

//---------------------------------------------------------------------
/// @brief 状態変更の予約系列と遅延方法を表す列挙型。
/// @details
///   1度の Psyque::RulesEngine::_private::TAccumulator::_flush
///   で状態変更を適用する際に、
///   1つの状態値に対して異なる予約系列から複数回の状態変更がある場合の、
///   2回目以降の状態変更の遅延方法を決める。
///   EPsyqueAccumulationDelay::Follow と
///   EPsyque:AccumulationDelay::Yield の使用を推奨する。
UENUM(BlueprintType)
enum class EPsyqueAccumulationDelay: uint8
{
	/// 予約系列を切り替えず、
	/// 直前の状態変更の予約と同じタイミングで状態変更を適用する。
	Follow = 0 UMETA(DisplayName="Follow"),

	/// 予約系列を切り替える。1度の _private::TAccumulator::_flush
	/// で、対象となる状態値が既に変更されていた場合、
	/// 同じ予約系列の状態変更の適用が次回以降の
	/// _private::TAccumulator::_flush まで遅延する。
	Yield = 1 UMETA(DisplayName="Yield"),

	/// 予約系列を切り替える。1度の _private::TAccumulator::_flush
	/// で、対象となる状態値が既に変更されていた場合、
	/// 以後にある全ての状態変更の適用が次回以降の
	/// _private::TAccumulator::_flush まで遅延する。
	/// @warning
	///   EAccumulationDelay::Block を
	///   _private::TAccumulator::Accumulate に何度も渡すと、
	///   状態変更の予約がどんどん蓄積する場合があるので、注意すること。
	Block = 2 UMETA(DisplayName="Block"),

	/// 予約系列を切り替える。1度の _private::TAccumulator::_flush
	/// で、対象となる状態値が既に変更されていた場合でも、
	/// 遅延せずに状態変更を適用する。
	/// @warning
	///   EAccumulationDelay::Nonblock を
	///   _private::TAccumulator::Accumulate に渡すと、
	///   それ以前の状態変更が無視されることになるので、注意すること。
	Nonblock = 3 UMETA(DisplayName="Nonblock"),

}; // enum class EPsyqueAccumulationDelay

//----------------------------------------------------------------------------
/// @brief 状態値を代入する演算子の種類を表す列挙型。
UENUM(BlueprintType)
enum class EPsyqueStatusAssignment: uint8
{
	/// 単純代入。
	Copy = 0 UMETA(DisplayName="Copy"),

	/// 加算代入。論理型の値は、加算代入できない。
	Add = 1 UMETA(DisplayName="Add"),

	/// 減算代入。論理型の値は、加算代入できない。
	Sub = 2 UMETA(DisplayName="Sub"),

	/// 乗算代入。論理型の値は、乗算代入できない。
	Mul = 3 UMETA(DisplayName="Mul"),

	/// 除算代入。論理型の値は、除算代入できない。
	Div = 4 UMETA(DisplayName="Div"),

	/// 除算の余りの代入。論理型の値は、除算の余りの代入ができない。
	Mod = 5 UMETA(DisplayName="Mod"),

	/// 論理和の代入。浮動小数点数型の値は、論理和の代入ができない。
	Or = 6 UMETA(DisplayName="Or"),

	/// 排他的論理和の代入。浮動小数点数型の値は、排他的論理和の代入ができない。
	Xor = 7 UMETA(DisplayName="Xor"),

	/// 論理積の代入。浮動小数点数型の値は、論理積の代入ができない。
	And = 8 UMETA(DisplayName="And"),

}; // enum class EPsyqueStatusAssignment

//---------------------------------------------------------------------
/// @brief 状態値の型の種類を表す列挙型。
UENUM(BlueprintType)
enum class EPsyqueRulesStatusKind: uint8
{
	/// 空。
	Empty = 0 UMETA(DisplayName="Empty"),

	/// 論理値。
	Bool = 1 UMETA(DisplayName="Bool"),

	/// 浮動小数点数。
	Float = 2 UMETA(DisplayName="Float"),

	/// 符号なし整数。
	Unsigned = 3 UMETA(DisplayName="Unsigned"),

	/// 符号あり整数。
	Signed = 66 UMETA(DisplayName="Signed"),

	/// @cond
	EPsyqueRulesStatusKind_Max = 129
	/// @endcond

}; // enum class EPsyqueRulesStatusKind

//---------------------------------------------------------------------
/// @brief 単位条件。 THandler::FCondition を構成する単位となる条件。
/// @details THandler::MakeCondition で条件を作る引数として使う。
UENUM(BlueprintType)
enum class EPsyqueRulesUnitCondition: uint8
{
	/// 無効な条件。
	Invalid = 0 UMETA(DisplayName="Invalid"),

	/// 条件式が評価不能であることが条件。
	Unknown = 1 UMETA(DisplayName="Unknown"),

	/// 条件式の評価が偽であることが条件。
	IsFalse = 2 UMETA(DisplayName="False"),

	/// 条件式の評価が真以外であることが条件。
	NotTrue = 3 UMETA(DisplayName="NotTrue"),

	/// 条件式の評価が真であることが条件。
	IsTrue = 4 UMETA(DisplayName="True"),

	/// 条件式の評価が偽以外であることが条件。
	NotFalse = 5 UMETA(DisplayName="NotFalse"),

	/// 条件式が評価可能であることが条件。
	Known = 6 UMETA(DisplayName="Known"),

	/// 条件式の評価を問わない。
	Any = 7 UMETA(DisplayName="Any"),

}; // enum class EPsyqueRulesUnitCondition

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 状態値が記述されている UDataTable の行。
USTRUCT(BlueprintType)
struct FPsyqueRulesStatusTableRow: public FTableRowBase
{
	GENERATED_USTRUCT_BODY()
 
	/// @brief 状態値の初期値のもととなる文字列。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PsyqueRulesPlugin")
	FString InitialValue;

	/// @brief 状態値が整数型の場合のビット幅。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PsyqueRulesPlugin")
	int32 BitWidth;

}; // struct FPsyqueRulesStatusTableRow

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 条件式が記述されている UDataTable の行。
USTRUCT(BlueprintType)
struct FPsyqueRulesExpressionTableRow: public FTableRowBase
{
	GENERATED_USTRUCT_BODY()
 
	/// @brief 条件式の種類。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PsyqueRulesPlugin")
	EPsyqueRulesExpressionKind Kind;

	/// @brief 条件式が持つ論理項を結合する論理演算子。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PsyqueRulesPlugin")
	EPsyqueRulesExpressionLogic Logic;

	/// @brief 条件式が持つ論理項の要素。
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="PsyqueRulesPlugin")
	TArray<FString> Elements;

}; // FPsyqueRulesExpressionTableRow

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @class FPsyqueRulesBehaviorDelegate
/// @brief 条件挙動で実行するデリゲートを表す型。
/// @par
///   - 引数#0は、評価された条件式の名前ハッシュ値。
///   - 引数#1は、条件式の今回の評価結果。
///   - 引数#2は、条件式の前回の評価結果。
/// @cond
DECLARE_DELEGATE_ThreeParams(
	FPsyqueRulesBehaviorDelegate,
	int32 const,
	EPsyqueKleene const,
	EPsyqueKleene const);
/// @endcond

/// @class FPsyqueRulesBehaviorDynamicDelegate
/// @brief 条件挙動で実行する動的デリゲートを表す型。
/// @par
///   - 引数#0は、評価された条件式の名前ハッシュ値。
///   - 引数#1は、条件式の今回の評価結果。
///   - 引数#2は、条件式の前回の評価結果。
/// @note マクロ引数の記述を1行に収めないと、UE4.11以前ではビルド時にエラーが出る。
/// @cond
DECLARE_DYNAMIC_DELEGATE_ThreeParams(
	FPsyqueRulesBehaviorDynamicDelegate, int32 const, InExpressionKey, EPsyqueKleene const, InNowEvaluation, EPsyqueKleene const, InLastEvaluation);
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
UCLASS()
class UPsyqueRulesFunctionLibrary: public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

	//-------------------------------------------------------------------------
	public:
	/// @brief 単位条件を合成して挙動条件を作る。
	/// @warning
	///   条件式の評価が最新と前回で同じ場合は、
	///   TExpressionMonitor::CacheHandlers で挙動条件の判定が行われない。
	///   このため、以下の単位条件の組み合わせは無効となることに注意。
	///   @code
	///     MakeCondition(EPsyqueRulesUnitCondition::Unknown, EPsyqueRulesUnitCondition::Unknown);
	///     MakeCondition(EPsyqueRulesUnitCondition::IsFalse, EPsyqueRulesUnitCondition::IsFalse);
	///     MakeCondition(EPsyqueRulesUnitCondition::IsTrue, EPsyqueRulesUnitCondition::IsTrue);
	///   @endcode
	/// @return
	///   関数が呼び出される挙動条件。単位条件の組み合わせが無効な場合は
	///   EPsyqueRulesUnitCondition::Invalid を返す。
	/// @param InNowCondition  条件式の、最新の評価の単位条件。
	/// @param InLastCondition 条件式の、前回の評価の単位条件。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	static uint8 MakeCondition(
		EPsyqueRulesUnitCondition const InNowCondition,
		EPsyqueRulesUnitCondition const InLastCondition)
	{
		return ThisClass::MixUnitCondition(
			InNowCondition != EPsyqueRulesUnitCondition::Invalid
				&& InLastCondition != EPsyqueRulesUnitCondition::Invalid
				&& (InNowCondition != InLastCondition
					// 2のべき乗か判定する。
					|| 0 != (
						static_cast<uint8>(InNowCondition)
							& (static_cast<uint8>(InNowCondition) - 1))),
			InNowCondition,
			InLastCondition);
	}

	/// @brief 条件式の評価を合成して挙動条件を作る。
	/// @warning
	///   条件式の評価が最新と前回で同じ場合は、
	///   TExpressionMonitor::CacheHandlers で挙動条件の判定が行われない。
	///   このため、以下の評価の組み合わせは無効となることに注意。
	///   @code
	///     MakeCondition(EPsyqueKleene::IsTrue, EPsyqueKleene::IsTrue);
	///     MakeCondition(EPsyqueKleene::IsFalse, EPsyqueKleene::IsFalse);
	///     MakeCondition(EPsyqueKleene::Unknown, EPsyqueKleene::Unknown);
	///   @endcode
	/// @return
	///   関数が呼び出される挙動条件。評価の組み合わせが無効な場合は
	///   EPsyqueRulesUnitCondition::Invalid を返す。
	/// @param InNowEvaluation  条件となる、条件式の最新の評価。
	/// @param InLastEvaluation 条件となる、条件式の前回の評価。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	static uint8 MakeConditionFromKleene(
		EPsyqueKleene const InNowEvaluation,
		EPsyqueKleene const InLastEvaluation)
	{
		auto const LocalNowCondition(
			ThisClass::MakeUnitCondition(InNowEvaluation));
		auto const LocalLastCondition(
			ThisClass::MakeUnitCondition(InLastEvaluation));
		return ThisClass::MixUnitCondition(
			LocalNowCondition != LocalLastCondition,
			LocalNowCondition,
			LocalLastCondition);
	}

	/// @brief 条件式の評価から、単位条件を作る。
	/// @param InEvaluation 条件式の評価。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	static EPsyqueRulesUnitCondition MakeUnitCondition(
		EPsyqueKleene const InEvaluation)
	{
		return InEvaluation == EPsyqueKleene::IsTrue?
			EPsyqueRulesUnitCondition::IsTrue:
			InEvaluation == EPsyqueKleene::IsFalse?
				EPsyqueRulesUnitCondition::IsFalse:
				EPsyqueRulesUnitCondition::Unknown;
	}

	//-------------------------------------------------------------------------
	private:
	/// @brief 単位条件を合成して挙動条件を作る。
	/// @return 挙動条件。
	static uint8 MixUnitCondition(
		/// [in] 合成可能かどうか。
		bool const InMix,
		/// [in] 条件となる、最新の条件式の評価。
		EPsyqueRulesUnitCondition const InNowCondition,
		/// [in] 条件となる、前回の条件式の評価。
		EPsyqueRulesUnitCondition const InLastCondition)
	{
		enum: uint8 {UnitConditionBitWidth = 3}; // 単位条件に使うビット幅。
		return InMix?
			static_cast<uint8>(InNowCondition) | (
				static_cast<uint8>(InLastCondition) << UnitConditionBitWidth):
			static_cast<uint8>(EPsyqueRulesUnitCondition::Invalid);
	}

}; // class UPsyqueRulesFunctionLibrary

// vim: set noexpandtab:
