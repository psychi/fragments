/** @file
    @brief 標準C++ライブラリの設定。

    @author Hillco Psychi (https://twitter.com/psychi)
*/
#ifndef PSYQ_STD_CONFIG_HPP_
#define PSYQ_STD_CONFIG_HPP_

//-----------------------------------------------------------------------------
//#define PSYQ_NO_STD_MOVE
//#define PSYQ_NO_STD_NULLPTR
//#define PSYQ_NO_STD_ARRAY
//#define PSYQ_NO_STD_DEFAULT_DELETE
//#define PSYQ_NO_STD_UNIQUE_PTR
//#define PSYQ_NO_STD_SHARED_PTR

//-----------------------------------------------------------------------------
#if defined(_MSC_VER) && _MSC_VER <= 1700
#   define PSYQ_NO_STD_DEFAULTED_FUNCTION
#endif // defined(_MSC_VER) && _MSC_VER <= 1700

//-----------------------------------------------------------------------------
#if defined(__clang__)
#   define PSYQ_NOEXCEPT noexcept
#   define PSYQ_CONSTEXPR constexpr
#elif defined(__GNUC__)
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

#endif // PSYQ_STD_CONFIG_HPP_
// vim: set expandtab:
