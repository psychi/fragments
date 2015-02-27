/** @file
    @brief std::default_delete 互換のインスタンス破棄関数オブジェクト。

    std::default_delete がある開発環境ではそれをラップし、
    std::default_delete がない開発環境では、独自の実装を用意する。

    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_STD_DEFAULT_DELETE_HPP_
#define PSYQ_STD_DEFAULT_DELETE_HPP_

#ifndef PSYQ_STD_DEFAULT_DELETE_OBJECT
#define PSYQ_STD_DEFAULT_DELETE_OBJECT(define_object) delete define_object
#endif // !defined(PSYQ_STD_DEFAULT_DELETE_OBJECT)

#ifndef PSYQ_STD_DEFAULT_DELETE_ARRAY
#define PSYQ_STD_DEFAULT_DELETE_ARRAY(define_array) delete[] define_array
#endif // !defined(PSYQ_STD_DEFAULT_DELETE_ARRAY)

/// @cond
namespace psyq
{
    template<typename> struct std_default_delete;
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
#ifdef PSYQ_STD_NO_DEFAULT_DELETE
/** @brief std::default_delete 互換のインスタンス破棄関数オブジェクト。

    std::default_delete がない開発環境では、独自の実装を用意する。
 */
template<typename template_element> struct psyq::std_default_delete
{
    /** @brief インスタンスを破棄する。
        @param[in] in_delete_element 破棄するインスタンスのポインタ。
     */
    void operator()(template_element* const in_delete_element)
    const PSYQ_NOEXCEPT
    {
        PSYQ_STD_DEFAULT_DELETE_OBJECT(in_delete_element);
    }
};

/** @brief std::default_delete 互換の配列破棄関数オブジェクト。

    std::default_delete がない開発環境では、独自の実装を用意する。
 */
template<typename template_element>
struct psyq::std_default_delete<template_element[]>
{
    /** @brief 配列を破棄する。
        @param[in] in_delete_array 破棄する配列。
     */
    void operator()(template_element* const in_delete_array)
    const PSYQ_NOEXCEPT
    {
        PSYQ_STD_DEFAULT_DELETE_ARRAY(in_delete_array);
    }

    /// 他の型の配列は破棄できない。
    private: template <typename template_other_element>
    void operator()(template_other_element* const) const;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
#else
#include <memory>
/** @brief std::default_delete 互換のインスタンス破棄関数オブジェクト。

    std::default_deleteがある開発環境では、それをラップする。
 */
template<typename template_element>
struct psyq::std_default_delete: public std::default_delete<template_element>
{};
#endif // defined(PSYQ_STD_NO_DEFAULT_DELETE)

#endif // !defined(PSYQ_STD_DEFAULT_DELETE_HPP_)
