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

/// デリゲート実行器。条件式の評価が実行条件と合致すると、デリゲートを実行する。
///
/// @par 使い方の概略
///   - TDispatcher::RegisterDelegate で、実行条件と実行するデリゲートを登録する。
///   - TDispatcher::_dispatch
///     で状態値の変化を検知し、変化した状態値を参照する条件式を評価して、
///     条件式の評価の変化が実行条件と合致するデリゲートを実行する。
/// @tparam TemplateEvaluator @copydoc TDispatcher::FEvaluator
/// @tparam TemplatePriority  @copydoc TDispatcher::FHook::FPriority
template<typename TemplateEvaluator, typename TemplatePriority>
class Psyque::RulesEngine::_private::TDispatcher
{
	/// @copydoc RulesEngine::TDriver::ThisClass
	private: using ThisClass = TDispatcher;
	//-------------------------------------------------------------------------

	/// デリゲート実行器で使う TEvaluator の型。
	public: using FEvaluator = TemplateEvaluator;
	/// コンテナで使うメモリ割当子の型。
	public: using FAllocator = typename ThisClass::FEvaluator::FAllocator;
	/// デリゲート実行器で使う実行フック。
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
			std::vector<
				typename ThisClass::FHook, typename ThisClass::FAllocator>>,
		Psyque::Hash::TPrimitiveBits<
			typename ThisClass::FEvaluator::FExpressionKey>,
		std::equal_to<typename ThisClass::FEvaluator::FExpressionKey>,
		typename ThisClass::FAllocator>;
	//-------------------------------------------------------------------------
	/// @name 構築と代入
	/// @{

	/// 空のデリゲート実行器を構築する。
	public: TDispatcher(
		/// [in] 監視する状態値のバケット数。
		std::size_t const InStatusCapacity,
		/// [in] 監視する条件式のバケット数。
		std::size_t const InExpressionCapacity,
		/// [in] デリゲートのキャッシュ予約数。
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
	DelegateCaches(InAllocator),
	DispatchLock(false)
	{
		this->DelegateCaches.reserve(InCacheCapacity);
	}

	/// コピー構築子。
	public: TDispatcher(
		/// [in] コピー元となるインスタンス。
		ThisClass const& InSource):
	StatusMonitors(InSource.StatusMonitors),
	ExpressionMonitors(InSource.ExpressionMonitors),
	DelegateCaches(InSource.DelegateCaches.get_allocator()),
	DispatchLock(false)
	{
		this->DelegateCaches.reserve(InSource.DelegateCaches.capacity());
	}

	/// ムーブ構築子。
	public: TDispatcher(
		/// [in,out] ムーブ元となるインスタンス。
		ThisClass&& OutSource):
	StatusMonitors((
		/// @warning ThisClass::_dispatch 実行中はムーブできない。
		PSYQUE_ASSERT(!OutSource.DispatchLock),
		MoveTemp(OutSource.StatusMonitors))),
	ExpressionMonitors(MoveTemp(OutSource.ExpressionMonitors)),
	DelegateCaches(MoveTemp(OutSource.DelegateCaches)),
	DispatchLock(false)
	{}

	/// コピー代入演算子。
	/// @return *this
	public: ThisClass& operator=(
		/// [in] コピー元となるインスタンス。
		ThisClass const& InSource)
	{
		/// @warning ThisClass::_dispatch 実行中はムーブできない。
		check(!this->DispatchLock && !InSource.DispatchLock);
		this->StatusMonitors = InSource.StatusMonitors;
		this->ExpressionMonitors = InSource.ExpressionMonitors;
		this->DelegateCaches.reserve(InSource.DelegateCaches.capacity());
		return *this;
	}

	/// ムーブ代入演算子。
	/// @return *this
	public: ThisClass& operator=(
		/// [in,out] ムーブ元となるインスタンス。
		ThisClass&& OutSource)
	{
		/// @warning ThisClass::_dispatch 実行中はムーブできない。
		check(!this->DispatchLock && !OutSource.DispatchLock);
		this->StatusMonitors = MoveTemp(OutSource.StatusMonitors);
		this->ExpressionMonitors = MoveTemp(OutSource.ExpressionMonitors);
		this->DelegateCaches = MoveTemp(OutSource.DelegateCaches);
		return *this;
	}

	/// デリゲート実行器を解体する。
	public: ~TDispatcher()
	{
		/// @warning ThisClass::_dispatch 実行中は解体できない。
		check(!this->DispatchLock);
	}

	/// デリゲート実行器で使われているメモリ割当子を取得する。
	/// @return *this で使われているメモリ割当子のコピー。
	public: typename ThisClass::FAllocator _get_allocator() const PSYQUE_NOEXCEPT
	{
		return this->ExpressionMonitors.get_allocator();
	}

	/// デリゲート実行器を再構築し、メモリ領域を必要最小限にする。
	public: void Rebuild(
		/// [in] 監視する状態値のバケット数。
		std::size_t const InStatusCapacity,
		/// [in] 監視する条件式のバケット数。
		std::size_t const InExpressionCapacity,
		/// [in] デリゲートのキャッシュ予約数。
		std::size_t const InCacheCapacity)
	{
		using FExpressionMonitor =
			typename ThisClass::FExpressionMonitorMap::mapped_type;
		ThisClass::RebuildMonitors(
			this->ExpressionMonitors,
			InExpressionCapacity,
			[](FExpressionMonitor& OutExpressionMonitor)->bool
			{
				return OutExpressionMonitor.IsEmpty();
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
		check(this->DelegateCaches.empty());
		this->DelegateCaches = decltype(this->DelegateCaches)(
			this->DelegateCaches.get_allocator());
		this->DelegateCaches.reserve(InCacheCapacity);
	}
	/// @}
	//-------------------------------------------------------------------------
	/// @name デリゲート
	/// @{

	/// デリゲートを登録する。
	///
	/// InExpressionKey が指す条件式の評価が
	/// ThisClass::_dispatch の呼び出し前後で変化し、その変化が
	/// InTransition と合致していた場合に、 InDelegate が実行される。
	/// @sa
	///   InDelegate が無効になると、自動的に取り除かれる。
	///   明示的に取り除くには ThisClass::UnregisterDelegates を使う。
	/// @return 登録したデリゲートを指すハンドル。
	///   ただし登録に失敗した場合は、空のハンドルを戻す。
	///   - InTransition が無効だと、失敗する。
	///   - InDelegate が無効だと、失敗する。
	public: ::FDelegateHandle RegisterDelegate(
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
		return ThisClass::FExpressionMonitorMap::mapped_type::RegisterDelegate(
			this->ExpressionMonitors,
			InExpressionKey,
			InTransition,
			InPriority,
			InDelegate);
	}

	/// デリゲートを登録する。
	///
	/// InExpressionKey が指す条件式の評価が
	/// ThisClass::_dispatch の呼び出し前後で変化し、それが
	/// InBeforeCondition から InLatestCondition へ変化していた場合に、
	/// InDelegate が実行される。
	/// @sa
	///   InDelegate が無効になると、自動的に取り除かれる。
	///   明示的に取り除くには ThisClass::UnregisterDelegates を使う。
	/// @return 登録したデリゲートを指すハンドル。
	///   ただし登録に失敗した場合は、空のハンドルを戻す。
	///   - InBeforeCondition と InLatestCondition が等価だと、失敗する。
	///   - InDelegate が無効だと、失敗する。
	public: ::FDelegateHandle RegisterDelegate(
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
		return this->RegisterDelegate(
			InExpressionKey,
			ThisClass::FHook::MakeTransition(
				InBeforeCondition, InLatestCondition),
			InPriority,
			InDelegate);
	}

	/// デリゲートを取り除く。
	///
	/// ThisClass::RegisterDelegate で登録したデリゲートのうち、
	/// 以下のすべてが該当するものを取り除く。
	/// - InExpressionKey が指す条件式を参照している。
	/// - InTransition と同じ実行条件。
	/// - InDelegate が指すデリゲートを持つ。
	public: template<typename TemplateDelegate>
	void UnregisterDelegates(
		/// [in] 取り除くデリゲートに対応する TEvaluator::FExpression の識別値。
		typename ThisClass::FEvaluator::FExpressionKey const InExpressionKey,
		/// [in] 取り除くデリゲートに対応する実行条件。
		typename ThisClass::FHook::FTransition const InTransition,
		/// [in] 取り除くデリゲートを指すインスタンス。
		/// 以下のいずれかの型のみが対応している。
		/// - Psyque::RulesEngine::FDelegateIdentifier
		/// - FPsyqueRulesDelegate
		/// - FDelegateHandle
		TemplateDelegate const& InDelegate)
	{
		auto const LocalFind(this->ExpressionMonitors.find(InExpressionKey));
		if (LocalFind != this->ExpressionMonitors.end())
		{
			LocalFind->second.UnregisterDelegates(InTransition, InDelegate);
		}
	}

	/// デリゲートを取り除く。
	///
	/// ThisClass::RegisterDelegate で登録したデリゲートのうち、
	/// 以下のすべてが該当するものを取り除く。
	/// - InExpressionKey が指す条件式を参照している。
	/// - 実行条件の古い評価が InBeforeCondition と等値。
	/// - 実行条件の新しい評価が InLatestCondition と等値。
	/// - InDelegate が指すデリゲートを持つ。
	public: template<typename TemplateDelegate>
	void UnregisterDelegates(
		/// [in] 取り除くデリゲートに対応する TEvaluator::FExpression の識別値。
		typename ThisClass::FEvaluator::FExpressionKey const InExpressionKey,
		/// [in] 取り除くデリゲートに対応する、条件式の古い評価。
		::EPsyqueKleene const InBeforeCondition,
		/// [in] 取り除くデリゲートに対応する、条件式の新しい評価。
		::EPsyqueKleene const InLatestCondition,
		/// [in] 取り除くデリゲートを指すインスタンス。
		/// 以下のいずれかの型のインスタンスのみが対応している。
		/// - Psyque::RulesEngine::FDelegateIdentifier
		/// - FPsyqueRulesDelegate
		/// - FDelegateHandle
		TemplateDelegate const& InDelegate)
	{
		this->UnregisterDelegates(
			InExpressionKey,
			ThisClass::FHook::MakeTransition(
				InBeforeCondition, InLatestCondition),
			InDelegate);
	}

	/// デリゲートを取り除く。
	///
	/// ThisClass::RegisterDelegate で登録したデリゲートのうち、
	/// 以下のすべてが該当するものをすべて取り除く。
	/// - InExpressionKey が指す条件式を参照している。
	/// - InDelegate が指すデリゲートを持つ。
	public: template<typename TemplateDelegate>
	void UnregisterDelegates(
		/// [in] 取り除くデリゲートに対応する TEvaluator::FExpression の識別値。
		typename ThisClass::FEvaluator::FExpressionKey const InExpressionKey,
		/// [in] 取り除くデリゲートを指すインスタンス。
		/// 以下のいずれかの型のインスタンスのみが対応している。
		/// - Psyque::RulesEngine::FDelegateIdentifier
		/// - FPsyqueRulesDelegate
		/// - FDelegateHandle
		TemplateDelegate const& InDelegate)
	{
		auto const LocalFind(this->ExpressionMonitors.find(InExpressionKey));
		if (LocalFind != this->ExpressionMonitors.end())
		{
			LocalFind->second.UnregisterDelegates(InDelegate);
		}
	}

	/// デリゲートを取り除く。
	///
	/// ThisClass::RegisterDelegate で登録したデリゲートのうち、
	/// InDelegate が指すデリゲートを持つものをすべて取り除く。
	public: template<typename TemplateDelegate>
	void UnregisterDelegates(
		/// [in] 取り除くデリゲートを指すインスタンス。
		/// 以下のいずれかの型のインスタンスのみが対応している。
		/// - Psyque::RulesEngine::FDelegateIdentifier
		/// - FPsyqueRulesDelegate
		/// - FDelegateHandle
		TemplateDelegate const& InDelegate)
	{
		for (auto& LocalExpressionMonitor: this->ExpressionMonitors)
		{
			LocalExpressionMonitor.second.UnregisterDelegates(InDelegate);
		}
	}

	/// デリゲートを取り除く。
	///
	/// ThisClass::RegisterDelegate で登録したデリゲートのうち、
	/// InExpressionKey が指す条件式を参照するものをすべて取り除く。
	public: void UnregisterDelegates(
		/// [in] 取り除くデリゲートに対応する TEvaluator::FExpression の識別値。
		typename ThisClass::FEvaluator::FExpressionKey const InExpressionKey)
	{
		this->ExpressionMonitors.erase(InExpressionKey);
	}

	/// Psyque::RulesEngine 管理者以外は、この関数は使用禁止。
	///
	/// 前回の ThisClass::_dispatch と今回の ThisClass::_dispatch
	/// で条件式の評価が変化した場合に、 ThisClass::RegisterDelegate
	/// で指定した実行条件と合致していれば、
	/// ThisClass::FHook::Delegate が実行される。
	/// @warning
	///   前回から今回の間（基本的には1フレームの間）で条件式の評価が変化しても、
	///   前回の時点と今回の時点の評価が同じ場合、デリゲートは実行されない。
	///   たとえば、前回から今回の間で条件式の評価が
	///   「true（前回）／false（前回と今回の間）／true（今回）」
	///   と変化した場合、デリゲートは実行されない。
	public: void _dispatch(
		/// [in,out] 条件式の評価で参照する状態貯蔵器。
		typename ThisClass::FEvaluator::FReservoir& OutReservoir,
		/// [in] 条件式の評価に使う条件評価器。
		typename ThisClass::FEvaluator const& InEvaluator)
	{
		// 条件式を評価し、実行条件に合致したデリゲートをキャッシュに貯める。
		if (this->CacheDelegates(OutReservoir, InEvaluator))
		{
			// 条件式の評価が済んだので、状態変化フラグを初期化する。
			OutReservoir._reset_transitions();

			// キャッシュに貯めたデリゲートを実行する。
			this->ExecuteDelegates();
		}
	}
	/// @}
	//-------------------------------------------------------------------------

	/// Psyque::RulesEngine 管理者以外は、この関数は使用禁止。
	///
	/// 登録されている実行フックを取得する。
	/// @return
	///   ThisClass::RegisterDelegate で登録された、
	///   InExpressionKey に対応し InDelegateHandle の指すデリゲートを持つ
	///   ThisClass::FHook を指すポインタ。
	private: template<typename TemplateDelegate>
	typename ThisClass::FHook const* FindHook(
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

	/// 条件式を評価し、実行条件に合致したデリゲートをキャッシュに貯める。
	/// @retval true  成功。
	/// @retval false 失敗。
	private: bool CacheDelegates(
		/// [in,out] 条件式の評価で参照する状態貯蔵器。
		typename ThisClass::FEvaluator::FReservoir const& InReservoir,
		/// [in] 条件式の評価に使う条件評価器。
		typename ThisClass::FEvaluator const& InEvaluator)
	{
		// this->_dispatch を多重に実行しないようにロックする。
		if (this->DispatchLock)
		{
			check(false);
			return false;
		}
		this->DispatchLock = true;

		// 条件式を状態監視器へ登録する。
		ThisClass::FExpressionMonitorMap::mapped_type::RegisterExpressions(
			this->StatusMonitors, this->ExpressionMonitors, InEvaluator);

		// 状態値の変化を検知し、条件式監視器へ知らせる。
		ThisClass::FStatusMonitorMap::mapped_type::NotifyStatusTransitions(
			this->StatusMonitors, this->ExpressionMonitors, InReservoir);

		// 条件式を評価し、実行条件に合致したデリゲートをキャッシュに貯める。
		check(this->DelegateCaches.empty());
		this->DelegateCaches.clear();
		ThisClass::FExpressionMonitorMap::mapped_type::CacheDelegates(
			this->DelegateCaches,
			this->ExpressionMonitors,
			InReservoir,
			InEvaluator);
		return true;
	}

	/// ThisClass::CacheDelegates でキャッシュに貯めたデリゲートを実行する。
	private: void ExecuteDelegates()
	{
		// ThisClass::Rebuild に対応するため、
		// キャッシュをローカル変数に移し替えた後、デリゲートを実行する。
		auto LocalDelegateCaches(MoveTemp(this->DelegateCaches));
		//this->DelegateCaches.clear();
		ThisClass::FExpressionMonitorMap::mapped_type::ExecuteDelegates(
			LocalDelegateCaches, this->ExpressionMonitors);

		// キャッシュを空にする。
		if (0 < this->DelegateCaches.capacity())
		{
			check(this->DelegateCaches.empty());
		}
		else
		{
			this->DelegateCaches = MoveTemp(LocalDelegateCaches);
		}
		this->DelegateCaches.clear();

		// ロックを解除する。
		check(this->DispatchLock);
		this->DispatchLock = false;
	}

	/// 監視器を再構築する。
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

	/// TStatusMonitor の辞書。
	private: typename ThisClass::FStatusMonitorMap StatusMonitors;
	/// TExpressionMonitor の辞書。
	private: typename ThisClass::FExpressionMonitorMap ExpressionMonitors;
	/// 実行するデリゲートをキャッシュする配列。
	private: typename ThisClass::FExpressionMonitorMap::mapped_type::FDelegateCacheArray
		DelegateCaches;
	/// 多重に ThisClass::_dispatch しないためのロック。
	private: bool DispatchLock;

}; // class Psyque::RulesEngine::_private::TDispatcher

// vim: set noexpandtab:
