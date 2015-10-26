/// @file
/// @author Hillco Psychi (https://twitter.com/psychi)
#ifndef PSYQ_EVENT_DRIVEN_PACKET_HPP_
#define PSYQ_EVENT_DRIVEN_PACKET_HPP_

#include <memory>
#include "../any/rtti.hpp"

/// @cond
namespace psyq
{
    namespace event_driven
    {
        template<typename> class packet;
    } // namespace event_driven
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 送受信するメッセージを内包するパケットの基底型。
/// @tparam template_base_message @copydoc packet::message
template<typename template_base_message>
class psyq::event_driven::packet
{
    /// @copydoc psyq::string::view::this_type
    private: typedef packet this_type;

    //-------------------------------------------------------------------------
    public: template<typename template_message> class zonal;
    public: template<typename template_message> class external;
    /// @brief this_type を強参照するスマートポインタ。
    public: typedef std::shared_ptr<this_type> shared_ptr;
    /// @brief this_type を弱参照するスマートポインタ。
    public: typedef std::weak_ptr<this_type> weak_ptr;
    /// @brief 送受信するメッセージの基底型。
    /// @details event_driven::message 互換であること。
    public: typedef template_base_message message;

    //-------------------------------------------------------------------------
    /// @name 解体
    /// @{

    /// @brief メッセージを解体する。
    public: virtual ~packet() PSYQ_NOEXCEPT {}
    /// @}
    //-------------------------------------------------------------------------
    /// @name パケットのプロパティ
    /// @{

    /// @brief 保持しているメッセージを取得する。
    /// @return 保持しているメッセージ。
    public: virtual typename this_type::message const& get_message()
    const PSYQ_NOEXCEPT = 0;

    /// @brief 外部に送信可能なメッセージを取得する。
    /// @return 外部に送信可能なメッセージ。
    public: virtual typename this_type::message const* get_external_message()
    const PSYQ_NOEXCEPT = 0;

    /// @brief 保持しているメッセージが持つ引数のRTTIを取得する。
    /// @return 保持しているメッセージが持つ引数のRTTI。
    public: virtual psyq::any::rtti const* get_parameter_rtti()
    const PSYQ_NOEXCEPT = 0;

    /// @brief 保持しているメッセージが持つ引数の先頭位置を取得する。
    /// @retval nullptr以外 メッセージの引数をRTTIが指す型へキャストしたポインタ値。
    /// @retval nullptr     メッセージの引数をRTTIが指す型へキャストできない。
    public: virtual void const* get_parameter_data(
        /// [in] キャストする型のRTTI。
        psyq::any::rtti const* const in_rtti)
    const PSYQ_NOEXCEPT = 0;

    /// @brief 保持しているメッセージが持つ引数の先頭位置を取得する。
    /// @retval nullptr以外 メッセージの引数を template_type 型へキャストしたポインタ値。
    /// @retval nullptr     メッセージの引数を template_type 型へキャストできない。
    /// @tparam template_type キャストする型。
    public: template<typename template_type>
    template_type const* get_parameter() const
    {
        return static_cast<template_type const*>(
            this->get_parameter_data(psyq::any::rtti::find<template_type>()));
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @brief メッセージゾーン内パケットを生成する。
    /// @return
    ///   生成したメッセージパケットを指すスマートポインタ。
    ///   生成に失敗した場合は、スマートポインタは空となる。
    public: template<typename template_message, typename template_allocator>
    static typename this_type::shared_ptr create_zonal(
        /// [in,out] パケットに設定するメッセージ。
        template_message&& io_message,
        /// [in] 使用するメモリ割当子。
        template_allocator const& in_allocator)
    {
        return std::allocate_shared<
            typename this_type::zonal<template_message>>(
                in_allocator, std::move(io_message));
    }

    /// @brief メッセージゾーン外パケットを生成する。
    /// @return
    ///   生成したメッセージパケットを指すスマートポインタ。
    ///   生成に失敗した場合は、スマートポインタは空となる。
    public: template<typename template_message, typename template_allocator>
    static typename this_type::shared_ptr create_external(
        /// [in,out] パケットに設定するメッセージ。
        /// template_message::parameter が、void型かPOD型であること。
        template_message&& io_message,
        /// [in] 使用するメモリ割当子。
        template_allocator const& in_allocator)
    {
        return std::allocate_shared<
            typename this_type::external<template_message>>(
                in_allocator, std::move(io_message));
    }

    //-------------------------------------------------------------------------
    protected: packet() PSYQ_NOEXCEPT {}

}; // class psyq::event_driven::packet

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief メッセージゾーン内で送受信するメッセージを持つパケット。
/// @tparam template_base_message @copydoc packet::message
/// @tparam template_message      @copydoc packet::zonal::message
template<typename template_base_message>
template<typename template_message>
class psyq::event_driven::packet<template_base_message>::zonal:
public psyq::event_driven::packet<template_base_message>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef zonal this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef psyq::event_driven::packet<template_base_message> base_type;

    //-------------------------------------------------------------------------
    /// @copydoc this_type::message_
    public: typedef template_message message;

    //-------------------------------------------------------------------------
    /// @name 構築
    /// @{

    /// @brief メッセージパケットを構築する。
    public: explicit zonal(
        /// [in] this_type::message_ の初期値。
        typename this_type::message&& io_message)
    PSYQ_NOEXCEPT: message_(std::move(io_message))
    {}
    /// @}
    //-------------------------------------------------------------------------
    /// @name プロパティ
    /// @{
    public: typename base_type::message const& get_message()
    const PSYQ_NOEXCEPT override
    {
        return this->message_;
    }

    public: typename this_type::message const* get_external_message()
    const PSYQ_NOEXCEPT override
    {
        return nullptr;
    }

    public: psyq::any::rtti const* get_parameter_rtti()
    const PSYQ_NOEXCEPT override
    {
        return psyq::any::rtti::find<typename this_type::message::parameter>();
    }

    public: void const* get_parameter_data(
        psyq::any::rtti const* const in_rtti)
    const PSYQ_NOEXCEPT override
    {
        return psyq::any::rtti::find(in_rtti, this->this_type::get_parameter_rtti()) != nullptr?
            this->message_.get_parameter_data(): nullptr;
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @brief 保持しているメッセージ。
    private: typename this_type::message message_;

}; // class psyq::event_driven::packet::zonal

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief メッセージゾーン外に送受信するメッセージを持つパケット。
/// @tparam template_base_message @copydoc packet::message
/// @tparam template_message      @copydoc packet::external::message
/// @todo 今のところ仮実装。
template<typename template_base_message>
template<typename template_message>
class psyq::event_driven::packet<template_base_message>::external:
    public psyq::event_driven::packet<template_base_message>
{
    /// @copydoc psyq::string::view::this_type
    private: typedef external this_type;
    /// @copydoc psyq::string::view::base_type
    public: typedef psyq::event_driven::packet<template_base_message> base_type;

    //-------------------------------------------------------------------------
    /// @brief 送受信するメッセージの型。
    /// @details template_message::parameter が、void型かPOD型であること。
    public: typedef template_message message;
    static_assert(
        std::is_void<typename template_message::parameter>::value
        || std::is_pod<typename template_message::parameter>::value,
        "template_message::parameter is not POD type.");

    //-------------------------------------------------------------------------
    /// @name 構築
    /// @{

    /// @brief this_type を構築する。
    public: explicit external(
        /// [in] this_type::message_ の初期値。
        typename this_type::message&& io_message)
    PSYQ_NOEXCEPT: message_(std::move(io_message))
    {}
    /// @}
    //-------------------------------------------------------------------------
    /// @name this_type のプロパティ
    /// @{
    public: typename base_type::message const& get_message()
    const PSYQ_NOEXCEPT override
    {
        return this->message_;
    }

    public: typename this_type::message const* get_external_message()
    const PSYQ_NOEXCEPT override
    {
        return &this->message_;
    }

    public: psyq::any::rtti const* get_parameter_rtti()
    const PSYQ_NOEXCEPT override
    {
        return psyq::any::rtti::find<typename this_type::message::parameter>();
    }

    public: void const* get_parameter_data(
        psyq::any::rtti const* const in_rtti)
    const PSYQ_NOEXCEPT override
    {
        return psyq::any::rtti::find(in_rtti, this->this_type::get_parameter_rtti()) != nullptr?
            this->message_.get_parameter_data(): nullptr;
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @brief 保持しているメッセージ。
    private: typename this_type::message message_;

}; // class psyq::event_driven::packet::external

#endif // !defined(PSYQ_EVENT_DRIVEN_PACKET_HPP_)
// vim: set expandtab:
