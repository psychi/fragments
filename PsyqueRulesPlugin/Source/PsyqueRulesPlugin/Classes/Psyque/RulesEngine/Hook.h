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
		class FDelegateIdentifier;
		namespace _private
		{
			template<typename, typename> class THook;
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
	/// @brief シングルキャストデリゲートから、デリゲート識別子を構築する。
	public:
	template <typename... TemplateParameters>
	explicit FDelegateIdentifier(
		/// [in] 識別子のもととなるシングルキャストデリゲート。
		::TBaseDelegate<TemplateParameters...> const& InDelegate)
	{
		auto const LocalDelegate(InDelegate.GetDelegateInstanceProtected());
		if (!InDelegate.IsBound())
		{
			this->Name = NAME_None;
		}
		else if (LocalDelegate->GetType() == EDelegateInstanceType::Functor)
		{
			this->Name = ThisClass::GetFunctorName();
		}
		else
		{
			this->Object = LocalDelegate->GetUObject();
			this->Method = LocalDelegate->GetRawMethodPtr();
			this->Name = LocalDelegate->GetFunctionName();
			check(!this->IsEmpty());
			return;
		}
		this->Object = nullptr;
		this->Method = nullptr;
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
	UObject const* Object;
	/// @brief デリゲートから参照するメソッドを指すポインタ。
	void const* Method;
	/// @brief デリゲートから参照するメソッドの名前。
	FName Name;

}; // class Psyque::RulesEngine::FDelegateIdentifier

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
	/// @brief 条件式の古い評価から新しい評価への遷移状態を表す型。
	/// @sa ThisClass::MakeTransition で値を作る。
	public: using FTransition = uint8;

	//---------------------------------------------------------------------
	private: enum: uint8
	{
		/// 遷移状態を表すのに必要なビット数。
		TransitionBitWidth = 2,
		/// 無効な遷移状態。
		InvalidTransition = 255,
	};

	//---------------------------------------------------------------------
	/// @brief 条件挙動フックを構築する。
	public: THook(
		/// [in] THook::Condition の初期値。
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
	/// @name 実行するデリゲート
	/// @{

	/// @brief ThisClass::Delegate を取得する。
	/// @return ThisClass::Delegate
	public: ::FPsyqueRulesDelegate const& GetDelegate() const PSYQUE_NOEXCEPT
	{
		return this->Delegate;
	}

	/// @brief ThisClass::Delegate と等値か判定する。
	public: bool IsEqualDelegate(
		/// [in] 比較するデリゲート。
		::FPsyqueRulesDelegate const& InDelegate)
	const
	{
		return InDelegate == this->GetDelegate();
	}

	/// @brief ThisClass::Delegate と等値か判定する。
	public: bool IsEqualDelegate(
		/// [in] 比較するデリゲートを指すハンドル。
		::FDelegateHandle const& InDelegate)
	const
	{
		return InDelegate == this->GetDelegate().GetHandle();
	}

	/// @brief ThisClass::Delegate と等値か判定する。
	public: bool IsEqualDelegate(
		/// [in] 比較するデリゲートを指す識別子。
		Psyque::RulesEngine::FDelegateIdentifier const& InDelegate)
	const PSYQUE_NOEXCEPT
	{
		return InDelegate
			== Psyque::RulesEngine::FDelegateIdentifier(this->GetDelegate());
	}
	/// @}

	/// @brief 有効なデリゲートか判定する。
	public: static bool IsValidDelegate(
		/// [in] 判定するデリゲート。
		::FPsyqueRulesDelegate const& InDelegate)
	{
		return InDelegate.IsBound();
	}

	/// @brief 有効なデリゲートか判定する。
	public: static bool IsValidDelegate(
		/// [in] 判定するデリゲートを指すハンドル。
		::FDelegateHandle const& InDelegate)
	{
		return InDelegate.IsValid();
	}

	/// @brief 有効なデリゲートか判定する。
	public: static bool IsValidDelegate(
		/// [in] 判定するデリゲートを指す識別子。
		Psyque::RulesEngine::FDelegateIdentifier const& InDelegate)
	PSYQUE_NOEXCEPT
	{
		return !InDelegate.IsEmpty();
	}

	//-------------------------------------------------------------------------
	/// @name デリゲートを実行する条件
	/// @{

	/// @brief ThisClass::Delegate を実行する条件となる遷移状態を取得する。
	/// @return @copydoc THook::Condition
	public: typename ThisClass::FTransition GetTransition() const PSYQUE_NOEXCEPT
	{
		return this->Condition;
	}

	/// @brief ThisClass::Delegate を実行する条件となる、条件式の古い評価を取得する。
	public: ::EPsyqueKleene GetBeforeCondition() const PSYQUE_NOEXCEPT
	{
		return ThisClass::GetUnitCondition(
			this->GetTransition() >> ThisClass::TransitionBitWidth);
	}

	/// @brief ThisClass::Delegate を実行する条件となる、条件式の新しい評価を取得する。
	public: ::EPsyqueKleene GetLatestCondition() const PSYQUE_NOEXCEPT
	{
		return ThisClass::GetUnitCondition(this->GetTransition());
	}
	/// @}

	/// @brief 条件式の評価の遷移を表す値を構築する。
	/// @return 条件式の評価の遷移状態。
	public: static typename ThisClass::FTransition MakeTransition(
		/// [in] 条件式の古い評価。
		::EPsyqueKleene const InBeforeCondition,
		/// [in] 条件式の新しい評価。
		::EPsyqueKleene const InLatestCondition)
	PSYQUE_NOEXCEPT
	{
		return InBeforeCondition != InLatestCondition?
			ThisClass::GetUnitTransition(InLatestCondition) | (
				ThisClass::GetUnitTransition(InBeforeCondition)
					<< ThisClass::TransitionBitWidth):
			ThisClass::InvalidTransition;
	}

	/// @brief 有効な遷移状態か判定する。
	/// @return 有効な遷移状態か否か。
	public: static bool IsValidTransition(
		/// [in] 判定する遷移状態。
		typename ThisClass::FTransition const InTransition)
	PSYQUE_NOEXCEPT
	{
		return InTransition != ThisClass::InvalidTransition;
	}

	private: static ::EPsyqueKleene GetUnitCondition(
		typename ThisClass::FTransition const InTransition)
	PSYQUE_NOEXCEPT
	{
		auto const LocalEvaluation(
			InTransition & ((1 << ThisClass::TransitionBitWidth) - 1));
		return LocalEvaluation == static_cast<uint8>(::EPsyqueKleene::IsFalse)?
			::EPsyqueKleene::IsFalse:
			LocalEvaluation == static_cast<uint8>(::EPsyqueKleene::IsTrue)?
				::EPsyqueKleene::IsTrue: ::EPsyqueKleene::Unknown;
	}

	private: static typename ThisClass::FTransition GetUnitTransition(
		::EPsyqueKleene const InCondition)
	PSYQUE_NOEXCEPT
	{
		return static_cast<typename ThisClass::FTransition>(InCondition) & 3;
	}

	//---------------------------------------------------------------------
	/// @brief 条件に合致した際に実行するデリゲート。
	private: ::FPsyqueRulesDelegate Delegate;
	/// @brief デリゲートの実行優先順位。
	public: typename ThisClass::FPriority Priority;
	/// @brief デリゲートを実行する条件となる遷移状態。
	private: typename ThisClass::FTransition Condition;

}; // class Psyque::RulesEngine::_private::THook

// vim: set noexpandtab:
