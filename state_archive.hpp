/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief
 */
#ifndef PSYQ_STATE_ARCHIVE_HPP_
#define PSYQ_STATE_ARCHIVE_HPP_

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

    public: typedef std::int32_t format_type;

    public: enum kind: this_type::format_type
    {
        kind_SIGNED = -2,
        kind_DOUBLE,
        kind_FLOAT,
        kind_BOOL,
        kind_UNSIGNED,
    };

    public: typedef std::uint32_t key_type;
    public: typedef std::uint32_t pos_type;
    public: typedef std::uint32_t size_type;

    //-------------------------------------------------------------------------
    private: struct empty_block
    {
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
        empty_block block;
        state_archive::format_type format;
    }; // struct record

    private: typedef std::unorderd_map<
        this_type::key_type,
        this_type::record,
        this_type::key_hash,
        this_type::key_hash,
        this_type::allocator_type>
            record_map;

    private: typedef std::vector<std::uint32_t, this_type::allocator_type>
         unit_vector;

    private: enum: this_type::size_type
    {
        BITS_PER_BYTE = 8,
    };

    //-------------------------------------------------------------------------
    public: explicit state_archive(this_type::allocator_type const& in_allocator)
    PSYQ_NOEXCEPT:
    empty_blocks_(in_allocator),
    records_(in_allocator),
    units_(in_allocator)
    {}

    //-------------------------------------------------------------------------
    public: bool get_bool(
        this_type::key_type const in_key,
        bool& out_value)
    {
        auto const local_record(this->records_.find(in_key));
        if (local_record == this->records_.end()
            || local_record->format != this_type::kind_BOOL)
        {
            return false;
        }
        auto const local_get_bits(
            this_type::get_bits(
                this->units_, local_record->block.position, 1, local_bits));
        out_value = (local_bits != 0);
        return local_get_bits;
    }

    public: bool set_bool(
        this_type::key_type const in_key,
        bool const in_value)
    {
        auto const local_record(this->records_.find(in_key));
        if (local_record == this->records_.end()
            || local_record->format != this_type::kind_BOOL)
        {
            return false;
        }
        return this_type::set_bits(
            this->units_, local_record->block.position, 1, in_value);
    }

    //-------------------------------------------------------------------------
    public: bool insert_bool(
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
            this->units_, local_record.block.position, 1, in_initial_value);
    }

    public: template<typename template_unsigned>
    bool insert_unsigned(
        this_type::key_type const in_key,
        template_unsigned const in_initial_value,
        this_type::size_type const in_bit_size =
            sizeof(template_unsigned) * this_type::BITS_PER_BYTE)
    {
        static_assert(std::is_unsigned<template_unsigned>::value, "");
        auto const local_format(
            static_cast<this_type::format_type>(in_bit_size));
        if (local_format < this_type::kind_UNSIGNED
            || static_cast<std::uint32_t>(local_format) != in_bit_size)
        {
            return false;
        }
        auto const local_record(this->make_record(in_key, local_format));
        if (local_record == nullptr)
        {
            return false;
        }
        PSYQ_ASSERT((in_initial_value >> in_bit_size) == 0);
        return this_type::set_bits(
            this->units_,
            local_record.block.position,
            in_bit_size,
            in_initial_value);
    }

    //-------------------------------------------------------------------------
    private: this_type::record_map::value_type* make_record(
        this_type::key_type const in_key,
        this_type::format_type const in_format)
    {
        if (this->records_.find(in_key) != this->records_.end())
        {
            return nullptr;
        }
        auto const local_emplace(
            this->records_.emplace(in_key, this_type::block()));
        if (!local_emplace.second)
        {
            PSYQ_ASSERT(false);
            return nullptr;
        }
        auto& local_record(local_emplace.first->second);
        local_record.format = in_format;
        local_record.block.bit_size = this_type::get_format_bit_size(in_format);

        // ビット配列を用意する。
        auto const local_empty_block(
            this->empty_blocks_.lower_bound(
                this_type::empty_block(0, local_record.bit_size)));
        if (local_empty_block == this->empty_blocks_.end()
            || local_empty_blocK->bit_size < local_record.bit_size)
        {
            // 新たにビット配列を追加する。
            auto const UNIT_BIT_SIZE(
                sizeof(this_type::unit_vector::value_type)
                * this_type::BITS_PER_BYTE);
            auto const local_block_position(
                this->units_.size() * UNIT_BIT_SIZE);
            local_record.block.position =
                static_cast<this_type::pos_type>(local_block_position);
            PSYQ_ASSERT(local_record.block.position == local_block_position);
            auto const local_add_unit_size(
                (local_record.block.bit_size + UNIT_BIT_SIZE - 1)
                / UNIT_BIT_SIZE);
            this->units_.insert(
                this->units_.end(), local_add_unit_size, 0);

            // 空き領域を追加する。
            auto const local_add_bit_size(local_add_unit_size * UNIT_BIT_SIZE);
            if (local_record.block.bit_size < local_add_bit_size)
            {
                this->empty_blocks_.emplace(
                    local_record.block.position + local_record.block.bit_size,
                    local_add_bit_size - local_record.block.bit_size);
            }
        }
        else
        {
            // 既存のビット配列を使う。
            local_record.block.position = local_empty_block->position;

            // 空き領域を更新する。
            if (local_record.block.bit_size < local_empty_block->bit_size)
            {
                this->empty_blocks_.emplace(
                    local_record.block.position + local_record.block.bit_size,
                    local_empty_block->bit_size - local_record.block.bit_size);
            }
            this->empty_blocks_.erase(local_empty_block);
        }
        return &local_record;
    }

    private: static this_type::size_type get_format_bit_size(
        this_type::format const in_format)
    {
        switch (in_format)
        {
            case this_type::kind_BOOL:
            return 1;

            case this_type::kind_FLOAT:
            return this_type::BITS_PER_BYTE * sizeof(float);

            case this_type::kind_DOUBLE:
            return this_type::BITS_PER_BYTE * sizeof(double);

            default:
            return std::abs(in_format);
        }
    }

    private: static this_type::unit_vector::value_type get_bits(
        this_type::unit_vector const& in_unit_vector,
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
        return in_units.at(local_unit_index) >> local_mod_bit_size;
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
        if (in_units.size() <= local_unit_index)
        {
            PSYQ_ASSERT(false);
            return false;
        }

        auto& local_unit(in_units.at(local_unit_index));
        auto const local_mod_bit_size(
            in_position - local_unit_index * UNIT_BIT_SIZE);
        if (in_bit_size < UNIT_BIT_SIZE)
        {
            PSYQ_ASSERT(local_mod_bit_size < UNIT_BIT_SIZE);
            auto const local_bit_mask(
                ((this_type::unit_vector::value_type(1) << in_bit_size) - 1));
            local_unit = (~(local_bit_mask << local_mod_bit_size) & local_unit)
                | ((in_value & local_bit_mask) << local_mod_bit_size);
        }
        else
        {
            PSYQ_ASSERT(local_mod_bit_size == 0);
            local_unit = in_value;
        }
        return true;
    }

    //-------------------------------------------------------------------------
    private: this_type::empty_block_set empty_blocks_;
    private: this_type::record_map records_;
    private: this_type::unit_vector units_;

}; // class psyq::state_archive

#endif // !defined(PSYQ_STATE_ARCHIVE_HPP_)
