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
        template<typename, typename> class driver;
    } // namespace scenario_engine
} // namespace psyq
/// @endcond


//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief シナリオ駆動器。シナリオの進行を管理する。 

    @tparam template_hasher @copydoc hasher
    @tparam template_allocator @copydoc allocator_type
 */
template<
    typename template_hasher = psyq::string::view<char>::fnv1_hash32,
    typename template_allocator = std::allocator<void*>>
class psyq::scenario_engine::driver
{
    /// @brief thisが指す値の型。
    private: typedef driver this_type;

    /** @brief 文字列からハッシュ値を生成する、ハッシュ関数オブジェクトの型。

        std::hash 互換インターフェイスを持つこと。
     */
    public: typedef template_hasher hasher;

    /// @brief 各種コンテナに用いるメモリ割当子の型。
    public: typedef template_allocator allocator_type;

    /// @brief シナリオ駆動器で用いる状態値書庫の型。
    public: typedef psyq::scenario_engine::state_archive<
        typename this_type::hasher::result_type,
        typename this_type::allocator_type>
            state_archive;

    /// @brief シナリオ駆動器で用いる条件評価器の型。
    public: typedef psyq::scenario_engine::evaluator<
        typename this_type::state_archive,
        typename this_type::hasher::result_type,
        typename this_type::allocator_type>
            evaluator;

    /// @brief シナリオ駆動器で用いる条件監視器の型。
    public: typedef psyq::scenario_engine::dispatcher<
        typename this_type::hasher::result_type,
        typename this_type::hasher::result_type,
        typename this_type::allocator_type>
            dispatcher;

    /// @brief シナリオ駆動器で用いる条件挙動チャンクの型。
    public: typedef psyq::scenario_engine::behavior_chunk<
        typename this_type::dispatcher>
            behavior_chunk;

    //-------------------------------------------------------------------------
    /** @brief 空のシナリオ駆動器を構築する。
        @param[in] in_reserve_chunks      予約するチャンクの容量。
        @param[in] in_reserve_states      予約する状態値書庫の容量。
        @param[in] in_reserve_expressions 予約する条件式評価器の容量。
        @param[in] in_hasher              ハッシュ関数オブジェクトの初期値。
        @param[in] in_allocator           メモリ割当子の初期値。
     */
    public: driver(
        std::size_t const in_reserve_chunks,
        std::size_t const in_reserve_states,
        std::size_t const in_reserve_expressions,
        typename this_type::hasher in_hasher = this_type::hasher(),
        typename this_type::allocator_type const& in_allocator =
            this_type::allocator_type())
    :
    state_archive_(in_reserve_states, in_reserve_chunks, in_allocator),
    evaluator_(in_reserve_expressions, in_reserve_chunks, in_allocator),
    dispatcher_(in_allocator),
    behaviors_(in_allocator),
    hasher_(std::move(in_hasher))
    {
        this->behaviors_.reserve(in_reserve_chunks);
    }

    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元となるインスタンス。
     */
    public: driver(this_type&& io_source):
    state_archive_(std::move(io_source.state_archive_)),
    evaluator_(std::move(io_source.evaluator_)),
    dispatcher_(std::move(io_source.dispatcher_)),
    hasher_(std::move(io_source.hasher_))
    {}

    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元となるインスタンス。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source)
    {
        this->state_archive_ = std::move(io_source.state_archive_);
        this->evaluator_ = std::move(io_source.evaluator_);
        this->dispatcher_ = std::move(io_source.dispatcher_);
        this->hasher_ = std::move(io_source.hasher_);
        return *this;
    }

    /** @brief シナリオ進行を更新する。

        基本的には、フレーム毎に更新すること。
     */
    public: void update()
    {
        this->dispatcher_.dispatch(this->evaluator_, this->state_archive_);
    }

    //-------------------------------------------------------------------------
    /** @brief シナリオ駆動器で用いるハッシュ関数オブジェクトを取得する。
        @return シナリオ駆動器で用いるハッシュ関数オブジェクト。
     */
    public: typename this_type::hasher const& hash_function()
    const PSYQ_NOEXCEPT
    {
        return this->hasher_;
    }
    /// @copydoc hash_function
    public: typename this_type::hasher& hash_function() PSYQ_NOEXCEPT
    {
        return this->hasher_;
    }

    public: typename this_type::hasher::result_type make_hash(
        typename this_type::hasher::argument_type const& in_key)
    {
        auto const local_hash(this->hasher_(in_key));
        PSYQ_ASSERT(
            in_key.empty() || local_hash != this->hasher_(
                typename this_type::hasher::argument_type()));
        return local_hash;
    }

    //-------------------------------------------------------------------------
    /** @brief シナリオ駆動器で用いる状態値の書庫を取得する。
        @return シナリオ駆動器で用いる状態値の書庫。
     */
    public: typename this_type::state_archive const& get_state_archive()
    const PSYQ_NOEXCEPT
    {
        return this->state_archive_;
    }

    /// @copydoc psyq::scenario_engine::state_archive::set_value
    public: template<typename template_value>
    bool set_state_value(
        typename this_type::state_archive::key_type const& in_key,
        template_value const& in_value)
    PSYQ_NOEXCEPT
    {
        // 状態値を設定し、状態値の書き換えを条件監視器へ通知する。
        auto const local_set_value(
            this->state_archive_.set_value(in_key, in_value));
        if (local_set_value)
        {
            this->dispatcher_.notify_state_transition(in_key);
        }
        return local_set_value;
    }

    //-------------------------------------------------------------------------
    public: void add_behavior_chunk(
        typename this_type::behavior_chunk::key_type const& in_key,
        typename this_type::behavior_chunk::function_shared_ptr_vector
            in_functions)
    {
        this_type::behavior_chunk::add(
            this->behaviors_, in_key, std::move(in_functions));
    }

    public: bool remove_behavior_chunk(
        typename this_type::behavior_chunk::key_type const& in_key)
    {
        return this_type::behavior_chunk::remove(this->behaviors_, in_key);
    }

    //-------------------------------------------------------------------------
    /// @brief シナリオ駆動器で用いる状態値書庫。
    private: typename this_type::state_archive state_archive_;

    /// @brief シナリオ駆動器で用いる条件評価器。
    public: typename this_type::evaluator evaluator_;

    /// @brief シナリオ駆動器で用いる条件監視器。
    public: typename this_type::dispatcher dispatcher_;

    /// @brief シナリオ駆動器で用いる条件挙動チャンクのコンテナ。
    private: typename this_type::behavior_chunk::vector behaviors_;

    /// @brief シナリオ駆動器で用いるハッシュ関数オブジェクト。
    private: typename this_type::hasher hasher_;

}; // class psyq::scenario_engine::driver

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq_test
{
    inline void scenario_engine()
    {
        psyq::scenario_engine::driver<> local_driver(4, 16, 16);
        local_driver.get_state_archive().is_registered(0);
    }
}
#endif // !defined(PSYQ_SCENARIO_ENGINE_DRIVER_HPP_)
// vim: set expandtab:
