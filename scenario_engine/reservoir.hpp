/** @file
    @brief @copybrief psyq::scenario_engine::_private::reservoir
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_SCENARIO_ENGINE_RESERVOIR_HPP_
#define PSYQ_SCENARIO_ENGINE_RESERVOIR_HPP_

#include "./key_less.hpp"
#include "./state_registry.hpp"
#include "./state_chunk.hpp"
#include "./state_value.hpp"
#include "./state_operation.hpp"

/// @cond
namespace psyq
{
    namespace scenario_engine
    {
        namespace _private
        {
            template<typename, typename, typename, typename> class reservoir;
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
        } // namespace _private
    } // namespace scenario_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief シナリオ状態貯蔵器。任意のビット長の状態値を管理する。

    ### 使い方の概略
    - 以下の関数で、状態値を登録する。
      - reservoir::register_bool
      - reservoir::register_unsigned
      - reservoir::register_signed
      - reservoir::register_float
      - reservoir::register_state
    - reservoir::get_state で、状態値を取得する。
    - reservoir::set_state で、状態値を設定する。

    @tparam template_float     @copydoc reservoir::state_value::float_type
    @tparam template_state_key @copydoc reservoir::state_key
    @tparam template_chunk_key @copydoc reservoir::chunk_key
    @tparam template_allocator @copydoc reservoir::allocator_type
 */
template<
    typename template_float,
    typename template_state_key,
    typename template_chunk_key,
    typename template_allocator>
class psyq::scenario_engine::_private::reservoir
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

    private: typedef
        psyq::scenario_engine::_private::float_union<template_float>
        float_union;

    /// @brief 状態値の登記情報。
    private: typedef
         psyq::scenario_engine::_private::state_registry<
             typename this_type::state_key,
             typename this_type::chunk_key,
             std::uint32_t,
             std::uint8_t>
         state_registry;

    public: enum: typename this_type::state_registry::variety
    {
        EMPTY_VARIETY = this_type::state_registry::EMPTY_VARIETY,
    };

    //-------------------------------------------------------------------------
    /// @brief 状態値を格納するビット列のチャンク。
    private: typedef
        psyq::scenario_engine::_private::state_chunk<
            typename this_type::chunk_key,
            std::vector<std::uint64_t, typename this_type::allocator_type>,
            std::vector<
                typename this_type::state_registry::format,
                typename this_type::allocator_type>>
        chunk;

    /// @brief 状態値ビット列チャンクのコンテナ。
    private: typedef
         std::vector<
             typename this_type::chunk, typename this_type::allocator_type>
         chunk_container;

    /// @brief チャンク識別値を比較する関数オブジェクト。
    private: typedef
         psyq::scenario_engine::_private::key_less<
             psyq::scenario_engine::_private::object_key_getter<
                 typename this_type::chunk, typename this_type::chunk_key>>
         chunk_key_less;

    //-------------------------------------------------------------------------
    /// @brief 状態値。
    public: typedef
        psyq::scenario_engine::_private::state_value<
            typename this_type::chunk::block, template_float>
        state_value;

    /// @brief 状態値の比較演算。
    public: typedef
        psyq::scenario_engine::_private::state_operation<
            typename this_type::state_key,
            typename this_type::state_value::comparison,
            typename this_type::state_value>
        state_comparison;

    /// @brief 状態値の代入演算。
    public: typedef
        psyq::scenario_engine::_private::state_operation<
            typename this_type::state_key,
            typename this_type::state_value::assignment,
            typename this_type::state_value>
        state_assignment;

    /// @brief 状態値登記のコンテナ。
    private: typedef
        std::vector<
            typename this_type::state_registry,
            typename this_type::allocator_type>
        state_container;

    /// @brief 状態値登記ポインタのコンテナ。
    private: typedef
        std::vector<
            typename this_type::state_registry const*,
            typename this_type::allocator_type>
        state_pointer_container;

    /// @brief 状態値に対応する識別値を比較する関数オブジェクト。
    private: typedef
         psyq::scenario_engine::_private::key_less<
             psyq::scenario_engine::_private::object_key_getter<
                 typename this_type::state_registry,
                 typename this_type::state_key>>
         state_key_less;

    //-------------------------------------------------------------------------
    public: enum: typename this_type::state_registry::bit_width
    {
        /// @brief 浮動小数点数型のビット数。
        FLOAT_WIDTH = sizeof(template_float) *
            psyq::scenario_engine::_private::BITS_PER_BYTE,
    };
    static_assert(
        // this_type::chunk::block に浮動小数点数が収まることを確認する。
        this_type::FLOAT_WIDTH <= this_type::chunk::BLOCK_WIDTH,
        "");
    static_assert(
        // state_registry::bit_position に
        // ビット位置の最大値が収まることを確認する。
        this_type::state_registry::format_POSITION_BACK -
            this_type::state_registry::format_POSITION_FRONT
        < sizeof(typename this_type::state_registry::bit_position) *
            psyq::scenario_engine::_private::BITS_PER_BYTE,
        "");
    static_assert(
        // state_registry::bit_width にビット数の最大値が収まることを確認する。
        this_type::state_registry::format_WIDTH_BACK -
            this_type::state_registry::format_WIDTH_FRONT
        < sizeof(typename this_type::state_registry::bit_width) *
            psyq::scenario_engine::_private::BITS_PER_BYTE,
        "");

    //-------------------------------------------------------------------------
    /** @name 構築と代入
        @{
     */
    /** @brief 空の状態貯蔵器を構築する。
        @param[in] in_reserve_states 状態値の予約数。
        @param[in] in_reserve_chunks 状態値ビット列チャンクの予約数。
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

#ifdef PSYQ_NO_STD_DEFAULTED_FUNCTION
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
#endif // defined(PSYQ_NO_STD_DEFAULTED_FUNCTION)

    /** @brief 状態貯蔵器で使われているメモリ割当子を取得する。
        @return 状態貯蔵器で使われているメモリ割当子。
     */
    public: typename this_type::allocator_type get_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->states_.get_allocator();
    }

    /// @brief 状態貯蔵器を再構築し、メモリ領域を必要最小限にする。
    public: void shrink_to_fit()
    {
        // 新たな状態貯蔵器を用意する。
        this_type local_reservoir(
            this->states_.size(),
            this->chunks_.size(),
            this->states_.get_allocator());
        for (auto& local_old_chunk: this->chunks_)
        {
            auto& local_new_chunk(
                *local_reservoir.chunks_.insert(
                    local_reservoir.chunks_.end(),
                    typename this_type::chunk_container::value_type(
                        local_old_chunk.key_, this->states_.get_allocator())));
            local_new_chunk.blocks_.reserve(local_old_chunk.blocks_.size());
            local_new_chunk.empty_fields_.reserve(
                local_old_chunk.empty_fields_.size());
        }

        // 現在の状態貯蔵器をもとに、新たな状態貯蔵器を構築する。
        for (auto local_state: this_type::sort_state_by_width(this->states_))
        {
            local_reservoir.copy_state(*local_state, this->chunks_);
        }

        // 新たに構築した状態貯蔵器を移動して整理する。
        *this = std::move(local_reservoir);
        this->states_.shrink_to_fit();
        this->chunks_.shrink_to_fit();
        for (auto& local_chunk: this->chunks_)
        {
            local_chunk.blocks_.shrink_to_fit();
            local_chunk.empty_fields_.shrink_to_fit();
        }
    }
    /// @}
    //-------------------------------------------------------------------------
    /** @name 状態値の登録
        @{
     */
    /** @brief 論理型の状態値を登録する。

        - 登録した状態値は
          this_type::get_state と this_type::set_state でアクセスできる。
        - 登録した状態値は this_type::erase_chunk でチャンク毎に削除できる。

        @param[in] in_chunk_key   登録する状態値を格納する状態値ビット列チャンクの識別値。
        @param[in] in_state_key   登録する状態値の識別番号。
        @param[in] in_state_value 登録する状態値の初期値。
        @retval true 成功。状態値を登録した。
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
            this->insert_state_registry(
                local_chunk,
                std::move(in_state_key),
                this_type::state_value::kind_BOOL));
        if (local_state == nullptr)
        {
            return false;
        }
        return 0 <= local_chunk.set_bits(
            local_state->get_position(), 1, in_state_value);
    }

    /** @brief 符号なし整数型の状態値を登録する。

        - 登録した状態値は
          this_type::get_state と this_type::set_state でアクセスできる。
        - 登録した状態値は this_type::erase_chunk で削除できる。

        @param[in] in_chunk_key   登録する状態値を格納する状態値ビット列チャンクの識別値。
        @param[in] in_state_key   登録する状態値の識別番号。
        @param[in] in_state_value 登録する状態値の初期値。
        @param[in] in_state_width  登録する状態値のビット数。
        @retval true 成功。状態値を登録した。
        @retval false
            失敗。状態値を登録できなかった。
            - in_state_key に対応する状態値がすでに登録されていると失敗する。
            - this_type::chunk::BLOKK_SIZE より in_state_width が大きければ失敗する。
     */
    public: bool register_unsigned(
        typename this_type::chunk_key in_chunk_key,
        typename this_type::state_key in_state_key,
        typename this_type::state_value::unsigned_type const in_state_value,
        std::size_t const in_state_width)
    {
        // 登録不可能な状態値か判定する。
        auto const local_variety(
            static_cast<typename this_type::state_registry::variety>(
                in_state_width));
        if (this_type::chunk::BLOCK_WIDTH < in_state_width
            || local_variety < this_type::state_value::kind_UNSIGNED)
        {
            return false;
        }

        // 状態値を登録した後、状態値に初期値を設定する。
        auto& local_chunk(
            this_type::equip_chunk(this->chunks_, std::move(in_chunk_key)));
        auto const local_state(
            this->insert_state_registry(
                local_chunk, std::move(in_state_key), local_variety));
        if (local_state == nullptr)
        {
            return false;
        }
        return 0 <= local_chunk.set_bits(
            local_state->get_position(), in_state_width, in_state_value);
    }

    /** @brief 符号あり整数型の状態値を登録する。
        @copydetails register_unsigned
     */
    public: bool register_signed(
        typename this_type::chunk_key in_chunk_key,
        typename this_type::state_key in_state_key,
        typename this_type::state_value::signed_type const in_state_value,
        std::size_t const in_state_width)
    {
        // 登録不可能な状態値か判定する。
        auto const local_variety(
            -static_cast<typename this_type::state_registry::variety>(
                in_state_width));
        if (this_type::chunk::BLOCK_WIDTH < in_state_width
            || this_type::state_value::kind_SIGNED < local_variety)
        {
            return false;
        }

        // 状態値を登録した後、状態値に初期値を設定する。
        auto& local_chunk(
            this_type::equip_chunk(this->chunks_, std::move(in_chunk_key)));
        auto const local_state(
            this->insert_state_registry(
                local_chunk, std::move(in_state_key), local_variety));
        if (local_state == nullptr)
        {
            return false;
        }
        auto const local_width(
            static_cast<typename this_type::state_registry::bit_width>(
                in_state_width));
        auto local_bits(
            static_cast<typename this_type::chunk::block>(in_state_value));
        if (in_state_value < 0)
        {
            auto const local_mask(
                this_type::chunk::make_block_mask(local_width));
            if ((~local_mask & local_bits) != ~local_mask)
            {
                return false;
            }
            local_bits &= local_mask;
        }
        return 0 <= local_chunk.set_bits(
            local_state->get_position(), local_width, local_bits);
    }

    /** @brief 浮動小数点数型の状態値を登録する。
        @copydetails register_bool
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
            this->insert_state_registry(
                local_chunk,
                std::move(in_state_key),
                this_type::state_value::kind_FLOAT));
        if (local_state == nullptr)
        {
            return false;
        }
        typename this_type::float_union local_float;
        local_float.value = in_state_value;
        return 0 <= local_chunk.set_bits(
            local_state->get_position(),
            this_type::FLOAT_WIDTH,
            local_float.bits);
    }

    /** @brief 状態値を登録する。
        @copydetails register_bool
     */
    public: bool register_state(
        typename this_type::chunk_key in_chunk_key,
        typename this_type::state_key in_state_key,
        typename this_type::state_value const& in_state_value)
    {
        auto const local_bool(in_state_value.get_bool());
        if (local_bool != nullptr)
        {
            return this->register_bool(
                std::move(in_chunk_key), std::move(in_state_key), *local_bool);
        }
        auto const local_unsigned(in_state_value.get_unsigned());
        if (local_unsigned != nullptr)
        {
            return this->register_unsigned(
                std::move(in_chunk_key),
                std::move(in_state_key),
                *local_unsigned,
                sizeof(*local_unsigned) *
                    psyq::scenario_engine::_private::BITS_PER_BYTE);
        }
        auto const local_signed(in_state_value.get_signed());
        if (local_signed != nullptr)
        {
            return this->register_signed(
                std::move(in_chunk_key),
                std::move(in_state_key),
                *local_signed,
                sizeof(*local_signed) *
                    psyq::scenario_engine::_private::BITS_PER_BYTE);
        }
        auto const local_float(in_state_value.get_float());
        if (local_float != nullptr)
        {
            return this->register_float(
                std::move(in_chunk_key),
                std::move(in_state_key),
                *local_float);
        }
        return false;
    }
    /// @}
    //-------------------------------------------------------------------------
    /** @name 状態値の取得と設定
        @{
     */
    /** @brief 状態値の種類を取得する。
        @param[in] in_state_key 状態値に対応する識別値。
        @retval !=this_type::EMPTY_VARIETY 状態値の種類。
        @retval ==this_type::EMPTY_VARIETY
            in_state_key に対応する状態値がない。
     */
    public: typename this_type::state_registry::variety get_variety(
        typename this_type::state_key const& in_state_key)
    const PSYQ_NOEXCEPT
    {
        auto local_state(
            this_type::state_key_less::find_const_pointer(
                this->states_, in_state_key));
        return local_state != nullptr?
            local_state->get_variety(): this_type::EMPTY_VARIETY;
    }

    /** @brief 状態値を取得する。

        すでに登録されている状態値から、値を取得する。

        @param[in] in_state_key 取得する状態値に対応する識別値。
        @return
            取得した状態値。状態値の取得に失敗した場合は、
            this_type::state_value::is_empty で偽を返す。
        @sa this_type::register_bool
        @sa this_type::register_unsigned
        @sa this_type::register_signed
        @sa this_type::register_float
        @sa this_type::register_state
        @sa this_type::set_state
     */
    public: typename this_type::state_value get_state(
        typename this_type::state_key const& in_state_key)
    const PSYQ_NOEXCEPT
    {
        // 状態値登記を検索し、状態値ビット列チャンクから状態値のビット列を取得する。
        auto const local_state_registry(
            this_type::state_key_less::find_const_pointer(
                this->states_, in_state_key));
        if (local_state_registry == nullptr)
        {
            return typename this_type::state_value();
        }
        auto const local_chunk(
            this_type::chunk_key_less::find_const_pointer(
                this->chunks_, local_state_registry->chunk_key_));
        if (local_chunk == nullptr)
        {
            // 状態値登記があれば、対応する状態値ビット列チャンクもあるはず。
            PSYQ_ASSERT(false);
            return typename this_type::state_value();
        }
        auto const local_variety(local_state_registry->get_variety());
        auto const local_width(this_type::get_width(local_variety));
        auto const local_bits(
            local_chunk->get_bits(
                local_state_registry->get_position(), local_width));

        // 状態値の種別から、出力値のコピー処理を分岐する。
        switch (local_variety)
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
            if (local_variety < 0)
            {
                // 符号あり整数を取得する。
                auto const local_signed_bits(
                    static_cast<typename this_type::state_value::signed_type>(
                        local_bits));
                auto const local_minus(
                    1 & (local_signed_bits >> (local_width - 1)));
                return typename this_type::state_value(
                    local_signed_bits | (-local_minus << local_width));
            }
            else if (0 < local_variety)
            {
                // 符号なし整数を取得する。
                return typename this_type::state_value(local_bits);
            }
            PSYQ_ASSERT(false);
            return typename this_type::state_value();
        }
    }

    /** @brief 状態値を設定する。

        すでに登録されている状態値に、値を設定する。

        @param[in] in_state_key   設定する状態値に対応する識別値。
        @param[in] in_state_value 状態値に設定する値。
        @retval true 成功。
        @retval false
            失敗。状態値は変化しない。
            - in_state_key に対応する状態値がない場合は失敗する。
            - in_state_value を状態値の型へ変換できない場合は失敗する。
            - 論理型以外の値を論理型の状態値へ設定しようとすると失敗する。
            - 論理型の値を論理型以外の状態値へ設定しようとすると失敗する。
        @sa this_type::register_bool
        @sa this_type::register_unsigned
        @sa this_type::register_signed
        @sa this_type::register_float
        @sa this_type::register_state
        @sa this_type::get_state
     */
    public: bool set_state(
        typename this_type::state_key const& in_state_key,
        typename this_type::state_value const& in_state_value)
    PSYQ_NOEXCEPT
    {
        auto const local_bool(in_state_value.get_bool());
        if (local_bool != nullptr)
        {
            return this->set_state(in_state_key, *local_bool);
        }
        auto const local_unsigned(in_state_value.get_unsigned());
        if (local_unsigned != nullptr)
        {
            return this->set_state(in_state_key, *local_unsigned);
        }
        auto const local_signed(in_state_value.get_signed());
        if (local_signed != nullptr)
        {
            return this->set_state(in_state_key, *local_signed);
        }
        auto const local_float(in_state_value.get_float());
        if (local_float != nullptr)
        {
            return this->set_state(in_state_key, *local_float);
        }
        return false;
    }

    /** @copydoc set_state
        @note
            this_type::state_value::float_type より精度の高い浮動小数点数を
            浮動小数点数型の状態値へ設定しようとすると、
            コンパイル時にエラーか警告が発生する。
     */
    public: template<typename template_value>
    bool set_state(
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
            this_type::state_key_less::find_pointer(
                this->states_, in_state_key));
        if (local_state == nullptr)
        {
            return false;
        }

        // 状態値を設定する状態値ビット列チャンクを決定する。
        auto const local_chunk(
            this_type::chunk_key_less::find_pointer(
                this->chunks_, local_state->chunk_key_));
        if (local_chunk == nullptr)
        {
            // 状態値の登記があるなら、状態値ビット列チャンクもあるはず。
            PSYQ_ASSERT(false);
            return false;
        }

        // 状態値の種別から、設定するビット列とビット数を決定する。
        auto const local_variety(local_state->get_variety());
        typename this_type::state_registry::bit_width local_width;
        typename this_type::chunk::block local_bits;
        switch (local_variety)
        {
            case this_type::state_registry::EMPTY_VARIETY:
            PSYQ_ASSERT(false);
            return false;

            // 真偽値のビット列とビット数を決定する。
            case this_type::state_value::kind_BOOL:
            if (!std::is_same<bool, template_value>::value)
            {
                return false;
            }
            local_width = 1;
            local_bits = static_cast<typename this_type::chunk::block>(
                in_state_value);
            break;

            // 浮動小数点数のビット列とビット数を決定する。
            case this_type::state_value::kind_FLOAT:
            local_width = this_type::make_float_bits(
                local_bits, in_state_value);
            break;

            // 整数のビット列とビット数を決定する。
            default:
            local_width = this_type::make_integer_bits(
                local_bits, in_state_value, local_variety);
            break;
        }
        if (local_width <= 0)
        {
            return false;
        }

        // 状態値にビット列を設定する。
        auto const local_set_bits(
            local_chunk->set_bits(
                local_state->get_position(), local_width, local_bits));
        if (local_set_bits < 0)
        {
            return false;
        }
        else if (0 < local_set_bits)
        {
            // 状態値の変更を記録する。
            local_state->format_ |=
                1 << this_type::state_registry::format_TRANSITION_FRONT;
        }
        return true;
    }
    /// @}
    /** @brief 状態値の種別から、状態値のビット数を取得する。
        @param[in] in_variety 状態値の種別。
        @return 状態値のビット数。
     */
    public:
    static typename this_type::state_registry::bit_width get_width(
        typename this_type::state_registry::variety const in_variety)
    PSYQ_NOEXCEPT
    {
        switch (in_variety)
        {
            case this_type::state_registry::EMPTY_VARIETY:
            return 0;

            case this_type::state_value::kind_BOOL:
            return 1;

            case this_type::state_value::kind_FLOAT:
            return this_type::FLOAT_WIDTH;

            default:
            return in_variety < 0? -in_variety: in_variety;
        }
    }

    /** @brief 状態値の種別から、状態値の型の種別を取得する。
        @param[in] in_variety 状態値の種別。
        @return 状態値の型の種別。
     */
    public: static typename this_type::state_value::kind get_kind(
        typename this_type::state_registry::variety const in_variety)
    PSYQ_NOEXCEPT
    {
        switch (in_variety)
        {
            case this_type::state_value::kind_EMPTY:
            case this_type::state_value::kind_BOOL:
            case this_type::state_value::kind_FLOAT:
            return
                static_cast<typename this_type::state_value::kind>(in_variety);

            default:
            return in_variety < 0?
                this_type::state_value::kind_SIGNED:
                this_type::state_value::kind_UNSIGNED;
        }
    }
    //-------------------------------------------------------------------------
    /** @name 状態値の演算
        @{
     */
    /** @brief 状態値を比較する。
        @param[in] in_comparison 状態値の比較式。
        @return 比較演算の評価結果。
     */
    public: psyq::scenario_engine::evaluation compare_state(
        typename this_type::state_comparison const& in_comparison)
    const PSYQ_NOEXCEPT
    {
        if (!in_comparison.right_state_)
        {
            return this->compare_state(
                in_comparison.key_,
                in_comparison.operator_,
                in_comparison.value_);
        }

        // 右辺となる状態値を取得して演算する。
        auto const local_right_unsigned(in_comparison.value_.get_unsigned());
        if (local_right_unsigned != nullptr)
        {
            auto const local_right_key(
                static_cast<this_type::state_key>(*local_right_unsigned));
            if (local_right_key == *local_right_unsigned)
            {
                return this->compare_state(
                    in_comparison.key_,
                    in_comparison.operator_,
                    this->get_state(local_right_key));
            }
        }
        return -1;
    }

    /** @brief 状態値を比較する。
        @param[in] in_left_key    左辺となる状態値の識別値。
        @param[in] in_operator    適用する比較演算子。
        @param[in] in_right_value 右辺となる値。
        @return 比較演算の評価結果。
     */
    public: psyq::scenario_engine::evaluation compare_state(
        typename this_type::state_key const& in_left_key,
        typename this_type::state_value::comparison const in_operator,
        typename this_type::state_value const& in_right_value)
    const PSYQ_NOEXCEPT
    {
        return
            this->get_state(in_left_key).compare(in_operator, in_right_value);
    }

    /** @brief 状態値を演算し、結果を代入する。
        @param[in] in_assignment 状態値の代入式。
        @retval true  演算結果を状態値へ代入した。
        @retval false 演算に失敗。状態値は変化しない。
     */
    public: bool assign_state(
        typename this_type::state_assignment const& in_assignment)
    {
        if (!in_assignment.right_state_)
        {
            return this->assign_state(
                in_assignment.key_,
                in_assignment.operator_,
                in_assignment.value_);
        }

        // 右辺となる状態値を取得して演算する。
        auto const local_right_unsigned(in_assignment.value_.get_unsigned());
        if (local_right_unsigned != nullptr)
        {
            auto const local_right_key(
                static_cast<this_type::state_key>(*local_right_unsigned));
            if (local_right_key == *local_right_unsigned)
            {
                return this->assign_state(
                    in_assignment.key_,
                    in_assignment.operator_,
                    this->get_state(local_right_key));
            }
        }
        return false;
    }

    /** @brief 状態値を演算し、結果を代入する。
        @param[in] in_left_key    左辺となる状態値の識別値。
        @param[in] in_operator    適用する代入演算子。
        @param[in] in_right_value 右辺となる値。
        @retval true  演算結果を状態値へ代入した。
        @retval false 演算に失敗。状態値は変化しない。
     */
    public: bool assign_state(
        typename this_type::state_key const& in_left_key,
        typename this_type::state_value::assignment const in_operator,
        typename this_type::state_value const& in_right_value)
    {
        auto local_left_value(this->get_state(in_left_key));
        return local_left_value.assign(in_operator, in_right_value)
            && this->set_state(in_left_key, local_left_value);
    }
    /// @}
    //-------------------------------------------------------------------------
    /** @name 状態値の変化
        @{
     */
    /** @brief psyq::scenario_engine 管理者以外は、この関数は使用禁止。

        状態変化フラグを取得する。

        @param[in] in_state_key 変化フラグを取得する状態値に対応する識別値。
        @retval 正 状態変化フラグは真。
        @retval 0  状態変化フラグは偽。
        @retval 負 状態値がない。
     */
    public: std::int8_t _get_transition(
        typename this_type::state_key const& in_state_key)
    const PSYQ_NOEXCEPT
    {
        auto local_state(
            this_type::state_key_less::find_const_pointer(
                this->states_, in_state_key));
        return local_state != nullptr? local_state->get_transition(): -1;
    }

    /** @brief psyq::scenario_engine 管理者以外は、この関数は使用禁止。

        状態変化フラグを初期化する。
     */
    public: void _reset_transition()
    {
        typename this_type::state_registry::format const local_transition_mask(
            ~(1 << this_type::state_registry::format_TRANSITION_FRONT));
        for (auto& local_state: this->states_)
        {
            local_state.format_ &= local_transition_mask;
        }
    }
    /// @}
    //-------------------------------------------------------------------------
    /** @name 状態値ビット列チャンク
        @{
     */
    /** @brief 状態値ビット列チャンクを予約する。
        @param[in] in_chunk_key            予約する状態値ビット列チャンクの識別値。
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
        local_chunk.blocks_.reserve(in_reserve_blocks);
        local_chunk.empty_fields_.reserve(in_reserve_empty_fields);
    }

    /** @brief 状態値ビット列チャンクを破棄する。
        @param[in] in_chunk_key 破棄する状態値ビット列チャンクの識別値。
        @retval true  成功。チャンクを破棄した。
        @retval false 失敗。識別値に対応するチャンクが存在しない。
     */
    public: bool erase_chunk(
        typename this_type::chunk_key const& in_chunk_key)
    {
        // 状態値ビット列チャンクを削除する。
        auto const local_lower_bound(
            std::lower_bound(
                this->chunks_.begin(),
                this->chunks_.end(),
                in_chunk_key,
                typename this_type::chunk_key_less()));
        if (local_lower_bound == this->chunks_.end()
            || local_lower_bound->key_ != in_chunk_key)
        {
            return false;
        }
        this->chunks_.erase(local_lower_bound);

        // 状態値を削除する。
        for (auto i(this->states_.begin()); i != this->states_.end();)
        {
            if (in_chunk_key != i->chunk_key_)
            {
                ++i;
            }
            else
            {
                i = this->states_.erase(i);
            }
        }
        return true;
    }

    /** @brief 状態値ビット列チャンクをシリアル化する。
        @param[in] in_chunk_key シリアル化する状態値ビット列チャンクの識別番号。
        @return シリアル化した状態値ビット列チャンク。
        @todo 未実装。
     */
    public: typename this_type::chunk::block_container serialize_chunk(
        typename this_type::chunk_key const& in_chunk_key)
    const;

    /** @brief シリアル化された状態値ビット列チャンクを復元する。
        @param[in] in_chunk_key        復元する状態値ビット列チャンクの識別値。
        @param[in] in_serialized_chunk シリアル化された状態値ビット列チャンク。
        @todo 未実装。
     */
    public: bool deserialize_chunk(
        typename this_type::chunk_key const& in_chunk_key,
        typename this_type::chunk::block_container const& in_serialized_chunk);
    /// @}
    //-------------------------------------------------------------------------
    /** @brief ビット領域の大きさの降順で並び替えた状態値のコンテナを作る。
     */
    private:
    static typename this_type::state_pointer_container sort_state_by_width(
        typename this_type::state_container const& in_states)
    {
        typename this_type::state_pointer_container local_states(
            in_states.get_allocator());
        local_states.reserve(in_states.size());
        for (auto& local_state: in_states)
        {
            local_states.push_back(&local_state);
        }
        struct state_size_greater
        {
            bool operator()(
                typename this_type::state_registry const* const in_left,
                typename this_type::state_registry const* const in_right)
            const PSYQ_NOEXCEPT
            {
                auto const local_left_width(
                    this_type::get_width(in_left->get_variety()));
                auto const local_right_width(
                    this_type::get_width(in_right->get_variety()));
                return local_right_width < local_left_width;
            }
        };
        std::sort(
            local_states.begin(), local_states.end(), state_size_greater());
        return local_states;
    }

    private: void copy_state(
        typename this_type::state_registry const& in_source_state,
        typename this_type::chunk_container const& in_source_chunks)
    {
        // コピー元となる状態値ビット列チャンクを取得する。
        auto const local_source_chunk(
            this_type::chunk_key_less::find_const_pointer(
                in_source_chunks, in_source_state.chunk_key_));
        if (local_source_chunk == nullptr)
        {
            PSYQ_ASSERT(false);
            return;
        }

        // コピー先となる状態値を用意する。
        auto& local_target_chunk(
            this_type::equip_chunk(this->chunks_, in_source_state.chunk_key_));
        auto const local_variety(in_source_state.get_variety());
        auto const local_target_state(
            this->insert_state_registry(
                local_target_chunk, in_source_state.key_, local_variety));
        if (local_target_state == nullptr)
        {
            PSYQ_ASSERT(false);
            return;
        }

        // 状態値ビット列をコピーする。
        auto const local_width(this_type::get_width(local_variety));
        local_target_chunk.set_bits(
            local_target_state->get_position(),
            local_width,
            local_source_chunk->get_bits(
                in_source_state.get_position(), local_width));
        local_target_state->copy_transition(in_source_state);
    }

    //-------------------------------------------------------------------------
    /** @brief 状態値を登録する。
        @param[in,out] io_chunk 登録する状態値が所属する状態値ビット列チャンク。
        @param[in] in_state_key 登録する状態値に対応する識別値。
        @param[in] in_variety   登録する状態値の種別。
        @retval !=nullptr 成功。登録した状態登記。
        @retval ==nullptr
            失敗。状態値を登録できなかった。
            - in_state_key に対応する状態値がすでに登録されていると失敗する。
     */
    private:
    typename this_type::state_container::value_type* insert_state_registry(
        typename this_type::chunk& io_chunk,
        typename this_type::state_key in_state_key,
        typename this_type::state_registry::variety const in_variety)
    {
        // in_state_key と同じ状態登記がないことを確認する。
        auto const local_state_iterator(
            std::lower_bound(
                this->states_.begin(),
                this->states_.end(),
                in_state_key,
                typename this_type::state_key_less()));
        if (local_state_iterator != this->states_.end()
            && local_state_iterator->key_ == in_state_key)
        {
            return nullptr;
        }

        // 状態登記を新たに追加し、状態値ビット列チャンクを用意する。
        PSYQ_ASSERT(in_variety != this_type::state_registry::EMPTY_VARIETY);
        auto& local_state(
            *this->states_.emplace(
                local_state_iterator,
                std::move(in_state_key),
                io_chunk.key_,
                in_variety));

        // 状態値のビット位置を決定する。
        auto const local_set_position(
            local_state.set_position(
                io_chunk.template
                    make_state_field<typename this_type::state_registry>(
                        this_type::get_width(in_variety))));
        if (!local_set_position)
        {
            PSYQ_ASSERT(false);
            local_state.format_ = 0;
            return nullptr;
        }
        return &local_state;
    }

    //-------------------------------------------------------------------------
    /** @brief 浮動小数点数のビット列を取得する。
        @param[out] out_bits 浮動小数点数のビット列を出力する。
        @param[in] in_value  ビット列を取り出す浮動小数点数。
        @retval !=0 成功。ビット列のビット数。
        @retval ==0 失敗。
     */
    private:
    static typename this_type::state_registry::bit_width make_float_bits(
        typename this_type::chunk::block& out_bits,
        float const in_value)
    {
        typename this_type::float_union local_float;
        local_float.value = in_value;
        out_bits = local_float.bits;
        return this_type::FLOAT_WIDTH;
    }

    /// @copydoc make_float_bits
    private:
    static typename this_type::state_registry::bit_width make_float_bits(
        typename this_type::chunk::block& out_bits,
        double const in_value)
    {
        typename this_type::float_union local_float;
        /** @note
            ここでコンパイルエラーか警告が出る場合は
            double から float への型変換が発生しているのが原因。
            set_state の引数を手動で型変換すれば解決するはず。
         */
        local_float.value = in_value;
        out_bits = local_float.bits;
        return this_type::FLOAT_WIDTH;
    }

    /** @brief this_type::make_float_bits の真偽値のためのダミー関数。
        @return 必ず0。
     */
    private:
    static typename this_type::state_registry::bit_width make_float_bits(
        typename this_type::chunk::block&,
        bool const)
    {
        return 0;
    }

    /** @brief 整数から浮動小数点数のビット列を取得する。
        @param[out] out_bits 浮動小数点数のビット列を出力する。
        @param[in] in_value  ビット列を取り出す整数。
        @retval !=0 成功。ビット列のビット数。
        @retval ==0 失敗。
     */
    private: template<typename template_value>
    static typename this_type::state_registry::bit_width make_float_bits(
        typename this_type::chunk::block& out_bits,
        template_value const in_value)
    {
        // 整数を浮動小数点数に変換して、桁あふれが起きてないか判定する。
        typename this_type::float_union local_float;
        local_float.value =
            static_cast<typename this_type::state_value::float_type>(in_value);
        if (static_cast<template_value>(local_float.value) != in_value)
        {
            return 0;
        }
        out_bits = local_float.bits;
        return this_type::FLOAT_WIDTH;
    }

    /** @brief 整数からビット列を取得する。
        @param[out] out_bits  整数のビット列を出力する。
        @param[in] in_value   ビット列を取り出す整数。
        @param[in] in_variety 取り出すビット列の種別。
        @retval !=0 成功。ビット列のビット数。
        @retval ==0 失敗。
     */
    private: template<typename template_value>
    static typename this_type::state_registry::bit_width make_integer_bits(
        typename this_type::chunk::block& out_bits,
        template_value const in_value,
        typename this_type::state_registry::variety const in_variety)
    PSYQ_NOEXCEPT
    {
        // 整数に変換して、桁あふれが起きてないか判定する。
        auto const local_signed(
            static_cast<typename this_type::state_value::signed_type>(
                in_value));
        if (static_cast<template_value>(local_signed) != in_value)
        {
            return 0;
        }

        // 整数からビット列を取り出す。
        auto const local_width(this_type::get_width(in_variety));
        auto const local_mask(this_type::chunk::make_block_mask(local_width));
        out_bits = static_cast<typename this_type::chunk::block>(local_signed);
        if (in_value < 0)
        {
            if (0 < in_variety || (~local_mask & out_bits) != ~local_mask)
            {
                return 0;
            }
            out_bits &= local_mask;
        }
        else if ((~local_mask & out_bits) != 0)
        {
            return 0;
        }
        return local_width;
    }

    /** @brief this_type::make_integer_bits の真偽値のためのダミー関数。
        @return 必ず0。
     */
    private:
    static typename this_type::state_registry::bit_width make_integer_bits(
        typename this_type::chunk::block&,
        bool const,
        typename this_type::state_registry::variety const)
    {
        return 0;
    }

    //-------------------------------------------------------------------------
    /** @brief 識別値に対応する状態値ビット列チャンクを用意する。

        識別値に対応する状態値ビット列チャンクを、コンテナに用意する。
        同じ識別値のチャンクがすでにコンテナにあれば、それを返す。
        同じ識別値のチャンクがコンテナになければ、新たにチャンクを生成する。

        @param[in,out] io_chunks チャンクを用意するコンテナ。
        @param[in] in_chunk_key  用意するチャンクに対応する識別値。
        @return 用意したチャンクへの参照。
     */
    private:
    static typename this_type::chunk_container::value_type& equip_chunk(
        typename this_type::chunk_container& io_chunks,
        typename this_type::chunk_key in_chunk_key)
    {
        auto const local_lower_bound(
            std::lower_bound(
                io_chunks.begin(),
                io_chunks.end(),
                in_chunk_key,
                typename this_type::chunk_key_less()));
        if (local_lower_bound != io_chunks.end()
            && local_lower_bound->key_ == in_chunk_key)
        {
            return *local_lower_bound;
        }
        return *io_chunks.insert(
            local_lower_bound,
            typename this_type::chunk_container::value_type(
                std::move(in_chunk_key), io_chunks.get_allocator()));
    }

    //-------------------------------------------------------------------------
    /// @brief 状態値登記のコンテナ。
    private: typename this_type::state_container states_;
    /// @brief 状態値ビット列チャンクのコンテナ。
    private: typename this_type::chunk_container chunks_;

}; // class psyq::scenario_engine::_private::reservoir

#endif // !defined(PSYQ_SCENARIO_ENGINE_RESERVOIR_HPP_)
// vim: set expandtab:
