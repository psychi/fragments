/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copybrief psyq::any_rtti
 */
#ifndef PSYQ_ANY_MESSAGE_HPP_
#define PSYQ_ANY_MESSAGE_HPP_
//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 引数を持たないメッセージ。
 */
class void_message
{
    private: typedef void_message self;

    //-------------------------------------------------------------------------
    public: typedef std::uint32_t message_type; ///< メッセージ種別の識別子。
    public: typedef std::uint32_t user_handle;  ///< メッセージ使用者の識別子。

    public: static self::user_handle const UNKNOWN_USER = 0;

    //-------------------------------------------------------------------------
    /** @brief 引数を持たないメッセージを構築する。
        @param[in] in_message_type    メッセージ種別の識別子。
        @param[in] in_receiver_object メッセージを受信するオブジェクトの識別子。
        @param[in] in_sender_object   メッセージを送信したオブジェクトの識別子。
        @param[in] in_sender_terminal メッセージを送信した端末の識別子。
     */
    public: explicit PSYQ_CONSTEXPR void_message(
        self::message_type const in_message_type,
        self::user_handle const  in_receiver_object = self::UNKNOWN_USER,
        self::user_handle const  in_sender_object = self::UNKNOWN_USER,
        self::user_handle const  in_sender_terminal = self::UNKNOWN_USER)
    PSYQ_NOEXCEPT:
        argument_type_(psyq::any_rtti::VOID_HASH),
        message_type_(in_message_type),
        receiver_object_(in_receiver_object),
        sender_object_(in_sender_object),
        sender_terminal_(in_sender_terminal)
    {}

    /** @brief メッセージを構築する。
        @param[in] in_argument_type   メッセージ引数の型識別子。
        @param[in] in_message_type    メッセージ種別の識別子。
        @param[in] in_receiver_object メッセージを受信するオブジェクトの識別子。
        @param[in] in_sender_object   メッセージを送信したオブジェクトの識別子。
        @param[in] in_sender_terminal メッセージを送信した端末の識別子。
     */
    protected: PSYQ_CONSTEXPR void_message(
        psyq::any_rtti::hash const in_argument_type,
        self::message_type const    in_message_type,
        self::user_handle const     in_receiver_object,
        self::user_handle const     in_sender_object,
        self::user_handle const     in_sender_terminal)
    PSYQ_NOEXCEPT:
        argument_type_((
            PSYQ_ASSERT(in_argument_type != psyq::any_rtti::VOID_HASH),
            in_argument_type)),
        message_type_(in_message_type),
        receiver_object_(in_receiver_object),
        sender_object_(in_sender_object),
        sender_terminal_(in_sender_terminal)
    {}

    //-------------------------------------------------------------------------
    public: PSYQ_CONSTEXPR psyq::any_rtti::hash get_argument_type() const PSYQ_NOEXCEPT
    {
        return this->argument_type_;
    }

    public: PSYQ_CONSTEXPR self::message_type get_message_type() const PSYQ_NOEXCEPT
    {
        return this->message_type_;
    }

    public: PSYQ_CONSTEXPR self::user_handle get_receiver_object() const PSYQ_NOEXCEPT
    {
        return this->receiver_object_;
    }

    public: PSYQ_CONSTEXPR self::user_handle get_sender_object() const PSYQ_NOEXCEPT
    {
        return this->sender_object_;
    }

    public: PSYQ_CONSTEXPR self::user_handle get_sender_terminal() const PSYQ_NOEXCEPT
    {
        return this->sender_terminal_;
    }

    //-------------------------------------------------------------------------
    /// メッセージ引数の型識別子。
    private: psyq::any_rtti::hash argument_type_;
    /// メッセージの識別子。
    private: self::message_type message_type_;
    /// メッセージを受信するオブジェクトの識別子。
    private: self::user_handle receiver_object_;
    /// メッセージを送信したオブジェクトの識別子。
    private: self::user_handle sender_object_;
    /// メッセージを送信した端末の識別子。
    private: self::user_handle sender_terminal_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 任意の引数を持つメッセージ。
    @tparam template_argument @copydoc any_message::argument
 */
template<typename template_argument>
class any_message: public void_message
{
    private: typedef any_message self;  ///< thisが指す値の型。
    public: typedef void_message super; ///< self の上位型。

    //-------------------------------------------------------------------------
    public: typedef template_argument argument; ///< メッセージの引数。

    //-------------------------------------------------------------------------
    /** @brief メッセージを構築する。
        @param[in] in_argument        メッセージの引数。
        @param[in] in_message_type    メッセージの種別。
        @param[in] in_receiver_object メッセージを受信するオブジェクトの識別子。
        @param[in] in_sender_object   メッセージを送信したオブジェクトの識別子。
        @param[in] in_sender_terminal メッセージを送信した端末の識別子。
     */
    public: any_message(
        typename self::argument&&          in_argument,
        typename super::message_type const in_message_type,
        typename super::user_handle const  in_receiver_object = self::UNKOWN_USER,
        typename super::user_handle const  in_sender_object = self::UNKOWN_USER,
        typename super::user_handle const  in_sender_terminal = self::UNKOWN_USER)
    PSYQ_NOEXCEPT:
        super(
            psyq::any_rtti::find_hash<template_argument>(),
            in_message_type,
            in_receiver_object,
            in_sender_object,
            in_sender_terminal),
        argument_(std::move(in_argument))
    {}

    /// @copydoc any_message
    public: any_message(
        typename self::argument const&     in_argument,
        typename super::message_type const in_message_type,
        typename super::user_handle const  in_receiver_object = self::UNKOWN_USER,
        typename super::user_handle const  in_sender_object = self::UNKOWN_USER,
        typename super::user_handle const  in_sender_terminal = self::UNKOWN_USER)
    PSYQ_NOEXCEPT:
        super(
            psyq::any_rtti::find_hash<template_argument>(),
            in_message_type,
            in_receiver_object,
            in_sender_object,
            in_sender_terminal),
        argument_(in_argument)
    {}

    /** @brief メッセージ引数を取得する。
        @return メッセージ引数。
     */
    typename self::argument const& get_argument() const PSYQ_NOEXCEPT
    {
        return this->argument_;
    }

    //-------------------------------------------------------------------------
    private: typename self::argument argument_; ///< メッセージの引数。
};
#endif // !defined(PSYQ_ANY_MESSAGE_HPP_)
