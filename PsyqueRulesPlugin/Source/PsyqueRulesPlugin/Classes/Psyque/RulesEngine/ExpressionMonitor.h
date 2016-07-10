// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RulesEngine::_private::TExpressionMonitor
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#include <cstdint>
#include <bitset>
#include <tuple>
#include <vector>

/// @cond
namespace Psyque
{
	namespace RulesEngine
	{
		namespace _private
		{
			template<typename> class TExpressionMonitor;
		} // namespace _private
	} // namespace RulesEngine
} // namespace Psyque
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// 条件式監視器。条件式の評価の変化を検知し、実行フックに通知する。
/// @tparam TemplateHookArray @copydoc TExpressionMonitor::FHookArray
template<typename TemplateHookArray>
class Psyque::RulesEngine::_private::TExpressionMonitor
{
	/// @copydoc TDispatcher::ThisClass
	private: using ThisClass = TExpressionMonitor;
	//-------------------------------------------------------------------------

	/// TDispatcher で保持する THook のコンテナ。
	private: using FHookArray = TemplateHookArray;
	/// TDispatcher で保持する THook 。
	private: using FHook = typename TemplateHookArray::value_type;

	/// デリゲートのキャッシュ。
	public: using FDelegateCacheArray = std::vector<
		std::tuple<
			// デリゲートに対応する条件式の識別値。
			typename ThisClass::FHook::FExpressionKey,
			// デリゲートに対応する実行フックのあるインデクス番号。
			typename ThisClass::FHookArray::size_type,
			// デリゲートの実行優先順位。
			typename ThisClass::FHook::FPriority>,
		typename TemplateHookArray::allocator_type>;
	//-------------------------------------------------------------------------

	/// ThisClass::Flags の型。
	private: using FFlags = std::bitset<8>;
	/// ThisClass::Flags の構成。
	private: enum EFlag: uint8
	{
		StatusValidation,     ///< 状態変化の取得に成功。
		StatusInvalidation,   ///< 状態変化の取得に失敗。
		ExpressionValidation, ///< 条件式の前回の評価の成功／失敗。
		LatestEvaluation,     ///< 条件式の前回の評価。
		FlushEvaluation,      ///< 条件式の前回の評価を無視する。
		IsRegistered,         ///< 条件式の登録済みフラグ。
	};

	//-------------------------------------------------------------------------

	/// 条件式監視器を構築する。
	public: explicit TExpressionMonitor(
		/// [in] メモリ割当子の初期値。
		typename ThisClass::FHookArray::allocator_type const& InAllocator):
	Hooks(InAllocator)
	{}
	//-------------------------------------------------------------------------

	/// @copydoc TDispatcher::RegisterDelegate
	/// @todo
	///   TDispatcher::_dispatch の呼び出し前後で変化しなくても、遷移状態が
	///   InTransition と合致すれば、 InDelegate が実行されるようにしたい。
	public: template<typename TemplateExpressionMonitorMap>
	static ::FDelegateHandle RegisterDelegate(
		/// [in,out] InDelegate を登録する TExpressionMonitor の辞書。
		TemplateExpressionMonitorMap& OutExpressionMonitors,
		/// [in] InDelegate を実行するか判定する TEvaluator::FExpression の識別値。
		typename ThisClass::FHook::FExpressionKey const InExpressionKey,
		/// [in] InDelegate を実行する条件となる、条件式の評価の遷移状態。
		typename ThisClass::FHook::FTransition const InTransition,
		/// [in] InDelegate の実行優先順位。降順に実行される。
		typename ThisClass::FHook::FPriority const InPriority,
		/// [in] 条件に合致した際に実行するデリゲート。
		::FPsyqueRulesDelegate const& InDelegate)
	{
		if (!ThisClass::FHook::IsValidTransition(InTransition)
			|| !InDelegate.IsBound())
		{
			return ::FDelegateHandle();
		}

		// 条件式監視器を取得し、実行条件とデリゲートが等価な実行フックを探す。
		auto const LocalEmplace(
			OutExpressionMonitors.emplace(
				InExpressionKey,
				ThisClass(OutExpressionMonitors.get_allocator())));
		auto& LocalHooks(LocalEmplace.first->second.Hooks);
		auto const& LocalDelegateHandle(InDelegate.GetHandle());
		Psyque::RulesEngine::FDelegateIdentifier const
			LocalDelegateIdentifier(InDelegate);
		for (auto i(LocalHooks.size()); 0 < i;)
		{
			--i;
			auto& LocalHook(LocalHooks[i]);
			if (LocalHook.GetTransition() == InTransition
				&& LocalHook.GetDelegate().IsBound())
			{
				auto const& LocalHandle(LocalHook.GetDelegate().GetHandle());
				if (LocalHandle == LocalDelegateHandle
					|| LocalHook.IsEqualDelegate(LocalDelegateIdentifier))
				{
					// 実行条件とデリゲートが等価な実行フックがすでにあるので、
					// 優先順位だけ書き換える。
					LocalHook.Priority = InPriority;
					return LocalHandle;
				}
			}
		}

		// 実行条件とデリゲートが等価な実行フックがないので、新たに追加する。
		LocalHooks.emplace_back(InTransition, InPriority, InDelegate);
		check(LocalDelegateHandle == LocalHooks.back().GetDelegate().GetHandle());
		return LocalDelegateHandle;
	}

	/// 遷移条件に対応するデリゲートを取り除く。
	///
	/// ThisClass::RegisterDelegate で登録したデリゲートのうち、
	/// 以下のすべてに合致するものを取り除く。
	/// - InTransition と同じ遷移条件。
	/// - InDelegate が指すデリゲートを持つ。
	public: template<typename TemplateDelegate>
	void UnregisterDelegates(
		/// [in] 取り除く ThisClass::FHook に対応する遷移条件。
		typename ThisClass::FHook::FTransition const InTransition,
		/// [in] 取り除く ThisClass::FHook が持つデリゲートを指すインスタンス。
		/// 以下のいずれかの型のみが対応している。
		/// - Psyque::RulesEngine::FDelegateIdentifier
		/// - FPsyqueRulesDelegate
		/// - FDelegateHandle
		TemplateDelegate const& InDelegate)
	{
		for (auto& LocalHook: this->Hooks)
		{
			if (LocalHook.GetTransition() == InTransition
				&& LocalHook.IsEqualDelegate(InDelegate))
			{
				LocalHook.UnbindDelegate();
				break;
			}
		}
	}

	/// デリゲートを取り除く。
	///
	/// ThisClass::RegisterDelegate で登録したデリゲートのうち、
	/// InDelegate が指すものを取り除く。
	public: template<typename TemplateDelegate>
	void UnregisterDelegates(
		/// [in] 取り除くデリゲートを指すインスタンス。
		/// 以下のいずれかの型のみが対応している。
		/// - Psyque::RulesEngine::FDelegateIdentifier
		/// - FPsyqueRulesDelegate
		/// - FDelegateHandle
		TemplateDelegate const& InDelegate)
	{
		for (auto& LocalHook: this->Hooks)
		{
			if (LocalHook.IsEqualDelegate(InDelegate))
			{
				LocalHook.UnbindDelegate();
			}
		}
	}

	/// デリゲートに対応する実行フックを取得する。
	///
	/// ThisClass::RegisterDelegate で登録した ThisClass::FHook のうち、
	/// InDelegate が指すデリゲートを持つ何れかを取得する。
	public: template<typename TemplateDelegate>
	typename ThisClass::FHook const* FindHook(
		/// [in] 取得する ThisClass::FHook が持つデリゲートを指すインスタンス。
		TemplateDelegate const& InDelegate)
	const
	{
		if (ThisClass::FHook::IsValidDelegate(InDelegate))
		{
			for (auto const& LocalHook: this->Hooks)
			{
				if (LocalHook.IsEqualDelegate(InDelegate))
				{
					return &LocalHook;
				}
			}
		}
		return nullptr
	}

	public: bool IsEmpty() const
	{
		return this->Hooks.empty();
	}

	//-------------------------------------------------------------------------

	/// 条件式を状態監視器へ登録する。
	///
	/// OutExpressionMonitors の要素が監視している条件式から参照する
	/// 状態値が変化した際に通知されるよう、監視している条件式を
	/// OutStatusMonitors へ登録する。
	public: template<
		typename TemplateStatusMonitorMap,
		typename TemplateExpressionMonitorMap,
		typename TemplateEvaluator>
	static void RegisterExpressions(
		/// [in,out] 条件式を登録する FStatusMonitor の辞書。
		TemplateStatusMonitorMap& OutStatusMonitors,
		/// [in,out] 条件式を監視している TExpressionMonitor の辞書。
		TemplateExpressionMonitorMap& OutExpressionMonitors,
		/// [in] 監視している条件式を持つ _private::FEvaluator 。
		TemplateEvaluator const& InEvaluator)
	{
		for (auto& LocalExpressionMonitor: OutExpressionMonitors)
		{
			auto& LocalFlags(LocalExpressionMonitor.second.Flags);
			if (!LocalFlags.test(ThisClass::EFlag::IsRegistered))
			{
				auto const LocalRegisterExpression(
					ThisClass::RegisterExpression(
						OutStatusMonitors,
						OutExpressionMonitors,
						LocalExpressionMonitor.first,
						LocalExpressionMonitor.first,
						InEvaluator));
				if (LocalRegisterExpression != 0)
				{
					LocalFlags.set(ThisClass::EFlag::IsRegistered);
					LocalFlags.set(
						ThisClass::EFlag::FlushEvaluation,
						LocalRegisterExpression < 0);
				}
			}
		}
	}

	/// 状態値の変化を条件式監視器へ通知する。
	public: template<typename TemplateExpressionMap, typename TemplateKeyArray>
	static void NotifyStatusTransition(
		/// [in,out] 状態変化の通知を受け取る TExpressionMonitor の辞書。
		TemplateExpressionMap& OutExpressionMonitors,
		/// [in,out] 状態変化を通知する FEvaluator::FExpressionKey のコンテナ。
		TemplateKeyArray& OutExpressionKeys,
		/// [in] 状態値が存在するかどうか。
		bool const InStatusExistence)
	{
		auto const LocalFlagKey(
			InStatusExistence?
				ThisClass::EFlag::StatusValidation:
				ThisClass::EFlag::StatusInvalidation);
		for (std::size_t i(0); i < OutExpressionKeys.size();)
		{
			// 状態変化を通知する条件式監視器を取得する。
			auto const LocalFind(
				OutExpressionMonitors.find(OutExpressionKeys[i]));
			if (LocalFind == OutExpressionMonitors.end())
			{
				// 監視器のない条件式を削除し、コンテナを整理する。
				OutExpressionKeys.erase(OutExpressionKeys.begin() + i);
			}
			else
			{
				++i;
				// 状態変化を条件式監視器へ知らせる。
				auto& LocalFlags(LocalFind->second.Flags);
				if (LocalFlags.test(ThisClass::EFlag::IsRegistered))
				{
					LocalFlags.set(LocalFlagKey);
				}
			}
		}
	}
	//-------------------------------------------------------------------------

	/// 条件式が参照する状態値を状態監視器へ登録する。
	/// @retval 正 成功。条件式の評価を維持する。
	/// @retval 負 成功。条件式の評価を維持しない。
	/// @retval 0  失敗。
	private: template<
		typename TemplateStatusMonitorMap,
		typename TemplateExpressionMonitorMap,
		typename TemplateEvaluator>
	static int8 RegisterExpression(
		/// [in,out] 監視する状態値を登録する FStatusMonitor の辞書。
		TemplateStatusMonitorMap& OutStatusMonitors,
		/// [in] 条件式を監視する TExpressionMonitor の辞書。
		TemplateExpressionMonitorMap const& InExpressionMonitors,
		/// [in] 登録する条件式の識別値。
		typename TemplateEvaluator::FExpressionKey const InRegisterKey,
		/// [in] 走査する条件式の識別値。
		typename TemplateEvaluator::FExpressionKey const InScanKey,
		/// [in] 走査する条件式を持つ TEvaluator インスタンス。
		TemplateEvaluator const& InEvaluator)
	{
		// InScanKey に対応する条件式と論理項要素チャンクを取得する。
		auto const LocalExpression(InEvaluator.FindExpression(InScanKey));
		if (LocalExpression == nullptr)
		{
			return 0;
		}
		auto const LocalChunk(
			InEvaluator._find_chunk(LocalExpression->GetChunkKey()));
		if (LocalChunk == nullptr)
		{
			// 条件式があれば、論理項要素チャンクもあるはず。
			check(false);
			return 0;
		}

		// InScanKey に対応する条件式の種類によって、
		// InRegisterKey の登録先を選択する。
		switch (LocalExpression->GetKind())
		{
			case ::EPsyqueRulesExpressionKind::SubExpression:
			return ThisClass::RegisterCompoundExpression(
				OutStatusMonitors,
				InExpressionMonitors,
				InRegisterKey,
				*LocalExpression,
				LocalChunk->SubExpressions,
				InEvaluator);

			case ::EPsyqueRulesExpressionKind::StatusTransition:
			TemplateStatusMonitorMap::mapped_type::RegisterExpression(
				OutStatusMonitors,
				InRegisterKey,
				*LocalExpression,
				LocalChunk->StatusTransitions);
			return -1;

			case ::EPsyqueRulesExpressionKind::StatusComparison:
			TemplateStatusMonitorMap::mapped_type::RegisterExpression(
				OutStatusMonitors,
				InRegisterKey,
				*LocalExpression,
				LocalChunk->StatusComparisons);
			return 1;

			default:
			// 未対応の条件式の種類だった。
			check(false);
			return 0;
		}
	}

	/// 複合条件式を状態監視器へ登録する。
	/// @retval 正 成功。条件式の評価を維持する。
	/// @retval 負 成功。条件式の評価を維持しない。
	/// @retval 0  失敗。
	private: template<
		typename TemplateStatusMonitorMap,
		typename TemplateExpressionMonitorMap,
		typename TemplateEvaluator>
	static int8 RegisterCompoundExpression(
		/// [in,out] 状態変化を条件式監視器に知らせる TStatusMonitor の辞書。
		TemplateStatusMonitorMap& OutStatusMonitors,
		/// [in] 条件式の評価の変化を検知する TExpressionMonitor の辞書。
		TemplateExpressionMonitorMap const& InExpressionMonitors,
		/// [in] 登録する複合条件式の識別値。
		typename TemplateEvaluator::FExpressionKey const InExpressionKey,
		/// [in] 走査する複合条件式。
		typename TemplateEvaluator::FExpression const& InExpression,
		/// [in] InExpression が参照する論理項要素コンテナ。
		typename TemplateEvaluator::FChunk::FSubExpressionArray const&
			InSubExpressions,
		/// [in] 条件式を持つ TEvaluator インスタンス。
		TemplateEvaluator const& InEvaluator)
	{
		// InExpression の論理項要素を走査し、
		// InExpressionKey を状態監視器へ登録する。
		int8 LocalResult(1);
		for (
			auto i(InExpression.GetBeginIndex());
			i < InExpression.GetEndIndex();
			++i)
		{
			auto const LocalRegisterExpression(
				ThisClass::RegisterExpression(
					OutStatusMonitors,
					InExpressionMonitors,
					InExpressionKey,
					InSubExpressions[i].GetKey(),
					InEvaluator));
			if (LocalRegisterExpression == 0)
			{
				// 無限ループを防ぐため、
				// まだ存在しない条件式を複合条件式で使うのは禁止する。
				check(false);
				return 0;
			}
			else if (LocalRegisterExpression < 0)
			{
				LocalResult = -1;
			}
		}
		return LocalResult;
	}
	//-------------------------------------------------------------------------

	/// ThisClass::CacheDelegates でキャッシュしたデリゲートを実行する。
	public: template<typename TemplateExpressionMonitorMap>
	static void ExecuteDelegates(
		/// [in] 実行するデリゲートをキャッシュしているコンテナ。
		typename ThisClass::FDelegateCacheArray const& InCaches,
		/// [in] 実行するデリゲートを保持する辞書。
		TemplateExpressionMonitorMap const& InExpressionMonitors)
	{
		// キャッシュしたデリゲートを実行する。
		for (auto& LocalCache: InCaches)
		{
			auto const LocalFind(
				InExpressionMonitors.find(std::get<0>(LocalCache)));
			if (LocalFind != InExpressionMonitors.end()
				&& std::get<1>(LocalCache) < LocalFind->second.Hooks.size())
			{
				auto& LocalHook(
					LocalFind->second.Hooks[std::get<1>(LocalCache)]);
				LocalHook.GetDelegate().ExecuteIfBound(
					std::get<0>(LocalCache),
					LocalHook.GetBeforeCondition(),
					LocalHook.GetLatestCondition());
			}
		}
	}

	/// 条件式の評価の変化を検知し、条件に合致したデリゲートをキャッシュに貯める。
	///
	/// TEvaluator::FExpression の評価が最新と前回で異なっており、且つ
	/// ThisClass::RegisterDelegate で登録した ThisClass::FHook::FTransition
	/// と合致するなら、デリゲートを OutCaches に貯める。
	public: template<
		typename TemplateExpressionMonitorMap,
		typename TemplateEvaluator>
	static void CacheDelegates(
		/// [in,out] 実行条件に合致したデリゲートをキャッシュするコンテナ。
		typename ThisClass::FDelegateCacheArray& OutCaches,
		/// [in,out] FEvaluator::FExpression の評価の変化を検知する
		/// TExpressionMonitor の辞書。
		TemplateExpressionMonitorMap& OutExpressionMonitors,
		/// [in] 評価する FEvaluator::FExpression から参照する
		/// TReservoir インスタンス。
		typename TemplateEvaluator::FReservoir const& InReservoir,
		/// [in] 評価する FEvaluator::FExpression を持つ
		/// TEvaluator インスタンス。
		TemplateEvaluator const& InEvaluator)
	{
		// 条件式の評価の要求を検知し、デリゲートをキャッシュに貯める。
		for (auto& LocalElement: OutExpressionMonitors)
		{
			LocalElement.second.CacheDelegates(
				OutCaches,
				InReservoir,
				InEvaluator,
				LocalElement.first);
		}

		// デリゲートのキャッシュを、優先順位の降順で並び替える。
		std::sort(
			OutCaches.begin(),
			OutCaches.end(),
			[](
				typename ThisClass::FDelegateCacheArray::value_type const& InLeft,
				typename ThisClass::FDelegateCacheArray::value_type const& InRight)
			->bool
			{
				return std::get<2>(InRight) < std::get<2>(InLeft);
			});
	}

	/// 条件式の評価の変化を検知し、実行フックをキャッシュに貯める。
	///
	/// TEvaluator::FExpression の評価が最新と前回で異なっており、且つ
	/// ThisClass::RegisterDelegate で登録した ThisClass::FHook::FTransition
	/// と合致するなら、デリゲートを OutCaches にキャッシュする。
	private: template<typename TemplateEvaluator>
	void CacheDelegates(
		/// [in,out] デリゲートをキャッシュするコンテナ。
		typename ThisClass::FDelegateCacheArray& OutCaches,
		/// [in] 評価する FEvaluator::FExpression から参照する
		/// TReservoir インスタンス。
		typename TemplateEvaluator::FReservoir const& InReservoir,
		/// [in] 評価する FEvaluator::FExpression を持つ
		/// TEvaluator インスタンス。
		TemplateEvaluator const& InEvaluator,
		/// [in] 評価する FEvaluator::FExpression の識別値。
		typename TemplateEvaluator::FExpressionKey const InExpressionKey)
	{
		// 条件式の、古い評価から新しい評価への遷移状態を取得する。
		auto const LocalTransition(
			ThisClass::UpdateEvaluation(
				this->Flags, InReservoir, InEvaluator, InExpressionKey));

		// 実行フックの配列を走査する。
		typename ThisClass::FHookArray::size_type LocalHookCount(0);
		for (auto& LocalHook: this->Hooks)
		{
			if (LocalHook.GetDelegate().IsBound())
			{
				// 条件式の評価の遷移状態が条件と合致したら、
				// デリゲートをキャッシュに貯める。
				check(ThisClass::FHook::IsValidTransition(LocalHook.GetTransition()));
				if (LocalHook.GetTransition() == LocalTransition)
				{
					OutCaches.emplace_back(
						InExpressionKey, LocalHookCount, LocalHook.Priority);
				}

				// デリゲートが空になったフックを詰める。
				if (&this->Hooks[LocalHookCount] != &LocalHook)
				{
					this->Hooks[LocalHookCount] = MoveTemp(LocalHook);
				}
				++LocalHookCount;
			}
		}
		if (LocalHookCount < this->Hooks.size())
		{
			this->Hooks.erase(
				this->Hooks.begin() + LocalHookCount, this->Hooks.end());
		}
	}

	/// 条件式の評価を更新し、古い評価から新しい評価への遷移状態を取得する。
	/// @return 条件式の、古い評価から新しい評価への遷移状態。
	private: template<typename TemplateEvaluator>
	static typename ThisClass::FHook::FTransition UpdateEvaluation(
		/// [in,out] 条件式の評価を記録しているフラグ集合。
		typename ThisClass::FFlags& OutFlags,
		/// [in] 評価する FEvaluator::FExpression から参照する
		/// TReservoir インスタンス。
		typename TemplateEvaluator::FReservoir const& InReservoir,
		/// [in] 条件式の評価に使う TEvaluator インスタンス。
		TemplateEvaluator const& InEvaluator,
		/// [in] 評価する条件式の識別値。
		typename TemplateEvaluator::FExpressionKey const InExpressionKey)
	{
		// 状態変化フラグを更新する。
		auto const LocalStatusInvalidation(
			OutFlags.test(ThisClass::EFlag::StatusInvalidation));
		auto const LocalStatusValidation(
			OutFlags.test(ThisClass::EFlag::StatusValidation));
		OutFlags.reset(ThisClass::EFlag::StatusValidation);
		OutFlags.reset(ThisClass::EFlag::StatusInvalidation);

		// 条件式の新しい評価を決定する。
		auto const LocalOldExpressionValidation(
			OutFlags.test(ThisClass::EFlag::ExpressionValidation));
		auto const LocalOldEvaluation(
			LocalOldExpressionValidation?
				static_cast<EPsyqueKleene>(
					OutFlags.test(ThisClass::EFlag::LatestEvaluation)):
				::EPsyqueKleene::Unknown);
		auto const LocalFlushEvaluation(
			OutFlags.test(ThisClass::EFlag::FlushEvaluation)
			&& LocalOldEvaluation == ::EPsyqueKleene::IsTrue);
		if (LocalStatusInvalidation)
		{
			OutFlags.reset(ThisClass::EFlag::ExpressionValidation);
			OutFlags.reset(ThisClass::EFlag::LatestEvaluation);
			return ThisClass::FHook::MakeTransition(
				LocalOldEvaluation, ::EPsyqueKleene::Unknown);
		}
		if (!LocalStatusValidation && !LocalFlushEvaluation)
		{
			auto const LocalNewExpressionValidation(
				InEvaluator.FindExpression(InExpressionKey) != nullptr);
			if (LocalOldExpressionValidation == LocalNewExpressionValidation)
			{
				// 条件式の評価は変化しないので、以前の評価を流用する。
				return ThisClass::FHook::MakeTransition(
					LocalOldEvaluation, LocalOldEvaluation);
			}
			if (!LocalNewExpressionValidation)
			{
				OutFlags.reset(ThisClass::EFlag::ExpressionValidation);
				OutFlags.reset(ThisClass::EFlag::LatestEvaluation);
				return ThisClass::FHook::MakeTransition(
					LocalOldEvaluation, ::EPsyqueKleene::Unknown);
			}
		}
		auto const LocalNewEvaluation(
			InEvaluator.EvaluateExpression(InExpressionKey, InReservoir));

		// 条件式の新しい評価を記録する。
		OutFlags.set(
			ThisClass::EFlag::ExpressionValidation,
			LocalNewEvaluation != ::EPsyqueKleene::Unknown);
		OutFlags.set(
			ThisClass::EFlag::LatestEvaluation,
			LocalNewEvaluation == ::EPsyqueKleene::IsTrue);

		// 条件式の、古い評価から新しい評価への遷移状態を決定する。
		return ThisClass::FHook::MakeTransition(
			LocalFlushEvaluation? ::EPsyqueKleene::IsFalse: LocalOldEvaluation,
			LocalNewEvaluation);
	}
	//-------------------------------------------------------------------------

	/// @copydoc ThisClass::FHookArray
	private: typename ThisClass::FHookArray Hooks;
	/// 条件式の評価結果を記録するフラグの集合。
	private: typename ThisClass::FFlags Flags;

}; // class Psyque::RulesEngine::_private::TExpressionMonitor
