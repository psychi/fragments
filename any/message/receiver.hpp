/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_ANY_MESSAGE_RECEIVER_HPP_
#define PSYQ_ANY_MESSAGE_RECEIVER_HPP_

#include <functional>
//#include "psyq/any/message/packet.hpp"

/// @cond
namespace psyq
{
    namespace any
    {
        namespace message
        {
            template<typename> class receiver;
        } // namespace message
    } // namespace any
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief RPCメッセージ受信器。メッセージ受信関数オブジェクトを持つ。

    - 動的メモリ確保で生成し、 this_type::shared_ptr で保持する。
    - psyq::any::message::transmitter::register_receiver() で登録しておくと、
      メッセージアドレスとメソッド種別が合致するメッセージパケットの受信時に、
      this_type::get_functor() で取得できる関数オブジェクトが実行される。

    @tparam template_base_suite @copydoc psyq::any::message::packet::suite
 */
template<typename template_base_suite>
class psyq::any::message::receiver
{
    private: typedef receiver this_type; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
    /// this_type の保持子。
    public: typedef std::shared_ptr<this_type> shared_ptr;
    /// this_type の監視子。
    public: typedef std::weak_ptr<this_type> weak_ptr;
    /// メッセージ一式を保持するパケットの基底型。
    public: typedef psyq::any::message::packet<template_base_suite> packet;
    /// メッセージの呼出状。
    public: typedef typename this_type::packet::suite::call call;
    /// メッセージの荷札。
    public: typedef typename this_type::packet::suite::tag tag;
    /// @copydoc this_type::functor_
    public: typedef std::function<void(typename this_type::packet const&)>
        functor;

    //-------------------------------------------------------------------------
    /// @name this_type の構築
    //@{
    /** @brief this_type を構築する。
        @param[in] in_functor         @copydoc this_type::functor_
        @param[in] in_message_address @copydoc this_type::message_address_
     */
    public: PSYQ_CONSTEXPR receiver(
        typename this_type::functor in_functor,
        typename this_type::tag::key const in_message_address)
    PSYQ_NOEXCEPT:
        functor_((PSYQ_ASSERT(bool(in_functor)), std::move(in_functor))),
        message_address_(in_message_address)
    {}
    //@}
    /// immutable値として扱いたいので、代入演算子は使用禁止。
    private: this_type operator=(this_type const& in_source)
    {
        this->functor_ = in_source.get_functor();
        this->message_address_ = in_source.get_message_address();
        return *this;
    }

    /// immutable値として扱いたいので、代入演算子は使用禁止。
    private: this_type operator=(this_type&& io_source) PSYQ_NOEXCEPT
    {
        this->functor_ = std::move(in_source.functor_);
        this->message_address_ = in_source.get_message_address();
        return *this;
    }

    //-------------------------------------------------------------------------
    /// @name this_type のプロパティ
    //@{
    /** @brief メッセージ受信関数オブジェクトを取得する。
        @return @copydoc this_type::functor_
     */
    public: PSYQ_CONSTEXPR typename this_type::functor const& get_functor()
    const PSYQ_NOEXCEPT
    {
        return this->functor_;
    }

    /** @brief メッセージ受信アドレスを取得する。
        @return @copydoc this_type::message_address_
     */
    public: PSYQ_CONSTEXPR typename this_type::tag::key get_message_address()
    const PSYQ_NOEXCEPT
    {
        return this->message_address_;
    }
    //@}
    //-------------------------------------------------------------------------
    /// メッセージ受信時に呼び出すコールバック関数オブジェクト。
    private: typename this_type::functor functor_;
    /// メッセージ受信アドレス。
    private: typename this_type::tag::key message_address_;

}; // class psyq::any::message::receiver

#endif // !defined(PSYQ_ANY_MESSAGE_RECEIVER_HPP_)
