/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_ANY_MESSAGE_CALL_HPP_
#define PSYQ_ANY_MESSAGE_CALL_HPP_

#include <type_traits>

namespace psyq
{
    /// @cond
    template<typename> class any_message_call;
    /// @endcond
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief メッセージの呼出状。
    @tparam template_key @copydoc psyq::any_message_call::key
 */
template<typename template_key>
class psyq::any_message_call
{
    private: typedef any_message_call this_type; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
    /// メッセージの呼出状の属性値。
    public: typedef template_key key;
    static_assert(
        std::is_integral<template_key>::value,
        "'template_key' is not integral type.");

    //-------------------------------------------------------------------------
    /** @brief メッセージの呼出状を構築する。
        @param[in] in_method   this_type::method_ の初期値。
        @param[in] in_sequence this_type::sequence_ の初期値。
     */
    public: explicit PSYQ_CONSTEXPR any_message_call(
        typename this_type::key const in_method,
        typename this_type::key const in_sequence = 0)
    PSYQ_NOEXCEPT:
        sequence_(in_sequence),
        method_(in_method)
    {}

    //-------------------------------------------------------------------------
    /** @brief 呼び出しの順序番号を取得する。
        @return @copydoc this_type::sequence_
     */
    public: PSYQ_CONSTEXPR typename this_type::key get_sequence()
    const PSYQ_NOEXCEPT
    {
        return this->sequence_;
    }

    /** @brief 呼び出しメソッドの種別を取得する。
        @return @copydoc this_type::method_
     */
    public: PSYQ_CONSTEXPR typename this_type::key get_method()
    const PSYQ_NOEXCEPT
    {
        return this->method_;
    }

    //-------------------------------------------------------------------------
    /// 呼び出しの順序番号。
    private: typename this_type::key sequence_;
    /// 呼び出しメソッドの種別番号。
    private: typename this_type::key method_;
};

#endif // !defined(PSYQ_ANY_MESSAGE_CALL_HPP_)
