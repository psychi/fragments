/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_ANY_MESSAGE_TAG_HPP_
#define PSYQ_ANY_MESSAGE_TAG_HPP_

#include <type_traits>

namespace psyq
{
    namespace any
    {
        /// psyq::any::rtti を使ったRPCメッセージ。
        namespace message
        {
            /// @cond
            template<typename> class tag;
            /// @endcond
        } // namespace message
    } // namespace any
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief RPCメッセージの荷札。
    @tparam template_key @copydoc psyq::any::message::tag::key
 */
template<typename template_key>
class psyq::any::message::tag
{
    private: typedef tag this_type; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
    /// メッセージの荷札の属性値。
    public: typedef template_key key;
    static_assert(
        std::is_integral<template_key>::value,
        "'template_key' is not integral type.");

    //-------------------------------------------------------------------------
    /** @brief メッセージの荷札を構築する。
        @param[in] in_sender_address   メッセージ送信アドレス。
        @param[in] in_receiver_address メッセージ受信アドレス。
        @param[in] in_receiver_mask    メッセージ受信マスク。
     */
    public: explicit PSYQ_CONSTEXPR tag(
        typename this_type::key const in_sender_address,
        typename this_type::key const in_receiver_address,
        typename this_type::key const in_receiver_mask)
    PSYQ_NOEXCEPT:
        sender_address_(in_sender_address),
        receiver_address_(in_receiver_address),
        receiver_mask_(in_receiver_mask)
    {}

    //-------------------------------------------------------------------------
    /** @brief メッセージ送信アドレスを取得する。
        @return メッセージ送信アドレス。
     */
    public: PSYQ_CONSTEXPR typename this_type::key get_sender_address()
    const PSYQ_NOEXCEPT
    {
        return this->sender_address_;
    }

    /** @brief メッセージ受信アドレスを取得する。
        @return メッセージ受信アドレス。
     */
    public: PSYQ_CONSTEXPR typename this_type::key get_receiver_address()
    const PSYQ_NOEXCEPT
    {
        return this->receiver_address_;
    }

    /** @brief メッセージ受信マスクを取得する。
        @return メッセージ受信マスク。
     */
    public: PSYQ_CONSTEXPR typename this_type::key get_receiver_mask()
    const PSYQ_NOEXCEPT
    {
        return this->receiver_mask_;
    }

    /** @brief メッセージ受信アドレスに該当するか判定する。
        @retval true  メッセージ受信アドレスに該当する。
        @retval false メッセージ受信アドレスに該当しない。
     */
    public: PSYQ_CONSTEXPR bool agree_receiver_address(
        typename this_type::key const in_address)
    const PSYQ_NOEXCEPT
    {
        return (in_address & this->get_receiver_mask()) == this->get_receiver_address();
    }

    //-------------------------------------------------------------------------
    /// メッセージ送信アドレス。
    private: typename this_type::key sender_address_;
    /// メッセージ受信アドレス。
    private: typename this_type::key receiver_address_;
    /// メッセージ受信マスク。
    private: typename this_type::key receiver_mask_;

}; // class psyq::any::message::tag

#endif // !defined(PSYQ_ANY_MESSAGE_TAG_HPP_)
