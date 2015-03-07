/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief
 */
#ifndef PSYQ_STATE_ARCHIVE_HPP_
#define PSYQ_STATE_ARCHIVE_HPP_

#include <vector>
#include <set>
#include <unordered_map>
#include <type_traits>

namespace psyq
{
    template<typename> class state_archive;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 状態値書庫。任意のビット長の状態値を管理する。

    使い方の概略。
    - 以下の関数で、状態値を登録する。
      - state_archive::add_bool
      - state_archive::add_unsigned
      - state_archive::add_signed
    - state_archive::get_value で、状態値を取得する。
    - state_archive::set_value で、状態値を更新する。

    @tparam template_allocator @copydoc allocator_type
 */
template<typename template_allocator = std::allocator<void*>>
class psyq::state_archive
{
    /// thisが指す値の型。
    private: typedef state_archive this_type;

    //-------------------------------------------------------------------------
    /// @brief 各種コンテナに用いるメモリ割当子の型。
    public: typedef template_allocator allocator_type;

    /// @brief 識別番号を表す型。
    public: typedef std::uint32_t key_type;

    /// @brief 状態値の構成を表す型。
    public: typedef std::int8_t format_type;

    /// @brief 状態値の型の種別。
    public: enum kind: typename this_type::format_type
    {
        kind_SIGNED = -2, ///< 有符号整数。
        kind_FLOAT,       ///< 浮動小数点数。
        kind_NULL,        ///< 空。
        kind_BOOL,        ///< 真偽値。
        kind_UNSIGNED,    ///< 無符号整数。
    };

    //-------------------------------------------------------------------------
    /// @brief 状態値のビット数を表す型。
    private: typedef
        typename std::make_unsigned<typename this_type::format_type>::type
            size_type;

    /// @brief 状態値のビット位置を表す型。
    private: typedef std::uint32_t pos_type;

    /// @brief 状態値のビット位置とビット数を表す型。
    private: typedef typename this_type::pos_type field_type;

    /// @brief 空きビット領域を比較する関数オブジェクト。
    private: struct empty_field_less
    {
        bool operator()(
            typename state_archive::field_type const in_left,
            typename state_archive::field_type const in_right)
        const PSYQ_NOEXCEPT
        {
            // ビット領域のビット数で比較する。
            auto const local_left_size(
                state_archive::get_field_size(in_left));
            auto const local_right_size(
                state_archive::get_field_size(in_right));
            if (local_left_size != local_right_size)
            {
                return local_left_size < local_right_size;
            }

            // ビット領域のビット位置で比較する。
            auto const local_left_position(
                state_archive::get_field_position(in_left));
            auto const local_right_position(
                state_archive::get_field_position(in_right));
            return local_left_position < local_right_position;
        }

        bool operator()(
            typename state_archive::field_type const in_left,
            typename state_archive::size_type const in_right)
        const PSYQ_NOEXCEPT
        {
            // ビット領域のビット数で比較する。
            auto const local_left_size(
                state_archive::get_field_size(in_left));
            return local_left_size < in_right;
        }

        bool operator()(
            typename state_archive::size_type const in_left,
            typename state_archive::field_type const in_right)
        const PSYQ_NOEXCEPT
        {
            // ビット領域のビット数で比較する。
            auto const local_right_size(
                state_archive::get_field_size(in_right));
            return in_left < local_right_size;
        }

    }; // struct empty_field_less

    /// @brief 空きビット領域のコンテナ。
    private: typedef std::vector<
         typename this_type::field_type, typename this_type::allocator_type>
            empty_field_vector;

    //-------------------------------------------------------------------------
    /// @brief ビット列ブロックを表す型。
    private: typedef std::uint64_t block_type;

    /// @brief 符号つきのビット列ブロックを表す型。
    private: typedef
         typename std::make_signed<typename this_type::block_type>::type
             signed_block_type;

    /// @brief ビット列ブロックを格納するコンテナ。
    private: typedef
         std::vector<
             typename this_type::block_type,
             typename this_type::allocator_type>
                 block_vector;

    //-------------------------------------------------------------------------
    /// @brief 状態値の登記。
    private: struct entry
    {
        /// 状態値の識別番号。
        typename state_archive::key_type key;
        /// 状態値が格納されているビット領域。
        typename state_archive::field_type field;
    };

    /// @brief 状態値の識別番号を比較する関数オブジェクト。
    private: struct entry_key_less
    {
        bool operator()(
            typename state_archive::entry const& in_left,
            typename state_archive::entry const& in_right)
        const PSYQ_NOEXCEPT
        {
            return in_left.key < in_right.key;
        }

        bool operator()(
            typename state_archive::key_type const in_left,
            typename state_archive::entry const& in_right)
        const PSYQ_NOEXCEPT
        {
            return in_left < in_right.key;
        }

        bool operator()(
            typename state_archive::entry const& in_left,
            typename state_archive::key_type const in_right)
        const PSYQ_NOEXCEPT
        {
            return in_left.key < in_right;
        }

    }; // struct entry_key_less

    /// @brief 状態値登記のコンテナ。　
    private: typedef std::vector<
         typename this_type::entry, typename this_type::allocator_type>
             entry_vector;

    private: enum: typename this_type::size_type
    {
        BITS_PER_BYTE = 8,
        FIELD_POSITION_SIZE = 24,
    };

    //-------------------------------------------------------------------------
    public: state_archive(
        std::size_t const in_reserved_entry_size,
        std::size_t const in_reserved_block_size,
        std::size_t const in_reserved_empty_field_size,
        typename this_type::allocator_type const& in_allocator =
            this_type::allocator_type())
    :
    entries_(in_allocator),
    blocks_(in_allocator),
    empty_fields_(in_allocator)
    {
        this->entries_.reserve(in_reserved_entry_size);
        this->blocks_.reserve(in_reserved_block_size);
        this->empty_fields_.reserve(in_reserved_empty_field_size);
    }

    public: state_archive(this_type&& io_source) PSYQ_NOEXCEPT:
    entries_(std::move(io_source.entries_)),
    blocks_(std::move(io_source.blocks_)),
    empty_fields_(std::move(io_source.empty_fields_))
    {}

    public: this_type& operator=(this_type&& io_source) PSYQ_NOEXCEPT
    {
        this->entries_ = std::move(io_source.entries_);
        this->blocks_ = std::move(io_source.blocks_);
        this->empty_fields_ = std::move(io_source.empty_fields_);
        return *this;
    }

    //-------------------------------------------------------------------------
    /** @brief 状態値の型の種別を取得する。
        @param[in] in_key 取得する状態値の識別番号。
        @return in_key に対応する状態値の型の種別。
     */
    public: typename this_type::kind get_kind(
        typename this_type::key_type const in_key)
    const PSYQ_NOEXCEPT
    {
        auto const local_entry(
            this_type::find_entry(this->entries_, in_key));
        if (local_entry == nullptr)
        {
            return this_type::kind_NULL;
        }
        auto const local_format(this_type::get_entry_format(*local_entry));
        switch (local_format)
        {
            case this_type::kind_NULL:
            PSYQ_ASSERT(false);
            // case this_type::kind_BOOLに続く。
            case this_type::kind_BOOL:
            case this_type::kind_FLOAT:
            return static_cast<this_type::kind>(local_format);

            default:
            return local_format < 0?
                this_type::kind_SIGNED: this_type::kind_UNSIGNED;
        }
    }

    /** @brief 状態値のビット数を取得する。
        @param[in] in_key 取得する状態値の識別番号。
        @return in_key に対応する状態値のビット数。
     */
    public: std::size_t get_size(typename this_type::key_type const in_key)
    const PSYQ_NOEXCEPT
    {
        auto const local_entry(
            this_type::find_entry(this->entries_, in_key));
        if (local_entry == nullptr)
        {
            return 0;
        }
        return this_type::get_entry_size(*local_entry);
    }

    //-------------------------------------------------------------------------
    /** @brief 状態値を取得する。
        @param[in] in_key 取得する状態値の識別番号。
        @param[out] out_value 取得した状態値の格納先。
        @retval true  成功。取得した状態値を out_value に格納した。
        @retval false 失敗。 out_value は変化しない。
     */
    public: template<typename template_value>
    bool get_value(
        typename this_type::key_type const in_key,
        template_value& out_value)
    const PSYQ_NOEXCEPT
    {
        auto const local_entry(
            this_type::find_entry(this->entries_, in_key));
        if (local_entry == nullptr)
        {
            return false;
        }
        auto const local_format(this_type::get_entry_format(*local_entry));
        auto const local_size(this_type::get_format_size(local_format));
        auto local_bits(
            this_type::get_bits(
                this->blocks_,
                this_type::get_entry_position(*local_entry),
                local_size));
        switch (local_format)
        {
            case this_type::kind_NULL:
            PSYQ_ASSERT(false);
            return false;

            // 真偽値を取得する。
            case this_type::kind_BOOL:
            out_value = (local_bits != 0);
            return true;

            // 浮動小数点数を取得する。
            case this_type::kind_FLOAT:
            /// @todo 浮動小数点数の取得は未実装。
            PSYQ_ASSERT(false);
            return false;

            default:
            break;
        }

        // 無符号整数を取得する。
        if (0 < local_format)
        {
            PSYQ_ASSERT(
                this_type::make_block_mask(local_size)
                <= static_cast<typename this_type::block_type>(
                    (std::numeric_limits<template_value>::max)()));
            this_type::copy_value(out_value, local_bits);
            return true;
        }

        // 有符号整数を取得する。
        PSYQ_ASSERT(
            (this_type::make_block_mask(local_size) >> 1)
            <= static_cast<typename this_type::block_type>(
                (std::numeric_limits<template_value>::max)()));
        if ((local_bits >> (local_size - 1)) != 0)
        {
            // 符号ビットを拡張する。
            local_bits |= (
                (std::numeric_limits<typename this_type::block_type>::max)()
                << local_size);
        }
        this_type::copy_value(
            out_value,
            static_cast<typename this_type::signed_block_type>(local_bits));
        return true;
    }

    private: template<typename template_source>
    static void copy_value(
        bool& out_value,
        template_source const in_value)
    PSYQ_NOEXCEPT
    {
        out_value = (in_value != 0);
    }
    private: template<typename template_target, typename template_source>
    static void copy_value(
        template_target& out_value,
        template_source const in_value)
    PSYQ_NOEXCEPT
    {
        out_value = static_cast<template_target>(in_value);
    }

    private: static typename this_type::block_type get_bits(
        typename this_type::block_vector const& in_blocks,
        typename this_type::pos_type const in_position,
        typename this_type::size_type const in_size)
    PSYQ_NOEXCEPT
    {
        auto const BLOCK_SIZE(
            sizeof(typename this_type::block_type) * this_type::BITS_PER_BYTE);
        if (BLOCK_SIZE < in_size)
        {
            PSYQ_ASSERT(false);
            return 0;
        }
        auto const local_block_index(in_position / BLOCK_SIZE);
        if (in_blocks.size() <= local_block_index)
        {
            PSYQ_ASSERT(false);
            return 0;
        }

        auto const local_mod_size(in_position - local_block_index * BLOCK_SIZE);
        PSYQ_ASSERT(
            (in_size < BLOCK_SIZE && local_mod_size < BLOCK_SIZE)
            || (in_size == BLOCK_SIZE && local_mod_size == 0));
        return (in_blocks.at(local_block_index) >> local_mod_size)
            & this_type::make_block_mask(in_size);
    }

    //-------------------------------------------------------------------------
    /** @brief 状態値を設定する。
        @param[in] in_key   設定する状態値の識別番号。
        @param[in] in_value 設定する値。
        @retval true  成功。状態値を設定した。
        @retval false 失敗。状態値は変化しない。
     */
    public: template<typename template_value>
    bool set_value(
        typename this_type::key_type const in_key,
        template_value const in_value)
    PSYQ_NOEXCEPT
    {
        auto const local_entry(
            this_type::find_entry(this->entries_, in_key));
        if (local_entry == nullptr)
        {
            return false;
        }
        auto const local_format(this_type::get_entry_format(*local_entry));
        auto const local_position(
            this_type::get_entry_position(*local_entry));
        switch (local_format)
        {
            case this_type::kind_NULL:
            PSYQ_ASSERT(false);
            return false;

            // 真偽値を設定する。
            case this_type::kind_BOOL:
            if (!std::is_same<bool, template_value>::value)
            {
                return false;
            }
            return this_type::set_bits(
                this->blocks_, local_position, 1, in_value);

            // 浮動小数点数を設定する。
            case this_type::kind_FLOAT:
            /// @todo 浮動小数点数の設定は未実装。
            PSYQ_ASSERT(false);
            return false;

            default:
            break;
        }
        if (!std::is_integral<template_value>::value)
        {
            return false;
        }
        auto const local_size(this_type::get_format_size(local_format));

        // 無符号整数を設定する。
        if (0 < local_format)
        {
            return this_type::set_bits(
                this->blocks_,
                local_position,
                local_size,
                static_cast<typename this_type::block_type>(in_value));
        }

        // 有符号整数を設定する。
        return this_type::set_signed(
            this->blocks_, local_position, local_size, in_value);
    }

    private: static bool set_signed(
        typename this_type::block_vector& io_blocks,
        typename this_type::pos_type const in_position,
        typename this_type::size_type const in_size,
        bool const in_value)
    PSYQ_NOEXCEPT
    {
        return this_type::set_bits(io_blocks, in_position, in_size, in_value);
    }

    private: template<typename template_value>
    static bool set_signed(
        typename this_type::block_vector& io_blocks,
        typename this_type::pos_type const in_position,
        typename this_type::size_type const in_size,
        template_value const in_value)
    PSYQ_NOEXCEPT
    {
        auto local_bits(
            static_cast<typename this_type::block_type>(
                static_cast<typename this_type::signed_block_type>(in_value)));
        if (in_value < 0)
        {
            auto const local_mask(this_type::make_block_mask(in_size));
            PSYQ_ASSERT((~local_mask & local_bits) == ~local_mask);
            local_bits &= local_mask;
        }
        return this_type::set_bits(io_blocks, in_position, in_size, local_bits);
    }

    private: static bool set_bits(
        typename this_type::block_vector& io_blocks,
        typename this_type::pos_type const in_position,
        typename this_type::size_type const in_size,
        typename this_type::block_type const in_value)
    PSYQ_NOEXCEPT
    {
        PSYQ_ASSERT((in_value >> in_size) == 0);
        auto const BLOCK_SIZE(
            sizeof(typename this_type::block_type) * this_type::BITS_PER_BYTE);
        if (BLOCK_SIZE < in_size)
        {
            PSYQ_ASSERT(false);
            return false;
        }
        auto const local_block_index(in_position / BLOCK_SIZE);
        if (io_blocks.size() <= local_block_index)
        {
            PSYQ_ASSERT(false);
            return false;
        }

        auto const local_mod_size(
            in_position - local_block_index * BLOCK_SIZE);
        PSYQ_ASSERT(local_mod_size + in_size <= BLOCK_SIZE);
        auto const local_mask(this_type::make_block_mask(in_size));
        auto& local_block(io_blocks.at(local_block_index));
        local_block = (~(local_mask << local_mod_size) & local_block)
            | ((in_value & local_mask) << local_mod_size);
        return true;
    }

    //-------------------------------------------------------------------------
    /** @brief 真偽型の状態値を追加する。
        @param[in] in_chunk 追加する状態値が所属するチャンクの識別番号。
        @param[in] in_key   追加する状態値の識別番号。
        @param[in] in_value 追加する状態値の初期値。
        @retval true  成功。状態値を追加した。
        @retval false 失敗。状態値を追加できなかった。
     */
    public: bool add_bool(
        typename this_type::key_type const in_chunk,
        typename this_type::key_type const in_key,
        bool const in_value)
    {
        auto const local_entry(
            this->make_entry(in_chunk, in_key, this_type::kind_BOOL));
        if (local_entry == nullptr)
        {
            return false;
        }
        return this_type::set_bits(
            this->blocks_,
            this_type::get_entry_position(*local_entry),
            1,
            in_value);
    }

    /** @brief 符号なし整数型の状態値を追加する。
        @param[in] in_chunk 追加する状態値が所属するチャンクの識別番号。
        @param[in] in_key   追加する状態値の識別番号。
        @param[in] in_value 追加する状態値の初期値。
        @param[in] in_size  状態値のビット数。
        @retval true  成功。状態値を追加した。
        @retval false 失敗。状態値を追加できなかった。
     */
    public: bool add_unsigned(
        typename this_type::key_type const in_chunk,
        typename this_type::key_type const in_key,
        typename this_type::block_type const in_value,
        std::size_t const in_size =
            sizeof(typename this_type::block_type) * this_type::BITS_PER_BYTE)
    {
        auto const BLOCK_SIZE(
            sizeof(typename this_type::block_type) * this_type::BITS_PER_BYTE);
        auto const local_format(
            static_cast<typename this_type::format_type>(in_size));
        if (BLOCK_SIZE < in_size || local_format < this_type::kind_UNSIGNED)
        {
            return false;
        }
        auto const local_entry(
            this->make_entry(in_chunk, in_key, local_format));
        if (local_entry == nullptr)
        {
            return false;
        }
        return this_type::set_bits(
            this->blocks_,
            this_type::get_entry_position(*local_entry),
            static_cast<typename this_type::size_type>(in_size),
            in_value);
    }

    /** @brief 符号あり整数型の状態値を追加する。
        @param[in] in_chunk 追加する状態値が所属するチャンクの識別番号。
        @param[in] in_key   追加する状態値の識別番号。
        @param[in] in_value 追加する状態値の初期値。
        @param[in] in_size  状態値のビット数。
        @retval true  成功。状態値を追加した。
        @retval false 失敗。状態値を追加できなかった。
     */
    public: bool add_signed(
        typename this_type::key_type const in_chunk,
        typename this_type::key_type const in_key,
        typename this_type::signed_block_type const in_value,
        std::size_t const in_size =
            sizeof(typename this_type::signed_block_type)
            * this_type::BITS_PER_BYTE)
    {
        auto const BLOCK_SIZE(
            sizeof(typename this_type::block_type) * this_type::BITS_PER_BYTE);
        auto const local_format(
            -static_cast<typename this_type::format_type>(in_size));
        if (BLOCK_SIZE < in_size || this_type::kind_SIGNED < local_format)
        {
            return false;
        }
        auto const local_entry(
            this->make_entry(in_chunk, in_key, local_format));
        if (local_entry == nullptr)
        {
            return false;
        }
        return this_type::set_signed(
            this->blocks_,
            this_type::get_entry_position(*local_entry),
            static_cast<typename this_type::size_type>(in_size),
            in_value);
    }

    //-------------------------------------------------------------------------
    /// @brief コンテナが使うメモリ領域を必要最小限にする。
    public: void shrink_to_fit()
    {
        // 状態値を大きさの降順で並び替える。
        std::vector<typename this_type::entry_vector::value_type const*>
            local_entries(this->entries_.get_allocator());
        local_entries.reserve(this->entries_.size());
        for (auto& local_entry: this->entries_)
        {
            local_entries.push_back(&local_entry);
        }
        struct entry_size_greater
        {
            bool operator()(
                typename this_type::entry_vector::value_type const* const in_left,
                typename this_type::entry_vector::value_type const* const in_right)
            const
            {
                return this_type::get_entry_size(*in_right)
                    <  this_type::get_entry_size(*in_left);
            }
        };
        std::sort(
            local_entries.begin(), local_entries.end(), entry_size_greater());

        // 新たな書庫を構築する。
        this_type local_states(
            this->entries_.size(),
            this->blocks_.size(),
            this->empty_fields_.size(),
            this->entries_.get_allocator());
        for (auto local_entry: local_entries)
        {
            auto const local_position(
                this_type::get_entry_position(*local_entry));
            auto const local_format(
                this_type::get_entry_format(*local_entry));
            unsigned const local_chunk(0);
            switch (local_format)
            {
                case this_type::kind_NULL:
                PSYQ_ASSERT(false);
                continue;

                case this_type::kind_BOOL:
                local_states.add_bool(
                    local_chunk,
                    local_entry->key,
                    this_type::get_bits(this->blocks_, local_position, 1) != 0);
                continue;

                case this_type::kind_FLOAT:
                PSYQ_ASSERT(false);
                continue;

                default:
                break;
            }
            auto const local_size(this_type::get_format_size(local_format));
            auto const local_bits(
                this_type::get_bits(
                    this->blocks_, local_position, local_size));
            if (0 < local_format)
            {
                local_states.add_unsigned(
                    local_chunk, local_entry->key, local_bits, local_size);
            }
            else
            {
                local_states.add_signed(
                    local_chunk, local_entry->key, local_bits, local_size);
            }
        }

        // 新たに構築した書庫を移動する。
        *this = std::move(local_states);
        this->entries_.shrink_to_fit();
        this->blocks_.shrink_to_fit();
        this->empty_fields_.shrink_to_fit();
    }

    //-------------------------------------------------------------------------
    /** @brief 状態値の登記を新たに生成する。
        @param[in] in_chunk  新たに生成する状態値が所属するチャンクの識別番号。
        @param[in] in_key    新たに生成する状態値の識別番号。
        @param[in] in_format 新たに生成する状態値登記の構成。
        @retval !=nullptr 成功。新たに生成した状態値登記。
        @retval ==nullptr 失敗。状態値登記は生成されなかった。
     */
    private: typename this_type::entry_vector::value_type* make_entry(
        typename this_type::key_type const in_chunk,
        typename this_type::key_type const in_key,
        typename this_type::format_type const in_format)
    {
        // in_key と同じ状態値登記がないことを確認する。
        auto const local_entry_iterator(
            std::lower_bound(
                this->entries_.begin(),
                this->entries_.end(),
                in_key,
                this_type::entry_key_less()));
        if (local_entry_iterator != this->entries_.end()
            && local_entry_iterator->key == in_key)
        {
            return nullptr;
        }

        // 状態値の登記を新たに追加する。
        auto& local_entry(
            *(this->entries_.insert(
                local_entry_iterator,
                this_type::entry_vector::value_type())));
        local_entry.key = in_key;
        this_type::set_entry_format(local_entry, in_format);

        // 状態値のビット位置を決定する。
        auto const local_set_entry_position(
            this_type::set_entry_position(
                local_entry,
                this_type::make_state_field(
                    this_type::get_format_size(in_format),
                    this->empty_fields_,
                    this->blocks_)));
        if (!local_set_entry_position)
        {
            PSYQ_ASSERT(false);
            local_entry.field = 0;
            return nullptr;
        }
        return &local_entry;
    }

    /** @brief 状態値の領域を生成する。
        @param[in] in_size             生成する領域のビット数。
        @param[in,out] io_empty_fields 空き領域のコンテナ。
        @param[in,out] io_blocks       状態値の領域に使うブロックのコンテナ。
        @return 生成した領域のビット位置。
     */
    private: static std::size_t make_state_field(
        typename this_type::size_type const in_size,
        typename this_type::empty_field_vector& io_empty_fields,
        typename this_type::block_vector& io_blocks)
    {
        // 状態値を格納するビット領域を用意する。
        auto const local_empty_field(
            std::lower_bound(
                io_empty_fields.begin(),
                io_empty_fields.end(),
                in_size,
                this_type::empty_field_less()));
        if (local_empty_field != io_empty_fields.end())
        {
            // 既存の空き領域を再利用する。
            return this_type::reuse_empty_field(
                in_size, io_empty_fields, local_empty_field);
        }
        else
        {
            // 新たな領域を追加する。
            return this_type::add_state_field(
                in_size, io_empty_fields, io_blocks);
        }
    }

    private: static std::size_t reuse_empty_field(
        typename this_type::size_type const in_size,
        typename this_type::empty_field_vector& io_empty_fields,
        typename this_type::empty_field_vector::iterator const in_empty_field)
    {
        // 既存の空き領域を再利用する。
        auto const local_empty_position(
            this_type::get_field_position(*in_empty_field));

        // 空き領域を更新する。
        auto const local_empty_size(
            this_type::get_field_size(*in_empty_field));
        io_empty_fields.erase(in_empty_field);
        if (in_size < local_empty_size)
        {
            this_type::add_empty_field(
                io_empty_fields,
                local_empty_position + in_size,
                local_empty_size - in_size);
        }
        return local_empty_position;
    }

    private: static std::size_t add_state_field(
        typename this_type::size_type const in_size,
        typename this_type::empty_field_vector& io_empty_fields,
        typename this_type::block_vector& io_blocks)
    {
        // 新たにビット列ブロックを追加する。
        auto const BLOCK_SIZE(
            sizeof(typename this_type::block_type) * this_type::BITS_PER_BYTE);
        auto const local_position(io_blocks.size() * BLOCK_SIZE);
        if ((local_position >> this_type::FIELD_POSITION_SIZE) == 0)
        {
            auto const local_add_block_size(
                (in_size + BLOCK_SIZE - 1) / BLOCK_SIZE);
            io_blocks.insert(io_blocks.end(), local_add_block_size, 0);

            // 空き領域を追加する。
            auto const local_add_size(local_add_block_size * BLOCK_SIZE);
            if (in_size < local_add_size)
            {
                this_type::add_empty_field(
                    io_empty_fields,
                    local_position + in_size,
                    local_add_size - in_size);
            }
        }
        else
        {
            PSYQ_ASSERT(false);
        }
        return local_position;
    }

    private: static void add_empty_field(
        typename this_type::empty_field_vector& io_empty_fields,
        typename this_type::pos_type const in_position,
        std::size_t const in_size)
    {
        auto const local_size(
            static_cast<typename this_type::size_type>(in_size));
        if ((in_position >> this_type::FIELD_POSITION_SIZE) != 0
            || local_size != in_size)
        {
            PSYQ_ASSERT(false);
            return;
        }
        typename this_type::field_type const local_empty_field(
           (local_size << this_type::FIELD_POSITION_SIZE) | in_position);
        io_empty_fields.insert(
            std::lower_bound(
                io_empty_fields.begin(),
                io_empty_fields.end(),
                local_empty_field,
                this_type::empty_field_less()),
            local_empty_field);
    }

    //-------------------------------------------------------------------------
    /** @brief 状態値の登記を検索する。
        @param[in] in_entries 状態値の登記を検索するコンテナ。
        @param[in] in_key     検索対象とな状態値の識別番号。
        @retval !=nullptr in_key に対応する状態値の登記。
        @retval ==nullptr in_key に対応する状態値の登記が見つからなかった。
     */
    private: static typename this_type::entry_vector::value_type const* find_entry(
        typename this_type::entry_vector const& in_entries,
        typename this_type::key_type const in_key)
    PSYQ_NOEXCEPT
    {
        auto const local_entry(
            std::lower_bound(
                in_entries.begin(),
                in_entries.end(),
                in_key,
                this_type::entry_key_less()));
        return local_entry != in_entries.end() && local_entry->key == in_key?
            &(*local_entry): nullptr;
    }

    /// @copydoc find_entry
    private: static typename this_type::entry_vector::value_type* find_entry(
        typename this_type::entry_vector& in_entries,
        typename this_type::key_type const in_key)
    PSYQ_NOEXCEPT
    {
        return const_cast<typename this_type::entry_vector::value_type*>(
            this_type::find_entry(
                const_cast<typename this_type::entry_vector const&>(in_entries),
                in_key));
    }

    /** @brief ビット領域のビット位置を取得する。
        @param[in] in_field ビット領域。
        @return ビット領域のビット位置。
     */
    private: static typename this_type::pos_type get_field_position(
        typename this_type::field_type const in_field)
    PSYQ_NOEXCEPT
    {
        return in_field & ((1 << this_type::FIELD_POSITION_SIZE) - 1);
    }

    /** @brief ビット領域のビット数を取得する。
        @param[in] in_field ビット領域。
        @return ビット領域のビット数。
     */
    private: static typename this_type::size_type get_field_size(
        typename this_type::field_type const in_field)
    PSYQ_NOEXCEPT
    {
        return static_cast<typename this_type::size_type>(
            in_field >> this_type::FIELD_POSITION_SIZE);
    }

    /** @brief 状態値の登記から、状態値のビット位置を取得する。
        @param[in] in_entry 状態値の登記。
        @return 状態値のビット位置。
     */
    private: static typename this_type::pos_type get_entry_position(
        typename this_type::entry_vector::value_type const& in_entry)
    PSYQ_NOEXCEPT
    {
        return this_type::get_field_position(in_entry.field);
    }

    /** @brief 状態値の登記に、状態値のビット位置を設定する。
        @param[in,out] io_entry ビット位置を設定する状態値の登記。
        @param[in] in_position  状態値に設定するビット位置。
     */
    private: static bool set_entry_position(
        typename this_type::entry_vector::value_type& io_entry,
        std::size_t const in_position)
    PSYQ_NOEXCEPT
    {
        if ((in_position >> this_type::FIELD_POSITION_SIZE) != 0)
        {
            return false;
        }
        auto const local_mask(
            (1 << this_type::FIELD_POSITION_SIZE) - 1);
        io_entry.field = (~local_mask & io_entry.field)
            | static_cast<typename this_type::field_type>(
                  local_mask & in_position);
        return true;
    }

    /** @brief 状態値の登記から、状態値の構成を取得する。
        @param[in] in_entry 状態値の登記。
        @return 状態値の構成。
     */
    private: static typename this_type::format_type get_entry_format(
        typename this_type::entry_vector::value_type const& in_entry)
    PSYQ_NOEXCEPT
    {
        return static_cast<typename this_type::format_type>(
            in_entry.field >> this_type::FIELD_POSITION_SIZE);
    }

    /** @brief 状態値の登記に、状態値の構成を設定する。
        @param[in,out] io_entry 構成を設定する状態値の登記。
        @param[in] in_format    状態値に設定する構成。
     */
    private: static void set_entry_format(
        typename this_type::entry_vector::value_type& io_entry,
        typename this_type::format_type const in_format)
    PSYQ_NOEXCEPT
    {
        auto const local_mask(
            ~((1 << this_type::FIELD_POSITION_SIZE) - 1));
        io_entry.field = (io_entry.field & local_mask)
            | (in_format << this_type::FIELD_POSITION_SIZE);
    }

    /** @brief 状態値の登記から、状態値のビット数を取得する。
        @param[in] in_entry 状態値の登記。
        @return 状態値のビット数。
     */
    private: static typename this_type::size_type get_entry_size(
        typename this_type::entry_vector::value_type const& in_entry)
    PSYQ_NOEXCEPT
    {
        return this_type::get_format_size(
            this_type::get_entry_format(in_entry));
    }

    /** @brief 状態値の構成から、状態値のビット数を取得する。
        @param[in] in_format 状態値の構成。
        @return 状態値のビット数。
     */
    private: static typename this_type::size_type get_format_size(
        typename this_type::format_type const in_format)
    PSYQ_NOEXCEPT
    {
        switch (in_format)
        {
            case this_type::kind_NULL:
            PSYQ_ASSERT(false);
            return 0;

            case this_type::kind_BOOL:
            return 1;

            case this_type::kind_FLOAT:
            return this_type::BITS_PER_BYTE * sizeof(float);

            default:
            return std::abs(in_format);
        }
    }

    private: static typename this_type::block_type make_block_mask(
        typename this_type::size_type const in_size)
    PSYQ_NOEXCEPT
    {
        auto const BLOCK_SIZE(
            sizeof(typename this_type::block_type) * this_type::BITS_PER_BYTE);
        auto const local_max(
            (std::numeric_limits<typename this_type::block_type>::max)());
        return in_size < BLOCK_SIZE? ~(local_max << in_size): local_max;
    }

    //-------------------------------------------------------------------------
    private: typename this_type::entry_vector entries_;
    private: typename this_type::block_vector blocks_;
    private: typename this_type::empty_field_vector empty_fields_;

}; // class psyq::state_archive

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq_test
{
    inline void state_archive()
    {
        psyq::state_archive<> local_states(128, 128, 128);

        std::int64_t local_signed(0);
        std::uint64_t local_unsigned(0);
        unsigned local_chunk(0);
        for (int i(2); i <= 64; ++i)
        {
            PSYQ_ASSERT(local_states.add_unsigned(local_chunk, i, i - 1, i));
            PSYQ_ASSERT(local_states.get_value(i, local_unsigned));
            PSYQ_ASSERT(local_unsigned == i - 1);

            PSYQ_ASSERT(local_states.add_signed(local_chunk, -i, 1 - i, i));
            PSYQ_ASSERT(local_states.get_value(-i, local_signed));
            PSYQ_ASSERT(local_signed == 1 - i);
        }
        local_states.shrink_to_fit();
        for (int i(2); i <= 64; ++i)
        {
            PSYQ_ASSERT(local_states.get_value(i, local_unsigned));
            PSYQ_ASSERT(local_unsigned == i - 1);
            PSYQ_ASSERT(local_states.set_value(i, local_unsigned));
            PSYQ_ASSERT(local_states.get_value(i, local_unsigned));
            PSYQ_ASSERT(local_unsigned == i - 1);

            PSYQ_ASSERT(local_states.get_value(-i, local_signed));
            PSYQ_ASSERT(local_signed == 1 - i);
            PSYQ_ASSERT(local_states.set_value(-i, local_signed));
            PSYQ_ASSERT(local_states.get_value(-i, local_signed));
            PSYQ_ASSERT(local_signed == 1 - i);
        }

        bool local_bool(false);
        PSYQ_ASSERT(local_states.add_bool(local_chunk, 1, true));
        PSYQ_ASSERT(local_states.get_value(1, local_bool));
        PSYQ_ASSERT(local_bool);
        PSYQ_ASSERT(local_states.set_value(1, local_bool));
    }
} // namespace psyq_test

#endif // !defined(PSYQ_STATE_ARCHIVE_HPP_)
