/** @file
    @brief @copydoc psyq::std_weak_ptr

    - std::shared_ptr がある開発環境では std::weak_ptr を、
      psyq::std_weak_ptr でラップする。
    - std::shared_ptr がない開発環境では boost::weak_ptr を、
      psyq::std_weak_ptr でラップする。

    @author Hillco Psychi (https://twitter.com/psychi)
*/
#ifndef PSYQ_STD_WEAK_PTR_HPP_
#define PSYQ_STD_WEAK_PTR_HPP_

//#include "psyq/std/shared_ptr.hpp"

#ifdef PSYQ_STD_NO_SHARED_PTR
#define PSYQ_STD_WEAK_PTR_BASE boost::weak_ptr
#include <boost/weak_ptr.hpp>
#else
#define PSYQ_STD_WEAK_PTR_BASE std::weak_ptr
#endif // defined(PSYQ_STD_NO_SHARED_PTR)

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::weak_ptr 互換のスマートポインタ。
 */
template<typename template_element> class psyq::std_weak_ptr:
public PSYQ_STD_WEAK_PTR_BASE<template_element>
{
    /// thisが指す値の型。
    private: typedef std_weak_ptr this_type;

    /// this_type の基底型。
    private: typedef PSYQ_STD_WEAK_PTR_BASE<template_element> base_type;

    //-------------------------------------------------------------------------
    /// @name コンストラクタ
    //@{
    /** @brief 空のスマートポインタを構築する。
     */
    public: PSYQ_CONSTEXPR std_weak_ptr() PSYQ_NOEXCEPT: base_type() {}

    /** @brief スマートポインタをコピー構築する。
        @param[in] in_source コピー元となるスマートポインタ。
     */
    public: std_weak_ptr(this_type const& in_source) PSYQ_NOEXCEPT: 
    base_type(in_source)
    {}

    /// @copydoc std_weak_ptr(this_type const&)
    public: template<typename template_observe_element>
    std_weak_ptr(psyq::std_weak_ptr<template_observe_element> const& in_source)
    PSYQ_NOEXCEPT:
    base_type(in_source)
    {}

    /// @copydoc std_weak_ptr(this_type const&)
    public: template<typename template_observe_element>
    std_weak_ptr(psyq::std_shared_ptr<template_observe_element> const& in_source)
    PSYQ_NOEXCEPT:
    base_type(in_source)
    {}

    /** @brief スマートポインタをムーブ構築する。
        @param[in,out] io_source ムーブ元となるスマートポインタ。
     */
    public: std_weak_ptr(PSYQ_RV_REF(this_type) io_source) _NOEXCEPT:
    base_type(PSYQ_MOVE(io_source))
    {}

    /// @copydoc std_weak_ptr(this_type&&)
    public: template<typename template_observe_element>
    std_weak_ptr(
        PSYQ_RV_REF(psyq::std_weak_ptr<template_observe_element>) io_source)
    PSYQ_NOEXCEPT:
    base_type(PSYQ_MOVE(io_source))
    {}
    //@}
    //-------------------------------------------------------------------------
    /// @name 代入演算子
    //@{
    /** @brief スマートポインタをコピー代入する。
        @param[in] in_source コピー元となるスマートポインタ。
     */
    public: this_type& operator=(this_type const& in_source) _NOEXCEPT
    {
        this->base_type::operator=(in_source);
        return *this;
    }

    /// @copydoc operator=(this_type const&)
    public: template<typename template_observe_element>
    this_type& operator=(
        psyq::std_weak_ptr<template_observe_element> const& in_source)
    PSYQ_NOEXCEPT
    {
        this->base_type::operator=(in_source);
        return *this;
    }

    /// @copydoc operator=(this_type const&)
    public: template<typename template_observe_element>
    this_type& operator=(
        psyq::std_shared_ptr<template_observe_element> const& in_source)
    PSYQ_NOEXCEPT
    {
        this->base_type::operator=(in_source);
        return *this;
    }

    /** @brief スマートポインタをムーブ代入する。
        @param[in,out] io_source ムーブ元となるスマートポインタ。
     */
    public: this_type& operator=(PSYQ_RV_REF(this_type) io_source)
    PSYQ_NOEXCEPT
    {
        this->base_type::operator=(PSYQ_MOVE(io_source));
        return *this;
    }

    /// @copydoc operator=(this_type&&)
    public: template<typename template_observe_element>
    this_type& operator=(
        PSYQ_RV_REF(psyq::std_weak_ptr<template_observe_element>) io_source)
    PSYQ_NOEXCEPT
    {
        this->base_type::operator=(PSYQ_MOVE(io_source));
        return *this;
    }
    //@}
    //-------------------------------------------------------------------------
    /** @brief 監視しているオブジェクトのスマートポインタを取得する。
        @return 監視しているオブジェクトのスマートポインタ。
     */
    public: psyq::std_shared_ptr<template_element> lock() const
    {
        return psyq::std_shared_ptr<template_element>(*this);
    }

}; // class psyq::std_weak_ptr

#endif // PSYQ_STD_WEAK_PTR_HPP_
