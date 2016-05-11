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
	/// @copydoc TDispatcher::ThisClass
	private: using ThisClass = TExpressionMonitor;

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
			BeforeEvaluation,  ///< 条件式の前回の評価の成功／失敗。
			BeforeCondition,   ///< 条件式の前回の評価。
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

	//-------------------------------------------------------------------------
	/// @copydoc TDispatcher::RegisterHook
	/// @todo
	///   TDispatcher::_dispatch の呼び出し前後で変化しなくても、遷移状態が
	///   InTransition と合致すれば、 InDelegate が実行されるようにしたい。
	public: template<typename TemplateExpressionMonitorMap>
	static ::FDelegateHandle RegisterHook(
		/// [in,out] ThisClass::FHook を登録する TExpressionMonitor の辞書。
		TemplateExpressionMonitorMap& OutExpressionMonitors,
		/// [in] InDelegate を実行するか判定する、
		/// TEvaluator::FExpression の識別値。
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

		// 条件式監視器を取得し、条件とデリゲートが等価なフックを探す。
		auto const LocalEmplace(
			OutExpressionMonitors.emplace(
				InExpressionKey,
				ThisClass(OutExpressionMonitors.get_allocator())));
		auto& LocalHooks(LocalEmplace.first->second.Hooks);
		auto const& LocalDelegateHandle(InDelegate.GetHandle());
		Psyque::RulesEngine::FDelegateIdentifier const
			LocalDelegateIdentifier(InDelegate);
		for (std::size_t i(0); i < LocalHooks.size();)
		{
			auto& LocalHook(LocalHooks[i]);
			if (LocalHook.GetDelegate().IsBound())
			{
				// 等価な条件か判定する。
				if (LocalHook.GetTransition() == InTransition)
				{
					// 等価なデリゲートか判定する。
					auto const LocalHandle(LocalHook.GetDelegate().GetHandle());
					if (LocalHandle == LocalDelegateHandle
						|| LocalHook.IsEqualDelegate(LocalDelegateIdentifier))
					{
						// 条件とデリゲートが等価なフックがすでにあるので、
						// 優先順位だけ書き換える。
						LocalHook.Priority = InPriority;
						return LocalHandle;
					}
				}
				++i;
			}
			else
			{
				LocalHooks.erase(LocalHooks.begin() + i);
			}
		}

		// 条件とデリゲートが等価なフックがないので、新たに追加する。
		LocalHooks.emplace_back(InTransition, InPriority, InDelegate);
		check(LocalDelegateHandle == LocalHooks.back().GetDelegate().GetHandle());
		return LocalDelegateHandle;
	}

	/// @brief 遷移条件とデリゲートに対応する条件挙動を取り除く。
	/// @details ThisClass::RegisterHook で登録した ThisClass::FHook のうち、
	///   以下のすべてに合致するものを取り除く。
	///   - InTransition と同じ遷移条件。
	///   - InDelegate が指すデリゲートを持つ。
	public: template<typename TemplateDelegate>
	void UnregisterHooks(
		/// [in] 取り除く ThisClass::FHook に対応する遷移条件。
		typename ThisClass::FHook::FTransition const InTransition,
		/// [in] 取り除く ThisClass::FHook が持つデリゲートを指すインスタンス。
		TemplateDelegate const& InDelegate)
	{
		if (ThisClass::FHook::IsValidDelegate(InDelegate))
		{
			this->EraseHooks(
				[&InDelegate, InTransition](
					typename ThisClass::FHook const& InHook)
				->bool
				{
					return !InHook.GetDelegate().IsBound()
						|| (InHook.GetTransition() == InTransition
							&& InHook.IsEqualDelegate(InDelegate));
				});
		}
	}

	/// @brief デリゲートに対応する条件挙動を取り除く。
	/// @details ThisClass::RegisterHook で登録した ThisClass::FHook のうち、
	///   InDelegate が指すデリゲートを持つものを取り除く。
	public: template<typename TemplateDelegate>
	void UnregisterHooks(
		/// [in] 取り除く ThisClass::FHook が持つデリゲートを指すインスタンス。
		TemplateDelegate const& InDelegate)
	{
		if (ThisClass::FHook::IsValidDelegate(InDelegate))
		{
			this->EraseHooks(
				[&InDelegate](typename ThisClass::FHook const& InHook)->bool
				{
					return !InHook.GetDelegate().IsBound()
						|| InHook.IsEqualDelegate(InDelegate);
				});
		}
	}

	/// @brief 条件挙動フックを整理する。
	/// @retval true  ThisClass::FHook がなくなった。
	/// @retval false ThisClass::FHook はまだある。
	public: bool ShrinkHooks()
	{
		this->EraseHooks(
			[](typename ThisClass::FHook const& InHook)->bool
			{
				return !InHook.GetDelegate().IsBound();
			});
		this->Hooks.shrink_to_fit();
		return this->Hooks.empty();
	}

	/// @brief デリゲートに対応する条件挙動を取得する。
	/// @details ThisClass::RegisterHook で登録した ThisClass::FHook のうち、
	///   InDelegate が指すデリゲートを持つ何れかを取得する。
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
	///   ThisClass::RegisterHook で登録した ThisClass::FHook::FTransition
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
		for (auto& LocalElement: OutExpressionMonitors)
		{
			// 条件式の評価の要求を検知する。
			auto const LocalExpressionKey(LocalElement.first);
			auto& LocalExpressionMonitor(LocalElement.second);
			if (LocalExpressionMonitor.DetectTransition(InEvaluator, LocalExpressionKey))
			{
				// 条件挙動フックをキャッシュに貯める。
				LocalExpressionMonitor.CacheHooks(
					OutCachedHooks,
					InReservoir,
					InEvaluator,
					LocalExpressionKey);
			}
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

	/// @brief 条件挙動フックを削除する。
	private: template<typename TemplateFunction>
	void EraseHooks(
		/// [in] 削除する条件挙動フックを判定する関数オブジェクト。
		TemplateFunction const& InFunction)
	{
		for (std::size_t i(0); i < this->Hooks.size();)
		{
			auto& LocalHook(this->Hooks[i]);
			if (InFunction(LocalHook))
			{
				this->Hooks.erase(this->Hooks.begin() + i);
			}
			else
			{
				++i;
			}
		}
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
		typename TemplateEvaluator::FExpressionKey const InExpressionKey)
	{
		// 条件式を評価し、古い評価から新しい評価への遷移状態を取得する。
		auto const LocalFlushCondition(
			this->Flags.test(ThisClass::EFlag::FlushCondition));
		auto const LocalBeforeEvaluation(
			this->GetBeforeEvaluation(LocalFlushCondition));
		auto const LocalTransition(
			ThisClass::FHook::MakeTransition(
				LocalBeforeEvaluation,
				this->EvaluateExpression(
					InReservoir,
					InEvaluator,
					InExpressionKey,
					LocalFlushCondition)));

		// 条件式の評価の遷移状態が条件と合致すれば、
		// 条件挙動フックをキャッシュに貯める。
		if (ThisClass::FHook::IsValidTransition(LocalTransition))
		{
			for (auto const& LocalHook: this->Hooks)
			{
				if (LocalHook.GetTransition() == LocalTransition)
				{
					OutCachedHooks.emplace_back(LocalHook, InExpressionKey);
				}
			}
		}
	}

	/// @brief 条件式を評価する。
	/// @return 条件式の評価結果。
	private: template<typename TemplateEvaluator>
	typename ::EPsyqueKleene EvaluateExpression(
		/// [in] 条件式から参照する TReservoir インスタンス。
		typename TemplateEvaluator::FReservoir const& InReservoir,
		/// [in] 評価する条件式を持つ TEvaluator インスタンス。
		TemplateEvaluator const& InEvaluator,
		/// [in] 評価する条件式の識別値。
		typename TemplateEvaluator::FExpressionKey const InExpressionKey,
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
			this->Flags.reset(ThisClass::EFlag::BeforeEvaluation);
			this->Flags.reset(ThisClass::EFlag::BeforeCondition);
			return ::EPsyqueKleene::Unknown;
		}

		// 条件式を評価し、結果を記録する。
		auto const LocalEvaluateExpression(
			InEvaluator.EvaluateExpression(InExpressionKey, InReservoir));
		this->Flags.set(
			ThisClass::EFlag::BeforeEvaluation,
			LocalEvaluateExpression != ::EPsyqueKleene::Unknown);
		this->Flags.set(
			ThisClass::EFlag::BeforeCondition,
			LocalEvaluateExpression == ::EPsyqueKleene::IsTrue);
		return this->GetBeforeEvaluation(false);
	}

	/// @brief 条件式の評価要求を検知する。
	private: template<typename TemplateEvaluator>
	bool DetectTransition(
		/// [in] 条件式の評価に使う TEvaluator インスタンス。
		TemplateEvaluator const& InEvaluator,
		/// [in] 評価する条件式の識別値。
		typename TemplateEvaluator::FExpressionKey const InExpressionKey)
	{
		if (this->Flags.test(ThisClass::EFlag::InvalidTransition)
			|| this->Flags.test(ThisClass::EFlag::ValidTransition))
		{
			return true;
		}

		// 条件式の生成と削除を検知する。
		auto const LocalExistence(
			InEvaluator.FindExpression(InExpressionKey) != nullptr);
		auto const LocalBeforeEvaluation(
			this->Flags.test(ThisClass::EFlag::BeforeEvaluation));
		auto const LocalInvalid(!LocalExistence && LocalBeforeEvaluation);
		auto const LocalValid(LocalExistence && !LocalBeforeEvaluation);
		this->Flags.set(ThisClass::EFlag::InvalidTransition, LocalInvalid);
		this->Flags.set(ThisClass::EFlag::ValidTransition, LocalValid);
		return LocalInvalid || LocalValid;
	}

	/// @brief 監視している条件式の古い評価を取得する。
	/// @return 監視している条件式の古い評価。
	private: ::EPsyqueKleene GetBeforeEvaluation(
		/// [in] 前回の評価を無視する。
		bool const InFlush)
	const PSYQUE_NOEXCEPT
	{
		return this->Flags.test(ThisClass::EFlag::BeforeEvaluation)?
			static_cast<EPsyqueKleene>(
				!InFlush && this->Flags.test(ThisClass::EFlag::BeforeCondition)):
			::EPsyqueKleene::Unknown;
	}

	//-------------------------------------------------------------------------
	/// @copydoc ThisClass::FHookArray
	private: typename ThisClass::FHookArray Hooks;
	/// @brief 条件式の評価結果を記録するフラグの集合。
	private: std::bitset<8> Flags;

}; // class Psyque::RulesEngine::_private::TExpressionMonitor

// vim: set noexpandtab:
