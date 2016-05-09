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
	/// @brief 条件式の評価の遷移条件。 THook::Delegate を実行する条件。
	/// @details
	///   TExpressionMonitor::CacheHooks で条件式の評価の変化を検知した際に、
	///   条件式の前回の評価と最新の評価が合致すると、
	///   THook::Delegate が実行される。
	/// @sa
	///   ThisClass::MakeTransition
	///   で、条件式の最新の評価と前回の評価を組み合わせて作る。
	public: using FTransition = uint8;
	/// @brief 挙動条件で実行するデリゲートを表す型。
	public: using FDelegateInstance
		= ::FPsyqueRulesDelegate::TDelegateInstanceInterface;

	private: enum{TransitionBitWidth = 2};

	//---------------------------------------------------------------------
	/// @brief 条件挙動フックを構築する。
	public: THook(
		/// [in] THook::Condition の初期値。 ThisClass::MakeTransition で作る。
		typename ThisClass::FTransition const InCondition,
		/// [in] THook::Priority の初期値。
		typename ThisClass::FPriority const InPriority,
		/// [in] THook::Delegate の初期値。
		::FPsyqueRulesDelegate const& InDelegate):
	Delegate(InDelegate),
	Priority(InPriority),
	Condition(InCondition)
	{}

	//-------------------------------------------------------------------------
	/// @brief ThisClass::Delegate を実行する条件を取得する。
	/// @return @copydoc THook::Condition
	public: typename ThisClass::FTransition GetCondition() const PSYQUE_NOEXCEPT
	{
		return this->Condition;
	}

	/// @brief ThisClass::Delegate を実行する条件となる、条件式の直前の評価を取得する。
	public: EPsyqueKleene GetBeforeCondition() const
	{
		return ThisClass::GetUnitCondition(
			this->GetCondition() >> ThisClass::TransitionBitWidth);
	}

	/// @brief ThisClass::Delegate を実行する条件となる、条件式の最新の評価を取得する。
	public: EPsyqueKleene GetLatestCondition() const
	{
		return ThisClass::GetUnitCondition(this->GetCondition());
	}

	/// @brief 条件式の評価の遷移を表す値を取得する。
	public: static typename ThisClass::FTransition MakeTransition(
		/// [in] 条件式の前回の評価。
		::EPsyqueKleene const InBeforeCondition,
		/// [in] 条件式の今回の評価。
		::EPsyqueKleene const InLatestCondition)
	{
		return InBeforeCondition != InLatestCondition?
			ThisClass::GetUnitTransition(InLatestCondition) | (
				ThisClass::GetUnitTransition(InBeforeCondition)
					<< ThisClass::TransitionBitWidth):
			0;
	}

	private: static EPsyqueKleene GetUnitCondition(
		typename ThisClass::FTransition const InTransition)
	{
		auto const LocalEvaluation(
			InTransition & ((1 << ThisClass::TransitionBitWidth) - 1));
		return LocalEvaluation == static_cast<uint8>(EPsyqueKleene::IsFalse)?
			EPsyqueKleene::IsFalse:
			LocalEvaluation == static_cast<uint8>(EPsyqueKleene::IsTrue)?
				EPsyqueKleene::IsTrue: EPsyqueKleene::Unknown;
	}

	private: static typename ThisClass::FTransition GetUnitTransition(
		::EPsyqueKleene const InCondition)
	{
		return static_cast<typename ThisClass::FTransition>(InCondition) & 3;
	}

	//---------------------------------------------------------------------
	/// @brief 条件に合致した際に実行するデリゲート。
	public: ::FPsyqueRulesDelegate Delegate;
	/// @brief デリゲートの実行優先順位。
	public: typename ThisClass::FPriority Priority;
	/// @brief デリゲートを実行する遷移条件。
	private: typename ThisClass::FTransition Condition;

}; // class Psyque::RulesEngine::_private::THook

// vim: set noexpandtab:
