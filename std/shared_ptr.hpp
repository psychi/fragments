/** @file
    @brief @copybrief psyq::std_shared_ptr

    - std::shared_ptr がある開発環境では std::shared_ptr を、
      psyq::std_shared_ptr でラップする。
    - std::shared_ptr がない開発環境では boost::shared_ptr を、
      psyq::std_shared_ptr でラップする。

    @author Hillco Psychi (https://twitter.com/psychi)
*/
#ifndef PSYQ_STD_SHARED_PTR_HPP_
#define PSYQ_STD_SHARED_PTR_HPP_

#include "./unique_ptr.hpp"

#ifdef PSYQ_NO_STD_SMART_PTR
#   define PSYQ_STD_SHARED_PTR_BASE boost::shared_ptr
#   include <boost/shared_ptr.hpp>
#else
#   define PSYQ_STD_SHARED_PTR_BASE std::shared_ptr
#endif // defined(PSYQ_NO_STD_SMART_PTR)

#ifndef PSYQ_STD_SHARED_PTR_DEFAULT_ALLOCATOR
#   define PSYQ_STD_SHARED_PTR_DEFAULT_ALLOCATOR std::allocator<void*>
#endif // !defined(PSYQ_STD_SHARED_PTR_DEFAULT_ALLOCATOR)

/// @cond
namespace psyq
{
    template<typename> class std_shared_ptr;
    template<typename> class std_weak_ptr;
}
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::shared_ptr 互換のスマートポインタ。
 */
template<typename template_element> class psyq::std_shared_ptr:
public PSYQ_STD_SHARED_PTR_BASE<template_element>
{
    /// thisが指す値の型。
    private: typedef std_shared_ptr this_type;

    /// this_type の基底型。
    private: typedef PSYQ_STD_SHARED_PTR_BASE<template_element> base_type;

    private: typedef PSYQ_STD_SHARED_PTR_DEFAULT_ALLOCATOR default_allocator;

    //-------------------------------------------------------------------------
    /// @name コンストラクタ
    //@{
    /** @brief 空のスマートポインタを構築する。
     */
    public: PSYQ_CONSTEXPR std_shared_ptr() PSYQ_NOEXCEPT {}

#ifndef PSYQ_NO_STD_NULLPTR
    /// @copydoc std_shared_ptr()
    public: std_shared_ptr(psyq::std_nullptr_t) PSYQ_NOEXCEPT:
    base_type(
        PSYQ_NULLPTR,
        psyq::std_default_delete<template_element>(),
        typename this_type::default_allocator())
    {}

    /** @copydoc std_shared_ptr()
        @param[in] in_deleter 不要なオブジェクトを破棄する関数オブジェクト。
     */
    public: template<typename template_deleter>
    std_shared_ptr(psyq::std_nullptr_t, template_deleter in_deleter):
    base_type(
        PSYQ_NULLPTR,
        PSYQ_MOVE(in_deleter),
        typename this_type::default_allocator())
    {}

    /** @copydoc std_shared_ptr(psyq::std_nullptr_t, template_deleter)
        @param[in] in_allocator 使用するメモリ割当子。
     */
    public: template<typename template_deleter, typename template_allocator>
    std_shared_ptr(
        psyq::std_nullptr_t,
        template_deleter in_deleter,
        template_allocator in_allocator)
    :
    base_type(PSYQ_NULLPTR, PSYQ_MOVE(in_deleter), PSYQ_MOVE(in_allocator))
    {}
#endif // !defined(PSYQ_NO_STD_NULLPTR)

    /** @brief オブジェクトを所有するスマートポインタを構築する。
        @param[in] in_hold_element 所有するオブジェクトを指すポインタ。
     */
    public: template<typename template_hold_element>
    explicit std_shared_ptr(template_hold_element* const in_hold_element):
    base_type(
        in_hold_element,
        psyq::std_default_delete<template_element>(),
        typename this_type::default_allocator())
    {}

    /** @brief @copydoc std_shared_ptr(template_hold_element*)
        @param[in] in_deleter 不要なオブジェクトを破棄する関数オブジェクト。
     */
    public: template<typename template_hold_element, typename template_deleter>
    std_shared_ptr(
        template_hold_element* const in_hold_element,
        template_deleter in_deleter)
    :
    base_type(
        in_hold_element,
        PSYQ_MOVE(in_deleter),
        typename this_type::default_allocator())
    {}

    /** @brief @copydoc std_shared_ptr(template_hold_element*, template_deleter)
        @param[in] in_allocator 使用するメモリ割当子。
     */
    public: template<
        typename template_hold_element,
        typename template_deleter,
        typename template_allocator>
    std_shared_ptr(
        template_hold_element* const in_hold_element,
        template_deleter in_deleter,
        template_allocator in_allocator)
    :
    base_type(in_hold_element, PSYQ_MOVE(in_deleter), PSYQ_MOVE(in_allocator))
    {}

    /** @brief スマートポインタをコピー構築する。
        @param[in] in_source コピー元となるスマートポインタ。
     */
    public: std_shared_ptr(this_type const& in_source) PSYQ_NOEXCEPT:
    base_type(in_source)
    {}

    /// @copydoc std_shared_ptr(this_type const&)
    public: template<typename template_hold_element>
    std_shared_ptr(
        psyq::std_shared_ptr<template_hold_element> const& in_source)
    PSYQ_NOEXCEPT:
    base_type(in_source)
    {}

    /// @copydoc std_shared_ptr(this_type const&)
    public: template<typename template_hold_element>
    explicit std_shared_ptr(
        psyq::std_weak_ptr<template_hold_element> const& in_source)
    PSYQ_NOEXCEPT:
    base_type(in_source)
    {}

    /** @copydoc std_shared_ptr(this_type const&)
        @param[in] in_member in_source のメンバ。
     */
    public: template<typename template_hold_element>
    std_shared_ptr(
        psyq::std_shared_ptr<template_hold_element> const& in_source,
        template_element* const in_member)
    PSYQ_NOEXCEPT:
    base_type(in_source, in_member)
    {}

    /** @brief スマートポインタをムーブ構築する。
        @param[in,out] io_source ムーブ元となるスマートポインタ。
     */
    public: std_shared_ptr(PSYQ_RV_REF(this_type) io_source) PSYQ_NOEXCEPT:
    base_type(PSYQ_MOVE(io_source))
    {}

    /** @brief スマートポインタをムーブ構築する。
        @param[in,out] io_source ムーブ元となるスマートポインタ。
     */
    public: std_shared_ptr(PSYQ_RV_REF(base_type) io_source) PSYQ_NOEXCEPT:
    base_type(PSYQ_MOVE(io_source))
    {}

    /** @brief スマートポインタをムーブ構築する。
        @param[in,out] io_source ムーブ元となるスマートポインタ。
     */
    public: template<typename template_hold_element>
    std_shared_ptr(
        PSYQ_RV_REF(std_shared_ptr<template_hold_element>) io_source)
    PSYQ_NOEXCEPT:
    base_type(PSYQ_MOVE(io_source))
    {}

    /** @brief スマートポインタをムーブ構築する。
        @param[in,out] io_source ムーブ元となるスマートポインタ。
     */
    public: template<
        typename template_hold_element,
        typename template_deleter>
    std_shared_ptr(
        PSYQ_RV_REF_2_TEMPL_ARGS(
            psyq::std_unique_ptr, template_hold_element, template_deleter)
                io_source)
    :
#ifdef PSYQ_NO_STD_SMART_PTR
    base_type(
        io_source.release(),
        io_source.get_deleter(),
        typename this_type::default_allocator())
#else
    base_type(PSYQ_MOVE(io_source))
#endif // defined(PSYQ_NO_STD_SMART_PTR)
    {}
    //@}
    //-------------------------------------------------------------------------
    /// @name 代入演算子
    //@{
    /** @brief スマートポインタをコピー代入する。
        @param[in] in_source コピー元となるスマートポインタ。
        @return *this
     */
    public: this_type& operator=(this_type const& in_source) PSYQ_NOEXCEPT
    {
        this->base_type::operator=(in_source);
        return *this;
    }

    /// @copydoc operator=(this_type const&)
    public: template<typename template_hold_element>
    this_type& operator=(
        psyq::std_shared_ptr<template_hold_element> const& in_source)
    PSYQ_NOEXCEPT
    {
        this->base_type::operator=(in_source);
        return *this;
    }

    /** @brief スマートポインタをムーブ代入する。
        @param[in,out] io_source ムーブ元となるスマートポインタ。
        @return *this
     */
    public: this_type& operator=(PSYQ_RV_REF(this_type) io_source)
    PSYQ_NOEXCEPT
    {
        this->base_type::operator=(PSYQ_MOVE(io_source));
        return *this;
    }

    /** @brief スマートポインタをムーブ代入する。
        @param[in,out] io_source ムーブ元となるスマートポインタ。
        @return *this
     */
    public: template<typename template_hold_element>
    this_type& operator=(
        PSYQ_RV_REF(psyq::std_shared_ptr<template_hold_element>) io_source)
    PSYQ_NOEXCEPT
    {
        this->base_type::operator=(PSYQ_MOVE(io_source));
        return *this;
    }
    //@}
}; // class psyq::std_shared_ptr

#endif // PSYQ_STD_SHARED_PTR_HPP_
// vim: set expandtab:
