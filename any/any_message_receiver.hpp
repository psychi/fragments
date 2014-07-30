/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_ANY_MESSAGE_RECEIVER_HPP_
#define PSYQ_ANY_MESSAGE_RECEIVER_HPP_

#include <functional>
//#include "psyq/any/any_message_packet.hpp"

namespace psyq
{
    /// @cond
    template<typename> class any_message_receiver;
    /// @endcond
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief メッセージ受信器。
    @tparam template_base_suite @copydoc psyq::any_message_packet::suite
 */
template<typename template_base_suite>
class psyq::any_message_receiver
{
    private: typedef any_message_receiver this_type; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
    /// メッセージ受信器の保持子。
    public: typedef std::shared_ptr<this_type> shared_ptr;
    /// メッセージ受信器の監視子。
    public: typedef std::weak_ptr<this_type> weak_ptr;
    /// メッセージスイートを保持するパケットの基底型。
    public: typedef psyq::any_message_packet<template_base_suite> packet;
    /// @copydoc this_type::functor_
    public: typedef std::function<void(typename this_type::packet const&)>
        functor;

    //-------------------------------------------------------------------------
    /** @brief メッセージ受信器を構築する。
        @param[in] in_functor this_type::functor_ の初期値。
        @param[in] in_address this_type::address_ の初期値。
     */
    public: PSYQ_CONSTEXPR any_message_receiver(
        typename this_type::functor in_functor,
        typename this_type::packet::suite::tag::key const in_address)
    PSYQ_NOEXCEPT:
        functor_((PSYQ_ASSERT(bool(in_functor)), std::move(in_functor))),
        address_(in_address)
    {}

    //-------------------------------------------------------------------------
    /// immutable値として扱いたいので、代入演算子は使用禁止。
    private: this_type operator=(this_type const& in_source)
    {
        this->functor_ = in_source.get_functor();
        this->address_ = in_source.get_address();
        return *this;
    }

    /// immutable値として扱いたいので、代入演算子は使用禁止。
    private: this_type operator=(this_type&& io_source) PSYQ_NOEXCEPT
    {
        this->functor_ = std::move(in_source.functor_);
        this->address_ = in_source.get_address();
        return *this;
    }

    //-------------------------------------------------------------------------
    /** @brief メッセージ受信関数オブジェクトを取得する。
        @return @copydoc this_type::functor_
     */
    public: PSYQ_CONSTEXPR typename this_type::functor const& get_functor()
    const PSYQ_NOEXCEPT
    {
        return this->functor_;
    }

    /** @brief メッセージ受信アドレスを取得する。
        @return @copydoc this_type::address_
     */
    public: PSYQ_CONSTEXPR typename this_type::packet::suite::tag::key
    get_address() const PSYQ_NOEXCEPT
    {
        return this->address_;
    }

    //-------------------------------------------------------------------------
    /// メッセージ受信時に呼び出すコールバック関数オブジェクト。
    private: typename this_type::functor functor_;
    /// メッセージ受信アドレス。
    private: typename this_type::packet::suite::tag::key address_;
};

#endif // !defined(PSYQ_ANY_MESSAGE_RECEIVER_HPP_)
