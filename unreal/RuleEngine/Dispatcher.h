// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RuleEngine::_private::TDispatcher
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#include <unordered_map>
#include "../PrimitiveBits.h"
#include "./StatusMonitor.h"
#include "./ExpressionMonitor.h"
#include "./Handler.h"

/// @brief 挙動関数の呼び出し優先順位のデフォルト値。
#ifndef PSYQUE_IF_THEN_ENGINE_DISPATCHER_FUNCTION_PRIORITY_DEFAULT
#define PSYQUE_IF_THEN_ENGINE_DISPATCHER_FUNCTION_PRIORITY_DEFAULT 0
#endif // !defined(PSYQUE_IF_THEN_ENGINE_DISPATCHER_FUNCTION_PRIORITY_DEFAULT)

/// @cond
namespace Psyque
{
	namespace RuleEngine
	{
		namespace _private
		{
			template<typename, typename> class TDispatcher;
		} // namespace _private
	} // namespace RuleEngine
} // namespace Psyque
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 条件挙動器。条件式の評価が条件と合致すると、関数を呼び出す。
/// @par 使い方の概略
///   - TDispatcher::RegisterHandler
///     で、挙動条件に合致した際に呼び出す条件挙動関数を登録する。
///   - TDispatcher::_dispatch
///     で状態値の変化を検知して、変化した状態値を参照する条件式を評価し、
///     条件式の評価の変化が挙動条件と合致する条件挙動関数を呼び出す。
/// @tparam TemplateEvaluator @copydoc TDispatcher::FEvaluator
/// @tparam TemplatePriority  @copydoc TDispatcher::FHandler::FPriority
template<typename TemplateEvaluator, typename TemplatePriority>
class Psyque::RuleEngine::_private::TDispatcher
{
	private: using This = TDispatcher; ///< @copydoc RuleEngine::TDriver::This

	//-------------------------------------------------------------------------
	/// @brief 条件挙動器で使う TEvaluator の型。
	public: using FEvaluator = TemplateEvaluator;
	/// @brief コンテナで使うメモリ割当子の型。
	public: using FAllocator = typename This::FEvaluator::FAllocator;
	/// @brief 条件挙動器で使う条件挙動ハンドラ。
	public: using FHandler = Psyque::RuleEngine::_private::THandler<
		typename This::FEvaluator::FExpressionKey, TemplatePriority>;

	//-------------------------------------------------------------------------
	/// @copydoc This::StatusMonitors
	private: using FStatusMonitorMap = std::unordered_map<
		typename This::FEvaluator::FReservoir::FStatusKey,
		Psyque::RuleEngine::_private::TStatusMonitor<
			std::vector<
				typename This::FEvaluator::FExpressionKey,
				typename This::FAllocator>>,
		Psyque::Hash::TPrimitiveBits<
			typename This::FEvaluator::FReservoir::FStatusKey>,
		std::equal_to<typename This::FEvaluator::FReservoir::FStatusKey>,
		typename This::FAllocator>;
	/// @copydoc This::ExpressionMonitors
	private: using FExpressionMonitorMap = std::unordered_map<
		typename This::FEvaluator::FExpressionKey,
		Psyque::RuleEngine::_private::TExpressionMonitor<
			std::vector<typename This::FHandler, typename This::FAllocator>>,
		Psyque::Hash::TPrimitiveBits<
			typename This::FEvaluator::FExpressionKey>,
		std::equal_to<typename This::FEvaluator::FExpressionKey>,
		typename This::FAllocator>;
	/// @copydoc This::CachedHandlers
	private: using FHandlerCacheArray = std::vector<
		typename This::FHandler::FCache, typename This::FAllocator>;

	//-------------------------------------------------------------------------
	/// @name 構築と代入
	/// @{

	/// @brief 空の条件挙動器を構築する。
	public: TDispatcher(
		/// [in] 監視する状態値のバケット数。
		std::size_t const InStatusCapacity,
		/// [in] 監視する条件式のバケット数。
		std::size_t const InExpressionCapacity,
		/// [in] 条件挙動キャッシュの予約数。
		std::size_t const InCacheCapacity,
		/// [in] メモリ割当子の初期値。
		typename This::FAllocator const& InAllocator):
	StatusMonitors(
		InStatusCapacity,
		typename This::FStatusMonitorMap::hasher(),
		typename This::FStatusMonitorMap::key_equal(),
		InAllocator),
	ExpressionMonitors(
		InExpressionCapacity,
		typename This::FExpressionMonitorMap::hasher(),
		typename This::FExpressionMonitorMap::key_equal(),
		InAllocator),
	CachedHandlers(InAllocator),
	DispatchLock(false)
	{
		this->CachedHandlers.reserve(InCacheCapacity);
	}

	/// @brief コピー構築子。
	public: TDispatcher(
		/// [in] コピー元となるインスタンス。
		This const& InSource):
	StatusMonitors(InSource.StatusMonitors),
	ExpressionMonitors(InSource.ExpressionMonitors),
	CachedHandlers(InSource.CachedHandlers.get_allocator()),
	DispatchLock(false)
	{
		this->CachedHandlers.reserve(InSource.CachedHandlers.capacity());
	}

	/// @brief ムーブ構築子。
	public: TDispatcher(
		/// [in,out] ムーブ元となるインスタンス。
		This&& OutSource):
	StatusMonitors((
		/// @warning This::_dispatch 実行中はムーブできない。
		PSYQUE_ASSERT(!OutSource.DispatchLock),
		MoveTemp(OutSource.StatusMonitors))),
	ExpressionMonitors(MoveTemp(OutSource.ExpressionMonitors)),
	CachedHandlers(MoveTemp(OutSource.CachedHandlers)),
	DispatchLock(false)
	{}

	/// @brief コピー代入演算子。
	/// @return *this
	public: This& operator=(
		/// [in] コピー元となるインスタンス。
		This const& InSource)
	{
		/// @warning This::_dispatch 実行中はムーブできない。
		check(!this->DispatchLock && !InSource.DispatchLock);
		this->StatusMonitors = InSource.StatusMonitors;
		this->ExpressionMonitors = InSource.ExpressionMonitors;
		this->CachedHandlers.reserve(InSource.CachedHandlers.capacity());
		return *this;
	}

	/// @brief ムーブ代入演算子。
	/// @return *this
	public: This& operator=(
		/// [in,out] ムーブ元となるインスタンス。
		This&& OutSource)
	{
		/// @warning This::_dispatch 実行中はムーブできない。
		check(!this->DispatchLock && !OutSource.DispatchLock);
		this->StatusMonitors = MoveTemp(OutSource.StatusMonitors);
		this->ExpressionMonitors = MoveTemp(OutSource.ExpressionMonitors);
		this->CachedHandlers = MoveTemp(OutSource.CachedHandlers);
		return *this;
	}

	/// @brief 条件挙動器を解体する。
	public: ~TDispatcher()
	{
		/// @warning This::_dispatch 実行中は解体できない。
		check(!this->DispatchLock);
	}

	/// @brief 条件挙動器で使われているメモリ割当子を取得する。
	/// @return *this で使われているメモリ割当子のコピー。
	public: typename This::FAllocator get_allocator() const PSYQUE_NOEXCEPT
	{
		return this->ExpressionMonitors.get_allocator();
	}

	/// @brief 条件挙動器を再構築し、メモリ領域を必要最小限にする。
	public: void Rebuild(
		/// [in] 監視する状態値のバケット数。
		std::size_t const InStatusCapacity,
		/// [in] 監視する条件式のバケット数。
		std::size_t const InExpressionCapacity,
		/// [in] 条件挙動キャッシュの予約数。
		std::size_t const InCacheCapacity)
	{
		using FExpressionMonitor =
			typename This::FExpressionMonitorMap::mapped_type;
		This::RebuildMonitors(
			this->ExpressionMonitors,
			InExpressionCapacity,
			[](FExpressionMonitor& OutExpressionMonitor)->bool
			{
				return OutExpressionMonitor.ShrinkHandlers();
			});
		using FStatusMonitor = typename This::FStatusMonitorMap::mapped_type;
		This::RebuildMonitors(
			this->StatusMonitors,
			InStatusCapacity,
			[this](FStatusMonitor& OutStatusMonitor)->bool
			{
				return OutStatusMonitor.ShrinkExpressionKeys(
					this->ExpressionMonitors);
			});
		check(this->CachedHandlers.empty());
		this->CachedHandlers = decltype(this->CachedHandlers)(
			this->CachedHandlers.get_allocator());
		this->CachedHandlers.reserve(InCacheCapacity);
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 条件挙動ハンドラ
	/// @{

	/// @brief 条件挙動ハンドラが登録されているか判定する。
	/// @retval true
	///   InExpressionKey に対応し *InFunction を弱参照している
	///   This::FHandler が *this に登録されている。
	/// @retval false 該当する This::FHandler は *this に登録されてない。
	public: bool IsRegistered(
		/// [in] 判定する This::FHandler に対応する
		/// TEvaluator::FExpression の識別値。
		typename This::FEvaluator::FExpressionKey const InExpressionKey,
		/// [in] 判定する This::FHandler が弱参照している条件挙動関数。
		typename This::FHandler::FFunction const* const InFunction)
	const
	{
		return this->FindHandlerPtr(InExpressionKey, InFunction) != nullptr;
	}

	/// @brief 条件挙動ハンドラを登録する。
	/// @sa
	///   This::_dispatch で、 InExpressionKey に対応する条件式の評価が変化し
	///   InCondition と合致すると、 InFunction の指す条件挙動関数が呼び出される。
	/// @sa
	///   InFunction の指す条件挙動関数が解体されると、それを弱参照している
	///   This::FHandler は自動的に削除される。
	///   明示的に削除するには This::UnregisterHandler を使う。
	/// @retval true
	///   成功。 InFunction の指す条件挙動関数を弱参照する
	///   This::FHandler を構築し、 *this に登録した。
	/// @retval false
	///   失敗。 This::FHandler は構築されなかった。
	///   - InCondition が THandler::EUnitCondition::Invalid だと、失敗する。
	///   - InFunction が空か、空の関数を指していると、失敗する。
	///   - InExpressionKey と対応する This::FHandler に、
	///     InFunction の指す条件挙動関数が既に登録されていると、失敗する。
	public: bool RegisterHandler(
		/// [in] InFunction の指す条件挙動関数に対応する
		/// TEvaluator::FExpression の識別値。
		typename This::FEvaluator::FExpressionKey const InExpressionKey,
		/// [in] InFunction の指す条件挙動関数を呼び出す挙動条件。
		/// THandler::MakeCondition から作る。
		typename This::FHandler::FCondition const InCondition,
		/// [in] 登録する THandler::FFunction を指すスマートポインタ。
		/// InExpressionKey に対応する条件式の評価が変化して
		/// InCondition に合致すると、呼び出される。
		typename This::FHandler::FFunctionSharedPtr const& InFunction,
		/// [in] InFunction の指す条件挙動関数の呼び出し優先順位。
		/// 昇順に呼び出される。
		typename This::FHandler::FPriority const InPriority =
			PSYQUE_IF_THEN_ENGINE_DISPATCHER_FUNCTION_PRIORITY_DEFAULT)
	{
		return This::FExpressionMonitorMap::mapped_type::RegisterHandler(
			this->ExpressionMonitors,
			InExpressionKey,
			InCondition,
			InFunction,
			InPriority);
	}

	/// @brief 条件挙動ハンドラを削除する。
	/// @details
	///   This::RegisterHandler で登録した This::FHandler を削除する。
	/// @retval true
	///   InExpressionKey に対応し InFunction を弱参照している
	///   This::FHandler を削除した。
	/// @retval false 該当する This::FHandler が見つからなかった。
	public: bool UnregisterHandler(
		/// [in] 削除する This::FHandler に対応する
		/// TEvaluator::FExpression の識別値。
		typename This::FEvaluator::FExpressionKey const InExpressionKey,
		/// [in] 削除する This::FHandler が弱参照している条件挙動関数。
		typename This::FHandler::FFunction const& InFunction)
	{
		auto const LocalFind(this->ExpressionMonitors.find(InExpressionKey));
		return LocalFind != this->ExpressionMonitors.end() 
			&& LocalFind->second.UnregisterHandler(InFunction);
	}

	/// @brief ある条件式に対応する条件挙動ハンドラをすべて削除する。
	/// @details
	///   This::RegisterHandler で登録した This::FHandler のうち、
	///   InExpressionKey に対応するものをすべて削除する。
	/// @retval true
	///   InExpressionKey に対応する This::FHandler をすべて削除した。
	/// @retval false 該当する This::FHandler がなかった。
	public: bool UnregisterHandlers(
		/// [in] 削除する This::FHandler に対応する
		/// TEvaluator::FExpression の識別値。
		typename This::FEvaluator::FExpressionKey const InExpressionKey)
	{
		return 0 < this->ExpressionMonitors.erase(InExpressionKey);
	}

	/// @brief ある条件挙動関数を弱参照している条件挙動ハンドラをすべて削除する。
	/// @details
	///   This::RegisterHandler で登録した This::FHandler のうち、
	///   InFunction を弱参照しているものをすべて削除する。
	/// @retval true
	///   InFunction を弱参照している This::FHandler をすべて削除した。
	/// @retval false 該当する This::FHandler がなかった。
	public: bool UnregisterHandlers(
		/// [in] 削除する This::FHandler が弱参照している条件挙動関数。
		typename This::FHandler::FFunction const& InFunction)
	{
		std::size_t local_count(0);
		for (auto& LocalExpressionMonitor: this->ExpressionMonitors)
		{
			if (LocalExpressionMonitor.second.UnregisterHandler(InFunction))
			{
				++local_count;
			}
		}
		return 0 < local_count;
	}

	/// @brief 登録されている条件挙動ハンドラを取得する。
	/// @return
	///   This::RegisterHandler で *this に登録された、 InExpressionKey
	///   に対応し *InFunction を弱参照している This::FHandler 。
	///   該当するものがない場合は、 This::FHandler::GetCondition が
	///   This::FHandler::EUnitCondition::Invalid となる値を返す。
	public: typename This::FHandler FindHandler(
		/// [in] 取得する This::FHandler に対応する
		/// TEvaluator::FExpression の識別値。
		typename This::FEvaluator::FExpressionKey const InExpressionKey,
		/// [in] 取得する This::FHandler が弱参照している条件挙動関数。
		typename This::FHandler::FFunction const* const InFunction)
	const
	{
		auto const local_handler(
			this->FindHandlerPtr(InExpressionKey, InFunction));
		return local_handler != nullptr?
			*local_handler:
			typename This::FHandler(
				This::FHandler::EUnitCondition::Invalid,
				typename This::FHandler::FFunctionWeakPtr(),
				typename This::FHandler::FPriority());
	}

	/// @brief Psyque::RuleEngine 管理者以外は、この関数は使用禁止。
	/// @details
	///   前回の This::_dispatch と今回の This::_dispatch
	///   で条件式の評価が変化した場合に、 This::RegisterHandler
	///   で指定した挙動条件と合致していれば、
	///   This::FHandler に通知して条件挙動関数を呼び出す。
	/// @warning
	///   前回から今回の間（基本的には1フレームの間）で条件式の評価が変化しても、
	///   前回の時点と今回の時点の評価が同じ場合は、条件挙動関数が呼び出されない。
	///   たとえば、前回から今回の間で条件式の評価が
	///   「true（前回）／false（前回と今回の間）／true（今回）」
	///   と変化した場合、条件挙動関数は呼び出されない。
	public: void _dispatch(
		/// [in,out] 条件式の評価で参照する状態貯蔵器。
		typename This::FEvaluator::FReservoir& OutReservoir,
		/// [in] 条件式の評価に使う条件評価器。
		typename This::FEvaluator const& InEvaluator)
	{
		// _dispatch を多重に実行しないようにロックする。
		if (this->DispatchLock)
		{
			check(false);
			return;
		}
		this->DispatchLock = true;

		// 条件式を状態監視器へ登録する。
		This::FExpressionMonitorMap::mapped_type::RegisterExpressions(
			this->StatusMonitors, this->ExpressionMonitors, InEvaluator);

		// 状態値の変化を検知し、条件式監視器へ知らせる。
		This::FStatusMonitorMap::mapped_type::NotifyStatusTransitions(
			this->StatusMonitors, this->ExpressionMonitors, OutReservoir);

		// 条件挙動ハンドラキャッシュの作業領域を再利用する。
		check(this->CachedHandlers.empty());
		auto LocalCachedHandlers(this->CachedHandlers);
		LocalCachedHandlers.swap(this->CachedHandlers);

		// 変化した状態値を参照する条件式を評価し、
		// 挙動条件に合致した条件挙動ハンドラをキャッシュに貯めて、
		// 優先順位で並び替える。
		This::FExpressionMonitorMap::mapped_type::CacheHandlers(
			LocalCachedHandlers,
			this->ExpressionMonitors,
			OutReservoir,
			InEvaluator);
		std::sort(
			LocalCachedHandlers.begin(),
			LocalCachedHandlers.end(),
			[](
				typename This::FHandler const& InLeft,
				typename This::FHandler const& InRight)
			->bool
			{
				return InLeft.GetPriority() < InRight.GetPriority();
			});

		// 条件式の評価が済んだので、状態変化フラグを初期化する。
		OutReservoir._reset_transitions();

		// キャッシュに貯まった条件挙動関数を呼び出す。
		for (auto const& LocalCachedHandler: LocalCachedHandlers)
		{
			LocalCachedHandler.CallFunction();
		}

		// 条件挙動ハンドラキャッシュの作業領域を回収する。
		if (0 < this->CachedHandlers.capacity())
		{
			check(this->CachedHandlers.empty());
		}
		else
		{
			LocalCachedHandlers.clear();
			this->CachedHandlers = MoveTemp(LocalCachedHandlers);
		}
		check(this->DispatchLock);
		this->DispatchLock = false;
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @brief 登録されている条件挙動ハンドラを取得する。
	/// @return
	///   InExpressionKey に対応し *InFunction を弱参照している
	///   This::FHandler を指すポインタ。
	///   該当するものがない場合は nullptr を返す。
	private: typename This::FHandler const* FindHandlerPtr(
		/// [in] 取得する This::FHandler に対応する
		/// TEvaluator::FExpression の識別値。
		typename This::FEvaluator::FExpressionKey const InExpressionKey,
		/// [in] 取得する This::FHandler が弱参照している条件挙動関数。
		typename This::FHandler::FFunction const* const InFunction)
	const
	{
		if (InFunction != nullptr)
		{
			auto const LocalFind(
				this->ExpressionMonitors.find(InExpressionKey));
			if (LocalFind != this->ExpressionMonitors.end())
			{
				return LocalFind->second._find_handler_ptr(*InFunction);
			}
		}
		return nullptr;
	}

	/// @brief 監視器を再構築する。
	private: template<
		 typename TemplateMonitorMap, typename TemplateRebuildFunction>
	static void RebuildMonitors(
		/// [in,out] 再構築する監視器の辞書。
		TemplateMonitorMap& OutMonitors,
		/// [in] 辞書のバケット数。
		std::size_t const InBucketCount,
		/// [in] 監視器を再構築する関数。
		TemplateRebuildFunction const& InRebuildFunction)
	{
		for (auto i(OutMonitors.begin()); i != OutMonitors.end();)
		{
			if (InRebuildFunction(i->second))
			{
				i = OutMonitors.erase(i);
			}
			else
			{
				++i;
			}
		}
		OutMonitors.rehash(InBucketCount);
	}

	//-------------------------------------------------------------------------
	/// @brief TStatusMonitor の辞書。
	private: typename This::FStatusMonitorMap StatusMonitors;
	/// @brief TExpressionMonitor の辞書。
	private: typename This::FExpressionMonitorMap ExpressionMonitors;
	/// @brief THandler::FCache の配列。
	private: typename This::FHandlerCacheArray CachedHandlers;
	/// @brief 多重に This::_dispatch しないためのロック。
	private: bool DispatchLock;

}; // class Psyque::RuleEngine::_private::TDispatcher

// vim: set noexpandtab:
