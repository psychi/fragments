// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RulesEngine::_private::THook
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#include <cstdint>
#include <functional>
#include <memory>

/// @cond
namespace Psyque
{
	namespace RulesEngine
	{
		namespace _private
		{
			template<typename, typename> class THook;
		} // namespace _private
	} // namespace RulesEngine
} // namespace Psyque
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 条件挙動フック。挙動条件と実行するデリゲートを保持する。
/// @tparam TemplateExpressionKey @copydoc THook::FExpressionKey
/// @tparam TemplatePriority      @copydoc THook::FPriority
template<typename TemplateExpressionKey, typename TemplatePriority>
class Psyque::RulesEngine::_private::THook
{
	private: using ThisClass = THook; ///< @copydoc TDispatcher::ThisClass

	//-------------------------------------------------------------------------
	/// @copydoc TEvaluator::FExpressionKey
	public: using FExpressionKey = TemplateExpressionKey;
	/// @brief THook::Delegate の実行優先順位。降順に実行される。
	public: using FPriority = TemplatePriority;
	/// @brief 挙動条件。 THook::Delegate を実行する条件。
	/// @details
	///   TExpressionMonitor::CacheHooks で条件式の評価の変化を検知した際に、
	///   挙動条件と条件式の評価が合致すると、 THook::Delegate が実行される。
	/// @sa
	///   UPsyqueRulesFunctionLibrary::MakeCondition
	///   で、条件式の最新の評価と前回の評価を組み合わせて作る。
	public: using FCondition = uint8;
	/// @brief 挙動条件で実行するデリゲートを表す型。
	public: using FDelegateInstance
		= ::FPsyqueRulesBehaviorDelegate::TDelegateInstanceInterface;

	//-------------------------------------------------------------------------
	/// @brief 条件挙動フックのキャッシュ。
	public: class FCache: public THook
	{
		private: using ThisClass = FCache; ///< @copydoc THook::ThisClass
		public: using Super = THook;       ///< @brief ThisClass の基底型。

		/// @brief 条件挙動フックのキャッシュを構築する。
		public: FCache(
			/// [in] キャッシュする条件挙動フック。
			THook const& InHook,
			/// [in] ThisClass::ExpressionKey の初期値。
			typename Super::FExpressionKey InExpressionKey,
			/// [in] ThisClass::CurrentEvaluation の初期値。
			EPsyqueKleene const InCurrentEvaluation,
			/// [in] ThisClass::LastEvaluation の初期値。
			EPsyqueKleene const InLastEvaluation):
		Super(InHook),
		ExpressionKey(MoveTemp(InExpressionKey)),
		CurrentEvaluation(InCurrentEvaluation),
		LastEvaluation(InLastEvaluation)
		{}

		/// @brief デリゲートを実行する。
		public: bool ExecuteDelegate() const
		{
			auto const LocalDelegate(
				static_cast<typename Super::FDelegateInstance*>(
					this->Delegate.GetDelegateInstance()));
			return LocalDelegate != nullptr
				&& LocalDelegate->ExecuteIfSafe(
					this->ExpressionKey,
					this->CurrentEvaluation,
					this->LastEvaluation);
		}

		/// @brief 条件式の識別値。
		private: typename Super::FExpressionKey ExpressionKey;
		/// @brief 条件式の最新の評価結果。
		private: EPsyqueKleene CurrentEvaluation;
		/// @brief 条件式の前回の評価結果。
		private: EPsyqueKleene LastEvaluation;

	}; // class FCache

	//---------------------------------------------------------------------
	/// @brief 条件挙動フックを構築する。
	public: THook(
		/// [in] THook::Condition の初期値。 THook::MakeCondition で作る。
		typename ThisClass::FCondition const InCondition,
		/// [in] THook::Priority の初期値。
		typename ThisClass::FPriority const InPriority,
		/// [in] THook::Delegate の初期値。
		typename ThisClass::FDelegateInstance* const InDelegate):
	Delegate((PSYQUE_ASSERT(InDelegate != nullptr), InDelegate)),
	Priority(InPriority),
	Condition(InCondition)
	{}

	//-------------------------------------------------------------------------
	/// @brief 挙動条件を取得する。
	/// @return @copydoc THook::Condition
	public: typename ThisClass::FCondition GetCondition() const PSYQUE_NOEXCEPT
	{
		return this->Condition;
	}

	/// @brief 条件挙動関数の呼び出し優先順位を取得する。
	/// @return @copydoc THook::Priority
	public: typename ThisClass::FPriority GetPriority() const PSYQUE_NOEXCEPT
	{
		return this->Priority;
	}

	/// @brief 条件式の評価の遷移と挙動条件が合致するか判定する。
	public: bool IsMatched(
		/// [in] 条件式の評価の、最新と前回を合成した値。
		typename ThisClass::FCondition const InTransition)
	const
	{
		check(InTransition != static_cast<uint8>(EPsyqueRulesUnitCondition::Invalid));
		return InTransition == (InTransition & this->GetCondition());
	}

	//---------------------------------------------------------------------
	/// @brief 条件に合致した際に実行するデリゲート。
	public: ::FDelegateBase<> Delegate;
	/// @brief デリゲートの実行優先順位。
	private: typename ThisClass::FPriority Priority;
	/// @brief デリゲートを実行する条件。
	private: typename ThisClass::FCondition Condition;

}; // class Psyque::RulesEngine::_private::THook

// vim: set noexpandtab:
