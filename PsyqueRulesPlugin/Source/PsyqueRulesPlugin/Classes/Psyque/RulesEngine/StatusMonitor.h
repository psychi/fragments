// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RulesEngine::_private::TStatusMonitor
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#include <cstdint>
#include <vector>

/// @cond
namespace Psyque
{
	namespace RulesEngine
	{
		namespace _private
		{
			template<typename> class TStatusMonitor;
		} // namespace _private
	} // namespace RulesEngine
} // namespace Psyque
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 状態監視器。条件式の要素条件が参照する状態値を監視する。
/// @details 状態値が変化した際に、条件式の評価を更新するために使う。
/// @tparam TemplateExpressionKeyArray @copydoc TStatusMonitor::ExpressionKeys
template<typename TemplateExpressionKeyArray>
class Psyque::RulesEngine::_private::TStatusMonitor
{
	private: using ThisClass = TStatusMonitor; ///< @copydoc TDispatcher::ThisClass

	//-------------------------------------------------------------------------
	/// @copydoc ThisClass::ExpressionKeys
	public: using FExpressionKeyArray = TemplateExpressionKeyArray;

	//-------------------------------------------------------------------------
	/// @brief 状態監視器を構築する。
	public: explicit TStatusMonitor(
		/// [in] メモリ割当子の初期値。
		typename ThisClass::FExpressionKeyArray::allocator_type const&
			InAllocator):
	ExpressionKeys(InAllocator),
	LastExistence(false)
	{}

#ifdef PSYQUE_NO_STD_DEFAULTED_FUNCTION
	/// @brief ムーブ構築子。
	public: TStatusMonitor(
		/// [in,out] ムーブ元となるインスタンス。
		ThisClass&& OutSource):
	ExpressionKeys(MoveTemp(OutSource.ExpressionKeys)),
	LastExistence(MoveTemp(OutSource.LastExistence))
	{}

	/// @brief ムーブ代入演算子。
	/// @return *this
	public: ThisClass& operator=(
		/// [in,out] ムーブ元となるインスタンス。
		ThisClass&& OutSource)
	{
		this->ExpressionKeys = MoveTemp(OutSource.ExpressionKeys);
		this->LastExistence = MoveTemp(OutSource.LastExistence);
		return *this;
	}
#endif // defined(PSYQUE_NO_STD_DEFAULTED_FUNCTION)

	/// @brief 条件式識別値コンテナを整理する。
	/// @retval true  条件式識別値コンテナが空になった。
	/// @retval false 条件式識別値コンテナは空になってない。
	public: template<typename template_container>
	bool ShrinkExpressionKeys(
		/// [in] 参照する FExpressionMonitor の辞書。
		template_container const& InExpressionMonitors)
	{
		return ThisClass::ShrinkExpressionKeys(
			this->ExpressionKeys, InExpressionMonitors);
	}

	//-------------------------------------------------------------------------
	/// @brief 条件式を状態監視器へ登録する。
	/// @details
	///   InExpression が使う要素条件を走査し、要素条件が参照する状態値ごとに、
	///   InExpressionKey を TStatusMonitor へ登録する。
	public: template<
		typename TemplateStatusMonitorMap,
		typename TemplateExpression,
		typename TemplateExpressionElementArray>
	static void RegisterExpression(
		/// [in,out] 状態変化を FExpressionMonitor に知らせる、
		/// TStatusMonitor の辞書。
		TemplateStatusMonitorMap& OutStatusMonitors,
		/// [in] 登録する FEvaluator::FExpressionKey 。
		typename ThisClass::FExpressionKeyArray::value_type const&
			InExpressionKey,
		/// [in] 要素条件を走査する FEvaluator::FExpression 。
		TemplateExpression const& InExpression,
		/// [in] 条件式が参照する要素条件のコンテナ。
		TemplateExpressionElementArray const& InExpressionElements)
	{
		for (
			auto i(InExpression.GetBeginIndex());
			i < InExpression.GetEndIndex();
			++i)
		{
			// 要素条件が参照する状態値の監視器を取得し、
			// InRegisterKey を状態監視器に登録する。
			auto const LocalEmplace(
				OutStatusMonitors.emplace(
					InExpressionElements.at(i).GetKey(),
					ThisClass(OutStatusMonitors.get_allocator())));
			ThisClass::insert_expression_key(
				LocalEmplace.first->second.ExpressionKeys,
				InExpressionKey);
		}
	}

	/// @brief 状態変化を検知し、条件式監視器へ知らせる。
	public: template<
		typename TemplateStatusMonitorMap,
		typename TemplateExpressionMonitorMap,
		typename TemplateReservoir>
	static void NotifyStatusTransitions(
		/// [in,out] 状態変化を検知する TStatusMonitor のコンテナ。
		TemplateStatusMonitorMap& OutStatusMonitors,
		/// [in,out] 状態変化を知らせる FExpressionMonitor のコンテナ。
		TemplateExpressionMonitorMap& OutExpressionMonitors,
		/// [in] 状態変化を把握している _private::FReservoir 。
		TemplateReservoir const& InReservoir)
	{
		for (auto i(OutStatusMonitors.begin()); i != OutStatusMonitors.end();)
		{
			auto& LocalStatusMonitor(i->second);
			LocalStatusMonitor.NotifyTransition(
				OutExpressionMonitors, InReservoir.FindTransition(i->first));
			if (LocalStatusMonitor.ExpressionKeys.empty())
			{
				i = OutStatusMonitors.erase(i);
			}
			else
			{
				++i;
			}
		}
	}

	//-------------------------------------------------------------------------
	/// @brief 状態変化を通知する条件式を登録する。
	/// @retval true 条件式を登録した。
	/// @retval false 同じ識別値の条件式がすでに登録されていたので、何もしなかった。
	private: static bool insert_expression_key(
		/// [in,out] 状態変化を通知する条件式の識別値を挿入するコンテナ。
		typename ThisClass::FExpressionKeyArray& OutExpressionKeys,
		/// [in] 状態変化を通知する FEvaluator::FExpressionKey 。
		typename ThisClass::FExpressionKeyArray::value_type const&
			InExpressionKey)
	{
		auto const LocalLowerBound(
			std::lower_bound(
				OutExpressionKeys.begin(),
				OutExpressionKeys.end(),
				InExpressionKey));
		if (LocalLowerBound != OutExpressionKeys.end()
			&& *LocalLowerBound == InExpressionKey)
		{
			return false;
		}
		OutExpressionKeys.insert(LocalLowerBound, InExpressionKey);
		return true;
	}

	/// @brief 条件式識別値コンテナを整理する。
	/// @retval true  条件式識別値コンテナが空になった。
	/// @retval false 条件式識別値コンテナは空になってない。
	private: template<typename TemplateExpressionMonitorMap>
	static bool ShrinkExpressionKeys(
		/// [in,out] 整理する条件式識別値のコンテナ。
		typename ThisClass::FExpressionKeyArray& OutExpressionKeys,
		/// [in] 参照する FExpressionMonitor の辞書。
		TemplateExpressionMonitorMap const& InExpressions)
	{
		for (auto i(OutExpressionKeys.begin()); i != OutExpressionKeys.end();)
		{
			if (InExpressions.find(*i) != InExpressions.end())
			{
				++i;
			}
			else
			{
				i = OutExpressionKeys.erase(i);
			}
		}
		OutExpressionKeys.shrink_to_fit();
		return OutExpressionKeys.empty();
	}

	/// @brief 状態変化を検知し、条件式監視器へ通知する。
	private: template<typename TemplateExpressionMonitorMap>
	void NotifyTransition(
		/// [in,out] 状態変化を通知する FExpressionMonitor の辞書。
		TemplateExpressionMonitorMap& OutExpressionMonitors,
		/// [in] FReservoir::FindTransition の戻り値。
		EPsyqueKleene const InTransition)
	{
		// 状態変化を検知する。
		auto const LocalExistence(InTransition != EPsyqueKleene::TernaryUnknown);
		if (InTransition == EPsyqueKleene::TernaryTrue
			|| LocalExistence != this->LastExistence)
		{
			TemplateExpressionMonitorMap::mapped_type::NotifyStatusTransition(
				OutExpressionMonitors, this->ExpressionKeys, LocalExistence);
		}
		this->LastExistence = LocalExistence;
	}

	//-------------------------------------------------------------------------
	/// @brief 評価の更新を要求する TEvaluator::FExpressionKey のコンテナ。
	private: typename ThisClass::FExpressionKeyArray ExpressionKeys;
	/// @brief 前回の NotifyTransition で、状態値が存在したか。
	private: bool LastExistence;

}; // class Psyque::RulesEngine::_private::TStatusMonitor

// vim: set noexpandtab:
