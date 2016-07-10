// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RulesEngine::TDriver
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#if !defined(PSYQUE_RULES_ENGINE_DRIVER_CHUNK_CAPACITY_DEFAULT)
#define PSYQUE_RULES_ENGINE_DRIVER_CHUNK_CAPACITY_DEFAULT 256
#endif // !defined(PSYQUE_RULES_ENGINE_DRIVER_CHUNK_CAPACITY_DEFAULT)

#if !defined(PSYQUE_RULES_ENGINE_DRIVER_STATUS_CAPACITY_DEFAULT)
#define PSYQUE_RULES_ENGINE_DRIVER_STATUS_CAPACITY_DEFAULT 256
#endif // !defined(PSYQUE_RULES_ENGINE_DRIVER_STATUS_CAPACITY_DEFAULT)

#if !defined(PSYQUE_RULES_ENGINE_DRIVER_EXPRESSION_CAPACITY_DEFAULT)
#define PSYQUE_RULES_ENGINE_DRIVER_EXPRESSION_CAPACITY_DEFAULT 256
#endif // !defined(PSYQUE_RULES_ENGINE_DRIVER_EXPRESSION_CAPACITY_DEFAULT)

#if !defined(PSYQUE_RULES_ENGINE_DRIVER_CACHE_CAPACITY_DEFAULT)
#define PSYQUE_RULES_ENGINE_DRIVER_CACHE_CAPACITY_DEFAULT 256
#endif // !defined(PSYQUE_RULES_ENGINE_DRIVER_CACHE_CAPACITY_DEFAULT)

//#include "../string/csv_table.h"
#include "./Reservoir.h"
#include "./Accumulator.h"
#include "./Evaluator.h"
#include "./Dispatcher.h"
//#include "./HandlerChunk.h"
//#include "./HandlerBuilder.h"
#include "./StatusBuilder.h"
#include "./ExpressionBuilder.h"

/// @cond
namespace Psyque
{
	namespace RulesEngine
	{
		template<typename, typename, typename, typename, typename> class TDriver;
	} // namespace RulesEngine
} // namespace Psyque
/// @endcond
//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ

template<> struct std::hash<FName>
{
	using argument_type = FName;
	using result_type = uint32;

	uint32 operator()(FName const& InKey) const
	{
		if (InKey.GetNumber() != 0)
		{
			UE_LOG(
				LogPsyqueRulesEngine,
				Warning,
				TEXT(
					"std::hash<FName>::operator() is failed."
					"\n\tFName('%s').GetNumber() is not 0."),
				*InKey.ToString())
		}
		return static_cast<uint32>(InKey.GetDisplayIndex());
	}
};
//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ

/// if-then 規則による有限状態機械の駆動器。
///
/// @par 使い方の概略
/// - TDriver::TDriver で駆動器を構築する。
/// - TDriver::ExtendChunk で、状態値と条件式とデリゲートを登録する。
///   - 状態値の登録のみしたい場合は ThisClass::RegisterStatus を呼び出す。
///   - 条件式の登録のみしたい場合は ThisClass::Evaluator に対して
///     ThisClass::FEvaluator::RegisterExpression を呼び出す。
///   - デリゲートの登録のみしたい場合は ThisClass::Dispatcher に対して
///     FDispatcher::RegisterDelegate を呼び出す。
/// - ThisClass::Accumulator に対して
///   ThisClass::FAccumulator::Accumulate を呼び出し、状態値の変更を予約する。
/// - TDriver::Tick
///   を時間フレーム毎に呼び出す。状態値の変更と条件式の評価が行われ、
///   実行条件に合致するデリゲートが実行される。
/// .
/// @tparam TemplateUnsigned  @copydoc FReservoir::FStatusValue::FUnsigned
/// @tparam TemplateFloat     @copydoc FReservoir::FStatusValue::FFloat
/// @tparam TemplatePriority  @copydoc FDispatcher::FHook::FPriority
/// @tparam TemplateHasher    @copydoc FHasher
/// @tparam TemplateAllocator @copydoc FAllocator
template<
	typename TemplateUnsigned = uint64,
	typename TemplateFloat = float,
	typename TemplatePriority = int32,
	typename TemplateHasher = std::hash<FName>,
	typename TemplateAllocator = std::allocator<void*>>
class Psyque::RulesEngine::TDriver
{
	/// this が指す値の型。
	private: using ThisClass = TDriver;
	//-------------------------------------------------------------------------

	// @copydoc Psyque::string::_private::flyweight_factory::FHasher
	public: using FHasher = TemplateHasher;
	/// 各種コンテナに用いるメモリ割当子の型。
	public: using FAllocator = TemplateAllocator;
	//-------------------------------------------------------------------------

	/// 駆動器で用いる状態貯蔵器の型。
	public: using FReservoir = Psyque::RulesEngine::_private::TReservoir<
		TemplateUnsigned,
		TemplateFloat,
		typename ThisClass::FHasher::result_type,
		typename ThisClass::FHasher::result_type,
		typename ThisClass::FAllocator>;
	/// 駆動器で用いる状態変更器の型。
	public: using FAccumulator = Psyque::RulesEngine::_private::TAccumulator<
		typename ThisClass::FReservoir>;
	/// 駆動器で用いる条件評価器の型。
	public: using FEvaluator = Psyque::RulesEngine::_private::TEvaluator<
		typename ThisClass::FReservoir, typename ThisClass::FHasher::result_type>;
	/// 駆動器で用いるデリゲート実行器の型。
	public: using FDispatcher = Psyque::RulesEngine::_private::TDispatcher<
		typename ThisClass::FEvaluator, TemplatePriority>;
	/// チャンクの識別値を表す型。
	public: using FChunkKey = typename ThisClass::FReservoir::FChunkKey;
	//-------------------------------------------------------------------------
	/// @name 構築と代入
	/// @{

	/// 空の駆動器を構築する。
	public: TDriver():
	Reservoir(
		PSYQUE_RULES_ENGINE_DRIVER_CHUNK_CAPACITY_DEFAULT,
		PSYQUE_RULES_ENGINE_DRIVER_STATUS_CAPACITY_DEFAULT,
		ThisClass::FAllocator()),
	Accumulator(
		PSYQUE_RULES_ENGINE_DRIVER_CACHE_CAPACITY_DEFAULT, ThisClass::FAllocator()),
	Evaluator(
		PSYQUE_RULES_ENGINE_DRIVER_CHUNK_CAPACITY_DEFAULT,
		PSYQUE_RULES_ENGINE_DRIVER_EXPRESSION_CAPACITY_DEFAULT,
		ThisClass::FAllocator()),
	Dispatcher(
		PSYQUE_RULES_ENGINE_DRIVER_STATUS_CAPACITY_DEFAULT,
		PSYQUE_RULES_ENGINE_DRIVER_EXPRESSION_CAPACITY_DEFAULT,
		PSYQUE_RULES_ENGINE_DRIVER_CACHE_CAPACITY_DEFAULT,
		ThisClass::FAllocator())
	{}

	/// 空の駆動器を構築する。
	public: TDriver(
		/// [in] チャンク辞書の予約容量。
		std::size_t const InChunkCapacity,
		/// [in] 状態値辞書の予約容量。
		std::size_t const InStatusCapacity,
		/// [in] 条件式辞書の予約容量。
		std::size_t const InExpressionCapacity,
		/// [in] キャッシュの予約容量。
		std::size_t const InCacheCapacity,
		/// [in] 文字列ハッシュ関数オブジェクトの初期値。
		typename ThisClass::FHasher InHashFunction = ThisClass::FHasher(),
		/// [in] メモリ割当子の初期値。
		typename ThisClass::FAllocator const& InAllocator = ThisClass::FAllocator()):
	Reservoir(InChunkCapacity, InStatusCapacity, InAllocator),
	Accumulator(InCacheCapacity, InAllocator),
	Evaluator(InChunkCapacity, InExpressionCapacity, InAllocator),
	Dispatcher(
		InStatusCapacity, InExpressionCapacity, InCacheCapacity, InAllocator),
	HashFunction(MoveTemp(InHashFunction))
	{}

	/// 駆動器を再構築する。
	public: void Rebuild(
		/// [in] チャンク辞書の予約容量。
		std::size_t const InChunkCapacity,
		/// [in] 状態値辞書の予約容量。
		std::size_t const InStatusCapacity,
		/// [in] 条件式辞書の予約容量。
		std::size_t const InExpressionCapacity,
		/// [in] キャッシュの予約容量。
		std::size_t const InCacheCapacity =
			PSYQUE_RULES_ENGINE_DRIVER_CACHE_CAPACITY_DEFAULT)
	{
		this->Reservoir.Rebuild(InChunkCapacity, InStatusCapacity);
		//this->Accumulator.Rebuild(InCacheCapacity);
		this->Evaluator.Rebuild(InChunkCapacity, InExpressionCapacity);
		this->Dispatcher.Rebuild(
			InStatusCapacity, InExpressionCapacity, InCacheCapacity);
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name チャンク
	/// @{

	/// 状態値と条件式とデリゲートを、チャンクへ追加する。
	public: template<
		typename TemplateStatusBuilder,
		typename TemplateStatusIntermediation,
		typename TemplateExpressionBuilder,
		typename TemplateExpressionIntermediation,
		typename TemplateHandlerBuilder,
		typename TemplateHandlerIntermediation>
	void ExtendChunk(
		/// [in] 追加するチャンクの識別値。
		typename ThisClass::FChunkKey const InChunkKey,

		/// [in] 状態値を状態貯蔵器に登録する関数オブジェクト。
		/// 以下に相当するメンバ関数を使えること。
		/// @code
		///   // 中間表現を解析して状態値を構築し、状態貯蔵器に登録する。
		///   void TemplateStatusBuilder::operator()(
		///     // [in,out] 状態値を登録する状態貯蔵器。
		///     TDriver::FReservoir& OutReservoir,
		///     // [in] 文字列から識別値を生成する関数オブジェクト。
		///     TDriver::FHasher const& InHashFunction,
		///     // [in] 状態値を登録するチャンクを表す識別値。
		///     TDriver::FReservoir::FChunkKey const InChunkKey,
		///     // [in] 解析する中間表現。
		///     TemplateStatusIntermediation const& InIntermediation)
		///   const;
		/// @endcode
		TemplateStatusBuilder const& InStatusBuilder,
		/// [in] 解析する状態値の中間表現。
		TemplateStatusIntermediation const& InStatusIntermediation,

		/// [in] 条件式を条件評価器に登録する関数オブジェクト。
		/// 以下に相当するメンバ関数を使えること。
		/// @code
		///   // 文字列表を解析して条件式を構築し、条件評価器に登録する。
		///   void TemplateExpressionBuilder::operator()(
		///     // [in,out] 条件式を登録する条件評価器。
		///     TDriver::FEvaluator& OutEvaluator,
		///     // [in] 文字列から識別値を生成する関数オブジェクト。
		///     TDriver::FHasher const& InHashFunction,
		///     // [in] 条件式を登録するチャンクを表す識別値。
		///     TDriver::FEvaluator::FChunkKey const InChunkKey,
		///     // [in] 条件式で使う状態貯蔵器。
		///     TDriver::FReservoir const& InReservoir,
		///     // [in] 解析する中間表現。
		///     TemplateExpressionIntermediation const& InIntermediation)
		///   const;
		/// @endcode
		TemplateExpressionBuilder const& InExpressionBuilder,
		/// [in] 解析する条件式の中間表現。
		TemplateExpressionIntermediation const& InExpressionIntermediation,

		/// [in] デリゲートをデリゲート実行器に登録する関数オブジェクト。
		/// 以下に相当するメンバ関数を使えること。
		/// @code
		///   // 文字列表からデリゲートを構築し、デリゲート実行器に登録する。
		///   // return
		///   //   デリゲート実行器に登録したデリゲートに対応する関数を指す、
		///   //   スマートポインタのコンテナ。
		///   template<typename template_function_shared_ptr_container>
		///   template_function_shared_ptr_container TemplateHandlerBuilder::operator()(
		///     // [in,out] デリゲートを登録するデリゲート実行器。
		///     TDriver::FDispatcher& OutDispatcher,
		///     // [in] 文字列から識別値を生成する関数オブジェクト。
		///     TDriver::FHasher const& InHashFunction,
		///     // [in,out] デリゲートで使う状態変更器。
		///     TDriver::FAccumulator& OutAccumulator,
		///     // [in] 解析する中間表現。
		///     TemplateRelationTable const& InIntermediation)
		///   const;
		/// @endcode
		TemplateHandlerBuilder const& InHandlerBuilder,
		/// [in] 解析するデリゲートの中間表現。
		TemplateHandlerIntermediation const& InHandlerIntermediation)
	{
		InStatusBuilder(
			this->Reservoir,
			this->HashFunction,
			InChunkKey,
			InStatusIntermediation);
		InExpressionBuilder(
			this->Evaluator,
			this->HashFunction,
			InChunkKey,
			this->Reservoir,
			InExpressionIntermediation);
/*
		ThisClass::FHandlerChunk::Extend(
			this->HandlerChunks,
			InChunkKey,
			InHandlerBuilder(
				this->Dispatcher,
				this->HashFunction,
				this->Accumulator,
				InHandlerIntermediation));
*/
	}
/*
	/// 状態値と条件式とデリゲートを、チャンクへ追加する。
	public: template<typename TemplateString>
	void ExtendChunk(
		/// [out] 文字列表の構築の作業領域として使う文字列。
		/// std::basic_string 互換のインタフェイスを持つこと。
		TemplateString& out_workspace,
		/// [in] 文字列表の構築に使う
		/// Psyque::string::flyweight::factory の強参照。空ではないこと。
		 typename Psyque::string::flyweight<
			typename ThisClass::FHasher, typename ThisClass::FAllocator>
		::factory::shared_ptr
			const& in_string_factory,
		/// [in] 追加するチャンクの識別値。
		typename ThisClass::FChunkKey const InChunkKey,
		/// [in] 状態値CSV文字列。空文字列の場合は、状態値を追加しない。
		Psyque::String::TView<
			typename TemplateString::value_type,
			typename TemplateString::traits_type>
				const& in_status_csv,
		/// [in] 状態値CSVの属性の行番号。
		std::size_t const in_status_attribute,
		/// [in] 条件式CSV文字列。空文字列の場合は、条件式を追加しない。
		Psyque::String::TView<
			typename TemplateString::value_type,
			typename TemplateString::traits_type>
				const& in_expression_csv,
		/// [in] 条件式CSVの属性の行番号。
		std::size_t const in_expression_attribute,
		/// [in] デリゲートCSV文字列。空文字列の場合は、デリゲートを追加しない。
		Psyque::String::TView<
			typename TemplateString::value_type,
			typename TemplateString::traits_type>
				const& in_handler_csv,
		/// [in] デリゲートCSVの属性の行番号。
		std::size_t const in_handler_attribute)
	{
		using csv_table = Psyque::string::csv_table<
			std::size_t, 
			typename ThisClass::FHasher,
			typename ThisClass::FAllocator>
		this->ExtendChunk(
			InChunkKey,
			Psyque::RulesEngine::TStatusBuilder(),
			csv_table::build_relation_table(
				out_workspace,
				in_string_factory,
				in_status_csv,
				in_status_attribute),
			Psyque::RulesEngine::expression_builder(),
			csv_table::build_relation_table(
				out_workspace,
				in_string_factory,
				in_expression_csv,
				in_expression_attribute),
			Psyque::RulesEngine::handler_builder(),
			csv_table::build_relation_table(
				out_workspace,
				in_string_factory,
				in_handler_csv,
				in_handler_attribute));
	}
*/
	/// チャンクを削除する。
	public: void RemoveChunk(
		/// [in] 削除するチャンクの識別値。
		typename ThisClass::FChunkKey const InChunkKey)
	{
		this->Reservoir.RemoveChunk(InChunkKey);
		this->Evaluator.RemoveChunk(InChunkKey);
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 状態値
	/// @{

	/// 状態貯蔵器を取得する。
	/// @return *this が持つ状態貯蔵器。
	public: typename ThisClass::FReservoir const& GetReservoir()
	const PSYQUE_NOEXCEPT
	{
		return this->Reservoir;
	}

	/// 状態値を登録する。
	/// @sa
	/// - 登録した状態値を取得するには、 ThisClass::GetReservoir から
	///   FReservoir::FindStatus を呼び出す。
	/// - 状態値の変更は ThisClass::Accumulator から
	///   FAccumulator::Accumulate を呼び出して行う。
	/// - ThisClass::RemoveChunk で、登録した状態値をチャンク毎に削除できる。
	/// @retval true  成功。状態値を登録した。
	/// @retval false 失敗。状態値は登録されなかった。
	/// - InStatusKey に対応する状態値がすでに登録されていると失敗する。
	public: template<typename TemplateValue>
	bool RegisterStatus(
		/// [in] 登録する状態値を格納する状態値ビット列チャンクの識別値。
		typename ThisClass::FChunkKey const InChunkKey,
		/// [in] 登録する状態値の識別値。
		typename ThisClass::FReservoir::FStatusKey const InStatusKey,
		/// [in] 登録する状態値の初期値。以下の型の値を登録できる。
		/// - bool 型。
		/// - C++ 組み込み整数型。
		/// - C++ 組み込み浮動小数点数型。
		TemplateValue const InValue)
	{
		return this->Reservoir.RegisterStatus(InChunkKey, InStatusKey, InValue);
	}

	/// 整数型の状態値を登録する。
	/// @sa
	/// - 登録した状態値を取得するには、 ThisClass::GetReservoir から
	///   FReservoir::FindStatus を呼び出す。
	/// - 登録した状態値を書き換えるには、 ThisClass::Accumulator から
	///   FAccumulator::Accumulate を呼び出す。
	/// - ThisClass::RemoveChunk で、登録した状態値をチャンク毎に削除できる。
	/// @retval true  成功。状態値を登録した。
	/// @retval false 失敗。状態値は登録されなかった。
	/// - InStatusKey に対応する状態値がすでに登録されていると失敗する。
	/// - InValue のビット幅が InBitWidth を超えていると失敗する。
	/// - FReservoir::FStatusChunk::BLOCK_BIT_WIDTH より
	///   InBitWidth が大きいと失敗する。
	/// - InBitWidth が2未満だと失敗する。
	///   1ビットの値は論理型として登録すること。
	public: template<typename TemplateValue>
	bool RegisterStatus(
		/// [in] 登録する状態値を格納する状態値ビット列チャンクの識別値。
		typename ThisClass::FChunkKey const InChunkKey,
		/// [in] 登録する状態値の識別値。
		typename ThisClass::FReservoir::FStatusKey const InStatusKey,
		/// [in] 登録する状態値の初期値。 C++ 組み込み整数型であること。
		TemplateValue const InValue,
		/// [in] 登録する状態値のビット幅。
		std::size_t const InBitWidth)
	{
		return this->Reservoir.RegisterStatus(
			InChunkKey, InStatusKey, InValue, InBitWidth);
	}

	/// 状態値を更新し、条件式を評価して、条件に合致するデリゲートを実行する。
	/// @details 基本的には、時間フレーム毎に呼び出すこと。
	public: void Tick()
	{
		this->Accumulator._flush(this->Reservoir);
		this->Dispatcher._dispatch(this->Reservoir, this->Evaluator);
	}
	/// @}
	//-------------------------------------------------------------------------

	/// 駆動器で用いる状態貯蔵器。
	private: typename ThisClass::FReservoir Reservoir;

	/// 駆動器で用いる状態変更器。
	public: typename ThisClass::FAccumulator Accumulator;

	/// 駆動器で用いる条件評価器。
	public: typename ThisClass::FEvaluator Evaluator;

	/// 駆動器で用いるデリゲート実行器。
	public: typename ThisClass::FDispatcher Dispatcher;

	/// 駆動器で用いる文字列ハッシュ関数オブジェクト。
	public: typename ThisClass::FHasher HashFunction;

}; // class Psyque::RulesEngine::TDriver

// vim: set noexpandtab:
