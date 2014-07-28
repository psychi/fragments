/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copybrief psyq::any_rtti
 */
#ifndef PSYQ_ANY_MESSAGE_HPP_
#define PSYQ_ANY_MESSAGE_HPP_

#include <functional>
#include <unordered_map>

#ifndef PSYQ_ANY_MESSAGE_OBJECT_KEY
#define PSYQ_ANY_MESSAGE_OBJECT_KEY std::uint32_t
#endif // !defined(PSYQ_ANY_MESSAGE_OBJECT_KEY)

#ifndef PSYQ_ANY_MESSAGE_METHOD_KEY
#define PSYQ_ANY_MESSAGE_METHOD_KEY std::uint32_t
#endif // !defined(PSYQ_ANY_MESSAGE_METHOD_KEY)

#ifndef PSYQ_ANY_MESSAGE_SEQUENCE_INDEX
#define PSYQ_ANY_MESSAGE_SEQUENCE_INDEX std::uint32_t
#endif // !defined(PSYQ_ANY_MESSAGE_SEQUENCE_INDEX)

namespace psyq
{
    /// @cond
    class any_message_header;
    class any_message_method;
    class any_message_router;
    /// @endcond

    /// メッセージを使用するオブジェクトの識別子。
    typedef PSYQ_ANY_MESSAGE_OBJECT_KEY any_message_object_key;
    /// メッセージ呼び出しメソッドの識別子。
    typedef PSYQ_ANY_MESSAGE_METHOD_KEY any_message_method_key;
    /// メッセージのシーケンス番号。
    typedef PSYQ_ANY_MESSAGE_SEQUENCE_INDEX any_message_sequence_index;
    /// 未知のメッセージ使用者の識別子。
    static any_message_object_key const ANY_MESSAGE_UNKNOWN_OBJECT = 0;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::any_message_header
{
    private: typedef any_message_header this_type;

    //-------------------------------------------------------------------------
    /** @brief メッセージヘッダを構築する。
        @param[in] in_receiving_address メッセージ受信先アドレス。
        @param[in] in_receiving_port    メッセージ受信先ポート番号。
        @param[in] in_sending_address   メッセージ送信元アドレス。
        @param[in] in_sending_port      メッセージ送信元ポート番号。
     */
    public: explicit PSYQ_CONSTEXPR any_message_header(
        psyq::any_message_object_key const in_receiving_address,
        psyq::any_message_object_key const in_receiving_port,
        psyq::any_message_object_key const in_sending_address,
        psyq::any_message_object_key const in_sending_port)
    PSYQ_NOEXCEPT:
        receiver_address_(in_receiving_address),
        receiving_port_(in_receiving_port),
        sending_address_(in_sending_address),
        sending_port_(in_sending_port)
    {}

    //-------------------------------------------------------------------------
    /** @brief メッセージ受信先アドレスを取得する。
        @return メッセージ受信先アドレス。
     */
    public: PSYQ_CONSTEXPR psyq::any_message_object_key get_receiving_address()
    const PSYQ_NOEXCEPT
    {
        return this->sending_address_;
    }

    /** @brief メッセージ受信先ポートを取得する。
        @return メッセージ受信先ポート番号。
     */
    public: PSYQ_CONSTEXPR psyq::any_message_object_key get_receiving_port()
    const PSYQ_NOEXCEPT
    {
        return this->receiving_port_;
    }

    /** @brief メッセージ送信元アドレスを取得する。
        @return メッセージ送信元アドレス。
     */
    public: PSYQ_CONSTEXPR psyq::any_message_object_key get_sending_address()
    const PSYQ_NOEXCEPT
    {
        return this->sending_address_;
    }

    /** @brief メッセージ送信元ポートを取得する。
        @return メッセージ送信元ポート番号。
     */
    public: PSYQ_CONSTEXPR psyq::any_message_object_key get_sending_port()
    const PSYQ_NOEXCEPT
    {
        return this->sending_port_;
    }

    //-------------------------------------------------------------------------
    /// メッセージ受信先アドレス。
    private: psyq::any_message_object_key receiver_address_;
    /// メッセージ受信先ポート番号。
    private: psyq::any_message_object_key receiving_port_;
    /// メッセージ送信元アドレス。
    private: psyq::any_message_object_key sending_address_;
    /// メッセージ送信元ポート番号。
    private: psyq::any_message_object_key sending_port_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 引数を持たないRPCメッセージ。
 */
class psyq::any_message_method
{
    private: typedef any_message_method this_type;

    //-------------------------------------------------------------------------
    /** @brief 任意の引数を持つRPCメッセージ。
     */
    public: template<typename template_value> class with_argument;

    //-------------------------------------------------------------------------
    /** @brief 引数を持たないメッセージを構築する。
        @param[in] in_method_key     呼び出しメソッドの識別子。
        @param[in] in_sequence_index メッセージのシーケンス番号。
     */
    public: explicit PSYQ_CONSTEXPR any_message_method(
        psyq::any_message_method_key const     in_method_key,
        psyq::any_message_sequence_index const in_sequence_index = 0)
    PSYQ_NOEXCEPT:
        argument_type_(psyq::ANY_RTTI_VOID_KEY),
        method_key_(in_method_key),
        sequence_index_(in_sequence_index)
    {}

    /** @brief メッセージを構築する。
        @param[in] in_argument_type  メッセージ引数の型の識別子。
        @param[in] in_method_key     呼び出しメソッドの識別子。
        @param[in] in_sequence_index メッセージのシーケンス番号。
     */
    protected: PSYQ_CONSTEXPR any_message_method(
        psyq::any_rtti_key const               in_argument_type,
        psyq::any_message_method_key const     in_method_key,
        psyq::any_message_sequence_index const in_sequence_index)
    PSYQ_NOEXCEPT:
        argument_type_((
            PSYQ_ASSERT(in_argument_type != psyq::ANY_RTTI_VOID_KEY),
            in_argument_type)),
        method_key_(in_method_key),
        sequence_index_(in_sequence_index)
    {}

    //-------------------------------------------------------------------------
    /** @brief メッセージ引数の型を取得する。
        @return メッセージ引数の型の識別子。
     */
    public: PSYQ_CONSTEXPR psyq::any_rtti_key get_argument_type()
    const PSYQ_NOEXCEPT
    {
        return this->argument_type_;
    }

    /** @brief 呼び出しメソッドを取得する。
        @return 呼び出しメソッドの識別子。
     */
    public: PSYQ_CONSTEXPR psyq::any_message_method_key get_method_key()
    const PSYQ_NOEXCEPT
    {
        return this->method_key_;
    }

    /** @brief メッセージのシーケンス番号を取得する。
        @return メッセージのシーケンス番号。
     */
    public: PSYQ_CONSTEXPR psyq::any_message_sequence_index get_sequence_index()
    const PSYQ_NOEXCEPT
    {
        return this->sequence_index_;
    }

    //-------------------------------------------------------------------------
    /// メッセージ引数の型識別子。
    private: psyq::any_rtti_key argument_type_;
    /// 呼び出しメソッドの識別子。
    private: psyq::any_message_method_key method_key_;
    /// メッセージのシーケンス番号。
    private: psyq::any_message_sequence_index sequence_index_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 任意の引数を持つメッセージ。
    @tparam template_value @copydoc value_type
 */
template<typename template_value>
class psyq::any_message_method::with_argument: public psyq::any_message_method
{
    private: typedef with_argument this_type; ///< thisが指す値の型。
    public: typedef psyq::any_message_method base_type; ///< this_type の基底型。

    //-------------------------------------------------------------------------
    public: typedef template_value value_type; ///< メッセージの引数。

    //-------------------------------------------------------------------------
    /** @brief メッセージを構築する。
        @param[in] in_method_key     呼び出しメソッドの識別子。
        @param[in] in_argument       メッセージ引数の値。
        @param[in] in_sequence_index メッセージのシーケンス番号。
     */
    public: with_argument(
        typename psyq::any_message_method_key const     in_method_key,
        typename this_type::value_type&&                     in_argument,
        typename psyq::any_message_sequence_index const in_sequence_index = 0)
    :
        base_type(
            psyq::any_rtti::find_key<template_value>(),
            in_method_key,
            in_sequence_index),
        argument_(std::move(in_argument))
    {}

    /// @copydoc with_argument
    public: with_argument(
        typename psyq::any_message_method_key const     in_method_key,
        typename this_type::value_type const&                in_argument,
        typename psyq::any_message_sequence_index const in_sequence_index = 0)
    :
        base_type(
            psyq::any_rtti::find_key<template_value>(),
            in_method_key,
            in_sequence_index),
        argument_(in_argument)
    {}

    /** @brief メッセージ引数を取得する。
        @return メッセージ引数。
     */
    typename this_type::value_type const& get_argument() const PSYQ_NOEXCEPT
    {
        return this->argument_;
    }

    //-------------------------------------------------------------------------
    private: typename this_type::value_type argument_; ///< メッセージの引数。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::any_message_router
{
    private: typedef any_message_router this_type; ///< thisが指す値の型。
    private: typedef std::allocator<std::size_t> template_allocator;

    /// メッセージ受信コールバック関数オブジェクト。
    public: typedef std::function<
        void(psyq::any_message_header const&, psyq::any_message_method const&)>
            callback_functor;
    /// メッセージ受信コールバック関数オブジェクトの保持子。
    public: typedef std::shared_ptr<this_type::callback_functor>
        callback_shared_ptr;
    /// メッセージ受信コールバック関数オブジェクトの監視子。
    public: typedef std::weak_ptr<this_type::callback_functor>
        callback_weak_ptr;
    /** @brief メッセージ受信コールバック関数の辞書のキー。

        - 要素#0は、メッセージを受信するポート番号。
        - 要素#1は、呼び出しメッセージの識別子。
     */
    public: typedef std::pair<
        psyq::any_message_object_key, psyq::any_message_method_key>
            callback_key;
    /// コールバック関数辞書のキーからハッシュ値を生成する関数オブジェクト。
    public: struct callback_hash_maker
    {
        std::size_t operator()(callback_key const& in_key) const
        {
            return static_cast<std::size_t>(in_key.first ^ in_key.second);
        }
    };
    /// メッセージ受信コールバック関数の辞書。
    public: typedef std::unordered_multimap<
        this_type::callback_key,
        this_type::callback_weak_ptr,
        this_type::callback_hash_maker,
        std::equal_to<this_type::callback_key>,
        template_allocator>
            callback_map;

    //-------------------------------------------------------------------------
    /** @brief メッセージ受信コールバック関数の辞書を取得する。
        @return メッセージ受信コールバック関数の辞書。
     */
    public: this_type::callback_map const& get_callback_map() const
    {
        return this->callback_map_;
    }

    /** @brief メッセージ受信コールバック関数を登録する。
        @param[in] in_key     登録するメッセージ受信コールバック関数の辞書キー。
        @param[in] in_functor 登録するメッセージ受信コールバック関数。
     */
    public: void register_callback(
        this_type::callback_key const& in_key,
        this_type::callback_shared_ptr const& in_functor)
    {
        auto const local_functor(in_functor.get());
        if (local_functor != nullptr)
        {
            auto const local_position(
                this->find_callback_iterator(in_key, *local_functor));
            if (local_position == this->get_callback_map().end() ||
                local_position->first != in_key)
            {
                this->callback_map_.insert(
                    local_position,
                    this_type::callback_map::value_type(in_key, in_functor));
            }
        }
    }

    /** @brief メッセージ受信コールバック関数を削除する。
        @param[in] in_key     削除するメッセージ受信コールバック関数の辞書キー。
        @param[in] in_functor 削除するメッセージ受信コールバック関数。
     */
    public: void unregister_callback(
        this_type::callback_key const& in_key,
        this_type::callback_shared_ptr const& in_functor)
    {
        auto const local_functor(in_functor.get());
        if (local_functor != nullptr)
        {
            auto const local_iterator(
                this->find_callback_iterator(in_key, *local_functor));
            if (local_iterator != this->get_callback_map().end()
                && local_iterator->first == in_key)
            {
                this->callback_map_.erase(local_iterator);
            }
        }
    }

    /** @brief メッセージ受信コールバック関数をすべて削除する。
        @param[in] in_functor 削除するメッセージ受信コールバック関数。
     */
    public: void unregister_callback(
        this_type::callback_functor const* const in_functor)
    {
        if (in_functor != nullptr)
        {
            for (
                auto i(this->callback_map_.begin());
                i != this->callback_map_.end();)
            {
                auto const local_functor(i->second.lock().get());
                if (local_functor != nullptr && local_functor != in_functor)
                {
                    ++i;
                }
                else
                {
                    i = this->callback_map_.erase(i);
                }
            }
        }
    }

    /** @brief メッセージを送信する。
        @param[in] in_header    送信するメッセージのヘッダ。
        @param[in] in_method    送信するメッセージのメソッド。
        @param[in] in_parameter 送信するメッセージのメソッド引数。
     */
    public: template<typename template_parameter>
    void send_message(
        psyq::any_message_header const& in_header,
        psyq::any_message_method const& in_method,
        template_parameter in_parameter);

    /** @brief メッセージを受信し、コールバック関数を呼び出す。
        @param[in] in_header 受信したメッセージのヘッダ。
        @param[in] in_method 受信したメッセージのメソッド。
     */
    public: void receive_message(
        psyq::any_message_header const& in_header,
        psyq::any_message_method const& in_method)
    {
        auto const local_key(
            std::make_pair(
                in_header.get_receiving_port(), in_method.get_method_key()));
        for (
            auto i(this->callback_map_.find(local_key));
            i != this->callback_map_.end() && i->first == local_key;)
        {
            auto const local_holder(i->second.lock());
            auto const local_functor(local_holder.get());
            if (local_functor != nullptr)
            {
                (*local_functor)(in_header, in_method);
                ++i;
            }
            else
            {
                i = this->callback_map_.erase(i);
            }
        }
    }

    /** @brief このルータから送信するメッセージのヘッダを構築する。
        @param[in] in_receiving_address メッセージ受信先アドレス。
        @param[in] in_receiving_port    メッセージ受信先ポート番号。
        @param[in] in_sending_port      メッセージ送信元アドレス。
     */
    public: psyq::any_message_header make_header(
        psyq::any_message_object_key const in_receiving_address,
        psyq::any_message_object_key const in_receiving_port = psyq::ANY_MESSAGE_UNKNOWN_OBJECT,
        psyq::any_message_object_key const in_sending_port = psyq::ANY_MESSAGE_UNKNOWN_OBJECT)
    const
    {
        return psyq::any_message_header(
            in_receiving_address,
            in_receiving_port,
            this->message_address_,
            in_sending_port);
    }

    //-------------------------------------------------------------------------
    private: this_type::callback_map::const_iterator find_callback_iterator(
        this_type::callback_key const& in_key,
        this_type::callback_functor const& in_functor)
    const
    {
        auto local_iterator(this->get_callback_map().find(in_key));
        while (
            local_iterator != this->get_callback_map().end()
            && local_iterator->first == in_key
            && local_iterator->second.lock().get() != &in_functor)
        {
            ++local_iterator;
        }
        return local_iterator;
    }

    //-------------------------------------------------------------------------
    /// メッセージ受信コールバック関数の辞書。
    private: this_type::callback_map callback_map_;
    /// このルータのアドレス。
    private: psyq::any_message_object_key message_address_;
};

#endif // !defined(PSYQ_ANY_MESSAGE_HPP_)
