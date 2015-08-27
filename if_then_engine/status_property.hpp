/// @file
/// @brief @copybrief psyq::if_then_engine::_private::status_property
/// @author Hillco Psychi (https://twitter.com/psychi)
#ifndef PSYQ_IF_THEN_ENGINE_STATUS_PROPERTY_HPP_
#define PSYQ_IF_THEN_ENGINE_STATUS_PROPERTY_HPP_

#include "../bit_algorithm.hpp"

/// @cond
namespace psyq
{
    namespace if_then_engine
    {
        namespace _private
        {
            template<typename, typename, typename> class status_property;

        } // namespace _private
    } // namespace if_then_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @copybrief psyq::if_then_engine::_private::reservoir::status_property
/// @tparam template_chunk_key    @copydoc status_property::chunk_key
/// @tparam template_bit_position @copydoc status_property::bit_position
/// @tparam template_format       @copydoc status_property::format
template<
    typename template_chunk_key,
    typename template_bit_position,
    typename template_format>
class psyq::if_then_engine::_private::status_property
{
    /// @brief this が指す値の型。
    private: typedef status_property this_type;

    //-------------------------------------------------------------------------
    /// @brief 状態値が格納されているビット領域チャンクの識別値を表す型。
    public: typedef template_chunk_key chunk_key;
    /// @brief 状態値が格納されているビット位置を表す型。
    public: typedef template_bit_position bit_position;
    static_assert(
        std::is_integral<template_bit_position>::value,
        "template_bit_position is not integer type.");
    /// @brief 状態値のビット構成を表す型。
    public: typedef template_format format;
    static_assert(
        std::is_integral<template_format>::value,
        "'template_format' is not integer type.");

    //-------------------------------------------------------------------------
    /// @brief 状態値プロパティを構築する。
    public: status_property(
        /// [in] this_type::chunk_key_ の初期値。
        typename this_type::chunk_key const& in_chunk_key,
        /// [in] this_type::bit_position_ の初期値。
        typename this_type::bit_position const in_bit_position,
        /// [in] this_type::format_ の初期値。
        typename this_type::format const in_format)
    PSYQ_NOEXCEPT:
    chunk_key_(in_chunk_key),
    bit_position_(in_bit_position),
    format_(in_format),
    transition_(true)
    {}

    //-------------------------------------------------------------------------
    /// @name 状態値のプロパティ
    /// @{

    /// @brief 状態値のプロパティが空か判定する。
    /// @retval true  *this は空。
    /// @retval false *this は空ではない。
    public: bool is_empty() const PSYQ_NOEXCEPT
    {
        return this->format_ == 0;
    }

    /// @brief 状態値が格納されているビット列チャンクの識別値を取得する。
    /// @return @copydoc this_type::chunk_key_
    public: typename this_type::chunk_key const& get_chunk_key()
    const PSYQ_NOEXCEPT
    {
        return this->chunk_key_;
    }

    /// @brief 状態値のビット位置を取得する。
    /// @return 状態値のビット位置。
    public: typename this_type::bit_position get_bit_position()
    const PSYQ_NOEXCEPT
    {
        return this->bit_position_;
    }

    /// @brief 状態値のビット構成を取得する。
    /// @return 状態値のビット構成。
    public: typename this_type::format get_format() const PSYQ_NOEXCEPT
    {
        return this->format_;
    }

    /// @brief 状態変化フラグを取得する。
    /// @return 状態変化フラグ。
    public: bool get_transition() const PSYQ_NOEXCEPT
    {
        return this->transition_;
    }

    public: void set_transition(bool const in_transition) PSYQ_NOEXCEPT
    {
        this->transition_ = in_transition;
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @brief 状態値が格納されているビット領域チャンクの識別値。
    private: typename this_type::chunk_key chunk_key_;
    /// @brief 状態値が格納されているビット領域のビット位置。
    private: typename this_type::bit_position bit_position_;
    /// @brief 状態値のビット構成。
    private: typename this_type::format format_;
    /// @brief 状態変化フラグ。
    private: bool transition_;

}; // class psyq::if_then_engine::_private::status_property

#endif // defined(PSYQ_IF_THEN_ENGINE_STATUS_PROPERTY_HPP_)
// vim: set expandtab:
