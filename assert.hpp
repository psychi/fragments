/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief constexpr関数でも使用可能なassertion判定。
 */
#ifndef PSYQ_ASSERT_HPP_
#define PSYQ_ASSERT_HPP_

#if defined(__GNUC__)
#   if (4 < __GNUC__ || (__GNUC__ == 4 && 6 <= __GNUC_MINOR__))
#       define PSYQ_NOEXCEPT noexcept
#       define PSYQ_CONSTEXPR constexpr
#   endif
#elif defined(_MSC_VER)
#   define PSYQ_NOEXCEPT throw()
#   define PSYQ_CONSTEXPR
#else
#   define PSYQ_NOEXCEPT noexcept
#   define PSYQ_CONSTEXPR
#endif

#define PSYQ_PP_STRINGIZE(define_text) PSYQ_INTERNAL_PP_STRINGIZE(define_text)
#define PSYQ_INTERNAL_PP_STRINGIZE(define_text) #define_text

#ifndef PSYQ_ASSERTION_FAILED_STRING
#   define PSYQ_ASSERTION_FAILED_STRING(\
        define_expression, define_file, define_line)\
            "psyq assertion failed: " #define_expression\
            ", file " define_file\
            ", line " PSYQ_PP_STRINGIZE(define_line)
#endif // !define_file(PSYQ_ASSERTION_FAILED_STRING)

#if defined(PSYQ_DISABLE_ASSERT) || defined(NDEBUG)
#   define PSYQ_ASSERT(define_expression) ((void)0)
#else
#   include <cstdlib>
#   include <iostream>
#   define PSYQ_ASSERT(define_expression) (\
        (void)psyq::internal::assertion_check(\
            (define_expression),\
            PSYQ_ASSERTION_FAILED_STRING(\
                define_expression, __FILE__, __LINE__)))
namespace psyq
{
    namespace internal
    {
        /** @brief assertionしたときに呼び出される。
            @param[in] in_message assertionしたとき、consoleに出力する文字列。
            @return false
         */
        inline bool assertion_failed(char const* const in_message)
        {
            return
                (std::cerr << in_message << std::endl), std::abort(), false;
        }

        /** @brief assertionしないか判定する。
            @param[in] in_condition assertionしないかどうか。
            @param[in] in_message   assertionしたとき、consoleに出力する文字列。
            @retval true  assertionしなかった。
            @retval false assertionした。
         */
        inline PSYQ_CONSTEXPR bool assertion_check(
            bool const in_condition,
            char const* const in_message)
        {
            return in_condition? true: assertion_failed(in_message);
        }
    }
}
#endif // defined(PSYQ_DISABLE_ASSERT) || defined(NDEBUG)

#endif // !defined(PSYQ_ASSERT_HPP_)
