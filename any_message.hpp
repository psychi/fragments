/** @file
    @brief RPCメッセージの送受信。
    @copydetails psyq::any_message_router
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_ANY_MESSAGE_HPP_
#define PSYQ_ANY_MESSAGE_HPP_

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
    template<typename, typename, typename> class any_message_suite;
    template<typename> class any_message_packet;
    template<typename> class any_message_receiver;
    template<
        typename = psyq::any_message_suite<std::uint32_t, std::uint32_t, std::uint32_t>,
        typename = std::allocator<void*>>
            class any_message_router;
    /// @endcond
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief メッセージの荷札。
    @tparam template_key @copydoc psyq::any_message_tag::key
 */
template<typename template_key>
class psyq::any_message_tag
{
    private: typedef any_message_tag this_type; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
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

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief メッセージの荷札と呼出状と引数を一揃いにしたスイートの基底型。
    @tparam template_tag_key  @copydoc psyq::any_message_tag::key
    @tparam template_call_key @copydoc psyq::any_message_call::key
    @tparam template_size     @copydoc psyq::any_message_suite::size_type
 */
template<
    typename template_tag_key,
    typename template_call_key,
    typename template_size>
class psyq::any_message_suite
{
    private: typedef any_message_suite this_type; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
    /// @copydoc this_type::tag_
    public: typedef psyq::any_message_tag<template_tag_key> tag;
    /// @copydoc this_type::call_
    public: typedef psyq::any_message_call<template_call_key> call;
    /// バイト数を表す型。
    public: typedef template_size size_type;
    static_assert(
        std::is_unsigned<template_size>::value,
        "'template_size' is not unsigned type.");
    /// メッセージ引数を持たないのでvoid型。
    public: typedef void parameter;
    /// @cond
    public: template<typename template_parameter> class parametric;
    /// @endcond

    //-------------------------------------------------------------------------
    /** @brief メッセージスイートを構築する。
        @param[in] in_tag  this_type::tag_ の初期値。
        @param[in] in_call this_type::call_ の初期値。
     */
    public: PSYQ_CONSTEXPR any_message_suite(
        typename this_type::tag const& in_tag,
        typename this_type::call const& in_call)
    PSYQ_NOEXCEPT:
        tag_(in_tag),
        call_(in_call),
        parameter_offset_(sizeof(this_type)),
        parameter_size_(0)
    {}

    /** @brief 引数を持つメッセージスイートを構築する。
        @param[in] in_tag         this_type::tag_ の初期値。
        @param[in] in_call        this_type::call_ の初期値。
        @param[in] in_parameter   引数の先頭位置。
        @param[in] in_suite_size メッセージスイート全体のバイトサイズ。
     */
    protected: PSYQ_CONSTEXPR any_message_suite(
        typename this_type::tag const& in_tag,
        typename this_type::call const& in_call,
        void const* const in_parameter,
        std::size_t const in_suite_size)
    PSYQ_NOEXCEPT:
        tag_(in_tag),
        call_(in_call),
        parameter_offset_((
            PSYQ_ASSERT(in_parameter != nullptr),
            std::distance(
                reinterpret_cast<std::uint8_t const*>(this),
                static_cast<std::uint8_t const*>(in_parameter)))),
        parameter_size_((
            PSYQ_ASSERT(in_suite_size <= (std::numeric_limits<typename this_type::size_type>::max)()),
            PSYQ_ASSERT(this->parameter_offset_ <= in_suite_size),
            static_cast<typename this_type::size_type>(
                in_suite_size - this->parameter_offset_)))
    {}

    /** @brief メッセージの荷札を取得する。
        @return メッセージの荷札。
     */
    public: PSYQ_CONSTEXPR typename this_type::tag const& get_tag()
    const PSYQ_NOEXCEPT
    {
        return this->tag_;
    }

    /** @brief メッセージの呼出状を取得する。
        @return メッセージの呼出状。
     */
    public: PSYQ_CONSTEXPR typename this_type::call const& get_call()
    const PSYQ_NOEXCEPT
    {
        return this->call_;
    }

    /** @brief メッセージ引数の先頭位置を取得する。
        @return メッセージ引数の先頭位置。
     */
    public: PSYQ_CONSTEXPR void const* get_parameter_data()
    const PSYQ_NOEXCEPT
    {
        return reinterpret_cast<std::uint8_t const*>(this)
            +  this->parameter_offset_;
    }

    /** @brief メッセージ引数のバイトサイズを取得する。
        @return メッセージ引数のバイトサイズ。
     */
    public: PSYQ_CONSTEXPR typename this_type::size_type get_parameter_size()
    const PSYQ_NOEXCEPT
    {
        return this->parameter_size_;
    }

    //-------------------------------------------------------------------------
    /// メッセージの荷札。
    private: typename this_type::tag tag_;
    /// メッセージの呼出状。
    private: typename this_type::call call_;
    /// メッセージ引数の先頭位置へのオフセットバイト数。
    private: typename this_type::size_type parameter_offset_;
    /// メッセージ引数のバイトサイズ。
    private: typename this_type::size_type parameter_size_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief メッセージの荷札と呼出状と引数をひとまとめにしたスイート。
    @tparam template_tag_key   @copydoc psyq::any_message_tag::key
    @tparam template_call_key  @copydoc psyq::any_message_call::key
    @tparam template_size      @copydoc psyq::any_message_suite::size_type
    @tparam template_parameter @copydoc psyq::any_message_suite::parametric::parameter
 */
template<
    typename template_tag_key,
    typename template_call_key,
    typename template_size>
template<typename template_parameter>
class psyq::any_message_suite<template_tag_key, template_call_key, template_size>::parametric:
    public psyq::any_message_suite<template_tag_key, template_call_key, template_size>
{
    /// thisが指す値の型。
    private: typedef parametric this_type;
    /// this_type の基底型。
    public: typedef psyq::any_message_suite<
        template_tag_key, template_call_key, template_size>
            base_type;

    //-------------------------------------------------------------------------
    /// @copydoc this_type::parameter_
    public: typedef template_parameter parameter;

    //-------------------------------------------------------------------------
    /** @brief メッセージスイートを構築する。
        @param[in] in_tag       base_type::tag_ の初期値。
        @param[in] in_call      base_type::call_ の初期値。
        @param[in] in_parameter this_type::parameter_ の初期値。
     */
    public: PSYQ_CONSTEXPR parametric(
        typename base_type::tag const& in_tag,
        typename base_type::call const& in_call,
        typename this_type::parameter in_parameter)
    PSYQ_NOEXCEPT:
        base_type(in_tag, in_call, &this->parameter_, sizeof(this_type)),
        parameter_(std::move(in_parameter))
    {}

    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元インスタンス。
     */
    public: PSYQ_CONSTEXPR parametric(this_type&& io_source) PSYQ_NOEXCEPT:
        base_type(std::move(io_source)),
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

    //-------------------------------------------------------------------------
    private: typename this_type::parameter parameter_; ///< メッセージの引数。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief メッセージスイートを保持し、仮想関数でアクセスするパケットの基底型。
    @tparam template_base_suite @copydoc psyq::any_message_packet::suite
 */
template<typename template_base_suite>
class psyq::any_message_packet
{
    private: typedef any_message_packet this_type; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
    /** @brief 保持しているメッセージスイートの基底型。

        psyq::any_message_suite に準拠している必要がある。
     */
    public: typedef template_base_suite suite;

    /// @cond
    public: template<typename template_suite> class internal;
    public: template<typename template_suite> class external;
    /// @endcond

    //-------------------------------------------------------------------------
    /// デフォルト構築子。
    protected: any_message_packet() PSYQ_NOEXCEPT {}
    /// メッセージスイートを破壊する。
    public: virtual ~any_message_packet() PSYQ_NOEXCEPT {}

    /** @brief 保持しているメッセージスイートを取得する。
        @return 保持しているメッセージスイート。
     */
    public: virtual typename this_type::suite const& get_suite()
    const PSYQ_NOEXCEPT = 0;

    /** @brief 保持しているメッセージが持つ引数のRTTIを取得する。
        @return 保持しているメッセージが持つ引数のRTTI。
     */
    public: virtual psyq::any_rtti const* get_parameter_rtti()
    const PSYQ_NOEXCEPT = 0;

    /** @brief 保持しているメッセージが持つ引数の先頭位置を取得する。
        @param[in] in_rtti キャストする型のRTTI。
        @retval !=nullptr
            メッセージの引数をRTTIが指す型へキャストしたポインタ値。
        @retval ==nullptr
            メッセージの引数をRTTIが指す型へキャストできない。
     */
    public: virtual void const* get_parameter_data(
        psyq::any_rtti const* const in_rtti)
    const PSYQ_NOEXCEPT = 0;

    /** @brief 保持しているメッセージが持つ引数の先頭位置を取得する。
        @tparam template_type キャストする型。
        @retval !=nullptr
            メッセージの引数を template_type 型へキャストしたポインタ値。
        @retval ==nullptr
            メッセージの引数を template_type 型へキャストできない。
     */
    public: template<typename template_type>
    template_type const* get_parameter() const
    {
        return static_cast<template_type const*>(
            this->get_parameter_data(psyq::any_rtti::find<template_type>()));
    }
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief プロセス内で送受信するメッセージスイートのパケット。
    @tparam template_base_suite @copydoc psyq::any_message_packet::suite
    @tparam template_suite      @copydoc psyq::any_message_packet::internal::suite
 */
template<typename template_base_suite>
template<typename template_suite>
class psyq::any_message_packet<template_base_suite>::internal:
    public psyq::any_message_packet<template_base_suite>
{
    /// thisが指す値の型。
    private: typedef internal this_type;
    /// this_type の基底型。
    public: typedef psyq::any_message_packet<template_base_suite> base_type;

    //-------------------------------------------------------------------------
    /// @copydoc this_type::suite_
    public: typedef template_suite suite;

    //-------------------------------------------------------------------------
    /** @brief メッセージパケットを構築する。
        @param[in] in_suite this_type::suite_ の初期値。
     */
    public: explicit internal(typename this_type::suite in_suite)
    PSYQ_NOEXCEPT: suite_(std::move(in_suite))
    {}

    public: typename base_type::suite const& get_suite()
    const PSYQ_NOEXCEPT override
    {
        return this->suite_;
    }

    public: psyq::any_rtti const* get_parameter_rtti()
    const PSYQ_NOEXCEPT override
    {
        return psyq::any_rtti::find<typename this_type::suite::parameter>();
    }

    public: void const* get_parameter_data(
        psyq::any_rtti const* const in_rtti)
    const PSYQ_NOEXCEPT override
    {
        return psyq::any_rtti::find(in_rtti, this->this_type::get_parameter_rtti()) != nullptr?
            this->suite_.get_parameter_data(): nullptr;
    }

    //-------------------------------------------------------------------------
    /// 保持しているメッセージスイート。
    private: typename this_type::suite suite_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief プロセス外に送受信するメッセージスイートのパケット。
    @tparam template_base_suite @copydoc psyq::any_message_packet::suite
    @tparam template_suite      @copydoc psyq::any_message_packet::external::suite
 */
template<typename template_base_suite>
template<typename template_suite>
class psyq::any_message_packet<template_base_suite>::external:
    public psyq::any_message_packet<template_base_suite>
{
    /// thisが指す値の型。
    private: typedef external this_type;
    /// this_type の基底型。
    public: typedef psyq::any_message_packet<template_base_suite> base_type;

    //-------------------------------------------------------------------------
    public: typedef template_suite suite;
};

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

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief メッセージ中継器。

    - メッセージ送受信アドレスを指定し、 psyq::any_message_router を構築する。
    - psyq::any_message_router::register_receiver() でメッセージ受信関数を登録する。
    - psyq::any_message_router::send_message() でメッセージを送信する。
    - psyq::any_message_router::flush() でメッセージ送受信の処理をすると、
      psyq::any_message_router::send_message() で送信されたメッセージに対応する、
      psyq::any_message_router::register_receiver() で登録した
      メッセージ受信器が持つ関数オブジェクトが呼び出される。

    @tparam template_base_suite @copydoc psyq::any_message_packet::suite
    @tparam template_allocator   @copydoc psyq::any_message_router::allocator_type
 */
template<typename template_base_suite, typename template_allocator>
class psyq::any_message_router
{
    private: typedef any_message_router this_type; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
    /// パケットに用いるメモリ割当子。
    public: typedef template_allocator allocator_type;
    /// メッセージ受信器。
    public: typedef psyq::any_message_receiver<template_base_suite> receiver;
    /// @copydoc psyq::any_message_receiver::packet
    public: typedef typename this_type::receiver::packet packet;
    /// メッセージの荷札と呼出状と引数をひとまとめにしたスイート。
    public: typedef typename this_type::packet::suite suite;
    /// メッセージの呼出状。
    public: typedef typename this_type::suite::call call;
    /// メッセージの荷札。
    public: typedef typename this_type::suite::tag tag;

    //-------------------------------------------------------------------------
    /// メッセージのメソッド種別からハッシュ値を生成する関数オブジェクト。
    private: struct method_hash
    {
        PSYQ_CONSTEXPR std::size_t operator()(
            typename this_type::call::key const in_method)
        const PSYQ_NOEXCEPT
        {
            return static_cast<std::size_t>(in_method);
        }
    };

    /// @copydoc receiver_map_
    private: typedef std::unordered_multimap<
        typename this_type::call::key,
        typename this_type::receiver::weak_ptr,
        typename this_type::method_hash,
        std::equal_to<typename this_type::call::key>,
        template_allocator>
            receiver_map;

    /// @copydoc provisional_list_
    private: typedef std::list<
        typename this_type::receiver_map::value_type, template_allocator>
            provisional_list;

    /// メッセージパケットの保持子。
    private: typedef std::unique_ptr<
        typename this_type::packet,
        void(*)(typename this_type::packet* const)>
            packet_unique_ptr;

    /// メッセージパケットの動的配列。
    private: typedef std::vector<
        typename this_type::packet_unique_ptr, template_allocator>
            packet_array;

    //-------------------------------------------------------------------------
    /** @brief メッセージ中継器を構築する。
        @param[in] in_address   構築する中継器のメッセージ送受信アドレス。
        @param[in] in_allocator メモリ割当子の初期値。
     */
    public: any_message_router(
        typename this_type::tag::key const in_address,
        typename this_type::allocator_type const& in_allocator)
    PSYQ_NOEXCEPT:
        receiver_map_(in_allocator),
        provisional_list_(in_allocator),
        packet_array_(in_allocator),
        address_((
            PSYQ_ASSERT(in_address != this_type::tag::EMPTY_KEY),
            in_address)),
        flushing_(false)
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
        address_(this_type::tag::EMPTY_KEY),
        flushing_(false)
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
    public: PSYQ_CONSTEXPR typename this_type::tag::key const get_address()
    const PSYQ_NOEXCEPT
    {
        return this->address_;
    }

    /** @brief このルータから送信するメッセージの荷札を構築する。
        @param[in] in_receiver_address メッセージ受信アドレス。
        @param[in] in_receiver_mask    メッセージ受信マスク。
     */
    public: PSYQ_CONSTEXPR typename this_type::tag make_receiver_tag(
        typename this_type::tag::key const in_receiver_address,
        typename this_type::tag::key const in_receiver_mask =
            ~this_type::tag::EMPTY_KEY)
    const PSYQ_NOEXCEPT
    {
        return typename this_type::tag(
            this->get_address(), in_receiver_address, in_receiver_mask);
    }

    //-------------------------------------------------------------------------
    /** @brief メッセージ受信器を登録する。

        this_type::unregister_receiver() で登録を除去できる。

        @param[in] in_method   登録するメッセージ受信メソッドの種別。
        @param[in] in_receiver 登録するメッセージ受信器。
     */
    public: void register_receiver(
        typename this_type::call::key const in_method,
        typename this_type::receiver::shared_ptr const& in_receiver)
    {
        // 仮登録する。 flush() で実際に登録される。
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
        typename this_type::call::key const in_method,
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
    /** @brief 引数を持たないメッセージを送信する。

        引数を持つメッセージを送信するには、以下の関数を使う。
        - this_type::send_internal_message()
        - this_type::send_external_message()

        @param[in] in_tag      送信するメッセージの荷札。
        @param[in] in_call     送信するメッセージの呼出状。
        @param[in] in_external プロセス外にも送信するかどうか。
     */
    public: bool send_message(
        typename this_type::tag const& in_tag,
        typename this_type::call const& in_call,
        bool const in_external)
    {
        typename this_type::suite local_suite(in_tag, in_call);
        if (in_external)
        {
            /// @todo プロセス外にメッセージを送信する処理を実装すること。
        }
        return this_type::create_internal_message(
            this->packet_array_, std::move(local_suite));
    }

    /** @brief プロセス内に、任意型の引数を持つメッセージを送信する。

        - 引数を持たないメッセージを送信するには、
          this_type::send_message() を使う。
        - 引数を持つメッセージをプロセス外にも送信するには、
          this_type::send_internal_message() を使う。

        @param[in] in_tag       送信するメッセージの荷札。
        @param[in] in_call      送信するメッセージの呼出状。
        @param[in] in_parameter 送信するメッセージの引数。
     */
    public: template<typename template_parameter>
    bool send_internal_message(
        typename this_type::tag const& in_tag,
        typename this_type::call const& in_call,
        template_parameter in_parameter)
    {
        return this_type::create_internal_message(
            this->packet_array_,
            typename this_type::suite::template
                parametric<template_parameter>(
                    in_tag, in_call, std::move(in_parameter)));
    }

    /** @brief プロセスの内と外に、POD型の引数を持つメッセージを送信する。

        - 引数を持たないメッセージを送信するには、
          this_type::send_message() を使う。
        - 引数を持つメッセージをプロセス内だけに送信するには、
          this_type::send_internal_message() を使う。

        @note 未実装。
        @param[in] in_tag       送信するメッセージの荷札。
        @param[in] in_call      送信するメッセージの呼出状。
        @param[in] in_parameter 送信するメッセージの引数。必ずPOD型。
     */
    public: template<typename template_parameter>
    bool send_external_message(
        typename this_type::tag const& in_tag,
        typename this_type::call const& in_call,
        template_parameter in_parameter);

    /** @brief メッセージを受信し、メッセージ受信関数を呼び出す。
        @param[in] in_packet 受信するメッセージパケット。
     */
    public: void receive_message(
        typename this_type::packet const& in_packet)
    {
        this_type::transmit_message(this->receiver_map_, in_packet);
    }

    //-------------------------------------------------------------------------
    /** @brief メッセージを処理する。
        @warning
            flush() している途中に flush() すると失敗する。
            flush() している途中かどうかは、 is_flushing() で判定できる。
        @retval true  成功。メッセージを処理した。
        @retval false 失敗。メッセージを処理しなかった。
     */
    public: bool flush()
    {
        if (this->is_flushing())
        {
            return false;
        }
        this->flushing_ = true;
        this_type::remove_empty_receiver(this->receiver_map_);
        this_type::merge_receiver_packet(
            this->receiver_map_, this->provisional_list_);
        this_type::transmit_message(
            this->receiver_map_, this->packet_array_);
        this->flushing_ = false;
        return true;
    }

    /** @brief メッセージの処理途中か判定する。
        @retval true  メッセージの処理途中。
        @retval false メッセージの処理途中ではない。
     */
    public: PSYQ_CONSTEXPR bool is_flushing() const PSYQ_NOEXCEPT
    {
        return this->flushing_;
    }

    //-------------------------------------------------------------------------
    /** @brief メッセージ中継器のインスタンスをムーブする。
        @param[in,out] io_source ムーブ元インスタンス。
        @retval true  成功。ムーブした。
        @retval false 失敗。ムーブしなかった。
     */
    private: bool move(this_type&& io_source)
    {
        if (this->is_flushing() || io_source.is_flushing())
        {
            return false;
        }
        io_source.flushing_ = true;
        this->flushing_ = true;
        this->receiver_map_ = std::move(io_source.receiver_map_);
        this->provisional_list_ = std::move(io_source.provisional_list_);
        this->packet_array_ = std::move(io_source.packet_array_);
        this->address_ = io_source.get_address();
        io_source.address_ = this_type::tag::EMPTY_KEY;
        this->flushing_ = false;
        io_source.flushing_ = false;
        return true;
    }

    /** @brief メッセージ受信器を辞書から検索する。
        @param[in] in_receiver_map メッセージ受信器の辞書。
        @param[in] in_method       メッセージのメソッド番号。
        @param[in] in_receiver     メッセージ受信器。
     */
    private: static typename this_type::receiver_map::const_iterator
    find_receiver_iterator(
        typename this_type::receiver_map const& in_receiver_map,
        typename this_type::call::key const in_method,
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

    /** @brief 空になったメッセージ受信器を辞書から削除する。
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
    private: static void merge_receiver_packet(
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

    /** @brief メッセージ受信器へメッセージを中継する。
        @param[in]     in_receiver_map メッセージ受信器の辞書。
        @param[in,out] io_packet_array 中継するメッセージのパケット。
     */
    private: static void transmit_message(
        typename this_type::receiver_map const& in_receiver_map,
        typename this_type::packet_array& io_packet_array)
    {
        // メッセージのパケットを走査し、メッセージ受信器の辞書へ中継する。
        /** @note
            io_packet_array の持つメモリをこの関数で破棄するのはもったいない。
            ダブルバッファ式にして、
            パケットが持つメモリを使いまわすなどの実装をしたい。
         */
        auto const local_packet_array(std::move(io_packet_array));
        io_packet_array.clear();
        io_packet_array.reserve(local_packet_array.size());
        for (auto& local_message_holder: local_packet_array)
        {
            auto const local_message_packet(local_message_holder.get());
            if (local_message_packet != nullptr)
            {
                this_type::transmit_message(
                    in_receiver_map, *local_message_packet);
            }
            else
            {
                PSYQ_ASSERT(false);
            }
        }
    }

    /** @brief メッセージ受信器へメッセージを中継する。
        @param[in] in_receiver_map メッセージ受信器の辞書。
        @param[in] in_packet      中継するメッセージ。
     */
    private: static void transmit_message(
        typename this_type::receiver_map const& in_receiver_map,
        typename this_type::packet const& in_packet)
    {
        // メッセージ受信器の辞書のうち、
        // メソッド番号が一致するものへメッセージを中継する。
        auto& local_suite(in_packet.get_suite());
        auto const local_method(local_suite.get_call().get_method());
        for (
            auto i(in_receiver_map.find(local_method));
            i != in_receiver_map.end() && i->first == local_method;
            ++i)
        {
            // メッセージ受信アドレスとメッセージ受信器が合致するか判定する。
            auto const local_holder(i->second.lock());
            auto const local_receiver(local_holder.get());
            if (local_receiver != nullptr
                && local_suite.get_tag().agree_receiver_address(local_receiver->get_address()))
            {
                // メッセージ受信関数を呼び出す。
                auto& local_functor(local_receiver->get_functor());
                if (bool(local_functor))
                {
                    local_functor(in_packet);
                }
            }
        }
    }

    /** @brief プロセス内メッセージのパケットを構築し、配列に追加する。
        @param[in,out] io_packet_array 構築したパケットを追加する配列。
        @param[in,out] io_suite        パケット化するメッセージスイート。
     */
    private: template<typename template_suite>
    static bool create_internal_message(
        typename this_type::packet_array& io_packet_array,
        template_suite&& io_suite)
    {
        typedef typename this_type::packet::template
            internal<template_suite>
                internal_message;
        typename this_type::allocator_type::template
            rebind<internal_message>::other
                local_allocator;
        auto const local_storage(local_allocator.allocate(1));
        if (local_storage == nullptr)
        {
            PSYQ_ASSERT(false);
            return false;
        }
        io_packet_array.emplace_back(
            typename this_type::packet_unique_ptr(
                new(local_storage) internal_message(std::move(io_suite)),
                this_type::destroy_message<internal_message>));
        return true;
    }

    /** @brief メッセージを破棄する。
        @param[in] in_packet 破棄するメッセージ。
     */
    private: template<typename template_message>
    static void destroy_message(typename this_type::packet* const in_packet)
    {
        if (in_packet != nullptr)
        {
            typename this_type::allocator_type::template
                rebind<template_message>::other
                    local_allocator;
            local_allocator.destroy(in_packet);
            local_allocator.deallocate(
                static_cast<template_message*>(in_packet), 1);
        }
    }

    //-------------------------------------------------------------------------
    /// メッセージ受信器の辞書。
    private: typename this_type::receiver_map receiver_map_;
    /// 仮登録したメッセージ受信器のリスト。
    private: typename this_type::provisional_list provisional_list_;
    /// 中継するメッセージパケットの配列。
    private: typename this_type::packet_array packet_array_;
    /// この中継器のメッセージ送受信アドレス。
    private: typename this_type::tag::key address_;
    /// メッセージ中継の途中かどうか。
    private: bool flushing_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    namespace test
    {
        inline void any_message()
        {
            psyq::any_rtti::make<psyq::test::floating_wrapper>();

            typedef psyq::any_message_router<> message_router;
            message_router local_router(
                0x7f000001, message_router::allocator_type());
            enum: message_router::call::key
            {
                METHOD_PARAMETER_VOID = 1,
                METHOD_PARAMETER_DOUBLE,
            };
            message_router::receiver::shared_ptr const local_method_a(
                new message_router::receiver(
                    [=](message_router::packet const& in_packet)
                    {
                        PSYQ_ASSERT(
                            in_packet.get_parameter_rtti()
                            == psyq::any_rtti::find<void>());
                    },
                    local_router.get_address()));
            double const local_double(0.5);
            message_router::receiver::shared_ptr const local_method_b(
                new message_router::receiver(
                    [local_double]
                    (message_router::packet const& in_packet)
                    {
                        auto const local_parameter(
                            in_packet.get_parameter<psyq::test::floating_wrapper>());
                        if (local_parameter != nullptr)
                        {
                            PSYQ_ASSERT(local_parameter->value == local_double);
                        }
                        else
                        {
                            PSYQ_ASSERT(false);
                        }
                    },
                    local_router.get_address()));
            local_router.register_receiver(METHOD_PARAMETER_VOID, local_method_a);
            local_router.register_receiver(METHOD_PARAMETER_DOUBLE, local_method_b);
            local_router.send_message(
                local_router.make_receiver_tag(local_router.get_address()),
                message_router::call(METHOD_PARAMETER_VOID),
                false);
            local_router.send_internal_message(
                local_router.make_receiver_tag(local_router.get_address()),
                message_router::call(METHOD_PARAMETER_DOUBLE),
                floating_wrapper(0.5));
            local_router.flush();
            local_router.unregister_receiver(0, nullptr);
            local_router.unregister_receiver(nullptr);
        }
    }
}

#endif // !defined(PSYQ_ANY_MESSAGE_HPP_)
