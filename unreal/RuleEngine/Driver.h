// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RuleEngine::TDriver
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#ifndef PSYQUE_IF_THEN_ENGINE_DRIVER_CACHE_CAPACITY_DEFAULT
#define PSYQUE_IF_THEN_ENGINE_DRIVER_CACHE_CAPACITY_DEFAULT 64
#endif // !defined(PSYQUE_IF_THEN_ENGINE_DRIVER_CACHE_CAPACITY_DEFAULT)

//#include "../string/csv_table.hpp"
#include "./Reservoir.h"
//#include "./accumulator.hpp"
#include "./Evaluator.h"
//#include "./dispatcher.hpp"
//#include "./handler_chunk.hpp"
//#include "./handler_builder.hpp"
//#include "./status_builder.hpp"
//#include "./expression_builder.hpp"

/// @cond
namespace Psyque
{
	namespace RuleEngine
	{
		template<typename, typename, typename, typename, typename>
			class TDriver;
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
/// - TDriver::TDriver で駆動器のインスタンスを構築する。
/// - TDriver::extend_chunk で、状態値と条件式と条件挙動関数を登録する。
///   - 状態値の登録のみしたい場合は TDriver::RegisterStatus を呼び出す。
///   - 条件式の登録のみしたい場合は TDriver::Evaluator に対して
///     \_private::TEvaluator::RegisterExpression を呼び出す。
///   - 条件挙動関数の登録のみしたい場合は TDriver::register_handler を呼び出す。
/// - TDriver::accumulator_ に対して
///   accumulator::accumulate を呼び出し、状態値の変更を予約する。
/// - TDriver::Tick を時間フレーム毎に呼び出す。状態値の変更と条件式の評価が行われ、
///   挙動条件に合致する条件挙動関数が呼び出される。
///.
/// @tparam TemplateUnsigned     @copydoc _private::TStatusValue::FUnsigned
/// @tparam TemplateFloat        @copydoc _private::TStatusValue::FFloat
/// @tparam TemplatePriority     @copydoc dispatcher::handler::priority
/// @tparam TemplateHasher       @copydoc FHasher
/// @tparam TemplateSetAllocator @copydoc FSetAllocator
template<
	typename TemplateUnsigned = uint64,
	typename TemplateFloat = float,
	typename TemplatePriority = int32,
	typename TemplateHasher = std::hash<FName>,
	typename TemplateSetAllocator = FDefaultSetAllocator>
class Psyque::RuleEngine::TDriver
{
	using This = TDriver; ///< this が指す値の型。

	//-------------------------------------------------------------------------
	public:
//	/// @copydoc Psyque::string::_private::flyweight_factory::FHasher
	using FHasher = TemplateHasher;
	/// @copydoc _private::TReservoir::FSetAllocator
	using FSetAllocator = TemplateSetAllocator;

	//-------------------------------------------------------------------------
	public:
	/// @brief 駆動器で用いる状態貯蔵器の型。
	using FReservoir = Psyque::RuleEngine::_private::TReservoir<
		TemplateUnsigned,
		TemplateFloat,
		typename TemplateHasher::result_type,
		typename TemplateHasher::result_type,
		TemplateSetAllocator>;
/*
	/// @brief 駆動器で用いる状態変更器の型。
	public: typedef
		Psyque::RuleEngine::_private::accumulator<
			typename This::FReservoir>
		accumulator;
*/
	/// @brief 駆動器で用いる条件評価器の型。
	using FEvaluator = Psyque::RuleEngine::_private::TEvaluator<
		typename This::FReservoir, typename TemplateHasher::result_type>;
/*
	/// @brief 駆動器で用いる条件挙動器の型。
	public: typedef
		Psyque::RuleEngine::_private::dispatcher<
			typename This::FEvaluator, TemplatePriority>
		dispatcher;
*/
	/// @brief チャンクの識別値を表す型。
	using FChunkKey = typename This::FReservoir::FChunkKey;

	//-------------------------------------------------------------------------
/*
	/// @brief 駆動器で用いる条件挙動チャンクの型。
	private: typedef
		Psyque::RuleEngine::_private::handler_chunk<
			typename This::dispatcher>
		handler_chunk;
*/

	//-------------------------------------------------------------------------
	/// @name 構築
	/// @{
	public:
	/// @brief 空の駆動器を構築する。
	TDriver(
		/// [in] チャンク辞書の予約容量。
		int32 const InChunkCapacity,
		/// [in] 状態値辞書の予約容量。
		int32 const InStatusCapacity,
		/// [in] 条件式辞書の予約容量。
		int32 const InExpressionCapacity,
		/// [in] キャッシュの予約容量。
		int32 const InCacheCapacity =
			PSYQUE_IF_THEN_ENGINE_DRIVER_CACHE_CAPACITY_DEFAULT,
		/// [in] 文字列ハッシュ関数オブジェクトの初期値。
		TemplateHasher InHashFunction = TemplateHasher()):
	Reservoir(InChunkCapacity, InStatusCapacity),
//	accumulator_(InCacheCapacity, in_allocator),
	Evaluator(InChunkCapacity, InExpressionCapacity),
/*
	dispatcher_(
		InStatusCapacity,
		InExpressionCapacity,
		InCacheCapacity,
		in_allocator),
	handler_chunks_(in_allocator),
*/
	HashFunction(MoveTemp(InHashFunction))
	{
//		this->handler_chunks_.reserve(InChunkCapacity);
	}

	/// @brief 駆動器を再構築する。
	void Rebuild(
		/// [in] チャンク辞書の予約容量。
		int32 const InChunkCapacity,
		/// [in] 状態値辞書の予約容量。
		int32 const InStatusCapacity,
		/// [in] 条件式辞書の予約容量。
		int32 const InExpressionCapacity,
		/// [in] キャッシュの予約容量。
		int32 const InCacheCapacity =
			PSYQUE_IF_THEN_ENGINE_DRIVER_CACHE_CAPACITY_DEFAULT)
	{
		this->Reservoir.Rebuild(InChunkCapacity, InStatusCapacity);
//		this->accumulator_.rebuild(InCacheCapacity);
		this->Evaluator.Rebuild(InChunkCapacity, InExpressionCapacity);
/*
		this->dispatcher_.rebuild(
			InStatusCapacity, InExpressionCapacity, InCacheCapacity);
		this->handler_chunks_.shrink_to_fit();
		for (auto& local_handler_chunk: this->handler_chunks_)
		{
			local_handler_chunk.shrink_to_fit();
		}
*/
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name チャンク
	/// @{
	public:
/*
	/// @brief 状態値と条件式と条件挙動ハンドラを、チャンクへ追加する。
	public: template<
		typename template_status_builder,
		typename template_expression_builder,
		typename template_handler_builder,
		typename template_relation_table>
	void extend_chunk(
		/// [in] 追加するチャンクの識別値。
		typename This::FChunkKey const& InChunkKey,

		/// [in] 状態値を状態貯蔵器に登録する関数オブジェクト。
		/// 以下に相当するメンバ関数を使えること。
		/// @code
		/// // brief 文字列表を解析して状態値を構築し、状態貯蔵器に登録する。
		/// void template_status_builder::operator()(
		///   // [in,out] 状態値を登録する状態貯蔵器。
		///   TDriver::FReservoir& io_Reservoir,
		///   // [in,out] 文字列から識別値を生成する関数オブジェクト。
		///   TDriver::FHasher& io_hasher,
		///   // [in] 状態値を登録するチャンクを表す識別値。
		///   TDriver::FReservoir::FChunkKey const& InChunkKey,
		///   // [in] 解析する文字列表。
		///   template_relation_table const& in_table)
		/// const;
		/// @endcode
		template_status_builder const& in_status_builder,
		/// [in] 状態値が記述されている Psyque::string::relation_table 。
		/// 文字列表が空の場合は、状態値を追加しない。
		template_relation_table const& in_status_table,

		/// [in] 条件式を条件評価器に登録する関数オブジェクト。
		/// 以下に相当するメンバ関数を使えること。
		/// @code
		/// // brief 文字列表を解析して条件式を構築し、条件評価器に登録する。
		/// void template_expression_builder::operator()(
		///   // [in,out] 条件式を登録する条件評価器。
		///   TDriver::FEvaluator& io_evaluator,
		///   // [in,out] 文字列から識別値を生成する関数オブジェクト。
		///   TDriver::FHasher& io_hasher,
		///   // [in] 条件式を登録するチャンクを表す識別値。
		///   TDriver::FEvaluator::FChunkKey const& InChunkKey,
		///   // [in] 条件式で使う状態貯蔵器。
		///   TDriver::FReservoir const& in_Reservoir,
		///   // [in] 解析する文字列表。
		///   template_relation_table const& in_table)
		/// const;
		/// @endcode
		template_expression_builder const& in_expression_builder,
		/// [in] 条件式が記述されている Psyque::string::relation_table 。
		/// 文字列表が空の場合は、条件式を追加しない。
		template_relation_table const& in_expression_table,

		/// [in] 条件挙動ハンドラを条件挙動器に登録する関数オブジェクト。
		/// 以下に相当するメンバ関数を使えること。
		/// @code
		/// // brief 文字列表から条件挙動ハンドラを構築し、条件挙動器に登録する。
		/// // return
		/// // 条件挙動器に登録した条件挙動ハンドラに対応する関数を指す、
		/// // スマートポインタのコンテナ。
		/// template<typename template_function_shared_ptr_container>
		/// template_function_shared_ptr_container template_handler_builder::operator()(
		///   // [in,out] 条件挙動ハンドラを登録する条件挙動器。
		///   TDriver::dispatcher& io_dispatcher,
		///   // [in,out] 文字列から識別値を生成する関数オブジェクト。
		///   TDriver::FHasher& io_hasher,
		///   // [in,out] 条件挙動ハンドラで使う状態変更器。
		///   TDriver::accumulator& io_accumulator,
		///   // [in] 解析する文字列表。
		///   template_relation_table const& in_table)
		/// const;
		/// @endcode
		template_handler_builder const& in_handler_builder,
		/// [in] 条件挙動ハンドラが記述されている Psyque::string::relation_table 。
		/// 文字列表が空の場合は、条件挙動ハンドラを追加しない。
		template_relation_table const& in_handler_table)
	{
		in_status_builder(
			this->Reservoir,
			this->HashFunction,
			InChunkKey,
			in_status_table);
		in_expression_builder(
			this->Evaluator,
			this->HashFunction,
			InChunkKey,
			this->Reservoir,
			in_expression_table);
		This::handler_chunk::extend(
			this->handler_chunks_,
			InChunkKey,
			in_handler_builder(
				this->dispatcher_,
				this->HashFunction,
				this->accumulator_,
				in_handler_table));
	}

	/// @brief 状態値と条件式と条件挙動ハンドラを、チャンクへ追加する。
	public: template<typename template_string>
	void extend_chunk(
		/// [out] 文字列表の構築の作業領域として使う文字列。
		/// std::basic_string 互換のインタフェイスを持つこと。
		template_string& out_workspace,
		/// [in] 文字列表の構築に使う
		/// Psyque::string::flyweight::factory の強参照。空ではないこと。
		typename Psyque::string::flyweight<
			TemplateHasher, TemplateSetAllocator>
		::factory::shared_ptr
			const& in_string_factory,
		/// [in] 追加するチャンクの識別値。
		typename This::FChunkKey const& InChunkKey,
		/// [in] 状態値CSV文字列。空文字列の場合は、状態値を追加しない。
		Psyque::string::view<
			typename template_string::value_type,
			typename template_string::traits_type>
				const& in_status_csv,
		/// [in] 状態値CSVの属性の行番号。
		std::size_t const in_status_attribute,
		/// [in] 条件式CSV文字列。空文字列の場合は、条件式を追加しない。
		Psyque::string::view<
			typename template_string::value_type,
			typename template_string::traits_type>
				const& in_expression_csv,
		/// [in] 条件式CSVの属性の行番号。
		std::size_t const in_expression_attribute,
		/// [in] 条件挙動CSV文字列。空文字列の場合は、条件挙動ハンドラを追加しない。
		Psyque::string::view<
			typename template_string::value_type,
			typename template_string::traits_type>
				const& in_handler_csv,
		/// [in] 条件挙動CSVの属性の行番号。
		std::size_t const in_handler_attribute)
	{
		typedef
			Psyque::string::csv_table<
				std::size_t, 
				TemplateHasher,
				TemplateSetAllocator>
			csv_table;
		this->extend_chunk(
			InChunkKey,
			Psyque::RuleEngine::status_builder(),
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
	void RemoveChunk(
		/// [in] 削除するチャンクの識別値。
		typename This::FChunkKey const& InChunkKey)
	{
		this->Reservoir.RemoveChunk(InChunkKey);
		this->Evaluator.RemoveChunk(InChunkKey);
//		This::handler_chunk::erase(this->handler_chunks_, InChunkKey);
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 条件挙動ハンドラ
	/// @{
	public:
/*
	/// @brief 条件挙動ハンドラを登録し、条件挙動関数を強参照する。
	/// @sa
	/// This::progress で、 in_expression_key に対応する条件式の評価が変化し
	/// in_condition と合致すると、 in_function の指す条件挙動関数が呼び出される。
	/// @sa
	/// in_function の指す条件挙動関数が解体されると、それを弱参照している
	/// dispatcher::handler は自動的に削除される。明示的に削除するには、
	/// This::dispatcher_ に対して dispatcher::unregister_handler を使う。
	/// @retval true
	/// 成功。 in_function の指す条件挙動関数を弱参照する
	/// dispatcher::handler を構築して This::dispatcher_
	/// に登録し、登録した条件挙動関数の強参照をチャンクに追加した。
	/// @retval false
	/// 失敗。 dispatcher::handler は構築されず、
	/// 条件挙動関数の強参照はチャンクに追加されなかった。
	/// - in_condition が dispatcher::handler::INVALID_CONDITION だと、失敗する。
	/// - in_function が空か、空の関数を指していると、失敗する。
	/// - in_expression_key と対応する dispatcher::handler に、
	///   in_function の指す条件挙動関数が既に登録されていると、失敗する。
	public: bool register_handler(
		/// [in] 条件挙動関数を追加するチャンクの識別値。
		typename This::FChunkKey const& InChunkKey,
		/// [in] in_function の指す条件挙動関数に対応する
		/// FEvaluator::expression の識別値。
		typename This::FEvaluator::expression_key const& in_expression_key,
		/// [in] in_function の指す条件挙動関数を呼び出す挙動条件。
		/// dispatcher::handler::make_condition から作る。
		typename This::dispatcher::handler::condition const in_condition,
		/// [in] 登録する dispatcher::handler::function を指すスマートポインタ。
		/// in_expression_key に対応する条件式の評価が変化して
		/// in_condition に合致すると、呼び出される。
		typename This::dispatcher::handler::function_shared_ptr in_function,
		/// [in] in_function の指す条件挙動関数の呼び出し優先順位。
		/// 昇順に呼び出される。
		typename This::dispatcher::handler::priority const in_priority =
			PSYQUE_IF_THEN_ENGINE_DISPATCHER_FUNCTION_PRIORITY_DEFAULT)
	{
		// 条件挙動関数を条件挙動器へ登録する。
		auto const local_register_handler(
			this->dispatcher_.register_handler(
				in_expression_key, in_condition, in_function, in_priority));
		if (local_register_handler)
		{
			// 条件挙動関数を条件挙動チャンクへ追加する。
			This::handler_chunk::extend(
				this->handler_chunks_, InChunkKey, std::move(in_function));
		}
		return local_register_handler;
	}
*/
	/// @}
	//-------------------------------------------------------------------------
	/// @name 状態値
	/// @{
	public:
	/// @brief 状態貯蔵器を取得する。
	/// @return *this が持つ状態貯蔵器。
	typename This::FReservoir const& GetReservoir() const PSYQUE_NOEXCEPT
	{
		return this->Reservoir;
	}

	/// @brief 状態値を登録する。
	/// @sa
	/// - 登録した状態値を取得するには、 This::GetReservoir から
	///   FReservoir::find_status を呼び出す。
	/// - 状態値の変更は This::accumulator_ から
	///   accumulator::accumulate を呼び出して行う。
	/// - This::RemoveChunk で、登録した状態値をチャンク毎に削除できる。
	/// @retval true  成功。状態値を登録した。
	/// @retval false 失敗。状態値は登録されなかった。
	/// - InStatusKey に対応する状態値がすでに登録されていると失敗する。
	template<typename TemplateValue>
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
		return this->Reservoir.RegisterStatus(InChunkKey, InStatusKey, InValue);
	}

	/// @brief 整数型の状態値を登録する。
	/// @sa
	/// - 登録した状態値を取得するには、 This::GetReservoir から
	///   FReservoir::find_status を呼び出す。
	/// - 登録した状態値を書き換えるには、 This::accumulator_ から
	///   accumulator::accumulate を呼び出す。
	/// - This::RemoveChunk で、登録した状態値をチャンク毎に削除できる。
	/// @retval true  成功。状態値を登録した。
	/// @retval false 失敗。状態値は登録されなかった。
	/// - InStatusKey に対応する状態値がすでに登録されていると失敗する。
	/// - InValue のビット幅が InBitWidth を超えていると失敗する。
	/// - FReservoir::status_chunk::BLOCK_BIT_WIDTH より
	///   InBitWidth が大きいと失敗する。
	/// - InBitWidth が2未満だと失敗する。
	///   1ビットの値は論理型として登録すること。
	template<typename TemplateValue>
	bool RegisterStatus(
		/// [in] 登録する状態値を格納する状態値ビット列チャンクの識別値。
		typename This::FChunkKey const InChunkKey,
		/// [in] 登録する状態値の識別値。
		typename This::FReservoir::FStatusKey const& InStatusKey,
		/// [in] 登録する状態値の初期値。 C++ 組み込み整数型であること。
		TemplateValue const InValue,
		/// [in] 登録する状態値のビット幅。
		SIZE_T const InBitWidth)
	{
		return this->Reservoir.RegisterStatus(
			InChunkKey, InStatusKey, InValue, InBitWidth);
	}

	/// @brief 状態値を更新し、条件式を評価して、条件挙動関数を呼び出す。
	/// @details 基本的には、時間フレーム毎に呼び出すこと。
	void Tick()
	{
		//this->accumulator_._flush(this->Reservoir);
		//this->dispatcher_._dispatch(this->Reservoir, this->Evaluator);
	}
	/// @}
	//-------------------------------------------------------------------------
	private:
	/// @brief 駆動器で用いる状態貯蔵器。
	typename This::FReservoir Reservoir;
/*
	/// @brief 駆動器で用いる条件挙動チャンクのコンテナ。
	private: typename This::handler_chunk::container handler_chunks_;
*/
	//-------------------------------------------------------------------------
	public:
/*
	/// @brief 駆動器で用いる状態変更器。
	public: typename This::accumulator accumulator_;
*/
	/// @brief 駆動器で用いる条件評価器。
	typename This::FEvaluator Evaluator;
/*
	/// @brief 駆動器で用いる条件挙動器。
	public: typename This::dispatcher dispatcher_;
*/
	/// @brief 駆動器で用いる文字列ハッシュ関数オブジェクト。
	public: TemplateHasher HashFunction;
}; // class Psyque::RuleEngine::TDriver

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace PsyqueTest
{
	inline void RuleEngine()
	{
		using FDriver = Psyque::RuleEngine::TDriver<>;
		FDriver LocalDriver(256, 256, 256);
		FName const LocalChunkName(TEXT("Psyque"));

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
				FDriver::FReservoir::FStatusValue::EComparison::EQUAL,
				LocalBool));

		auto const LocalIntegerStatus(
			LocalDriver.GetReservoir().FindStatus(
				LocalDriver.HashFunction(LocalIntegerName)));
		check(
			0 < LocalIntegerStatus.Compare(
				FDriver::FReservoir::FStatusValue::EComparison::EQUAL,
				LocalInteger));

		auto const LocalFloatStatus(
			LocalDriver.GetReservoir().FindStatus(
				LocalDriver.HashFunction(LocalFloatName)));
		check(
			0 < LocalFloatStatus.Compare(
				FDriver::FReservoir::FStatusValue::EComparison::EQUAL,
				LocalFloat));

		LocalDriver.Tick();
		LocalDriver.RemoveChunk(LocalDriver.HashFunction(LocalChunkName));
	}
}

// vim: set noexpandtab:
