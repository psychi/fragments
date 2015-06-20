/** @file
    @brief @copybrief psyq::scenario_engine::_private::state_registry
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_SCENARIO_ENGINE_STATE_REGISTRY_HPP_
#define PSYQ_SCENARIO_ENGINE_STATE_REGISTRY_HPP_

#include <cstdint>

/// @cond
namespace psyq
{
    namespace scenario_engine
    {
        namespace _private
        {
            enum: std::uint8_t
            {
                /// @brief 1バイトあたりのビット数。
                BITS_PER_BYTE = 8,
            };
            template<typename, typename, typename, typename>
                class state_registry;
            template<typename, typename, typename> class state_chunk;
        } // namespace _private
    } // namespace scenario_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 状態値の登記情報。
    @tparam template_key          @copydoc state_registry::key
    @tparam template_chunk_key    @copydoc state_registry::chunk_key
    @tparam template_bit_position @copydoc state_registry::bit_position
    @tparam template_bit_size     @copydoc state_registry::bit_size
 */
template<
    typename template_key,
    typename template_chunk_key,
    typename template_bit_position,
    typename template_bit_size>
class psyq::scenario_engine::_private::state_registry
{
    /// @brief thisが指す値の型。
    private: typedef state_registry this_type;

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
    public: typedef template_bit_size bit_size;
    static_assert(
        std::is_integral<template_bit_size>::value
        && std::is_unsigned<template_bit_size>::value,
        "'template_bit_size' is not unsigned integer type");

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
        format_SIZE_FRONT,
        /// @brief 状態値のビット数の末尾ビット位置。
        format_SIZE_BACK = 31,
        /// @brief 状態値が格納されているビット位置を取得するためのビットマスク。
        format_POSITION_MASK =
            (2 << (format_POSITION_BACK - format_POSITION_FRONT)) - 1,
        /// @brief 状態値のビット数を取得するためのビットマスク。
        format_SIZE_MASK = (2 << (format_SIZE_BACK - format_SIZE_FRONT)) - 1,
    };

    /// @brief 状態値の種別を表す型。
    public: typedef
        typename std::make_signed<typename this_type::bit_size>::type
        variety;

    /// @brief 状態値のビット位置とビット数を表す型。
    public: typedef typename this_type::bit_position format;
    public: struct format_less;

    //-------------------------------------------------------------------------
    /** @brief 状態値登記を構築する。
        @param[in] in_state_key this_type::key_ の初期値。
        @param[in] in_chunk_key this_type::chunk_key_ の初期値。
        @param[in] in_variety   状態値の種別の初期値。
     */
    public: state_registry(
        typename this_type::key in_state_key,
        typename this_type::chunk_key in_chunk_key,
        typename this_type::variety const in_variety)
    PSYQ_NOEXCEPT:
    chunk_key_(std::move(in_chunk_key)),
    key_(std::move(in_state_key)),
    format_(
        (1 << this_type::format_TRANSITION_FRONT) | (
            (in_variety & this_type::format_SIZE_MASK)
            << this_type::format_SIZE_FRONT))
    {}

    /** @brief 状態値の種別を取得する。
        @return 状態値の種別。
     */
    public: typename this_type::variety get_variety() const PSYQ_NOEXCEPT
    {
        auto const local_mod_size(
            this_type::format_SIZE_BACK - this_type::format_SIZE_FRONT);
        auto const local_minus(
            1 & static_cast<typename this_type::variety>(
                this->format_ >> this_type::format_SIZE_BACK));
        return (-local_minus << local_mod_size) |
            static_cast<typename this_type::variety>(
                this_type::format_SIZE_MASK & (
                    this->format_ >> this_type::format_SIZE_FRONT));
    }

    /** @brief 状態値の種別を設定する。
        @param[in] in_variety 設定する種別。
     */
    public: void set_variety(typename this_type::variety const in_variety)
    PSYQ_NOEXCEPT
    {
        typename this_type::format const local_variety(in_variety);
        auto const local_mask(
            this_type::format_SIZE_MASK << this_type::format_SIZE_FRONT);
        this->format_ = (~local_mask & this->format_) |
            (local_mask & (local_variety << this_type::format_SIZE_FRONT));
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
    public: typename this_type::bit_size get_size()
    const PSYQ_NOEXCEPT
    {
        return this_type::get_size(this->format_);
    }

    /** @brief ビット領域のビット数を取得する。
        @param[in] in_format ビット領域の構成。
        @return ビット領域のビット数。
     */
    public: static typename this_type::bit_size get_size(
        typename this_type::format const in_format)
    PSYQ_NOEXCEPT
    {
        return static_cast<typename this_type::bit_size>(
            (in_format >> this_type::format_SIZE_FRONT)
            & this_type::format_SIZE_MASK);
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
        this->format_ = (~local_transition_mask & this->format_) |
            (local_transition_mask & in_source.format_);
    }

    //-------------------------------------------------------------------------
    /// @brief 状態値が格納されているビット列チャンクの識別値。
    typename this_type::chunk_key chunk_key_;
    /// @brief 状態値に対応する識別値。
    typename this_type::key key_;
    /// @brief 状態値が格納されているビット領域。
    typename this_type::format format_;

}; // class psyq::scenario_engine::_private::state_registry

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 空きビット領域を比較する関数オブジェクト。
template<
    typename template_key,
    typename template_chunk_key,
    typename template_bit_position,
    typename template_bit_size>
struct psyq::scenario_engine::_private::state_registry<
    template_key,
    template_chunk_key,
    template_bit_position,
    template_bit_size>
        ::format_less
{
    bool operator()(
        typename state_registry::format const in_left,
        typename state_registry::format const in_right)
    const PSYQ_NOEXCEPT
    {
        // ビット領域のビット数で比較する。
        auto const local_left_size(state_registry::get_size(in_left));
        auto const local_right_size(state_registry::get_size(in_right));
        if (local_left_size != local_right_size)
        {
            return local_left_size < local_right_size;
        }

        // ビット領域のビット位置で比較する。
        auto const local_left_position(state_registry::get_position(in_left));
        auto const local_right_position(state_registry::get_position(in_right));
        return local_left_position < local_right_position;
    }

    bool operator()(
        typename state_registry::format const in_left,
        typename state_registry::bit_size const in_right)
    const PSYQ_NOEXCEPT
    {
        // ビット領域のビット数で比較する。
        auto const local_left_size(state_registry::get_size(in_left));
        return local_left_size < in_right;
    }

    bool operator()(
        typename state_registry::bit_size const in_left,
        typename state_registry::format const in_right)
    const PSYQ_NOEXCEPT
    {
        // ビット領域のビット数で比較する。
        auto const local_right_size(state_registry::get_size(in_right));
        return in_left < local_right_size;
    }

}; // struct psyq::scenario_engine::_private::state_registry::format_less

#endif // defined(PSYQ_SCENARIO_ENGINE_STATE_REGISTRY_HPP_)
// vim: set expandtab:
