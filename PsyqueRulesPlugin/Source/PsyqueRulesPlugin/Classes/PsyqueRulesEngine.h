// Copyright (c) 2016, Hillco Psychi, All rights reserved.

#pragma once

#include <memory>
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
/// @brief Kleeneの3値論理による真理値。
/// @details http://goo.gl/0kyClO
UENUM(BlueprintType)
enum class EPsyqueKleene: uint8
{
	TernaryFalse =     0 UMETA(DisplayName="Ternary False"),  ///< 偽。
	TernaryTrue =      1 UMETA(DisplayName="Ternary True"),   ///< 真。
	TernaryUnknown = 128 UMETA(DisplayName="Ternary Unknown") ///< 真か偽か不明。
};
static_assert(
	static_cast<int8>(EPsyqueKleene::TernaryFalse) == false
	&& static_cast<int8>(EPsyqueKleene::TernaryTrue) == true
	&& static_cast<int8>(EPsyqueKleene::TernaryUnknown) < 0,
	"");

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief Blueprintからは、このクラスを介してルールエンジンを操作する。
UCLASS()
class UPsyqueRulesEngine: public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	//-------------------------------------------------------------------------
	public:
	/// @brief ルールエンジン駆動器を生成する。
	/// @retval true  成功。ルールエンジン駆動器を生成した。
	/// @retval false 失敗。ルールエンジン駆動器を生成しなかった。
	/// - ルールエンジン駆動器がすでに生成されていると、失敗する。
	static bool CreateDriver();

	/// @brief ルールエンジン駆動器を破棄する。
	/// @retval true  成功。ルールエンジン駆動器を破棄した。
	/// @retval false 失敗。ルールエンジン駆動器を破棄しなかった。
	/// - ルールエンジン駆動器がすでに破棄されていると、失敗する。
	static bool DestroyDriver();

	/// @brief ルールエンジンを更新する。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulePlugin")
	static void Tick();

	/// @brief 名前文字列から名前ハッシュ値を取得する。
	UFUNCTION(BlueprintPure, Category="PsyqueRulePlugin")
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
	/// - InStatusKey に対応する状態値がすでに登録されていると失敗する。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulePlugin")
	static bool RegisterBoolStatus(
		/// [in] 登録する状態値を格納する状態値ビット列チャンクの名前ハッシュ値。
		int32 const InChunkKey,
		/// [in] 登録する状態値の名前ハッシュ値。
		int32 const InStatusKey,
		/// [in] 登録する状態値の初期値。
		bool const InValue);

	/// @brief 符号なし整数型の状態値を登録する。
	/// @retval true  成功。状態値を登録した。
	/// @retval false 失敗。状態値は登録されなかった。
	/// - InStatusKey に対応する状態値がすでに登録されていると失敗する。
	/// - InValue が0未満だと失敗する。
	/// - InBitWidth が1以下だと失敗する。ビット幅が1の場合は、論理型として
	///   UPsyqueRulesEngine::RegisterBoolStatus で登録すること。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulePlugin")
	static bool RegisterUnsignedStatus(
		/// [in] 登録する状態値を格納する状態値ビット列チャンクの名前ハッシュ値。
		int32 const InChunkKey,
		/// [in] 登録する状態値の名前ハッシュ値。
		int32 const InStatusKey,
		/// [in] 登録する状態値の初期値。
		int32 const InValue,
		/// [in] 登録する状態値のビット幅。
		uint8 const InBitWidth);

	/// @brief 符号あり整数型の状態値を登録する。
	/// @retval true  成功。状態値を登録した。
	/// @retval false 失敗。状態値は登録されなかった。
	/// - InStatusKey に対応する状態値がすでに登録されていると失敗する。
	/// - InValue が UPsyqueRulesEngine::GetIntegerNan と等値だと失敗する。
	/// - InBitWidth が1以下だと失敗する。ビット幅が1の場合は、論理型として
	///   UPsyqueRulesEngine::RegisterBoolStatus で登録すること。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulePlugin")
	static bool RegisterSignedStatus(
		/// [in] 登録する状態値を格納する状態値ビット列チャンクの名前ハッシュ値。
		int32 const InChunkKey,
		/// [in] 登録する状態値の名前ハッシュ値。
		int32 const InStatusKey,
		/// [in] 登録する状態値の初期値。
		int32 const InValue,
		/// [in] 登録する状態値のビット幅。
		uint8 const InBitWidth);

	/// @brief 浮動小数点数型の状態値を登録する。
	/// @retval true  成功。状態値を登録した。
	/// @retval false 失敗。状態値は登録されなかった。
	/// - InStatusKey に対応する状態値がすでに登録されていると失敗する。
	/// - InValue が UPsyqueRulesEngine::GetFloatNan と等値だと失敗する。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulePlugin")
	static bool RegisterFloatStatus(
		/// [in] 登録する状態値を格納する状態値ビット列チャンクの名前ハッシュ値。
		int32 const InChunkKey,
		/// [in] 登録する状態値の名前ハッシュ値。
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
	UFUNCTION(BlueprintPure, Category="PsyqueRulePlugin")
	static EPsyqueKleene GetBoolStatus(
		/// [in] 取得する状態値の名前ハッシュ値。
		int32 const InStatusKey);

	/// @brief 符号なし整数型の状態値を取得する。
	/// @return InStatusKey に対応する、符号なし整数型の状態値の値。
	/// - ただし戻り値が UPsyqueRulesEngine::GetIntegerNan
	///   と等値の場合は、状態値の取得に失敗した。
	///   - InStatusKey に対応する状態値がない場合は、失敗する。
	///   - InStatusKey に対応する状態値が符号なし整数型でない場合は、失敗する。
	UFUNCTION(BlueprintPure, Category="PsyqueRulePlugin")
	static int32 GetUnsignedStatus(
		/// [in] 取得する状態値の名前ハッシュ値。
		int32 const InStatusKey);

	/// @brief 符号あり整数型の状態値を取得する。
	/// @return InStatusKey に対応する、符号あり整数型の状態値の値。
	/// - ただし戻り値が UPsyqueRulesEngine::GetIntegerNan
	///   と等値の場合は、状態値の取得に失敗した。
	///   - InStatusKey に対応する状態値がない場合は、失敗する。
	///   - InStatusKey に対応する状態値が符号あり整数型でない場合は、失敗する。
	UFUNCTION(BlueprintPure, Category="PsyqueRulePlugin")
	static int32 GetSignedStatus(
		/// [in] 取得する状態値の名前ハッシュ値。
		int32 const InStatusKey);

	/// @brief 浮動小数点数型の状態値を取得する。
	/// @return InStatusKey に対応する、浮動小数点数型の状態値の値。
	/// - ただし戻り値が UPsyqueRulesEngine::GetFloatNan
	///   と等値の場合は、状態値の取得に失敗した。
	///   - InStatusKey に対応する状態値がない場合は、失敗する。
	///   - InStatusKey に対応する状態値が浮動小数点数型でない場合は、失敗する。
	UFUNCTION(BlueprintPure, Category="PsyqueRulePlugin")
	static float GetFloatStatus(
		/// [in] 取得する状態値の名前ハッシュ値。
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
	UFUNCTION(BlueprintCallable, Category="PsyqueRulePlugin")
	static bool SetBoolStatus(
		/// [in] 代入先となる状態値の名前ハッシュ値。
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
	UFUNCTION(BlueprintCallable, Category="PsyqueRulePlugin")
	static bool SetUnsignedStatus(
		/// [in] 代入先となる状態値の名前ハッシュ値。
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
	UFUNCTION(BlueprintCallable, Category="PsyqueRulePlugin")
	static bool SetSignedStatus(
		/// [in] 代入先となる状態値の名前ハッシュ値。
		int32 const InStatusKey,
		/// [in] 状態値へ代入する値。
		int32 const InValue);

	/// @brief 浮動小数点数型の状態値へ値を代入する。
	/// @retval true  成功。 InValue を InStatusKey に対応する状態値へ代入した。
	/// @retval false 失敗。状態値は変化しない。
	/// - UPsyqueRulesEngine::RegisterFloatStatus で、
	///   InStatusKey に対応する状態値が登録されてないと、失敗する。
	/// - InValue の値が UPsyqueRulesEngine::GetFloatNan と等値だと、失敗する。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulePlugin")
	static bool SetFloatStatus(
		/// [in] 代入先となる状態値の名前ハッシュ値。
		int32 const InStatusKey,
		/// [in] 状態値へ代入する値。
		float const InValue);
	/// @}
	//-------------------------------------------------------------------------
	/// @name チャンク
	/// @{
	public:
	/// @brief チャンクを拡張する。
	/// @details
	///   JSON形式の文字列を解析し、
	///   状態値と条件式と条件挙動を1つのチャンクに登録する。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulePlugin")
	static void ExtendChunkByJson(
		/// [in] 拡張するチャンクの名前ハッシュ値。
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
	UFUNCTION(BlueprintCallable, Category="PsyqueRulePlugin")
	static void RemoveChunk(
		/// [in] 削除するチャンクの名前ハッシュ値。
		int32 const InChunkKey);
	/// @}
	//-------------------------------------------------------------------------
	public:
	/// @brief 整数型のNaN値を取得する。
	UFUNCTION(BlueprintPure, Category="PsyqueRulePlugin")
	static int32 GetIntegerNan()
	{
		return std::numeric_limits<int32>::min();
	}

	/// @brief 浮動小数点数型のNaN値を取得する。
	UFUNCTION(BlueprintPure, Category="PsyqueRulePlugin")
	static float GetFloatNan()
	{
		return std::numeric_limits<float>::quiet_NaN();
	}

}; // class UPsyqueRulesEngine

// vim: set noexpandtab:
