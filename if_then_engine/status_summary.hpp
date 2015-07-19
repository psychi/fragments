/** @file
    @brief @copybrief psyq::if_then_engine::_private::status_summary
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_IF_THEN_ENGINE_STATUS_SUMMARY_HPP_
#define PSYQ_IF_THEN_ENGINE_STATUS_SUMMARY_HPP_

#include <cstdint>
#include "../assert.hpp"
#include "../member_comparison.hpp"

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
            template<typename, typename, typename, typename>
                class status_summary;
        } // namespace _private
    } // namespace if_then_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 状態値の登記情報。
    @tparam template_key          @copydoc status_summary::key
    @tparam template_chunk_key    @copydoc status_summary::chunk_key
    @tparam template_bit_position @copydoc status_summary::bit_position
    @tparam template_bit_width    @copydoc status_summary::bit_width
 */
template<
    typename template_key,
    typename template_chunk_key,
    typename template_bit_position,
    typename template_bit_width>
class psyq::if_then_engine::_private::status_summary
{
    /// @brief thisが指す値の型。
    private: typedef status_summary this_type;

    /// @brief 状態値に対応する識別値。
    public: typedef template_key key;

    /// @brief 状態値が格納されているビット列チャンクの識別値。
    public: typedef template_chunk_key chunk_key;

    /// @brief ビット列チャンク中のビット位置を表す型。
    public: typedef template_bit_position bit_position;
    static_assert(
        std::is_integral<template_bit_position>::value
        && std::is_unsigned<template_bit_position>::value,
        "'template_bit_position' is not unsigned integer type");

    /// @brief 状態値のビット数を表す型。
    public: typedef template_bit_width bit_width;
    static_assert(
        std::is_integral<template_bit_width>::value
        && std::is_unsigned<template_bit_width>::value,
        "'template_bit_width' is not unsigned integer type");

    /// @brief this_type::format のビット構成。
    public: enum: typename this_type::bit_position
    {
        /// @brief 状態値が格納されているビット位置の先頭ビット位置。
        format_POSITION_FRONT,
        /// @brief 状態値が格納されているビット位置の末尾ビット位置。
        format_POSITION_BACK= 23,
        /// @brief 状態変化フラグの先頭ビット位置。
        format_TRANSITION_FRONT,
        /// @brief 状態変化フラグの末尾ビット位置。
        format_TRANSITION_BACK = format_TRANSITION_FRONT,
        /// @brief 状態値のビット数の先頭ビット位置。
        format_WIDTH_FRONT,
        /// @brief 状態値のビット数の末尾ビット位置。
        format_WIDTH_BACK = 31,
        /// @brief 状態値が格納されているビット位置を取得するためのビットマスク。
        format_POSITION_MASK =
            (2 << (format_POSITION_BACK - format_POSITION_FRONT)) - 1,
        /// @brief 状態値のビット数を取得するためのビットマスク。
        format_WIDTH_MASK = (2 << (format_WIDTH_BACK - format_WIDTH_FRONT)) - 1,
    };

    /// @brief 状態値の種別を表す型。
    public: typedef
        typename std::make_signed<typename this_type::bit_width>::type
        variety;

    /// @brief 状態値のビット位置とビット数を表す型。
    public: typedef typename this_type::bit_position format;
    public: struct format_less;

    /// @brief 状態値の識別値を取得する関数オブジェクト。
    public: struct key_fetcher
    {
        public: typename this_type::key const& operator()(this_type const& in_status)
        const PSYQ_NOEXCEPT
        {
            return in_status.get_key();
        }

    }; // struct key_fetcher

    /// @brief 状態値の識別値を比較する関数オブジェクト。
    public: typedef
         psyq::member_comparison<this_type, typename this_type::key>
         key_comparison;

    //-------------------------------------------------------------------------
    /** @brief 状態値登記を構築する。
        @param[in] in_status_key this_type::key_ の初期値。
        @param[in] in_chunk_key this_type::chunk_key_ の初期値。
        @param[in] in_variety   状態値の種別の初期値。
     */
    public: status_summary(
        typename this_type::key in_status_key,
        typename this_type::chunk_key in_chunk_key,
        typename this_type::variety const in_variety)
    PSYQ_NOEXCEPT:
    chunk_key_(std::move(in_chunk_key)),
    key_(std::move(in_status_key)),
    format_(
        (1 << this_type::format_TRANSITION_FRONT) | (
            (in_variety & this_type::format_WIDTH_MASK)
            << this_type::format_WIDTH_FRONT))
    {}

    /** @brief 状態値に対応する識別値を取得する。
        @return @copydoc this_type::key_
     */
    public: typename this_type::key const& get_key() const PSYQ_NOEXCEPT
    {
        return this->key_;
    }

    /** @brief 状態値が格納されているビット列チャンクの識別値を取得する。
        @return @copydoc this_type::chunk_key_
     */
    public: typename this_type::chunk_key const& get_chunk_key() const PSYQ_NOEXCEPT
    {
        return this->chunk_key_;
    }

    /** @brief 状態値の種別を取得する。
        @return 状態値の種別。
     */
    public: typename this_type::variety get_variety() const PSYQ_NOEXCEPT
    {
        auto const local_mod_width(
            this_type::format_WIDTH_BACK - this_type::format_WIDTH_FRONT);
        auto const local_minus(
            1 & static_cast<typename this_type::variety>(
                this->format_ >> this_type::format_WIDTH_BACK));
        return (-local_minus << local_mod_width) |
            static_cast<typename this_type::variety>(
                this_type::format_WIDTH_MASK & (
                    this->format_ >> this_type::format_WIDTH_FRONT));
    }

    /** @brief 状態値の種別を設定する。
        @param[in] in_variety 設定する種別。
     */
    public: void set_variety(typename this_type::variety const in_variety)
    PSYQ_NOEXCEPT
    {
        enum: typename this_type::variety
        {
            EMPTY_VARIETY = 0,
        };
        PSYQ_ASSERT(in_variety != EMPTY_VARIETY);
        typename this_type::format const local_variety(in_variety);
        auto const local_mask(
            this_type::format_WIDTH_MASK << this_type::format_WIDTH_FRONT);
        this->format_ = (~local_mask & this->format_) |
            (local_mask & (local_variety << this_type::format_WIDTH_FRONT));
    }

    /** @brief 状態値のビット位置を取得する。
        @return 状態値のビット位置。
     */
    public: typename this_type::bit_position get_position()
    const PSYQ_NOEXCEPT
    {
        return this_type::get_position(this->format_);
    }

    /** @brief ビット領域のビット位置を取得する。
        @param[in] in_format ビット領域の構成。
        @return ビット領域のビット位置。
     */
    public: static typename this_type::bit_position get_position(
        typename this_type::format const in_format)
    PSYQ_NOEXCEPT
    {
        return (in_format >> this_type::format_POSITION_FRONT) &
            this_type::format_POSITION_MASK;
    }

    /** @brief 状態値のビット位置を設定する。
        @param[in] in_position 設定するビット位置。
     */
    public: bool set_position(std::size_t const in_position)
    PSYQ_NOEXCEPT
    {
        if (this_type::format_POSITION_MASK < in_position)
        {
            return false;
        }
        auto const local_position(
            static_cast<typename this_type::format>(in_position)
            << this_type::format_POSITION_FRONT);
        auto const local_mask(
            this_type::format_POSITION_MASK << this_type::format_POSITION_FRONT);
        this->format_ =
            (~local_mask & this->format_) | (local_mask & local_position);
        return true;
    }

    /** @brief 状態値のビット数を取得する。
        @return 状態値のビット数。
     */
    public: typename this_type::bit_width get_width()
    const PSYQ_NOEXCEPT
    {
        return this_type::get_width(this->format_);
    }

    /** @brief ビット領域のビット数を取得する。
        @param[in] in_format ビット領域の構成。
        @return ビット領域のビット数。
     */
    public: static typename this_type::bit_width get_width(
        typename this_type::format const in_format)
    PSYQ_NOEXCEPT
    {
        return static_cast<typename this_type::bit_width>(
            (in_format >> this_type::format_WIDTH_FRONT)
            & this_type::format_WIDTH_MASK);
    }

    /** @brief 状態変化フラグを取得する。
        @return 状態変化フラグ。
     */
    public: bool get_transition() const PSYQ_NOEXCEPT
    {
        return 1 & (this->format_ >> this_type::format_TRANSITION_FRONT);
    }

    /** @brief 状態変化フラグをコピーする。
        @param[in] in_source コピー元となる状態値登記。
     */
    public: void copy_transition(this_type const& in_source)
    {
        typename this_type::format const local_transition_mask(
            1 << this_type::format_TRANSITION_FRONT);
        this->format_ = (~local_transition_mask & this->format_)
            | (local_transition_mask & in_source.format_);
    }

    public: void reset_transition()
    {
        this->format_ &= ~(1 << this_type::format_TRANSITION_FRONT);
    }

    public: void set_transition()
    {
        this->format_ |= 1 << this_type::format_TRANSITION_FRONT;
    }

    public: static typename this_type::key_comparison::template function<
        typename this_type::key_fetcher, std::less<typename this_type::key>>
    make_key_less()
    {
        return this_type::key_comparison::make_function(
            typename this_type::key_fetcher(),
            std::less<typename this_type::key>());
    }

    //-------------------------------------------------------------------------
    /// @brief 状態値が格納されているビット列チャンクの識別値。
    private: typename this_type::chunk_key chunk_key_;
    /// @brief 状態値に対応する識別値。
    private: typename this_type::key key_;
    /// @brief 状態値が格納されているビット領域。
    private: typename this_type::format format_;

}; // class psyq::if_then_engine::_private::status_summary

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 空きビット領域を比較する関数オブジェクト。
template<
    typename template_key,
    typename template_chunk_key,
    typename template_bit_position,
    typename template_bit_width>
struct psyq::if_then_engine::_private::status_summary<
    template_key,
    template_chunk_key,
    template_bit_position,
    template_bit_width>
        ::format_less
{
    bool operator()(
        typename status_summary::format const in_left,
        typename status_summary::format const in_right)
    const PSYQ_NOEXCEPT
    {
        // ビット領域のビット数で比較する。
        auto const local_left_width(status_summary::get_width(in_left));
        auto const local_right_width(status_summary::get_width(in_right));
        if (local_left_width != local_right_width)
        {
            return local_left_width < local_right_width;
        }

        // ビット領域のビット位置で比較する。
        auto const local_left_position(status_summary::get_position(in_left));
        auto const local_right_position(status_summary::get_position(in_right));
        return local_left_position < local_right_position;
    }

    bool operator()(
        typename status_summary::format const in_left,
        typename status_summary::bit_width const in_right)
    const PSYQ_NOEXCEPT
    {
        // ビット領域のビット数で比較する。
        auto const local_left_width(status_summary::get_width(in_left));
        return local_left_width < in_right;
    }

    bool operator()(
        typename status_summary::bit_width const in_left,
        typename status_summary::format const in_right)
    const PSYQ_NOEXCEPT
    {
        // ビット領域のビット数で比較する。
        auto const local_right_width(status_summary::get_width(in_right));
        return in_left < local_right_width;
    }

}; // struct psyq::if_then_engine::_private::status_summary::format_less

#endif // defined(PSYQ_IF_THEN_ENGINE_STATUS_SUMMARY_HPP_)
// vim: set expandtab:
