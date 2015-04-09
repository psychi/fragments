/** @file
    @brief @copydoc psyq::scenario_engine::driver
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
    - driver::add_reservoir_chunk で、状態値を登録する。
    - driver::add_evaluator_chunk で、条件式を登録する。
    - driver::add_behavior_chunk で、条件挙動を登録する。
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

    /// @brief シナリオ駆動器で用いる条件評価器の型。
    public: typedef psyq::scenario_engine::evaluator<
        typename this_type::reservoir,
        typename this_type::hasher::result_type,
        typename this_type::allocator_type>
            evaluator;

    /// @brief シナリオ駆動器で用いる条件監視器の型。
    public: typedef psyq::scenario_engine::dispatcher<
        typename this_type::reservoir::state_key,
        typename this_type::evaluator::expression_key,
        typename this_type::allocator_type>
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
    evaluator_(in_reserve_expressions, in_reserve_chunks, in_allocator),
    dispatcher_(
        in_reserve_expressions,
        in_reserve_states,
        in_reserve_caches,
        in_allocator),
    behaviors_(in_allocator),
    hash_function_(std::move(in_hash_function))
    {
        this->behaviors_.reserve(in_reserve_chunks);
    }

    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元となるインスタンス。
     */
    public: driver(this_type&& io_source):
    reservoir_(std::move(io_source.reservoir_)),
    evaluator_(std::move(io_source.evaluator_)),
    dispatcher_(std::move(io_source.dispatcher_)),
    hash_function_(std::move(io_source.hash_function_))
    {}

    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元となるインスタンス。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source)
    {
        this->reservoir_ = std::move(io_source.reservoir_);
        this->evaluator_ = std::move(io_source.evaluator_);
        this->dispatcher_ = std::move(io_source.dispatcher_);
        this->hash_function_ = std::move(io_source.hash_function_);
        return *this;
    }

    /// @brief シナリオ駆動器を再構築し、メモリ領域を必要最小限にする。
    public: void shrink_to_fit()
    {
        this->reservoir_.shrink_to_fit();
        this->evaluator_.shrink_to_fit();
        this->dispatcher_.shrink_to_fit();
        this->behaviors_.shrink_to_fit();
    }
    //@}
    /** @brief シナリオ進行を更新する。

        基本的には、フレーム毎に更新すること。
     */
    public: void update()
    {
        this->dispatcher_._detect(this->evaluator_, this->reservoir_);
        this->reservoir_._reset_transition();
        this->dispatcher_._dispatch(this->evaluator_, this->reservoir_);
    }

    /** @brief 文字列からハッシュ値を生成する。
        @param[in] in_string ハッシュ値のもととなる文字列。
        @return 文字列から生成したハッシュ値。
     */
    public: typename this_type::hasher::result_type make_hash(
        typename this_type::hasher::argument_type const& in_string)
    {
        auto const local_hash(this->hash_function_(in_string));
        PSYQ_ASSERT(
            in_string.empty() || local_hash != this->hash_function_(
                typename this_type::hasher::argument_type()));
        return local_hash;
    }

    //-------------------------------------------------------------------------
    /// @name チャンクの追加
    /** @brief シナリオ駆動器に状態値を追加する。

        追加した状態値を削除するには、 this_type::reservoir_ に対して
        this_type::reservoir::remove_chunk を呼び出す。

        @param[in] in_chunk_key 状態値を追加するチャンクの識別値。
        @param[in] in_state_builder
            状態値を登録する関数オブジェクト。
            以下に相当するメンバ関数を使えること。
            @code
            // @brief 状態貯蔵器に状態値を登録する。
            // @param[in,out] io_reservoir 状態値を登録する書庫。
            // @param[in,out] io_hasher    文字列から識別値を生成する関数オブジェクト。
            // @param[in] in_chunk_key     状態値を登録するチャンクを表す識別値。
            // @return 登録した状態値の数。
            std::size_t template_builder::operator()(
                driver::reservoir& io_reservoir,
                driver::hasher& io_hasher,
                driver::reservoir::chunk_key const& in_chunk_key)
            const;
            @endcode
        @return 登録した状態値の数。
     */
    public: template<typename template_builder>
    std::size_t add_reservoir_chunk(
        typename this_type::reservoir::chunk_key const& in_chunk_key,
        template_builder const& in_state_builder)
    {
        return in_state_builder(
            this->reservoir_, this->hash_function_, in_chunk_key);
    }

    /** @brief シナリオ駆動器に条件式を追加する。

        追加した条件式を削除するには、 this_type::evaluator_ に対して
        this_type::evaluator::remove_chunk を呼び出す。

        @param[in] in_chunk_key 条件式を追加するチャンクの識別値。
        @param[in] in_expression_builder
            条件式を登録する関数オブジェクト。
            以下に相当するメンバ関数を使えること。
            @code
            // @brief 条件評価器に条件式を登録する。
            // @param[in,out] io_evaluator 条件式を登録する条件評価器。
            // @param[in,out] io_hasher    文字列から識別値を生成する関数オブジェクト。
            // @param[in] in_chunk_key     条件式を登録するチャンクを表す識別値。
            // @param[in] in_reservoir     条件式で使う状態値の書庫。
            // @return 登録した条件式の数。
            std::size_t template_builder::operator()(
                driver::evaluator& io_evaluator,
                driver::hasher& io_hasher,
                driver::reservoir::key_type const& in_chunk
                driver::reservoir const& in_reservoir)
            const;
            @endcode
        @return 登録した条件式の数。
     */
    public: template<typename template_builder>
    std::size_t add_evaluator_chunk(
        typename this_type::evaluator::reservoir::chunk_key const& in_chunk_key,
        template_builder const& in_expression_builder)
    {
        return in_expression_builder(
            this->evaluator_,
            this->hash_function_,
            in_chunk_key,
            this->reservoir_);
    }

    /** @brief シナリオ駆動器に条件挙動を追加する。
        @param[in] in_chunk     条件挙動を追加するチャンクの識別値。
        @param[in] in_functions 追加する条件挙動関数オブジェクトのコンテナ。
     */
    public: void add_behavior_chunk(
        typename this_type::behavior_chunk::key_type const& in_chunk,
        typename this_type::dispatcher::function_shared_ptr_vector
            in_functions)
    {
        this_type::behavior_chunk::add(
            this->behaviors_, in_chunk, std::move(in_functions));
    }

    /** @brief シナリオ駆動器から条件挙動を削除する。
        @param[in] in_chunk 条件挙動を削除するチャンクのキー。
     */
    public: bool remove_behavior_chunk(
        typename this_type::behavior_chunk::key_type const& in_chunk)
    {
        return this_type::behavior_chunk::remove(this->behaviors_, in_chunk);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @brief シナリオ駆動器で用いる状態貯蔵器。
    public: typename this_type::reservoir reservoir_;

    /// @brief シナリオ駆動器で用いる条件評価器。
    public: typename this_type::evaluator evaluator_;

    /// @brief シナリオ駆動器で用いる条件挙動器。
    public: typename this_type::dispatcher dispatcher_;

    /// @brief シナリオ駆動器で用いる条件挙動チャンクのコンテナ。
    private: typename this_type::behavior_chunk::vector behaviors_;

    /// @brief シナリオ駆動器で用いるハッシュ関数オブジェクト。
    public: typename this_type::hasher hash_function_;

}; // class psyq::scenario_engine::driver

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq_test
{
    inline void scenario_engine()
    {
        typedef psyq::scenario_engine::state_builder<std::string>
            state_builder;
        typedef state_builder::string_table string_table;
        typedef psyq::scenario_engine
            ::driver<float, string_table::string_view::fnv1_hash32>
                driver;
        driver local_driver(16, 16, 16);
        auto const local_chunk_key(local_driver.hash_function_("chunk_0"));

        // 状態値を登録する。
        string_table::string_view const local_state_table_csv(
            "KEY,            KIND,      VALUE,\n"
            "state_bool,     BOOL,       TRUE,\n"
            "state_unsigned, UNSIGNED_7,   10,\n"
            "state_signed,   SIGNED_13,   -20,\n"
            "state_float,    FLOAT,      1.25,\n"
            );
        local_driver.add_reservoir_chunk(
            local_chunk_key,
            state_builder(
                state_builder::string_table(local_state_table_csv, 0)));

        // 条件式を登録する。
        string_table::string_view const local_expression_table_csv(
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
            "expression_9, AND,   STATE_COMPARISON, state_unsigned, ==, 0,\n");
        typedef psyq::scenario_engine::expression_builder<string_table::string>
            expression_builder;
        local_driver.add_evaluator_chunk(
            local_chunk_key,
            expression_builder(
                expression_builder::string_table(
                    local_expression_table_csv, 0)));

        // 条件挙動チャンクを登録する。
        string_table::string_view const local_behavior_table_csv(
            "KEY         , CONDITION, KIND,  ARGUMENT\n"
            "expression_0, TRUE,      STATE, state_unsigned, :=, 1\n"
            "expression_1, TRUE,      STATE, state_unsigned, +=, 1\n"
            "expression_2, TRUE,      STATE, state_unsigned, -=, 1\n"
            "expression_3, TRUE,      STATE, state_unsigned, *=, 1\n"
            "expression_4, TRUE,      STATE, state_unsigned, /=, 1\n"
            "expression_5, TRUE,      STATE, state_unsigned, %=, 1\n"
            "expression_6, TRUE,      STATE, state_unsigned, |=, 1\n"
            "expression_7, TRUE,      STATE, state_unsigned, ^=, 0\n"
            "expression_8, TRUE,      STATE, state_unsigned, &=, 0\n");
        typedef psyq::scenario_engine::behavior_builder<driver::dispatcher>
            behavior_builder;
        local_driver.add_behavior_chunk(
            local_chunk_key,
            behavior_builder::build(
                local_driver.dispatcher_,
                local_driver.hash_function_,
                local_driver.evaluator_,
                local_driver.reservoir_,
                string_table(local_behavior_table_csv, 0)));

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
    }
}
#endif // !defined(PSYQ_SCENARIO_ENGINE_DRIVER_HPP_)
// vim: set expandtab:
