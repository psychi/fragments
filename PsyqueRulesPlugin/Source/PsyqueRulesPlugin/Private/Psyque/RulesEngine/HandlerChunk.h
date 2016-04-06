// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief @copybrief Psyque::RulesEngine::_private::THandlerChunk
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

/// @cond
namespace Psyque
{
	namespace RulesEngine
	{
		namespace _private
		{
			template<typename> class THandlerChunk;
		} // namespace _private
	} // namespace RulesEngine
} // namespace Psyque
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 条件挙動チャンク。条件式の評価が変化した際に呼び出す関数を保持する。
/// @tparam THandlerChunk::FDispather
template<typename TemplateDispatcher>
class Psyque::RulesEngine::_private::THandlerChunk
{
	private: using ThisClass = THandlerChunk; ///< @copydoc TDispatcher::ThisClass

	//-------------------------------------------------------------------------
	/// @brief _private::FDispather を表す型。
	public: using FDispather = TemplateDispatcher;
	/// @brief 条件挙動チャンクのコンテナを表す型。
	public: using FArray = std::vector<
		ThisClass, typename ThisClass::FDispather::FAllocator>;
	/// @brief 条件挙動チャンクの識別値を表す型。
	public: using FKey =
		typename ThisClass::FDispather::FEvaluator::FReservoir::FChunkKey;

	//-------------------------------------------------------------------------
	/// @copydoc Functions
	private: using FunctionSharedPtrArray = std::vector<
		typename TemplateDispatcher::FHandler::FFunctionSharedPtr,
		typename TemplateDispatcher::FAllocator>;
	/// @brief 条件挙動チャンクの識別値を比較する関数オブジェクト。
	private: struct FKeyLess
	{
		bool operator()(
			THandlerChunk const& InLeft,
			THandlerChunk const& InRight)
		const PSYQUE_NOEXCEPT
		{
			return InLeft.Key < InRight.Key;
		}

		bool operator()(
			THandlerChunk const& InLeft,
			typename THandlerChunk::FKey const InRight)
		const PSYQUE_NOEXCEPT
		{
			return InLeft.Key < InRight;
		}

		bool operator()(
			typename THandlerChunk::FKey const InLeft,
			THandlerChunk const& InRight)
		const PSYQUE_NOEXCEPT
		{
			return InLeft < InRight.Key;
		}

	}; // struct priority_less

	//-------------------------------------------------------------------------
#ifdef PSYQUE_NO_STD_DEFAULTED_FUNCTION
	/// @brief ムーブ構築子。
	public: THandlerChunk(
		/// [in,out] ムーブ元となるインスタンス。
		ThisClass&& OutSource):
	Functions(MoveTemp(OutSource.Functions)),
	Key(MoveTemp(OutSource.Key))
	{}

	/** @brief ムーブ代入演算子。
		@return *this
	 */
	public: ThisClass& operator=(
		/// [in,out] ムーブ元となるインスタンス。
		ThisClass&& OutSource)
	{
		this->Functions = MoveTemp(OutSource.Functions);
		this->Key = MoveTemp(OutSource.Key);
		return *this;
	}
#endif // defined(PSYQUE_NO_STD_DEFAULTED_FUNCTION)

	/// @brief 条件挙動関数のコンテナを整理する。
	public: void shrink_to_fit()
	{
		this->Functions.shrink_to_fit();
	}

	//-------------------------------------------------------------------------
	/// @brief 条件挙動チャンクに THandler::FFunction を追加する。
	/// @retval true  成功。 THandler::FFunction を追加した。
	/// @retval false 失敗。 THandler::FFunction を追加しなかった。
	public: static bool Extend(
		/// [in,out] THandler::FFunction を追加する条件挙動チャンクのコンテナ。
		typename ThisClass::FArray& OutChunks,
		/// [in] THandler::FFunction を追加する条件挙動チャンクの識別値。
		typename ThisClass::FKey const InKey,
		/// [in] 条件挙動チャンクに追加する THandler::FFunctionSharedPtr 。
		typename ThisClass::FDispather::FHandler::FFunctionSharedPtr InFunction)
	{
		if (InFunction.get() == nullptr)
		{
			return false;
		}

		// 条件挙動関数を条件挙動チャンクに追加する。
		ThisClass::Equip(OutChunks, InKey).Functions.push_back(
			MoveTemp(InFunction));
		return true;
	}

	/// @brief 条件挙動チャンクに THandler::FFunction を追加する。
	/// @return 追加した THandler::FFunction の数。
	public: template<typename TemplateFunctionContainer>
	static std::size_t Extend(
		/// [in,out] FHandler::FFunction を追加する条件挙動チャンクのコンテナ。
		typename ThisClass::FArray& OutChunks,
		/// [in] FHandler::FFunction を追加する条件挙動チャンクの識別値。
		typename ThisClass::FKey const InKey,
		/// [in] 条件挙動チャンクに追加する
		/// FHandler::FFunctionSharedPtr のコンテナ。
		TemplateFunctionContainer InFunctions)
	{
		// 条件挙動関数を条件挙動チャンクに追加する。
		auto& local_chunk_functions(
			ThisClass::Equip(OutChunks, InKey).Functions);
		local_chunk_functions.reserve(
			local_chunk_functions.size() + InFunctions.size());
		std::size_t local_count(0);
		for (auto& LocalFunction: InFunctions)
		{
			if (LocalFunction.get() != nullptr)
			{
				local_chunk_functions.push_back(MoveTemp(LocalFunction));
				++local_count;
			}
		}
		return local_count;
	}

	/// @brief コンテナから条件挙動チャンクを削除する。
	/// @retval true  InKey に対応する条件挙動チャンクを削除した。
	/// @retval false InKey に対応する条件挙動チャンクがコンテナになかった。
	public: static bool erase(
		/// [in,out] 条件挙動チャンクを削除するコンテナ。
		typename ThisClass::FArray& OutChunks,
		/// [in] 削除する条件挙動チャンクの識別値。
		typename ThisClass::FKey const InKey)
	{
		auto const LocalLowerBound(
			std::lower_bound(
				OutChunks.begin(),
				OutChunks.end(),
				InKey,
				typename ThisClass::FKeyLess()));
		if (LocalLowerBound == OutChunks.end()
			|| LocalLowerBound->Key != InKey)
		{
			return false;
		}
		OutChunks.erase(LocalLowerBound);
		return true;
	}

	//-------------------------------------------------------------------------
	/// @brief 空の条件挙動チャンクを構築する。
	private: THandlerChunk(
		/// [in] 条件挙動チャンクの識別値。
		typename ThisClass::FKey const InKey,
		/// [in] メモリ割当子の初期値。
		typename ThisClass::FDispather::FAllocator const& InAllocator):
	Functions(InAllocator),
	Key(InKey)
	{}

	/// @brief 条件挙動チャンクを用意する。
	/// @return 用意した条件挙動チャンク。
	private: static ThisClass& Equip(
		/// [in,out] 条件挙動チャンクのコンテナ。
		typename ThisClass::FArray& OutChunks,
		/// [in] 用意する条件挙動チャンクの識別値。
		typename ThisClass::FKey const InKey)
	{
		// 条件挙動関数を追加する条件挙動チャンクを用意する。
		auto const LocalLowerBound(
			std::lower_bound(
				OutChunks.begin(),
				OutChunks.end(),
				InKey,
				typename ThisClass::FKeyLess()));
		return LocalLowerBound != OutChunks.end()
		&& LocalLowerBound->Key == InKey?
			*LocalLowerBound:
			*OutChunks.insert(
				LocalLowerBound,
				ThisClass(InKey, OutChunks.get_allocator()));
	}

	//-------------------------------------------------------------------------
	/// @brief FHandler::FFunctionSharedPtr のコンテナ。
	private: typename ThisClass::FunctionSharedPtrArray Functions;
	/// @brief 条件挙動チャンクの識別値。
	private: typename ThisClass::FKey Key;

}; // class Psyque::RulesEngine::THandlerChunk

// vim: set noexpandtab:
