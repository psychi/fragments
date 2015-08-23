/// @file
/// @brief @copybrief psyq::if_then_engine::_private::dispatcher
/// @author Hillco Psychi (https://twitter.com/psychi)
#ifndef PSYQ_IF_THEN_ENGINE_DISPATCHER_HPP_
#define PSYQ_IF_THEN_ENGINE_DISPATCHER_HPP_

#include <unordered_map>
#include "./status_monitor.hpp"
#include "./expression_monitor.hpp"
#include "./handler.hpp"

/// @brief 挙動関数の呼び出し優先順位のデフォルト値。
#ifndef PSYQ_IF_THEN_ENGINE_DISPATCHER_FUNCTION_PRIORITY_DEFAULT
#define PSYQ_IF_THEN_ENGINE_DISPATCHER_FUNCTION_PRIORITY_DEFAULT 0
#endif // !defined(PSYQ_IF_THEN_ENGINE_DISPATCHER_FUNCTION_PRIORITY_DEFAULT)

/// @cond
namespace psyq
{
    namespace if_then_engine
    {
        namespace _private
        {
            template<typename, typename> class dispatcher;
        } // namespace _private
    } // namespace if_then_engine
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief 条件挙動器。条件式の評価が条件と合致すると、関数を呼び出す。
/// @par 使い方の概略
/// - dispatcher::register_handler
///   で、条件式の評価の変化条件に合致した際に呼び出す関数を登録する。
/// - dispatcher::_dispatch
///   で状態値の変化を検知して、変化した状態値を参照する条件式を評価し、
///   変化条件に合致した関数を呼び出す。
/// @tparam template_evaluator @copydoc dispatcher::evaluator
/// @tparam template_priority  @copydoc dispatcher::handler::priority
template<typename template_evaluator, typename template_priority>
class psyq::if_then_engine::_private::dispatcher
{
    /// @brief this が指す値の型。
    private: typedef dispatcher this_type;

    //-------------------------------------------------------------------------
    /// @brief dispather で使う条件評価器の型。
    /// @details psyq::if_then_engine::_private::evaluator と互換性があること。
    public: typedef template_evaluator evaluator;
    /// @brief コンテナに用いるメモリ割当子の型。
    public:
        typedef typename this_type::evaluator::allocator_type
        allocator_type;
    /// @brief dispather で使う条件挙動ハンドラの型。
    public: typedef 
         psyq::if_then_engine::_private::handler<
             typename this_type::evaluator::expression_key,
             typename this_type::evaluator::evaluation,
             typename template_priority>
         handler;

    //-------------------------------------------------------------------------
    /// @copydoc this_type::status_monitors_
    private: typedef
         std::unordered_map<
             typename this_type::evaluator::reservoir::status_key,
             psyq::if_then_engine::_private::status_monitor<
                 std::vector<
                     typename this_type::evaluator::expression_key,
                     typename this_type::allocator_type> >,
             psyq::integer_hash<
                 typename this_type::evaluator::reservoir::status_key>,
             std::equal_to<
                 typename this_type::evaluator::reservoir::status_key>,
             typename this_type::allocator_type>
         status_monitor_map;
    /// @copydoc this_type::expression_monitors_
    private: typedef
         std::unordered_map<
             typename this_type::evaluator::expression_key,
             psyq::if_then_engine::_private::expression_monitor<
                std::vector<
                    typename this_type::handler,
                    typename this_type::allocator_type> >,
             psyq::integer_hash<typename this_type::evaluator::expression_key>,
             std::equal_to<typename this_type::evaluator::expression_key>,
             typename this_type::allocator_type>
         expression_monitor_map;
    /// @copydoc this_type::cached_handlers_
    private: typedef
        std::vector<
            typename this_type::handler::cache,
            typename this_type::allocator_type>
        handler_cache_container;

    //-------------------------------------------------------------------------
    /// @name 構築と代入
    /// @{

    /// @brief 空の条件挙動器を構築する。
    public: dispatcher(
        /// [in] 監視する状態値のバケット数。
        std::size_t const in_status_count,
        /// [in] 監視する条件式のバケット数。
        std::size_t const in_expression_count,
        /// [in] 条件挙動キャッシュの予約数。
        std::size_t const in_cache_capacity,
        /// [in] メモリ割当子の初期値。
        typename this_type::allocator_type const& in_allocator):
    status_monitors_(
        in_status_count,
        typename this_type::status_monitor_map::hasher(),
        typename this_type::status_monitor_map::key_equal(),
        in_allocator),
    expression_monitors_(
        in_expression_count,
        typename this_type::expression_monitor_map::hasher(),
        typename this_type::expression_monitor_map::key_equal(),
        in_allocator),
    cached_handlers_(in_allocator),
    dispatch_lock_(false)
    {
        this->cached_handlers_.reserve(in_cache_capacity);
    }

    /// @brief コピー構築子。
    public: dispatcher(
        /// [in] コピー元となるインスタンス。
        this_type const& in_source):
    status_monitors_(in_source.status_monitors_),
    expression_monitors_(in_source.expression_monitors_),
    cached_handlers_(in_source.cached_handlers_.get_allocator()),
    dispatch_lock_(false)
    {
        this->cached_handlers_.reserve(in_source.cached_handlers_.capacity());
    }

    /// @brief ムーブ構築子。
    public: dispatcher(
        /// [in,out] ムーブ元となるインスタンス。
        this_type&& io_source):
    status_monitors_((
        /// @warning this_type::_dispatch 実行中はムーブできない。
        PSYQ_ASSERT(!io_source.dispatch_lock_),
        std::move(io_source.status_monitors_))),
    expression_monitors_(std::move(io_source.expression_monitors_)),
    cached_handlers_(std::move(io_source.cached_handlers_)),
    dispatch_lock_(false)
    {}

    /// @brief コピー代入演算子。
    /// @return *this
    public: this_type& operator=(
        /// [in] コピー元となるインスタンス。
        this_type const& in_source)
    {
        /// @warning this_type::_dispatch 実行中はムーブできない。
        PSYQ_ASSERT(!this->dispatch_lock_ && !in_source.dispatch_lock_);
        this->status_monitors_ = in_source.status_monitors_;
        this->expression_monitors_ = in_source.expression_monitors_;
        this->cached_handlers_.reserve(io_source.cached_handlers_.capacity());
        return *this;
    }

    /// @brief ムーブ代入演算子。
    /// @return *this
    public: this_type& operator=(
        /// [in,out] ムーブ元となるインスタンス。
        this_type&& io_source)
    {
        /// @warning this_type::_dispatch 実行中はムーブできない。
        PSYQ_ASSERT(!this->dispatch_lock_ && !io_source.dispatch_lock_);
        this->status_monitors_ = std::move(io_source.status_monitors_);
        this->expression_monitors_ = std::move(io_source.expression_monitors_);
        this->cached_handlers_ = std::move(io_source.cached_handlers_);
        return *this;
    }

    /// @brief 条件挙動器を解体する。
    public: ~dispatcher()
    {
        /// @warning this_type::_dispatch 実行中は解体できない。
        PSYQ_ASSERT(!this->dispatch_lock_);
    }

    /// @brief 条件挙動器で使われているメモリ割当子を取得する。
    /// @return 条件挙動器で使われているメモリ割当子。
    public: typename this_type::allocator_type get_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->expression_monitors_.get_allocator();
    }

    /// @brief 条件挙動器を再構築し、メモリ領域を必要最小限にする。
    public: void rebuild(
        /// [in] 監視する状態値のバケット数。
        std::size_t const in_status_count,
        /// [in] 監視する条件式のバケット数。
        std::size_t const in_expression_count,
        /// [in] 条件挙動キャッシュの予約数。
        std::size_t const in_cache_capacity)
    {
        this_type::rebuild_monitors(
            this->status_monitors_,
            in_status_count,
            [this](
                typename this_type::status_monitor_map::mapped_type&
                    io_status_monitor)
            ->bool
            {
                return io_status_monitor.shrink_expression_keys(
                    this->expression_monitors_);
            });
        this_type::rebuild_monitors(
            this->expression_monitors_,
            in_expression_count,
            [](
                typename this_type::expression_monitor_map::mapped_type&
                    io_expression_monitor)
            ->bool
            {
                return io_expression_monitor.shrink_handlers();
            });
        this->cached_handlers_ = decltype(this->cached_handlers_)(
            this->cached_handlers_.get_allocator());
        this->cached_handlers_.reserve(in_cache_capacity);
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name 条件挙動
    /// @{

    /// @brief 条件式に対応する条件挙動ハンドラを登録する。
    /// @details
    /// this_type::_dispatch で条件が合致した際に呼び出す関数を登録する。
    /// 登録された関数は、スマートポインタが空になると、自動的に取り除かれる。
    /// 明示的に関数を取り除くには、 this_type::unregister_handler を呼び出す。
    /// @return
    /// 登録した条件挙動ハンドラを指すポインタ。失敗した場合は nullptr を返す。
    /// - in_function が空だと失敗する。
    /// - in_function と同じ関数が同じ条件式に既に登録されていると失敗する。
    public: typename this_type::handler const* register_handler(
        /// [in] 評価に使う条件式の識別値。
        typename this_type::evaluator::expression_key const& in_expression_key,
        /// [in] 関数を呼び出す条件となる、条件式の評価の変化。
        /// this_type::handler::make_condition から作る。
        typename this_type::handler::condition const in_condition,
        /// [in] 登録する関数を指すスマートポインタ。
        typename this_type::handler::function_shared_ptr const& in_function,
        /// [in] 関数の呼び出し優先順位。優先順位の昇順に呼び出される。
        typename this_type::handler::priority const in_priority =
            PSYQ_IF_THEN_ENGINE_DISPATCHER_FUNCTION_PRIORITY_DEFAULT)
    {
        return this_type::expression_monitor_map::mapped_type::register_handler(
            this->expression_monitors_,
            in_expression_key,
            in_condition,
            in_function,
            in_priority);
    }

    /// @brief 条件式に対応する条件挙動を取り除く。
    /// @details
    /// this_type::register_handler
    /// で登録した条件挙動ハンドラを、条件式監視器から取り除く。
    /// @retval true  条件挙動ハンドラを取り除いた。
    /// @retval false 該当する条件挙動ハンドラが見つからなかった。
    public: bool unregister_handler(
        /// [in] 取り除く条件挙動に対応する条件式の識別値。
        typename this_type::evaluator::expression_key const& in_expression_key,
        /// [in] 取り除く条件挙動に対応する挙動関数。
        typename this_type::handler::function const& in_function)
    {
        auto const local_find(this->expression_monitors_.find(in_expression_key));
        return local_find != this->expression_monitors_.end() 
            && local_find->second.unregister_handler(in_function);
    }

    /// @brief 条件式に対応する条件挙動ハンドラをすべて取り除く。
    /// @retval true  in_expression_key に対応する条件挙動ハンドラを取り除いた。
    /// @retval false 該当する条件挙動がなかった。
    public: bool unregister_handler(
        /// [in] 取り除く条件挙動ハンドラに対応する条件式の識別値。
        typename this_type::evaluator::expression_key const& in_expression_key)
    {
        return 0 < this->expression_monitors_.erase(in_expression_key);
    }

    /// @brief 関数に対応する条件挙動ハンドラをすべて取り除く。
    /// @return 取り除いた条件挙動ハンドラの数。
    public: std::size_t unregister_handler(
        /// [in] 取り除く条件挙動ハンドラに対応する挙動関数。
        typename this_type::handler::function const& in_function)
    {
        std::size_t local_count(0);
        for (auto& local_expression_monitor: this->expression_monitors_)
        {
            if (local_expression_monitor.second.unregister_handler(in_function))
            {
                ++local_count;
            }
        }
        return local_count;
    }

    /// @brief 条件挙動ハンドラを取得する。
    /// @return
    /// this_type::register_handler で登録した条件挙動ハンドラを指すポインタ。
    /// 該当する条件挙動ハンドラがない場合は nullptr を返す。
    public: typename this_type::handler const* find_handler(
        /// [in] 取得する条件挙動ハンドラに対応する条件式の識別値。
        typename this_type::evaluator::expression_key const& in_expression_key,
        /// [in] 取得する条件挙動ハンドラに対応する関数。
        typename this_type::handler::function const* const in_function)
    const
    {
        if (in_function != nullptr)
        {
            auto const local_find(
                this->expression_monitors_.find(in_expression_key));
            if (local_find != this->expression_monitors_.end())
            {
                return local_find->second.find_handler(*in_function);
            }
        }
        return nullptr;
    }

    /// @brief psyq::if_then_engine 管理者以外は、この関数は使用禁止。
    /// @details
    /// 前回の this_type::_dispatch と今回の this_type::_dispatch
    /// で条件式の評価が異なっている場合に、 this_type::register_handler
    /// で指定した条件と合致していれば、関数を呼び出す。
    /// @warning
    /// 前回から今回の間（基本的には1フレームの間）で条件式の評価が変化しても、
    /// 前回の時点と今回の時点の評価が同じ場合は、関数が呼び出されない。
    /// たとえば、前回から今回の間で条件式の評価が
    /// 「true（前回）／false（前回と今回の間）／true（今回）」
    /// と変化した場合、関数は呼び出されない。
    public: void _dispatch(
        /// [in,out] 条件式の評価に使う状態貯蔵器。
        typename this_type::evaluator::reservoir& io_reservoir,
        /// [in] 条件式の評価に使う条件評価器。
        typename this_type::evaluator const& in_evaluator)
    {
        // _dispatch を多重に実行しないようにロックする。
        if (this->dispatch_lock_)
        {
            PSYQ_ASSERT(false);
            return;
        }
        this->dispatch_lock_ = true;

        // 条件式を状態監視器へ登録する。
        this_type::expression_monitor_map::mapped_type::register_expressions(
            this->status_monitors_, this->expression_monitors_, in_evaluator);

        // 状態値の変化を検知し、条件式監視器へ知らせる。
        this_type::status_monitor_map::mapped_type::notify_status_transitions(
            this->status_monitors_, this->expression_monitors_, io_reservoir);

        // 変化した状態値を参照する条件式を評価し、条件挙動をキャッシュに貯める。
        auto local_cached_handlers(std::move(this->cached_handlers_));
        this->cached_handlers_.clear();
        local_cached_handlers.clear();
        this_type::expression_monitor_map::mapped_type::cache_handlers(
            local_cached_handlers,
            this->expression_monitors_,
            io_reservoir,
            in_evaluator);

        // 条件式の評価が済んだので、状態変化フラグを初期化する。
        io_reservoir._reset_transitions();

        // キャッシュに貯まった関数を呼び出す。
        for (auto const& local_cached_handler: local_cached_handlers)
        {
            local_cached_handler.call_function();
        }

        // 挙動挙動キャッシュを片づける。
        PSYQ_ASSERT(this->cached_handlers_.empty());
        if (this->cached_handlers_.capacity() <= 0)
        {
            local_cached_handlers.clear();
            this->cached_handlers_ = std::move(local_cached_handlers);
        }
        PSYQ_ASSERT(this->dispatch_lock_);
        this->dispatch_lock_ = false;
    }
    /// @}

    //-------------------------------------------------------------------------
    /// @brief 監視器を再構築する。
    private: template<
         typename template_monitor_map, typename template_rebuild_function>
    static void rebuild_monitors(
        /// [in,out] 再構築する監視器の辞書。
        template_monitor_map& io_monitors,
        /// [in] 辞書のバケット数。
        std::size_t const in_bucket_count,
        /// [in] 監視器を再構築する関数。
        template_rebuild_function const& in_rebuild)
    {
        for (auto i(io_monitors.begin()); i != io_monitors.end();)
        {
            if (in_rebuild(i->second))
            {
                i = io_monitors.erase(i);
            }
            else
            {
                ++i;
            }
        }
        io_monitors.rehash(in_bucket_count);
    }

    //-------------------------------------------------------------------------
    /// @brief status_monitor の辞書。
    private: typename this_type::status_monitor_map status_monitors_;
    /// @brief expression_monitor の辞書。
    private: typename this_type::expression_monitor_map expression_monitors_;
    /// @brief this_type::handler::cache のコンテナ。
    private: typename this_type::handler_cache_container cached_handlers_;
    /// @brief 多重に this_type::_dispatch しないためのロック。
    private: bool dispatch_lock_;

}; // class psyq::if_then_engine::_private::dispatcher

#endif // !defined(PSYQ_IF_THEN_ENGINE_DISPATCHER_HPP_)
// vim: set expandtab:
