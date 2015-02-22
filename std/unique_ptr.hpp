/** @file
    @brief @copydoc psyq::std_unique_ptr

    std::unique_ptr がある開発環境では std::unique_ptr を、
    std::unique_ptr がない開発環境では boost::interprocess::unique_ptr を、
    psyq::std_unique_ptr でラップする。
    http://d.hatena.ne.jp/gintenlabo/20101205/1291555934

    @author Hillco Psychi (https://twitter.com/psychi)
*/
#ifndef PSYQ_STD_UNIQUE_PTR_HPP_
#define PSYQ_STD_UNIQUE_PTR_HPP_
//#include "psyq/std/move.hpp"
//#include "psyq/std/default_delete.hpp"

#ifdef PSYQ_STD_NO_UNIQUE_PTR
#define PSYQ_STD_UNIQUE_PTR_BASE boost::interprocess::unique_ptr
#include <boost/interprocess/smart_ptr/unique_ptr.hpp>
#else
/// psyq::std_unique_ptr の基底クラステンプレート。
#define PSYQ_STD_UNIQUE_PTR_BASE std::unique_ptr
#include <memory>
#endif // defined(PSYQ_STD_NO_UNIQUE_PTR)

namespace psyq
{
    template<typename, typename> class std_unique_ptr;
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::unique_ptr 互換のスマートポインタ。
 */
template<
    typename template_element,
    typename template_deleter = psyq::std_default_delete<template_element>>
class psyq::std_unique_ptr:
public PSYQ_STD_UNIQUE_PTR_BASE<template_element, template_deleter>
{
    /// thisが指す値の型。
    private: typedef std_unique_ptr this_type;

    /// this_type の基底型。
    private: typedef
        PSYQ_STD_UNIQUE_PTR_BASE<template_element, template_deleter> base_type;

    //-------------------------------------------------------------------------
    /** @brief 空のスマートポインタを構築する。
     */
    public: PSYQ_CONSTEXPR std_unique_ptr() PSYQ_NOEXCEPT {}

#ifndef PSYQ_STD_NO_NULLPTR
    /// @copydoc unique_ptr()
    public: PSYQ_CONSTEXPR std_unique_ptr(psyq::std_nullptr_t const)
    PSYQ_NOEXCEPT:
    base_type(PSYQ_NULLPTR)
    {}
#endif // !defined(PSYQ_STD_NO_NULLPTR)

    /** @brief オブジェクトを所有するスマートポインタを構築する。
        @param[in] in_pointer 所有するオブジェクトを指すポインタ。
     */
    public: explicit std_unique_ptr(
        typename base_type::pointer const in_pointer)
    PSYQ_NOEXCEPT:
    base_type(in_pointer)
    {}

    /** @brief @copydoc std_unique_ptr(base_type::pointer)
        @param[in] in_deleter オブジェクトを破棄する関数オブジェクト。
     */
    public: std_unique_ptr(
        typename base_type::pointer const in_pointer,
        typename base_type::deleter_type const& in_deleter)
    PSYQ_NOEXCEPT:
    base_type(in_pointer, in_deleter)
    {}

    /** @brief @copydoc std_unique_ptr(base_type::pointer)
        @param[in,out] io_deleter オブジェクトを破棄する関数オブジェクト。
     */
    public: std_unique_ptr(
        typename base_type::pointer const in_pointer,
        PSYQ_RV_REF(typename base_type::deleter_type) io_deleter)
    PSYQ_NOEXCEPT:
    base_type(in_pointer, PSYQ_MOVE(io_deleter))
    {}

    /** @brief スマートポインタをムーブ構築する。
        @param[in,out] io_source ムーブ元となるスマートポインタ。
     */
    public: std_unique_ptr(PSYQ_RV_REF(this_type) io_source) PSYQ_NOEXCEPT:
    base_type(PSYQ_MOVE(io_source))
    {}

    /// @copydoc unique_ptr(unique_ptr&&)
    public: template<
        typename template_other_element, 
        typename template_other_deleter>
    std_unique_ptr(
        PSYQ_RV_REF_2_TEMPL_ARGS(
            psyq::std_unique_ptr,
            template_other_element,
            template_other_deleter)
                io_source)
    PSYQ_NOEXCEPT:
    base_type(PSYQ_MOVE(io_source))
    {}

    //-------------------------------------------------------------------------
    /** @brief スマートポインタをムーブ代入する。
        @param[in,out] io_source ムーブ元となるスマートポインタ。
     */
    public: this_type& operator=(PSYQ_RV_REF(this_type) io_source)
    PSYQ_NOEXCEPT
    {
        this->base_type::operator=(PSYQ_MOVE(io_source));
        return *this;
    }

    /// @copydoc operator==(unique_ptr&&)
    public: template<
        typename template_other_element, 
        typename template_other_deleter>
    this_type& operator=(
        PSYQ_RV_REF_2_TEMPL_ARGS(
            psyq::std_unique_ptr,
            template_other_element,
            template_other_deleter)
                io_source)
    PSYQ_NOEXCEPT
    {
        this->base_type::operator=(PSYQ_MOVE(io_source));
        return *this;
    }

    /// @brief スマートポインタを空にする。
    public: this_type& operator=(psyq::std_nullptr_t const)
    PSYQ_NOEXCEPT
    {
#ifdef PSYQ_STD_NO_NULLPTR
        this->reset();
#else
        this->base_type::operator=(PSYQ_NULLPTR);
#endif // !defined(PSYQ_STD_NO_NULLPTR)
        return *this;
    }

    //-------------------------------------------------------------------------
    /// コピー構築は禁止。
    private: std_unique_ptr(this_type const&);

    /// コピー代入は禁止。
    private: this_type& operator=(this_type const&);

}; // class psyq::std_unique_ptr

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq_test
{
    inline void std_unique_ptr()
    {
        typedef psyq::std_unique_ptr<int> int_unique_ptr;
        int_unique_ptr local_ptr_a(PSYQ_NULLPTR);
        int_unique_ptr local_ptr_b(new int);
        *local_ptr_b = 10;
        local_ptr_a = PSYQ_MOVE(local_ptr_b);
        local_ptr_b = PSYQ_NULLPTR;
    }
} // namespace psyq_test

#endif // PSYQ_STD_UNIQUE_PTR_HPP_
