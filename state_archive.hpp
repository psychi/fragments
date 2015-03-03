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
    public: typedef std::int32_t format_type;

    public: enum kind: this_type::format_type
    {
        kind_SIGNED = -2,
        kind_FLOAT,
        kind_NULL,
        kind_BOOL,
        kind_UNSIGNED,
    };

    //-------------------------------------------------------------------------
    private: typedef std::uint32_t pos_type;
    private: typedef std::uint32_t size_type;

    private: struct empty_block
    {
        empty_block(
            state_archive::pos_type const in_position,
            state_archive::size_type const in_bit_size)
        PSYQ_NOEXCEPT:
        position(in_position),
        bit_size(in_bit_size)
        {}

        state_archive::pos_type position;
        state_archive::size_type bit_size;
    }; // struct empty_block

    private: struct empty_block_less
    {
        bool operator()(
            state_archive::empty_block const& in_left,
            state_archive::empty_block const& in_right)
        const
        {
            return in_left.bit_size != in_right.bit_size?
                in_left.bit_size < in_right.bit_size:
                in_left.position < in_right.position;
        }
    }; // empty_block_less

    private: typedef std::set<
        this_type::empty_block,
        this_type::empty_block_less,
        this_type::allocator_type>
            empty_block_set;

    private: struct key_hash
    {
        std::size_t operator()(state_archive::key_type const in_key) const
        {
            return in_key;
        }
        bool operator()(
            state_archive::key_type const in_left,
            state_archive::key_type const in_right)
        const
        {
            return in_left == in_right;
        }
    }; // struct key_hash

    private: struct record
    {
        state_archive::pos_type position;
        state_archive::format_type format;
    }; // struct record

    private: typedef std::unordered_map<
        this_type::key_type,
        this_type::record,
        this_type::key_hash,
        this_type::key_hash,
        this_type::allocator_type>
            record_map;

    private: typedef std::vector<std::uint64_t, this_type::allocator_type>
         unit_vector;
    private: typedef std::make_signed<this_type::unit_vector::value_type>::type
         signed_unit;

    private: enum: this_type::size_type
    {
        BITS_PER_BYTE = 8,
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
        auto const local_record(this->records_.find(in_key));
        if (local_record == this->records_.end())
        {
            return this_type::kind_NULL;
        }
        auto const local_format(local_record->second.format);
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
    public: std::size_t get_bit_size(this_type::key_type const in_key)
    const PSYQ_NOEXCEPT
    {
        auto const local_record(this->records_.find(in_key));
        if (local_record == this->records_.end())
        {
            return 0;
        }
        return this_type::get_format_bit_size(local_record->second.format);
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
        auto const local_record_iterator(this->records_.find(in_key));
        if (local_record_iterator == this->records_.end())
        {
            return false;
        }
        auto const& local_record(local_record_iterator->second);
        auto const local_bit_size(
            this_type::get_format_bit_size(local_record.format));
        auto local_bits(
            this_type::get_bits(
                this->units_, local_record.position, local_bit_size));
        switch (local_record.format)
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
        if (0 < local_record.format)
        {
            PSYQ_ASSERT(
                this_type::make_bit_mask(local_bit_size)
                <= static_cast<this_type::unit_vector::value_type>(
                    (std::numeric_limits<template_value>::max)()));
            this_type::copy_value(out_value, local_bits);
            return true;
        }

        // 有符号整数を取得する。
        PSYQ_ASSERT(
            (this_type::make_bit_mask(local_bit_size) >> 1)
            <= static_cast<this_type::unit_vector::value_type>(
                (std::numeric_limits<template_value>::max)()));
        if ((local_bits >> (local_bit_size - 1)) != 0)
        {
            // 符号ビットを拡張する。
            local_bits |= (
                (std::numeric_limits<this_type::unit_vector::value_type>::max)()
                << local_bit_size);
        }
        this_type::copy_value(
            out_value, static_cast<this_type::signed_unit>(local_bits));
        return true;
    }

    private: template<typename template_source>
    static void copy_value(
        bool& out_value,
        template_source const in_value)
    {
        out_value = (in_value != 0);
    }
    private: template<typename template_target, typename template_source>
    static void copy_value(
        template_target& out_value,
        template_source const in_value)
    {
        out_value = static_cast<template_target>(in_value);
    }

    private: static this_type::unit_vector::value_type get_bits(
        this_type::unit_vector const& in_units,
        this_type::pos_type const in_position,
        this_type::size_type const in_bit_size)
    {
        auto const UNIT_BIT_SIZE(
            sizeof(this_type::unit_vector::value_type)
            * this_type::BITS_PER_BYTE);
        if (UNIT_BIT_SIZE < in_bit_size)
        {
            PSYQ_ASSERT(false);
            return 0;
        }
        auto const local_unit_index(in_position / UNIT_BIT_SIZE);
        if (in_units.size() <= local_unit_index)
        {
            PSYQ_ASSERT(false);
            return 0;
        }

        auto const local_mod_bit_size(
            in_position - local_unit_index * UNIT_BIT_SIZE);
        PSYQ_ASSERT(
            (in_bit_size < UNIT_BIT_SIZE && local_mod_bit_size < UNIT_BIT_SIZE)
            || (in_bit_size == UNIT_BIT_SIZE && local_mod_bit_size == 0));
        return (in_units.at(local_unit_index) >> local_mod_bit_size)
            & this_type::make_bit_mask(in_bit_size);
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
        auto const local_record_iterator(this->records_.find(in_key));
        if (local_record_iterator == this->records_.end())
        {
            return false;
        }
        auto const& local_record(local_record_iterator->second);
        switch (local_record.format)
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
                this->units_, local_record.position, 1, in_value);

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
        auto const local_bit_size(
            this_type::get_format_bit_size(local_record.format));

        // 無符号整数を設定する。
        if (0 < local_record.format)
        {
            return this_type::set_bits(
                this->units_,
                local_record.position,
                local_bit_size,
                static_cast<this_type::unit_vector::value_type>(in_value));
        }

        // 有符号整数を設定する。
        return this_type::set_signed(
            this->units_, local_record.position, local_bit_size, in_value);
    }

    private: static bool set_signed(
        this_type::unit_vector& io_units,
        this_type::pos_type const in_position,
        this_type::size_type const in_bit_size,
        bool const in_value)
    {
        return this_type::set_bits(
            io_units, in_position, in_bit_size, in_value);
    }

    private: template<typename template_value>
    static bool set_signed(
        this_type::unit_vector& io_units,
        this_type::pos_type const in_position,
        this_type::size_type const in_bit_size,
        template_value const in_value)
    {
        auto local_bits(
            static_cast<this_type::unit_vector::value_type>(
                static_cast<this_type::signed_unit>(in_value)));
        if (in_value < 0)
        {
            auto const local_bit_mask(this_type::make_bit_mask(in_bit_size));
            PSYQ_ASSERT((~local_bit_mask & local_bits) == ~local_bit_mask);
            local_bits &= local_bit_mask;
        }
        return this_type::set_bits(
            io_units, in_position, in_bit_size, local_bits);
    }

    private: static bool set_bits(
        this_type::unit_vector& io_units,
        this_type::pos_type const in_position,
        this_type::size_type const in_bit_size,
        this_type::unit_vector::value_type const in_value)
    {
        PSYQ_ASSERT((in_value >> in_bit_size) == 0);
        auto const UNIT_BIT_SIZE(
            sizeof(this_type::unit_vector::value_type)
            * this_type::BITS_PER_BYTE);
        if (UNIT_BIT_SIZE < in_bit_size)
        {
            PSYQ_ASSERT(false);
            return false;
        }
        auto const local_unit_index(in_position / UNIT_BIT_SIZE);
        if (io_units.size() <= local_unit_index)
        {
            PSYQ_ASSERT(false);
            return false;
        }

        auto const local_mod_bit_size(
            in_position - local_unit_index * UNIT_BIT_SIZE);
        PSYQ_ASSERT(local_mod_bit_size + in_bit_size <= UNIT_BIT_SIZE);
        auto const local_bit_mask(this_type::make_bit_mask(in_bit_size));
        auto& local_unit(io_units.at(local_unit_index));
        local_unit = (~(local_bit_mask << local_mod_bit_size) & local_unit)
            | ((in_value & local_bit_mask) << local_mod_bit_size);
        return true;
    }

    //-------------------------------------------------------------------------
    /** @brief 真偽型の状態値を追加する。
        @param[in] in_key           追加する状態値のキー。
        @param[in] in_initial_value 追加する状態値の初期値。
        @retval true  成功。状態値を追加した。
        @retval false 失敗。状態値を追加できなかった。
     */
    public: bool add_bool(
        this_type::key_type const in_key,
        bool const in_initial_value)
    {
        auto const local_record(
            this->make_record(in_key, this_type::kind_BOOL));
        if (local_record == nullptr)
        {
            return false;
        }
        return this_type::set_bits(
            this->units_, local_record->position, 1, in_initial_value);
    }

    /** @brief 符号なし整数型の状態値を追加する。
        @param[in] in_key           追加する状態値のキー。
        @param[in] in_initial_value 追加する状態値の初期値。
        @param[in] in_bit_size      状態値のビット数。
        @retval true  成功。状態値を追加した。
        @retval false 失敗。状態値を追加できなかった。
     */
    public: bool add_unsigned(
        this_type::key_type const in_key,
        this_type::unit_vector::value_type const in_initial_value,
        std::size_t const in_bit_size =
            sizeof(this_type::unit_vector::value_type)
            * this_type::BITS_PER_BYTE)
    {
        auto const UNIT_BIT_SIZE(
            sizeof(this_type::unit_vector::value_type)
            * this_type::BITS_PER_BYTE);
        auto const local_format(
            static_cast<this_type::format_type>(in_bit_size));
        if (UNIT_BIT_SIZE < in_bit_size
            || local_format < this_type::kind_UNSIGNED)
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
            local_record->position,
            static_cast<this_type::size_type>(in_bit_size),
            in_initial_value);
    }

    /** @brief 符号あり整数型の状態値を追加する。
        @param[in] in_key           追加する状態値のキー。
        @param[in] in_initial_value 追加する状態値の初期値。
        @param[in] in_bit_size      状態値のビット数。
        @retval true  成功。状態値を追加した。
        @retval false 失敗。状態値を追加できなかった。
     */
    public: bool add_signed(
        this_type::key_type const in_key,
        this_type::signed_unit const in_initial_value,
        std::size_t const in_bit_size =
            sizeof(this_type::signed_unit) * this_type::BITS_PER_BYTE)
    {
        auto const UNIT_BIT_SIZE(
            sizeof(this_type::unit_vector::value_type)
            * this_type::BITS_PER_BYTE);
        auto const local_format(
            -static_cast<this_type::format_type>(in_bit_size));
        if (UNIT_BIT_SIZE < in_bit_size
            || this_type::kind_SIGNED < local_format)
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
            local_record->position,
            static_cast<this_type::size_type>(in_bit_size),
            in_initial_value);
    }

    //-------------------------------------------------------------------------
    public: void shrink_to_fit()
    {
        // 状態値を大きさの降順で並び替える。
        std::vector<this_type::record_map::value_type const*> local_records(
            this->records_.get_allocator());
        local_records.reserve(this->records_.size());
        for (auto& local_record: this->records_)
        {
            local_records.push_back(&local_record);
        }
        struct record_size_greater
        {
            bool operator()(
                this_type::record_map::value_type const* const in_left,
                this_type::record_map::value_type const* const in_right)
            const
            {
                return this_type::get_format_bit_size(in_right->second.format)
                    <  this_type::get_format_bit_size(in_left->second.format);
            }
        };
        std::sort(
            local_records.begin(), local_records.end(), record_size_greater());

        // 新たな書庫を構築する。
        this_type local_states(this->records_.get_allocator());
        for (auto local_record: local_records)
        {
            switch (local_record->second.format)
            {
                case this_type::kind_NULL:
                PSYQ_ASSERT(false);
                continue;

                case this_type::kind_BOOL:
                local_states.add_bool(
                    local_record->first,
                    0 != this_type::get_bits(
                        this->units_, local_record->second.position, 1));
                continue;

                case this_type::kind_FLOAT:
                PSYQ_ASSERT(false);
                continue;

                default:
                break;
            }
            auto const local_bit_size(
                this_type::get_format_bit_size(local_record->second.format));
            auto const local_bits(
                this_type::get_bits(
                    this->units_,
                    local_record->second.position,
                    local_bit_size));
            if (0 < local_record->second.format)
            {
                local_states.add_unsigned(
                    local_record->first, local_bits, local_bit_size);
            }
            else
            {
                local_states.add_signed(
                    local_record->first, local_bits, local_bit_size);
            }
        }

        // 新たに構築した書庫を移動する。
        *this = std::move(local_states);
        this->units_.shrink_to_fit();
    }

    //-------------------------------------------------------------------------
    private: this_type::record_map::mapped_type* make_record(
        this_type::key_type const in_key,
        this_type::format_type const in_format)
    {
        if (this->records_.find(in_key) != this->records_.end())
        {
            return nullptr;
        }
        auto const local_emplace(
            this->records_.emplace(in_key, this_type::record()));
        if (!local_emplace.second)
        {
            PSYQ_ASSERT(false);
            return nullptr;
        }
        auto& local_record(local_emplace.first->second);
        local_record.format = in_format;
        auto const local_bit_size(this_type::get_format_bit_size(in_format));

        // ビット配列を用意する。
        auto const local_empty_block(
            this->empty_blocks_.lower_bound(
                this_type::empty_block(0, local_bit_size)));
        if (local_empty_block == this->empty_blocks_.end()
            || local_empty_block->bit_size < local_bit_size)
        {
            // 新たにビット配列を追加する。
            auto const UNIT_BIT_SIZE(
                sizeof(this_type::unit_vector::value_type)
                * this_type::BITS_PER_BYTE);
            auto const local_record_position(
                this->units_.size() * UNIT_BIT_SIZE);
            local_record.position = static_cast<this_type::pos_type>(
                local_record_position);
            PSYQ_ASSERT(local_record.position == local_record_position);
            auto const local_add_unit_size(
                (local_bit_size + UNIT_BIT_SIZE - 1) / UNIT_BIT_SIZE);
            this->units_.insert(
                this->units_.end(), local_add_unit_size, 0);

            // 空き領域を追加する。
            auto const local_add_bit_size(local_add_unit_size * UNIT_BIT_SIZE);
            if (local_bit_size < local_add_bit_size)
            {
                this->empty_blocks_.emplace(
                    local_record.position + local_bit_size,
                    local_add_bit_size - local_bit_size);
            }
        }
        else
        {
            // 既存のビット配列を使う。
            local_record.position = local_empty_block->position;

            // 空き領域を更新する。
            if (local_bit_size < local_empty_block->bit_size)
            {
                this->empty_blocks_.emplace(
                    local_record.position + local_bit_size,
                    local_empty_block->bit_size - local_bit_size);
            }
            this->empty_blocks_.erase(local_empty_block);
        }
        return &local_record;
    }

    private: static this_type::size_type get_format_bit_size(
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

    private: static this_type::unit_vector::value_type make_bit_mask(
        this_type::size_type const in_bit_size)
    {
        auto const UNIT_BIT_SIZE(
            sizeof(this_type::unit_vector::value_type)
            * this_type::BITS_PER_BYTE);
        auto const local_max(
            (std::numeric_limits<this_type::unit_vector::value_type>::max)());
        return in_bit_size < UNIT_BIT_SIZE?
            ~(local_max << in_bit_size): local_max;
    }

    //-------------------------------------------------------------------------
    private: this_type::empty_block_set empty_blocks_;
    private: this_type::record_map records_;
    private: this_type::unit_vector units_;

}; // class psyq::state_archive

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq_test
{
    inline void state_archive()
    {
        psyq::state_archive local_states;

        std::int32_t local_signed(0);
        std::uint32_t local_unsigned(0);
        for (int i(2); i < 32; ++i)
        {
            PSYQ_ASSERT(local_states.add_unsigned(i, i - 1, i));
            PSYQ_ASSERT(local_states.get_value(i, local_unsigned));
            PSYQ_ASSERT(local_unsigned == i - 1);

            PSYQ_ASSERT(local_states.add_signed(-i, 1 - i, i));
            PSYQ_ASSERT(local_states.get_value(-i, local_signed));
            PSYQ_ASSERT(local_signed == 1 - i);
        }
        local_states.shrink_to_fit();
        for (int i(2); i < 32; ++i)
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
