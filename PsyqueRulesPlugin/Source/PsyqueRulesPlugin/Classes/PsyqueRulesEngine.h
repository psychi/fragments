// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief UPsyqueRulesEngine
/// @author Hillco Psychi (https://twitter.com/psychi)

#pragma once

#include "./PsyqueRulesStruct.h"
#include "./Psyque/RulesEngine/Driver.h"
#include "PsyqueRulesEngine.generated.h"

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief Blueprintからは、このクラスを介してルールエンジンを操作する。
/// @par 使い方の概略
/// - ブループリントグラフの「Construct Object from Class」ノードや、
///   C++の NewObject 関数などで、ルールエンジンをインスタンス化する。
/// - UPsyqueRulesEngine::ExtendChunkFromDataTable
///   で、状態値と条件式と条件イベントをルールエンジンに登録する。
///   - 以下の関数からでも、状態値をルールエンジンに登録できる。
///     - UPsyqueRulesEngine::RegisterBoolStatus
///     - UPsyqueRulesEngine::RegisterUnsignedStatus
///     - UPsyqueRulesEngine::RegisterSignedStatus
///     - UPsyqueRulesEngine::RegisterFloatStatus
///   - UPsyqueRulesEngine::RegisterEvent
///     からでも、条件イベントをルールエンジンに登録できる。
/// - 以下の関数で、ルールエンジンの状態値を書き換える。
///   - UPsyqueRulesEngine::AccumulateAssignmentBool
///   - UPsyqueRulesEngine::AccumulateAssignmentUnsigned
///   - UPsyqueRulesEngine::AccumulateAssignmentSigned
///   - UPsyqueRulesEngine::AccumulateAssignmentFloat
/// - 状態値を書き換えると、書き換えられた状態値を参照する条件式が再評価され、
///   UPsyqueRulesEngine::RegisterEvent で登録した評価の遷移条件に合致していれば、
///   登録した条件イベントが実行される。
UCLASS(BlueprintType, Blueprintable)
class PSYQUERULESPLUGIN_API UPsyqueRulesEngine:
	public UObject, public FTickableGameObject
{
	GENERATED_UCLASS_BODY()

	//-------------------------------------------------------------------------
	public:
	/// @brief 使用するルールエンジン駆動器を表す型。
	using FDriver = Psyque::RulesEngine::TDriver<
		uint64, float, int32, std::hash<FName>, std::allocator<void*>>;

	//-------------------------------------------------------------------------
	public:
	/// @brief 名前文字列から名前ハッシュ値を取得する。
	/// @return InName に対応する名前ハッシュ値。
	/// @param InName 名前ハッシュ値のもととなる FName インスタンス。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	int32 MakeHash(FName const InName) const
	{
		return this->Driver.HashFunction(InName);
	}

	//-------------------------------------------------------------------------
	/// @name ルールエンジンの更新
	/// @{
	public:
	/// @brief ルールエンジンを更新する。
	/// @details
	///   - ルールエンジン駆動器は、状態値の変化を検知する。
	///   - ルールエンジン駆動器は、変化した状態値を参照している条件式を評価する。
	///   - ルールエンジン駆動器は、
	///     評価結果が変化した条件式を参照している条件挙動を呼び出す。
	/// @param InDeltaTime 前回の更新からの差分時間。
	virtual void Tick(float InDeltaTime) override
	{
		if (0 < InDeltaTime)
		{
			this->Driver.Tick();
		}
	}

	/// @brief UPsyqueRulesEngine::Tick が可能か判定する。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	virtual bool IsTickable() const override
	{
		return true;
	}

	/// @brief UPsyqueRulesEngine::Tick するための識別値を取得する。
	virtual TStatId GetStatId() const override
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(UPsyqueRulesEngine, STATGROUP_Tickables);
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 状態値の登録
	/// @{
	public:
	/// @brief 論理型の状態値を登録する。
	/// @return 状態値の登録に成功したか否か。
	///   失敗した場合は、状態値は登録されない。
	///   - InStatusKey に対応する状態値がすでに登録されていると、失敗する。
	/// @param InChunkKey
	///   登録する状態値を格納する状態値ビット列チャンクの名前ハッシュ値。
	///   UPsyqueRulesEngine::MakeHash から取得する。
	/// @param InStatusKey 登録する状態値の名前ハッシュ値。
	///   UPsyqueRulesEngine::MakeHash から取得する。
	/// @param InValue 登録する状態値の初期値。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	bool RegisterBoolStatus(
		int32 const InChunkKey,
		int32 const InStatusKey,
		bool const InValue)
	{
		return this->Driver.RegisterStatus(InChunkKey, InStatusKey, InValue);
	}

	/// @brief 符号なし整数型の状態値を登録する。
	/// @note InBitWidth が31より大きく、状態値が31ビット幅より大きい値だと、
	///   UPsyqueRulesEngine::GetUnsignedStatus で状態値の取得に失敗する。
	/// @return 状態値の登録に成功したか否か。
	///   失敗した場合は、状態値は登録されない。
	///   - InValue が0未満だと、失敗する。
	///   - InBitWidth が1以下だと、失敗する。ビット幅が1の場合は、論理型として
	///     UPsyqueRulesEngine::RegisterBoolStatus で登録すること。
	/// @param InChunkKey
	///   登録する状態値を格納する状態値ビット列チャンクの名前ハッシュ値。
	///   UPsyqueRulesEngine::MakeHash から取得する。
	/// @param InStatusKey 登録する状態値の名前ハッシュ値。
	///   UPsyqueRulesEngine::MakeHash から取得する。
	/// @param InValue 登録する状態値の初期値。
	/// @param InBitWidth 登録する状態値のビット幅。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	bool RegisterUnsignedStatus(
		int32 const InChunkKey,
		int32 const InStatusKey,
		int32 const InValue,
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
	/// @return 状態値の登録に成功したか否か。
	///   失敗した場合は、状態値は登録されない。
	///   - InStatusKey に対応する状態値がすでに登録されていると、失敗する。
	///   - InValue が UPsyqueRulesEngine::GetIntegerNan と等値だと、失敗する。
	///   - InBitWidth が1以下だと失敗する。ビット幅が1の場合は、論理型として
	///     UPsyqueRulesEngine::RegisterBoolStatus で登録すること。
	/// @param InChunkKey
	///   登録する状態値を格納する状態値ビット列チャンクの名前ハッシュ値。
	///   UPsyqueRulesEngine::MakeHash から取得する。
	/// @param InStatusKey 登録する状態値の名前ハッシュ値。
	///   UPsyqueRulesEngine::MakeHash から取得する。
	/// @param InValue 登録する状態値の初期値。
	/// @param InBitWidth 登録する状態値のビット幅。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	bool RegisterSignedStatus(
		int32 const InChunkKey,
		int32 const InStatusKey,
		int32 const InValue,
		uint8 const InBitWidth)
	{
		return InValue != ThisClass::GetIntegerNan()
			&& this->Driver.RegisterStatus(
				InChunkKey, InStatusKey, InValue, InBitWidth);
	}

	/// @brief 浮動小数点数型の状態値を登録する。
	/// @return 状態値の登録に成功したか否か。
	///   失敗した場合は、状態値は登録されない。
	///   - InStatusKey に対応する状態値がすでに登録されていると、失敗する。
	///   - InValue が UPsyqueRulesEngine::GetFloatNan と等値だと、失敗する。
	/// @param InChunkKey
	///   登録する状態値を格納する状態値ビット列チャンクの名前ハッシュ値。
	///   UPsyqueRulesEngine::MakeHash から取得する。
	/// @param InStatusKey 登録する状態値の名前ハッシュ値。
	///   UPsyqueRulesEngine::MakeHash から取得する。
	/// @param InValue 登録する状態値の初期値。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	bool RegisterFloatStatus(
		int32 const InChunkKey,
		int32 const InStatusKey,
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
	/// @return InStatusKey に対応する論理型の状態値の値。ただし
	///   EPsyqueKleene::Unknown の場合は、取得に失敗している。
	///   - InStatusKey に対応する状態値がない場合は、失敗する。
	///   - InStatusKey に対応する状態値が論理型でない場合は、失敗する。
	/// @param InStatusKey 取得する状態値の名前ハッシュ値。
	///   UPsyqueRulesEngine::MakeHash から取得する。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	EPsyqueKleene FindBoolStatus(int32 const InStatusKey) const
	{
		auto const LocalValue(
			this->Driver.GetReservoir().FindStatus(InStatusKey));
		auto const LocalBool(LocalValue.GetBool());
		return LocalBool != nullptr?
			static_cast<EPsyqueKleene>(*LocalBool):
			EPsyqueKleene::Unknown;
	}

	/// @brief 符号なし整数型の状態値を取得する。
	/// @return InStatusKey に対応する、符号なし整数型の状態値の値。ただし
	///   UPsyqueRulesEngine::GetIntegerNan と等値の場合は、取得に失敗している。
	///   - InStatusKey に対応する状態値がない場合は、失敗する。
	///   - InStatusKey に対応する状態値が符号なし整数型でない場合は、失敗する。
	///   - 状態値が31ビット幅より大きい値だと、失敗する。
	/// @param InStatusKey 取得する状態値の名前ハッシュ値。
	///   UPsyqueRulesEngine::MakeHash から取得する。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	int32 FindUnsignedStatus(int32 const InStatusKey) const
	{
		auto const LocalValue(
			this->Driver.GetReservoir().FindStatus(InStatusKey));
		auto const LocalUnsigned(LocalValue.GetUnsigned());
		return LocalUnsigned != nullptr && *LocalUnsigned <= MAX_int32?
			static_cast<int32>(*LocalUnsigned): ThisClass::GetIntegerNan();
	}

	/// @brief 符号あり整数型の状態値を取得する。
	/// @return InStatusKey に対応する、符号あり整数型の状態値の値。ただし
	///   UPsyqueRulesEngine::GetIntegerNan と等値の場合は、取得に失敗している。
	///   - InStatusKey に対応する状態値がない場合は、失敗する。
	///   - InStatusKey に対応する状態値が符号あり整数型でない場合は、失敗する。
	///   - 状態値が32ビット幅より大きい値だと、失敗する。
	/// @param InStatusKey 取得する状態値の名前ハッシュ値。
	///   UPsyqueRulesEngine::MakeHash から取得する。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	int32 FindSignedStatus(int32 const InStatusKey) const
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
	/// @return InStatusKey に対応する、浮動小数点数型の状態値の値。ただし
	///   UPsyqueRulesEngine::GetFloatNan と等値の場合は、取得に失敗している。
	///   - InStatusKey に対応する状態値がない場合は、失敗する。
	///   - InStatusKey に対応する状態値が浮動小数点数型でない場合は、失敗する。
	/// @param InStatusKey 取得する状態値の名前ハッシュ値。
	///   UPsyqueRulesEngine::MakeHash から取得する。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	float FindFloatStatus(int32 const InStatusKey) const
	{
		auto const LocalValue(
			this->Driver.GetReservoir().FindStatus(InStatusKey));
		auto const LocalFloat(LocalValue.GetFloat());
		return LocalFloat != nullptr? *LocalFloat: ThisClass::GetFloatNan();
	}

	/// @brief 状態値の型を取得する。
	/// @return 状態値の型の種類。ただし
	///   EPsyqueRulesStatusKind::Empty の場合は、取得に失敗している。
	///   - InStatusKey に対応する状態値がない場合は、失敗する。
	/// @param InStatusKey 型を取得する状態値の名前ハッシュ値。
	///   UPsyqueRulesEngine::MakeHash から取得する。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	EPsyqueRulesStatusKind FindStatusKind(
		int32 const InStatusKey)
	const
	{
		return this->Driver.GetReservoir().FindKind(InStatusKey);
	}

	/// @brief 状態値のビット幅を取得する。
	/// @return 状態値のビット幅。ただし0の場合は、取得に失敗している。
	///   - InStatusKey に対応する状態値がない場合は、失敗する。
	/// @param InStatusKey ビット幅を取得する状態値の名前ハッシュ値。
	///   UPsyqueRulesEngine::MakeHash から取得する。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	uint8 FindStatusBitWidth(int32 const InStatusKey) const
	{
		return this->Driver.GetReservoir().FindBitWidth(InStatusKey);
	}

	/// @brief 状態値のビット構成を取得する。
	/// @return 状態値のビット構成。ただし0の場合は、取得に失敗している。
	///   InStatusKey に対応する状態値がない場合は、失敗する。
	/// @param InStatusKey ビット構成を取得する状態値の名前ハッシュ値。
	///   UPsyqueRulesEngine::MakeHash から取得する。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	uint8 FindStatusBitFormat(
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
	/// @brief 状態値と真偽値の代入演算を予約する。
	/// @details 代入演算は、次回以降の UPsyqueRulesEngine::Tick で行われる。
	///   InStatusKey に対応する状態値が論理型以外の場合、
	///   代入演算の予約には成功するが、
	///   UPsyqueRulesEngine::Tick で行われる代入演算には失敗する。
	/// @return 代入演算の予約に成功したか否か。
	///   失敗の場合は、代入演算は予約されない。
	/// @param InDelay 代入演算の遅延方法。
	/// @param InStatusKey  代入演算の左辺値となる状態値の名前ハッシュ値。
	///   UPsyqueRulesEngine::MakeHash から取得する。
	/// @param InOperator 代入演算の種類。
	/// @param InValue 代入演算の右辺値。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	bool AccumulateAssignmentBool(
		EPsyqueAccumulationDelay const InDelay,
		int32 const InStatusKey,
		EPsyqueStatusAssignment const InOperator,
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

	/// @brief 状態値と符号なし整数値の代入演算を予約する。
	/// @details 代入演算は、次回以降の UPsyqueRulesEngine::Tick で行われる。
	///   InStatusKey に対応する状態値が論理型の場合、
	///   代入演算の予約には成功するが、
	///   UPsyqueRulesEngine::Tick で行われる代入演算には失敗する。
	/// @return 代入演算の予約に成功したか否か。
	///   失敗の場合は、代入演算は予約されない。
	/// @param InDelay 代入演算の遅延方法。
	/// @param InStatusKey  代入演算の左辺値となる状態値の名前ハッシュ値。
	///   UPsyqueRulesEngine::MakeHash から取得する。
	/// @param InOperator 代入演算の種類。
	/// @param InValue 代入演算の右辺値。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	bool AccumulateAssignmentUnsigned(
		EPsyqueAccumulationDelay const InDelay,
		int32 const InStatusKey,
		EPsyqueStatusAssignment const InOperator,
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

	/// @brief 状態値と符号あり整数値の代入演算を予約する。
	/// @details 代入演算は、次回以降の UPsyqueRulesEngine::Tick で行われる。
	///   InStatusKey に対応する状態値が符号あり整数型かつ
	///   浮動小数点数型以外の場合、代入演算の予約には成功するが、
	///   UPsyqueRulesEngine::Tick で行われる代入演算には失敗する。
	/// @return 代入演算の予約に成功したか否か。
	///   失敗の場合は、代入演算は予約されない。
	/// @param InDelay 代入演算の遅延方法。
	/// @param InStatusKey  代入演算の左辺値となる状態値の名前ハッシュ値。
	///   UPsyqueRulesEngine::MakeHash から取得する。
	/// @param InOperator 代入演算の種類。
	/// @param InValue 代入演算の右辺値。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	bool AccumulateAssignmentSigned(
		EPsyqueAccumulationDelay const InDelay,
		int32 const InStatusKey,
		EPsyqueStatusAssignment const InOperator,
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

	/// @brief 状態値と浮動小数点数値の代入演算を予約する。
	/// @details 代入演算は、次回以降の UPsyqueRulesEngine::Tick で行われる。
	///   InStatusKey に対応する状態値が浮動小数点数型以外の場合、
	///   代入演算の予約には成功するが、
	///   UPsyqueRulesEngine::Tick で行われる代入演算には失敗する。
	/// @return 代入演算の予約に成功したか否か。
	///   失敗の場合は、代入演算は予約されない。
	/// @param InDelay 代入演算の遅延方法。
	/// @param InStatusKey 代入演算の左辺値となる状態値の名前ハッシュ値。
	///   UPsyqueRulesEngine::MakeHash から取得する。
	/// @param InOperator 代入演算の種類。
	/// @param InValue 代入演算の右辺値。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	bool AccumulateAssignmentFloat(
		EPsyqueAccumulationDelay const InDelay,
		int32 const InStatusKey,
		EPsyqueStatusAssignment const InOperator,
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
	/// @name 条件式
	/// @{
	public:
	/// @brief 条件式が登録されているか判定する。
	/// @return InExpressionKey に対応する条件式が登録されているか否か。
	/// @param InExpressionKey 判定する条件式の名前ハッシュ値。
	///   UPsyqueRulesEngine::MakeHash から取得する。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	bool FindExpression(int32 const InExpressionKey) const
	{
		return this->Driver.Evaluator.FindExpression(InExpressionKey)
			!= nullptr;
	}

	/// @brief 登録されている条件式を評価する。
	/// @return 条件式の評価結果。
	///   - 条件式が登録されていないと、 EPsyqueKleene::Unknown となる。
	///   - 条件式が参照する状態値が登録されていないと、
	///     EPsyqueKleene::Unknown となる。
	/// @param InExpressionKey 評価する条件式の名前ハッシュ値。
	///   UPsyqueRulesEngine::MakeHash から取得する。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	EPsyqueKleene EvaluateExpression(int32 const InExpressionKey) const
	{
		return this->Driver.Evaluator.EvaluateExpression(
			InExpressionKey, this->Driver.GetReservoir());
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 条件イベント
	/// @{
	public:
	/// @brief 条件イベントを登録する。
	/// @details 条件式の評価が変化した際に実行する動的デリゲートを登録する。
	/// @sa 登録した条件イベントは、以下の関数によって取り除ける。
	///   - UPsyqueRulesEngine::UnregisterEvents
	///   - UPsyqueRulesEngine::UnregisterEventsByObject
	///   - UPsyqueRulesEngine::UnregisterEventsOfEachExpression
	///   - UPsyqueRulesEngine::UnregisterEventsOfEachExpressionByObject
	///   - UPsyqueRulesEngine::UnregisterEventsOfEachCondition
	/// @return 登録に成功したかどうか。
	///   - InBeforeCondition と InLatestCondition が等価だと、失敗する。
	///   - InDelegate が無効だと、失敗する。
	/// @param InExpressionKey
	///   InDelegate を実行するか判定する条件式を指す名前ハッシュ値。
	///   UPsyqueRulesEngine::MakeHash から取得する。
	/// @param InBeforeCondition
	///   InDelegate を実行する条件となる、条件式の直前の評価。
	/// @param InLatestCondition
	///   InDelegate を実行する条件となる、条件式の最新の評価。
	/// @param InPriority InDelegate の実行優先順位。降順に実行される。
	/// @param InDelegate 条件イベントで実行する動的デリゲート。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	bool RegisterEvent(
		int32 const InExpressionKey,
		EPsyqueKleene const InBeforeCondition,
		EPsyqueKleene const InLatestCondition,
		int32 const InPriority,
		FPsyqueRulesDynamicDelegate const& InDelegate)
	{
		return this->Driver.Dispatcher.RegisterHook(
			InExpressionKey,
			InBeforeCondition,
			InLatestCondition,
			InPriority,
			FPsyqueRulesDelegate::CreateUFunction(
				const_cast<UObject*>(InDelegate.GetUObject()),
				InDelegate.GetFunctionName()))
		.IsValid();
	}

	/// @brief デリゲートに対応する条件イベントを、すべて取り除く。
	/// @param InDelegate 取り除く条件イベントのデリゲート。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	void UnregisterEvents(FPsyqueRulesDynamicDelegate const& InDelegate)
	{
		this->Driver.Dispatcher.UnregisterHooks(
			Psyque::RulesEngine::FDelegateIdentifier(InDelegate));
	}

	/// @brief UObject に対応する条件イベントを、すべて取り除く。
	/// @param InObject 取り除く条件イベントに対応する UObject を指すポインタ。
	/// @param InFunctionName
	///   取り除く条件イベントに対応する UObject のメソッド名。
	///   ただし空文字列の場合は、すべてのメソッドが対象となる。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	void UnregisterEventsByObject(
		UObject const* const InObject,
		FName const InFunctionName)
	{
		if (InObject != nullptr)
		{
			this->Driver.Dispatcher.UnregisterHooks(
				Psyque::RulesEngine::FDelegateIdentifier(
					*InObject, InFunctionName));
		}
	}

	/// @brief 条件式とデリゲートに対応する条件イベントを、すべて取り除く。
	/// @param InExpressionKey
	///   取り除く条件イベントに対応する条件式を指す名前ハッシュ値。
	///   UPsyqueRulesEngine::MakeHash から取得する。
	/// @param InDelegate 取り除く条件イベントのデリゲート。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	void UnregisterEventsOfEachExpression(
		int32 const InExpressionKey,
		FPsyqueRulesDynamicDelegate const& InDelegate)
	{
		this->Driver.Dispatcher.UnregisterHooks(
			InExpressionKey,
			Psyque::RulesEngine::FDelegateIdentifier(InDelegate));
	}

	/// @brief 条件式と UObject に対応する条件イベントを、すべて取り除く。
	/// @param InExpressionKey
	///   取り除く条件イベントに対応する条件式を指す名前ハッシュ値。
	///   UPsyqueRulesEngine::MakeHash から取得する。
	/// @param InObject 取り除く条件イベントに対応する UObject を指すポインタ。
	///   取り除く条件イベントに対応する UObject のメソッド名。
	///   ただし空文字列の場合は、すべてのメソッドが対象となる。
	/// @param InFunctionName
	///   取り除く条件イベントに対応する UObject のメソッド名。
	///   ただし空文字列の場合は、すべてのメソッドが対象となる。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	void UnregisterEventsOfEachExpressionByObject(
		int32 const InExpressionKey,
		UObject const* const InObject,
		FName const InFunctionName)
	{
		if (InObject != nullptr)
		{
			this->Driver.Dispatcher.UnregisterHooks(
				InExpressionKey,
				Psyque::RulesEngine::FDelegateIdentifier(
					*InObject, InFunctionName));
		}
	}

	/// @brief 条件式と遷移条件とデリゲートに対応する条件イベントを取り除く。
	/// @param InExpressionKey
	///   取り除く条件イベントに対応する条件式を指す名前ハッシュ値。
	///   UPsyqueRulesEngine::MakeHash から取得する。
	/// @param InBeforeCondition
	///   取り除く条件イベントに対応する条件式の前回の評価。
	/// @param InLatestCondition
	///   取り除く条件イベントに対応する条件式の最新の評価。
	/// @param InDelegate 取り除く条件イベントのデリゲート。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	void UnregisterEventsOfEachCondition(
		int32 const InExpressionKey,
		EPsyqueKleene const InBeforeCondition,
		EPsyqueKleene const InLatestCondition,
		FPsyqueRulesDynamicDelegate const& InDelegate)
	{
		this->Driver.Dispatcher.UnregisterHooks(
			InExpressionKey,
			InBeforeCondition,
			InLatestCondition,
			Psyque::RulesEngine::FDelegateIdentifier(InDelegate));
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
	/// @param InChunkKey 拡張するチャンクの名前ハッシュ値。
	///   UPsyqueRulesEngine::MakeHash から取得する。
	/// @param InStatusTable 登録する状態値のもととなる UDataTable 。
	///   FPsyqueRulesStatusTableRow で構成されていること。
	/// @param InExpressionTable 登録する条件式のもととなる UDataTable 。
	///  FPsyqueRulesExpressionTableRow で構成されていること。
	/// @param InBehaviorTable 登録する条件式のもととなる UDataTable 。
	///   FPsyqueRulesHandlerTableRow で構成されていること。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	void ExtendChunkFromDataTable(
		int32 const InChunkKey,
		UDataTable const* const InStatusTable,
		UDataTable const* const InExpressionTable,
		UDataTable const* const InBehaviorTable);

	/// @brief JSON形式の文字列を解析し、チャンクを拡張する。
	/// @details
	///   JSON形式の文字列を解析し、
	///   状態値と条件式と条件挙動を1つのチャンクに登録する。
	/// @param InChunkKey 拡張するチャンクの名前ハッシュ値。
	///   UPsyqueRulesEngine::MakeHash から取得する。
	/// @param InStatusJson チャンクに登録する状態値が記述されている、
	///   JSON形式の文字列。最上位要素が配列であること。
	/// @param InExpressionJson チャンクに登録する条件式が記述されている、
	///   JSON形式の文字列。最上位要素が配列であること。
	/// @param InBehaviorJson チャンクに登録する条件挙動が記述されている、
	///   JSON形式の文字列。最上位要素が配列であること。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	void ExtendChunkFromJsonString(
		int32 const InChunkKey,
		FString const& InStatusJson,
		FString const& InExpressionJson,
		FString const& InBehaviorJson);

	/// @brief チャンクを削除する。
	/// @param InChunkKey 削除するチャンクの名前ハッシュ値。
	///   UPsyqueRulesEngine::MakeHash から取得する。
	UFUNCTION(BlueprintCallable, Category="PsyqueRulesPlugin")
	void RemoveChunk(
		int32 const InChunkKey)
	{
		this->Driver.RemoveChunk(InChunkKey);
	}
	/// @}
	//-------------------------------------------------------------------------
	public:
	/// @brief 状態値のビット構成から、状態値の型を構築する。
	/// @return 状態値の型の種類。
	/// @param InBitFormat 状態値のビット構成。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	static EPsyqueRulesStatusKind MakeStatusKind(uint8 const InBitFormat)
	{
		using FStatusValue = ThisClass::FDriver::FReservoir::FStatusValue;
		return FStatusValue::MakeKind(InBitFormat);
	}

	/// @brief 状態値の型とビット幅から、状態値のビット構成を構築する。
	/// @return 状態値のビット構成。ただしビット構成が存在しない場合は0を戻す。
	/// @param InKind 状態値の型の種類。
	/// @param InBitWidth 状態値が整数型の場合の、ビット幅。
	///   状態値が整数型以外の場合は、必ず0を指定すること。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	static uint8 MakeStatusBitFormat(
		EPsyqueRulesStatusKind const InKind,
		uint8 const InBitWidth = 0)
	{
		using FStatusValue = ThisClass::FDriver::FReservoir::FStatusValue;
		return FStatusValue::MakeBitFormat(InKind, InBitWidth);
	}

	/// @brief 状態値のビット構成から、状態値のビット幅を構築する。
	/// @return 状態値のビット幅。
	/// @param InBitFormat 状態値のビット構成。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	static uint8 MakeStatusBitWidth(uint8 const InBitFormat)
	{
		using FStatusValue = ThisClass::FDriver::FReservoir::FStatusValue;
		return FStatusValue::MakeBitWidth(InBitFormat);
	}

	/// @brief 整数型状態値のNaN値を取得する。
	/// @return 整数型状態値のNaN値。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	static int32 GetIntegerNan()
	{
		return MIN_int32;
	}

	/// @brief 浮動小数点数型状態値のNaN値を取得する。
	/// @return 浮動小数点数型状態値のNaN値。
	UFUNCTION(BlueprintPure, Category="PsyqueRulesPlugin")
	static float GetFloatNan()
	{
		return std::numeric_limits<float>::quiet_NaN();
	}

	//-------------------------------------------------------------------------
	public:
	/// @brief ルールエンジンで使用する、駆動器のインスタンス。
	ThisClass::FDriver Driver;

}; // class UPsyqueRulesEngine

// vim: set noexpandtab:
