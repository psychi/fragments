/** @file
    @brief @copybrief psyq::if_then_engine::_private::status_property
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_IF_THEN_ENGINE_STATUS_PROPERTY_HPP_
#define PSYQ_IF_THEN_ENGINE_STATUS_PROPERTY_HPP_

#include <cstdint>
#include "../assert.hpp"

/// @cond
namespace psyq
{
    namespace if_then_engine
    {
        namespace _private
        {
            enum: std::uint8_t
            {
                /// @brief 1バイトあたりのビット数。
                BITS_PER_BYTE = 8,
            };
            template<typename, typename, typename> class status_property;
        } // namespace _private
    } // namespace if_then_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 状態値のプロパティ情報。
    @tparam template_chunk_key @copydoc status_property::chunk_key
    @tparam template_pack      @copydoc status_property::pack
    @tparam template_bit_width @copydoc status_property::bit_width
 */
template<
    typename template_chunk_key,
    typename template_pack,
    typename template_bit_width>
class psyq::if_then_engine::_private::status_property
{
    /// @brief thisが指す値の型。
    private: typedef status_property this_type;

    /// @brief 状態値が格納されているビット列チャンクの識別値。
    public: typedef template_chunk_key chunk_key;

    /// @brief プロパティ情報のビット列を表す型。
    public: typedef template_pack pack;
    static_assert(
        std::is_integral<template_pack>::value
        && std::is_unsigned<template_pack>::value,
        "'template_pack' is not unsigned integer type");

    /// @brief 状態値のビット数を表す型。
    public: typedef template_bit_width bit_width;
    static_assert(
        std::is_integral<template_bit_width>::value
        && std::is_unsigned<template_bit_width>::value,
        "'template_bit_width' is not unsigned integer type");

    /// @brief ビット列チャンク中のビット位置を表す型。
    public: typedef typename this_type::pack bit_position;

    /// @brief this_type::pack のビット構成。
    public: enum pack_format: typename this_type::pack
    {
        /// @brief 状態値が格納されているビット位置の先頭ビット位置。
        pack_POSITION_FRONT,

        /// @brief 状態値が格納されているビット位置の末尾ビット位置。
        pack_POSITION_BACK= 23,

        /// @brief 状態変化フラグの先頭ビット位置。
        pack_TRANSITION_FRONT,

        /// @brief 状態変化フラグの末尾ビット位置。
        pack_TRANSITION_BACK = pack_TRANSITION_FRONT,

        /// @brief 状態値のビット構成の先頭ビット位置。
        pack_FORMAT_FRONT,

        /// @brief 状態値のビット構成の末尾ビット位置。
        pack_FORMAT_BACK = 31,

        /// @brief 状態値が格納されているビット位置を取得するためのビットマスク。
        pack_POSITION_MASK =
            (2 << (pack_POSITION_BACK - pack_POSITION_FRONT)) - 1,

        /// @brief 状態値のビット構成を取得するためのビットマスク。
        pack_FORMAT_MASK =
            (2 << (pack_FORMAT_BACK - pack_FORMAT_FRONT)) - 1,
    };
    static_assert(
        // ビット位置の最大値が
        // status_property::bit_position に収まることを確認する。
        this_type::pack_POSITION_BACK - this_type::pack_POSITION_FRONT
        < sizeof(typename this_type::bit_position) *
            psyq::if_then_engine::_private::BITS_PER_BYTE,
        "");

    /// @brief 状態値のビット構成を表す型。
    public: typedef
        typename std::make_signed<typename this_type::bit_width>::type
        format;

    //-------------------------------------------------------------------------
    /** @brief 状態値プロパティを構築する。
        @param[in] in_chunk_key this_type::chunk_key_ の初期値。
        @param[in] in_format    状態値のビット構成の初期値。
     */
    public: status_property(
        typename this_type::chunk_key const& in_chunk_key,
        typename this_type::format const in_format)
    PSYQ_NOEXCEPT:
    chunk_key_(in_chunk_key),
    pack_(
        this_type::set_format(
            1 << this_type::pack_TRANSITION_FRONT, in_format))
    {}

    /** @brief 状態値が格納されているビット列チャンクの識別値を取得する。
        @return @copydoc this_type::chunk_key_
     */
    public: typename this_type::chunk_key const& get_chunk_key()
    const PSYQ_NOEXCEPT
    {
        return this->chunk_key_;
    }

    //-------------------------------------------------------------------------
    /** @name 状態値のビット構成
        @{
     */
    /** @brief 状態値のビット構成を取得する。
        @return 状態値のビット構成。
     */
    public: typename this_type::format get_format() const PSYQ_NOEXCEPT
    {
        return this_type::get_format(this->pack_);
    }

    /** @brief 状態値のビット構成を設定する。
        @param[in] in_format 設定するビット構成。
     */
    public: void set_format(typename this_type::format const in_format)
    PSYQ_NOEXCEPT
    {
        this->pack_ = this_type::set_format(this->pack_, in_format);
    }
    /// @}
    public: static typename this_type::format get_format_bits(
        typename this_type::pack const& in_pack)
    PSYQ_NOEXCEPT
    {
        return static_cast<typename this_type::format>(
            this_type::pack_FORMAT_MASK & (
                in_pack >> this_type::pack_FORMAT_FRONT));
    }

    /** @brief 状態値のビット構成を取得する。
        @param[in] in_pack プロパティビット列。
        @return 状態値のビット構成。
     */
    public: static typename this_type::format get_format(
        typename this_type::pack const& in_pack)
    PSYQ_NOEXCEPT
    {
        auto const local_minus(
            1 & static_cast<typename this_type::format>(
                in_pack >> this_type::pack_FORMAT_BACK));
        auto const local_mod_width(
            this_type::pack_FORMAT_BACK - this_type::pack_FORMAT_FRONT);
        return this_type::get_format_bits(in_pack)
            | (-local_minus << local_mod_width);
    }

    /** @brief 状態値のビット構成を設定する。
        @param[in] in_pack   状態値のビット構成を設定するビット列。
        @param[in] in_format 設定するビット構成。
     */
    public: static typename this_type::pack set_format(
        typename this_type::pack const& in_pack,
        typename this_type::format const in_format)
    PSYQ_NOEXCEPT
    {
        PSYQ_ASSERT(in_format != 0);
        auto const local_mask(
            this_type::pack_FORMAT_MASK << this_type::pack_FORMAT_FRONT);
        return (~local_mask & in_pack) | (
            (in_format & this_type::pack_FORMAT_MASK)
            << this_type::pack_FORMAT_FRONT);
    }

    //-------------------------------------------------------------------------
    /** @name 状態値のビット位置
        @{
     */
    /** @brief 状態値のビット位置を取得する。
        @return 状態値のビット位置。
     */
    public: typename this_type::bit_position get_bit_position()
    const PSYQ_NOEXCEPT
    {
        return this_type::get_bit_position(this->pack_);
    }

    /** @brief 状態値のビット位置を設定する。
        @param[in] in_position 設定するビット位置。
     */
    public: bool set_bit_position(std::size_t const in_position) PSYQ_NOEXCEPT
    {
        if (this_type::pack_POSITION_MASK < in_position)
        {
            return false;
        }
        auto const local_position(
            static_cast<typename this_type::pack>(in_position)
            << this_type::pack_POSITION_FRONT);
        auto const local_mask(
            this_type::pack_POSITION_MASK << this_type::pack_POSITION_FRONT);
        this->pack_ =
            (~local_mask & this->pack_) | (local_mask & local_position);
        return true;
    }

    /// @}
    /** @brief ビット領域のビット位置を取得する。
        @param[in] in_pack プロパティビット列。
        @return ビット領域のビット位置。
     */
    public: static typename this_type::bit_position get_bit_position(
        typename this_type::pack const& in_pack)
    PSYQ_NOEXCEPT
    {
        return (in_pack >> this_type::pack_POSITION_FRONT)
            & this_type::pack_POSITION_MASK;
    }

    //-------------------------------------------------------------------------
    /** @name 状態値の変化
        @{
     */
    /** @brief 状態変化フラグを取得する。
        @return 状態変化フラグ。
     */
    public: bool get_transition() const PSYQ_NOEXCEPT
    {
        return 1 & (this->pack_ >> this_type::pack_TRANSITION_FRONT);
    }

    /** @brief 状態変化フラグをコピーする。
        @param[in] in_source コピー元となる状態値プロパティ。
     */
    public: void copy_transition(this_type const& in_source)
    {
        typename this_type::pack const local_transition_mask(
            1 << this_type::pack_TRANSITION_FRONT);
        this->pack_ = (~local_transition_mask & this->pack_)
            | (local_transition_mask & in_source.pack_);
    }

    /// @brief 状態変化フラグを偽にする。
    public: void reset_transition()
    {
        this->pack_ &= ~(1 << this_type::pack_TRANSITION_FRONT);
    }

    /// @brief 状態変化フラグを真にする。
    public: void set_transition()
    {
        this->pack_ |= 1 << this_type::pack_TRANSITION_FRONT;
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @brief 状態値が格納されているビット列チャンクの識別値。
    private: typename this_type::chunk_key chunk_key_;
    /// @brief 状態値のプロパティ情報を詰め込んだビット列。
    private: typename this_type::pack pack_;

}; // class psyq::if_then_engine::_private::status_property

#endif // defined(PSYQ_IF_THEN_ENGINE_STATUS_PROPERTY_HPP_)
// vim: set expandtab:
