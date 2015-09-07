/// @file
/// @brief @copybrief psyq::if_then_engine::_private::status_chunk
/// @author Hillco Psychi (https://twitter.com/psychi)
#ifndef PSYQ_IF_THEN_ENGINE_STATUS_CHUNK_HPP_
#define PSYQ_IF_THEN_ENGINE_STATUS_CHUNK_HPP_

#include <vector>
#include "../bit_algorithm.hpp"

/// @cond
namespace psyq
{
    namespace if_then_engine
    {
        namespace _private
        {
            template<typename, typename, typename, typename> class status_chunk;
        } // namespace _private
    } // namespace if_then_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @copybrief psyq::if_then_engine::_private::reservoir::status_chunk
/// @tparam template_bit_block    @copydoc status_chunk::bit_block
/// @tparam template_bit_position @copydoc status_chunk::bit_position
/// @tparam template_bit_width    @copydoc status_chunk::bit_width
/// @tparam template_allocator    @copydoc status_chunk::allocator_type
template<
    typename template_bit_block,
    typename template_bit_position,
    typename template_bit_width,
    typename template_allocator>
class psyq::if_then_engine::_private::status_chunk
{
    /// @brief thisが指す値の型。
    private: typedef status_chunk this_type;

    //-------------------------------------------------------------------------
    /// @brief ビット列の単位を表す型。
    public: typedef template_bit_block bit_block;
    static_assert(
        std::is_unsigned<template_bit_block>::value,
        "template_bit_block is not unsigned integer type.");
    /// @brief ビット列のビット位置を表す型。
    public: typedef template_bit_position bit_position;
    static_assert(
        std::is_unsigned<template_bit_position>::value,
        "template_bit_position is not unsigned integer type.");
    /// @brief ビット列のビット幅を表す型。
    public: typedef template_bit_width bit_width;
    static_assert(
        std::is_unsigned<template_bit_width>::value,
        "template_bit_width is not unsigned integer type.");
    /// @brief コンテナに用いるメモリ割当子の型。
    public: typedef template_allocator allocator_type;
    /// @copydoc this_type::bit_blocks_
    public: typedef
        std::vector<
            typename this_type::bit_block, typename this_type::allocator_type>
        bit_block_container;
    /// @copybrief this_type::empty_fields_
    /// @details
    /// - first は、空きビット領域のビット幅。
    /// - second は、空きビット領域のビット位置。
    public: typedef
        std::vector<
            std::pair<
                typename this_type::bit_width,
                typename this_type::bit_position>,
            typename this_type::allocator_type>
        empty_field_container;
    public: enum: typename this_type::bit_position
    {
        /// @brief 無効なビット位置。
        INVALID_BIT_POSITION = ~static_cast<typename this_type::bit_position>(0),
    };
    public: enum: typename this_type::bit_width
    {
        /// @brief ビット列単位のビット数。
        BLOCK_BIT_WIDTH = static_cast<typename this_type::bit_width>(
            sizeof(typename this_type::bit_block) * CHAR_BIT),
    };
    static_assert(
        // this_type::BLOCK_BIT_WIDTH が this_type::bit_width に収まるのを確認する。
        this_type::BLOCK_BIT_WIDTH < (
            1 << (sizeof(this_type::bit_width) * CHAR_BIT - 1)),
        "this_type::BLOCK_BIT_WIDTH is overflow.");

    //-------------------------------------------------------------------------
    /// @brief 空のビット領域チャンクを構築する。
    public: explicit status_chunk(
        /// [in] コンテナが使うメモリ割当子の初期値。
        typename this_type::allocator_type const& in_allocator):
    bit_blocks_(in_allocator),
    empty_fields_(in_allocator)
    {}

#ifdef PSYQ_NO_STD_DEFAULTED_FUNCTION
    /// @brief ムーブ構築子。
    public: status_chunk(
        /// [in,out] ムーブ元となるインスタンス。
        this_type&& io_source):
    bit_blocks_(std::move(io_source.bit_blocks_)),
    empty_fields_(std::move(io_source.empty_fields_))
    {}

    /// @brief ムーブ代入演算子。
    /// @return *this
    public: this_type& operator=(
        /// [in,out] ムーブ元となるインスタンス。
        this_type&& io_source)
    {
        this->bit_blocks_ = std::move(io_source.bit_blocks_);
        this->empty_fields_ = std::move(io_source.empty_fields_);
        return *this;
    }
#endif // defined(PSYQ_NO_STD_DEFAULTED_FUNCTION)

    /// @brief 状態値を格納するビット領域を生成する。
    /// @return
    /// 生成したビット領域のビット位置。
    /// 失敗した場合は this_type::INVALID_BIT_POSITION を返す。
    public: typename this_type::bit_position allocate_bit_field(
        /// [in] 生成するビット領域のビット数。
        typename this_type::bit_width const in_bit_width)
    {
        // 状態値を格納できるビット領域を、空きビット領域から取得する。
        auto const local_empty_field(
            std::lower_bound(
                this->empty_fields_.begin(),
                this->empty_fields_.end(),
                typename this_type::empty_field_container::value_type(
                    in_bit_width, 0)));
        return local_empty_field != this->empty_fields_.end()?
            // 既存の空き領域を再利用する。
            this->reuse_empty_field(in_bit_width, local_empty_field):
            // 適切な空き領域がないので、新たな領域を追加する。
            this->add_bit_field(in_bit_width);
    }

    /// @brief ビット領域の値を取得する。
    /// @return ビット領域の値。該当するビット領域がない場合は~0を返す。
    public: typename this_type::bit_block get_bit_field(
        /// [in] 値を取得するビット領域のビット位置。
        std::size_t const in_bit_position,
        /// [in] 値を取得するビット領域のビット幅。
        std::size_t const in_bit_width)
    const PSYQ_NOEXCEPT
    {
        if (this_type::BLOCK_BIT_WIDTH < in_bit_width)
        {
            PSYQ_ASSERT(false);
            return ~static_cast<typename this_type::bit_block>(0);
        }
        auto const local_block_index(
            in_bit_position / this_type::BLOCK_BIT_WIDTH);
        if (this->bit_blocks_.size() <= local_block_index)
        {
            PSYQ_ASSERT(false);
            return ~static_cast<typename this_type::bit_block>(0);
        }

        // ビット列ブロックでのビット位置を決定し、値を取り出す。
        return psyq::get_bit_field(
            this->bit_blocks_.at(local_block_index),
            in_bit_position - local_block_index * this_type::BLOCK_BIT_WIDTH,
            in_bit_width);
    }

    /// @brief ビット領域に値を設定する。
    /// @retval 正 元とは異なる値を設定した。
    /// @retval 0  元と同じ値を設定した。
    /// @retval 負 失敗。値を設定できなかった。
    public: std::int8_t set_bit_field(
        /// [in] 値を設定するビット領域のビット位置。
        std::size_t const in_bit_position,
        /// [in] 値を設定するビット領域のビット幅。
        std::size_t const in_bit_width,
        /// [in] ビット領域に設定する値。
        typename this_type::bit_block const& in_value)
    PSYQ_NOEXCEPT
    {
        if (psyq::shift_right_bitwise(in_value, in_bit_width) != 0)
        {
            return -1;
        }
        auto const local_block_index(
            in_bit_position / this_type::BLOCK_BIT_WIDTH);
        if (this->bit_blocks_.size() <= local_block_index)
        {
            PSYQ_ASSERT(false);
            return -1;
        }

        // ビット列単位でのビット位置を決定し、値を埋め込む。
        auto& local_block(this->bit_blocks_.at(local_block_index));
        auto const local_last_block(local_block);
        local_block = psyq::set_bit_field(
            local_block,
            in_bit_position - local_block_index * this_type::BLOCK_BIT_WIDTH,
            in_bit_width,
            in_value);
        return local_last_block != local_block;
    }

    //-------------------------------------------------------------------------
    /// @brief 空きビット領域を再利用する。
    /// @return 再利用したビット領域のビット位置。
    private: typename this_type::bit_position reuse_empty_field(
        /// [in] 再利用したい領域のビット幅。
        typename this_type::bit_position const in_bit_width,
        /// [in] 再利用する空きビット領域のプロパティを指す反復子。
        typename this_type::empty_field_container::iterator const in_empty_field)
    {
        // 既存の空き領域を再利用する。
        auto const local_empty_position(in_empty_field->second);
        auto const local_empty_width(in_empty_field->first);
        PSYQ_ASSERT(0 < local_empty_width);

        // 再利用する空き領域を削除する。
        this->empty_fields_.erase(in_empty_field);
        if (in_bit_width < local_empty_width)
        {
            // 余りを空き領域として追加する。
            this_type::add_empty_field(
                this->empty_fields_,
                local_empty_position + in_bit_width,
                local_empty_width - in_bit_width);
        }
        return local_empty_position;
    }

    /// @brief 状態値に使うビット領域を追加する。
    /// @return 追加したビット領域のビット位置。
    /// 失敗した場合は this_type::INVALID_BIT_POSITION を返す。
    private: typename this_type::bit_position add_bit_field(
        /// [in] 追加するビット領域のビット幅。
        typename this_type::bit_width const in_bit_width)
    {
        if (in_bit_width <= 0 || this_type::BLOCK_BIT_WIDTH < in_bit_width)
        {
            PSYQ_ASSERT(false);
            return this_type::INVALID_BIT_POSITION;
        }

        // 新たにビット列を追加する。
        auto const local_position(
            this->bit_blocks_.size() * this_type::BLOCK_BIT_WIDTH);
        if (this_type::INVALID_BIT_POSITION <= local_position)
        {
            // ビット位置の最大値を超過した。
            PSYQ_ASSERT(false);
            return this_type::INVALID_BIT_POSITION;
        }
        auto const local_add_block_width(
            (in_bit_width + this_type::BLOCK_BIT_WIDTH - 1)
            / this_type::BLOCK_BIT_WIDTH);
        this->bit_blocks_.insert(
            this->bit_blocks_.end(), local_add_block_width, 0);

        // 余りを空きビット領域に追加する。
        auto const local_add_width(
            local_add_block_width * this_type::BLOCK_BIT_WIDTH);
        if (in_bit_width < local_add_width)
        {
            this_type::add_empty_field(
                this->empty_fields_,
                local_position + in_bit_width,
                local_add_width - in_bit_width);
        }
        return static_cast<typename this_type::bit_position>(local_position);
    }

    /// @brief 空きビット領域を追加する。
    private: static void add_empty_field(
        /// [in,out] 空きビット領域情報を追加するコンテナ。
        typename this_type::empty_field_container& io_empty_fields,
        /// [in] 追加する空きビット領域のビット位置。
        std::size_t const in_bit_position,
        /// [in] 追加する空きビット領域のビット幅。
        std::size_t const in_bit_width)
    {
        typedef
            typename this_type::empty_field_container::value_type
            empty_field;
        empty_field const local_empty_field(
            static_cast<typename empty_field::first_type>(in_bit_width),
            static_cast<typename empty_field::second_type>(in_bit_position));
        if (in_bit_position == local_empty_field.second
            && in_bit_width == local_empty_field.first)
        {
            io_empty_fields.insert(
                std::lower_bound(
                    io_empty_fields.begin(),
                    io_empty_fields.end(),
                    local_empty_field),
                local_empty_field);
        }
        else
        {
            PSYQ_ASSERT(false);
        }
    }

    //-------------------------------------------------------------------------
    /// @brief ビット列のコンテナ。
    public: typename this_type::bit_block_container bit_blocks_;
    /// @brief 空きビット領域情報のコンテナ。
    public: typename this_type::empty_field_container empty_fields_;

}; // class psyq::if_then_engine::_private::status_chunk

#endif // defined(PSYQ_IF_THEN_ENGINE_STATUS_CHUNK_HPP_)
// vim: set expandtab:
