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
/// @brief 条件式監視器。条件式の評価の変化を検知し、条件挙動ハンドラに通知する。
/// @tparam TemplateHandlerArray @copydoc TExpressionMonitor::FHandlerArray
template<typename TemplateHandlerArray>
class Psyque::RulesEngine::_private::TExpressionMonitor
{
	private: using ThisClass = TExpressionMonitor; ///< @copydoc TDispatcher::ThisClass

	//-------------------------------------------------------------------------
	/// @brief 条件式監視器で保持する THandler のコンテナ。
	private: using FHandlerArray = TemplateHandlerArray;
	/// @brief 条件式監視器で保持する THandler 。
	private: using FHandler = typename TemplateHandlerArray::value_type;

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
		typename ThisClass::FHandlerArray::allocator_type const& InAllocator):
	Handlers(InAllocator)
	{}

#ifdef PSYQUE_NO_STD_DEFAULTED_FUNCTION
	/// @brief ムーブ構築子。
	public: TExpressionMonitor(
		/// [in,out] ムーブ元となるインスタンス。
		ThisClass&& OutSource):
	Handlers(MoveTemp(OutSource.Handlers)),
	Flags(MoveTemp(OutSource.Flags))
	{}

	/// @brief ムーブ代入演算子。
	/// @return *this
	public: ThisClass& operator=(
		/// [in,out] ムーブ元となるインスタンス。
		ThisClass&& OutSource)
	{
		if (this != &OutSource)
		{
			this->Handlers = MoveTemp(OutSource.Handlers);
			this->Flags = MoveTemp(OutSource.Flags);
		}
		return *this;
	}
#endif // !defined(PSYQUE_NO_STD_DEFAULTED_FUNCTION)

	//-------------------------------------------------------------------------
	/// @brief 条件挙動ハンドラを登録する。
	/// @sa
	///   TDispatcher::_dispatch で、 InExpressionKey に対応する条件式の評価が変化し
	///   InCondition と合致すると、 InFunction の指す条件挙動関数が呼び出される。
	/// @sa
	///   InFunction の指す条件挙動関数が解体されると、それを弱参照している
	///   ThisClass::FHandler は自動的に削除される。明示的に削除するには
	///   ThisClass::UnregisterHandler を使う。
	/// @retval true
	///   成功。 InFunction の指す条件挙動関数を弱参照する
	///   ThisClass::FHandler を構築し、 OutExpressionMonitors に登録した。
	/// @retval false
	///   失敗。 ThisClass::FHandler は構築されなかった。
	///   - InCondition が ThisClass::FHandler::EUnitCondition::Invalid だと、失敗する。
	///   - InFunction が空か、空の関数を指していると、失敗する。
	///   - InExpressionKey と対応する ThisClass::FHandler に、
	///     InFunction の指す条件挙動関数が既に登録されていると、失敗する。
	public: template<typename TemplateExpressionMonitorMap>
	static bool RegisterHandler(
		/// [in,out] ThisClass::FHandler を登録する TExpressionMonitor の辞書。
		TemplateExpressionMonitorMap& OutExpressionMonitors,
		/// [in] InFunction の指す条件挙動関数に対応する
		/// FEvaluator::FExpression の識別値。
		typename ThisClass::FHandler::FExpressionKey const& InExpressionKey,
		/// [in] InFunction の指す条件挙動関数を呼び出す挙動条件。
		/// FHandler::MakeCondition から作る。
		typename ThisClass::FHandler::FCondition const InCondition,
		/// [in] 登録する FHandler::FFunction を強参照するスマートポインタ。
		/// InExpressionKey に対応する条件式の評価が変化して
		/// InCondition に合致すると、呼び出される。
		typename ThisClass::FHandler::FFunctionSharedPtr const& InFunction,
		/// [in] InFunction の指す条件挙動関数の呼び出し優先順位。
		/// 昇順に呼び出される。
		typename ThisClass::FHandler::FPriority const InPriority)
	{
		auto const LocalFunction(InFunction.get());
		if (InCondition != ThisClass::FHandler::EUnitCondition::Invalid
			&& LocalFunction != nullptr
			&& static_cast<bool>(*LocalFunction))
		{
			// 条件式監視器を用意し、同じ関数が登録されてないか判定する。
			auto const LocalEmplace(
				OutExpressionMonitors.emplace(
					InExpressionKey,
					ThisClass(OutExpressionMonitors.get_allocator())));
			auto& LocalHandlers(LocalEmplace.first->second.Handlers);
			if (LocalEmplace.second
				|| !ThisClass::trim_handlers(LocalHandlers, LocalFunction, false))
			{
				// 条件式監視器へ条件挙動ハンドラを追加する。
				LocalHandlers.emplace_back(InCondition, InFunction, InPriority);
				return true;
			}
		}
		return false;
	}

	/// @brief ThisClass::RegisterHandler で登録した条件挙動ハンドラを取り除く。
	/// @retval true  InFunction を弱参照している ThisClass::FHandler を取り除いた。
	/// @retval false 該当する ThisClass::FHandler がない。
	public: bool UnregisterHandler(
		/// [in] 削除する ThisClass::FHandler に対応する条件挙動関数。
		typename ThisClass::FHandler::FFunction const& InFunction)
	{
		return ThisClass::trim_handlers(this->Handlers, &InFunction, true);
	}

	/// @brief 条件挙動ハンドラを整理する。
	/// @retval true  ThisClass::FHandler がなくなった。
	/// @retval false ThisClass::FHandler はまだある。
	public: bool ShrinkHandlers()
	{
		ThisClass::trim_handlers(this->Handlers, nullptr, false);
		this->Handlers.shrink_to_fit();
		return this->Handlers.empty();
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
			auto& local_flags(LocalExpressionMonitor.second.Flags);
			if (!local_flags.test(ThisClass::EFlag::IsRegistered))
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
					local_flags.set(ThisClass::EFlag::IsRegistered);
					local_flags.set(
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
		auto const local_flag_key(
			InStatusExistence?
				ThisClass::EFlag::ValidTransition:
				ThisClass::EFlag::InvalidTransition);
		for (auto i(OutExpressionKeys.begin()); i != OutExpressionKeys.end();)
		{
			// 状態変化を通知する条件式監視器を取得する。
			auto const LocalFind(OutExpressionMonitors.find(*i));
			if (LocalFind == OutExpressionMonitors.end())
			{
				// 監視器のない条件式を削除し、コンテナを整理する。
				i = OutExpressionKeys.erase(i);
			}
			else
			{
				++i;
				// 状態変化を条件式監視器へ知らせる。
				auto& local_flags(LocalFind->second.Flags);
				if (local_flags.test(ThisClass::EFlag::IsRegistered))
				{
					local_flags.set(local_flag_key);
				}
			}
		}
	}

	/// @brief 条件式の評価の変化を検知し、条件挙動ハンドラをキャッシュに貯める。
	/// @details
	///   TEvaluator::FExpression の評価が最新と前回で異なっており、且つ
	///   ThisClass::RegisterHandler で登録した ThisClass::FHandler::FCondition
	///   と合致するなら、 ThisClass::FHandler を OutCachedHandlers に貯める。
	public: template<
		typename TemplateHandlerCacheArray,
		typename TemplateExpressionMonitorMap,
		typename TemplateEvaluator>
	static void CacheHandlers(
		/// [in,out] 挙動条件に合致した FHandler::FCache を貯めるコンテナ。
		TemplateHandlerCacheArray& OutCachedHandlers,
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
			if (LocalExpressionMonitor.DetectTransition(
					InEvaluator, LocalExpressionKey))
			{
				// 条件挙動ハンドラをキャッシュに貯める。
				LocalExpressionMonitor.CacheHandlers(
					OutCachedHandlers,
					InReservoir,
					InEvaluator,
					LocalExpressionKey);
				if (LocalExpressionMonitor.Handlers.empty())
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
		// InScanKey に対応する条件式と要素条件チャンクを取得する。
		auto const LocalExpression(InEvaluator.FindExpression(InScanKey));
		if (LocalExpression.IsEmpty())
		{
			return 0;
		}
		auto const LocalChunk(
			InEvaluator._find_chunk(LocalExpression.GetChunkKey()));
		if (LocalChunk == nullptr)
		{
			// 条件式があれば、要素条件チャンクもあるはず。
			check(false);
			return 0;
		}

		// InScanKey に対応する条件式の種類によって、
		// InRegisterKey の登録先を選択する。
		switch (LocalExpression.GetKind())
		{
			case EExpressionKind::SubExpression:
			return ThisClass::RegisterCompoundExpression(
				OutStatusMonitors,
				InExpressionMonitors,
				InRegisterKey,
				LocalExpression,
				LocalChunk->SubExpressions,
				InEvaluator);

			case EExpressionKind::StatusTransition:
			TemplateStatusMonitorMap::mapped_type::RegisterExpression(
				OutStatusMonitors,
				InRegisterKey,
				LocalExpression,
				LocalChunk->StatusTransitions);
			return -1;

			case EExpressionKind::StatusComparison:
			TemplateStatusMonitorMap::mapped_type::RegisterExpression(
				OutStatusMonitors,
				InRegisterKey,
				LocalExpression,
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
		/// [in] InExpression が参照する要素条件コンテナ。
		typename TemplateEvaluator::FChunk::FSubExpressionArray const&
			InSubExpressions,
		/// [in] 条件式を持つ TEvaluator インスタンス。
		TemplateEvaluator const& InEvaluator)
	{
		// InExpression の要素条件を走査し、
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
					InSubExpressions.at(i).GetKey(),
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

	/// @brief 条件挙動ハンドラを検索しつつ、コンテナを整理する。
	/// @retval true  OutHandlers から InFunction が見つかった。
	/// @retval false OutHandlers から InFunction が見つからなかった。
	private: static bool trim_handlers(
		/// [in,out] 走査する条件挙動ハンドラのコンテナ。
		typename ThisClass::FHandlerArray& OutHandlers,
		/// [in] 検索する挙動挙動ハンドラに対応する条件挙動関数。
		typename ThisClass::FHandler::FFunction const* const InFunction,
		/// [in] 検索する条件挙動ハンドラを削除するかどうか。
		bool const InErase)
	{
		auto LocalFind(InFunction == nullptr);
		for (auto i(OutHandlers.begin()); i != OutHandlers.end();)
		{
			auto& LocalObserber(i->GetFunction());
			bool LocalErase;
			if (LocalFind)
			{
				LocalErase = LocalObserber.expired();
			}
			else
			{
				auto const LocalPointer(LocalObserber.lock().get());
				LocalFind = LocalPointer == InFunction;
				LocalErase = LocalFind? InErase: LocalPointer == nullptr;
			}
			if (LocalErase)
			{
				i = OutHandlers.erase(i);
			}
			else
			{
				++i;
			}
		}
		return LocalFind && InFunction != nullptr;
	}

	/// @copydoc ThisClass::CacheHandlers
	private: template<
		typename TemplateHandlerCacheArray, typename TemplateEvaluator>
	void CacheHandlers(
		/// [in,out] FHandler::FCache を貯めるコンテナ。
		TemplateHandlerCacheArray& OutCachedHandlers,
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
				InReservoir,
				InEvaluator,
				InExpressionKey,
				LocalFlushCondition));
		auto const LocalTransition(
			ThisClass::FHandler::MakeCondition(
				LocalNowEvaluation, LocalLastEvaluation));
		if (LocalTransition != ThisClass::FHandler::EUnitCondition::Invalid)
		{
			// 条件式の評価の変化が挙動条件と合致すれば、
			// 条件挙動ハンドラをキャッシュに貯める。
			for (auto i(this->Handlers.begin()); i != this->Handlers.end();)
			{
				auto const& LocalHandler(*i);
				if (LocalHandler.GetFunction().expired())
				{
					i = this->Handlers.erase(i);
				}
				else
				{
					++i;
					if (LocalHandler.IsMatched(LocalTransition))
					{
						OutCachedHandlers.emplace_back(
							LocalHandler,
							InExpressionKey,
							LocalNowEvaluation,
							LocalLastEvaluation);
					}
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
			return EPsyqueKleene::TernaryUnknown;
		}

		// 条件式を評価し、結果を記録する。
		auto const LocalEvaluateExpression(
			InEvaluator.EvaluateExpression(InExpressionKey, InReservoir));
		this->Flags.set(
			ThisClass::EFlag::LastEvaluation,
			LocalEvaluateExpression != EPsyqueKleene::TernaryUnknown);
		this->Flags.set(
			ThisClass::EFlag::LastCondition,
			LocalEvaluateExpression == EPsyqueKleene::TernaryTrue);
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
		auto const LocalExistence(InEvaluator.IsRegistered(InExpressionKey));
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
			EPsyqueKleene::TernaryUnknown;
	}

	/// @brief Psyque::RulesEngine 管理者以外は使用禁止。
	/// @details 登録されている条件挙動ハンドラを取得する。
	/// @return
	///   InFunction に対応する ThisClass::FHandler を指すポインタ。該当する
	///   ThisClass::FHandler がない場合は nullptr を返す。
	public: typename ThisClass::FHandler const* _find_handler_ptr(
		/// [in] 取得する ThisClass::FHandler に対応する条件挙動関数。
		typename ThisClass::FHandler::FFunction const& InFunction)
	const PSYQUE_NOEXCEPT
	{
		for (auto& LocalHandler: this->Handlers)
		{
			if (!LocalHandler.expired()
				&& LocalHandler.lock().get() == &InFunction)
			{
				return &LocalHandler;
			}
		}
		return nullptr;
	}

	//-------------------------------------------------------------------------
	/// @copydoc ThisClass::FHandlerArray
	private: typename ThisClass::FHandlerArray Handlers;
	/// @brief 条件式の評価結果を記録するフラグの集合。
	private: std::bitset<8> Flags;

}; // class Psyque::RulesEngine::_private::TExpressionMonitor

// vim: set noexpandtab:
