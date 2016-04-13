// Copyright (c) 2016, Hillco Psychi, All rights reserved.

#pragma once

#include "./PsyqueRulesStruct.h"
#include "./Psyque/RulesEngine/Driver.h"
#include "PsyqueRulesEngine.generated.h"

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief Blueprintからは、このクラスを介してルールエンジンを操作する。
UCLASS(BlueprintType)
class PSYQUERULESPLUGIN_API UPsyqueRulesEngine: public UObject
{
	GENERATED_UCLASS_BODY()

	//-------------------------------------------------------------------------
	public:
	/// @brief 使用するルールエンジン駆動器を表す型。
	using FDriver = Psyque::RulesEngine::TDriver<
		uint64, float, int32, std::hash<FName>, std::allocator<void*>>;

	//-------------------------------------------------------------------------
	public:
	/// @brief ルールエンジンを更新する。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	void Tick()
	{
		this->Driver.Tick();
	}

	/// @brief 名前文字列から名前ハッシュ値を取得する。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	int32 MakeHash(
		/// [in] 名前文字列となる FName インスタンス。
		FName const& InName)
	const
	{
		return this->Driver.HashFunction(InName);
	}

	//-------------------------------------------------------------------------
	/// @name 状態値の登録
	/// @{
	public:
	/// @brief 論理型の状態値を登録する。
	/// @retval true  成功。状態値を登録した。
	/// @retval false 失敗。状態値は登録されなかった。
	/// - InStatusKey に対応する状態値がすでに登録されていると、失敗する。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	bool RegisterBoolStatus(
		/// [in] 登録する状態値を格納する状態値ビット列チャンクの名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InChunkKey,
		/// [in] 登録する状態値の名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InStatusKey,
		/// [in] 登録する状態値の初期値。
		bool const InValue)
	{
		return this->Driver.RegisterStatus(InChunkKey, InStatusKey, InValue);
	}

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
	bool RegisterUnsignedStatus(
		/// [in] 登録する状態値を格納する状態値ビット列チャンクの名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InChunkKey,
		/// [in] 登録する状態値の名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InStatusKey,
		/// [in] 登録する状態値の初期値。
		int32 const InValue,
		/// [in] 登録する状態値のビット幅。
		uint8 const InBitWidth)
	{
		return 0 <= InValue
			&& this->Driver.RegisterStatus(
				InChunkKey,
				InStatusKey,
				static_cast<uint32>(InValue), InBitWidth);
	}

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
	bool RegisterSignedStatus(
		/// [in] 登録する状態値を格納する状態値ビット列チャンクの名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InChunkKey,
		/// [in] 登録する状態値の名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InStatusKey,
		/// [in] 登録する状態値の初期値。
		int32 const InValue,
		/// [in] 登録する状態値のビット幅。
		uint8 const InBitWidth)
	{
		return InValue != ThisClass::GetIntegerNan()
			&& this->Driver.RegisterStatus(
				InChunkKey, InStatusKey, InValue, InBitWidth);
	}

	/// @brief 浮動小数点数型の状態値を登録する。
	/// @retval true  成功。状態値を登録した。
	/// @retval false 失敗。状態値は登録されなかった。
	/// - InStatusKey に対応する状態値がすでに登録されていると、失敗する。
	/// - InValue が UPsyqueRulesEngine::GetFloatNan と等値だと、失敗する。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	bool RegisterFloatStatus(
		/// [in] 登録する状態値を格納する状態値ビット列チャンクの名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InChunkKey,
		/// [in] 登録する状態値の名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InStatusKey,
		/// [in] 登録する状態値の初期値。
		float const InValue)
	{
		return InValue != ThisClass::GetFloatNan()
			&& this->Driver.RegisterStatus(InChunkKey, InStatusKey, InValue);
	}
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
	EPsyqueKleene FindBoolStatus(
		/// [in] 取得する状態値の名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InStatusKey)
	const
	{
		auto const LocalValue(
			this->Driver.GetReservoir().FindStatus(InStatusKey));
		auto const LocalBool(LocalValue.GetBool());
		return LocalBool != nullptr?
			static_cast<EPsyqueKleene>(*LocalBool):
			EPsyqueKleene::TernaryUnknown;
	}

	/// @brief 符号なし整数型の状態値を取得する。
	/// @return InStatusKey に対応する、符号なし整数型の状態値の値。
	/// - ただし戻り値が UPsyqueRulesEngine::GetIntegerNan
	///   と等値の場合は、状態値の取得に失敗した。
	///   - InStatusKey に対応する状態値がない場合は、失敗する。
	///   - InStatusKey に対応する状態値が符号なし整数型でない場合は、失敗する。
	///   - 状態値が31ビット幅より大きい値だと、失敗する。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	int32 FindUnsignedStatus(
		/// [in] 取得する状態値の名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InStatusKey)
	const
	{
		auto const LocalValue(
			this->Driver.GetReservoir().FindStatus(InStatusKey));
		auto const LocalUnsigned(LocalValue.GetUnsigned());
		if (LocalUnsigned != nullptr &&
			*LocalUnsigned <= static_cast<uint32>(std::numeric_limits<int32>::max()))
		{
			return static_cast<int32>(*LocalUnsigned);
		}
		return ThisClass::GetIntegerNan();
	}

	/// @brief 符号あり整数型の状態値を取得する。
	/// @return InStatusKey に対応する、符号あり整数型の状態値の値。
	/// - ただし戻り値が UPsyqueRulesEngine::GetIntegerNan
	///   と等値の場合は、状態値の取得に失敗した。
	///   - InStatusKey に対応する状態値がない場合は、失敗する。
	///   - InStatusKey に対応する状態値が符号あり整数型でない場合は、失敗する。
	///   - 状態値が32ビット幅より大きい値だと、失敗する。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	int32 FindSignedStatus(
		/// [in] 取得する状態値の名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InStatusKey)
	const
	{
		auto const LocalValue(
			this->Driver.GetReservoir().FindStatus(InStatusKey));
		auto const LocalSigned(LocalValue.GetSigned());
		if (LocalSigned != nullptr
			&& *LocalSigned <= MAX_int32
			&& ThisClass::GetIntegerNan() < *LocalSigned)
		{
			return static_cast<int32>(*LocalSigned);
		}
		return ThisClass::GetIntegerNan();
	}

	/// @brief 浮動小数点数型の状態値を取得する。
	/// @return InStatusKey に対応する、浮動小数点数型の状態値の値。
	/// - ただし戻り値が UPsyqueRulesEngine::GetFloatNan
	///   と等値の場合は、状態値の取得に失敗した。
	///   - InStatusKey に対応する状態値がない場合は、失敗する。
	///   - InStatusKey に対応する状態値が浮動小数点数型でない場合は、失敗する。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	float FindFloatStatus(
		/// [in] 取得する状態値の名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InStatusKey)
	const
	{
		auto const LocalValue(
			this->Driver.GetReservoir().FindStatus(InStatusKey));
		auto const LocalFloat(LocalValue.GetFloat());
		return LocalFloat != nullptr? *LocalFloat: ThisClass::GetFloatNan();
	}

	/// @brief 状態値の型を取得する。
	/// @return 状態値の型の種類。 InStatusKey に対応する状態値がない場合は、
	///   EPsyqueRulesStatusKind::Empty となる。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	EPsyqueRulesStatusKind FindStatusKind(
		/// [in] 取得する状態値の名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InStatusKey)
	const
	{
		return this->Driver.GetReservoir().FindKind(InStatusKey);
	}

	/// @brief 状態値のビット幅を取得する。
	/// @return 状態値のビット幅。
	///   InStatusKey に対応する状態値がない場合は、0となる。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	uint8 FindStatusBitWidth(
		/// [in] 取得する状態値の名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InStatusKey)
	const
	{
		return this->Driver.GetReservoir().FindBitWidth(InStatusKey);
	}

	/// @brief 状態値のビット構成を取得する。
	/// @return 状態値のビット構成。
	///   InStatusKey に対応する状態値がない場合は、0となる。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	int32 FindStatusBitFormat(
		/// [in] 取得する状態値の名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InStatusKey)
	const
	{
		return this->Driver.GetReservoir().FindBitFormat(InStatusKey);
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 状態値の代入演算
	/// @{
	public:
	/// @brief 真偽値の状態値への代入演算を予約する。
	/// @note 代入演算は、次回の UPsyqueRulesEngine::Tick 以降に行われる。
	/// @retval true  成功。代入演算を予約した。
	/// @retval false 失敗。代入演算を予約しなかった。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	bool AccumulateAssignmentBool(
		/// [in] 代入演算の遅延方法。
		EPsyqueAccumulationDelay const InDelay,
		/// [in] 代入演算の左辺値となる状態値の名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InStatusKey,
		/// [in] 代入演算の種類。
		EPsyqueStatusAssignment const InOperator,
		/// [in] 代入演算の右辺値。
		bool const InValue)
	{
		this->Driver.Accumulator.Accumulate(
			ThisClass::FDriver::FReservoir::FStatusAssignment(
				InStatusKey,
				InOperator,
				ThisClass::FDriver::FReservoir::FStatusValue(InValue)),
			InDelay);
		return true;
	}

	/// @brief 符号なし整数値の状態値への代入演算を予約する。
	/// @note 代入演算は、次回の UPsyqueRulesEngine::Tick 以降に行われる。
	/// @retval true  成功。代入演算を予約した。
	/// @retval false 失敗。代入演算を予約しなかった。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	bool AccumulateAssignmentUnsigned(
		/// [in] 代入演算の遅延方法。
		EPsyqueAccumulationDelay const InDelay,
		/// [in] 代入演算の左辺値となる状態値の名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InStatusKey,
		/// [in] 代入演算の種類。
		EPsyqueStatusAssignment const InOperator,
		/// [in] 代入演算の右辺値。
		int32 const InValue)
	{
		if (InValue < 0)
		{
			return false;
		}
		using FStatusValue = ThisClass::FDriver::FReservoir::FStatusValue;
		this->Driver.Accumulator.Accumulate(
			ThisClass::FDriver::FReservoir::FStatusAssignment(
				InStatusKey,
				InOperator,
				FStatusValue(static_cast<FStatusValue::FUnsigned>(InValue))),
			InDelay);
		return true;
	}

	/// @brief 符号あり整数値の状態値への代入演算を予約する。
	/// @note 代入演算は、次回の UPsyqueRulesEngine::Tick 以降に行われる。
	/// @retval true  成功。代入演算を予約した。
	/// @retval false 失敗。代入演算を予約しなかった。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	bool AccumulateAssignmentSigned(
		/// [in] 代入演算の遅延方法。
		EPsyqueAccumulationDelay const InDelay,
		/// [in] 代入演算の左辺値となる状態値の名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InStatusKey,
		/// [in] 代入演算の種類。
		EPsyqueStatusAssignment const InOperator,
		/// [in] 代入演算の右辺値。
		int32 const InValue)
	{
		if (InValue == ThisClass::GetIntegerNan())
		{
			return false;
		}
		using FStatusValue = ThisClass::FDriver::FReservoir::FStatusValue;
		this->Driver.Accumulator.Accumulate(
			ThisClass::FDriver::FReservoir::FStatusAssignment(
				InStatusKey,
				InOperator,
				FStatusValue(static_cast<FStatusValue::FSigned>(InValue))),
			InDelay);
		return true;
	}

	/// @brief 浮動小数点数値の状態値への代入演算を予約する。
	/// @note 代入演算は、次回の UPsyqueRulesEngine::Tick 以降に行われる。
	/// @retval true  成功。代入演算を予約した。
	/// @retval false 失敗。代入演算を予約しなかった。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	bool AccumulateAssignmentFloat(
		/// [in] 代入演算の遅延方法。
		EPsyqueAccumulationDelay const InDelay,
		/// [in] 代入演算の左辺値となる状態値の名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InStatusKey,
		/// [in] 代入演算の種類。
		EPsyqueStatusAssignment const InOperator,
		/// [in] 代入演算の右辺値。
		float const InValue)
	{
		if (InValue == ThisClass::GetFloatNan())
		{
			return false;
		}
		using FStatusValue = ThisClass::FDriver::FReservoir::FStatusValue;
		this->Driver.Accumulator.Accumulate(
			ThisClass::FDriver::FReservoir::FStatusAssignment(
				InStatusKey,
				InOperator,
				FStatusValue(static_cast<FStatusValue::FFloat>(InValue))),
			InDelay);
		return true;
	}
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
	void ExtendChunkFromDataTable(
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
	void ExtendChunkFromJsonString(
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
	void RemoveChunk(
		/// [in] 削除するチャンクの名前ハッシュ値。
		/// UPsyqueRulesEngine::MakeHash から取得する。
		int32 const InChunkKey)
	{
		this->Driver.RemoveChunk(InChunkKey);
	}
	/// @}
	//-------------------------------------------------------------------------
	public:
	/// @brief 現在のルールエンジンを取得する。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	static UPsyqueRulesEngine* Get();

	/// @brief 新たなルールエンジンを生成する。
	//UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	static UPsyqueRulesEngine* Create();

	/// @brief 現在のルールエンジンを破棄する。
	/// @retval true  成功。ルールエンジンを破棄した。
	/// @retval false 失敗。ルールエンジンを破棄しなかった。
	/// - ルールエンジンがすでに破棄されていると、失敗する。
	//UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	static bool Destroy();

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

	//-------------------------------------------------------------------------
	public:
	ThisClass::FDriver Driver;

}; // class UPsyqueRulesEngine

// vim: set noexpandtab:
