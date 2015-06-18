/** @file
    @brief @copybrief psyq::scenario_engine::driver
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_SCENARIO_ENGINE_DRIVER_HPP_
#define PSYQ_SCENARIO_ENGINE_DRIVER_HPP_

//#include "scenario_engine/evaluator.hpp"
//#include "scenario_engine/behavior.hpp"

/// @cond
namespace psyq
{
    namespace scenario_engine
    {
        template<typename, typename, typename> class driver;
    } // namespace scenario_engine
} // namespace psyq
/// @endcond


//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief シナリオ駆動器。シナリオ進行の全体を統括して管理する。 

    ### 使い方の概略
    - driver::driver でシナリオ駆動機を構築する。
    - driver::add_chunk で、状態値と条件式と条件挙動を登録する。
    - driver::update をフレームごとに呼び出す。
      条件式の評価が変化して条件に合致していたら、
      条件挙動関数オブジェクトが呼び出される。

    @tparam template_float     @copydoc reservoir::state_value::float_type
    @tparam template_hasher    @copydoc hasher
    @tparam template_allocator @copydoc allocator_type
 */
template<
    typename template_float = float,
    typename template_hasher = psyq::string::view<char>::fnv1_hash32,
    typename template_allocator = std::allocator<void*>>
class psyq::scenario_engine::driver
{
    /// @brief thisが指す値の型。
    private: typedef driver this_type;

    /** @brief 文字列からハッシュ値を生成する、ハッシュ関数オブジェクトの型。

        - std::hash 互換インターフェイスを持つこと。
        - hasher::argument_type
          が文字列型で、以下に相当するメンバ関数を使えること。
          @code
          // @brief 文字列の先頭位置を取得する。
          hasher::argument_type::const_pointer hasher::argument_type::data() const;
          // @brief 文字列の要素数を取得する。
          std::size_t hasher::argument_type::size() const;
          @endcode
     */
    public: typedef template_hasher hasher;

    /// @brief 各種コンテナに用いるメモリ割当子の型。
    public: typedef template_allocator allocator_type;

    /// @brief シナリオ駆動器で用いる状態貯蔵器の型。
    public: typedef psyq::scenario_engine::reservoir<
        template_float,
        typename this_type::hasher::result_type,
        typename this_type::hasher::result_type,
        typename this_type::allocator_type>
            reservoir;

    /// @brief シナリオ駆動器で用いる状態変更器の型。
    public: typedef psyq::scenario_engine::modifier<
        typename this_type::reservoir>
            modifier;

    /// @brief シナリオ駆動器で用いる条件評価器の型。
    public: typedef psyq::scenario_engine::evaluator<
        typename this_type::reservoir, typename this_type::hasher::result_type>
            evaluator;

    /// @brief シナリオ駆動器で用いる条件監視器の型。
    public: typedef psyq::scenario_engine::dispatcher<
        typename this_type::evaluator, std::int32_t>
            dispatcher;

    /// @brief シナリオ駆動器で用いる条件挙動チャンクの型。
    public: typedef psyq::scenario_engine::behavior_chunk<
        typename this_type::dispatcher>
            behavior_chunk;

    //-------------------------------------------------------------------------
    /// @name 構築と代入
    //@{
    /** @brief 空のシナリオ駆動器を構築する。
        @param[in] in_reserve_chunks      チャンクの予約数。
        @param[in] in_reserve_states      状態値の予約数。
        @param[in] in_reserve_expressions 条件式の予約数。
        @param[in] in_reserve_caches      条件挙動キャッシュの予約数。
        @param[in] in_hash_function       ハッシュ関数オブジェクトの初期値。
        @param[in] in_allocator           メモリ割当子の初期値。
     */
    public: driver(
        std::size_t const in_reserve_chunks,
        std::size_t const in_reserve_states,
        std::size_t const in_reserve_expressions,
        std::size_t const in_reserve_caches = 16,
        typename this_type::hasher in_hash_function = this_type::hasher(),
        typename this_type::allocator_type const& in_allocator =
            this_type::allocator_type())
    :
    reservoir_(in_reserve_states, in_reserve_chunks, in_allocator),
    modifier_(in_reserve_caches, in_allocator),
    evaluator_(in_reserve_expressions, in_reserve_chunks, in_allocator),
    dispatcher_(
        in_reserve_expressions,
        in_reserve_states,
        in_reserve_caches,
        in_allocator),
    behavior_chunks_(in_allocator),
    hash_function_(std::move(in_hash_function))
    {
        this->behavior_chunks_.reserve(in_reserve_chunks);
    }

    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元となるインスタンス。
     */
    public: driver(this_type&& io_source):
    reservoir_(std::move(io_source.reservoir_)),
    modifier_(std::move(io_source.modifier_)),
    evaluator_(std::move(io_source.evaluator_)),
    dispatcher_(std::move(io_source.dispatcher_)),
    behavior_chunks_(std::move(io_source.behavior_chunks_)),
    hash_function_(std::move(io_source.hash_function_))
    {}

    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元となるインスタンス。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source)
    {
        this->reservoir_ = std::move(io_source.reservoir_);
        this->modifier_ = std::move(io_source.modifier_);
        this->evaluator_ = std::move(io_source.evaluator_);
        this->dispatcher_ = std::move(io_source.dispatcher_);
        this->behavior_chunks_ = std::move(io_source.behavior_chunks_);
        this->hash_function_ = std::move(io_source.hash_function_);
        return *this;
    }

    /// @brief シナリオ駆動器を再構築し、メモリ領域を必要最小限にする。
    public: void shrink_to_fit()
    {
        this->reservoir_.shrink_to_fit();
        //this->modifier_.shrink_to_fit();
        this->evaluator_.shrink_to_fit();
        this->dispatcher_.shrink_to_fit();
        this->behavior_chunks_.shrink_to_fit();
        for (auto& local_behavior_chunk: this->behavior_chunks_)
        {
            local_behavior_chunk.functions_.shrink_to_fit();
        }
    }
    //@}
    //-------------------------------------------------------------------------
    /** @brief シナリオ進行を更新する。

        基本的には、フレーム毎に更新すること。
     */
    public: void update()
    {
        this->modifier_.modify(this->reservoir_);
        this->dispatcher_._dispatch(this->evaluator_, this->reservoir_);
    }

    //-------------------------------------------------------------------------
    /// @name チャンク
    //@{
    /** @brief 状態値と条件式と条件挙動関数を、チャンクへ追加する。
        @param[in] in_chunk_key 追加するチャンクの識別値。
        @param[in] in_state_builder
            状態値を状態貯蔵器に登録する関数オブジェクト。
            以下に相当するメンバ関数を使えること。
            @code
            // @brief 状態値を状態貯蔵器に登録する。
            // @param[in,out] io_reservoir 状態値を登録する状態貯蔵器。
            // @param[in,out] io_hasher    文字列から識別値を生成する関数オブジェクト。
            // @param[in] in_chunk_key     状態値を登録するチャンクを表す識別値。
            void template_state_builder::operator()(
                driver::reservoir& io_reservoir,
                driver::hasher& io_hasher,
                driver::reservoir::chunk_key const& in_chunk_key)
            const;
            @endcode
        @param[in] in_expression_builder
            条件式を条件評価器に登録する関数オブジェクト。
            以下に相当するメンバ関数を使えること。
            @code
            // @brief 条件式を条件評価器に登録する。
            // @param[in,out] io_evaluator 条件式を登録する条件評価器。
            // @param[in,out] io_hasher    文字列から識別値を生成する関数オブジェクト。
            // @param[in] in_chunk_key     条件式を登録するチャンクを表す識別値。
            // @param[in] in_reservoir     条件式で使う状態貯蔵器。
            void template_expression_builder::operator()(
                driver::evaluator& io_evaluator,
                driver::hasher& io_hasher,
                driver::reservoir::key_type const& in_chunk
                driver::reservoir const& in_reservoir)
            const;
            @endcode
        @param[in] in_behavior_builder
            条件挙動関数を条件挙動器に登録する関数オブジェクト。
            以下に相当するメンバ関数を使えること。
            @code
            // @brief 条件挙動関数を条件挙動器に登録する。
            // @param[in,out] io_dispatcher 条件挙動関数を登録する条件挙動器。
            // @param[in,out] io_hasher     文字列から識別値を生成する関数オブジェクト。
            // @param[in] in_evaluator      条件挙動関数で使う条件評価器。
            // @param[in] in_reservoir      条件挙動関数で使う状態貯蔵器。
            // @return
            //     条件挙動器に登録した条件挙動関数オブジェクトを指す、
            //     スマートポインタのコンテナ。
            driver::dispatcher::function_shared_ptr_vector
            template_behavior_builder::operator()(
                driver::dispatcher& io_dispatcher,
                driver::hasher& io_hasher,
                driver::evaluator const& in_evaluator,
                driver::reservoir const& in_reservoir)
            const;
            @endcode
     */
    public: template<
        typename template_state_builder,
        typename template_expression_builder,
        typename template_behavior_builder>
    void extend_chunk(
        typename this_type::reservoir::chunk_key const& in_chunk_key,
        template_state_builder const& in_state_builder,
        template_expression_builder const& in_expression_builder,
        template_behavior_builder const& in_behavior_builder) 
    {
        in_state_builder(
            this->reservoir_, this->hash_function_, in_chunk_key);
        in_expression_builder(
            this->evaluator_,
            this->hash_function_,
            in_chunk_key,
            this->reservoir_);
        this_type::behavior_chunk::add(
            this->behavior_chunks_,
            in_chunk_key,
            in_behavior_builder(
                this->dispatcher_,
                this->hash_function_,
                this->evaluator_,
                this->reservoir_));
    }

    /** @brief 条件式に対応する条件挙動関数を、チャンクへ追加する。
        @param[in] in_chunk_key      チャンクの識別値。
        @param[in] in_expression_key 評価に用いる条件式の識別値。
        @param[in] in_function
            追加する条件挙動関数オブジェクトを指すスマートポインタ。
        @retval true  成功。
        @retval false 失敗。
     */
    public: bool extend_chunk(
        typename this_type::reservoir::chunk_key const& in_chunk_key,
        typename this_type::evaluator::expression::key const& in_expression_key,
        typename this_type::dispatcher::function_shared_ptr in_function)
    {
        // 条件挙動関数を条件挙動器へ登録する。
        auto const local_register_function(
            this->dispatcher_.register_function(
                in_expression_key, in_function));
        if (!local_register_function)
        {
            return false;
        }

        // 条件挙動関数を条件挙動関数チャンクへ追加する。
        this_type::behavior_chunk::add(
            this->behavior_chunks_, in_chunk_key, std::move(in_function));
        return true;
    }

    /** @brief チャンクを削除する。
        @param[in] in_chunk 削除するチャンクのキー。
     */
    public: void remove_chunk(
        typename this_type::reservoir::chunk_key const& in_chunk)
    {
        this->reservoir_.remove_chunk(in_chunk);
        this->evaluator_.remove_chunk(in_chunk);
        this_type::behavior_chunk::remove(this->behavior_chunks_, in_chunk);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @brief シナリオ駆動器で用いる状態貯蔵器。
    public: typename this_type::reservoir reservoir_;

    /// @brief シナリオ駆動器で用いる状態変更器。
    public: typename this_type::modifier modifier_;

    /// @brief シナリオ駆動器で用いる条件評価器。
    public: typename this_type::evaluator evaluator_;

    /// @brief シナリオ駆動器で用いる条件挙動器。
    public: typename this_type::dispatcher dispatcher_;

    /// @brief シナリオ駆動器で用いる条件挙動チャンクのコンテナ。
    private: typename this_type::behavior_chunk::container behavior_chunks_;

    /// @brief シナリオ駆動器で用いるハッシュ関数オブジェクト。
    public: typename this_type::hasher hash_function_;

}; // class psyq::scenario_engine::driver

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
#include <deque>
namespace psyq_test
{
    inline void scenario_engine()
    {
        typedef psyq::string::csv_table<std::string> string_table;
        typedef psyq::scenario_engine
            ::driver<float, string_table::string_view::fnv1_hash32>
                driver;
        driver local_driver(16, 16, 16);
        auto const local_chunk_key(local_driver.hash_function_("chunk_0"));

        // 状態値テーブルを構築する。
        string_table local_state_table(
            "KEY,            KIND,      VALUE,\n"
            "state_bool,     BOOL,       TRUE,\n"
            "state_unsigned, UNSIGNED_7,   10,\n"
            "state_signed,   SIGNED_13,   -20,\n"
            "state_float,    FLOAT,      1.25,\n"
            "");
        local_state_table.constraint_attribute(0);

        // 条件式テーブルを構築する。
        string_table local_expression_table(
            "KEY,          LOGIC, KIND,             ELEMENT,\n"
            "expression_0, AND,   STATE_COMPARISON, state_bool,     ==, FALSE,\n"
            "expression_1, AND,   STATE_COMPARISON, state_unsigned, <=, 10,\n"
            "expression_2, AND,   STATE_COMPARISON, state_signed,   >=, -20,\n"
            "expression_3, AND,   STATE_COMPARISON, state_unsigned, ==, 0,\n"
            "expression_4, AND,   STATE_COMPARISON, state_unsigned, ==, 0,\n"
            "expression_5, AND,   STATE_COMPARISON, state_unsigned, ==, 0,\n"
            "expression_6, AND,   STATE_COMPARISON, state_unsigned, ==, 0,\n"
            "expression_7, AND,   STATE_COMPARISON, state_unsigned, ==, 0,\n"
            "expression_8, AND,   STATE_COMPARISON, state_unsigned, ==, 0,\n"
            "expression_9, OR,    SUB_EXPRESSION,   expression_0, TRUE, expression_1, FALSE,\n"
            "");
        local_expression_table.constraint_attribute(0);

        // 条件挙動テーブルを構築する。
        string_table local_behavior_table(
            "KEY         , CONDITION, KIND,  ARGUMENT\n"
            "expression_0, TRUE,      STATE, state_unsigned, :=, 1\n"
            "expression_1, TRUE,      STATE, state_unsigned, +=, 1\n"
            "expression_2, TRUE,      STATE, state_unsigned, -=, 1\n"
            "expression_3, TRUE,      STATE, state_unsigned, *=, 1\n"
            "expression_4, TRUE,      STATE, state_unsigned, /=, 1\n"
            "expression_5, TRUE,      STATE, state_unsigned, %=, 1\n"
            "expression_6, TRUE,      STATE, state_unsigned, |=, 1\n"
            "expression_7, TRUE,      STATE, state_unsigned, ^=, 0\n"
            "expression_8, TRUE,      STATE, state_unsigned, &=, 0\n"
            "");
        local_behavior_table.constraint_attribute(0);

        // シナリオ駆動機に登録する。
        local_driver.extend_chunk(
            local_chunk_key,
            psyq::scenario_engine::state_builder<string_table::string>(
                std::move(local_state_table)),
            psyq::scenario_engine::expression_builder<string_table::string>(
                std::move(local_expression_table)),
            psyq::scenario_engine::behavior_builder<string_table::string, driver::dispatcher>(
                std::move(local_behavior_table)));
        PSYQ_ASSERT(
            local_driver.reservoir_.register_value(
                local_chunk_key,
                local_driver.hash_function_("10"),
                driver::reservoir::state_value(10u)));
        PSYQ_ASSERT(!local_driver.extend_chunk(0, 0, nullptr));
        local_driver.shrink_to_fit();
        PSYQ_ASSERT(
            true == *local_driver.reservoir_.get_value(
                local_driver.hash_function_("state_bool")).get_bool());
        PSYQ_ASSERT(
            10 == *local_driver.reservoir_.get_value(
                local_driver.hash_function_("state_unsigned")).get_unsigned());
        PSYQ_ASSERT(
            -20 == *local_driver.reservoir_.get_value(
                local_driver.hash_function_("state_signed")).get_signed());
        PSYQ_ASSERT(
            1.25 <= *local_driver.reservoir_.get_value(
                local_driver.hash_function_("state_float")).get_float());


        local_driver.reservoir_.set_value(
            local_driver.hash_function_("state_bool"), false);
        local_driver.reservoir_.set_value(
            local_driver.hash_function_("state_unsigned"), 10);
        local_driver.reservoir_.set_value(
            local_driver.hash_function_("state_signed"), -20);
        local_driver.reservoir_.set_value(
            local_driver.hash_function_("state_float"), true);
        local_driver.reservoir_.set_value(
            local_driver.hash_function_("state_float"), 0x20u);
        local_driver.reservoir_.set_value(
            local_driver.hash_function_("state_float"), -10);
        local_driver.reservoir_.set_value(
            local_driver.hash_function_("state_float"), 1.25f);
        auto const local_float_state(
            local_driver.reservoir_.get_value(
                local_driver.hash_function_("state_float")));

        local_driver.update();
        local_driver.remove_chunk(local_chunk_key);
    }
}
#endif // !defined(PSYQ_SCENARIO_ENGINE_DRIVER_HPP_)
// vim: set expandtab:
