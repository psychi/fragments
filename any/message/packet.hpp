/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_ANY_MESSAGE_PACKET_HPP_
#define PSYQ_ANY_MESSAGE_PACKET_HPP_

//#include "psyq/any/message/suite.hpp"

/// @cond
namespace psyq
{
    namespace any
    {
        namespace message
        {
            template<typename> class packet;
        } // namespace message
    } // namespace any
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief メッセージスイートを保持し、仮想関数でアクセスするパケットの基底型。
    @tparam template_base_suite @copydoc psyq::any::message::packet::suite
 */
template<typename template_base_suite>
class psyq::any::message::packet
{
    private: typedef packet this_type; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
    /// this_type の保持子。
    public: typedef std::shared_ptr<this_type> shared_ptr;
    /// this_type の監視子。
    public: typedef std::weak_ptr<this_type> weak_ptr;

    /** @brief 保持しているメッセージスイートの基底型。

        psyq::any::message::suite に準拠している必要がある。
     */
    public: typedef template_base_suite suite;

    /// @cond
    public: template<typename template_suite> class internal;
    public: template<typename template_suite> class external;
    /// @endcond

    //-------------------------------------------------------------------------
    /// デフォルト構築子。
    protected: packet() PSYQ_NOEXCEPT {}
    /// メッセージスイートを破壊する。
    public: virtual ~packet() PSYQ_NOEXCEPT {}

    /** @brief 保持しているメッセージスイートを取得する。
        @return 保持しているメッセージスイート。
     */
    public: virtual typename this_type::suite const& get_suite()
    const PSYQ_NOEXCEPT = 0;

    /** @brief 外部に送信可能なメッセージスイートを取得する。
        @return 外部に送信可能なメッセージスイート。
     */
    public: virtual typename this_type::suite const* get_external_suite()
    const PSYQ_NOEXCEPT = 0;

    /** @brief 保持しているメッセージが持つ引数のRTTIを取得する。
        @return 保持しているメッセージが持つ引数のRTTI。
     */
    public: virtual psyq::any::rtti const* get_parameter_rtti()
    const PSYQ_NOEXCEPT = 0;

    /** @brief 保持しているメッセージが持つ引数の先頭位置を取得する。
        @param[in] in_rtti キャストする型のRTTI。
        @retval !=nullptr
            メッセージの引数をRTTIが指す型へキャストしたポインタ値。
        @retval ==nullptr
            メッセージの引数をRTTIが指す型へキャストできない。
     */
    public: virtual void const* get_parameter_data(
        psyq::any::rtti const* const in_rtti)
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
            this->get_parameter_data(psyq::any::rtti::find<template_type>()));
    }

}; // class psyq::any::message::packet

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief プロセス内で送受信するメッセージスイートのパケット。
    @tparam template_base_suite @copydoc psyq::any::message::packet::suite
    @tparam template_suite      @copydoc psyq::any::message::packet::internal::suite
 */
template<typename template_base_suite>
template<typename template_suite>
class psyq::any::message::packet<template_base_suite>::internal:
    public psyq::any::message::packet<template_base_suite>
{
    /// thisが指す値の型。
    private: typedef internal this_type;
    /// this_type の基底型。
    public: typedef psyq::any::message::packet<template_base_suite> base_type;

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

    public: typename this_type::suite const* get_external_suite()
    const PSYQ_NOEXCEPT override
    {
        return nullptr;
    }

    public: psyq::any::rtti const* get_parameter_rtti()
    const PSYQ_NOEXCEPT override
    {
        return psyq::any::rtti::find<typename this_type::suite::parameter>();
    }

    public: void const* get_parameter_data(
        psyq::any::rtti const* const in_rtti)
    const PSYQ_NOEXCEPT override
    {
        return psyq::any::rtti::find(in_rtti, this->this_type::get_parameter_rtti()) != nullptr?
            this->suite_.get_parameter_data(): nullptr;
    }

    //-------------------------------------------------------------------------
    /// 保持しているメッセージスイート。
    private: typename this_type::suite suite_;

}; // class psyq::any::message::packet::internal

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief プロセス外に送受信するメッセージスイートのパケット。
    @tparam template_base_suite @copydoc psyq::any::message::packet::suite
    @tparam template_suite      @copydoc psyq::any::message::packet::external::suite
 */
template<typename template_base_suite>
template<typename template_suite>
class psyq::any::message::packet<template_base_suite>::external:
    public psyq::any::message::packet<template_base_suite>
{
    /// thisが指す値の型。
    private: typedef external this_type;
    /// this_type の基底型。
    public: typedef psyq::any::message::packet<template_base_suite> base_type;

    //-------------------------------------------------------------------------
    public: typedef template_suite suite;
    public: typedef std::unique_ptr<template_suite> suite_unique_ptr;

    //-------------------------------------------------------------------------
    /** @brief メッセージパケットを構築する。
        @param[in] in_suite this_type::suite_ の初期値。
     */
    public: explicit external(typename this_type::suite in_suite)
    PSYQ_NOEXCEPT: suite_(std::move(in_suite))
    {}

    public: typename base_type::suite const& get_suite()
    const PSYQ_NOEXCEPT override
    {
        return this->suite_;
    }

    public: typename this_type::suite const* get_external_suite()
    const PSYQ_NOEXCEPT override
    {
        return &this->suite_;
    }

    public: psyq::any::rtti const* get_parameter_rtti()
    const PSYQ_NOEXCEPT override
    {
        return psyq::any::rtti::find<typename this_type::suite::parameter>();
    }

    public: void const* get_parameter_data(
        psyq::any::rtti const* const in_rtti)
    const PSYQ_NOEXCEPT override
    {
        return psyq::any::rtti::find(in_rtti, this->this_type::get_parameter_rtti()) != nullptr?
            this->suite_.get_parameter_data(): nullptr;
    }

    //-------------------------------------------------------------------------
    /// 保持しているメッセージスイート。
    private: typename this_type::suite suite_;

}; // class psyq::any::message::packet::external

#endif // !defined(PSYQ_ANY_MESSAGE_PACKET_HPP_)
