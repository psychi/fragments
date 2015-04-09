﻿/** @file
    @copydoc psyq::scenario_engine::reservoir
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_SCENARIO_ENGINE_RESERVOIR_HPP_
#define PSYQ_SCENARIO_ENGINE_RESERVOIR_HPP_

#include <type_traits>
#include <vector>

namespace psyq
{
    /// @brief ビデオゲームでのシナリオ進行を管理するための実装
    namespace scenario_engine
    {
        /// @cond
        template<typename, typename, typename, typename> class reservoir;
        /// @endcond

        /// @brief psyq::scenario_engine の管理者以外は、直接アクセス禁止。
        namespace _private
        {
            /// @cond
            template<typename, typename> class state_value;
            template<typename, typename> struct key_less;

            template<typename template_float> union float_union {};
            template<> union float_union<float>
            {
                float value;
                std::uint32_t bits;
            };
            template<> union float_union<double>
            {
                double value;
                std::uint64_t bits;
            };
            /// @endcond
        } // namespace _private
    } // namespace scenario_engine
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 状態貯蔵器。任意のビット長の状態値を管理する。

    使い方の概略。
    - 以下の関数で、状態値を登録する。
      - reservoir::register_bool
      - reservoir::register_unsigned
      - reservoir::register_signed
      - reservoir::register_float
    - reservoir::get_value で、状態値を取得する。
    - reservoir::set_value で、状態値を設定する。

    @tparam template_float     @copydoc reservoir::state_value::float_type
    @tparam template_state_key @copydoc reservoir::state_key
    @tparam template_chunk_key @copydoc reservoir::chunk_key
    @tparam template_allocator @copydoc reservoir::allocator_type
 */
template<
    typename template_float = float,
    typename template_state_key = std::uint32_t,
    typename template_chunk_key = template_state_key,
    typename template_allocator = std::allocator<void*>>
class psyq::scenario_engine::reservoir
{
    /// thisが指す値の型。
    private: typedef reservoir this_type;

    //-------------------------------------------------------------------------
    /// @brief 状態値を識別するための値を表す型。
    public: typedef template_state_key state_key;

    /// @brief チャンクを識別するための値を表す型。
    public: typedef template_chunk_key chunk_key;

    /// @brief 各種コンテナに用いるメモリ割当子の型。
    public: typedef template_allocator allocator_type;

    /// @brief 状態値のビット数を表す型。
    public: typedef std::uint8_t size_type;

    /// @brief 状態値のビット位置を表す型。
    private: typedef std::uint32_t pos_type;

    /// @brief 状態値の構成を表す型。
    private: typedef
        typename std::make_signed<typename this_type::size_type>::type
            format_type;

    private: typedef psyq::scenario_engine::_private::float_union<template_float>
        float_union;

    //-------------------------------------------------------------------------
    /// @brief ビット列ブロックを表す型。
    private: typedef std::uint64_t block_type;

    /// @brief ビット列ブロックを格納するコンテナ。
    public: typedef std::vector<
         typename this_type::block_type,
         typename this_type::allocator_type>
             block_vector;

    public: enum: typename this_type::size_type
    {
        /// @brief 1バイトあたりのビット数。
        BITS_PER_BYTE = 8,
        /// @brief ビット列ブロックのビット数。
        BLOCK_SIZE =
            sizeof(typename this_type::block_type) * this_type::BITS_PER_BYTE,
        /// @brief 浮動小数点数型のビット数。
        FLOAT_SIZE = 
            sizeof(template_float) * this_type::BITS_PER_BYTE,
        /// @brief 状態値のビット数の最大値。
        MAX_STATE_SIZE = this_type::BLOCK_SIZE,
    };
    static_assert(this_type::FLOAT_SIZE <= this_type::BLOCK_SIZE, "");

    //-------------------------------------------------------------------------
    /// @brief 状態値のビット位置とビット数を表す型。
    private: typedef typename this_type::pos_type field_type;

    private: enum field_enum: typename this_type::field_type
    {
        field_POSITION_FRONT,
        field_POSITION_BACK= 23,
        field_TRANSITION_FRONT,
        field_TRANSITION_BACK = field_TRANSITION_FRONT,
        field_SIZE_FRONT,
        field_SIZE_BACK = 31,
        field_POSITION_MASK = (2 << (field_POSITION_BACK - field_POSITION_FRONT)) - 1,
        field_SIZE_MASK = (2 << (field_SIZE_BACK - field_SIZE_FRONT)) - 1,
    };
    static_assert(
        this_type::field_POSITION_BACK- this_type::field_POSITION_FRONT
        < sizeof(typename this_type::pos_type) * this_type::BITS_PER_BYTE,
        "");
    static_assert(
        this_type::field_SIZE_BACK - this_type::field_SIZE_FRONT
        < sizeof(typename this_type::size_type) * this_type::BITS_PER_BYTE,
        "");

    /// @brief 空き領域のコンテナ。
    private: typedef std::vector<
         typename this_type::field_type, typename this_type::allocator_type>
            empty_field_vector;

    /// @brief 空きビット領域を比較する関数オブジェクト。
    private: struct empty_field_less
    {
        bool operator()(
            typename reservoir::field_type const in_left,
            typename reservoir::field_type const in_right)
        const PSYQ_NOEXCEPT
        {
            // ビット領域のビット数で比較する。
            auto const local_left_size(
                reservoir::get_empty_field_size(in_left));
            auto const local_right_size(
                reservoir::get_empty_field_size(in_right));
            if (local_left_size != local_right_size)
            {
                return local_left_size < local_right_size;
            }

            // ビット領域のビット位置で比較する。
            auto const local_left_position(
                reservoir::get_field_position(in_left));
            auto const local_right_position(
                reservoir::get_field_position(in_right));
            return local_left_position < local_right_position;
        }

        bool operator()(
            typename reservoir::field_type const in_left,
            typename reservoir::size_type const in_right)
        const PSYQ_NOEXCEPT
        {
            // ビット領域のビット数で比較する。
            auto const local_left_size(
                reservoir::get_empty_field_size(in_left));
            return local_left_size < in_right;
        }

        bool operator()(
            typename reservoir::size_type const in_left,
            typename reservoir::field_type const in_right)
        const PSYQ_NOEXCEPT
        {
            // ビット領域のビット数で比較する。
            auto const local_right_size(
                reservoir::get_empty_field_size(in_right));
            return in_left < local_right_size;
        }

    }; // struct empty_field_less

    //-------------------------------------------------------------------------
    /// @brief 状態値。
    public: typedef psyq::scenario_engine::_private::state_value<
        typename this_type::block_type, template_float>
            state_value;

    /// @brief 状態値の登記。
    private: struct state_registry
    {
        /** @brief 状態値の構成を取得する。
            @return 状態値の構成。
         */
        typename reservoir::format_type get_format() const PSYQ_NOEXCEPT
        {
            auto const local_mod_size(
                reservoir::field_SIZE_BACK - reservoir::field_SIZE_FRONT);
            auto const local_minus(
                1 & static_cast<typename reservoir::format_type>(
                    this->field >> reservoir::field_SIZE_BACK));
            return (-local_minus << local_mod_size)
                | static_cast<typename reservoir::format_type>(
                    reservoir::field_SIZE_MASK & (
                        this->field >> reservoir::field_SIZE_FRONT));
        }

        /// @brief 状態値が格納されているビット列チャンクの識別値。
        typename reservoir::chunk_key chunk;
        /// @brief 状態値に対応する識別値。
        typename reservoir::state_key key;
        /// @brief 状態値が格納されているビット領域。
        typename reservoir::field_type field;

    }; // struct state_registry

    /// @brief 状態値登記のコンテナ。
    private: typedef std::vector<
        typename this_type::state_registry, typename this_type::allocator_type>
            state_vector;

    /// @brief 状態値に対応する識別値を比較する関数オブジェクト。
    private: typedef psyq::scenario_engine::_private::key_less<
         typename this_type::state_registry, typename this_type::state_key>
             state_key_less;

    //-------------------------------------------------------------------------
    /// @brief 状態値を格納するビット列のチャンク。
    private: struct chunk
    {
        typedef chunk this_type;

        chunk(
            typename reservoir::chunk_key in_key,
            typename reservoir::allocator_type const& in_allocator)
        :
        blocks(in_allocator),
        empty_fields(in_allocator),
        key(std::move(in_key))
        {}

        /** @brief ムーブ構築子。
            @param[in,out] io_source ムーブ元となるインスタンス。
         */
        chunk(this_type&& io_source):
        blocks(std::move(io_source.blocks)),
        empty_fields(std::move(io_source.empty_fields)),
        key(std::move(io_source.key))
        {}

        /** @brief ムーブ代入演算子。
            @param[in,out] io_source ムーブ元となるインスタンス。
            @return *this
         */
        this_type& operator=(this_type&& io_source)
        {
            this->blocks = std::move(io_source.blocks);
            this->empty_fields = std::move(io_source.empty_fields);
            this->key = std::move(io_source.key);
            return *this;
        }

        /// @copydoc block_vector
        typename reservoir::block_vector blocks;
        /// @copydoc empty_field_vector
        typename reservoir::empty_field_vector empty_fields;
        /// @brief チャンクの識別値。
        typename reservoir::chunk_key key;

    }; // struct chunk

    /// @brief ビット列チャンクのコンテナ。
    private: typedef std::vector<
         typename this_type::chunk, typename this_type::allocator_type>
             chunk_vector;

    /// @brief チャンク識別値を比較する関数オブジェクト。
    private: typedef psyq::scenario_engine::_private::key_less<
         typename this_type::chunk, typename this_type::chunk_key>
             chunk_key_less;

    //-------------------------------------------------------------------------
    /// @name 構築と代入
    //@{
    /** @brief 空の状態貯蔵器を構築する。
        @param[in] in_reserve_states 状態値の予約数。
        @param[in] in_reserve_chunks ビット列チャンクの予約数。
        @param[in] in_allocator      使用するメモリ割当子の初期値。
     */
    public: reservoir(
        std::size_t const in_reserve_states,
        std::size_t const in_reserve_chunks,
        typename this_type::allocator_type const& in_allocator =
            this_type::allocator_type())
    :
    states_(in_allocator),
    chunks_(in_allocator)
    {
        this->states_.reserve(in_reserve_states);
        this->chunks_.reserve(in_reserve_chunks);
    }

    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元となるインスタンス。
     */
    public: reservoir(this_type&& io_source) PSYQ_NOEXCEPT:
    states_(std::move(io_source.states_)),
    chunks_(std::move(io_source.chunks_))
    {}

    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元となるインスタンス。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source) PSYQ_NOEXCEPT
    {
        this->states_ = std::move(io_source.states_);
        this->chunks_ = std::move(io_source.chunks_);
        return *this;
    }

    /** @brief 状態貯蔵器で使われているメモリ割当子を取得する。
        @return 状態貯蔵器で使われているメモリ割当子。
     */
    public: typename this_type::allocator_type get_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->states_.get_allocator();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 状態値の取得と設定
    //@{
    /** @brief 状態値の構成を取得する。
        @param[in] in_state_key 構成を取得する状態値に対応する識別値。
        @retval !=this_type::state_value::kind_NULL 状態値の構成。
        @retval ==this_type::state_value::kind_NULL in_state_key に対応する状態値がない。
        @sa this_type::get_format_size
        @sa this_type::get_format_kind
     */
    public: typename this_type::format_type get_format(
        typename this_type::state_key const& in_state_key)
    const PSYQ_NOEXCEPT
    {
        auto local_state(
            this_type::state_key_less::find_const_pointer(
                this->states_, in_state_key));
        return local_state != nullptr?
            local_state->get_format(): this_type::state_value::kind_NULL;
    }
    //@}
    /** @brief 状態値の構成から、状態値のビット数を取得する。
        @param[in] in_format 状態値の構成。
        @return 状態値のビット数。
        @sa this_type::get_format
     */
    public: static typename this_type::size_type get_format_size(
        typename this_type::format_type const in_format)
    PSYQ_NOEXCEPT
    {
        switch (in_format)
        {
            case this_type::state_value::kind_NULL:
            return 0;

            case this_type::state_value::kind_BOOL:
            return 1;

            case this_type::state_value::kind_FLOAT:
            return this_type::FLOAT_SIZE;

            default:
            return in_format < 0? -in_format: in_format;
        }
    }

    /** @brief 状態値の構成から、状態値の型の種別を取得する。
        @param[in] in_format 状態値の構成。
        @return 状態値の型の種別。
        @sa this_type::get_format
     */
    public: static typename this_type::state_value::kind_enum get_format_kind(
        typename this_type::format_type const in_format)
    PSYQ_NOEXCEPT
    {
        switch (in_format)
        {
            case this_type::state_value::kind_NULL:
            case this_type::state_value::kind_BOOL:
            case this_type::state_value::kind_FLOAT:
            return static_cast<typename this_type::state_value::kind_enum>(
                in_format);

            default:
            return in_format < 0?
                this_type::state_value::kind_SIGNED:
                this_type::state_value::kind_UNSIGNED;
        }
    }

    //-------------------------------------------------------------------------
    /// @name 状態値の取得と設定
    //@{
    /** @brief 状態値を取得する。

        すでに登録されている状態値から、値を取得する。

        @param[in] in_state_key 取得する状態値に対応する識別値。
        @return
            取得した状態値。状態値の取得に失敗した場合は、
            this_type::state_value::get_kind で
            this_type::state_value::kind_NULL を返す。
        @sa this_type::register_bool
        @sa this_type::register_unsigned
        @sa this_type::register_signed
        @sa this_type::register_float
        @sa this_type::set_value
     */
    public: typename this_type::state_value get_value(
        typename this_type::state_key const& in_state_key)
    const PSYQ_NOEXCEPT
    {
        // 状態値登記を検索し、ビット列チャンクから状態値のビット列を取得する。
        auto const local_state_registry(
            this_type::state_key_less::find_const_pointer(
                this->states_, in_state_key));
        if (local_state_registry == nullptr)
        {
            return typename this_type::state_value();
        }
        auto const local_chunk(
            this_type::chunk_key_less::find_const_pointer(
                this->chunks_, local_state_registry->chunk));
        if (local_chunk == nullptr)
        {
            // 状態値登記があれば、対応するビット列ちゃんくもあるはず。
            PSYQ_ASSERT(false);
            return typename this_type::state_value();
        }
        auto const local_format(local_state_registry->get_format());
        auto const local_size(this_type::get_format_size(local_format));
        auto const local_bits(
            this_type::get_bits(
                local_chunk->blocks,
                this_type::get_field_position(local_state_registry->field),
                local_size));

        // 状態値の構成から、出力値のコピー処理を分岐する。
        switch (local_format)
        {
            // 真偽値を取得する。
            case this_type::state_value::kind_BOOL:
            return typename this_type::state_value(local_bits != 0);

            // 浮動小数点数を取得する。
            case this_type::state_value::kind_FLOAT:
            {
                typename this_type::float_union local_float;
                local_float.bits =
                    static_cast<decltype(local_float.bits)>(local_bits);
                return typename this_type::state_value(local_float.value);
            }

            // 整数を取得する。
            default:
            if (local_format < 0)
            {
                // 符号あり整数を取得する。
                auto const local_signed_bits(
                    static_cast<typename this_type::state_value::signed_type>(
                        local_bits));
                auto const local_minus(
                    1 & (local_signed_bits >> (local_size - 1)));
                return typename this_type::state_value(
                    local_signed_bits | (-local_minus << local_size));
            }
            else if (0 < local_format)
            {
                // 符号なし整数を取得する。
                return typename this_type::state_value(local_bits);
            }
            PSYQ_ASSERT(false);
            return typename this_type::state_value();
        }
    }
    //@}
    /** @brief ビット列から値を取得する。
        @param[in,out] in_blocks 値を取得するビット列のコンテナ。
        @param[in] in_position   値を取得するビット列のビット位置。
        @param[in] in_size       値を取得するビット列のビット数。
        @return
            ビット列から取得した値。
            ただし、該当する値がない場合は、0を返す。
     */
    private: static typename this_type::block_type get_bits(
        typename this_type::block_vector const& in_blocks,
        typename this_type::pos_type const in_position,
        typename this_type::size_type const in_size)
    PSYQ_NOEXCEPT
    {
        if (this_type::BLOCK_SIZE < in_size)
        {
            PSYQ_ASSERT(false);
            return 0;
        }
        auto const local_block_index(in_position / this_type::BLOCK_SIZE);
        if (in_blocks.size() <= local_block_index)
        {
            // 状態値登記があれば、対応するビット列チャンクもあるはず。
            PSYQ_ASSERT(false);
            return 0;
        }

        // ブロックビット列でのビット位置を決定し、値を取り出す。
        auto const local_position(
            in_position - local_block_index * this_type::BLOCK_SIZE);
        PSYQ_ASSERT(
            (in_size == this_type::BLOCK_SIZE && local_position == 0)
            || (in_size < this_type::BLOCK_SIZE
                && local_position < this_type::BLOCK_SIZE));
        return (in_blocks.at(local_block_index) >> local_position)
            & this_type::make_block_mask(in_size);
    }

    //-------------------------------------------------------------------------
    /// @name 状態値の取得と設定
    //@{
    /** @brief 状態値を設定する。

        すでに登録されている状態値に、値を設定する。

        @param[in] in_state_key   設定する状態値に対応する識別値。
        @param[in] in_state_value 状態値に設定する値。
        @retval true  成功。
        @retval false
            失敗。状態値は変化しない。
            - in_state_key に対応する状態値がない場合は失敗する。
            - 論理型以外の値を論理型の状態値へ設定しようとすると失敗する。
            - 整数型以外の値を整数型の状態値へ設定しようとすると失敗する。
            - 論理型の値を浮動小数点数型の状態値へ設定しようとすると失敗する。
        @note
            this_type::state_value::float_type より精度の高い浮動小数点数を
            浮動小数点数型の状態値へ設定しようとすると、
            コンパイル時にエラーか警告が発生する。
        @sa this_type::register_bool
        @sa this_type::register_unsigned
        @sa this_type::register_signed
        @sa this_type::register_float
        @sa this_type::get_value
     */
    public: template<typename template_value>
    bool set_value(
        typename this_type::state_key const& in_state_key,
        template_value const in_state_value)
    PSYQ_NOEXCEPT
    {
        static_assert(
            std::is_floating_point<template_value>::value
            || std::is_integral<template_value>::value,
            "'template_value' is not an integer or floating-point number.");

        // 状態値登記を検索する。
        auto const local_state(
            this_type::state_key_less::find_const_pointer(
                this->states_, in_state_key));
        if (local_state == nullptr)
        {
            return false;
        }

        // 状態値を設定するビット列チャンクを決定する。
        auto const local_chunk(
            this_type::chunk_key_less::find_pointer(
                this->chunks_, local_state->chunk));
        if (local_chunk == nullptr)
        {
            // 状態値の登記があるなら、ビット列チャンクもあるはず。
            PSYQ_ASSERT(false);
            return false;
        }
        auto& local_chunk_blocks(local_chunk->blocks);

        // 状態値の構成によって、設定処理を分岐する。
        auto const local_format(local_state->get_format());
        auto const local_position(
            this_type::get_field_position(local_state->field));
        switch (local_format)
        {
            case this_type::state_value::kind_NULL:
            PSYQ_ASSERT(false);
            return false;

            // 真偽値を設定する。
            case this_type::state_value::kind_BOOL:
            if (std::is_same<bool, template_value>::value)
            {
                return this_type::notify_transition(
                    const_cast<this_type::state_registry&>(*local_state),
                    this_type::set_bits(
                        local_chunk_blocks,
                        local_position,
                        1,
                        static_cast<typename this_type::block_type>(
                            in_state_value)));
            }
            return false;

            // 浮動小数点数を設定する。
            case this_type::state_value::kind_FLOAT:
            if (!std::is_same<bool, template_value>::value)
            {
                return this_type::notify_transition(
                    const_cast<this_type::state_registry&>(*local_state),
                    this_type::set_bits(
                        local_chunk_blocks,
                        local_position,
                        this_type::FLOAT_SIZE,
                        this_type::get_float_bits(in_state_value)));
            }
            return false;

            // 整数を設定する。
            default:
            if (!std::is_same<bool, template_value>::value
                && std::is_integral<template_value>::value)
            {
                auto const local_size(
                    this_type::get_format_size(local_format));
                return this_type::notify_transition(
                    const_cast<this_type::state_registry&>(*local_state),
                    local_format < 0?
                        // 符号あり整数を設定する。
                        this_type::set_signed(
                            local_chunk_blocks,
                            local_position,
                            local_size,
                            in_state_value):
                        // 符号なし整数を設定する。
                        this_type::set_bits(
                            local_chunk_blocks,
                            local_position,
                            local_size,
                            static_cast<typename this_type::block_type>(
                                in_state_value)));
            }
            return false;
        }
    }
    //@}
    private: static typename this_type::block_type get_float_bits(
        bool const in_value)
    {
        typename this_type::float_union local_float;
        local_float.value = in_value;
        return local_float.bits;
    }
    private: static typename this_type::block_type get_float_bits(
        float const in_value)
    {
        typename this_type::float_union local_float;
        local_float.value = in_value;
        return local_float.bits;
    }
    private: static typename this_type::block_type get_float_bits(
        double const in_value)
    {
        typename this_type::float_union local_float;
        /** @note
            ここでコンパイルエラーか警告が出る場合は
            double から float への型変換が発生しているのが原因。
            set_value の引数を手動で型変換する必要がある。
         */
        local_float.value = in_value;
        return local_float.bits;
    }
    private: template<typename template_value>
    static typename this_type::block_type get_float_bits(
        template_value const in_value)
    {
        typename this_type::float_union local_float;
        local_float.value =
            static_cast<typename this_type::state_value::float_type>(in_value);
        PSYQ_ASSERT(
            // 桁あふれが発生してないか確認する。
            static_cast<template_value>(local_float.value) == in_value);
        return local_float.bits;
    }

    private: static int set_signed(
        typename this_type::block_vector& io_blocks,
        typename this_type::pos_type const in_position,
        typename this_type::size_type const in_size,
        bool const in_value)
    PSYQ_NOEXCEPT
    {
        return this_type::set_bits(io_blocks, in_position, in_size, in_value);
    }

    private: template<typename template_value>
    static int set_signed(
        typename this_type::block_vector& io_blocks,
        typename this_type::pos_type const in_position,
        typename this_type::size_type const in_size,
        template_value const in_value)
    PSYQ_NOEXCEPT
    {
        auto local_bits(
            static_cast<typename this_type::block_type>(
                static_cast<typename this_type::state_value::signed_type>(in_value)));
        if (in_value < 0)
        {
            auto const local_mask(this_type::make_block_mask(in_size));
            PSYQ_ASSERT((~local_mask & local_bits) == ~local_mask);
            local_bits &= local_mask;
        }
        return this_type::set_bits(
            io_blocks, in_position, in_size, local_bits);
    }

    /** @brief ビット列に値を設定する。
        @param[in,out] io_blocks 値を設定するビット列のコンテナ。
        @param[in] in_position   値を設定するビット列のビット位置。
        @param[in] in_size       値を設定するビット列のビット数。
        @param[in] in_value      設定する値。
        @retval 正 元とは異なる値を設定した。
        @retval  0 元と同じ値を設定した。
        @retval 負 失敗。値を設定できなかった。
     */
    private: static int set_bits(
        typename this_type::block_vector& io_blocks,
        typename this_type::pos_type const in_position,
        typename this_type::size_type const in_size,
        typename this_type::block_type const in_value)
    PSYQ_NOEXCEPT
    {
        PSYQ_ASSERT((in_value >> in_size) == 0);
        if (this_type::BLOCK_SIZE < in_size)
        {
            PSYQ_ASSERT(false);
            return -1;
        }
        auto const local_block_index(in_position / this_type::BLOCK_SIZE);
        if (io_blocks.size() <= local_block_index)
        {
            PSYQ_ASSERT(false);
            return -1;
        }

        // ブロックビット列でのビット位置を決定し、値を埋め込む。
        auto const local_position(
            in_position - local_block_index * this_type::BLOCK_SIZE);
        PSYQ_ASSERT(local_position + in_size <= this_type::BLOCK_SIZE);
        auto const local_mask(this_type::make_block_mask(in_size));
        auto& local_block(io_blocks.at(local_block_index));
        auto const local_last_block(local_block);
        local_block = (~(local_mask << local_position) & local_block)
            | ((in_value & local_mask) << local_position);
        return local_last_block != local_block? 1: 0;
    }

    //-------------------------------------------------------------------------
    /// @name 状態値の登録
    //@{
    /** @brief 論理型の状態値を登録する。

        - 登録した状態値は
          this_type::get_value と this_type::set_value でアクセスできる。
        - 登録した状態値は this_type::remove_chunk で削除できる。

        @param[in] in_chunk_key   登録する状態値を格納するビット列チャンクの識別値。
        @param[in] in_state_key   登録する状態値の識別番号。
        @param[in] in_state_value 登録する状態値の初期値。
        @retval true  成功。状態値を登録した。
        @retval false
            失敗。状態値を登録できなかった。
            in_state_key に対応する状態値がすでに登録されていると失敗する。
     */
    public: bool register_bool(
        typename this_type::chunk_key in_chunk_key,
        typename this_type::state_key in_state_key,
        bool const in_state_value)
    {
        // 状態値登記を登録した後、状態値に初期値を設定する。
        auto& local_chunk(
            this_type::equip_chunk(this->chunks_, std::move(in_chunk_key)));
        auto const local_state(
            this->register_state(
                local_chunk,
                std::move(in_state_key),
                this_type::state_value::kind_BOOL));
        if (local_state == nullptr)
        {
            return false;
        }
        return 0 <= this_type::set_bits(
            local_chunk.blocks,
            this_type::get_field_position(local_state->field),
            1,
            in_state_value);
    }

    /** @brief 符号なし整数型の状態値を登録する。

        - 登録した状態値は
          this_type::get_value と this_type::set_value でアクセスできる。
        - 登録した状態値は this_type::remove_chunk で削除できる。

        @param[in] in_chunk_key   登録する状態値を格納するビット列チャンクの識別値。
        @param[in] in_state_key   登録する状態値の識別番号。
        @param[in] in_state_value 登録する状態値の初期値。
        @param[in] in_state_size  登録する状態値のビット数。
        @retval true  成功。状態値を登録した。
        @retval false
            失敗。状態値を登録できなかった。
            - in_state_key に対応する状態値がすでに登録されていると失敗する。
            - this_type::MAX_STATE_SIZE より in_state_size が大きければ失敗する。
        @sa remove_chunk
     */
    public: bool register_unsigned(
        typename this_type::chunk_key in_chunk_key,
        typename this_type::state_key in_state_key,
        typename this_type::state_value::unsigned_type const in_state_value,
        std::size_t const in_state_size)
    {
        // 登録不可能な状態値か判定する。
        auto const local_format(
            static_cast<typename this_type::format_type>(in_state_size));
        if (this_type::BLOCK_SIZE < in_state_size
            || local_format < this_type::state_value::kind_UNSIGNED)
        {
            return false;
        }

        // 状態値を登録した後、状態値に初期値を設定する。
        auto& local_chunk(
            this_type::equip_chunk(this->chunks_, std::move(in_chunk_key)));
        auto const local_state(
            this->register_state(
                local_chunk, std::move(in_state_key), local_format));
        if (local_state == nullptr)
        {
            return false;
        }
        return 0 <= this_type::set_bits(
            local_chunk.blocks,
            this_type::get_field_position(local_state->field),
            static_cast<typename this_type::size_type>(in_state_size),
            in_state_value);
    }

    /** @brief 符号あり整数型の状態値を登録する。

        - 登録した状態値は
          this_type::get_value と this_type::set_value でアクセスできる。
        - 登録した状態値は this_type::remove_chunk で削除できる。

        @param[in] in_chunk_key   登録する状態値を格納するビット列チャンクの識別値。
        @param[in] in_state_key   登録する状態値の識別番号。
        @param[in] in_state_value 登録する状態値の初期値。
        @param[in] in_state_size  登録する状態値のビット数。
        @retval true  成功。状態値を登録した。
        @retval false
            失敗。状態値を登録できなかった。
            - in_state_key に対応する状態値がすでに登録されていると失敗する。
            - this_type::MAX_STATE_SIZE より in_state_size が大きければ失敗する。
        @sa remove_chunk
     */
    public: bool register_signed(
        typename this_type::chunk_key in_chunk_key,
        typename this_type::state_key in_state_key,
        typename this_type::state_value::signed_type const in_state_value,
        std::size_t const in_state_size)
    {
        // 登録不可能な状態値か判定する。
        auto const local_format(
            -static_cast<typename this_type::format_type>(in_state_size));
        if (this_type::BLOCK_SIZE < in_state_size
            || this_type::state_value::kind_SIGNED < local_format)
        {
            return false;
        }

        // 状態値を登録した後、状態値に初期値を設定する。
        auto& local_chunk(
            this_type::equip_chunk(this->chunks_, std::move(in_chunk_key)));
        auto const local_state(
            this->register_state(
                local_chunk, std::move(in_state_key), local_format));
        if (local_state == nullptr)
        {
            return false;
        }
        return 0 <= this_type::set_signed(
            local_chunk.blocks,
            this_type::get_field_position(local_state->field),
            static_cast<typename this_type::size_type>(in_state_size),
            in_state_value);
    }
    /** @brief 浮動小数点数型の状態値を登録する。

        - 登録した状態値は
          this_type::get_value と this_type::set_value でアクセスできる。
        - 登録した状態値は this_type::remove_chunk で削除できる。

        @param[in] in_chunk_key   登録する状態値を格納するビット列チャンクの識別値。
        @param[in] in_state_key   登録する状態値の識別番号。
        @param[in] in_state_value 登録する状態値の初期値。
        @retval true  成功。状態値を登録した。
        @retval false
            失敗。状態値を登録できなかった。
            in_state_key に対応する状態値がすでに登録されていると失敗する。
        @sa remove_chunk
     */
    public: bool register_float(
        typename this_type::chunk_key in_chunk_key,
        typename this_type::state_key in_state_key,
        typename this_type::state_value::float_type const in_state_value)
    {
        // 状態値を登録した後、状態値に初期値を設定する。
        auto& local_chunk(
            this_type::equip_chunk(this->chunks_, std::move(in_chunk_key)));
        auto const local_state(
            this->register_state(
                local_chunk,
                std::move(in_state_key),
                this_type::state_value::kind_FLOAT));
        if (local_state == nullptr)
        {
            return false;
        }
        typename this_type::float_union local_float;
        local_float.value = in_state_value;
        return 0 <= this_type::set_bits(
            local_chunk.blocks,
            this_type::get_field_position(local_state->field),
            this_type::FLOAT_SIZE,
            local_float.bits);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 状態値の変化
    //@{
    /** @brief psyq::scenario_engine 管理者以外は、この関数は使用禁止。

        直前の状態変化フラグを取得する。

        @param[in] in_state_key 登録する状態値に対応する識別値。
        @retval 0以上 直前の状態変化フラグ。
        @retval 0未満 状態値がない。
     */
    public: int _get_transition(
        typename this_type::state_key const& in_state_key)
    const PSYQ_NOEXCEPT
    {
        auto local_state(
            this_type::state_key_less::find_const_pointer(
                this->states_, in_state_key));
        return local_state != nullptr?
            (local_state->field >> reservoir::field_TRANSITION_FRONT) & 1: -1;
    }

    /** @brief psyq::scenario_engine 管理者以外は、この関数は使用禁止。

        状態変化フラグを初期化する。
     */
    public: void _reset_transition()
    {
        auto const local_mask(
            ~static_cast<typename this_type::field_type>(
                1 << this_type::field_TRANSITION_FRONT));
        for (auto& local_state: this->states_)
        {
            local_state.field &= local_mask;
        }
    }
    //@}
    private: static bool notify_transition(
        typename this_type::state_registry& io_state,
        int const in_set_bits)
    {
        if (in_set_bits < 0)
        {
            return false;
        }
        if (0 < in_set_bits)
        {
            io_state.field |= 1 << this_type::field_TRANSITION_FRONT;
        }
        return true;
    }

    //-------------------------------------------------------------------------
    /// @name ビット列チャンク
    //@{
    /** @brief ビット列チャンクを予約する。
        @param[in] in_chunk_key            予約するビット列チャンクの識別値。
        @param[in] in_reserve_blocks       予約しておくブロックの数。
        @param[in] in_reserve_empty_fields 予約しておく空き領域の数。
     */
    public: void reserve_chunk(
        typename this_type::chunk_key in_chunk_key,
        std::size_t const in_reserve_blocks,
        std::size_t const in_reserve_empty_fields)
    {
        auto& local_chunk(
            this_type::equip_chunk(this->chunks_, std::move(in_chunk_key)));
        local_chunk.blocks.reserve(in_reserve_blocks);
        local_chunk.empty_fields.reserve(in_reserve_empty_fields);
    }

    /** @brief ビット列チャンクを破棄する。
        @param[in] in_chunk_key 破棄するビット列チャンクの識別値。
        @todo 未実装。
     */
    public: bool remove_chunk(
        typename this_type::chunk_key const& in_chunk_key);

    /** @brief ビット列チャンクをシリアル化する。
        @param[in] in_chunk_key シリアル化するビット列チャンクの識別番号。
        @return シリアル化したビット列チャンク。
        @todo 未実装。
     */
    public: typename this_type::block_vector serialize_chunk(
        typename this_type::chunk_key const& in_chunk_key)
    const;

    /** @brief シリアル化されたビット列チャンクを復元する。
        @param[in] in_chunk_key        復元するビット列チャンクの識別値。
        @param[in] in_serialized_chunk シリアル化されたビット列チャンク。
        @todo 未実装。
     */
    public: bool deserialize_chunk(
        typename this_type::chunk_key const& in_chunk_key,
        typename this_type::block_vector const& in_serialized_chunk);
    //@}
    /** @brief 識別値に対応するビット列チャンクを用意する。

        識別値に対応するビット列チャンクを、コンテナに用意する。
        同じ識別値のチャンクがすでにコンテナにあれば、それを返す。
        同じ識別値のチャンクがコンテナになければ、新たにチャンクを生成する。

        @param[in,out] io_chunks チャンクを用意するコンテナ。
        @param[in] in_chunk_key  用意するチャンクに対応する識別値。
        @return 用意したチャンクへの参照。
     */
    private: static typename this_type::chunk_vector::value_type& equip_chunk(
        typename this_type::chunk_vector& io_chunks,
        typename this_type::chunk_key in_chunk_key)
    {
        auto const local_lower_bound(
            std::lower_bound(
                io_chunks.begin(),
                io_chunks.end(),
                in_chunk_key,
                typename this_type::chunk_key_less()));
        if (local_lower_bound != io_chunks.end()
            && local_lower_bound->key == in_chunk_key)
        {
            return *local_lower_bound;
        }
        return *io_chunks.insert(
            local_lower_bound,
            typename this_type::chunk_vector::value_type(
                std::move(in_chunk_key), io_chunks.get_allocator()));
    }

    //-------------------------------------------------------------------------
    /// @name 構築と代入
    //@{
    /// @brief 状態貯蔵器を再構築し、メモリ領域を必要最小限にする。
    public: void shrink_to_fit()
    {
        // ビット領域の大きさの降順で、状態値を並び替える。
        std::vector<typename this_type::state_vector::value_type const*>
            local_states(this->states_.get_allocator());
        local_states.reserve(this->states_.size());
        for (auto& local_state: this->states_)
        {
            local_states.push_back(&local_state);
        }
        struct state_size_greater
        {
            bool operator()(
                typename this_type::state_vector::value_type const* const in_left,
                typename this_type::state_vector::value_type const* const in_right)
            const
            {
                auto const local_left_size(
                    this_type::get_format_size(in_left->get_format()));
                auto const local_right_size(
                    this_type::get_format_size(in_right->get_format()));
                return local_right_size < local_left_size;
            }
        };
        std::sort(
            local_states.begin(), local_states.end(), state_size_greater());

        // 新たな書庫を用意する。
        this_type local_reservoir(
            this->states_.size(),
            this->chunks_.size(),
            this->states_.get_allocator());
        for (auto& local_old_chunk: this->chunks_)
        {
            auto& local_new_chunk(
                *local_reservoir.chunks_.insert(
                    local_reservoir.chunks_.end(),
                    typename this_type::chunk_vector::value_type(
                        local_old_chunk.key, this->states_.get_allocator())));
            local_new_chunk.blocks.reserve(local_old_chunk.blocks.size());
            local_new_chunk.empty_fields.reserve(
                local_old_chunk.empty_fields.size());
        }

        // 現在の書庫をもとに、新たな書庫を構築する。
        for (auto local_state: local_states)
        {
            auto const local_chunk(
                this_type::chunk_key_less::find_const_pointer(
                    this->chunks_, local_state->chunk));
            if (local_chunk == nullptr)
            {
                PSYQ_ASSERT(false);
                continue;
            }
            auto const local_position(
                this_type::get_field_position(local_state->field));
            auto const local_format(local_state->get_format());
            switch (local_format)
            {
                case this_type::state_value::kind_NULL:
                PSYQ_ASSERT(false);
                continue;

                case this_type::state_value::kind_BOOL:
                local_reservoir.register_bool(
                    local_state->chunk,
                    local_state->key,
                    0 != this_type::get_bits(
                        local_chunk->blocks, local_position, 1));
                continue;

                case this_type::state_value::kind_FLOAT:
                PSYQ_ASSERT(false);
                continue;

                default:
                break;
            }
            auto const local_size(this_type::get_format_size(local_format));
            auto const local_bits(
                this_type::get_bits(
                    local_chunk->blocks, local_position, local_size));
            if (0 < local_format)
            {
                local_reservoir.register_unsigned(
                    local_state->chunk,
                    local_state->key,
                    local_bits,
                    local_size);
            }
            else
            {
                local_reservoir.register_signed(
                    local_state->chunk,
                    local_state->key,
                    local_bits,
                    local_size);
            }
        }

        // 新たに構築した書庫を移動して整理する。
        *this = std::move(local_reservoir);
        this->states_.shrink_to_fit();
        this->chunks_.shrink_to_fit();
        for (auto& local_chunk: this->chunks_)
        {
            local_chunk.blocks.shrink_to_fit();
            local_chunk.empty_fields.shrink_to_fit();
        }
    }
    //@}
    //-------------------------------------------------------------------------
    /** @brief 状態値を登録する。
        @param[in,out] io_chunk 登録する状態値が所属するビット列チャンク。
        @param[in] in_state_key 登録する状態値に対応する識別値。
        @param[in] in_format    登録する状態値の構成。
        @retval !=nullptr 成功。登録した状態登記。
        @retval ==nullptr
            失敗。状態値を登録できなかった。
            - in_state_key に対応する状態値がすでに登録されていると失敗する。
     */
    private: typename this_type::state_vector::value_type* register_state(
        typename this_type::chunk& io_chunk,
        typename this_type::state_key in_state_key,
        typename this_type::format_type const in_format)
    {
        // in_state_key と同じ状態登記がないことを確認する。
        auto const local_state_iterator(
            std::lower_bound(
                this->states_.begin(),
                this->states_.end(),
                in_state_key,
                typename this_type::state_key_less()));
        if (local_state_iterator != this->states_.end()
            && local_state_iterator->key == in_state_key)
        {
            return nullptr;
        }

        // 状態登記を新たに追加し、ビット列チャンクを用意する。
        auto& local_state(
            *this->states_.insert(
                local_state_iterator,
                typename this_type::state_vector::value_type()));
        local_state.chunk = io_chunk.key;
        local_state.key = std::move(in_state_key);
        local_state.field = 1 << this_type::field_TRANSITION_FRONT;
        PSYQ_ASSERT(in_format != this_type::state_value::kind_NULL);
        this_type::set_state_format(local_state, in_format);

        // 状態値のビット位置を決定する。
        auto const local_set_state_position(
            this_type::set_state_position(
                local_state,
                this_type::make_state_field(
                    this_type::get_format_size(in_format),
                    io_chunk.empty_fields,
                    io_chunk.blocks)));
        if (!local_set_state_position)
        {
            PSYQ_ASSERT(false);
            local_state.field = 0;
            return nullptr;
        }
        return &local_state;
    }

    /** @brief 状態値を格納するビット領域を生成する。
        @param[in] in_size             生成するビット領域のビット数。
        @param[in,out] io_empty_fields 空き領域のコンテナ。
        @param[in,out] io_blocks       状態値のビット領域に使うビット列ブロックのコンテナ。
        @return 生成したビット領域の、ビット列ブロックでのビット位置。
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
            this_type::get_empty_field_size(*in_empty_field));
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
        auto const local_position(io_blocks.size() * this_type::BLOCK_SIZE);
        if (local_position <= this_type::field_POSITION_MASK)
        {
            auto const local_add_block_size(
                (in_size + this_type::BLOCK_SIZE - 1) / this_type::BLOCK_SIZE);
            io_blocks.insert(io_blocks.end(), local_add_block_size, 0);

            // 空き領域を追加する。
            auto const local_add_size(
                local_add_block_size * this_type::BLOCK_SIZE);
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
        std::size_t const in_position,
        std::size_t const in_size)
    {
        if (in_position <= this_type::field_POSITION_MASK
            && in_size <= this_type::field_SIZE_MASK)
        {
            auto const local_empty_field(
               static_cast<typename this_type::field_type>(
                   (in_size << this_type::field_SIZE_FRONT)
                   | (in_position << this_type::field_POSITION_FRONT)));
            io_empty_fields.insert(
                std::lower_bound(
                    io_empty_fields.begin(),
                    io_empty_fields.end(),
                    local_empty_field,
                    this_type::empty_field_less()),
                local_empty_field);
        }
        else
        {
            PSYQ_ASSERT(false);
        }
    }

    //-------------------------------------------------------------------------
    /** @brief ビット領域のビット位置を取得する。
        @param[in] in_field ビット領域。
        @return ビット領域のビット位置。
     */
    private: static typename this_type::pos_type get_field_position(
        typename this_type::field_type const in_field)
    PSYQ_NOEXCEPT
    {
        return (in_field >> this_type::field_POSITION_FRONT)
            & this_type::field_POSITION_MASK;
    }

    /** @brief 空きビット領域のビット数を取得する。
        @param[in] in_field 空きビット領域。
        @return 空きビット領域のビット数。
     */
    private: static typename this_type::size_type get_empty_field_size(
        typename this_type::field_type const in_field)
    PSYQ_NOEXCEPT
    {
        return static_cast<typename this_type::size_type>(
            (in_field >> this_type::field_SIZE_FRONT)
            & this_type::field_SIZE_MASK);
    }

    /** @brief 状態値の登記に、状態値のビット位置を設定する。
        @param[in,out] io_state ビット位置を設定する状態値の登記。
        @param[in] in_position  状態値に設定するビット位置。
     */
    private: static bool set_state_position(
        typename this_type::state_registry& io_state,
        std::size_t const in_position)
    PSYQ_NOEXCEPT
    {
        if (this_type::field_POSITION_MASK < in_position)
        {
            return false;
        }
        auto const local_position(
            static_cast<typename this_type::field_type>(in_position)
            << this_type::field_POSITION_FRONT);
        auto const local_mask(
            this_type::field_POSITION_MASK
            << this_type::field_POSITION_FRONT);
        io_state.field =
            (~local_mask & io_state.field) | (local_mask & local_position);
        return true;
    }

    /** @brief 状態値の登記に、状態値の構成を設定する。
        @param[in,out] io_state 構成を設定する状態値の登記。
        @param[in] in_format    状態値に設定する構成。
     */
    private: static void set_state_format(
        typename this_type::state_registry& io_state,
        typename this_type::format_type const in_format)
    PSYQ_NOEXCEPT
    {
        typename this_type::field_type const local_format(in_format);
        auto const local_mask(
            this_type::field_SIZE_MASK << this_type::field_SIZE_FRONT);
        io_state.field = (~local_mask & io_state.field)
            | (local_mask & (local_format << this_type::field_SIZE_FRONT));
    }

    private: static typename this_type::block_type make_block_mask(
        typename this_type::size_type const in_size)
    PSYQ_NOEXCEPT
    {
        auto const local_max(
            (std::numeric_limits<typename this_type::block_type>::max)());
        return in_size < this_type::BLOCK_SIZE?
            ~(local_max << in_size): local_max;
    }

    //-------------------------------------------------------------------------
    /// @brief 状態値登記のコンテナ。
    private: typename this_type::state_vector states_;
    /// @brief ビット列チャンクのコンテナ。
    private: typename this_type::chunk_vector chunks_;

}; // class psyq::scenario_engine::reservoir

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 状態値。
template<typename template_unsigned, typename template_float>
class psyq::scenario_engine::_private::state_value
{
    /// @brief thisが指す値の型。
    private: typedef state_value this_type;

    /// @brief 状態値で扱う符号なし整数の型。
    public: typedef template_unsigned unsigned_type;
    static_assert(std::is_unsigned<template_unsigned>::value, "");

    /// @brief 状態値で扱う符号あり整数の型。
    public: typedef
        typename std::make_signed<typename this_type::unsigned_type>::type
            signed_type;

    /// @brief 状態値で扱う浮動小数点数の型。
    public: typedef template_float float_type;
    static_assert(std::is_floating_point<template_float>::value, "");

    /// @brief 状態値の型の種別。
    public: enum kind_enum: std::int8_t
    {
        kind_SIGNED = -2, ///< 符号あり整数。
        kind_FLOAT,       ///< 浮動小数点数。
        kind_NULL,        ///< 空。
        kind_BOOL,        ///< 真偽値。
        kind_UNSIGNED,    ///< 符号なし整数。
    };

    /// @brief this_type::compare の戻り値の型。
    public: enum compare_enum: std::int8_t
    {
        compare_FAILED = -2, ///< 比較に失敗。
        compare_LESS,        ///< 左辺のほうが小さい。
        compare_EQUAL,       ///< 左辺と右辺は等価。
        compare_GREATER,     ///< 左辺のほうが大きい。
    };

    //-------------------------------------------------------------------------
    public: state_value() PSYQ_NOEXCEPT: kind_(this_type::kind_NULL) {}

    public: explicit state_value(bool const in_bool)
    PSYQ_NOEXCEPT:
    kind_(this_type::kind_BOOL)
    {
        this->bool_ = in_bool;
    }

    public: explicit state_value(typename this_type::unsigned_type in_unsigned)
    PSYQ_NOEXCEPT:
    kind_(this_type::kind_UNSIGNED)
    {
        this->unsigned_ = in_unsigned;
    }

    public: explicit state_value(typename this_type::signed_type in_signed)
    PSYQ_NOEXCEPT:
    kind_(this_type::kind_SIGNED)
    {
        this->signed_ = in_signed;
    }

    public: explicit state_value(typename this_type::float_type in_float)
    PSYQ_NOEXCEPT:
    kind_(this_type::kind_FLOAT)
    {
        this->float_ = in_float;
    }

    //-------------------------------------------------------------------------
    public: typename this_type::kind_enum get_kind() const PSYQ_NOEXCEPT
    {
        return this->kind_;
    }

    public: bool const* get_bool() const PSYQ_NOEXCEPT
    {
        return this->get_kind() == this_type::kind_BOOL? &this->bool_: nullptr;
    }

    public: typename this_type::unsigned_type const* get_unsigned()
    const PSYQ_NOEXCEPT
    {
        return this->get_kind() == this_type::kind_UNSIGNED?
            &this->unsigned_: nullptr;
    }

    public: typename this_type::signed_type const* get_signed()
    const PSYQ_NOEXCEPT
    {
        return this->get_kind() == this_type::kind_SIGNED?
            &this->signed_: nullptr;
    }

    public: typename this_type::float_type const* get_float()
    const PSYQ_NOEXCEPT
    {
        return this->get_kind() == this_type::kind_FLOAT?
            &this->float_: nullptr;
    }

    //-------------------------------------------------------------------------
    public: typename this_type::compare_enum compare(this_type const& in_right)
    const PSYQ_NOEXCEPT
    {
        switch (this->get_kind())
        {
            case this_type::kind_BOOL:
            if (in_right.get_kind() != this_type::kind_BOOL)
            {
                return this_type::compare_FAILED;
            }
            return this->bool_ == in_right.bool_?
                this_type::compare_EQUAL:
                (this->bool_?
                    this_type::compare_GREATER: this_type::compare_LESS);

            case this_type::kind_UNSIGNED:
            return this_type::compare_unsigned(this->unsigned_, in_right);

            case this_type::kind_SIGNED:
            return this_type::compare_signed(this->signed_, in_right);

            case this_type::kind_FLOAT:
            return this_type::compare_float(this->float_, in_right);

            default: return this_type::compare_FAILED;
        }
    }

    private: static typename this_type::compare_enum compare_unsigned(
        typename this_type::unsigned_type const in_left,
        this_type const& in_right)
    {
        switch (in_right.get_kind())
        {
            case this_type::kind_UNSIGNED:
            return this_type::compare_value(in_left, in_right.unsigned_);

            case this_type::kind_SIGNED:
            return in_right.signed_ < 0?
                this_type::compare_GREATER:
                this_type::compare_value(in_left, in_right.unsigned_);

            case this_type::kind_FLOAT:
            return this_type::compare_float_right(in_left, in_right.float_);

            default: return this_type::compare_FAILED;
        }
    }

    private: static typename this_type::compare_enum compare_signed(
        typename this_type::signed_type const in_left,
        this_type const& in_right)
    {
        switch (in_right.get_kind())
        {
            case this_type::kind_UNSIGNED:
            return in_left < 0?
                this_type::compare_LESS:
                this_type::compare_value(
                    static_cast<typename this_type::unsigned_type>(in_left),
                    in_right.unsigned_);

            case this_type::kind_SIGNED:
            return this_type::compare_value(in_left, in_right.signed_);

            case this_type::kind_FLOAT:
            return this_type::compare_float_right(in_left, in_right.float_);

            default: return this_type::compare_FAILED;
        }
    }

    private: template<typename template_value>
    static typename this_type::compare_enum compare_value(
        template_value in_left,
        template_value in_right)
    {
        return in_left < in_right?
            this_type::compare_LESS:
            (in_right < in_left?
                this_type::compare_GREATER: this_type::compare_EQUAL);
    }

    private: static typename this_type::compare_enum compare_float(
        typename this_type::float_type const in_left,
        this_type const& in_right)
    {
        switch (in_right.get_kind())
        {
            case this_type::kind_UNSIGNED:
            return in_left < 0?
                this_type::compare_LESS:
                this_type::compare_float_left(in_left, in_right.unsigned_);

            case this_type::kind_SIGNED:
            return this_type::compare_float_left(in_left, in_right.signed_);

            case this_type::kind_FLOAT:
            return this_type::compare_value(in_left, in_right.float_);

            default: return this_type::compare_FAILED;
        }
    }
    private: template<typename template_value>
    static typename this_type::compare_enum compare_float_left(
        typename this_type::float_type const in_left,
        template_value const in_right)
    {
        auto const local_right(
            static_cast<typename this_type::float_type>(in_right));
        return static_cast<template_value>(local_right) != in_right?
            this_type::compare_FAILED:
            this_type::compare_value(in_left, local_right);
    }
    private: template<typename template_value>
    static typename this_type::compare_enum compare_float_right(
        template_value const in_left,
        typename this_type::float_type const in_right)
    {
        auto const local_comapre(
            this_type::compare_float_left(in_right, in_left));
        switch (local_comapre)
        {
            case this_type::compare_LESS:    return this_type::compare_GREATER;
            case this_type::compare_GREATER: return this_type::compare_LESS;
            default:                         return local_comapre;
        }
    }

    //-------------------------------------------------------------------------
    private: union
    {
        bool bool_;
        typename this_type::unsigned_type unsigned_;
        typename this_type::signed_type signed_;
        typename this_type::float_type float_;
    };
    private: typename this_type::kind_enum kind_;

}; // class psyq::scenario_engine::_private::state_value

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 識別値を比較する関数オブジェクト。
template<typename template_value, typename template_key>
struct psyq::scenario_engine::_private::key_less
{
    bool operator()(
        template_value const& in_left,
        template_value const& in_right)
    const PSYQ_NOEXCEPT
    {
        return in_left.key < in_right.key;
    }

    bool operator()(
        template_key const& in_left,
        template_value const& in_right)
    const PSYQ_NOEXCEPT
    {
        return in_left < in_right.key;
    }

    bool operator()(
        template_value const& in_left,
        template_key const& in_right)
    const PSYQ_NOEXCEPT
    {
        return in_left.key < in_right;
    }

    /** @brief コンテナから値を検索する。
        @param[in] in_container 検索するコンテナ。
        @param[in] in_key       検索する値の識別値。
        @retval !=in_container.end() in_key に対応する値を指すポインタ。
        @retval ==in_container.end() in_key に対応する値が見つからなかった。
     */
    template<typename template_container>
    static typename template_container::const_iterator find_const_iterator(
        template_container const& in_container,
        template_key const& in_key)
    PSYQ_NOEXCEPT
    {
        auto const local_end(in_container.end());
        auto const local_lower_bound(
            std::lower_bound(
                in_container.begin(), local_end, in_key, key_less()));
        return local_lower_bound != local_end
            && local_lower_bound->key == in_key?
                local_lower_bound: local_end;
    }

    /** @brief コンテナから値を検索する。
        @param[in] in_container 検索するコンテナ。
        @param[in] in_key       検索する値の識別値。
        @retval !=nullptr in_key に対応する値を指すポインタ。
        @retval ==nullptr in_key に対応する値が見つからなかった。
     */
    template<typename template_container>
    static typename template_container::value_type const* find_const_pointer(
        template_container const& in_container,
        template_key const& in_key)
    PSYQ_NOEXCEPT
    {
        auto const local_end(in_container.end());
        auto const local_lower_bound(
            std::lower_bound(
                in_container.begin(), local_end, in_key, key_less()));
        return local_lower_bound != local_end
            && local_lower_bound->key == in_key?
                &(*local_lower_bound): nullptr;
    }

    template<typename template_container>
    static typename template_container::value_type* find_pointer(
        template_container& in_container,
        template_key const& in_key)
    PSYQ_NOEXCEPT
    {
        return const_cast<typename template_container::value_type*>(
            find_const_pointer(in_container, in_key));
    }

}; // struct psyq::scenario_engine::_private::key_less

#endif // !defined(PSYQ_SCENARIO_ENGINE_RESERVOIR_HPP_)
// vim: set expandtab: