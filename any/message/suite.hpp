/// @file
/// @author Hillco Psychi (https://twitter.com/psychi)

#ifndef PSYQ_ANY_MESSAGE_SUITE_HPP_
#define PSYQ_ANY_MESSAGE_SUITE_HPP_

#include "../../assert.hpp"
#include "./tag.hpp"
#include "./call.hpp"

/// @cond
namespace psyq
{
    namespace any
    {
        namespace message
        {
            template<typename> class invoice;
            template<typename, typename> class suite;
        } // namespace message
    } // namespace any
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief RPCメッセージの送り状。
/// @tparam template_key @copydoc psyq::any::message::invoice::key_type
template<typename template_key>
class psyq::any::message::invoice
{
    /// @copydoc psyq::string::view::this_type
    private: typedef invoice this_type;

    //-------------------------------------------------------------------------
    /// @brief メッセージ送り状が使う識別値を表す型。
    public: typedef template_key key_type;
    static_assert(
        std::is_integral<template_key>::value,
        "'template_key' is not integral type.");

    //-------------------------------------------------------------------------
    /// @brief メッセージの送り状を構築する。
    public: PSYQ_CONSTEXPR invoice(
        /// [in] メッセージ送信オブジェクトの識別値。
        typename this_type::key_type const in_sender_key,
        /// [in] メッセージ受信オブジェクトの識別値。
        typename this_type::key_type const in_receiver_key,
        /// [in] メッセージ受信オブジェクトの識別値マスク。
        typename this_type::key_type const in_receiver_mask,
        /// [in] メッセージ受信関数の識別値。
        typename this_type::key_type const in_selector_key)
    PSYQ_NOEXCEPT:
    sender_key_(in_sender_key),
    zone_key_(0),
    zone_mask_(0),
    dispatcher_key_(0),
    dispatcher_mask_(0),
    receiver_key_(in_receiver_key),
    receiver_mask_(in_receiver_mask),
    selector_key_(in_selector_key)
    {}

    /// @brief メッセージ送信オブジェクトの識別値を取得する。
    /// @return @copydoc sender_key_
    public: PSYQ_CONSTEXPR typename this_type::key_type get_sender_key()
    const PSYQ_NOEXCEPT
    {
        return this->sender_key_;
    }

    /// @brief メッセージ受信オブジェクトの識別値を取得する。
    /// @return @copydoc receiver_key_
    public: PSYQ_CONSTEXPR typename this_type::key_type get_receiver_key()
    const PSYQ_NOEXCEPT
    {
        return this->receiver_key_;
    }

    /// @brief メッセージ受信オブジェクトの識別値マスクを取得する。
    /// @return receiver_mask_
    public: PSYQ_CONSTEXPR typename this_type::key_type get_receiver_mask()
    const PSYQ_NOEXCEPT
    {
        return this->receiver_mask_;
    }

    /// @brief メッセージ受信関数の識別値を取得する。
    /// @return @copydoc receiver_key_
    public: PSYQ_CONSTEXPR typename this_type::key_type get_selector_key()
    const PSYQ_NOEXCEPT
    {
        return this->selector_key_;
    }

    /// @brief *this に合致するメッセージ受信オブジェクトか判定する。
    /// @retval true  合致する。
    /// @retval false 合致しない。
    public: PSYQ_CONSTEXPR bool verify_receiver_key(
        /// [in] 判定するメッセージ受信オブジェクトの識別値。
        typename this_type::key_type const in_key)
    const PSYQ_NOEXCEPT
    {
        return this_type::verify_key(
            in_key, this->get_receiver_key(), this->get_receiver_mask());
    }

    //-------------------------------------------------------------------------
    /// @brief 合致するメッセージ受信オブジェクトか判定する。
    /// @retval true  合致する。
    /// @retval false 合致しない。
    private: static PSYQ_CONSTEXPR bool verify_key(
        /// [in] 判定するメッセージ受信オブジェクトの識別値。
        typename this_type::key_type const in_key,
        /// [in] 対象となるメッセージ受信オブジェクトの識別値。
        typename this_type::key_type const in_target_key,
        /// [in] 対象となるメッセージ受信オブジェクトの識別値マスク。
        typename this_type::key_type const in_target_mask)
    PSYQ_NOEXCEPT
    {
        return (in_key & in_target_mask) == in_target_key;
    }

    //-------------------------------------------------------------------------
    /// @brief メッセージ送信オブジェクトの識別値。
    private: typename this_type::key_type sender_key_;
    /// @brief メッセージを中継するゾーンの識別値。
    private: typename this_type::key_type zone_key_;
    /// @brief メッセージを中継するゾーンの識別値マスク。
    private: typename this_type::key_type zone_mask_;
    /// @brief メッセージを中継する分配器の識別値。
    private: typename this_type::key_type dispatcher_key_;
    /// @brief メッセージを中継する分配器の識別値マスク。
    private: typename this_type::key_type dispatcher_mask_;
    /// @brief メッセージ受信オブジェクトの識別値。
    private: typename this_type::key_type receiver_key_;
    /// @brief メッセージ受信オブジェクトの識別値マスク。
    private: typename this_type::key_type receiver_mask_;
    /// @brief メッセージ受信関数の識別値。
    private: typename this_type::key_type selector_key_;

}; // class psyq::any::message::invoice

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 引数のないRPCメッセージ。
/// @details 任意型の引数を含めるには this_type::parametric を使う。
/// @tparam template_key  @copydoc invoice::key_type
/// @tparam template_size @copydoc suite::size_type
template<typename template_key, typename template_size>
class psyq::any::message::suite
{
    /// @copydoc psyq::string::view::this_type
    private: typedef suite this_type;

    //-------------------------------------------------------------------------
    /// @copydoc this_type::tag_
    public: typedef psyq::any::message::tag<template_key> tag;
    /// @copydoc this_type::call_
    public: typedef psyq::any::message::call<template_key> call;

    //-------------------------------------------------------------------------
    public: template<typename template_parameter> class parametric;
    /// @brief メッセージ引数の型。メッセージ引数を持たないのでvoid型。
    public: typedef void parameter;
    /// @brief メッセージの送り状。
    public: typedef psyq::any::message::invoice<template_key> invoice;
    /// @brief バイト数を表す型。
    public: typedef template_size size_type;
    static_assert(
        std::is_unsigned<template_size>::value,
        "'template_size' is not unsigned type.");

    //-------------------------------------------------------------------------
    /// @name 構築
    /// @{

    /** @brief this_type を構築する。
        @param[in] in_tag  this_type::tag_ の初期値。
        @param[in] in_call this_type::call_ の初期値。
     */
    public: PSYQ_CONSTEXPR suite(
        typename this_type::tag const& in_tag,
        typename this_type::call const& in_call)
    PSYQ_NOEXCEPT:
    tag_(in_tag),
    call_(in_call),
    invoice_(0, 0, 0, 0),
    parameter_offset_(sizeof(this_type)),
    parameter_size_(0)
    {}

    /// @brief 引数のないメッセージを構築する。
    public: explicit suite(
        /// [in] メッセージの送り状。
        typename this_type::invoice const& in_invoice)
    PSYQ_NOEXCEPT:
    tag_(0, 0, 0),
    call_(0),
    invoice_(in_invoice),
    parameter_offset_(sizeof(this_type)),
    parameter_size_(0)
    {}
    /// @}
    //-------------------------------------------------------------------------
    /// @name メッセージのプロパティ
    /// @{

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

    /// @brief メッセージの送り状を取得する。
    /// @return メッセージの送り状。
    public: PSYQ_CONSTEXPR typename this_type::invoice const& get_invoice()
    const PSYQ_NOEXCEPT
    {
        return this->invoice_;
    }

    /// @brief メッセージ引数の先頭位置を取得する。
    /// @return メッセージ引数の先頭位置。
    public: PSYQ_CONSTEXPR void const* get_parameter_data() const PSYQ_NOEXCEPT
    {
        return reinterpret_cast<char const*>(this) + this->parameter_offset_;
    }

    /// @brief メッセージ引数のバイトサイズを取得する。
    /// @return メッセージ引数のバイトサイズ。
    public: PSYQ_CONSTEXPR typename this_type::size_type get_parameter_size()
    const PSYQ_NOEXCEPT
    {
        return this->parameter_size_;
    }
    /// @}
    //-------------------------------------------------------------------------
    /** @brief 引数を持つ this_type を構築する。
        @param[in] in_tag         this_type::tag_ の初期値。
        @param[in] in_call        this_type::call_ の初期値。
        @param[in] in_parameter   引数の先頭位置。
        @param[in] in_suite_size メッセージ一式全体のバイトサイズ。
     */
    protected: PSYQ_CONSTEXPR suite(
        typename this_type::tag const& in_tag,
        typename this_type::call const& in_call,
        void const* const in_parameter,
        std::size_t const in_suite_size)
    PSYQ_NOEXCEPT:
        tag_(in_tag),
        call_(in_call),
        invoice_(0, 0, 0, 0),
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

    /// @brief 引数を持つメッセージを構築する。
    protected: PSYQ_CONSTEXPR suite(
        /// [in] メッセージの送り状。
        typename this_type::invoice const& in_invoice,
        /// [in] 引数の先頭位置。
        void const* const in_parameter,
        /// [in] メッセージ全体のバイトサイズ。
        std::size_t const in_suite_size)
    PSYQ_NOEXCEPT:
    tag_(0, 0, 0),
    call_(0),
    invoice_(in_invoice),
    parameter_offset_((
        PSYQ_ASSERT(in_parameter != nullptr),
        std::distance(
            reinterpret_cast<char const*>(this),
            static_cast<char const*>(in_parameter)))),
    parameter_size_((
        PSYQ_ASSERT(
            in_suite_size
            <= (std::numeric_limits<typename this_type::size_type>::max)()),
        PSYQ_ASSERT(this->parameter_offset_ <= in_suite_size),
        static_cast<typename this_type::size_type>(
            in_suite_size - this->parameter_offset_)))
    {}

    //-------------------------------------------------------------------------
    /// メッセージの荷札。
    private: typename this_type::tag tag_;
    /// メッセージの呼出状。
    private: typename this_type::call call_;

    /// @brief メッセージの送り状。
    private: typename this_type::invoice invoice_;
    /// @brief メッセージ引数の先頭位置へのオフセットバイト数。
    private: typename this_type::size_type parameter_offset_;
    /// @brief メッセージ引数のバイトサイズ。
    private: typename this_type::size_type parameter_size_;

}; // class psyq::any::message::suite

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 送り状と引数を一揃いにしたRPCメッセージ。
/// @tparam template_key       @copydoc invoice::key_type
/// @tparam template_size      @copydoc suite::size_type
/// @tparam template_parameter @copydoc suite::parametric::parameter
template<typename template_key, typename template_size>
template<typename template_parameter>
class psyq::any::message::suite<template_key, template_size>::parametric:
public psyq::any::message::suite<template_key, template_size>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef parametric this_type;
    /// @copydoc psyq::string::view::base_type
    public:
        typedef psyq::any::message::suite<template_key, template_size>
        base_type;

    //-------------------------------------------------------------------------
    /// @brief メッセージの引数の型。
    public: typedef template_parameter parameter;

    //-------------------------------------------------------------------------
    /// @name 構築と代入
    /// @{

    /** @brief this_type を構築する。
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

    /// @brief メッセージを構築する。
    public: PSYQ_CONSTEXPR parametric(
        /// [in] メッセージの送り状。
        typename base_type::invoice const& in_invoice,
        /// [in] メッセージの引数。
        typename this_type::parameter in_parameter)
    PSYQ_NOEXCEPT:
    base_type(in_invoice, &this->parameter_, sizeof(this_type)),
    parameter_(std::move(in_parameter))
    {}

#ifdef PSYQ_NO_STD_DEFAULTED_FUNCTION
    /// @brief メッセージをムーブ構築する。
    public: PSYQ_CONSTEXPR parametric(
        /// [in,out] ムーブ元となるメッセージ。
        this_type&& io_source)
    PSYQ_NOEXCEPT:
    base_type(std::move(io_source)),
    parameter_(std::move(io_source.parameter_))
    {}

    /// @brief メッセージをムーブ代入する。
    /// @return *this
    public: this_type& operator=(
        /// [in] ムーブ元となるメッセージ。
        this_type&& io_source)
    PSYQ_NOEXCEPT
    {
        if (this != &io_source)
        {
            static_cast<base_type&>(*this) = std::move(io_source);
            this->parameter_ = std::move(io_source.parameter_);
        }
        return *this;
    }
#endif // defined(PSYQ_NO_STD_DEFAULTED_FUNCTION)
    /// @}
    //-------------------------------------------------------------------------
    /// @name メッセージのプロパティ
    /// @{

    /// @brief メッセージの引数を取得する。
    /// @return メッセージの引数。
    public: PSYQ_CONSTEXPR typename this_type::parameter const& get_parameter()
    const PSYQ_NOEXCEPT
    {
        return this->parameter_;
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @brief メッセージの引数。
    private: typename this_type::parameter parameter_;

}; // class psyq::any::message::suite::parametric

#endif // !defined(PSYQ_ANY_MESSAGE_SUITE_HPP_)
// vim: set expandtab:
