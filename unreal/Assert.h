// Copyright 2016 Hillco Psychi All Rights Reserved.
/// @file
/// @author Hillco Psychi (https://twitter.com/psychi)
/// @brief constexpr関数でも使用可能なassertion判定。
#pragma once

#include "Misc/AssertionMacros.h"
#include "./Config.h"

#define PSYQUE_PP_STRINGIZE(define_text) PSYQUE_INTERNAL_PP_STRINGIZE(define_text)
#define PSYQUE_INTERNAL_PP_STRINGIZE(define_text) #define_text

#ifndef PSYQUE_ASSERTION_FAILED_STRING
#	define PSYQUE_ASSERTION_FAILED_STRING(\
		define_expression, define_file, define_line)\
			"psyq assertion failed: " #define_expression\
			", file " define_file\
			", line " PSYQUE_PP_STRINGIZE(define_line)
#endif // !define_file(PSYQUE_ASSERTION_FAILED_STRING)

#if defined(PSYQUE_DISABLE_ASSERT) || defined(NDEBUG)
#	define PSYQUE_ASSERT(define_expression) ((void)0)
#else
#	include <cstdlib>
#	include <iostream>
#	define PSYQUE_ASSERT(define_expression) (\
		(void)psyq::_private::AssertionCheck(\
			(define_expression),\
			PSYQUE_ASSERTION_FAILED_STRING(\
				define_expression, __FILE__, __LINE__)))
namespace psyq
{
	namespace _private
	{
		/// @brief assertionしたときに呼び出される。
		/// @return false
		inline bool AssertionFailed(
			/// @param[in] assertionしたとき、consoleに出力する文字列。
			char const* const in_message)
		{
			return
				(std::cerr << in_message << std::endl), std::abort(), false;
			if(!(expr))
			{
				FDebug::LogAssertFailedMessage(in_message, __FILE__, __LINE__);
				_DebugBreakAndPromptForRemote();
				FDebug::AssertFailed(in_message, __FILE__, __LINE__);
				CA_ASSUME(expr);
			}
		}

		/// @brief assertionしないか判定する。
		/// @retval true  assertionしなかった。
		/// @retval false assertionした。
		inline PSYQUE_CONSTEXPR bool AssertionCheck(
			/// @param[in] assertionしないかどうか。
			bool const in_condition,
			/// @param[in] assertionしたとき、consoleに出力する文字列。
			char const* const in_message)
		{
			return in_condition? true: AssertionFailed(in_message);
		}
	}
}
#endif // defined(PSYQUE_DISABLE_ASSERT) || defined(NDEBUG)

#ifdef PSYQUE_ENABLE_EXCEPTION
#	define PSYQUE_ASSERT_THROW(define_expression, define_exception)\
		if (!(define_expression))\
			throw define_exception(\
				PSYQUE_ASSERTION_FAILED_STRING(\
					define_expression, __FILE__, __LINE__))
#else
#	define PSYQUE_ASSERT_THROW(define_expression, define_exception)\
		PSYQUE_ASSERT(define_expression)
#endif // define(PSYQUE_ENABLE_EXCEPTION)
// vim: set noexpandtab:
