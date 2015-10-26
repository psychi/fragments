/// @file
/// @author Hillco Psychi (https://twitter.com/psychi)

#ifndef PSYQ_EVENT_DRIVEN_MESSAGE_HPP_
#define PSYQ_EVENT_DRIVEN_MESSAGE_HPP_

#include "../assert.hpp"

#ifndef PSYQ_EVENT_DRIVEN_MESSAGE_TAG_RECEIVER_MASK_DEFAULT
#define PSYQ_EVENT_DRIVEN_MESSAGE_TAG_RECEIVER_MASK_DEFAULT (~0)
#endif // !defined(PSYQ_EVENT_DRIVEN_MESSAGE_TAG_RECEIVER_MASK_DEFAULT)

/// @cond
namespace psyq
{
    namespace event_driven
    {
        template<typename> class tag;
        template<typename, typename> class message;
    } // namespace event_driven
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief メッセージの送り状。
/// @tparam template_key @copydoc psyq::event_driven::tag::key_type
template<typename template_key>
class psyq::event_driven::tag
{
    /// @copydoc psyq::string::view::this_type
    private: typedef tag this_type;

    //-------------------------------------------------------------------------
    /// @brief メッセージ送り状が使う識別値を表す型。
    public: typedef template_key key_type;
    static_assert(
        std::is_integral<template_key>::value,
        "'template_key' is not integral type.");

    //-------------------------------------------------------------------------
    /// @brief メッセージの送り状を構築する。
    public: PSYQ_CONSTEXPR tag(
        /// [in] メッセージ送信オブジェクトの識別値。
        typename this_type::key_type const in_sender_key,
        /// [in] メッセージ受信オブジェクトの識別値。
        typename this_type::key_type const in_receiver_key,
        /// [in] メッセージ受信関数の識別値。
        typename this_type::key_type const in_selector_key,
        /// [in] メッセージ受信オブジェクトの識別値マスク。
        typename this_type::key_type const in_receiver_mask
            = PSYQ_EVENT_DRIVEN_MESSAGE_TAG_RECEIVER_MASK_DEFAULT)
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

}; // class psyq::event_driven::tag

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 引数のないメッセージ。
/// @details 任意型の引数を含めるには this_type::parametric を使う。
/// @tparam template_key  @copydoc tag::key_type
/// @tparam template_size @copydoc message::size_type
template<typename template_key, typename template_size>
class psyq::event_driven::message
{
    /// @copydoc psyq::string::view::this_type
    private: typedef message this_type;

    //-------------------------------------------------------------------------
    public: template<typename template_parameter> class parametric;
    /// @brief メッセージ引数の型。メッセージ引数を持たないのでvoid型。
    public: typedef void parameter;
    /// @brief メッセージの送り状。
    public: typedef psyq::event_driven::tag<template_key> tag;
    /// @brief バイト数を表す型。
    public: typedef template_size size_type;
    static_assert(
        std::is_unsigned<template_size>::value,
        "'template_size' is not unsigned type.");

    //-------------------------------------------------------------------------
    /// @name 構築
    /// @{

    /// @brief 引数のないメッセージを構築する。
    public: explicit message(
        /// [in] メッセージの送り状。
        typename this_type::tag const& in_tag)
    PSYQ_NOEXCEPT:
    tag_(in_tag),
    parameter_offset_(sizeof(this_type)),
    parameter_size_(0)
    {}
    /// @}

    /// @brief 引数を持つメッセージを構築する。
    /// @return 構築したメッセージ。
    public: template<typename template_parameter>
    static typename this_type::parametric<template_parameter> construct(
        /// [in] メッセージの送り状。
        typename this_type::tag const& in_tag,
        /// [in] メッセージの引数。
        template_parameter&& io_parameter)
    {
        return typename this_type::parametric<template_parameter>(
            in_tag, std::move(io_parameter));
    }

    //-------------------------------------------------------------------------
    /// @name メッセージのプロパティ
    /// @{

    /// @brief メッセージの送り状を取得する。
    /// @return メッセージの送り状。
    public: PSYQ_CONSTEXPR typename this_type::tag const& get_tag()
    const PSYQ_NOEXCEPT
    {
        return this->tag_;
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
    /// @brief 引数を持つメッセージを構築する。
    protected: PSYQ_CONSTEXPR message(
        /// [in] メッセージの送り状。
        typename this_type::tag const& in_tag,
        /// [in] 引数の先頭位置。
        void const* const in_parameter,
        /// [in] メッセージ全体のバイトサイズ。
        std::size_t const in_message_size)
    PSYQ_NOEXCEPT:
    tag_(in_tag),
    parameter_offset_((
        PSYQ_ASSERT(in_parameter != nullptr),
        std::distance(
            reinterpret_cast<char const*>(this),
            static_cast<char const*>(in_parameter)))),
    parameter_size_((
        PSYQ_ASSERT(
            in_message_size
            <= (std::numeric_limits<typename this_type::size_type>::max)()),
        PSYQ_ASSERT(this->parameter_offset_ <= in_message_size),
        static_cast<typename this_type::size_type>(
            in_message_size - this->parameter_offset_)))
    {}

    //-------------------------------------------------------------------------
    /// @brief メッセージの送り状。
    private: typename this_type::tag tag_;
    /// @brief メッセージ引数の先頭位置へのオフセットバイト数。
    private: typename this_type::size_type parameter_offset_;
    /// @brief メッセージ引数のバイトサイズ。
    private: typename this_type::size_type parameter_size_;

}; // class psyq::event_driven::message

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 引数を持つメッセージ。
/// @tparam template_key       @copydoc tag::key_type
/// @tparam template_size      @copydoc message::size_type
/// @tparam template_parameter @copydoc message::parametric::parameter
template<typename template_key, typename template_size>
template<typename template_parameter>
class psyq::event_driven::message<template_key, template_size>::parametric:
public psyq::event_driven::message<template_key, template_size>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef parametric this_type;
    /// @copydoc psyq::string::view::base_type
    public:
        typedef psyq::event_driven::message<template_key, template_size>
        base_type;

    //-------------------------------------------------------------------------
    /// @brief メッセージの引数の型。
    public: typedef template_parameter parameter;

    //-------------------------------------------------------------------------
    /// @name 構築と代入
    /// @{

    /// @brief メッセージを構築する。
    public: PSYQ_CONSTEXPR parametric(
        /// [in] メッセージの送り状。
        typename base_type::tag const& in_tag,
        /// [in] メッセージの引数。
        typename this_type::parameter&& io_parameter)
    PSYQ_NOEXCEPT:
    base_type(in_tag, &this->parameter_, sizeof(this_type)),
    parameter_(std::move(io_parameter))
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

}; // class psyq::event_driven::message::parametric

#endif // !defined(PSYQ_EVENT_DRIVEN_MESSAGE_HPP_)
// vim: set expandtab:
