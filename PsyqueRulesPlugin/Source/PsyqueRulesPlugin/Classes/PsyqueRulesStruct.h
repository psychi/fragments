// Copyright (c) 2016, Hillco Psychi, All rights reserved.

#pragma once

#include <memory>
#include "Runtime/Engine/Classes/Engine/DataTable.h"
#include "PsyqueRulesStruct.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogPsyqueRulesEngine, Log, All);

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
//-----------------------------------------------------------------------------
/// @brief Kleeneの3値論理による真理値。
/// @details http://goo.gl/0kyClO
UENUM(BlueprintType)
enum class EPsyqueKleene: uint8
{
	/// 偽。
	TernaryFalse = 0 UMETA(DisplayName="TernaryFalse"),

	/// 真。
	TernaryTrue = 1 UMETA(DisplayName="TernaryTrue"),

	/// 真か偽か不明。
	TernaryUnknown = 128 UMETA(DisplayName="TernaryUnknown")

}; // enum class EPsyqueKleene

static_assert(
	static_cast<int8>(EPsyqueKleene::TernaryFalse) == false
	&& static_cast<int8>(EPsyqueKleene::TernaryTrue) == true
	&& static_cast<int8>(EPsyqueKleene::TernaryUnknown) < 0,
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
	Nonblock UMETA(DisplayName="Nonblock"),

}; // enum class EPsyqueAccumulationDelay

//----------------------------------------------------------------------------
/// @brief 状態値を代入する演算子の種類を表す列挙型。
UENUM(BlueprintType)
enum class EPsyqueStatusAssignment: uint8
{
	/// 単純代入。
	Copy = 0 UMETA(DisplayName="="),

	/// 加算代入。
	Add = 1 UMETA(DisplayName="+="),

	/// 減算代入。
	Sub = 2 UMETA(DisplayName="-="),

	/// 乗算代入。
	Mul = 3 UMETA(DisplayName="*="),

	/// 除算代入。
	Div = 4 UMETA(DisplayName="/="),

	/// 除算の余りの代入。
	Mod = 5 UMETA(DisplayName="%="),

	/// 論理和の代入。
	Or = 6 UMETA(DisplayName="|="),

	/// 排他的論理和の代入。
	Xor = 7 UMETA(DisplayName="^="),

	/// 論理積の代入。
	And = 8 UMETA(DisplayName="&="),

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
	Signed = 65 UMETA(DisplayName="Signed"),

	/// @cond
	EPsyqueRulesStatusKind_Max = 127
	/// @endcond

}; // enum class EPsyqueRulesStatusKind

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
};

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
};

// vim: set noexpandtab:
