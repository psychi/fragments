// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RulesEngine::_private::TExpressionMonitor
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#include <cstdint>
#include <bitset>
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
/// @brief 条件式監視器。条件式の評価の変化を検知し、条件挙動フックに通知する。
/// @tparam TemplateHookArray @copydoc TExpressionMonitor::FHookArray
template<typename TemplateHookArray>
class Psyque::RulesEngine::_private::TExpressionMonitor
{
	private: using ThisClass = TExpressionMonitor; ///< @copydoc TDispatcher::ThisClass

	//-------------------------------------------------------------------------
	/// @brief TDispatcher で保持する THook のコンテナ。
	private: using FHookArray = TemplateHookArray;
	/// @brief TDispatcher で保持する THook 。
	private: using FHook = typename TemplateHookArray::value_type;

	//-------------------------------------------------------------------------
	/// @brief ThisClass::Flags の構成。
	private: struct EFlag
	{
		enum Type: uint8
		{
			ValidTransition,   ///< 状態変化の取得に成功。
			InvalidTransition, ///< 状態変化の取得に失敗。
			LastEvaluation,    ///< 条件式の前回の評価の成功／失敗。
			LastCondition,     ///< 条件式の前回の評価。
			FlushCondition,    ///< 条件式の前回の評価を無視する。
			IsRegistered,      ///< 条件式の登録済みフラグ。
		};
	};

	//-------------------------------------------------------------------------
	/// @brief 条件式監視器を構築する。
	public: explicit TExpressionMonitor(
		/// [in] メモリ割当子の初期値。
		typename ThisClass::FHookArray::allocator_type const& InAllocator):
	Hooks(InAllocator)
	{}

	/// @brief 条件式監視器を破壊する。
	public: ~TExpressionMonitor()
	{
		for (auto& LocalHook: this->Hooks)
		{
			LocalHook.Delegate.Unbind();
		}
	}

	//-------------------------------------------------------------------------
	/// @brief 条件挙動を登録する。
	/// @details
	///   TDispatcher::_dispatch で、
	///   InExpressionKey に対応する条件式の評価が変化して
	///   InCondition と合致すると、 InDelegate が実行される。
	/// @sa
	///   InDelegate が無効になると、対応する
	///   ThisClass::FHook は自動的に削除される。明示的に削除するには
	///   ThisClass::UnregisterHook を使う。
	/// @return InDelegate を指すハンドル。
	///   ただし登録に失敗した場合は、空のハンドルを戻す。
	///   - InCondition が EPsyqueRulesUnitCondition::Invalid だと、失敗する。
	///   - InDelegate が無効だと失敗する。
	///   - InExpressionKey と対応する ThisClass::FHook に、
	///     InDelegate が既に登録されていると、失敗する。
	public: template<typename TemplateExpressionMonitorMap>
	static ::FDelegateHandle RegisterHook(
		/// [in,out] ThisClass::FHook を登録する TExpressionMonitor の辞書。
		TemplateExpressionMonitorMap& OutExpressionMonitors,
		/// [in] InDelegate を実行するか判定する、
		/// TEvaluator::FExpression の識別値。
		typename ThisClass::FHook::FExpressionKey const InExpressionKey,
		/// [in] InDelegate の実行判定に合格する条件。
		/// UPsyqueRulesFunctionLibrary::MakeCondition から作る。
		typename ThisClass::FHook::FCondition const InCondition,
		/// [in] InDelegate の実行優先順位。降順に実行される。
		typename ThisClass::FHook::FPriority const InPriority,
		/// [in] 条件挙動で実行するデリゲート。
		/// InExpressionKey に対応する条件式の評価が変化して
		/// InCondition に合致すると、実行される。
		::FPsyqueRulesBehaviorDelegate const& InDelegate)
	{
		if (InCondition != static_cast<uint8>(EPsyqueRulesUnitCondition::Invalid)
			&& InDelegate.IsBound())
		{
			// 条件式監視器を用意し、同じ関数が登録されてないか判定する。
			auto const LocalEmplace(
				OutExpressionMonitors.emplace(
					InExpressionKey,
					ThisClass(OutExpressionMonitors.get_allocator())));
			auto& LocalHooks(LocalEmplace.first->second.Hooks);
			if (!ThisClass::TrimHooks(LocalHooks, InDelegate.GetHandle(), false))
			{
				// 条件式監視器へ条件挙動フックを追加する。
				auto const LocalDelegate(
					static_cast<typename ThisClass::FHook::FDelegateInstance*>(
						InDelegate.GetDelegateInstance()));
				check(LocalDelegate != nullptr);
				LocalHooks.emplace_back(
					InCondition, InPriority, LocalDelegate->CreateCopy());
				check(LocalHooks.back().Delegate.GetHandle() == InDelegate.GetHandle());
				return InDelegate.GetHandle();
			}
		}
		return ::FDelegateHandle();
	}

	/// @brief ThisClass::RegisterHook で登録した条件挙動を取り除く。
	/// @retval true  InHandle が指すデリゲートを持つ条件挙動を取り除いた。
	/// @retval false 該当する条件挙動がない。
	public: bool UnregisterHook(
		/// [in] 削除する ThisClass::FHook に対応する条件挙動関数。
		::FDelegateHandle const& InHandle)
	{
		return ThisClass::TrimHooks(this->Hooks, InHandle, true);
	}

	/// @brief 条件挙動フックを整理する。
	/// @retval true  ThisClass::FHook がなくなった。
	/// @retval false ThisClass::FHook はまだある。
	public: bool ShrinkHooks()
	{
		ThisClass::TrimHooks(this->Hooks, ::FDelegateHandle(), false);
		this->Hooks.shrink_to_fit();
		return this->Hooks.empty();
	}

	//-------------------------------------------------------------------------
	/// @brief 条件式を状態監視器へ登録する。
	/// @details
	///   OutExpressionMonitors の要素が監視している条件式から参照する
	///   状態値が変化した際に通知されるよう、監視している条件式を
	///   FStatusMonitor へ登録する。
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
						ThisClass::EFlag::FlushCondition,
						LocalRegisterExpression < 0);
				}
			}
		}
	}

	/// @brief 状態値の変化を条件式監視器へ通知する。
	public: template<
		typename TemplateExpressionMap, typename TemplateKeyArray>
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
				ThisClass::EFlag::ValidTransition:
				ThisClass::EFlag::InvalidTransition);
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

	/// @brief 条件式の評価の変化を検知し、条件挙動フックをキャッシュに貯める。
	/// @details
	///   TEvaluator::FExpression の評価が最新と前回で異なっており、且つ
	///   ThisClass::RegisterHook で登録した ThisClass::FHook::FCondition
	///   と合致するなら、 ThisClass::FHook を OutCachedHooks に貯める。
	public: template<
		typename TemplateHookCacheArray,
		typename TemplateExpressionMonitorMap,
		typename TemplateEvaluator>
	static void CacheHooks(
		/// [in,out] 挙動条件に合致した FHook::FCache を貯めるコンテナ。
		TemplateHookCacheArray& OutCachedHooks,
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
		for (
			auto i(OutExpressionMonitors.begin());
			i != OutExpressionMonitors.end();)
		{
			// 条件式の評価の要求を検知する。
			auto& LocalExpressionKey(i->first);
			auto& LocalExpressionMonitor(i->second);
			if (LocalExpressionMonitor.DetectTransition(InEvaluator, LocalExpressionKey))
			{
				// 条件挙動フックをキャッシュに貯める。
				LocalExpressionMonitor.CacheHooks(
					OutCachedHooks,
					InReservoir,
					InEvaluator,
					LocalExpressionKey);
				if (LocalExpressionMonitor.Hooks.empty())
				{
					// 条件挙動コンテナが空になったら、条件式監視器を削除する。
					i = OutExpressionMonitors.erase(i);
					continue;
				}
			}
			++i;
		}
	}

	//-------------------------------------------------------------------------
	/// @brief 条件式が参照する状態値を状態監視器へ登録する。
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
		typename TemplateEvaluator::FExpressionKey const& InRegisterKey,
		/// [in] 走査する条件式の識別値。
		typename TemplateEvaluator::FExpressionKey const& InScanKey,
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
			case EPsyqueRulesExpressionKind::SubExpression:
			return ThisClass::RegisterCompoundExpression(
				OutStatusMonitors,
				InExpressionMonitors,
				InRegisterKey,
				*LocalExpression,
				LocalChunk->SubExpressions,
				InEvaluator);

			case EPsyqueRulesExpressionKind::StatusTransition:
			TemplateStatusMonitorMap::mapped_type::RegisterExpression(
				OutStatusMonitors,
				InRegisterKey,
				*LocalExpression,
				LocalChunk->StatusTransitions);
			return -1;

			case EPsyqueRulesExpressionKind::StatusComparison:
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

	/// @brief 複合条件式を状態監視器へ登録する。
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
		typename TemplateEvaluator::FExpressionKey const& InExpressionKey,
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

	/// @brief 条件挙動フックを検索しつつ、コンテナを整理する。
	/// @retval true  OutHooks から InDelegateHandle が見つかった。
	/// @retval false OutHooks から InDelegateHandle が見つからなかった。
	private: static bool TrimHooks(
		/// [in,out] 走査する条件挙動フックのコンテナ。
		typename ThisClass::FHookArray& OutHooks,
		/// [in] 検索する挙動挙動デリゲートに対応するハンドル。
		::FDelegateHandle const& InDelegateHandle,
		/// [in] 検索する条件挙動フックを削除するかどうか。
		bool const InErase)
	{
		auto LocalFind(!InDelegateHandle.IsValid());
		for (std::size_t i(0); i < OutHooks.size();)
		{
			bool LocalErase;
			auto& LocalDelegate(OutHooks[i].Delegate);
			if (LocalFind)
			{
				LocalErase = !LocalDelegate.IsBound();
			}
			else
			{
				LocalFind = LocalDelegate.GetHandle() == InDelegateHandle;
				LocalErase = LocalFind? InErase: !LocalDelegate.IsBound();
			}
			if (LocalErase)
			{
				LocalDelegate.Unbind();
				OutHooks.erase(OutHooks.begin() + i);
			}
			else
			{
				++i;
			}
		}
		return LocalFind && InDelegateHandle.IsValid();
	}

	/// @copydoc ThisClass::CacheHooks
	private: template<
		typename TemplateHookCacheArray, typename TemplateEvaluator>
	void CacheHooks(
		/// [in,out] FHook::FCache を貯めるコンテナ。
		TemplateHookCacheArray& OutCachedHooks,
		/// [in] 評価する FEvaluator::FExpression から参照する TReservoir インスタンス。
		typename TemplateEvaluator::FReservoir const& InReservoir,
		/// [in] 評価する FEvaluator::FExpression を持つ TEvaluator インスタンス。
		TemplateEvaluator const& InEvaluator,
		/// [in] 評価する FEvaluator::FExpression の識別値。
		typename TemplateEvaluator::FExpressionKey const& InExpressionKey)
	{
		// 条件式を評価し、結果が前回から変化してないか判定する。
		auto const LocalFlushCondition(
			this->Flags.test(ThisClass::EFlag::FlushCondition));
		auto const LocalLastEvaluation(
			this->GetLastEvaluation(LocalFlushCondition));
		auto const LocalNowEvaluation(
			this->EvaluateExpression(
				InReservoir, InEvaluator, InExpressionKey, LocalFlushCondition));
		auto const LocalTransition(
			UPsyqueRulesFunctionLibrary::MakeConditionFromKleene(
				LocalNowEvaluation, LocalLastEvaluation));
		if (LocalTransition != static_cast<uint8>(EPsyqueRulesUnitCondition::Invalid))
		{
			// 条件式の評価の変化が挙動条件と合致すれば、
			// 条件挙動フックをキャッシュに貯める。
			for (std::size_t i(0); i < this->Hooks.size();)
			{
				auto& LocalHook(this->Hooks[i]);
				if (LocalHook.Delegate.IsBound())
				{
					++i;
					if (LocalHook.IsMatched(LocalTransition))
					{
						OutCachedHooks.emplace_back(
							LocalHook,
							InExpressionKey,
							LocalNowEvaluation,
							LocalLastEvaluation);
					}
				}
				else
				{
					LocalHook.Delegate.Unbind();
					this->Hooks.erase(this->Hooks.begin() + i);
				}
			}
		}
	}

	/// @brief 条件式を評価する。
	/// @retval 正 条件式の評価は真となった。
	/// @retval 0  条件式の評価は偽となった。
	/// @retval 負 条件式の評価に失敗した。
	private: template<typename TemplateEvaluator>
	typename EPsyqueKleene EvaluateExpression(
		/// [in] 条件式から参照する TReservoir インスタンス。
		typename TemplateEvaluator::FReservoir const& InReservoir,
		/// [in] 評価する条件式を持つ TEvaluator インスタンス。
		TemplateEvaluator const& InEvaluator,
		/// [in] 評価する条件式の識別値。
		typename TemplateEvaluator::FExpressionKey const& InExpressionKey,
		/// [in] 前回の評価を無視するかどうか。
		bool const InFlush)
	{
		// 状態変化フラグを更新する。
		auto const LocalInvalidTransition(
			this->Flags.test(ThisClass::EFlag::InvalidTransition));
		this->Flags.reset(ThisClass::EFlag::ValidTransition);
		this->Flags.reset(ThisClass::EFlag::InvalidTransition);

		// 状態値の取得の失敗を検知したら、条件式の評価も失敗とみなす。
		if (LocalInvalidTransition)
		//if (InFlush && LocalInvalidTransition)
		{
			this->Flags.reset(ThisClass::EFlag::LastEvaluation);
			this->Flags.reset(ThisClass::EFlag::LastCondition);
			return EPsyqueKleene::Unknown;
		}

		// 条件式を評価し、結果を記録する。
		auto const LocalEvaluateExpression(
			InEvaluator.EvaluateExpression(InExpressionKey, InReservoir));
		this->Flags.set(
			ThisClass::EFlag::LastEvaluation,
			LocalEvaluateExpression != EPsyqueKleene::Unknown);
		this->Flags.set(
			ThisClass::EFlag::LastCondition,
			LocalEvaluateExpression == EPsyqueKleene::IsTrue);
		return this->GetLastEvaluation(false);
	}

	/// @brief 条件式の評価要求を検知する。
	private: template<typename TemplateEvaluator>
	bool DetectTransition(
		/// [in] 条件式の評価に使う TEvaluator インスタンス。
		TemplateEvaluator const& InEvaluator,
		/// [in] 評価する条件式の識別値。
		typename TemplateEvaluator::FExpressionKey const& InExpressionKey)
	{
		if (this->Flags.test(ThisClass::EFlag::InvalidTransition)
			|| this->Flags.test(ThisClass::EFlag::ValidTransition))
		{
			return true;
		}

		// 条件式の生成と削除を検知する。
		auto const LocalExistence(
			InEvaluator.FindExpression(InExpressionKey) != nullptr);
		auto const LocalLastEvaluation(
			this->Flags.test(ThisClass::EFlag::LastEvaluation));
		auto const LocalInvalid(!LocalExistence && LocalLastEvaluation);
		auto const LocalValid(LocalExistence && !LocalLastEvaluation);
		this->Flags.set(ThisClass::EFlag::InvalidTransition, LocalInvalid);
		this->Flags.set(ThisClass::EFlag::ValidTransition, LocalValid);
		return LocalInvalid || LocalValid;
	}

	/// @brief 監視している条件式の前回の評価を取得する。
	/// @retval 正 条件式の評価は真となった。
	/// @retval 0  条件式の評価は偽となった。
	/// @retval 負 条件式の評価に失敗した。
	private: EPsyqueKleene GetLastEvaluation(
		/// [in] 前回の評価を無視する。
		bool const InFlush)
	const PSYQUE_NOEXCEPT
	{
		return this->Flags.test(ThisClass::EFlag::LastEvaluation)?
			static_cast<EPsyqueKleene>(
				!InFlush && this->Flags.test(ThisClass::EFlag::LastCondition)):
			EPsyqueKleene::Unknown;
	}

	/// @brief Psyque::RulesEngine 管理者以外は使用禁止。
	/// @details 登録されている条件挙動を取得する。
	/// @return
	///   InFunction に対応する ThisClass::FHook を指すポインタ。該当する
	///   ThisClass::FHook がない場合は nullptr を返す。
	public: typename ThisClass::FHook const* _find_handler_ptr(
		/// [in] 取得する ThisClass::FHook に対応するデリゲートハンドル。
		::FDelegateHandle const& InDelegateHandle)
	const PSYQUE_NOEXCEPT
	{
		if (InDelegateHandle.IsValid())
		{
			for (auto& LocalHook: this->Hooks)
			{
				if (LocalHook.Delegate.GetHandle() == InDelegateHandle)
				{
					return &LocalHook;
				}
			}
		}
		return nullptr;
	}

	//-------------------------------------------------------------------------
	/// @copydoc ThisClass::FHookArray
	private: typename ThisClass::FHookArray Hooks;
	/// @brief 条件式の評価結果を記録するフラグの集合。
	private: std::bitset<8> Flags;

}; // class Psyque::RulesEngine::_private::TExpressionMonitor

// vim: set noexpandtab:
