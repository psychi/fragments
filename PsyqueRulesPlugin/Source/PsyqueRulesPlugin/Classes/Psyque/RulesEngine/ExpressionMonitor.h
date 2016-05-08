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
		class FDelegateIdentifier;
		namespace _private
		{
			template<typename> class TExpressionMonitor;
		} // namespace _private
	} // namespace RulesEngine
} // namespace Psyque
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief デリゲートの識別子。
class Psyque::RulesEngine::FDelegateIdentifier
{
	/// @copydoc _private::TDispatcher::ThisClass
	private: using ThisClass = FDelegateIdentifier;

	//-------------------------------------------------------------------------
	/// @brief デリゲートインスタンスから、デリゲート識別子を構築する。
	public: explicit FDelegateIdentifier(
		/// [in] 識別子のもととなるデリゲートインスタンスを指すポインタ。
		::IDelegateInstance const* const InDelegate)
	{
		this->Reset(InDelegate);
	}

	/// @brief シングルキャストデリゲートから、デリゲート識別子を構築する。
	public: explicit FDelegateIdentifier(
		/// [in] 識別子のもととなるシングルキャストデリゲート。
		::FDelegateBase<> const& InDelegate)
	{
		this->Reset(InDelegate.GetDelegateInstance());
	}

	/// @brief 動的デリゲートから、デリゲート識別子を構築する。
	public: explicit FDelegateIdentifier(
		/// [in] 識別子のもととなる動的デリゲート。
		::TScriptDelegate<> const& InDelegate):
	Object(InDelegate.GetUObject()),
	Method(nullptr),
	Name(this->Object == nullptr? ::FName(): InDelegate.GetFunctionName())
	{}

	/// @brief オブジェクトとメソッドから、デリゲート識別子を構築する。
	public: explicit FDelegateIdentifier(
		/// [in] デリゲートから参照するオブジェクト。
		::UObject const& InObject,
		/// [in] デリゲートから参照するメソッドの名前。
		::FName const& InFunctionName = FName()):
	Object(&InObject),
	Method(nullptr),
	Name(InFunctionName.IsNone()? ThisClass::GetWildcard(): InFunctionName)
	{}

	/// @brief 等価比較演算子。
	public: bool operator==(ThisClass const& InRight) const
	{
		return this->Object == InRight.Object
			&& (this->Name == ThisClass::GetWildcard()
				|| InRight.Name == ThisClass::GetWildcard()
				|| (this->Method == InRight.Method
					&& this->Name == InRight.Name
					&& this->Name != ThisClass::GetFunctorName()));
	}

	/// @brief 不等価比較演算子。
	public: bool operator!=(ThisClass const& InRight) const
	{
		return !this->operator==(InRight);
	}

	/// @brief 空デリゲートの識別子か判定する。
	public: bool IsEmpty() const
	{
		return this->Method == nullptr && this->Name.IsNone();
	}

	//-------------------------------------------------------------------------
	/// @brief *thisを初期化する。
	private: void Reset(::IDelegateInstance const* const InDelegate)
	{
		if (!::FDelegateBase<>(const_cast<IDelegateInstance*>(InDelegate)).IsBound())
		{
			this->Name = NAME_None;
		}
		else if (InDelegate->GetType() == EDelegateInstanceType::Functor)
		{
			this->Name = ThisClass::GetFunctorName();
		}
		else
		{
			this->Object = InDelegate->GetRawUserObject();
			this->Method = InDelegate->GetRawMethodPtr();
			this->Name = InDelegate->GetFunctionName();
			check(!this->IsEmpty());
			return;
		}
		this->Object = nullptr;
		this->Method = nullptr;
	}

	/// @brief オブジェクトのすべてのメソッドを対象とする名前を取得する。
	private: static FName const& GetWildcard()
	{
		static FName const StaticWildcard(TEXT("UObject::*"));
		return StaticWildcard;
	}

	/// @brief 関数オブジェクトを示す名前を取得する。
	private: static FName const& GetFunctorName()
	{
		static FName const StaticName(TEXT("EDelegateInstanceType::Functor"));
		return StaticName;
	}

	//-------------------------------------------------------------------------
	private:
	/// @brief デリゲートから参照するオブジェクトを指すポインタ。
	void const* Object;
	/// @brief デリゲートから参照するメソッドを指すポインタ。
	void const* Method;
	/// @brief デリゲートから参照するメソッドの名前。
	FName Name;

}; // class Psyque::RulesEngine::FDelegateIdentifier

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
	///   InExpressionKey に対応する条件式の評価が
	///   InBeforeCondition から InLatestCondition へ変化すると、
	///   InDelegate が実行される。
	/// @sa
	///   InDelegate が無効になると、対応する
	///   ThisClass::FHook は自動的に削除される。明示的に削除するには
	///   ThisClass::UnregisterHooks を使う。
	/// @return InDelegate を指すハンドル。
	///   ただし登録に失敗した場合は、空のハンドルを戻す。
	///   - InBeforeCondition と InLatestCondition が等価だと、失敗する。
	///   - InDelegate が無効だと失敗する。
	public: template<typename TemplateExpressionMonitorMap>
	static ::FDelegateHandle RegisterHook(
		/// [in,out] ThisClass::FHook を登録する TExpressionMonitor の辞書。
		TemplateExpressionMonitorMap& OutExpressionMonitors,
		/// [in] InDelegate を実行するか判定する、
		/// TEvaluator::FExpression の識別値。
		typename ThisClass::FHook::FExpressionKey const InExpressionKey,
		/// [in] InDelegate を実行する条件となる、条件式の直前の評価。
		::EPsyqueKleene const InBeforeCondition,
		/// [in] InDelegate を実行する条件となる、条件式の最新の評価。
		::EPsyqueKleene const InLatestCondition,
		/// [in] InDelegate の実行優先順位。降順に実行される。
		typename ThisClass::FHook::FPriority const InPriority,
		/// [in] 実行するデリゲート。
		/// InBeforeCondition から InLatestCondition へ変化すると、実行される。
		::FPsyqueRulesDelegate const& InDelegate)
	{
		auto const LocalCondition(
			ThisClass::FHook::MakeTransition(
				InBeforeCondition, InLatestCondition));
		if (LocalCondition == 0 || !InDelegate.IsBound())
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
			if (LocalHook.Delegate.IsBound())
			{
				// 等価な条件か判定する。
				if (LocalHook.GetCondition() == LocalCondition)
				{
					// 等価なデリゲートか判定する。
					auto const LocalHookHandle(LocalHook.Delegate.GetHandle());
					if (LocalHookHandle == LocalDelegateHandle
						|| LocalDelegateIdentifier
							== Psyque::RulesEngine::FDelegateIdentifier(
								LocalHook.Delegate))
					{
						// 条件とデリゲートが等価なフックがすでにあるので、
						// 優先順位だけ書き換える。
						LocalHook.Priority = InPriority;
						return LocalHookHandle;
					}
				}
				++i;
			}
			else
			{
				LocalHook.Delegate.Unbind();
				LocalHooks.erase(LocalHooks.begin() + i);
			}
		}

		// 条件とデリゲートが等価なフックがないので、新たに追加する。
		auto const LocalDelegate(
			static_cast<typename ThisClass::FHook::FDelegateInstance*>(
				InDelegate.GetDelegateInstance()));
		check(LocalDelegate != nullptr);
		LocalHooks.emplace_back(
			LocalCondition, InPriority, LocalDelegate->CreateCopy());
		check(LocalDelegateHandle == LocalHooks.back().Delegate.GetHandle());
		return LocalDelegateHandle;
	}

	/// @brief 条件とデリゲートに対応する条件挙動を取り除く。
	/// @details ThisClass::RegisterHook で登録した ThisClass::FHook のうち、
	///   InDelegateHandle が指すデリゲートを持つものを取り除く。
	public: void UnregisterHooks(
		/// [in] 取り除く ThisClass::FHook に対応する条件式の前回の評価。
		::EPsyqueKleene const InBeforeCondition,
		/// [in] 取り除く ThisClass::FHook に対応する条件式の今回の評価。
		::EPsyqueKleene const InLatestCondition,
		/// [in] 取り除く ThisClass::FHook が持つデリゲートを指すハンドル。
		::FDelegateHandle const& InDelegateHandle)
	{
		auto const LocalCondition(
			ThisClass::FHook::MakeTransition(
				InBeforeCondition, InLatestCondition));
		this->EraseHooks(
			[&InDelegateHandle, LocalCondition](
				typename ThisClass::FHook const& InHook)
			->bool
			{
				return !InHook.Delegate.IsBound()
					|| (LocalCondition == InHook.GetCondition()
						&& InHook.Delegate.GetHandle() == InDelegateHandle);
			});
	}

	/// @brief 条件とデリゲートに対応する条件挙動を取り除く。
	/// @details ThisClass::RegisterHook で登録した ThisClass::FHook のうち、
	///   InDelegateHandle が指すデリゲートを持つものを取り除く。
	public: void UnregisterHooks(
		/// [in] 取り除く ThisClass::FHook に対応する条件式の前回の評価。
		::EPsyqueKleene const InBeforeCondition,
		/// [in] 取り除く ThisClass::FHook に対応する条件式の今回の評価。
		::EPsyqueKleene const InLatestCondition,
		/// [in] 取り除く ThisClass::FHook が持つデリゲートの識別子。
		Psyque::RulesEngine::FDelegateIdentifier const& InDelegate)
	{
		auto const LocalCondition(
			ThisClass::FHook::MakeTransition(
				InBeforeCondition, InLatestCondition));
		this->EraseHooks([&InDelegate, LocalCondition](
				typename ThisClass::FHook const& InHook)
			->bool
			{
				Psyque::RulesEngine::FDelegateIdentifier const
					LocalDelegate(InHook.Delegate);
				return LocalDelegate.IsEmpty()
					|| (LocalCondition == InHook.GetCondition()
						&& LocalDelegate == InDelegate);
			});
	}

	/// @brief デリゲートに対応する条件挙動を取り除く。
	/// @details ThisClass::RegisterHook で登録した ThisClass::FHook のうち、
	///   InDelegateHandle が指すデリゲートを持つものを取り除く。
	public: void UnregisterHooks(
		/// [in] 取り除く ThisClass::FHook が持つデリゲートを指すハンドル。
		::FDelegateHandle const& InDelegateHandle)
	{
		this->EraseHooks(
			[&InDelegateHandle](typename ThisClass::FHook const& InHook)->bool
			{
				return !InHook.Delegate.IsBound()
					|| InHook.Delegate.GetHandle() == InDelegateHandle;
			});
	}

	/// @brief UObject に対応する条件挙動を取り除く。
	/// @details ThisClass::RegisterHook で登録した ThisClass::FHook のうち、
	///   InObject を参照するデリゲートを持つものを取り除く。
	public: void UnregisterHooks(
		/// [in] 取り除く ThisClass::FHook が持つデリゲートの識別子。
		Psyque::RulesEngine::FDelegateIdentifier const& InDelegate)
	{
		this->EraseHooks(
			[&InDelegate](typename ThisClass::FHook const& InHook)->bool
			{
				Psyque::RulesEngine::FDelegateIdentifier const
					LocalDelegate(InHook.Delegate);
				return LocalDelegate.IsEmpty() || LocalDelegate == InDelegate;
			});
	}

	/// @brief 条件挙動フックを整理する。
	/// @retval true  ThisClass::FHook がなくなった。
	/// @retval false ThisClass::FHook はまだある。
	public: bool ShrinkHooks()
	{
		this->EraseHooks(
			[](typename ThisClass::FHook const& InHook)->bool
			{
				return !InHook.Delegate.IsBound();
			});
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

	private: template<typename TemplateFunction>
	void EraseHooks(TemplateFunction const& InFunction)
	{
		for (std::size_t i(0); i < this->Hooks.size();)
		{
			auto& LocalHook(this->Hooks[i]);
			if (InFunction(LocalHook))
			{
				LocalHook.Delegate.Unbind();
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
		typename TemplateEvaluator::FExpressionKey const& InExpressionKey)
	{
		// 条件式を評価し、結果が前回から変化してないか判定する。
		auto const LocalFlushCondition(
			this->Flags.test(ThisClass::EFlag::FlushCondition));
		auto const LocalBeforeEvaluation(
			this->GetBeforeEvaluation(LocalFlushCondition));
		auto const LocalLatestEvaluation(
			this->EvaluateExpression(
				InReservoir,
				InEvaluator,
				InExpressionKey,
				LocalFlushCondition));
		auto const LocalTransition(
			ThisClass::FHook::MakeTransition(
				 LocalBeforeEvaluation, LocalLatestEvaluation));
		if (LocalTransition != 0)
		{
			// 条件式の評価の変化が挙動条件と合致すれば、
			// 条件挙動フックをキャッシュに貯める。
			for (auto& LocalHook: this->Hooks)
			{
				if (LocalHook.GetCondition() == LocalTransition
					&& LocalHook.Delegate.IsBound())
				{
					OutCachedHooks.emplace_back(LocalHook, InExpressionKey);
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
			this->Flags.reset(ThisClass::EFlag::BeforeEvaluation);
			this->Flags.reset(ThisClass::EFlag::BeforeCondition);
			return EPsyqueKleene::Unknown;
		}

		// 条件式を評価し、結果を記録する。
		auto const LocalEvaluateExpression(
			InEvaluator.EvaluateExpression(InExpressionKey, InReservoir));
		this->Flags.set(
			ThisClass::EFlag::BeforeEvaluation,
			LocalEvaluateExpression != EPsyqueKleene::Unknown);
		this->Flags.set(
			ThisClass::EFlag::BeforeCondition,
			LocalEvaluateExpression == EPsyqueKleene::IsTrue);
		return this->GetBeforeEvaluation(false);
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
		auto const LocalBeforeEvaluation(
			this->Flags.test(ThisClass::EFlag::BeforeEvaluation));
		auto const LocalInvalid(!LocalExistence && LocalBeforeEvaluation);
		auto const LocalValid(LocalExistence && !LocalBeforeEvaluation);
		this->Flags.set(ThisClass::EFlag::InvalidTransition, LocalInvalid);
		this->Flags.set(ThisClass::EFlag::ValidTransition, LocalValid);
		return LocalInvalid || LocalValid;
	}

	/// @brief 監視している条件式の前回の評価を取得する。
	/// @retval 正 条件式の評価は真となった。
	/// @retval 0  条件式の評価は偽となった。
	/// @retval 負 条件式の評価に失敗した。
	private: EPsyqueKleene GetBeforeEvaluation(
		/// [in] 前回の評価を無視する。
		bool const InFlush)
	const PSYQUE_NOEXCEPT
	{
		return this->Flags.test(ThisClass::EFlag::BeforeEvaluation)?
			static_cast<EPsyqueKleene>(
				!InFlush && this->Flags.test(ThisClass::EFlag::BeforeCondition)):
			EPsyqueKleene::Unknown;
	}

	/// @brief Psyque::RulesEngine 管理者以外は使用禁止。
	/// @details 登録されている条件挙動を取得する。
	/// @return
	///   InFunction に対応する ThisClass::FHook を指すポインタ。該当する
	///   ThisClass::FHook がない場合は nullptr を返す。
	public: typename ThisClass::FHook const* _find_hook(
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
