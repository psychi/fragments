// Copyright (c) 2016, Hillco Psychi, All rights reserved.
/// @file
/// @brief constexpr関数でも使用できるassertion。
/// @author Hillco Psychi (https://twitter.com/psychi)
#pragma once

#include "./Config.h"

#if defined(PSYQUE_DISABLE_ASSERT) || !(DO_CHECK)
#	define PSYQUE_ASSERT(DefineExpression) ((void)0)
#else
#	include "Misc/AssertionMacros.h"
#	define PSYQUE_ASSERT(DefineExpression) (\
		(void)Psyque::_private::CheckAssertion(\
			(DefineExpression), #DefineExpression, __FILE__, __LINE__))
namespace Psyque
{
	namespace _private
	{
		/// @brief assertionに失敗したときに呼び出される。
		/// @return false
		inline bool FailAssertion(
			/// [in] 判定式として出力する文字列。
			ANSICHAR const* const InExpression,
			/// [in] 判定式の記述されていたファイルの名前。
			ANSICHAR const* const InFilename,
			/// [in] 判定式の記述されていたファイルの行番号。
			uint32 const InLine)
		{
			FDebug::LogAssertFailedMessage(InExpression, InFilename, InLine);
			_DebugBreakAndPromptForRemote();
			FDebug::AssertFailed(InExpression, InFilename, InLine);
			CA_ASSUME(InExpression);
			return false;
		}

		/// @brief assertionに成功したか判定する。
		/// @retval true  assertionに成功した。
		/// @retval false assertionに失敗した。
		inline PSYQUE_CONSTEXPR bool CheckAssertion(
			/// [in] assertionに成功したかどうか。
			bool const InCondition,
			/// [in] 判定式として出力する文字列。
			ANSICHAR const* const InExpression,
			/// [in] 判定式の記述されていたファイルの名前。
			ANSICHAR const* const InFilename,
			/// [in] 判定式の記述されていたファイルの行番号。
			uint32 const InLine)
		{
			return InCondition?
				true: FailAssertion(InExpression, InFilename, InLine);
		}
	}
}
#endif // defined(PSYQUE_DISABLE_ASSERT) || !(DO_CHECK)

// vim: set noexpandtab:
