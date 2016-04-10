// Copyright (c) 2016, Hillco Psychi, All rights reserved.

#pragma once

#include <memory>
#include "Runtime/Engine/Classes/Engine/DataTable.h"
#include "PsyqueRulesEngine.generated.h"

/// @cond
namespace Psyque
{
	namespace RulesEngine
	{
		template<typename, typename, typename, typename, typename>
			class TDriver;
	} // namespace RulesEngine
} // namespace Psyque
/// @endcond

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
};
static_assert(
	static_cast<int8>(EPsyqueKleene::TernaryFalse) == false
	&& static_cast<int8>(EPsyqueKleene::TernaryTrue) == true
	&& static_cast<int8>(EPsyqueKleene::TernaryUnknown) < 0,
	"");

namespace Psyque
{
	inline EPsyqueKleene ParseKleene(FString const& InString)
	{
		if (InString == TEXT("true"))
		{
			return EPsyqueKleene::TernaryTrue;
		}
		if (InString == TEXT("false"))
		{
			return EPsyqueKleene::TernaryFalse;
		}
		return EPsyqueKleene::TernaryUnknown;
	}
}

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
};

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
};

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

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief Blueprintからは、このクラスを介してルールエンジンを操作する。
/// @todo 今のところBlueprintからの操作を前提としているが、
///   Psyque をPublicフォルダに配置し、C++から直接操作できるようにしたい。
UCLASS()
class UPsyqueRulesEngine: public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	//-------------------------------------------------------------------------
	public:
	/// @brief  現在のルールエンジン駆動器を破棄し、新たな駆動器を生成する。
	/// @retval true  成功。ルールエンジン駆動器を生成した。
	/// @retval false 失敗。ルールエンジン駆動器を生成しなかった。
	/// - ルールエンジン駆動器のメモリ割当に失敗すると、失敗する。
	static bool CreateDriver();

	/// @brief 現在のルールエンジン駆動器を破棄する。
	/// @retval true  成功。ルールエンジン駆動器を破棄した。
	/// @retval false 失敗。ルールエンジン駆動器を破棄しなかった。
	/// - ルールエンジン駆動器がすでに破棄されていると、失敗する。
	static bool DestroyDriver();

	/// @brief ルールエンジンを更新する。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	static void Tick();

	/// @brief 名前文字列から名前ハッシュ値を取得する。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	static int32 MakeHash(
		/// [in] 名前文字列となる FName インスタンス。
		FName const& InName);

	//-------------------------------------------------------------------------
	/// @name 状態値の登録
	/// @{
	public:
	/// @brief 論理型の状態値を登録する。
	/// @retval true  成功。状態値を登録した。
	/// @retval false 失敗。状態値は登録されなかった。
	/// - InStatusKey に対応する状態値がすでに登録されていると、失敗する。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	static bool RegisterBoolStatus(
		/// [in] 登録する状態値を格納する状態値ビット列チャンクの名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InChunkKey,
		/// [in] 登録する状態値の名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InStatusKey,
		/// [in] 登録する状態値の初期値。
		bool const InValue);

	/// @brief 符号なし整数型の状態値を登録する。
	/// @note InBitWidth が31より大きく、状態値が31ビット幅より大きい値だと、
	///   UPsyqueRulesEngine::GetUnsignedStatus で状態値の取得に失敗する。
	/// @retval true  成功。状態値を登録した。
	/// @retval false 失敗。状態値は登録されなかった。
	/// - InStatusKey に対応する状態値がすでに登録されていると、失敗する。
	/// - InValue が0未満だと、失敗する。
	/// - InBitWidth が1以下だと、失敗する。ビット幅が1の場合は、論理型として
	///   UPsyqueRulesEngine::RegisterBoolStatus で登録すること。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	static bool RegisterUnsignedStatus(
		/// [in] 登録する状態値を格納する状態値ビット列チャンクの名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InChunkKey,
		/// [in] 登録する状態値の名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InStatusKey,
		/// [in] 登録する状態値の初期値。
		int32 const InValue,
		/// [in] 登録する状態値のビット幅。
		uint8 const InBitWidth);

	/// @brief 符号あり整数型の状態値を登録する。
	/// @note InBitWidth が32より大きく、状態値が32ビット幅より大きい値だと、
	///   UPsyqueRulesEngine::GetSignedStatus で状態値の取得に失敗する。
	/// @retval true  成功。状態値を登録した。
	/// @retval false 失敗。状態値は登録されなかった。
	/// - InStatusKey に対応する状態値がすでに登録されていると、失敗する。
	/// - InValue が UPsyqueRulesEngine::GetIntegerNan と等値だと、失敗する。
	/// - InBitWidth が1以下だと失敗する。ビット幅が1の場合は、論理型として
	///   UPsyqueRulesEngine::RegisterBoolStatus で登録すること。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	static bool RegisterSignedStatus(
		/// [in] 登録する状態値を格納する状態値ビット列チャンクの名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InChunkKey,
		/// [in] 登録する状態値の名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InStatusKey,
		/// [in] 登録する状態値の初期値。
		int32 const InValue,
		/// [in] 登録する状態値のビット幅。
		uint8 const InBitWidth);

	/// @brief 浮動小数点数型の状態値を登録する。
	/// @retval true  成功。状態値を登録した。
	/// @retval false 失敗。状態値は登録されなかった。
	/// - InStatusKey に対応する状態値がすでに登録されていると、失敗する。
	/// - InValue が UPsyqueRulesEngine::GetFloatNan と等値だと、失敗する。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	static bool RegisterFloatStatus(
		/// [in] 登録する状態値を格納する状態値ビット列チャンクの名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InChunkKey,
		/// [in] 登録する状態値の名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InStatusKey,
		/// [in] 登録する状態値の初期値。
		float const InValue);
	/// @}
	//-------------------------------------------------------------------------
	/// @name 状態値の取得
	/// @{
	public:
	/// @brief 論理型の状態値を取得する。
	/// @retval EPsyqueKleene::TernaryTrue  InStatusName に対応する論理型の状態値は真。
	/// @retval EPsyqueKleene::TernaryFalse InStatusName に対応する論理型の状態値は偽。
	/// @retval EPsyqueKleene::TernaryUnknown 失敗。
	/// - InStatusKey に対応する状態値がない場合は、失敗する。
	/// - InStatusKey に対応する状態値が論理型でない場合は、失敗する。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	static EPsyqueKleene GetBoolStatus(
		/// [in] 取得する状態値の名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InStatusKey);

	/// @brief 符号なし整数型の状態値を取得する。
	/// @return InStatusKey に対応する、符号なし整数型の状態値の値。
	/// - ただし戻り値が UPsyqueRulesEngine::GetIntegerNan
	///   と等値の場合は、状態値の取得に失敗した。
	///   - InStatusKey に対応する状態値がない場合は、失敗する。
	///   - InStatusKey に対応する状態値が符号なし整数型でない場合は、失敗する。
	///   - 状態値が31ビット幅より大きい値だと、失敗する。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	static int32 GetUnsignedStatus(
		/// [in] 取得する状態値の名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InStatusKey);

	/// @brief 符号あり整数型の状態値を取得する。
	/// @return InStatusKey に対応する、符号あり整数型の状態値の値。
	/// - ただし戻り値が UPsyqueRulesEngine::GetIntegerNan
	///   と等値の場合は、状態値の取得に失敗した。
	///   - InStatusKey に対応する状態値がない場合は、失敗する。
	///   - InStatusKey に対応する状態値が符号あり整数型でない場合は、失敗する。
	///   - 状態値が32ビット幅より大きい値だと、失敗する。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	static int32 GetSignedStatus(
		/// [in] 取得する状態値の名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InStatusKey);

	/// @brief 浮動小数点数型の状態値を取得する。
	/// @return InStatusKey に対応する、浮動小数点数型の状態値の値。
	/// - ただし戻り値が UPsyqueRulesEngine::GetFloatNan
	///   と等値の場合は、状態値の取得に失敗した。
	///   - InStatusKey に対応する状態値がない場合は、失敗する。
	///   - InStatusKey に対応する状態値が浮動小数点数型でない場合は、失敗する。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	static float GetFloatStatus(
		/// [in] 取得する状態値の名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InStatusKey);
	/// @}
	//-------------------------------------------------------------------------
	/// @name 状態値の設定
	/// @{
	public:
	/// @brief 論理型の状態値へ値を代入する。
	/// @retval true  成功。 InValue を InStatusKey に対応する状態値へ代入した。
	/// @retval false 失敗。状態値は変化しない。
	/// - UPsyqueRulesEngine::RegisterBoolStatus で、
	///   InStatusKey に対応する状態値が登録されてないと、失敗する。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	static bool SetBoolStatus(
		/// [in] 代入先となる状態値の名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InStatusKey,
		/// [in] 状態値へ代入する値。
		bool const InValue);

	/// @brief 符号なし整数型の状態値へ値を代入する。
	/// @retval true  成功。 InValue を InStatusKey に対応する状態値へ代入した。
	/// @retval false 失敗。状態値は変化しない。
	/// - UPsyqueRulesEngine::RegisterUnsignedStatus で、
	///   InStatusKey に対応する状態値が登録されてないと、失敗する。
	/// - InValue の値が
	///   InStatusKey に対応する状態値のビット幅を超えていると、失敗する。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	static bool SetUnsignedStatus(
		/// [in] 代入先となる状態値の名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InStatusKey,
		/// [in] 状態値へ代入する値。
		int32 const InValue);

	/// @brief 符号あり整数型の状態値へ値を代入する。
	/// @retval true  成功。 InValue を InStatusKey に対応する状態値へ代入した。
	/// @retval false 失敗。状態値は変化しない。
	/// - UPsyqueRulesEngine::RegisterSignedStatus で、
	///   InStatusKey に対応する状態値が登録されてないと、失敗する。
	/// - InValue の値が
	///   InStatusKey に対応する状態値のビット幅を超えていると、失敗する。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	static bool SetSignedStatus(
		/// [in] 代入先となる状態値の名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InStatusKey,
		/// [in] 状態値へ代入する値。
		int32 const InValue);

	/// @brief 浮動小数点数型の状態値へ値を代入する。
	/// @retval true  成功。 InValue を InStatusKey に対応する状態値へ代入した。
	/// @retval false 失敗。状態値は変化しない。
	/// - UPsyqueRulesEngine::RegisterFloatStatus で、
	///   InStatusKey に対応する状態値が登録されてないと、失敗する。
	/// - InValue の値が UPsyqueRulesEngine::GetFloatNan と等値だと、失敗する。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	static bool SetFloatStatus(
		/// [in] 代入先となる状態値の名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InStatusKey,
		/// [in] 状態値へ代入する値。
		float const InValue);
	/// @}
	//-------------------------------------------------------------------------
	/// @name チャンク
	/// @{
	public:
	/// @brief UDataTable を解析し、チャンクを拡張する。
	/// @details
	///   UDataTable を解析し、
	///   状態値と条件式と条件挙動を1つのチャンクに登録する。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	static void ExtendChunkFromDataTable(
		/// [in] 拡張するチャンクの名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InChunkKey,
		/// [in] 登録する状態値のもととなる
		/// FPsyqueRulesStatusTableRow で構成される UDataTable 。
		UDataTable const* const InStatusTable,
		/// [in] 登録する条件式のもととなる
		/// FPsyqueRulesExpressionTableRow で構成される UDataTable 。
		UDataTable const* const InExpressionTable,
		/// [in] 登録する条件式のもととなる
		/// FPsyqueRulesHandlerTableRow で構成される UDataTable 。
		UDataTable const* const InBehaviorTable);

	/// @brief JSON形式の文字列を解析し、チャンクを拡張する。
	/// @details
	///   JSON形式の文字列を解析し、
	///   状態値と条件式と条件挙動を1つのチャンクに登録する。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	static void ExtendChunkFromJsonString(
		/// [in] 拡張するチャンクの名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InChunkKey,
		/// [in] チャンクに登録する状態値が記述されている、JSON形式の文字列。
		/// 最上位要素が配列であること。
		FString const& InStatusJson,
		/// [in] チャンクに登録する条件式が記述されている、JSON形式の文字列。
		/// 最上位要素が配列であること。
		FString const& InExpressionJson,
		/// [in] チャンクに登録する条件挙動が記述されている、JSON形式の文字列。
		/// 最上位要素が配列であること。
		FString const& InBehaviorJson);

	/// @brief チャンクを削除する。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	static void RemoveChunk(
		/// [in] 削除するチャンクの名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InChunkKey);
	/// @}
	//-------------------------------------------------------------------------
	public:
	/// @brief 整数型のNaN値を取得する。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	static int32 GetIntegerNan()
	{
		return std::numeric_limits<int32>::min();
	}

	/// @brief 浮動小数点数型のNaN値を取得する。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	static float GetFloatNan()
	{
		return std::numeric_limits<float>::quiet_NaN();
	}

}; // class UPsyqueRulesEngine

// vim: set noexpandtab:
