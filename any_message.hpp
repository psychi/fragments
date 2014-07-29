/** @file
    @brief スレッド間RPCメッセージの送受信。
    @copydetails psyq::any_message_router
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_RPC_MESSAGE_HPP_
#define PSYQ_RPC_MESSAGE_HPP_

#include <memory>
#include <functional>
#include <vector>
#include <list>
#include <unordered_map>

namespace psyq
{
    /// @cond
    template<typename> class any_message_tag;
    template<typename> class any_message_call;
    template<typename, typename> class any_message_packet;
    template<typename, typename> class any_message_wrapper;
    template<typename, typename> class any_message_receiver;
    template<typename, typename, typename> class any_message_router;
    /// @endcond
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief メッセージの荷札。
    @tparam template_key @copydoc key
 */
template<typename template_key>
class psyq::any_message_tag
{
    private: typedef any_message_tag this_type; ///< thisが指す値の型。

    /// メッセージの荷札の属性値。
    public: typedef template_key key;
    static_assert(
        std::is_integral<template_key>::value,
        "'template_key' is not integral type.");

    public: enum: typename this_type::key
    {
        EMPTY_KEY = 0, ///< 空の属性値。
    };

    //-------------------------------------------------------------------------
    /** @brief メッセージの荷札を構築する。
        @param[in] in_sender_address   メッセージ送信アドレス。
        @param[in] in_receiver_address メッセージ受信アドレス。
        @param[in] in_receiver_mask    メッセージ受信マスク。
     */
    public: explicit PSYQ_CONSTEXPR any_message_tag(
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
        return (in_address & this->get_receiver_mask())
            == this->get_receiver_address();
    }

    //-------------------------------------------------------------------------
    /// メッセージ送信アドレス。
    private: typename this_type::key sender_address_;
    /// メッセージ受信アドレス。
    private: typename this_type::key receiver_address_;
    /// メッセージ受信マスク。
    private: typename this_type::key receiver_mask_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 呼び出しメッセージ。
    @tparam template_key @copydoc key
 */
template<typename template_key>
class psyq::any_message_call
{
    private: typedef any_message_call this_type; ///< thisが指す値の型。

    /// 呼び出しメッセージの属性値。
    public: typedef template_key key;
    static_assert(
        std::is_integral<template_key>::value,
        "'template_key' is not integral type.");

    //-------------------------------------------------------------------------
    /** @brief 呼び出しメッセージを構築する。
        @param[in] in_sequence sequence_ の初期値。
        @param[in] in_method   method_ の初期値。
     */
    public: PSYQ_CONSTEXPR any_message_call(
        typename this_type::key const in_sequence,
        typename this_type::key const in_method)
    PSYQ_NOEXCEPT:
        sequence_(in_sequence),
        method_(in_method)
    {}

    //-------------------------------------------------------------------------
    /** @brief 呼び出しの順序番号を取得する。
        @return @copydoc sequence_
     */
    public: PSYQ_CONSTEXPR typename this_type::key get_sequence()
    const PSYQ_NOEXCEPT
    {
        return this->sequence_;
    }

    /** @brief 呼び出しメソッドの種別を取得する。
        @return @copydoc method_
     */
    public: PSYQ_CONSTEXPR typename this_type::key get_method()
    const PSYQ_NOEXCEPT
    {
        return this->method_;
    }

    //-------------------------------------------------------------------------
    /// @brief 呼び出しの順序番号。
    private: typename this_type::key sequence_;
    /// @brief 呼び出しメソッドの種別番号。
    private: typename this_type::key method_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief メッセージパケットの基底型。
    @tparam template_tag_key  @copydoc psyq::any_message_tag::key
    @tparam template_call_key @copydoc psyq::any_message_call::key
 */
template<typename template_tag_key, typename template_call_key>
class psyq::any_message_packet
{
    private: typedef any_message_packet this_type; ///< thisが指す値の型。

    /// メッセージの荷札。
    public: typedef psyq::any_message_tag<template_tag_key> tag;
    /// 呼び出しメッセージ。
    public: typedef psyq::any_message_call<template_call_key> call;

    /// @cond
    public: template<typename template_parameter> class parametric;
    /// @endcond

    //-------------------------------------------------------------------------
    /** @brief メッセージパケットを構築する。
        @param[in] in_tag  this_type::tag_ の初期値。
        @param[in] in_call this_type::call_ の初期値。
     */
    public: PSYQ_CONSTEXPR any_message_packet(
        typename this_type::tag const& in_tag,
        typename this_type::call const& in_call)
    PSYQ_NOEXCEPT:
        tag_(in_tag),
        call_(in_call)
    {}

    /** @brief メッセージの荷札を取得する。
        @return メッセージの荷札。
     */
    public: PSYQ_CONSTEXPR typename this_type::tag const& get_tag()
    const PSYQ_NOEXCEPT
    {
        return this->tag_;
    }

    /** @brief 呼び出しメッセージを取得する。
        @return 呼び出しメッセージ。
     */
    public: PSYQ_CONSTEXPR typename this_type::call const& get_call()
    const PSYQ_NOEXCEPT
    {
        return this->call_;
    }

    //-------------------------------------------------------------------------
    private: typename this_type::tag tag_;   ///< メッセージの荷札。
    private: typename this_type::call call_; ///< 呼び出しメッセージ。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 任意型の引数を持つメッセージパケット。
    @tparam template_tag_key  @copydoc psyq::any_message_tag::key
    @tparam template_call_key @copydoc psyq::any_message_call::key
    @tparam template_parameter @copydoc parameter
 */
template<typename template_tag_key, typename template_call_key>
template<typename template_parameter>
class psyq::any_message_packet<template_tag_key, template_call_key>::parametric:
    public psyq::any_message_packet<template_tag_key, template_call_key>
{
    private: typedef parametric this_type;  ///< thisが指す値の型。
    /// this_type の基底型。
    public: typedef psyq::any_message_packet<template_tag_key, template_call_key>
        base_type;
    /// メッセージの引数。
    public: typedef template_parameter parameter;

    /** @brief メッセージパケットを構築する。
        @param[in] in_tag       base_type::tag_ の初期値。
        @param[in] in_call      base_type::call_ の初期値。
        @param[in] in_parameter this_type::parameter_ の初期値。
     */
    public: PSYQ_CONSTEXPR parametric(
        typename this_type::tag const& in_tag,
        typename this_type::call const& in_call,
        typename this_type::parameter in_parameter)
    PSYQ_NOEXCEPT:
        base_type(in_tag, in_call),
        parameter_(std::move(in_parameter))
    {}

    public: PSYQ_CONSTEXPR parametric(this_type&& io_source) PSYQ_NOEXCEPT:
        base_type(io_source.get_tag(), io_source.get_call()),
        parameter_(std::move(io_source.parameter_))
    {}

    public: typename this_type::parameter const& get_parameter()
    const PSYQ_NOEXCEPT
    {
        return this->parameter_;
    }

    public: this_type& operator=(this_type&& io_source) PSYQ_NOEXCEPT
    {
        if (this != &io_source)
        {
            static_assert<base_type&>(*this) = std::move(io_source);
            this->parameter_ = std::move(io_source.parameter_);
        }
        return *this;
    }

    private: typename this_type::parameter parameter_; ///< メッセージの引数。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief メッセージラッパーの基底型。
    @tparam template_tag_key  @copydoc psyq::any_message_tag::key
    @tparam template_call_key @copydoc psyq::any_message_call::key
 */
template<typename template_tag_key, typename template_call_key>
class psyq::any_message_wrapper
{
    private: typedef any_message_wrapper this_type;
    public: typedef psyq::any_message_packet<template_tag_key, template_call_key> packet;
    /// @cond
    public: template<typename template_packet> class concrete;
    /// @endcond

    protected: any_message_wrapper() PSYQ_NOEXCEPT {}

    public: virtual ~any_message_wrapper() PSYQ_NOEXCEPT {}

    public: virtual typename this_type::packet const& get_packet()
    const PSYQ_NOEXCEPT = 0;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief メッセージラッパーの具象型。
    @tparam template_tag_key  @copydoc psyq::any_message_tag::key
    @tparam template_call_key @copydoc psyq::any_message_call::key
    @tparam template_packet   @copydoc this_type::packet
 */
template<typename template_tag_key, typename template_call_key>
template<typename template_packet>
class psyq::any_message_wrapper<template_tag_key, template_call_key>::concrete:
    public psyq::any_message_wrapper<template_tag_key, template_call_key>
{
    private: typedef concrete this_type;
    public: typedef psyq::any_message_wrapper<template_tag_key, template_call_key> base_type;
    /// ラップしてるメッセージパケット。
    public: typedef template_packet packet;

    public: concrete(typename this_type::packet in_packet) PSYQ_NOEXCEPT:
        packet_(std::move(in_packet))
    {}

    public: typename base_type::packet const& get_packet()
    const PSYQ_NOEXCEPT override
    {
        return this->packet_;
    }

    private: typename this_type::packet packet_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief メッセージ受信器。
    @tparam template_tag_key  @copydoc psyq::any_message_tag::key
    @tparam template_call_key @copydoc psyq::any_message_call::key
 */
template<typename template_tag_key, typename template_call_key>
class psyq::any_message_receiver
{
    private: typedef any_message_receiver this_type; ///< thisが指す値の型。

    /// メッセージ受信器の保持子。
    public: typedef std::shared_ptr<this_type> shared_ptr;
    /// メッセージ受信器の監視子。
    public: typedef std::weak_ptr<this_type> weak_ptr;
    /// メッセージ受信関数の引数となるメッセージパケットのラッパー。
    public: typedef psyq::any_message_wrapper<template_tag_key, template_call_key>
        message_wrapper;
    /// @copydoc functor_
    public: typedef std::function<void(typename this_type::message_wrapper const&)>
        functor;

    //-------------------------------------------------------------------------
    /** @brief メッセージ受信器を構築する。
        @param[in] in_functor this_type::functor_ の初期値。
        @param[in] in_address this_type::address_ の初期値。
     */
    public: PSYQ_CONSTEXPR any_message_receiver(
        typename this_type::functor in_functor,
        typename this_type::message_wrapper::packet::tag::key const in_address)
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
        @return @copydoc functor_
     */
    public: PSYQ_CONSTEXPR typename this_type::functor const& get_functor()
    const PSYQ_NOEXCEPT
    {
        return this->functor_;
    }

    /** @brief メッセージ受信アドレスを取得する。
        @return @copydoc address_
     */
    public: PSYQ_CONSTEXPR typename this_type::message_wrapper::packet::tag::key get_address()
    const PSYQ_NOEXCEPT
    {
        return this->address_;
    }

    /** @brief メッセージ受信関数を呼び出す。
        @param[in] in_packet 受信したメッセージパケット。
     */
    public: void receive_message(typename this_type::message_wrapper const& in_message)
    {
        auto& local_packet(in_message.get_packet());
        if (local_packet.get_tag().agree_receiver_address(this->get_address())
            && bool(this->functor_))
        {
            this->functor_(in_message);
        }
    }

    //-------------------------------------------------------------------------
    /// メッセージ受信関数オブジェクト。
    private: typename this_type::functor functor_;
    /// メッセージ受信アドレス。
    private: typename this_type::message_wrapper::packet::tag::key address_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief メッセージ中継器。

    - psyq::any_message_router をメッセージ送受信アドレスを指定して構築する。
    - psyq::any_message_router::register_receiver() でメッセージ受信関数を登録する。
    - psyq::any_message_router::send_message() でメッセージを送信すると、
      psyq::any_message_router::register_receiver() で登録したメッセージ受信関数が呼び出される。

    @tparam template_tag_key   @copydoc psyq::any_message_tag::key
    @tparam template_call_key  @copydoc psyq::any_message_call::key
    @tparam template_allocator @copydoc psyq::any_message_router::allocator_type
 */
template<
    typename template_tag_key,
    typename template_call_key,
    typename template_allocator>
class psyq::any_message_router
{
    private: typedef any_message_router this_type; ///< thisが指す値の型。

    /// コンテナに用いるメモリ割当子。
    public: typedef template_allocator allocator_type;

    /// メッセージ受信器。
    public: typedef psyq::any_message_receiver<template_tag_key, template_call_key>
        receiver;

    /// メッセージのメソッド種別からハッシュ値を生成する関数オブジェクト。
    private: struct method_hash
    {
        PSYQ_CONSTEXPR std::size_t operator()(
            typename this_type::receiver::message_wrapper::packet::call::key const in_method)
        const PSYQ_NOEXCEPT
        {
            return static_cast<std::size_t>(in_method);
        }
    };

    /// @copydoc receiver_map_
    private: typedef std::unordered_multimap<
        typename this_type::receiver::message_wrapper::packet::call::key,
        typename this_type::receiver::weak_ptr,
        typename this_type::method_hash,
        std::equal_to<typename this_type::receiver::message_wrapper::packet::call::key>,
        template_allocator>
            receiver_map;

    /// @copydoc provisional_list_
    private: typedef std::list<
        typename this_type::receiver_map::value_type, template_allocator>
            provisional_list;

    private: typedef psyq::any_message_wrapper<template_tag_key, template_call_key>
        packet_wrapper;

    /// メッセージラッパーの保持子。
    private: typedef std::unique_ptr<
        typename this_type::packet_wrapper,
        void(*)(typename this_type::packet_wrapper* const)>
            packet_unique_ptr;

    /// メッセージラッパーの動的配列。
    private: typedef std::vector<
        typename this_type::packet_unique_ptr, template_allocator>
            packet_array;

    //-------------------------------------------------------------------------
    /** @brief メッセージ中継器を構築する。
        @param[in] in_address   構築する中継器のメッセージ送受信アドレス。
        @param[in] in_allocator メモリ割当子の初期値。
     */
    public: any_message_router(
        typename this_type::receiver::message_wrapper::packet::tag::key const in_address,
        typename this_type::allocator_type const& in_allocator)
    PSYQ_NOEXCEPT:
        receiver_map_(in_allocator),
        provisional_list_(in_allocator),
        packet_array_(in_allocator),
        address_((
            PSYQ_ASSERT(
                in_address != this_type::receiver::message_wrapper::packet::tag::EMPTY_KEY),
            in_address)),
        transmitting_(false)
    {}

    /// コピー構築子は使用禁止。
    private: any_message_router(this_type const&);

    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元インスタンス。
     */
    public: any_message_router(this_type&& io_source) PSYQ_NOEXCEPT:
        receiver_map_(io_source.receiver_map_.get_allocator()),
        provisional_list_(io_source.provisional_list_.get_allocator()),
        packet_array_(io_source.packet_array_.get_allocator()),
        address_(this_type::receiver::message_wrapper::packet::tag::EMPTY_KEY),
        transmitting_(false)
    {
        if (!this->move(std::move(io_source)))
        {
            PSYQ_ASSERT(false);
        }
    }

    /// コピー代入演算子は使用禁止。
    private: this_type& operator=(this_type const&);

    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元インスタンス。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source)
    {
        if (!this->move(std::move(io_source)))
        {
            PSYQ_ASSERT(false);
        }
        return *this;
    }

    //-------------------------------------------------------------------------
    /** @brief メッセージの送受信に使うアドレスを取得する。
        @return メッセージの送受信に使うアドレス。
     */
    public: PSYQ_CONSTEXPR typename this_type::receiver::message_wrapper::packet::tag::key const
    get_address() const PSYQ_NOEXCEPT
    {
        return this->address_;
    }

    /** @brief このルータから送信するメッセージの荷札を構築する。
        @param[in] in_receiver_address メッセージ受信アドレス。
        @param[in] in_receiver_mask    メッセージ受信マスク。
     */
    public: PSYQ_CONSTEXPR typename this_type::receiver::message_wrapper::packet::tag
    make_receiver_tag(
        typename this_type::receiver::message_wrapper::packet::tag::key const
            in_receiver_address,
        typename this_type::receiver::message_wrapper::packet::tag::key const
            in_receiver_mask = ~this_type::receiver::message_wrapper::packet::tag::EMPTY_KEY)
    const PSYQ_NOEXCEPT
    {
        return typename this_type::receiver::message_wrapper::packet::tag(
            this->get_address(), in_receiver_address, in_receiver_mask);
    }

    //-------------------------------------------------------------------------
    /** @brief メッセージ受信器を登録する。

        this_type::unregister_receiver で登録を除去できる。

        @param[in] in_method   登録するメッセージ受信メソッドの種別。
        @param[in] in_receiver 登録するメッセージ受信器。
     */
    public: void register_receiver(
        typename this_type::receiver::message_wrapper::packet::call::key const in_method,
        typename this_type::receiver::shared_ptr const& in_receiver)
    {
        // 仮登録する。 transmit() で実際に登録される。
        this->provisional_list_.emplace_back(
            typename this_type::receiver_map::value_type(
                in_method, in_receiver));
    }

    /** @brief メッセージ受信器の登録を除去する。

        this_type::register_receiver() で登録したメッセージ受信器を解放する。

        @param[in] in_receiver 登録を除去するメッセージ受信器。
     */
    public: void unregister_receiver(
        typename this_type::receiver const* const in_receiver)
    {
        if (in_receiver != nullptr)
        {
            // メッセージ受信器の辞書から削除する。
            for (auto& local_value: this->receiver_map_)
            {
                auto const local_receiver(local_value.second.lock().get());
                if (local_receiver == nullptr || local_receiver == in_receiver)
                {
                    local_value.second.reset();
                }
            }

            // 仮登録リストからも削除する。
            for (auto& local_value: this->provisional_list_)
            {
                if (local_value.second.lock().get() == in_receiver)
                {
                    local_value.second.reset();
                }
            }
        }
    }

    /** @brief メッセージ受信器の登録を除去する。

        this_type::register_receiver() で登録したメッセージ受信器を解放する。

        @param[in] in_method   登録を除去するメッセージ受信器のメソッド番号。
        @param[in] in_receiver 登録を除去するメッセージ受信器。
     */
    public: void unregister_receiver(
        typename this_type::receiver::message_wrapper::packet::call::key const in_method,
        typename this_type::receiver const* const in_receiver)
    {
        if (in_receiver != nullptr)
        {
            // メッセージ受信器の辞書から削除する。
            auto const local_iterator(
                this_type::find_receiver_iterator(
                    this->receiver_map_, in_method, in_receiver));
            if (local_iterator != this->receiver_map_.end()
                && local_iterator->first == in_method)
            {
                const_cast<typename this_type::receiver::weak_ptr&>
                    (local_iterator->second).reset();
            }

            // 仮登録リストからも削除する。
            for (auto& local_value: this->provisional_list_)
            {
                if (local_value.first == in_method
                    && local_value.second.lock().get() == in_receiver)
                {
                    local_value.second.reset();
                }
            }
        }
    }

    //-------------------------------------------------------------------------
    /** @brief 引数のないメッセージを送信する。
        @param[in] in_tag  送信するメッセージの荷札。
        @param[in] in_call 送信する呼び出しメッセージ。
     */
    public: bool send_message(
        typename this_type::receiver::message_wrapper::packet::tag const& in_tag,
        typename this_type::receiver::message_wrapper::packet::call const& in_call)
    {
        return this_type::create_packet_wrapper(
            this->packet_array_,
            typename this_type::receiver::message_wrapper::packet(in_tag, in_call));
    }

    /** @brief 任意型の引数を持つプロセス内メッセージを送信する。
        @param[in] in_tag       送信するメッセージの荷札。
        @param[in] in_call      送信する呼び出しメッセージ。
        @param[in] in_parameter メッセージの引数。
     */
    public: template<typename template_parameter>
    bool send_parameteric_message(
        typename this_type::receiver::message_wrapper::packet::tag const& in_tag,
        typename this_type::receiver::message_wrapper::packet::call const& in_call,
        template_parameter in_parameter)
    {
        return this_type::create_packet_wrapper(
            this->packet_array_,
            typename this_type::receiver::message_wrapper::packet::template
                parametric<template_parameter>(
                    in_tag, in_call, std::move(in_parameter)));
    }

    /** @brief POD型の引数を持つプロセス外メッセージを送信する。
        @param[in] in_tag       送信するメッセージの荷札。
        @param[in] in_call      送信する呼び出しメッセージ。
        @param[in] in_parameter メッセージの引数。必ずPOD型。
     */
    public: template<typename template_parameter>
    bool send_external_message(
        typename this_type::receiver::message_wrapper::packet::tag const& in_tag,
        typename this_type::receiver::message_wrapper::packet::call const& in_call,
        template_parameter in_parameter);

    /** @brief メッセージを受信し、メッセージ受信関数を呼び出す。
        @param[in] in_packet 受信するメッセージのパケット。
     */
    public: void receive_message(
        typename this_type::receiver::message_wrapper::packet const& in_packet)
    {
        this_type::transmit_packet(this->receiver_map_, in_packet);
    }

    //-------------------------------------------------------------------------
    public: bool transmit()
    {
        if (this->is_transmitting())
        {
            return false;
        }
        this->transmitting_ = true;
        this_type::remove_empty_receiver(this->receiver_map_);
        this_type::merge_receiver_container(
            this->receiver_map_, this->provisional_list_);
        this_type::transmit_packet_wrapper(
            this->receiver_map_, this->packet_array_);
        this->transmitting_ = false;
        return true;
    }

    public: PSYQ_CONSTEXPR bool is_transmitting() const PSYQ_NOEXCEPT
    {
        return this->transmitting_;
    }

    //-------------------------------------------------------------------------
    private: bool move(this_type&& io_source)
    {
        if (this->is_transmitting() || io_source.is_transmitting())
        {
            return false;
        }
        io_source.transmitting_ = true;
        this->transmitting_ = true;
        this->receiver_map_ = std::move(io_source.receiver_map_);
        this->provisional_list_ = std::move(io_source.provisional_list_);
        this->packet_array_ = std::move(io_source.packet_array_);
        this->address_ = io_source.get_address();
        io_source.address_ = this_type::receiver::message_wrapper::packet::tag::EMPTY_KEY;
        this->transmitting_ = false;
        io_source.transmitting_ = false;
        return true;
    }

    private: static typename this_type::receiver_map::const_iterator
    find_receiver_iterator(
        typename this_type::receiver_map const& in_receiver_map,
        typename this_type::receiver::message_wrapper::packet::call::key const in_method,
        void const* const in_receiver)
    PSYQ_NOEXCEPT
    {
        auto local_iterator(in_receiver_map.find(in_method));
        while (
            local_iterator != in_receiver_map.end()
            && local_iterator->first == in_method
            && local_iterator->second.lock().get() != in_receiver)
        {
            ++local_iterator;
        }
        return local_iterator;
    }

    /** @brief 空になったメッセージ受信器を削除する。
        @param[in,out] io_receiver_map 空のメッセージ受信器を削除する辞書。
     */
    private: static void remove_empty_receiver(
        typename this_type::receiver_map& io_receiver_map)
    {
        for (auto i(io_receiver_map.begin()); i != io_receiver_map.end();)
        {
            if (i->second.lock().get() != nullptr)
            {
                ++i;
            }
            else
            {
                i = io_receiver_map.erase(i);
            }
        }
    }

    /** @brief 仮登録されたメッセージ受信器を、実際に登録する。
        @param[in,out] io_receiver_map     メッセージ受信器を登録する辞書。
        @param[in,out] io_provisional_list 仮登録されたメッセージ受信器のリスト。
     */
    private: static void merge_receiver_container(
        typename this_type::receiver_map& io_receiver_map,
        typename this_type::provisional_list& io_provisional_list)
    {
        auto const local_provisional_list(std::move(io_provisional_list));
        io_provisional_list.clear();
        for (auto& local_provision: local_provisional_list)
        {
            auto const local_receiver(local_provision.second.lock().get());
            if (local_receiver != nullptr)
            {
                auto const local_position(
                    this_type::find_receiver_iterator(
                        io_receiver_map, local_provision.first, local_receiver));
                if (local_position == io_receiver_map.end() ||
                    local_position->first != local_provision.first)
                {
                    io_receiver_map.emplace_hint(
                        local_position, std::move(local_provision));
                }
            }
        }
    }

    private: static void transmit_packet_wrapper(
        typename this_type::receiver_map const& in_receiver_map,
        typename this_type::packet_array& io_packet_array)
    {
        auto const local_packet_array(std::move(io_packet_array));
        io_packet_array.clear();
        io_packet_array.reserve(local_packet_array.size());
        for (auto& local_packet_holder: local_packet_array)
        {
            auto const local_packet_wrapper(local_packet_holder.get());
            if (local_packet_wrapper != nullptr)
            {
                this_type::transmit_packet_wrapper(
                    in_receiver_map, *local_packet_wrapper);
            }
            else
            {
                PSYQ_ASSERT(false);
            }
        }
    }

    private: static void transmit_packet_wrapper(
        typename this_type::receiver_map const& in_receiver_map,
        typename this_type::packet_wrapper const& in_packet_wrapper)
    {
        auto& local_packet(in_packet_wrapper.get_packet());
        auto const local_method(local_packet.get_call().get_method());
        for (
            auto i(in_receiver_map.find(local_method));
            i != in_receiver_map.end() && i->first == local_method;
            ++i)
        {
            auto const local_holder(i->second.lock());
            auto const local_receiver(local_holder.get());
            if (local_receiver != nullptr)
            {
                local_receiver->receive_message(in_packet_wrapper);
            }
        }
    }

    private: template<typename template_packet>
    static bool create_packet_wrapper(
        typename this_type::packet_array& io_packet_array,
        template_packet&& in_packet)
    {
        typedef typename this_type::packet_wrapper::template
            concrete<template_packet>
                concrete_wrapper;
        typename this_type::allocator_type::template
            rebind<concrete_wrapper>::other
                local_allocator;
        auto const local_storage(local_allocator.allocate(1));
        if (local_storage == nullptr)
        {
            PSYQ_ASSERT(false);
            return false;
        }
        io_packet_array.emplace_back(
            typename this_type::packet_unique_ptr(
                new(local_storage) concrete_wrapper(std::move(in_packet)),
                this_type::destroy_packet_wrapper<concrete_wrapper>));
        return true;
    }

    private: template<typename template_wrapper>
    static void destroy_packet_wrapper(
        typename this_type::packet_wrapper* const in_wrapper)
    {
        if (in_wrapper != nullptr)
        {
            typename this_type::allocator_type::template
                rebind<template_wrapper>::other
                    local_allocator;
            local_allocator.destroy(in_wrapper);
            local_allocator.deallocate(
                static_cast<template_wrapper*>(in_wrapper), 1);
        }
    }

    //-------------------------------------------------------------------------
    /// メッセージ受信器の辞書。
    private: typename this_type::receiver_map receiver_map_;
    /// 仮登録したメッセージ受信器のリスト。
    private: typename this_type::provisional_list provisional_list_;
    /// 中継するパケットの配列。
    private: typename this_type::packet_array packet_array_;
    /// このルータのメッセージ送受信アドレス。
    private: typename this_type::receiver::message_wrapper::packet::tag::key address_;
    /// メッセージ中継の途中かどうか。
    private: bool transmitting_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    namespace test
    {
        inline void any_message()
        {
            typedef psyq::any_message_router<
                std::uint32_t, std::uint32_t, std::allocator<void*>>
                    message_router;

            message_router local_router(0x7f000001, message_router::allocator_type());
            local_router.send_message(
                local_router.make_receiver_tag(local_router.get_address()),
                message_router::receiver::message_wrapper::packet::call(0, 1));
            local_router.send_parameteric_message(
                local_router.make_receiver_tag(local_router.get_address()),
                message_router::receiver::message_wrapper::packet::call(0, 1),
                0.5);
            local_router.transmit();
            local_router.unregister_receiver(0, nullptr);
            local_router.unregister_receiver(nullptr);
        }
    }
}

#endif // !defined(PSYQ_RPC_MESSAGE_HPP_)
