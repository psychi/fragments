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
    class state_archive;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::state_archive
{
    private: typedef state_archive this_type;

    //-------------------------------------------------------------------------
    public: typedef std::allocator<void*> allocator_type;

    public: typedef std::uint32_t key_type;
    public: typedef std::int8_t format_type;

    public: enum kind: this_type::format_type
    {
        kind_SIGNED = -2,
        kind_FLOAT,
        kind_NULL,
        kind_BOOL,
        kind_UNSIGNED,
    };

    //-------------------------------------------------------------------------
    private: typedef std::make_unsigned<this_type::format_type>::type
        size_type;
    private: typedef std::uint32_t pos_type;
    private: typedef this_type::pos_type block_type;

    private: struct empty_block_less
    {
        bool operator()(
            state_archive::block_type const in_left,
            state_archive::block_type const in_right)
        const PSYQ_NOEXCEPT
        {
            auto const local_left_size(
                state_archive::get_block_size(in_left));
            auto const local_right_size(
                state_archive::get_block_size(in_right));
            if (local_left_size != local_right_size)
            {
                return local_left_size < local_right_size;
            }
            auto const local_left_position(
                state_archive::get_block_position(in_left));
            auto const local_right_position(
                state_archive::get_block_position(in_right));
            return local_left_position < local_right_position;
        }

        bool operator()(
            state_archive::block_type const in_left,
            state_archive::size_type const in_right)
        const PSYQ_NOEXCEPT
        {
            auto const local_left_size(
                state_archive::get_block_size(in_left));
            return local_left_size < in_right;
        }

        bool operator()(
            state_archive::size_type const in_left,
            state_archive::block_type const in_right)
        const PSYQ_NOEXCEPT
        {
            auto const local_right_size(
                state_archive::get_block_size(in_right));
            return in_left < local_right_size;
        }

    }; // struct empty_block_less

    private: typedef
        std::vector<this_type::block_type, this_type::allocator_type>
            empty_block_vector;

    private: struct record
    {
        this_type::key_type key;
        this_type::block_type block;
    };

    private: struct record_key_less
    {
        bool operator()(
            state_archive::record const& in_left,
            state_archive::record const& in_right)
        const PSYQ_NOEXCEPT
        {
            return in_left.key < in_right.key;
        }

        bool operator()(
            state_archive::key_type const in_left,
            state_archive::record const& in_right)
        const PSYQ_NOEXCEPT
        {
            return in_left < in_right.key;
        }

        bool operator()(
            state_archive::record const& in_left,
            state_archive::key_type const in_right)
        const PSYQ_NOEXCEPT
        {
            return in_left.key < in_right;
        }

    }; // struct record_key_less

    private: typedef std::vector<this_type::record, this_type::allocator_type>
         record_vector;

    private: typedef std::vector<std::uint64_t, this_type::allocator_type>
         unit_vector;

    private: typedef std::make_signed<this_type::unit_vector::value_type>::type
         signed_unit;

    private: enum: this_type::size_type
    {
        BITS_PER_BYTE = 8,
        BLOCK_POSITION_SIZE = 24,
    };

    //-------------------------------------------------------------------------
    public: explicit state_archive(
        this_type::allocator_type const& in_allocator =
            this_type::allocator_type())
    PSYQ_NOEXCEPT:
    empty_blocks_(in_allocator),
    records_(in_allocator),
    units_(in_allocator)
    {}

    public: state_archive(this_type&& io_source) PSYQ_NOEXCEPT:
    empty_blocks_(std::move(io_source.empty_blocks_)),
    records_(std::move(io_source.records_)),
    units_(std::move(io_source.units_))
    {}

    public: this_type& operator=(this_type&& io_source) PSYQ_NOEXCEPT
    {
        this->empty_blocks_ = std::move(io_source.empty_blocks_);
        this->records_ = std::move(io_source.records_);
        this->units_ = std::move(io_source.units_);
        return *this;
    }

    //-------------------------------------------------------------------------
    /** @brief 状態値の型の種別を取得する。
        @param[in] in_key 取得する状態値のキー。
        @return in_key に対応する状態値の型の種別。
     */
    public: this_type::kind get_kind(this_type::key_type const in_key)
    const PSYQ_NOEXCEPT
    {
        auto const local_record(
            this_type::find_record(this->records_, in_key));
        if (local_record == nullptr)
        {
            return this_type::kind_NULL;
        }
        auto const local_format(this_type::get_record_format(*local_record));
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
        @param[in] in_key 取得する状態値のキー。
        @return in_key に対応する状態値のビット数。
     */
    public: std::size_t get_size(this_type::key_type const in_key)
    const PSYQ_NOEXCEPT
    {
        auto const local_record(
            this_type::find_record(this->records_, in_key));
        if (local_record == nullptr)
        {
            return 0;
        }
        return this_type::get_record_size(*local_record);
    }

    //-------------------------------------------------------------------------
    /** @brief 状態値を取得する。
        @param[in] in_key 取得する状態値のキー。
        @param[out] out_value 取得した状態値の格納先。
        @retval true  成功。取得した状態値を out_value に格納した。
        @retval false 失敗。 out_value は変化しない。
     */
    public: template<typename template_value>
    bool get_value(
        this_type::key_type const in_key,
        template_value& out_value)
    const PSYQ_NOEXCEPT
    {
        auto const local_record(
            this_type::find_record(this->records_, in_key));
        if (local_record == nullptr)
        {
            return false;
        }
        auto const local_format(this_type::get_record_format(*local_record));
        auto const local_size(this_type::get_format_size(local_format));
        auto local_bits(
            this_type::get_bits(
                this->units_,
                this_type::get_record_position(*local_record),
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
                this_type::make_unit_mask(local_size)
                <= static_cast<this_type::unit_vector::value_type>(
                    (std::numeric_limits<template_value>::max)()));
            this_type::copy_value(out_value, local_bits);
            return true;
        }

        // 有符号整数を取得する。
        PSYQ_ASSERT(
            (this_type::make_unit_mask(local_size) >> 1)
            <= static_cast<this_type::unit_vector::value_type>(
                (std::numeric_limits<template_value>::max)()));
        if ((local_bits >> (local_size - 1)) != 0)
        {
            // 符号ビットを拡張する。
            local_bits |= (
                (std::numeric_limits<this_type::unit_vector::value_type>::max)()
                << local_size);
        }
        this_type::copy_value(
            out_value, static_cast<this_type::signed_unit>(local_bits));
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

    private: static this_type::unit_vector::value_type get_bits(
        this_type::unit_vector const& in_units,
        this_type::pos_type const in_position,
        this_type::size_type const in_size)
    PSYQ_NOEXCEPT
    {
        auto const UNIT_SIZE(
            sizeof(this_type::unit_vector::value_type)
            * this_type::BITS_PER_BYTE);
        if (UNIT_SIZE < in_size)
        {
            PSYQ_ASSERT(false);
            return 0;
        }
        auto const local_unit_index(in_position / UNIT_SIZE);
        if (in_units.size() <= local_unit_index)
        {
            PSYQ_ASSERT(false);
            return 0;
        }

        auto const local_mod_size(in_position - local_unit_index * UNIT_SIZE);
        PSYQ_ASSERT(
            (in_size < UNIT_SIZE && local_mod_size < UNIT_SIZE)
            || (in_size == UNIT_SIZE && local_mod_size == 0));
        return (in_units.at(local_unit_index) >> local_mod_size)
            & this_type::make_unit_mask(in_size);
    }

    //-------------------------------------------------------------------------
    /** @brief 状態値を設定する。
        @param[in] in_key   設定する状態値のキー。
        @param[in] in_value 設定する値。
        @retval true  成功。状態値を設定した。
        @retval false 失敗。状態値は変化しない。
     */
    public: template<typename template_value>
    bool set_value(
        this_type::key_type const in_key,
        template_value const in_value)
    PSYQ_NOEXCEPT
    {
        auto const local_record(
            this_type::find_record(this->records_, in_key));
        if (local_record == nullptr)
        {
            return false;
        }
        auto const local_format(this_type::get_record_format(*local_record));
        auto const local_position(
            this_type::get_record_position(*local_record));
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
                this->units_, local_position, 1, in_value);

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
                this->units_,
                local_position,
                local_size,
                static_cast<this_type::unit_vector::value_type>(in_value));
        }

        // 有符号整数を設定する。
        return this_type::set_signed(
            this->units_, local_position, local_size, in_value);
    }

    private: static bool set_signed(
        this_type::unit_vector& io_units,
        this_type::pos_type const in_position,
        this_type::size_type const in_size,
        bool const in_value)
    PSYQ_NOEXCEPT
    {
        return this_type::set_bits(io_units, in_position, in_size, in_value);
    }

    private: template<typename template_value>
    static bool set_signed(
        this_type::unit_vector& io_units,
        this_type::pos_type const in_position,
        this_type::size_type const in_size,
        template_value const in_value)
    PSYQ_NOEXCEPT
    {
        auto local_bits(
            static_cast<this_type::unit_vector::value_type>(
                static_cast<this_type::signed_unit>(in_value)));
        if (in_value < 0)
        {
            auto const local_mask(this_type::make_unit_mask(in_size));
            PSYQ_ASSERT((~local_mask & local_bits) == ~local_mask);
            local_bits &= local_mask;
        }
        return this_type::set_bits(io_units, in_position, in_size, local_bits);
    }

    private: static bool set_bits(
        this_type::unit_vector& io_units,
        this_type::pos_type const in_position,
        this_type::size_type const in_size,
        this_type::unit_vector::value_type const in_value)
    PSYQ_NOEXCEPT
    {
        PSYQ_ASSERT((in_value >> in_size) == 0);
        auto const UNIT_SIZE(
            sizeof(this_type::unit_vector::value_type)
            * this_type::BITS_PER_BYTE);
        if (UNIT_SIZE < in_size)
        {
            PSYQ_ASSERT(false);
            return false;
        }
        auto const local_unit_index(in_position / UNIT_SIZE);
        if (io_units.size() <= local_unit_index)
        {
            PSYQ_ASSERT(false);
            return false;
        }

        auto const local_mod_size(in_position - local_unit_index * UNIT_SIZE);
        PSYQ_ASSERT(local_mod_size + in_size <= UNIT_SIZE);
        auto const local_mask(this_type::make_unit_mask(in_size));
        auto& local_unit(io_units.at(local_unit_index));
        local_unit = (~(local_mask << local_mod_size) & local_unit)
            | ((in_value & local_mask) << local_mod_size);
        return true;
    }

    //-------------------------------------------------------------------------
    /** @brief 真偽型の状態値を追加する。
        @param[in] in_key   追加する状態値のキー。
        @param[in] in_value 追加する状態値の初期値。
        @retval true  成功。状態値を追加した。
        @retval false 失敗。状態値を追加できなかった。
     */
    public: bool add_bool(
        this_type::key_type const in_key,
        bool const in_value)
    PSYQ_NOEXCEPT
    {
        auto const local_record(
            this->make_record(in_key, this_type::kind_BOOL));
        if (local_record == nullptr)
        {
            return false;
        }
        return this_type::set_bits(
            this->units_,
            this_type::get_record_position(*local_record),
            1,
            in_value);
    }

    /** @brief 符号なし整数型の状態値を追加する。
        @param[in] in_key   追加する状態値のキー。
        @param[in] in_value 追加する状態値の初期値。
        @param[in] in_size  状態値のビット数。
        @retval true  成功。状態値を追加した。
        @retval false 失敗。状態値を追加できなかった。
     */
    public: bool add_unsigned(
        this_type::key_type const in_key,
        this_type::unit_vector::value_type const in_value,
        std::size_t const in_size =
            sizeof(this_type::unit_vector::value_type)
            * this_type::BITS_PER_BYTE)
    PSYQ_NOEXCEPT
    {
        auto const UNIT_SIZE(
            sizeof(this_type::unit_vector::value_type)
            * this_type::BITS_PER_BYTE);
        auto const local_format(static_cast<this_type::format_type>(in_size));
        if (UNIT_SIZE < in_size || local_format < this_type::kind_UNSIGNED)
        {
            return false;
        }
        auto const local_record(this->make_record(in_key, local_format));
        if (local_record == nullptr)
        {
            return false;
        }
        return this_type::set_bits(
            this->units_,
            this_type::get_record_position(*local_record),
            static_cast<this_type::size_type>(in_size),
            in_value);
    }

    /** @brief 符号あり整数型の状態値を追加する。
        @param[in] in_key   追加する状態値のキー。
        @param[in] in_value 追加する状態値の初期値。
        @param[in] in_size  状態値のビット数。
        @retval true  成功。状態値を追加した。
        @retval false 失敗。状態値を追加できなかった。
     */
    public: bool add_signed(
        this_type::key_type const in_key,
        this_type::signed_unit const in_value,
        std::size_t const in_size =
            sizeof(this_type::signed_unit) * this_type::BITS_PER_BYTE)
    PSYQ_NOEXCEPT
    {
        auto const UNIT_SIZE(
            sizeof(this_type::unit_vector::value_type)
            * this_type::BITS_PER_BYTE);
        auto const local_format(
            -static_cast<this_type::format_type>(in_size));
        if (UNIT_SIZE < in_size || this_type::kind_SIGNED < local_format)
        {
            return false;
        }
        auto const local_record(this->make_record(in_key, local_format));
        if (local_record == nullptr)
        {
            return false;
        }
        return this_type::set_signed(
            this->units_,
            this_type::get_record_position(*local_record),
            static_cast<this_type::size_type>(in_size),
            in_value);
    }

    //-------------------------------------------------------------------------
    public: void shrink_to_fit()
    {
        // 状態値を大きさの降順で並び替える。
        std::vector<this_type::record_vector::value_type const*> local_records(
            this->records_.get_allocator());
        local_records.reserve(this->records_.size());
        for (auto& local_record: this->records_)
        {
            local_records.push_back(&local_record);
        }
        struct record_size_greater
        {
            bool operator()(
                this_type::record_vector::value_type const* const in_left,
                this_type::record_vector::value_type const* const in_right)
            const
            {
                return this_type::get_record_size(*in_right)
                    <  this_type::get_record_size(*in_left);
            }
        };
        std::sort(
            local_records.begin(), local_records.end(), record_size_greater());

        // 新たな書庫を構築する。
        this_type local_states(this->records_.get_allocator());
        for (auto local_record: local_records)
        {
            auto const local_position(
                this_type::get_record_position(*local_record));
            auto const local_format(
                this_type::get_record_format(*local_record));
            switch (local_format)
            {
                case this_type::kind_NULL:
                PSYQ_ASSERT(false);
                continue;

                case this_type::kind_BOOL:
                local_states.add_bool(
                    local_record->key,
                    0 != this_type::get_bits(this->units_, local_position, 1));
                continue;

                case this_type::kind_FLOAT:
                PSYQ_ASSERT(false);
                continue;

                default:
                break;
            }
            auto const local_size(this_type::get_format_size(local_format));
            auto const local_bits(
                this_type::get_bits(this->units_, local_position, local_size));
            if (0 < local_format)
            {
                local_states.add_unsigned(
                    local_record->key, local_bits, local_size);
            }
            else
            {
                local_states.add_signed(
                    local_record->key, local_bits, local_size);
            }
        }

        // 新たに構築した書庫を移動する。
        *this = std::move(local_states);
        this->empty_blocks_.shrink_to_fit();
        this->records_.shrink_to_fit();
        this->units_.shrink_to_fit();
    }

    //-------------------------------------------------------------------------
    private: this_type::record_vector::value_type* make_record(
        this_type::key_type const in_key,
        this_type::format_type const in_format)
    {
        auto const local_record_iterator(
            std::lower_bound(
                this->records_.begin(),
                this->records_.end(),
                in_key,
                this_type::record_key_less()));
        if (local_record_iterator != this->records_.end()
            && local_record_iterator->key == in_key)
        {
            return nullptr;
        }
        auto& local_record(
            *(this->records_.insert(
                local_record_iterator,
                this_type::record_vector::value_type())));
        local_record.key = in_key;
        this_type::set_record_format(local_record, in_format);
        auto const local_size(this_type::get_format_size(in_format));

        // ビット配列を用意する。
        auto const local_empty_block(
            std::lower_bound(
                this->empty_blocks_.begin(),
                this->empty_blocks_.end(),
                local_size,
                this_type::empty_block_less()));
        if (local_empty_block == this->empty_blocks_.end())
        {
            // 新たにビット配列を追加する。
            auto const UNIT_SIZE(
                sizeof(this_type::unit_vector::value_type)
                * this_type::BITS_PER_BYTE);
            auto const local_position(this->units_.size() * UNIT_SIZE);
            if (!this_type::set_record_position(local_record, local_position))
            {
                PSYQ_ASSERT(false);
                return nullptr;
            }
            auto const local_add_unit_size(
                (local_size + UNIT_SIZE - 1) / UNIT_SIZE);
            this->units_.insert(
                this->units_.end(), local_add_unit_size, 0);

            // 空き領域を追加する。
            auto const local_add_size(local_add_unit_size * UNIT_SIZE);
            if (local_size < local_add_size)
            {
                this_type::add_empty_block(
                    this->empty_blocks_,
                    local_position + local_size,
                    local_add_size - local_size);
            }
        }
        else
        {
            // 既存のビット配列を使う。
            auto const local_empty_position(
                this_type::get_block_position(*local_empty_block));
            auto const local_set_record_position(
                this_type::set_record_position(
                    local_record, local_empty_position));
            if (!local_set_record_position)
            {
                PSYQ_ASSERT(false);
                return nullptr;
            }

            // 空き領域を更新する。
            auto const local_empty_size(
                this_type::get_block_size(*local_empty_block));
            this->empty_blocks_.erase(local_empty_block);
            if (local_size < local_empty_size)
            {
                this_type::add_empty_block(
                    this->empty_blocks_,
                    local_empty_position + local_size,
                    local_empty_size - local_size);
            }
        }
        return &local_record;
    }

    private: static void add_empty_block(
        this_type::empty_block_vector& io_empty_blocks,
        this_type::pos_type const in_position,
        std::size_t const in_size)
    {
        auto const local_size(static_cast<this_type::size_type>(in_size));
        if ((in_position >> this_type::BLOCK_POSITION_SIZE) != 0
            || local_size != in_size)
        {
            PSYQ_ASSERT(false);
            return;
        }
        this_type::block_type const local_empty_block(
           (local_size << this_type::BLOCK_POSITION_SIZE) | in_position);
        io_empty_blocks.insert(
            std::lower_bound(
                io_empty_blocks.begin(),
                io_empty_blocks.end(),
                local_empty_block,
                this_type::empty_block_less()),
            local_empty_block);
    }

    //-------------------------------------------------------------------------
    private: static this_type::record_vector::value_type const* find_record(
        this_type::record_vector const& in_records,
        this_type::key_type const in_key)
    {
        auto const local_record(
            std::lower_bound(
                in_records.begin(),
                in_records.end(),
                in_key,
                this_type::record_key_less()));
        return local_record != in_records.end() && local_record->key == in_key?
            &(*local_record): nullptr;
    }

    private: static this_type::record_vector::value_type* find_record(
        this_type::record_vector& in_records,
        this_type::key_type const in_key)
    {
        return const_cast<this_type::record_vector::value_type*>(
            this_type::find_record(
                const_cast<this_type::record_vector const&>(in_records),
                in_key));
    }

    private: static this_type::pos_type get_block_position(
        this_type::block_type const in_block)
    {
        return in_block & ((1 << this_type::BLOCK_POSITION_SIZE) - 1);
    }

    private: static this_type::size_type get_block_size(
        this_type::block_type const in_block)
    {
        return static_cast<this_type::size_type>(
            in_block >> this_type::BLOCK_POSITION_SIZE);
    }

    private: static this_type::pos_type get_record_position(
        this_type::record_vector::value_type const& in_record)
    {
        return this_type::get_block_position(in_record.block);
    }

    private: static bool set_record_position(
        this_type::record_vector::value_type& io_record,
        std::size_t const in_position)
    {
        if ((in_position >> this_type::BLOCK_POSITION_SIZE) != 0)
        {
            return false;
        }
        auto const local_mask(
            (1 << this_type::BLOCK_POSITION_SIZE) - 1);
        io_record.block = (~local_mask & io_record.block)
            | static_cast<this_type::block_type>(local_mask & in_position);
        return true;
    }

    private: static this_type::format_type get_record_format(
        this_type::record_vector::value_type const& in_record)
    {
        return static_cast<this_type::format_type>(
            in_record.block >> this_type::BLOCK_POSITION_SIZE);
    }

    private: static void set_record_format(
        this_type::record_vector::value_type& io_record,
        this_type::format_type const in_format)
    {
        auto const local_mask(
            ~((1 << this_type::BLOCK_POSITION_SIZE) - 1));
        io_record.block = (io_record.block & local_mask)
            | (in_format << this_type::BLOCK_POSITION_SIZE);
    }

    private: static this_type::size_type get_record_size(
        this_type::record_vector::value_type const& in_record)
    {
        return this_type::get_format_size(
            this_type::get_record_format(in_record));
    }

    private: static this_type::size_type get_format_size(
        this_type::format_type const in_format)
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

    private: static this_type::unit_vector::value_type make_unit_mask(
        this_type::size_type const in_size)
    {
        auto const UNIT_SIZE(
            sizeof(this_type::unit_vector::value_type)
            * this_type::BITS_PER_BYTE);
        auto const local_max(
            (std::numeric_limits<this_type::unit_vector::value_type>::max)());
        return in_size < UNIT_SIZE? ~(local_max << in_size): local_max;
    }

    //-------------------------------------------------------------------------
    private: this_type::empty_block_vector empty_blocks_;
    private: this_type::record_vector records_;
    private: this_type::unit_vector units_;

}; // class psyq::state_archive

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq_test
{
    inline void state_archive()
    {
        psyq::state_archive local_states;

        std::int64_t local_signed(0);
        std::uint64_t local_unsigned(0);
        for (int i(2); i <= 64; ++i)
        {
            PSYQ_ASSERT(local_states.add_unsigned(i, i - 1, i));
            PSYQ_ASSERT(local_states.get_value(i, local_unsigned));
            PSYQ_ASSERT(local_unsigned == i - 1);

            PSYQ_ASSERT(local_states.add_signed(-i, 1 - i, i));
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
        PSYQ_ASSERT(local_states.add_bool(1, true));
        PSYQ_ASSERT(local_states.get_value(1, local_bool));
        PSYQ_ASSERT(local_bool);
        PSYQ_ASSERT(local_states.set_value(1, local_bool));
    }
} // namespace psyq_test

#endif // !defined(PSYQ_STATE_ARCHIVE_HPP_)
