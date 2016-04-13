// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RulesEngine::_private::THandler
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
			template<typename, typename> class THandler;
		} // namespace _private
	} // namespace RulesEngine
} // namespace Psyque
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 条件挙動ハンドラ。関数を呼び出す条件を保持し、呼び出される関数を弱参照する。
/// @tparam TemplateExpressionKey @copydoc THandler::FExpressionKey
/// @tparam TemplatePriority      @copydoc THandler::FPriority
template<typename TemplateExpressionKey, typename TemplatePriority>
class Psyque::RulesEngine::_private::THandler
{
	private: using ThisClass = THandler; ///< @copydoc TDispatcher::ThisClass

	//-------------------------------------------------------------------------
	/// @copydoc TEvaluator::FExpressionKey
	public: using FExpressionKey = TemplateExpressionKey;
	/// @brief THandler::FFunction の呼び出し優先順位。
	/// @details 優先順位の昇順で呼び出される。
	public: using FPriority = TemplatePriority;
	/// @brief 挙動条件。条件挙動関数を呼び出す条件。
	/// @details
	///   TExpressionMonitor::CacheHandlers で条件式の評価の変化を検知した際に、
	///   挙動条件と条件式の評価が合致すると、 THandler::FFunction が呼び出される。
	/// @sa
	///   THandler::MakeCondition
	///   で、条件式の最新の評価と前回の評価を組み合わせて作る。
	public: using FCondition = uint8;
	/// @brief 単位条件。 THandler::FCondition を構成する単位となる条件。
	/// @details THandler::MakeCondition で条件を作る引数として使う。
	public: struct EUnitCondition
	{
		enum Type: typename THandler::FCondition
		{
			/// @brief 無効な条件。
			Invalid = 0,
			/// @brief 条件式の評価に失敗していることが条件。
			Failed = 1,
			/// @brief 条件式の評価が偽であることが条件。
			False = 2,
			/// @brief 条件式の評価が真であることが条件。
			True = 4,
			/// @brief 条件式の評価に失敗してないことが条件。
			NotFailed = False | True,
			/// @brief 条件式の評価が偽以外であることが条件。
			NotFalse = Failed | True,
			/// @brief 条件式の評価が真以外であることが条件。
			NotTrue = False | Failed,
			/// @brief 条件式の評価を問わない。
			Any = Failed | False | True,
		};
	};

	//-------------------------------------------------------------------------
	/// @brief 条件挙動関数。挙動条件に合致すると呼び出される関数。
	/// @details
	///   TExpressionMonitor::CacheHandlers で条件式の評価の変化を検知した際に、
	///   THandler::FCondition と条件式の評価が合致すると、呼び出される関数。
	///   - 引数#0は、評価が変化した条件式の識別値。
	///   - 引数#1は、 TEvaluator::EvaluateExpression の最新の戻り値。
	///   - 引数#2は、 TEvaluator::EvaluateExpression の前回の戻り値。
	public: using FFunction = std::function<
		void (
			typename ThisClass::FExpressionKey const&,
			EPsyqueKleene const,
			EPsyqueKleene const)>;
	/// @brief THandler::FFunction の強参照スマートポインタ。
	public: using FFunctionSharedPtr = std::shared_ptr<
		typename ThisClass::FFunction>;
	/// @brief THandler::FFunction の弱参照スマートポインタ。
	public: using FFunctionWeakPtr = std::weak_ptr<typename ThisClass::FFunction>;

	//-------------------------------------------------------------------------
	private: enum: uint8
	{
		UnitConditionBitWidth = 3, ///< 単位条件に使うビット幅。
	};

	//-------------------------------------------------------------------------
	/// @brief 条件挙動ハンドラのキャッシュ。
	public: class FCache: public THandler
	{
		private: using ThisClass = FCache;   ///< @copydoc THandler::ThisClass
		public: using Super = THandler; ///< @brief ThisClass の基底型。

		/// @brief 条件挙動ハンドラのキャッシュを構築する。
		public: FCache(
			/// [in] キャッシュする条件挙動ハンドラ。
			THandler const& InHandler,
			/// [in] ThisClass::ExpressionKey の初期値。
			typename Super::FExpressionKey InExpressionKey,
			/// [in] ThisClass::CurrentEvaluation の初期値。
			EPsyqueKleene const InCurrentEvaluation,
			/// [in] ThisClass::LastEvaluation の初期値。
			EPsyqueKleene const InLastEvaluation):
		Super(InHandler),
		ExpressionKey(MoveTemp(InExpressionKey)),
		CurrentEvaluation(InCurrentEvaluation),
		LastEvaluation(InLastEvaluation)
		{}

#ifdef PSYQUE_NO_STD_DEFAULTED_FUNCTION
		/// @brief ムーブ構築子。
		public: FCache(
			/// [in,out] ムーブ元となるインスタンス。
			ThisClass&& OutSource):
		Super(MoveTemp(OutSource)),
		ExpressionKey(MoveTemp(OutSource.ExpressionKey)),
		CurrentEvaluation(MoveTemp(OutSource.CurrentEvaluation)),
		LastEvaluation(MoveTemp(OutSource.LastEvaluation))
		{}

		/// @brief ムーブ代入演算子。
		/// @return *this
		public: ThisClass& operator=(
			/// [in,out] ムーブ元となるインスタンス。
			ThisClass&& OutSource)
		{
			this->Super::operator=(MoveTemp(OutSource));
			this->ExpressionKey = MoveTemp(OutSource.ExpressionKey);
			this->CurrentEvaluation = MoveTemp(OutSource.CurrentEvaluation);
			this->LastEvaluation = MoveTemp(OutSource.LastEvaluation);
			return *this;
		}
#endif // defined(PSYQUE_NO_STD_DEFAULTED_FUNCTION)

		/// @brief 条件挙動関数を呼び出す。
		public: void CallFunction() const
		{
			auto const LocalFunctionHolder(this->GetFunction().lock());
			auto const LocalFunction(LocalFunctionHolder.get());
			if (LocalFunction != nullptr)
			{
				(*LocalFunction)(
					this->ExpressionKey,
					this->CurrentEvaluation,
					this->LastEvaluation);
			}
		}

		/// @brief 条件式の識別値。
		private: typename Super::FExpressionKey ExpressionKey;
		/// @brief 条件式の最新の評価結果。
		private: EPsyqueKleene CurrentEvaluation;
		/// @brief 条件式の前回の評価結果。
		private: EPsyqueKleene LastEvaluation;

	}; // class FCache

	//---------------------------------------------------------------------
	/// @brief 条件挙動ハンドラを構築する。
	public: THandler(
		/// [in] THandler::Condition の初期値。 THandler::MakeCondition で作る。
		typename ThisClass::FCondition const InCondition,
		/// [in] THandler::Function の初期値。
		typename ThisClass::FFunctionWeakPtr InFunction,
		/// [in] THandler::Priority の初期値。
		typename ThisClass::FPriority const InPriority):
	Function(MoveTemp(InFunction)),
	Priority(InPriority),
	Condition(InCondition)
	{}

#ifdef PSYQUE_NO_STD_DEFAULTED_FUNCTION
	/// @brief ムーブ構築子。
	public: THandler(
		/// [in,out] ムーブ元となるインスタンス。
		ThisClass&& OutSource):
	Function(MoveTemp(OutSource.Function)),
	Priority(MoveTemp(OutSource.Priority)),
	Condition(MoveTemp(OutSource.Condition))
	{}

	/// @brief ムーブ代入演算子。
	/// @return *this
	public: ThisClass& operator=(
		/// [in,out] ムーブ元となるインスタンス。
		ThisClass&& OutSource)
	{
		this->Function = MoveTemp(OutSource.Function);
		this->Priority = MoveTemp(OutSource.Priority);
		this->Condition = MoveTemp(OutSource.Condition);
		return *this;
	}
#endif // defined(PSYQUE_NO_STD_DEFAULTED_FUNCTION)

	//-------------------------------------------------------------------------
	/// @brief 挙動条件を取得する。
	/// @return @copydoc THandler::Condition
	public: typename ThisClass::FCondition GetCondition() const PSYQUE_NOEXCEPT
	{
		return this->Condition;
	}

	/// @brief 条件挙動関数を取得する。
	/// @return @copydoc THandler::Function
	public: typename ThisClass::FFunctionWeakPtr const& GetFunction()
	const PSYQUE_NOEXCEPT
	{
		return this->Function;
	}

	/// @brief 条件挙動関数の呼び出し優先順位を取得する。
	/// @return @copydoc THandler::Priority
	public: typename ThisClass::FPriority GetPriority() const PSYQUE_NOEXCEPT
	{
		return this->Priority;
	}

	//-------------------------------------------------------------------------
	/// @brief 条件式の評価の遷移と挙動条件が合致するか判定する。
	public: bool IsMatched(
		/// [in] 条件式の評価の、最新と前回を合成した値。
		typename ThisClass::FCondition const InTransition)
	const
	{
		return (
			PSYQUE_ASSERT(InTransition != ThisClass::EUnitCondition::Invalid),
			InTransition == (InTransition & this->GetCondition()));
	}

	/// @brief 単位条件を合成して挙動条件を作る。
	/// @warning
	///   条件式の評価が最新と前回で同じ場合は、
	///   FExpressionMonitor::CacheHandlers で挙動条件の判定が行われない。
	///   このため、以下の単位条件の組み合わせは無効となることに注意。
	///   @code
	///     MakeCondition(THandler::EUnitCondition::Failed, THandler::EUnitCondition::Failed);
	///     MakeCondition(THandler::EUnitCondition::False, THandler::EUnitCondition::False);
	///     MakeCondition(THandler::EUnitCondition::True, THandler::EUnitCondition::True);
	///   @endcode
	/// @return
	///   関数が呼び出される挙動条件。単位条件の組み合わせが無効な場合は
	///   ThisClass::EUnitCondition::Invalid を返す。
	public: static typename ThisClass::FCondition MakeCondition(
		/// [in] 条件式の、最新の評価の単位条件。
		typename ThisClass::EUnitCondition::Type const InNowCondition,
		/// [in] 条件式の、前回の評価の単位条件。
		typename ThisClass::EUnitCondition::Type const InLastCondition)
	{
		return ThisClass::MixUnitCondition(
			InNowCondition != ThisClass::EUnitCondition::Invalid
				&& InLastCondition != ThisClass::EUnitCondition::Invalid
				&& (InNowCondition != InLastCondition
					// 2のべき乗か判定する。
					|| (InNowCondition & (InNowCondition - 1)) != 0),
			InNowCondition,
			InLastCondition);
	}

	/// @brief 条件式の評価を合成して挙動条件を作る。
	/// @warning
	///   条件式の評価が最新と前回で同じ場合は、
	///   FExpressionMonitor::CacheHandlers で挙動条件の判定が行われない。
	///   このため、以下の評価の組み合わせは無効となることに注意。
	///   @code
	///     // NとMは、それぞれ任意の正の整数。
	///     MakeCondition(N, M);
	///     MakeCondition(0, 0);
	///     MakeCondition(-N, -M);
	///   @endcode
	/// @return
	///   関数が呼び出される挙動条件。評価の組み合わせが無効な場合は
	///   ThisClass::EUnitCondition::Invalid を返す。
	public: static typename ThisClass::FCondition MakeCondition(
		/// [in] 条件となる、条件式の最新の評価。
		EPsyqueKleene const InNowEvaluation,
		/// [in] 条件となる、条件式の前回の評価。
		EPsyqueKleene const InLastEvaluation)
	{
		auto const LocalNowCondition(
			ThisClass::MakeUnitCondition(InNowEvaluation));
		auto const LocalLastCondition(
			ThisClass::MakeUnitCondition(InLastEvaluation));
		return ThisClass::MixUnitCondition(
			LocalNowCondition != LocalLastCondition,
			LocalNowCondition,
			LocalLastCondition);
	}

	/// @brief 条件式の評価から単位条件を作る。
	public: static typename ThisClass::EUnitCondition::Type MakeUnitCondition(
		/// [in] 条件式の評価。
		EPsyqueKleene const InEvaluation)
	{
		return InEvaluation == EPsyqueKleene::TernaryTrue?
			ThisClass::EUnitCondition::True:
			InEvaluation == EPsyqueKleene::TernaryFalse?
				ThisClass::EUnitCondition::False:
				ThisClass::EUnitCondition::Failed;
	}

	/// @brief 単位条件を合成して挙動条件を作る。
	/// @return 挙動条件。
	private: static typename ThisClass::FCondition MixUnitCondition(
		/// [in] 合成可能かどうか。
		bool const InMix,
		/// [in] 条件となる、最新の条件式の評価。
		typename ThisClass::EUnitCondition::Type const InNowCondition,
		/// [in] 条件となる、前回の条件式の評価。
		typename ThisClass::EUnitCondition::Type const InLastCondition)
	{
		return InMix?
			InNowCondition | (InLastCondition << ThisClass::UnitConditionBitWidth):
			ThisClass::EUnitCondition::Invalid;
	}

	//---------------------------------------------------------------------
	/// @brief 条件挙動関数を指すスマートポインタ。
	private: typename ThisClass::FFunctionWeakPtr Function;
	/// @brief 条件挙動関数の呼び出し優先順位。
	private: typename ThisClass::FPriority Priority;
	/// @brief 条件挙動関数を呼び出す挙動条件。
	private: typename ThisClass::FCondition Condition;

}; // class Psyque::RulesEngine::_private::THandler

// vim: set noexpandtab:
