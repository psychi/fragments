// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RulesEngine::_private::TDispatcher
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#include <unordered_map>
#include "../PrimitiveBits.h"
#include "./StatusMonitor.h"
#include "./ExpressionMonitor.h"
#include "./Hook.h"

/// @cond
namespace Psyque
{
	namespace RulesEngine
	{
		namespace _private
		{
			template<typename, typename> class TDispatcher;
		} // namespace _private
	} // namespace RulesEngine
} // namespace Psyque
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 条件挙動器。条件式の評価が条件と合致すると、関数を呼び出す。
/// @par 使い方の概略
///   - TDispatcher::RegisterHook
///     で、挙動条件に合致した際に呼び出す条件挙動関数を登録する。
///   - TDispatcher::_dispatch
///     で状態値の変化を検知して、変化した状態値を参照する条件式を評価し、
///     条件式の評価の変化が挙動条件と合致する条件挙動関数を呼び出す。
/// @tparam TemplateEvaluator @copydoc TDispatcher::FEvaluator
/// @tparam TemplatePriority  @copydoc TDispatcher::FHook::FPriority
template<typename TemplateEvaluator, typename TemplatePriority>
class Psyque::RulesEngine::_private::TDispatcher
{
	private: using ThisClass = TDispatcher; ///< @copydoc RulesEngine::TDriver::ThisClass

	//-------------------------------------------------------------------------
	/// @brief 条件挙動器で使う TEvaluator の型。
	public: using FEvaluator = TemplateEvaluator;
	/// @brief コンテナで使うメモリ割当子の型。
	public: using FAllocator = typename ThisClass::FEvaluator::FAllocator;
	/// @brief 条件挙動器で使う条件挙動フック。
	public: using FHook = Psyque::RulesEngine::_private::THook<
		typename ThisClass::FEvaluator::FExpressionKey, TemplatePriority>;

	//-------------------------------------------------------------------------
	/// @copydoc ThisClass::StatusMonitors
	private: using FStatusMonitorMap = std::unordered_map<
		typename ThisClass::FEvaluator::FReservoir::FStatusKey,
		Psyque::RulesEngine::_private::TStatusMonitor<
			std::vector<
				typename ThisClass::FEvaluator::FExpressionKey,
				typename ThisClass::FAllocator>>,
		Psyque::Hash::TPrimitiveBits<
			typename ThisClass::FEvaluator::FReservoir::FStatusKey>,
		std::equal_to<typename ThisClass::FEvaluator::FReservoir::FStatusKey>,
		typename ThisClass::FAllocator>;
	/// @copydoc ThisClass::ExpressionMonitors
	private: using FExpressionMonitorMap = std::unordered_map<
		typename ThisClass::FEvaluator::FExpressionKey,
		Psyque::RulesEngine::_private::TExpressionMonitor<
			std::vector<typename ThisClass::FHook, typename ThisClass::FAllocator>>,
		Psyque::Hash::TPrimitiveBits<
			typename ThisClass::FEvaluator::FExpressionKey>,
		std::equal_to<typename ThisClass::FEvaluator::FExpressionKey>,
		typename ThisClass::FAllocator>;
	/// @copydoc ThisClass::CachedHooks

	//-------------------------------------------------------------------------
	/// @brief 条件挙動フックのキャッシュ。
	private: class FHookCache
	{
		/// @copydoc TDispatcher::ThisClass
		private: using ThisClass = FHookCache;

		public: using FArray
			= std::vector<ThisClass, typename TDispatcher::FAllocator>;

		/// @brief 条件挙動フックのキャッシュを構築する。
		public: FHookCache(
			/// [in] キャッシュする条件挙動フック。
			typename TDispatcher::FHook const& InHook,
			/// [in] ThisClass::ExpressionKey の初期値。
			typename TDispatcher::FHook::FExpressionKey const InExpressionKey):
		Hook(&InHook),
		ExpressionKey(InExpressionKey)
		{}

		public: typename TDispatcher::FHook::FPriority GetPriority() const
		{
			check(this->Hook != nullptr);
			return this->Hook->Priority;
		}

		/// @brief デリゲートを実行する。
		public: bool ExecuteDelegate() const
		{
			check(this->Hook != nullptr);
			return this->Hook->GetDelegate().ExecuteIfBound(
				this->ExpressionKey,
				this->Hook->GetBeforeCondition(),
				this->Hook->GetLatestCondition());
		}

		/// @brief 実行する条件挙動フック。
		private: typename TDispatcher::FHook const* Hook;
		/// @brief 条件式の識別値。
		private: typename TDispatcher::FHook::FExpressionKey ExpressionKey;

	}; // class FHookCache

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
		typename ThisClass::FAllocator const& InAllocator):
	StatusMonitors(
		InStatusCapacity,
		typename ThisClass::FStatusMonitorMap::hasher(),
		typename ThisClass::FStatusMonitorMap::key_equal(),
		InAllocator),
	ExpressionMonitors(
		InExpressionCapacity,
		typename ThisClass::FExpressionMonitorMap::hasher(),
		typename ThisClass::FExpressionMonitorMap::key_equal(),
		InAllocator),
	CachedHooks(InAllocator),
	DispatchLock(false)
	{
		this->CachedHooks.reserve(InCacheCapacity);
	}

	/// @brief コピー構築子。
	public: TDispatcher(
		/// [in] コピー元となるインスタンス。
		ThisClass const& InSource):
	StatusMonitors(InSource.StatusMonitors),
	ExpressionMonitors(InSource.ExpressionMonitors),
	CachedHooks(InSource.CachedHooks._get_allocator()),
	DispatchLock(false)
	{
		this->CachedHooks.reserve(InSource.CachedHooks.capacity());
	}

	/// @brief ムーブ構築子。
	public: TDispatcher(
		/// [in,out] ムーブ元となるインスタンス。
		ThisClass&& OutSource):
	StatusMonitors((
		/// @warning ThisClass::_dispatch 実行中はムーブできない。
		PSYQUE_ASSERT(!OutSource.DispatchLock),
		MoveTemp(OutSource.StatusMonitors))),
	ExpressionMonitors(MoveTemp(OutSource.ExpressionMonitors)),
	CachedHooks(MoveTemp(OutSource.CachedHooks)),
	DispatchLock(false)
	{}

	/// @brief コピー代入演算子。
	/// @return *this
	public: ThisClass& operator=(
		/// [in] コピー元となるインスタンス。
		ThisClass const& InSource)
	{
		/// @warning ThisClass::_dispatch 実行中はムーブできない。
		check(!this->DispatchLock && !InSource.DispatchLock);
		this->StatusMonitors = InSource.StatusMonitors;
		this->ExpressionMonitors = InSource.ExpressionMonitors;
		this->CachedHooks.reserve(InSource.CachedHooks.capacity());
		return *this;
	}

	/// @brief ムーブ代入演算子。
	/// @return *this
	public: ThisClass& operator=(
		/// [in,out] ムーブ元となるインスタンス。
		ThisClass&& OutSource)
	{
		/// @warning ThisClass::_dispatch 実行中はムーブできない。
		check(!this->DispatchLock && !OutSource.DispatchLock);
		this->StatusMonitors = MoveTemp(OutSource.StatusMonitors);
		this->ExpressionMonitors = MoveTemp(OutSource.ExpressionMonitors);
		this->CachedHooks = MoveTemp(OutSource.CachedHooks);
		return *this;
	}

	/// @brief 条件挙動器を解体する。
	public: ~TDispatcher()
	{
		/// @warning ThisClass::_dispatch 実行中は解体できない。
		check(!this->DispatchLock);
	}

	/// @brief 条件挙動器で使われているメモリ割当子を取得する。
	/// @return *this で使われているメモリ割当子のコピー。
	public: typename ThisClass::FAllocator _get_allocator() const PSYQUE_NOEXCEPT
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
			typename ThisClass::FExpressionMonitorMap::mapped_type;
		ThisClass::RebuildMonitors(
			this->ExpressionMonitors,
			InExpressionCapacity,
			[](FExpressionMonitor& OutExpressionMonitor)->bool
			{
				return OutExpressionMonitor.ShrinkHooks();
			});
		using FStatusMonitor = typename ThisClass::FStatusMonitorMap::mapped_type;
		ThisClass::RebuildMonitors(
			this->StatusMonitors,
			InStatusCapacity,
			[this](FStatusMonitor& OutStatusMonitor)->bool
			{
				return OutStatusMonitor.ShrinkExpressionKeys(
					this->ExpressionMonitors);
			});
		check(this->CachedHooks.empty());
		this->CachedHooks = decltype(this->CachedHooks)(
			this->CachedHooks.get_allocator());
		this->CachedHooks.reserve(InCacheCapacity);
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name 条件挙動
	/// @{

	/// @brief 条件挙動を登録する。
	/// @details
	///   InExpressionKey が指す条件式の評価が
	///   ThisClass::_dispatch の呼び出し前後で変化し、遷移状態が
	///   InTransition と合致すると、 InDelegate が実行される。
	/// @sa
	///   InDelegate が無効になると、対応する
	///   ThisClass::FHook は自動的に取り除かれる。
	///   明示的に取り除くには ThisClass::UnregisterHooks を使う。
	/// @return 登録した ThisClass::FHook が持つデリゲートを指すハンドル。
	///   ただし登録に失敗した場合は、空のハンドルを戻す。
	///   - InTransition が無効だと、失敗する。
	///   - InDelegate が無効だと、失敗する。
	public: ::FDelegateHandle RegisterHook(
		/// [in] InDelegate を実行するか判定する、
		/// TEvaluator::FExpression の識別値。
		typename ThisClass::FEvaluator::FExpressionKey const InExpressionKey,
		/// [in] InDelegate を実行する条件となる、条件式の評価の遷移状態。
		typename ThisClass::FHook::FTransition const InTransition,
		/// [in] InDelegate の実行優先順位。降順に実行される。
		typename ThisClass::FHook::FPriority const InPriority,
		/// [in] 条件に合致した際に実行するデリゲート。
		::FPsyqueRulesDelegate const& InDelegate)
	{
		return ThisClass::FExpressionMonitorMap::mapped_type::RegisterHook(
			this->ExpressionMonitors,
			InExpressionKey,
			InTransition,
			InPriority,
			InDelegate);
	}

	/// @brief 条件挙動を登録する。
	/// @details
	///   InExpressionKey が指す条件式の評価が
	///   ThisClass::_dispatch の呼び出し前後で変化し、
	///   InBeforeCondition から InLatestCondition へ遷移すると、
	///   InDelegate が実行される。
	/// @sa
	///   InDelegate が無効になると、対応する
	///   ThisClass::FHook は自動的に取り除かれる。
	///   明示的に取り除くには ThisClass::UnregisterHooks を使う。
	/// @return 登録した ThisClass::FHook が持つデリゲートを指すハンドル。
	///   ただし登録に失敗した場合は、空のハンドルを戻す。
	///   - InBeforeCondition と InLatestCondition が等価だと、失敗する。
	///   - InDelegate が無効だと、失敗する。
	public: ::FDelegateHandle RegisterHook(
		/// [in] InDelegate を実行するか判定する、
		/// TEvaluator::FExpression の識別値。
		typename ThisClass::FEvaluator::FExpressionKey const InExpressionKey,
		/// [in] InDelegate を実行する条件となる、条件式の古い評価。
		::EPsyqueKleene const InBeforeCondition,
		/// [in] InDelegate を実行する条件となる、条件式の新しい評価。
		::EPsyqueKleene const InLatestCondition,
		/// [in] InDelegate の実行優先順位。降順に実行される。
		typename ThisClass::FHook::FPriority const InPriority,
		/// [in] 条件に合致した際に実行するデリゲート。
		::FPsyqueRulesDelegate const& InDelegate)
	{
		return this->RegisterHook(
			InExpressionKey,
			ThisClass::FHook::MakeTransition(
				InBeforeCondition, InLatestCondition),
			InPriority,
			InDelegate);
	}

	/// @brief 指定した条件式と遷移条件とデリゲートに対応する条件挙動を取り除く。
	/// @details
	///   ThisClass::RegisterHook で登録した ThisClass::FHook のうち、
	///   以下のすべてに合致するものを取り除く。
	///   - InExpressionKey が指す条件式を参照している。
	///   - InTransition と同じ遷移条件。
	///   - InDelegate が指すデリゲートを持つ。
	public: template<typename TemplateDelegate>
	void UnregisterHooks(
		/// [in] 取り除く ThisClass::FHook に対応する
		/// TEvaluator::FExpression の識別値。
		typename ThisClass::FEvaluator::FExpressionKey const InExpressionKey,
		/// [in] 取り除く ThisClass::FHook に対応する遷移条件。
		typename ThisClass::FHook::FTransition const InTransition,
		/// [in] 取り除く ThisClass::FHook が持つデリゲートを指すインスタンス。
		/// 以下のいずれかの型のみが対応している。
		/// - Psyque::RulesEngine::FDelegateIdentifier
		/// - FPsyqueRulesDelegate
		/// - FDelegateHandle
		TemplateDelegate const& InDelegate)
	{
		auto const LocalFind(this->ExpressionMonitors.find(InExpressionKey));
		if (LocalFind != this->ExpressionMonitors.end())
		{
			LocalFind->second.UnregisterHooks(InTransition, InDelegate);
		}
	}

	/// @brief 指定した条件式と遷移条件とデリゲートに対応する条件挙動を取り除く。
	/// @details
	///   ThisClass::RegisterHook で登録した ThisClass::FHook のうち、
	///   以下のすべてに合致するものを取り除く。
	///   - InExpressionKey が指す条件式を参照している。
	///   - 以前評価が InBeforeCondition で、最新評価が
	///     InLatestCondition となる遷移条件。
	///   - InDelegate が指すデリゲートを持つ。
	public: template<typename TemplateDelegate>
	void UnregisterHooks(
		/// [in] 取り除く ThisClass::FHook に対応する
		/// TEvaluator::FExpression の識別値。
		typename ThisClass::FEvaluator::FExpressionKey const InExpressionKey,
		/// [in] 取り除く ThisClass::FHook が持つデリゲートを実行する条件となる、
		/// 条件式の古い評価。
		::EPsyqueKleene const InBeforeCondition,
		/// [in] 取り除く ThisClass::FHook が持つデリゲートを実行する条件となる、
		/// 条件式の新しい評価。
		::EPsyqueKleene const InLatestCondition,
		/// [in] 取り除く ThisClass::FHook が持つデリゲートを指すインスタンス。
		/// 以下のいずれかの型のインスタンスのみが対応している。
		/// - Psyque::RulesEngine::FDelegateIdentifier
		/// - FPsyqueRulesDelegate
		/// - FDelegateHandle
		TemplateDelegate const& InDelegate)
	{
		this->UnregisterHooks(
			InExpressionKey,
			ThisClass::FHook::MakeTransition(
				InBeforeCondition, InLatestCondition),
			InDelegate);
	}

	/// @brief 指定した条件式とデリゲートに対応する条件挙動をすべて取り除く。
	/// @details
	///   ThisClass::RegisterHook で登録した ThisClass::FHook のうち、
	///   InExpressionKey が指す条件式を参照し、かつ、
	///   InDelegate が指すデリゲートを持つものを取り除く。
	public: template<typename TemplateDelegate>
	void UnregisterHooks(
		/// [in] 取り除く ThisClass::FHook に対応する
		/// TEvaluator::FExpression の識別値。
		typename ThisClass::FEvaluator::FExpressionKey const InExpressionKey,
		/// [in] 取り除く ThisClass::FHook が持つデリゲートを指すインスタンス。
		/// 以下のいずれかの型のインスタンスのみが対応している。
		/// - Psyque::RulesEngine::FDelegateIdentifier
		/// - FPsyqueRulesDelegate
		/// - FDelegateHandle
		TemplateDelegate const& InDelegate)
	{
		auto const LocalFind(this->ExpressionMonitors.find(InExpressionKey));
		if (LocalFind != this->ExpressionMonitors.end())
		{
			LocalFind->second.UnregisterHooks(InDelegate);
		}
	}

	/// @brief 指定したデリゲートに対応する条件挙動をすべて取り除く。
	/// @details
	///   ThisClass::RegisterHook で登録した ThisClass::FHook のうち、
	///   InDelegate が指すデリゲートを持つ ThisClass::FHook をすべて取り除く。
	public: template<typename TemplateDelegate>
	void UnregisterHooks(
		/// [in] 取り除く ThisClass::FHook が持つデリゲートを指すインスタンス。
		/// 以下のいずれかの型のインスタンスのみが対応している。
		/// - Psyque::RulesEngine::FDelegateIdentifier
		/// - FPsyqueRulesDelegate
		/// - FDelegateHandle
		TemplateDelegate const& InDelegate)
	{
		for (auto& LocalExpressionMonitor: this->ExpressionMonitors)
		{
			LocalExpressionMonitor.second.UnregisterHooks(InDelegate);
		}
	}

	/// @brief 指定した条件式に対応する条件挙動をすべて取り除く。
	/// @details
	///   ThisClass::RegisterHook で登録した ThisClass::FHook のうち、
	///   InExpressionKey が指す条件式を参照する ThisClass::FHook をすべて取り除く。
	public: void UnregisterHooks(
		/// [in] 取り除く ThisClass::FHook に対応する
		/// TEvaluator::FExpression の識別値。
		typename ThisClass::FEvaluator::FExpressionKey const InExpressionKey)
	{
		this->ExpressionMonitors.erase(InExpressionKey);
	}

	/// @brief Psyque::RulesEngine 管理者以外は、この関数は使用禁止。
	/// @details 登録されている条件挙動フックを取得する。
	/// @return
	///   ThisClass::RegisterHook で登録された、
	///   InExpressionKey に対応し InDelegateHandle の指すデリゲートを持つ
	///   ThisClass::FHook の何れかを指すポインタ。
	public: template<typename TemplateDelegate>
	typename ThisClass::FHook const* _find_hook(
		/// [in] 取得する ThisClass::FHook に対応する
		/// TEvaluator::FExpression の識別値。
		typename ThisClass::FEvaluator::FExpressionKey const InExpressionKey,
		/// [in] 取得する ThisClass::FHook が持つデリゲートを指すインスタンス。
		/// 以下のいずれかの型のインスタンスのみが対応している。
		/// - Psyque::RulesEngine::FDelegateIdentifier
		/// - FPsyqueRulesDelegate
		/// - FDelegateHandle
		TemplateDelegate const& InDelegate)
	const
	{
		auto const LocalFind(this->ExpressionMonitors.find(InExpressionKey));
		return LocalFind != this->ExpressionMonitors.end()?
			LocalFind->second.FindHook(InDelegate): nullptr;
	}

	/// @brief Psyque::RulesEngine 管理者以外は、この関数は使用禁止。
	/// @details
	///   前回の ThisClass::_dispatch と今回の ThisClass::_dispatch
	///   で条件式の評価が変化した場合に、 ThisClass::RegisterHook
	///   で指定した挙動条件と合致していれば、
	///   ThisClass::FHook に通知して条件挙動関数を呼び出す。
	/// @warning
	///   前回から今回の間（基本的には1フレームの間）で条件式の評価が変化しても、
	///   前回の時点と今回の時点の評価が同じ場合は、条件挙動関数が呼び出されない。
	///   たとえば、前回から今回の間で条件式の評価が
	///   「true（前回）／false（前回と今回の間）／true（今回）」
	///   と変化した場合、条件挙動関数は呼び出されない。
	public: void _dispatch(
		/// [in,out] 条件式の評価で参照する状態貯蔵器。
		typename ThisClass::FEvaluator::FReservoir& OutReservoir,
		/// [in] 条件式の評価に使う条件評価器。
		typename ThisClass::FEvaluator const& InEvaluator)
	{
		// this->_dispatch を多重に実行しないようにロックする。
		if (this->DispatchLock)
		{
			check(false);
			return;
		}
		this->DispatchLock = true;

		// 条件式を状態監視器へ登録する。
		ThisClass::FExpressionMonitorMap::mapped_type::RegisterExpressions(
			this->StatusMonitors, this->ExpressionMonitors, InEvaluator);

		// 状態値の変化を検知し、条件式監視器へ知らせる。
		ThisClass::FStatusMonitorMap::mapped_type::NotifyStatusTransitions(
			this->StatusMonitors, this->ExpressionMonitors, OutReservoir);

		// 条件挙動フックキャッシュの作業領域を再利用する。
		check(this->CachedHooks.empty());
		auto LocalCachedHooks(this->CachedHooks);
		LocalCachedHooks.swap(this->CachedHooks);

		// 変化した状態値を参照する条件式を評価し、
		// 遷移条件に合致した条件挙動フックをキャッシュに貯めて、
		// 優先順位の降順で並び替える。
		ThisClass::FExpressionMonitorMap::mapped_type::CacheHooks(
			LocalCachedHooks,
			this->ExpressionMonitors,
			OutReservoir,
			InEvaluator);
		std::sort(
			LocalCachedHooks.begin(),
			LocalCachedHooks.end(),
			[](
				typename ThisClass::FHookCache const& InLeft,
				typename ThisClass::FHookCache const& InRight)
			->bool
			{
				return InRight.GetPriority() < InLeft.GetPriority();
			});

		// 条件式の評価が済んだので、状態変化フラグを初期化する。
		OutReservoir._reset_transitions();

		// キャッシュに貯まった条件挙動関数を呼び出す。
		for (auto const& LocalCachedHook: LocalCachedHooks)
		{
			LocalCachedHook.ExecuteDelegate();
		}

		// 条件挙動フックキャッシュの作業領域を回収する。
		if (0 < this->CachedHooks.capacity())
		{
			check(this->CachedHooks.empty());
		}
		else
		{
			LocalCachedHooks.clear();
			this->CachedHooks = MoveTemp(LocalCachedHooks);
		}
		check(this->DispatchLock);
		this->DispatchLock = false;
	}
	/// @}
	//-------------------------------------------------------------------------
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
	private: typename ThisClass::FStatusMonitorMap StatusMonitors;
	/// @brief TExpressionMonitor の辞書。
	private: typename ThisClass::FExpressionMonitorMap ExpressionMonitors;
	/// @brief THookCache の配列。
	private: typename ThisClass::FHookCache::FArray CachedHooks;
	/// @brief 多重に ThisClass::_dispatch しないためのロック。
	private: bool DispatchLock;

}; // class Psyque::RulesEngine::_private::TDispatcher

// vim: set noexpandtab:
