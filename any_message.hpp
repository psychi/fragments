/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copybrief psyq::any_rtti
 */
#ifndef PSYQ_ANY_MESSAGE_HPP_
#define PSYQ_ANY_MESSAGE_HPP_

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
        @param[in] in_sending_port      メッセージ送信元アドレス。
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
        @tparam template_value @copydoc with_argument::value_type
     */
    public: template<typename template_value> class with_argument;

    //-------------------------------------------------------------------------
    /** @brief 引数を持たないメッセージを構築する。
        @param[in] in_method_key        呼び出しメソッドの識別子。
        @param[in] in_sequence          メッセージのシーケンス番号。
        @param[in] in_receiving_address メッセージ受信先アドレス。
        @param[in] in_receiving_port    メッセージ受信先ポート番号。
        @param[in] in_sending_port      メッセージ送信元ポート番号。
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
        @param[in] in_argument_type     メッセージ引数の型の識別子。
        @param[in] in_method_key        呼び出しメソッドの識別子。
        @param[in] in_sequence          メッセージのシーケンス番号。
        @param[in] in_receiving_address メッセージ受信先アドレス。
        @param[in] in_receiving_port    メッセージ受信先ポート番号。
        @param[in] in_sending_port      メッセージ送信元ポート番号。
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
    @tparam template_value @copydoc psyq::any_message::with_argument::value_type
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
            in_sequence_index)
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
            in_sequence_index)
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

    //-------------------------------------------------------------------------
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
    /// このルータのアドレス。
    private: psyq::any_message_object_key message_address_;
};

#endif // !defined(PSYQ_ANY_MESSAGE_HPP_)
