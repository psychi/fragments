/** @file
    @brief 標準C++ライブラリの設定。

    @author Hillco Psychi (https://twitter.com/psychi)
*/
#ifndef PSYQ_STD_CONFIG_HPP_
#define PSYQ_STD_CONFIG_HPP_

//-----------------------------------------------------------------------------
/// @brief 自動定義される特殊メンバ関数に default / delete 宣言できないかどうか。
#if defined(_MSC_VER) && _MSC_VER < 1900
#   define PSYQ_NO_STD_DEFAULTED_FUNCTION
#endif // defined(_MSC_VER) && _MSC_VER < 1900

//-----------------------------------------------------------------------------
//#define PSYQ_NO_STD_MOVE
//#define PSYQ_NO_STD_NULLPTR
//#define PSYQ_NO_STD_ARRAY
//#define PSYQ_NO_STD_DEFAULT_DELETE
//#define PSYQ_NO_STD_SMART_PTR

//-----------------------------------------------------------------------------
#if defined(_MSC_VER) && _MSC_VER < 1800
#   define PSYQ_NO_STD_INITIALIZER_LIST
#endif // defined(_MSC_VER) && _MSC_VER < 1800

//-----------------------------------------------------------------------------
#if defined(__clang__)
#   define PSYQ_NOEXCEPT noexcept
#   define PSYQ_CONSTEXPR constexpr
#   define PSYQ_EXPLICIT_CAST explicit
#elif defined(__GNUC__)
#   if 4 < __GNUC__ || (__GNUC__ == 4 && 6 <= __GNUC_MINOR__)
#       define PSYQ_NOEXCEPT noexcept
#       define PSYQ_CONSTEXPR constexpr
#   endif
#   if 4 < __GNUC__ || (__GNUC__ == 4 && 5 <= __GNUC_MINOR__)
#       define PSYQ_EXPLICIT_CAST explicit
#   endif
#elif defined(_MSC_VER)
#   if 1800 <= _MSC_VER
#       define PSYQ_EXPLICIT_CAST explicit
#   endif
#   if 1900 <= _MSC_VER
#       define PSYQ_NOEXCEPT noexcept
#   else
#       define PSYQ_NOEXCEPT throw()
#   endif
#else
#   define PSYQ_NOEXCEPT noexcept
#endif

#if !defined(PSYQ_EXPLICIT_CAST)
#    define PSYQ_EXPLICIT_CAST
#endif
#if !defined(PSYQ_CONSTEXPR)
#    define PSYQ_CONSTEXPR
#endif

//-----------------------------------------------------------------------------
#ifndef PSYQ_ALIGNOF
#   if defined(_MSC_VER)
#       define PSYQ_ALIGNOF __alignof
#   else
#       define PSYQ_ALIGNOF alignof
#   endif
#endif // !defined(PSYQ_ALIGNOF)

//-----------------------------------------------------------------------------
// Convention is to define __BYTE_ORDER == to one of these values
#if !defined(__BIG_ENDIAN)
#   define __BIG_ENDIAN 4321
#endif
#if !defined(__LITTLE_ENDIAN)
#   define __LITTLE_ENDIAN 1234
#endif

// I386
#if defined(_M_X64) || defined(_M_AMD64) || defined(_M_IX86) || defined(__i386__) || defined(__i386) || defined(i386)
#   define __BYTE_ORDER __LITTLE_ENDIAN
#   define PSYQ_PLATFORM_X86
#endif

// gcc 'may' define __LITTLE_ENDIAN__ or __BIG_ENDIAN__ to 1 (Note the trailing __),
// or even _LITTLE_ENDIAN or _BIG_ENDIAN (Note the single _ prefix)
#if !defined(__BYTE_ORDER)
#   if defined(__LITTLE_ENDIAN__) && __LITTLE_ENDIAN__==1 || defined(_LITTLE_ENDIAN) && _LITTLE_ENDIAN==1
#       define __BYTE_ORDER __LITTLE_ENDIAN
#   elif defined(__BIG_ENDIAN__) && __BIG_ENDIAN__==1 || defined(_BIG_ENDIAN) && _BIG_ENDIAN==1
#       define __BYTE_ORDER __BIG_ENDIAN
#   endif
#endif

// gcc (usually) defines xEL/EB macros for ARM and MIPS endianess
#if !defined(__BYTE_ORDER)
#   if defined(__ARMEL__) || defined(__MIPSEL__)
#       define __BYTE_ORDER __LITTLE_ENDIAN
#   endif
#   if defined(__ARMEB__) || defined(__MIPSEB__)
#       define __BYTE_ORDER __BIG_ENDIAN
#   endif
#endif

#endif // PSYQ_STD_CONFIG_HPP_
// vim: set expandtab:
