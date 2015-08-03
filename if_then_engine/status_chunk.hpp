/** @file
    @brief @copybrief psyq::if_then_engine::_private::status_chunk
    @author Hillco Psychi (https://twitter.com/psychi)
 */
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
            template<typename, typename, typename> class status_chunk;
        } // namespace _private
    } // namespace if_then_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 状態値を格納するビット列のチャンク。
    @tparam template_status_property @copydoc status_chunk::status_property
    @tparam template_bit_block       @copydoc status_chunk::bit_block
    @tparam template_allocator       @copydoc status_chunk::allocator_type
*/
template<
    typename template_status_property,
    typename template_bit_block,
    typename template_allocator>
class psyq::if_then_engine::_private::status_chunk
{
    /// @brief thisが指す値の型。
    private: typedef status_chunk this_type;

    /// @brief 状態値プロパティ
    public: typedef template_status_property status_property;

    /// @brief 状態値ビット列の単位を表す型。
    public: typedef template_bit_block bit_block;

    /// @brief 各種コンテナに用いるメモリ割当子の型。
    public: typedef template_allocator allocator_type;

    /// @copydoc this_type::bit_blocks_
    public: typedef
        std::vector<
            typename this_type::bit_block, typename this_type::allocator_type>
        bit_block_container;

    /// @brief @copydoc this_type::empty_fields_
    public: typedef
        std::vector<
            typename this_type::status_property::pack,
            typename this_type::allocator_type>
        empty_field_container;

    private: struct empty_field_less;

    public: enum: std::uint8_t
    {
        /// @brief 状態値ビット列ブロックのビット数。
        BLOCK_BIT_WIDTH = sizeof(typename this_type::bit_block)
            * psyq::CHAR_BIT_WIDTH,
    };
    static_assert(
        // this_type::BLOCK_BIT_WIDTH が
        // this_type::status_property::pack_FORMAT_MASK に収まることを確認する。
        this_type::BLOCK_BIT_WIDTH <= this_type::status_property::pack_FORMAT_MASK,
        "");

    //-------------------------------------------------------------------------
    /** @brief 空の状態値ビット列チャンクを構築する。
        @param[in] in_allocator コンテナが使うメモリ割当子の初期値。
     */
    public: explicit status_chunk(
        typename this_type::allocator_type const& in_allocator):
    bit_blocks_(in_allocator),
    empty_fields_(in_allocator)
    {}

#ifdef PSYQ_NO_STD_DEFAULTED_FUNCTION
    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元となるインスタンス。
     */
    public: status_chunk(this_type&& io_source):
    bit_blocks_(std::move(io_source.bit_blocks_)),
    empty_fields_(std::move(io_source.empty_fields_))
    {}

    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元となるインスタンス。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source)
    {
        this->bit_blocks_ = std::move(io_source.bit_blocks_);
        this->empty_fields_ = std::move(io_source.empty_fields_);
        return *this;
    }
#endif // defined(PSYQ_NO_STD_DEFAULTED_FUNCTION)

    /** @brief 状態値を格納するビット領域を生成する。
        @param[in] in_bit_width 生成するビット領域のビット数。
        @return 生成したビット領域の、ビット列ブロックでのビット位置。
     */
    public: std::size_t make_status_field(
        typename this_type::status_property::bit_width const in_bit_width)
    {
        // 状態値を格納できるビット領域を、空きビット領域から取得する。
        auto const local_empty_field(
            std::lower_bound(
                this->empty_fields_.begin(),
                this->empty_fields_.end(),
                in_bit_width,
                typename this_type::empty_field_less()));
        return local_empty_field != this->empty_fields_.end()?
            // 既存の空き領域を再利用する。
            this->reuse_empty_field(in_bit_width, local_empty_field):
            // 適切な空き領域がないので、新たな領域を追加する。
            this->add_status_field(in_bit_width);
    }

    /** @brief ビット列から値を取得する。
        @param[in] in_bit_position 値を取得するビット列のビット位置。
        @param[in] in_bit_width    値を取得するビット列のビット数。
        @return
            ビット列から取得した値。
            ただし、該当する値がない場合は、0を返す。
     */
    public: typename this_type::bit_block extract_bitset_value(
        std::size_t const in_bit_position,
        std::size_t const in_bit_width)
    const PSYQ_NOEXCEPT
    {
        if (this_type::BLOCK_BIT_WIDTH < in_bit_width)
        {
            PSYQ_ASSERT(false);
            return 0;
        }
        auto const local_block_index(in_bit_position / this_type::BLOCK_BIT_WIDTH);
        if (this->bit_blocks_.size() <= local_block_index)
        {
            PSYQ_ASSERT(false);
            return 0;
        }

        // ビット列ブロックでのビット位置を決定し、値を取り出す。
        return psyq::extract_bitset_value(
            this->bit_blocks_.at(local_block_index),
            in_bit_position - local_block_index * this_type::BLOCK_BIT_WIDTH,
            in_bit_width);
    }

    /** @brief ビット列に値を設定する。
        @param[in] in_bit_position 値を設定するビット列のビット位置。
        @param[in] in_bit_width    値を設定するビット列のビット数。
        @param[in] in_value        設定する値。
        @retval 正 元とは異なる値を設定した。
        @retval 0  元と同じ値を設定した。
        @retval 負 失敗。値を設定できなかった。
     */
    public: std::int8_t embed_bitset(
        std::size_t const in_bit_position,
        std::size_t const in_bit_width,
        typename this_type::bit_block const in_value)
    PSYQ_NOEXCEPT
    {
        if (psyq::shift_right_bitwise(in_value, in_bit_width) != 0)
        {
            return -1;
        }
        auto const local_block_index(in_bit_position / this_type::BLOCK_BIT_WIDTH);
        if (this->bit_blocks_.size() <= local_block_index)
        {
            PSYQ_ASSERT(false);
            return -1;
        }

        // ビット列ブロックでのビット位置を決定し、値を埋め込む。
        auto& local_block(this->bit_blocks_.at(local_block_index));
        auto const local_last_block(local_block);
        local_block = psyq::embed_bitset(
            local_block,
            in_bit_position - local_block_index * this_type::BLOCK_BIT_WIDTH,
            in_bit_width,
            in_value);
        return local_last_block != local_block;
    }

    //-------------------------------------------------------------------------
    /** @brief 空きビット領域を再利用する。
        @param[in] in_bit_width   再利用したい領域のビット幅。
        @param[in] in_empty_field 再利用する空きビット領域のプロパティを指す反復子。
        @return 再利用したビット領域のビット位置。
     */
    private: std::size_t reuse_empty_field(
        typename this_type::status_property::bit_width const in_bit_width,
        typename this_type::empty_field_container::iterator const in_empty_field)
    {
        // 既存の空き領域を再利用する。
        auto const local_empty_position(
            this_type::status_property::get_bit_position(*in_empty_field));
        auto const local_empty_format(
            this_type::status_property::get_format(*in_empty_field));
        PSYQ_ASSERT(0 < local_empty_format);
        auto const local_empty_width(
            static_cast<typename this_type::status_property::bit_width>(
                local_empty_format));

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

    /** @brief 状態値に使うビット領域を追加する。
        @param[in] in_bit_width 追加するビット領域のビット幅。
        @return 追加したビット領域のビット位置。
     */
    private: std::size_t add_status_field(
        typename this_type::status_property::bit_width const in_bit_width)
    {
        if (in_bit_width <= 0)
        {
            PSYQ_ASSERT(false);
            return 0;
        }
        PSYQ_ASSERT(
            in_bit_width <= this_type::status_property::pack_FORMAT_MASK);

        // 新たにビット列ブロックを追加する。
        auto const local_position(
            this->bit_blocks_.size() * this_type::BLOCK_BIT_WIDTH);
        if (this_type::status_property::pack_POSITION_MASK < local_position)
        {
            // ビット位置の最大値を超過した。
            PSYQ_ASSERT(false);
            return ~0;
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
        return local_position;
    }

    /** @brief 空きビット領域を追加する。
        @param[in,out] io_empty_fields 空きビット領域プロパティのコンテナ。
        @param[in] in_bit_position     追加する空きビット領域のビット位置。
        @param[in] in_bit_width        追加する空きビット領域のビット幅。
     */
    private: static void add_empty_field(
        typename this_type::empty_field_container& io_empty_fields,
        std::size_t const in_bit_position,
        std::size_t const in_bit_width)
    {
        if (in_bit_position <= this_type::status_property::pack_POSITION_MASK
            && in_bit_width <= this_type::status_property::pack_FORMAT_MASK)
        {
            auto const local_empty_field(
               static_cast<typename this_type::status_property::pack>(
                   (in_bit_width << this_type::status_property::pack_FORMAT_FRONT)
                   | (in_bit_position << this_type::status_property::pack_POSITION_FRONT)));
            io_empty_fields.insert(
                std::lower_bound(
                    io_empty_fields.begin(),
                    io_empty_fields.end(),
                    local_empty_field,
                    typename this_type::empty_field_less()),
                local_empty_field);
        }
        else
        {
            PSYQ_ASSERT(false);
        }
    }

    //-------------------------------------------------------------------------
    /// @brief 状態値ビット列ブロックを格納するコンテナ。
    public: typename this_type::bit_block_container bit_blocks_;
    /// @brief 空きビット領域プロパティのコンテナ。
    public: typename this_type::empty_field_container empty_fields_;

}; // class psyq::if_then_engine::_private::status_chunk

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 空きビット領域プロパティを比較する関数オブジェクト。
template<
    typename template_status_property,
    typename template_bit_block,
    typename template_allocator>
struct psyq::if_then_engine::_private::status_chunk<
    template_status_property, template_bit_block, template_allocator>
        ::empty_field_less
{
    bool operator()(
        typename status_property::pack const& in_left,
        typename status_property::pack const& in_right)
    const PSYQ_NOEXCEPT
    {
        // ビット領域のビット数で比較する。
        auto const local_left(status_property::get_format_bits(in_left));
        auto const local_right(status_property::get_format_bits(in_right));
        if (local_left != local_right)
        {
            return local_left < local_right;
        }

        // ビット領域のビット位置で比較する。
        return status_property::get_bit_position(in_left)
            < status_property::get_bit_position(in_right);
    }

    bool operator()(
        typename status_property::pack const& in_left,
        typename status_property::bit_width const& in_right)
    const PSYQ_NOEXCEPT
    {
        // ビット領域のビット数で比較する。
        return status_property::get_format_bits(in_left) < in_right;
    }

    bool operator()(
        typename status_property::bit_width const& in_left,
        typename status_property::pack const& in_right)
    const PSYQ_NOEXCEPT
    {
        // ビット領域のビット数で比較する。
        return in_left < status_property::get_format_bits(in_right);
    }

}; // struct psyq::if_then_engine::_private::status_chunk::empty_field_less

#endif // defined(PSYQ_IF_THEN_ENGINE_STATUS_CHUNK_HPP_)
// vim: set expandtab:
