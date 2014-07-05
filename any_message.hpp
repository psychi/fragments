/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copybrief psyq::any_rtti
 */
#ifndef PSYQ_ANY_MESSAGE_HPP_
#define PSYQ_ANY_MESSAGE_HPP_

#ifndef PSYQ_ANY_MESSAGE_METHOD_KEY
#define PSYQ_ANY_MESSAGE_METHOD_KEY std::uint32_t
#endif // !defined(PSYQ_ANY_MESSAGE_METHOD_KEY)

#ifndef PSYQ_ANY_MESSAGE_SEQUENCE_INDEX
#define PSYQ_ANY_MESSAGE_SEQUENCE_INDEX std::uint32_t
#endif // !defined(PSYQ_ANY_MESSAGE_SEQUENCE_INDEX)

#ifndef PSYQ_ANY_MESSAGE_USER_HANDLE
#define PSYQ_ANY_MESSAGE_USER_HANDLE std::uint32_t
#endif // !defined(PSYQ_ANY_MESSAGE_USER_HANDLE)


namespace psyq
{
    /// @cond
    class any_message;
    /// @endcond
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 引数を持たないRPCメッセージ。
 */
class psyq::any_message
{
    private: typedef any_message self;

    //-------------------------------------------------------------------------
    /// 呼び出しメソッドの識別子。
    public: typedef PSYQ_ANY_MESSAGE_METHOD_KEY method_key;
    /// メッセージのシーケンス番号。
    public: typedef PSYQ_ANY_MESSAGE_SEQUENCE_INDEX sequence_index;
    /// メッセージ使用者の識別子。
    public: typedef PSYQ_ANY_MESSAGE_USER_HANDLE user_handle;
    /// 未知のメッセージ使用者の識別子。
    public: static self::user_handle const UNKNOWN_USER = 0;

    /** @brief 任意の引数を持つRPCメッセージ。
        @tparam template_value @copydoc with_argument::value_type
     */
    public: template<typename template_value> class with_argument;

    //-------------------------------------------------------------------------
    /** @brief 引数を持たないメッセージを構築する。
        @param[in] in_method_key      呼び出しメソッドの識別子。
        @param[in] in_sequence_index  メッセージのシーケンス番号。
        @param[in] in_receiver_object メッセージを受信するオブジェクトの識別子。
        @param[in] in_sender_object   メッセージを送信したオブジェクトの識別子。
        @param[in] in_sender_terminal メッセージを送信した端末の識別子。
     */
    public: explicit PSYQ_CONSTEXPR any_message(
        self::method_key const     in_method_key,
        self::sequence_index const in_sequence_index = 0,
        self::user_handle const    in_receiver_object = self::UNKNOWN_USER,
        self::user_handle const    in_sender_object = self::UNKNOWN_USER,
        self::user_handle const    in_sender_terminal = self::UNKNOWN_USER)
    PSYQ_NOEXCEPT:
        argument_type_(psyq::any_rtti::VOID_KEY),
        method_key_(in_method_key),
        sequence_index_(in_sequence_index),
        receiver_object_(in_receiver_object),
        sender_object_(in_sender_object),
        sender_terminal_(in_sender_terminal)
    {}

    /** @brief メッセージを構築する。
        @param[in] in_argument_type   メッセージ引数の型の識別子。
        @param[in] in_method_key      呼び出しメソッドの識別子。
        @param[in] in_sequence_index  メッセージのシーケンス番号。
        @param[in] in_receiver_object メッセージを受信するオブジェクトの識別子。
        @param[in] in_sender_object   メッセージを送信したオブジェクトの識別子。
        @param[in] in_sender_terminal メッセージを送信した端末の識別子。
     */
    protected: PSYQ_CONSTEXPR any_message(
        psyq::any_rtti::key const  in_argument_type,
        self::method_key const     in_method_key,
        self::sequence_index const in_sequence_index,
        self::user_handle const    in_receiver_object,
        self::user_handle const    in_sender_object,
        self::user_handle const    in_sender_terminal)
    PSYQ_NOEXCEPT:
        argument_type_((
            PSYQ_ASSERT(in_argument_type != psyq::any_rtti::VOID_KEY),
            in_argument_type)),
        method_key_(in_method_key),
        sequence_index_(in_sequence_index),
        receiver_object_(in_receiver_object),
        sender_object_(in_sender_object),
        sender_terminal_(in_sender_terminal)
    {}

    //-------------------------------------------------------------------------
    /** @brief メッセージ引数の型を取得する。
        @return メッセージ引数の型の識別子。
     */
    public: PSYQ_CONSTEXPR psyq::any_rtti::key get_argument_type()
    const PSYQ_NOEXCEPT
    {
        return this->argument_type_;
    }

    /** @brief 呼び出しメソッドを取得する。
        @return 呼び出しメソッドの識別子。
     */
    public: PSYQ_CONSTEXPR self::method_key get_method_key()
    const PSYQ_NOEXCEPT
    {
        return this->method_key_;
    }

    /** @brief メッセージのシーケンス番号を取得する。
        @return メッセージのシーケンス番号。
     */
    public: PSYQ_CONSTEXPR self::method_key get_sequence_index()
    const PSYQ_NOEXCEPT
    {
        return this->sequence_index_;
    }

    /** @brief メッセージを受信するオブジェクトを取得する。
        @return メッセージを受信するオブジェクトの識別子。
     */
    public: PSYQ_CONSTEXPR self::user_handle get_receiver_object()
    const PSYQ_NOEXCEPT
    {
        return this->receiver_object_;
    }

    /** @brief メッセージを送信したオブジェクトを取得する。
        @return メッセージを受信したオブジェクトの識別子。
     */
    public: PSYQ_CONSTEXPR self::user_handle get_sender_object()
    const PSYQ_NOEXCEPT
    {
        return this->sender_object_;
    }

    /** @brief メッセージを送信した端末を取得する。
        @return メッセージを受信した端末の識別子。
     */
    public: PSYQ_CONSTEXPR self::user_handle get_sender_terminal()
    const PSYQ_NOEXCEPT
    {
        return this->sender_terminal_;
    }

    //-------------------------------------------------------------------------
    /// メッセージ引数の型識別子。
    private: psyq::any_rtti::key argument_type_;
    /// 呼び出しメソッドの識別子。
    private: self::method_key method_key_;
    /// メッセージのシーケンス番号。
    private: self::sequence_index sequence_index_;
    /// メッセージを受信するオブジェクトの識別子。
    private: self::user_handle receiver_object_;
    /// メッセージを送信したオブジェクトの識別子。
    private: self::user_handle sender_object_;
    /// メッセージを送信した端末の識別子。
    private: self::user_handle sender_terminal_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 任意の引数を持つメッセージ。
    @tparam template_value @copydoc psyq::any_message::with_argument::value_type
 */
template<typename template_value>
class psyq::any_message::with_argument: public psyq::any_message
{
    private: typedef with_argument self; ///< thisが指す値の型。
    public: typedef psyq::any_message super; ///< self の上位型。

    //-------------------------------------------------------------------------
    public: typedef template_value value_type; ///< メッセージの引数。

    //-------------------------------------------------------------------------
    /** @brief メッセージを構築する。
        @param[in] in_argument        メッセージ引数の値。
        @param[in] in_method_key      メッセージの種別の識別子。
        @param[in] in_sequence_index  メッセージのシーケンス番号。
        @param[in] in_receiver_object メッセージを受信するオブジェクトの識別子。
        @param[in] in_sender_object   メッセージを送信したオブジェクトの識別子。
        @param[in] in_sender_terminal メッセージを送信した端末の識別子。
     */
    public: with_argument(
        typename self::value_type&&         in_argument,
        typename super::method_key const    in_method_key,
        typename self::sequence_index const in_sequence_index = 0,
        typename super::user_handle const   in_receiver_object = self::UNKOWN_USER,
        typename super::user_handle const   in_sender_object = self::UNKOWN_USER,
        typename super::user_handle const   in_sender_terminal = self::UNKOWN_USER)
    :
        super(
            psyq::any_rtti::find_key<template_value>(),
            in_method_key,
            in_sequence_index,
            in_receiver_object,
            in_sender_object,
            in_sender_terminal),
        argument_(std::move(in_argument))
    {}

    /// @copydoc with_argument
    public: with_argument(
        typename self::value_type const&    in_argument,
        typename super::method_key const    in_method_key,
        typename self::sequence_index const in_sequence_index = 0,
        typename super::user_handle const   in_receiver_object = self::UNKOWN_USER,
        typename super::user_handle const   in_sender_object = self::UNKOWN_USER,
        typename super::user_handle const   in_sender_terminal = self::UNKOWN_USER)
    :
        super(
            psyq::any_rtti::find_key<template_value>(),
            in_method_key,
            in_sequence_index,
            in_receiver_object,
            in_sender_object,
            in_sender_terminal),
        argument_(in_argument)
    {}

    /** @brief メッセージ引数を取得する。
        @return メッセージ引数。
     */
    typename self::value_type const& get_argument() const PSYQ_NOEXCEPT
    {
        return this->argument_;
    }

    //-------------------------------------------------------------------------
    private: typename self::value_type argument_; ///< メッセージの引数。
};
#endif // !defined(PSYQ_ANY_MESSAGE_HPP_)
