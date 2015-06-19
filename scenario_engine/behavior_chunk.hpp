/** @file
    @brief @copybrief psyq::scenario_engine::_private::behavior_chunk
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_SCENARIO_ENGINE_BEHAVIOR_CHUNK_HPP_
#define PSYQ_SCENARIO_ENGINE_BEHAVIOR_CHUNK_HPP_

//#include "scenario_engine/dispatcher.hpp"

/// @cond
namespace psyq
{
    namespace scenario_engine
    {
        namespace _private
        {
            template<typename> class behavior_chunk;
        } // namespace _private
    } // namespace scenario_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 条件挙動チャンク。
           条件式の評価が変化した際に呼び出す条件挙動関数を所有する。
 */
template<typename template_dispatcher>
class psyq::scenario_engine::_private::behavior_chunk
{
    /// @brief thisが指す値の型。
    private: typedef behavior_chunk this_type;

    /// @brief 条件挙動関数を登録する条件挙動器を表す型。
    public: typedef template_dispatcher dispatcher;

    /// @brief 条件挙動チャンクのコンテナを表す型。
    public: typedef
        std::vector<this_type, typename this_type::dispatcher::allocator_type>
        container;

    /// @brief 条件挙動チャンクの識別値を表す型。
    /// @note ここは条件式キーでなくて、チャンクキーにしないと。
    public: typedef
        typename this_type::dispatcher::evaluator::reservoir::chunk_key
        key;

    /// @brief 条件挙動チャンクの識別値を比較する関数オブジェクト。
    public: typedef
         psyq::scenario_engine::_private::key_less<
             psyq::scenario_engine::_private::object_key_getter<
                 this_type, typename this_type::key>>
         key_less;

    /// @brief 条件挙動関数オブジェクトの所有権ありスマートポインタのコンテナを表す型。
    private: typedef
        std::vector<
            typename template_dispatcher::function_shared_ptr,
            typename template_dispatcher::allocator_type>
        function_shared_ptr_container;

    //-------------------------------------------------------------------------
    /// @name 構築と代入
    //@{
    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元となるインスタンス。
     */
    public: behavior_chunk(this_type&& io_source):
    functions_(std::move(io_source.functions_)),
    key_(std::move(io_source.key_))
    {}

    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元となるインスタンス。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source)
    {
        this->functions_ = std::move(io_source.functions_);
        this->key_ = std::move(io_source.key_);
        return *this;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 条件挙動関数
    //@{
    /** @brief 条件挙動チャンクに条件挙動関数を追加する。
        @param[in,out] io_chunks
            条件挙動関数を追加する条件挙動チャンクのコンテナ。
        @param[in] in_key
            条件挙動関数を追加する条件挙動チャンクの識別値。
        @param[in] in_function
            条件挙動チャンクに追加する条件挙動関数を指すスマートポインタ。
        @retval true  成功。条件挙動関数を追加した。
        @retval false 失敗。条件挙動関数を追加しなかった。
     */
    public: static bool extend(
        typename this_type::container& io_chunks,
        typename this_type::key const& in_key,
        typename this_type::dispatcher::function_shared_ptr in_function)
    {
        if (in_function.get() == nullptr)
        {
            return false;
        }

        // 条件挙動関数を条件挙動チャンクに追加する。
        this_type::equip(io_chunks, in_key).functions_.push_back(
            std::move(in_function));
        return true;
    }

    /** @brief 条件挙動チャンクに条件挙動関数を追加する。
        @param[in,out] io_chunks
            条件挙動関数を追加する条件挙動チャンクのコンテナ。
        @param[in] in_key
            条件挙動関数を追加する条件挙動チャンクの識別値。
        @param[in] in_functions
            条件挙動チャンクに追加する条件挙動関数の、
            スマートポインタのコンテナ。
        @return 追加した条件挙動関数の数。
     */
    public: template<typename template_function_container>
    static std::size_t extend(
        typename this_type::container& io_chunks,
        typename this_type::key const& in_key,
        template_function_container in_functions)
    {
        // 条件挙動関数を条件挙動チャンクに追加する。
        auto& local_chunk_functions(
            this_type::equip(io_chunks, in_key).functions_);
        local_chunk_functions.reserve(
            local_chunk_functions.size() + in_functions.size());
        std::size_t local_count(0);
        for (auto& local_function: in_functions)
        {
            if (local_function.get() != nullptr)
            {
                local_chunk_functions.push_back(std::move(local_function));
                ++local_count;
            }
        }
        return local_count;
    }

    /** @brief コンテナから条件挙動チャンクを削除する。
        @param[in,out] io_chunks 条件挙動チャンクを削除するコンテナ。
        @param[in] in_key        削除する条件挙動チャンクの識別値。
        @retval true  in_key に対応する条件挙動チャンクを削除した。
        @retval false in_key に対応する条件挙動チャンクがコンテナになかった。
     */
    public: static bool erase(
        typename this_type::container& io_chunks,
        typename this_type::key const& in_key)
    {
        auto const local_iterator(
            this_type::key_less::find_const_iterator(io_chunks, in_key));
        auto const local_find(local_iterator != io_chunks.end());
        if (local_find)
        {
            io_chunks.erase(local_iterator);
        }
        return local_find;
    }

    /** @brief 条件挙動チャンクを用意する。
        @param[in,out] io_chunks 条件挙動チャンクのコンテナ。
        @param[in] in_key        用意する条件挙動チャンクの識別値。
        @return 用意した条件挙動チャンク。
     */
    private: static this_type& equip(
        typename this_type::container& io_chunks,
        typename this_type::key const& in_key)
    {
        // 条件挙動関数を追加する条件挙動チャンクを用意する。
        auto const local_iterator(
            this_type::key_less::find_iterator(io_chunks, in_key));
        return local_iterator != io_chunks.end()?
            *local_iterator:
            *io_chunks.insert(
                local_iterator, this_type(in_key, io_chunks.get_allocator()));
    }
    //@}
    //-------------------------------------------------------------------------
    /** @brief 空の条件挙動チャンクを構築する。
        @param[in] in_key       条件挙動チャンクの識別値。
        @param[in] in_allocator メモリ割当子の初期値。
     */
    private: behavior_chunk(
        typename this_type::key in_key,
        typename this_type::dispatcher::allocator_type const& in_allocator)
    :
    functions_(in_allocator),
    key_(std::move(in_key))
    {}

    //-------------------------------------------------------------------------
    /// @brief 条件挙動関数のコンテナ。
    public: typename this_type::function_shared_ptr_container functions_;
    /// @brief 条件挙動チャンクの識別値。
    public: typename this_type::key key_;

}; // class psyq::scenario_engine::behavior_chunk

#endif // !defined(PSYQ_SCENARIO_ENGINE_BEHAVIOR_CHUNK_HPP_)
// vim: set expandtab:
