/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_ANY_MESSAGE_SUITE_HPP_
#define PSYQ_ANY_MESSAGE_SUITE_HPP_

//#include "psyq/assert.hpp"
//#include "psyq/any/message/tag.hpp"
//#include "psyq/any/message/call.hpp"

/// @cond
namespace psyq
{
    namespace any
    {
        namespace message
        {
            template<typename, typename, typename> class suite;
        } // namespace message
    } // namespace any
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 荷札と呼出状を一揃いにしたメッセージ一式。
    @tparam template_tag_key  @copydoc psyq::any::message::tag::key
    @tparam template_call_key @copydoc psyq::any::message::call::key
    @tparam template_size     @copydoc psyq::any::message::suite::size_type
 */
template<
    typename template_tag_key,
    typename template_call_key,
    typename template_size>
class psyq::any::message::suite
{
    private: typedef suite this_type; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
    /// @copydoc this_type::tag_
    public: typedef psyq::any::message::tag<template_tag_key> tag;
    /// @copydoc this_type::call_
    public: typedef psyq::any::message::call<template_call_key> call;
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
    /// @name メッセージ一式の構築
    //@{
    /** @brief メッセージ一式を構築する。
        @param[in] in_tag  this_type::tag_ の初期値。
        @param[in] in_call this_type::call_ の初期値。
     */
    public: PSYQ_CONSTEXPR suite(
        typename this_type::tag const& in_tag,
        typename this_type::call const& in_call)
    PSYQ_NOEXCEPT:
        tag_(in_tag),
        call_(in_call),
        parameter_offset_(sizeof(this_type)),
        parameter_size_(0)
    {}
    //@}
    /** @brief 引数を持つメッセージ一式を構築する。
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

    //-------------------------------------------------------------------------
    /// @name メッセージ一式のプロパティ
    //@{
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
    //@}
    //-------------------------------------------------------------------------
    /// メッセージの荷札。
    private: typename this_type::tag tag_;
    /// メッセージの呼出状。
    private: typename this_type::call call_;
    /// メッセージ引数の先頭位置へのオフセットバイト数。
    private: typename this_type::size_type parameter_offset_;
    /// メッセージ引数のバイトサイズ。
    private: typename this_type::size_type parameter_size_;

}; // class psyq::any::message::suite

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 荷札と呼出状と任意型の引数を一揃いにしたメッセージ一式。
    @tparam template_tag_key   @copydoc psyq::any::message::tag::key
    @tparam template_call_key  @copydoc psyq::any::message::call::key
    @tparam template_size      @copydoc psyq::any::message::suite::size_type
    @tparam template_parameter @copydoc psyq::any::message::suite::parametric::parameter
 */
template<
    typename template_tag_key,
    typename template_call_key,
    typename template_size>
template<typename template_parameter>
class psyq::any::message::suite<template_tag_key, template_call_key, template_size>::parametric:
    public psyq::any::message::suite<template_tag_key, template_call_key, template_size>
{
    /// thisが指す値の型。
    private: typedef parametric this_type;
    /// this_type の基底型。
    public: typedef psyq::any::message::suite<
        template_tag_key, template_call_key, template_size>
            base_type;

    //-------------------------------------------------------------------------
    /// @copydoc this_type::parameter_
    public: typedef template_parameter parameter;

    //-------------------------------------------------------------------------
    /// @name メッセージ一式の構築
    //@{
    /** @brief メッセージ一式を構築する。
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

    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元インスタンス。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source) PSYQ_NOEXCEPT
    {
        if (this != &io_source)
        {
            static_assert<base_type&>(*this) = std::move(io_source);
            this->parameter_ = std::move(io_source.parameter_);
        }
        return *this;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name メッセージ一式のプロパティ
    //@{
    public: typename this_type::parameter const& get_parameter()
    const PSYQ_NOEXCEPT
    {
        return this->parameter_;
    }
    //@}

    //-------------------------------------------------------------------------
    private: typename this_type::parameter parameter_; ///< メッセージの引数。

}; // class psyq::any::message::suite::parametric

#endif // !defined(PSYQ_ANY_MESSAGE_SUITE_HPP_)
