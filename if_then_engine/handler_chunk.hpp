/// @file
/// @brief @copybrief psyq::if_then_engine::_private::handler_chunk
/// @author Hillco Psychi (https://twitter.com/psychi)
#ifndef PSYQ_IF_THEN_ENGINE_HANDLER_CHUNK_HPP_
#define PSYQ_IF_THEN_ENGINE_HANDLER_CHUNK_HPP_

/// @cond
namespace psyq
{
    namespace if_then_engine
    {
        namespace _private
        {
            template<typename> class handler_chunk;
        } // namespace _private
    } // namespace if_then_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 条件挙動チャンク。条件式の評価が変化した際に呼び出す関数を保持する。
/// @tparam handler_chunk::dispatcher
template<typename template_dispatcher>
class psyq::if_then_engine::_private::handler_chunk
{
    /// @brief this が指す値の型。
    private: typedef handler_chunk this_type;

    //-------------------------------------------------------------------------
    /// @brief _private::dispatcher を表す型。
    public: typedef template_dispatcher dispatcher;
    /// @brief 条件挙動チャンクのコンテナを表す型。
    public: typedef
        std::vector<this_type, typename this_type::dispatcher::allocator_type>
        container;
    /// @brief 条件挙動チャンクの識別値を表す型。
    public: typedef
        typename this_type::dispatcher::evaluator::reservoir::chunk_key
        key;

    //-------------------------------------------------------------------------
    /// @copydoc functions_
    private: typedef
        std::vector<
            typename template_dispatcher::handler::function_shared_ptr,
            typename template_dispatcher::allocator_type>
        function_shared_ptr_container;
    /// @brief 条件挙動チャンクの識別値を比較する関数オブジェクト。
    private: struct key_less
    {
        bool operator()(
            handler_chunk const& in_left,
            handler_chunk const& in_right)
        const PSYQ_NOEXCEPT
        {
            return in_left.key_ < in_right.key_;
        }

        bool operator()(
            handler_chunk const& in_left,
            typename handler_chunk::key const& in_right)
        const PSYQ_NOEXCEPT
        {
            return in_left.key_ < in_right;
        }

        bool operator()(
            typename handler_chunk::key const& in_left,
            handler_chunk const& in_right)
        const PSYQ_NOEXCEPT
        {
            return in_left < in_right.key_;
        }

    }; // struct priority_less

    //-------------------------------------------------------------------------
#ifdef PSYQ_NO_STD_DEFAULTED_FUNCTION
    /// @brief ムーブ構築子。
    public: handler_chunk(
        /// [in,out] ムーブ元となるインスタンス。
        this_type&& io_source):
    functions_(std::move(io_source.functions_)),
    key_(std::move(io_source.key_))
    {}

    /** @brief ムーブ代入演算子。
        @return *this
     */
    public: this_type& operator=(
        /// [in,out] ムーブ元となるインスタンス。
        this_type&& io_source)
    {
        this->functions_ = std::move(io_source.functions_);
        this->key_ = std::move(io_source.key_);
        return *this;
    }
#endif // defined(PSYQ_NO_STD_DEFAULTED_FUNCTION)

    /// @brief 条件挙動関数のコンテナを整理する。
    public: void shrink_to_fit()
    {
        this->functions_.shrink_to_fit();
    }

    //-------------------------------------------------------------------------
    /// @brief 条件挙動チャンクに handler::function を追加する。
    /// @retval true  成功。 handler::function を追加した。
    /// @retval false 失敗。 handler::function を追加しなかった。
    public: static bool extend(
        /// [in,out] handler::function を追加する条件挙動チャンクのコンテナ。
        typename this_type::container& io_chunks,
        /// [in] handler::function を追加する条件挙動チャンクの識別値。
        typename this_type::key const& in_key,
        /// [in] 条件挙動チャンクに追加する handler::function_shared_ptr 。
        typename this_type::dispatcher::handler::function_shared_ptr in_function)
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

    /// @brief 条件挙動チャンクに handler::function を追加する。
    /// @return 追加した handler::function の数。
    public: template<typename template_function_container>
    static std::size_t extend(
        /// [in,out] handler::function を追加する条件挙動チャンクのコンテナ。
        typename this_type::container& io_chunks,
        /// [in] handler::function を追加する条件挙動チャンクの識別値。
        typename this_type::key const& in_key,
        /// [in] 条件挙動チャンクに追加する
        /// handler::function_shared_ptr のコンテナ。
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

    /// @brief コンテナから条件挙動チャンクを削除する。
    /// @retval true  in_key に対応する条件挙動チャンクを削除した。
    /// @retval false in_key に対応する条件挙動チャンクがコンテナになかった。
    public: static bool erase(
        /// [in,out] 条件挙動チャンクを削除するコンテナ。
        typename this_type::container& io_chunks,
        /// [in] 削除する条件挙動チャンクの識別値。
        typename this_type::key const& in_key)
    {
        auto const local_lower_bound(
            std::lower_bound(
                io_chunks.begin(),
                io_chunks.end(),
                in_key,
                typename this_type::key_less()));
        if (local_lower_bound == io_chunks.end()
            || local_lower_bound->key_ != in_key)
        {
            return false;
        }
        io_chunks.erase(local_lower_bound);
        return true;
    }

    //-------------------------------------------------------------------------
    /// @brief 空の条件挙動チャンクを構築する。
    private: handler_chunk(
        /// [in] 条件挙動チャンクの識別値。
        typename this_type::key const& in_key,
        /// [in] メモリ割当子の初期値。
        typename this_type::dispatcher::allocator_type const& in_allocator):
    functions_(in_allocator),
    key_(in_key)
    {}

    /// @brief 条件挙動チャンクを用意する。
    /// @return 用意した条件挙動チャンク。
    private: static this_type& equip(
        /// [in,out] 条件挙動チャンクのコンテナ。
        typename this_type::container& io_chunks,
        /// [in] 用意する条件挙動チャンクの識別値。
        typename this_type::key const& in_key)
    {
        // 条件挙動関数を追加する条件挙動チャンクを用意する。
        auto const local_lower_bound(
            std::lower_bound(
                io_chunks.begin(),
                io_chunks.end(),
                in_key,
                typename this_type::key_less()));
        return local_lower_bound != io_chunks.end()
        && local_lower_bound->key_ == in_key?
            *local_lower_bound:
            *io_chunks.insert(
                local_lower_bound,
                this_type(in_key, io_chunks.get_allocator()));
    }

    //-------------------------------------------------------------------------
    /// @brief handler::function_shared_ptr のコンテナ。
    private: typename this_type::function_shared_ptr_container functions_;
    /// @brief 条件挙動チャンクの識別値。
    private: typename this_type::key key_;

}; // class psyq::if_then_engine::handler_chunk

#endif // !defined(PSYQ_IF_THEN_ENGINE_HANDLER_CHUNK_HPP_)
// vim: set expandtab:
