// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RuleEngine::TDriver
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#ifndef PSYQUE_IF_THEN_ENGINE_DRIVER_CACHE_CAPACITY_DEFAULT
#define PSYQUE_IF_THEN_ENGINE_DRIVER_CACHE_CAPACITY_DEFAULT 64
#endif // !defined(PSYQUE_IF_THEN_ENGINE_DRIVER_CACHE_CAPACITY_DEFAULT)

//#include "../string/csv_table.h"
#include "./Reservoir.h"
#include "./Accumulator.h"
#include "./Evaluator.h"
#include "./Dispatcher.h"
#include "./HandlerChunk.h"
//#include "./HandlerBuilder.h"
#include "./StatusBuilder.h"
//#include "./ExpressionBuilder.h"

/// @cond
namespace Psyque
{
	namespace RuleEngine
	{
		template<typename, typename, typename, typename, typename> class TDriver;
	} // namespace RuleEngine
} // namespace Psyque
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<> struct std::hash<FName>
{
	using argument_type = FName;
	using result_type = std::size_t;

	std::size_t operator()(FName const& InKey) const
	{
		check(InKey.GetNumber() == 0);
		return static_cast<uint32>(InKey.GetDisplayIndex());
	}
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief if-then 規則による有限状態機械の駆動器。
/// @par 使い方の概略
/// - TDriver::TDriver で駆動器を構築する。
/// - TDriver::ExtendChunk で、状態値と条件式と条件挙動関数を登録する。
///   - 状態値の登録のみしたい場合は This::RegisterStatus を呼び出す。
///   - 条件式の登録のみしたい場合は This::Evaluator に対して
///     This::FEvaluator::RegisterExpression を呼び出す。
///   - 条件挙動関数の登録のみしたい場合は TDriver::RegisterHandler を呼び出す。
/// - This::Accumulator に対して
///   This::FAccumulator::Accumulate を呼び出し、状態値の変更を予約する。
/// - TDriver::Tick
///   を時間フレーム毎に呼び出す。状態値の変更と条件式の評価が行われ、
///   挙動条件に合致する条件挙動関数が呼び出される。
/// .
/// @tparam TemplateUnsigned  @copydoc FReservoir::FStatusValue::FUnsigned
/// @tparam TemplateFloat     @copydoc FReservoir::FStatusValue::FFloat
/// @tparam TemplatePriority  @copydoc FDispatcher::FHandler::FPriority
/// @tparam TemplateHasher    @copydoc FHasher
/// @tparam TemplateAllocator @copydoc FAllocator
template<
	typename TemplateUnsigned = uint64,
	typename TemplateFloat = float,
	typename TemplatePriority = int32,
	typename TemplateHasher = std::hash<FName>,
	typename TemplateAllocator = std::allocator<void*>>
class Psyque::RuleEngine::TDriver
{
	/// @brief this が指す値の型。
	private: using This = TDriver;

	//-------------------------------------------------------------------------
	// @copydoc Psyque::string::_private::flyweight_factory::FHasher
	public: using FHasher = TemplateHasher;
	/// @brief 各種コンテナに用いるメモリ割当子の型。
	public: using FAllocator = TemplateAllocator;

	//-------------------------------------------------------------------------
	/// @brief 駆動器で用いる状態貯蔵器の型。
	public: using FReservoir = Psyque::RuleEngine::_private::TReservoir<
		TemplateUnsigned,
		TemplateFloat,
		typename This::FHasher::result_type,
		typename This::FHasher::result_type,
		typename This::FAllocator>;
	/// @brief 駆動器で用いる状態変更器の型。
	public: using FAccumulator = Psyque::RuleEngine::_private::TAccumulator<
		typename This::FReservoir>;
	/// @brief 駆動器で用いる条件評価器の型。
	public: using FEvaluator = Psyque::RuleEngine::_private::TEvaluator<
		typename This::FReservoir, typename This::FHasher::result_type>;
	/// @brief 駆動器で用いる条件挙動器の型。
	public: using FDispatcher = Psyque::RuleEngine::_private::TDispatcher<
		typename This::FEvaluator, TemplatePriority>;
	/// @brief チャンクの識別値を表す型。
	public: using FChunkKey = typename This::FReservoir::FChunkKey;

	//-------------------------------------------------------------------------
	/// @brief 駆動器で用いる条件挙動チャンクの型。
	private: using FHandlerChunk = Psyque::RuleEngine::_private::THandlerChunk<
		typename This::FDispatcher>;

	//-------------------------------------------------------------------------
	/// @name 構築と代入
	/// @{

	/// @brief 空の駆動器を構築する。
	public: TDriver(
		/// [in] チャンク辞書の予約容量。
		std::size_t const InChunkCapacity,
		/// [in] 状態値辞書の予約容量。
		std::size_t const InStatusCapacity,
		/// [in] 条件式辞書の予約容量。
		std::size_t const InExpressionCapacity,
		/// [in] キャッシュの予約容量。
		std::size_t const InCacheCapacity =
			PSYQUE_IF_THEN_ENGINE_DRIVER_CACHE_CAPACITY_DEFAULT,
		/// [in] 文字列ハッシュ関数オブジェクトの初期値。
		typename This::FHasher InHashFunction = This::FHasher(),
		/// [in] メモリ割当子の初期値。
		typename This::FAllocator const& InAllocator = This::FAllocator()):
	Reservoir(InChunkCapacity, InStatusCapacity, InAllocator),
	Accumulator(InCacheCapacity, InAllocator),
	Evaluator(InChunkCapacity, InExpressionCapacity, InAllocator),
	Dispatcher(
		InStatusCapacity, InExpressionCapacity, InCacheCapacity, InAllocator),
	HandlerChunks(InAllocator),
	HashFunction(MoveTemp(InHashFunction))
	{
		this->HandlerChunks.reserve(InChunkCapacity);
	}

#ifdef PSYQUE_NO_STD_DEFAULTED_FUNCTION
	/// @brief ムーブ構築子。
	public: TDriver(
		/// [in,out] ムーブ元となるインスタンス。
		This&& OutSource):
	Reservoir(MoveTemp(OutSource.Reservoir)),
	Accumulator(MoveTemp(OutSource.Accumulator)),
	Evaluator(MoveTemp(OutSource.Evaluator)),
	Dispatcher(MoveTemp(OutSource.Dispatcher)),
	HandlerChunks(MoveTemp(OutSource.HandlerChunks)),
	HashFunction(MoveTemp(OutSource.HashFunction))
	{}

	/// @brief ムーブ代入演算子。
	/// @return *this
	public: This& operator=(
		/// [in,out] ムーブ元となるインスタンス。
		This&& OutSource)
	{
		this->Reservoir = MoveTemp(OutSource.Reservoir);
		this->Accumulator = MoveTemp(OutSource.Accumulator);
		this->Evaluator = MoveTemp(OutSource.Evaluator);
		this->Dispatcher = MoveTemp(OutSource.Dispatcher);
		this->HandlerChunks = MoveTemp(OutSource.HandlerChunks);
		this->HashFunction = MoveTemp(OutSource.HashFunction);
		return *this;
	}
#endif // defined(PSYQUE_NO_STD_DEFAULTED_FUNCTION)

	/// @brief 駆動器を再構築する。
	public: void Rebuild(
		/// [in] チャンク辞書の予約容量。
		std::size_t const InChunkCapacity,
		/// [in] 状態値辞書の予約容量。
		std::size_t const InStatusCapacity,
		/// [in] 条件式辞書の予約容量。
		std::size_t const InExpressionCapacity,
		/// [in] キャッシュの予約容量。
		std::size_t const InCacheCapacity =
			PSYQUE_IF_THEN_ENGINE_DRIVER_CACHE_CAPACITY_DEFAULT)
	{
		this->Reservoir.Rebuild(InChunkCapacity, InStatusCapacity);
		//this->Accumulator.Rebuild(InCacheCapacity);
		this->Evaluator.Rebuild(InChunkCapacity, InExpressionCapacity);
		this->Dispatcher.Rebuild(
			InStatusCapacity, InExpressionCapacity, InCacheCapacity);
		this->HandlerChunks.shrink_to_fit();
		for (auto& LocalHandlerChunk: this->HandlerChunks)
		{
			LocalHandlerChunk.shrink_to_fit();
		}
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name チャンク
	/// @{

	/// @brief 状態値と条件式と条件挙動ハンドラを、チャンクへ追加する。
	public: template<
		typename TemplateStatusBuilder,
		typename TemplateStatusIntermediation,
		typename TemplateExpressionBuilder,
		typename TemplateExpressionIntermediation,
		typename TemplateHandlerBuilder,
		typename TemplateHandlerIntermediation>
	void ExtendChunk(
		/// [in] 追加するチャンクの識別値。
		typename This::FChunkKey const InChunkKey,

		/// [in] 状態値を状態貯蔵器に登録する関数オブジェクト。
		/// 以下に相当するメンバ関数を使えること。
		/// @code
		///   // brief 中間表現を解析して状態値を構築し、状態貯蔵器に登録する。
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
		///   // brief 文字列表を解析して条件式を構築し、条件評価器に登録する。
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

		/// [in] 条件挙動ハンドラを条件挙動器に登録する関数オブジェクト。
		/// 以下に相当するメンバ関数を使えること。
		/// @code
		///   // brief 文字列表から条件挙動ハンドラを構築し、条件挙動器に登録する。
		///   // return
		///   //   条件挙動器に登録した条件挙動ハンドラに対応する関数を指す、
		///   //   スマートポインタのコンテナ。
		///   template<typename template_function_shared_ptr_container>
		///   template_function_shared_ptr_container TemplateHandlerBuilder::operator()(
		///     // [in,out] 条件挙動ハンドラを登録する条件挙動器。
		///     TDriver::FDispatcher& OutDispatcher,
		///     // [in] 文字列から識別値を生成する関数オブジェクト。
		///     TDriver::FHasher const& InHashFunction,
		///     // [in,out] 条件挙動ハンドラで使う状態変更器。
		///     TDriver::FAccumulator& OutAccumulator,
		///     // [in] 解析する中間表現。
		///     TemplateRelationTable const& InIntermediation)
		///   const;
		/// @endcode
		TemplateHandlerBuilder const& InHandlerBuilder,
		/// [in] 解析する条件挙動ハンドラの中間表現。
		TemplateHandlerIntermediation const& InHandlerIntermediation)
	{
		InStatusBuilder(
			this->Reservoir,
			this->HashFunction,
			InChunkKey,
			InStatusIntermediation);
/*
		InExpressionBuilder(
			this->Evaluator,
			this->HashFunction,
			InChunkKey,
			this->Reservoir,
			InExpressionIntermediation);
		This::FHandlerChunk::Extend(
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
	/// @brief 状態値と条件式と条件挙動ハンドラを、チャンクへ追加する。
	public: template<typename TemplateString>
	void ExtendChunk(
		/// [out] 文字列表の構築の作業領域として使う文字列。
		/// std::basic_string 互換のインタフェイスを持つこと。
		TemplateString& out_workspace,
		/// [in] 文字列表の構築に使う
		/// Psyque::string::flyweight::factory の強参照。空ではないこと。
		 typename Psyque::string::flyweight<
			typename This::FHasher, typename This::FAllocator>
		::factory::shared_ptr
			const& in_string_factory,
		/// [in] 追加するチャンクの識別値。
		typename This::FChunkKey const InChunkKey,
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
		/// [in] 条件挙動CSV文字列。空文字列の場合は、条件挙動ハンドラを追加しない。
		Psyque::String::TView<
			typename TemplateString::value_type,
			typename TemplateString::traits_type>
				const& in_handler_csv,
		/// [in] 条件挙動CSVの属性の行番号。
		std::size_t const in_handler_attribute)
	{
		using csv_table = Psyque::string::csv_table<
			std::size_t, 
			typename This::FHasher,
			typename This::FAllocator>
		this->ExtendChunk(
			InChunkKey,
			Psyque::RuleEngine::TStatusBuilder(),
			csv_table::build_relation_table(
				out_workspace,
				in_string_factory,
				in_status_csv,
				in_status_attribute),
			Psyque::RuleEngine::expression_builder(),
			csv_table::build_relation_table(
				out_workspace,
				in_string_factory,
				in_expression_csv,
				in_expression_attribute),
			Psyque::RuleEngine::handler_builder(),
			csv_table::build_relation_table(
				out_workspace,
				in_string_factory,
				in_handler_csv,
				in_handler_attribute));
	}
*/
	/// @brief チャンクを削除する。
	public: void RemoveChunk(
		/// [in] 削除するチャンクの識別値。
		typename This::FChunkKey const InChunkKey)
	{
		this->Reservoir.RemoveChunk(InChunkKey);
		this->Evaluator.RemoveChunk(InChunkKey);
		This::FHandlerChunk::erase(this->HandlerChunks, InChunkKey);
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 条件挙動ハンドラ
	/// @{

	/// @brief 条件挙動ハンドラを登録し、条件挙動関数を強参照する。
	/// @sa
	///   This::Tick で、 InExpressionKey に対応する条件式の評価が変化し
	///   InCondition と合致すると、 InFunction の指す条件挙動関数が呼び出される。
	/// @sa
	///   InFunction の指す条件挙動関数が解体されると、それを弱参照している
	///   FDispatcher::FHandler は自動的に削除される。明示的に削除するには、
	///   This::Dispatcher に対して FDispatcher::UnregisterHandler を使う。
	/// @retval true
	///   成功。 InFunction の指す条件挙動関数を弱参照する
	///   FDispatcher::FHandler を構築して This::Dispatcher
	///   に登録し、登録した条件挙動関数の強参照をチャンクに追加した。
	/// @retval false
	///   失敗。 FDispatcher::FHandler は構築されず、
	///   条件挙動関数の強参照はチャンクに追加されなかった。
	///   - InCondition が
	///     FDispatcher::FHandler::EUnitCondition::Invalid だと、失敗する。
	///   - InFunction が空か、空の関数を指していると、失敗する。
	///   - InExpressionKey と対応する FDispatcher::FHandler に、
	///     InFunction の指す条件挙動関数が既に登録されていると、失敗する。
	public: bool RegisterHandler(
		/// [in] 条件挙動関数を追加するチャンクの識別値。
		typename This::FChunkKey const InChunkKey,
		/// [in] InFunction の指す条件挙動関数に対応する
		/// FEvaluator::FExpression の識別値。
		typename This::FEvaluator::FExpressionKey const InExpressionKey,
		/// [in] InFunction の指す条件挙動関数を呼び出す挙動条件。
		/// FDispatcher::FHandler::MakeCondition から作る。
		typename This::FDispatcher::FHandler::FCondition const InCondition,
		/// [in] 登録する FDispatcher::FHandler::FFunction を指すスマートポインタ。
		/// InExpressionKey に対応する条件式の評価が変化して
		/// InCondition に合致すると、呼び出される。
		typename This::FDispatcher::FHandler::FFunctionSharedPtr InFunction,
		/// [in] InFunction の指す条件挙動関数の呼び出し優先順位。
		/// 昇順に呼び出される。
		typename This::FDispatcher::FHandler::FPriority const InPriority =
			PSYQUE_IF_THEN_ENGINE_DISPATCHER_FUNCTION_PRIORITY_DEFAULT)
	{
		// 条件挙動関数を条件挙動器へ登録する。
		auto const local_register_handler(
			this->Dispatcher.RegisterHandler(
				InExpressionKey, InCondition, InFunction, InPriority));
		if (local_register_handler)
		{
			// 条件挙動関数を条件挙動チャンクへ追加する。
			This::FHandlerChunk::Extend(
				this->HandlerChunks, InChunkKey, MoveTemp(InFunction));
		}
		return local_register_handler;
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 状態値
	/// @{

	/// @brief 状態貯蔵器を取得する。
	/// @return *this が持つ状態貯蔵器。
	public: typename This::FReservoir const& GetReservoir()
	const PSYQUE_NOEXCEPT
	{
		return this->Reservoir;
	}

	/// @brief 状態値を登録する。
	/// @sa
	/// - 登録した状態値を取得するには、 This::GetReservoir から
	///   FReservoir::FindStatus を呼び出す。
	/// - 状態値の変更は This::Accumulator から
	///   FAccumulator::Accumulate を呼び出して行う。
	/// - This::RemoveChunk で、登録した状態値をチャンク毎に削除できる。
	/// @retval true  成功。状態値を登録した。
	/// @retval false 失敗。状態値は登録されなかった。
	/// - InStatusKey に対応する状態値がすでに登録されていると失敗する。
	public: template<typename TemplateValue>
	bool RegisterStatus(
		/// [in] 登録する状態値を格納する状態値ビット列チャンクの識別値。
		typename This::FChunkKey const InChunkKey,
		/// [in] 登録する状態値の識別値。
		typename This::FReservoir::FStatusKey const InStatusKey,
		/// [in] 登録する状態値の初期値。以下の型の値を登録できる。
		/// - bool 型。
		/// - C++ 組み込み整数型。
		/// - C++ 組み込み浮動小数点数型。
		TemplateValue const InValue)
	{
		return this->Reservoir.RegisterStatus(
			InChunkKey, InStatusKey, InValue);
	}

	/// @brief 整数型の状態値を登録する。
	/// @sa
	/// - 登録した状態値を取得するには、 This::GetReservoir から
	///   FReservoir::FindStatus を呼び出す。
	/// - 登録した状態値を書き換えるには、 This::Accumulator から
	///   FAccumulator::Accumulate を呼び出す。
	/// - This::RemoveChunk で、登録した状態値をチャンク毎に削除できる。
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
		typename This::FChunkKey const InChunkKey,
		/// [in] 登録する状態値の識別値。
		typename This::FReservoir::FStatusKey const InStatusKey,
		/// [in] 登録する状態値の初期値。 C++ 組み込み整数型であること。
		TemplateValue const InValue,
		/// [in] 登録する状態値のビット幅。
		std::size_t const InBitWidth)
	{
		return this->Reservoir.RegisterStatus(
			InChunkKey, InStatusKey, InValue, InBitWidth);
	}

	/// @brief 状態値を更新し、条件式を評価して、条件挙動関数を呼び出す。
	/// @details 基本的には、時間フレーム毎に呼び出すこと。
	public: void Tick()
	{
		this->Accumulator._flush(this->Reservoir);
		this->Dispatcher._dispatch(this->Reservoir, this->Evaluator);
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @brief 駆動器で用いる状態貯蔵器。
	private: typename This::FReservoir Reservoir;
	/// @brief 駆動器で用いる状態変更器。
	public: typename This::FAccumulator Accumulator;
	/// @brief 駆動器で用いる条件評価器。
	public: typename This::FEvaluator Evaluator;
	/// @brief 駆動器で用いる条件挙動器。
	public: typename This::FDispatcher Dispatcher;
	/// @brief 駆動器で用いる条件挙動チャンクのコンテナ。
	private: typename This::FHandlerChunk::FArray HandlerChunks;
	/// @brief 駆動器で用いる文字列ハッシュ関数オブジェクト。
	public: typename This::FHasher HashFunction;

}; // class Psyque::RuleEngine::TDriver

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace PsyqueTest
{
	inline void RuleEngine()
	{
		auto const LocalStatusJson(
			StaticCastSharedRef<TJsonReader<>>(
				FJsonStringReader::Create(
					TEXT(
						"{"
						"  \"bool\": \"TRUE\","
						"  \"signed\": -12,"
						"  \"unsigned\": 12,"
						"  \"float\": 0.5,"
						"}"))));

		using FDriver = Psyque::RuleEngine::TDriver<>;
		FDriver LocalDriver(256, 256, 256);

		FName const LocalChunkName(TEXT("PsyqueTest"));
		LocalDriver.ExtendChunk(
			LocalDriver.HashFunction(LocalChunkName),
			Psyque::RuleEngine::TStatusBuilder(),
			LocalStatusJson,
			Psyque::RuleEngine::TStatusBuilder(),
			*LocalStatusJson,
			Psyque::RuleEngine::TStatusBuilder(),
			*LocalStatusJson);

		FName const LocalBoolName(TEXT("Bool"));
		auto const LocalBool(true);
		check(
			LocalDriver.RegisterStatus(
				LocalDriver.HashFunction(LocalChunkName),
				LocalDriver.HashFunction(LocalBoolName),
				LocalBool));

		FName const LocalIntegerName(TEXT("Integer"));
		uint8 const LocalInteger(10);
		check(
			LocalDriver.RegisterStatus(
				LocalDriver.HashFunction(LocalChunkName),
				LocalDriver.HashFunction(LocalIntegerName),
				LocalInteger));

		FName const LocalFloatName(TEXT("Float"));
		auto const LocalFloat(0.5f);
		check(
			LocalDriver.RegisterStatus(
				LocalDriver.HashFunction(LocalChunkName),
				LocalDriver.HashFunction(LocalFloatName),
				LocalFloat));

		auto const LocalEmpyStatus(
			LocalDriver.GetReservoir().FindStatus(
				LocalDriver.HashFunction(LocalChunkName)));
		check(LocalEmpyStatus.IsEmpty());

		auto const LocalBoolStatus(
			LocalDriver.GetReservoir().FindStatus(
				LocalDriver.HashFunction(LocalBoolName)));
		check(
			0 < LocalBoolStatus.Compare(
				FDriver::FReservoir::FStatusValue::EComparison::Equal,
				LocalBool));

		auto const LocalIntegerStatus(
			LocalDriver.GetReservoir().FindStatus(
				LocalDriver.HashFunction(LocalIntegerName)));
		check(
			0 < LocalIntegerStatus.Compare(
				FDriver::FReservoir::FStatusValue::EComparison::Equal,
				LocalInteger));

		auto const LocalFloatStatus(
			LocalDriver.GetReservoir().FindStatus(
				LocalDriver.HashFunction(LocalFloatName)));
		check(
			0 < LocalFloatStatus.Compare(
				FDriver::FReservoir::FStatusValue::EComparison::Equal,
				LocalFloat));

		LocalDriver.Tick();
		LocalDriver.RemoveChunk(LocalDriver.HashFunction(LocalChunkName));
	}
}

// vim: set noexpandtab:
