/** @file
    @brief @copydoc psyq::scenario_engine::driver
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_SCENARIO_ENGINE_DRIVER_HPP_
#define PSYQ_SCENARIO_ENGINE_DRIVER_HPP_

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
template<
    typename template_hasher = psyq::string::view<char>::fnv1_hash32,
    typename template_allocator = std::allocator<void*>>
class psyq::scenario_engine::driver
{
    private: typedef driver this_type;

    /** @brief 文字列からハッシュ値を取得する、関数オブジェクトの型。

        std::hash 互換インターフェイスを持つこと。
     */
    public: typedef template_hasher hasher;

    public: typedef template_allocator allocator_type;

    /// シナリオ駆動器で用いる状態値書庫。
    public: typedef psyq::scenario_engine::state_archive<
        typename this_type::hasher::result_type,
        typename this_type::allocator_type>
            state_archive;

    //-------------------------------------------------------------------------
    /** @brief シナリオ駆動器で用いるハッシュ関数オブジェクトを取得する。
        @return シナリオ駆動器で用いるハッシュ関数オブジェクト。
     */
    public: typename this_type::hasher hash_function() const
    {
        return this->hasher_;
    }

    public: typename this_type::key_type make_hash(
        typename this_type::hasher::argument_type const& in_key)
    {
        auto const local_hash(this->hasher_(in_key));
        PSYQ_ASSERT(
            in_key.empty() || local_hash != this->hasher_(
                typename this_type::hasher::argument_type()));
        return local_hash;
    }

    //-------------------------------------------------------------------------
    /// @brief シナリオ駆動器で用いるハッシュ関数オブジェクト。
    private: typename this_type::hasher hasher_;

    /// @brief シナリオ駆動器で用いる状態値書庫。
    private: typename this_type::state_archive state_archive_;

    /// @brief シナリオ駆動器で用いる条件評価器。
    private: psyq::scenario_engine::evaluator evaluator_;

}; // class psyq::scenario_engine::driver

#endif // !defined(PSYQ_SCENARIO_ENGINE_DRIVER_HPP_)
