/// @file
/// @author Hillco Psychi (https://twitter.com/psychi)
/// @namespace psyq::event_driven
/// @brief 任意の引数を持つメッセージを送信できる、イベント駆動フレームワーク。
/// @par 初期化の手順
///   -# psyq::event_driven::dispatcher::zone インスタンスを用意する。
///   -# psyq::event_driven::dispatcher::zone::equip_dispatcher で、スレッドごとに
///      psyq::event_driven::dispatcher インスタンスを用意する。
///   -# 現在のスレッドに適合する psyq::event_driven::dispatcher インスタンスから
///      psyq::event_driven::dispatcher::register_receiving_function
///      を呼び出し、メッセージ受信関数を登録する。
/// @par メッセージ送受信の手順
///   -# メッセージループをまわしておく。
///      - psyq::event_driven::dispatcher::zone::dispatch
///        を、メインスレッドから定期的に呼び出す。
///      - それぞれのスレッドに適合する psyq::event_driven::dispatcher
///        インスタンスから psyq::event_driven::dispatcher::dispatch を定期的に呼び出す。
///   -# 現在のスレッドに適合する psyq::event_driven::dispatcher
///      インスタンスから以下のいずれかの関数を呼び出し、メッセージを送信する。
///      - 構築済みのメッセージパケットを送るなら、
///        psyq::event_driven::dispatcher::post を呼び出す。
///      - メッセージゾーンの内と外にメッセージを送るなら、
///        psyq::event_driven::dispatcher::post_external を呼び出す。
///      - メッセージゾーンの内にのみメッセージを送るなら、
///        psyq::event_driven::dispatcher::post_zonal を呼び出す。
///      - 現在のスレッドにのみメッセージを送るなら、
///        psyq::event_driven::dispatcher::send_local を呼び出す。
///   -# psyq::event_driven::dispatcher::register_receiving_function
///      で登録したメッセージ受信関数が呼び出される。
#ifndef PSYQ_EVENT_DRIVEN_DISPATCHER_HPP_
#define PSYQ_EVENT_DRIVEN_DISPATCHER_HPP_

#ifdef _MSC_VER
#include <eh.h>
#endif // defined(_MSC_VER)
#include <vector>
#include <thread>
#include <mutex>
#include "./message.hpp"
#include "./packet.hpp"

/// @cond
namespace psyq
{
    namespace event_driven
    {
        template<typename, typename, typename> class dispatcher;
    } // namespace event_driven
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief スレッド別のメッセージ配送器。
/// @par 使い方の概略
///   - zone::equip_dispatcher で、 dispatcher を用意する。
///   - dispatcher::register_receiving_function で、メッセージ受信関数を登録する。
///   - dispatcher::post_external や
///     dispatcher::post_zonal で、メッセージを送信する。
///   - zone::dispatch で、 dispatcher の持つメッセージパケットが集配される。
///   - dispatcher::dispatch で、
///     dispatcher が持つメッセージパケットをメッセージ受信関数へ配送する。
/// @tparam template_base_message @copydoc packet::message
/// @tparam template_priority     @copydoc dispatcher::priority
/// @tparam template_allocator    @copydoc dispatcher::allocator_type
template<
    typename template_base_message = psyq::event_driven::message<
        std::uint32_t, std::uint32_t>,
    typename template_priority = std::int32_t,
    typename template_allocator = std::allocator<void*>>
class psyq::event_driven::dispatcher
{
    /// @copydoc psyq::string::view::this_type
    private: typedef dispatcher this_type;

    //-------------------------------------------------------------------------
    /// this_type を強参照するスマートポインタ。
    public: typedef std::shared_ptr<this_type> shared_ptr;
    /// this_type を弱参照するスマートポインタ。
    public: typedef std::weak_ptr<this_type> weak_ptr;
    /// @brief メッセージの優先順位。
    public: typedef template_priority priority;
    /// @brief this_type で使うメモリ割当子。
    public: typedef template_allocator allocator_type;

    //-------------------------------------------------------------------------
    public: class zone;
    /// @brief メッセージパケットの基底型。
    public: typedef psyq::event_driven::packet<template_base_message> packet;
    /// @brief メッセージの基底型。
    public: typedef typename this_type::packet::message message;
    /// @brief メッセージの送り状の型。
    public: typedef typename this_type::message::tag tag;

    //-------------------------------------------------------------------------
    /// @brief メッセージ受信関数。
    public: typedef
        std::function<void(typename this_type::packet const&)>
        function;
    /// @brief メッセージ受信関数を強参照するスマートポインタ。
    public: typedef
        std::shared_ptr<typename this_type::function>
        function_shared_ptr;
    /// @brief メッセージ受信関数を弱参照するスマートポインタ。
    public: typedef
        std::weak_ptr<typename this_type::function>
        function_weak_ptr;

    //-------------------------------------------------------------------------
    /// this_type::function_shared_ptr のコンテナ。
    private: typedef
        std::vector<
            typename this_type::function_shared_ptr,
            typename this_type::allocator_type>
        function_shared_ptr_container;
    /// this_type::packet::shared_ptr のコンテナ。
    private: typedef
        std::vector<
            typename this_type::packet::shared_ptr,
            typename this_type::allocator_type>
        packet_shared_ptr_container;

    //-------------------------------------------------------------------------
    /// @brief メッセージ受信フック。
    private: class receiving_hook
    {
        private: typedef receiving_hook this_type;

        public: struct less
        {
            bool operator()(
                typename dispatcher::receiving_hook const& in_left,
                typename dispatcher::receiving_hook const& in_right)
            const PSYQ_NOEXCEPT
            {
                return in_left.selector_key_ != in_right.selector_key_?
                    in_left.selector_key_ < in_right.selector_key_:
                    in_left.priority_ < in_right.priority_;
            }

            bool operator()(
                typename dispatcher::receiving_hook const& in_left,
                typename dispatcher::tag::key_type const& in_right)
            const PSYQ_NOEXCEPT
            {
                return in_left.selector_key_ < in_right;
            }

            bool operator()(
                typename dispatcher::tag::key_type const& in_left,
                typename dispatcher::receiving_hook const& in_right)
            const PSYQ_NOEXCEPT
            {
                return in_left < in_right.selector_key_;
            }
        };

        public: typedef
            std::vector<this_type, typename dispatcher::allocator_type>
            container;

        public: receiving_hook(
            typename dispatcher::tag::key_type const in_receiver_key,
            typename dispatcher::tag::key_type const in_selector_key,
            typename dispatcher::priority const in_priority,
            typename dispatcher::function_weak_ptr in_function):
        function_(std::move(in_function)),
        receiver_key_(in_receiver_key),
        selector_key_(in_selector_key),
        priority_(in_priority)
        {}

#ifdef PSYQ_NO_STD_DEFAULTED_FUNCTION
        /** @brief ムーブ構築子。
            @param[in,out] io_source ムーブ元となるインスタンス。
         */
        public: receiving_hook(this_type&& io_source):
        function_(std::move(io_source.function_)),
        receiver_key_(std::move(io_source.receiver_key_)),
        selector_key_(std::move(io_source.selector_key_)),
        priority_(std::move(io_source.priority_))
        {}

        /** @brief ムーブ代入演算子。
            @param[in,out] io_source ムーブ元となるインスタンス。
            @return *this
         */
        public: this_type& operator=(this_type&& io_source)
        {
            this->function_ = std::move(io_source.function_);
            this->receiver_key_ = std::move(io_source.receiver_key_);
            this->selector_key_ = std::move(io_source.selector_key_);
            this->priority_ = std::move(io_source.priority_);
            return *this;
        }
#endif // !defined(PSYQ_NO_STD_DEFAULTED_FUNCTION)

        public: typename dispatcher::function_weak_ptr function_;
        public: typename dispatcher::tag::key_type receiver_key_;
        public: typename dispatcher::tag::key_type selector_key_;
        public: typename dispatcher::priority priority_;

    }; // class receiving_hook

    //-------------------------------------------------------------------------
    /// @brief メッセージ転送フック。
    private: class forwarding_hook
    {
        private: typedef forwarding_hook this_type;

        public: struct less
        {
            bool operator()(
                typename dispatcher::forwarding_hook const& in_left,
                typename dispatcher::forwarding_hook const& in_right)
            const PSYQ_NOEXCEPT
            {
                return in_left.receiver_key_ != in_right.selector_key_?
                    in_left.receiver_key_ < in_right.selector_key_:
                    in_left.priority_ < in_right.priority_;
            }

            bool operator()(
                typename dispatcher::forwarding_hook const& in_left,
                typename dispatcher::tag::key_type const& in_right)
            const PSYQ_NOEXCEPT
            {
                return in_left.receiver_key_ < in_right;
            }

            bool operator()(
                typename dispatcher::tag::key_type const& in_left,
                typename dispatcher::forwarding_hook const& in_right)
            const PSYQ_NOEXCEPT
            {
                return in_left < in_right.receiver_key_;
            }
        };

        public: typedef
            std::vector<this_type, typename dispatcher::allocator_type>
            container;

        public: typename dispatcher::function_weak_ptr function_;
        public: typename dispatcher::tag::key_type receiver_key_;
        public: typename dispatcher::priority priority_;

    }; // class forwarding_hook

    //-------------------------------------------------------------------------
    /// @brief *this が使うメモリ割当子を取得する。
    /// @return *this が使うメモリ割当子。
    public: typename this_type::allocator_type get_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->receiving_hooks_.get_allocator();
    }

    /// @brief *this に合致するスレッドの識別子を取得する。
    /// @return *this に合致するスレッドの識別子。
    public: std::thread::id get_thread_id() const PSYQ_NOEXCEPT
    {
        return this->thread_id_;
    }

    //-------------------------------------------------------------------------
    /// @name メッセージ受信関数
    /// @{

    /// @brief メッセージ受信関数を登録する。
    /// @details
    ///   - 登録に成功した後は、 in_receiver_key / in_selector_key
    ///     の組み合わせと合致するメッセージを受信するたび、
    ///     in_function が呼び出される。
    ///   - 登録した in_function は弱参照しているだけで、
    ///     this_type は所有権を持たない。
    ///     in_function の所有権は、ユーザーが管理すること。
    ///   - in_function を強参照するスマートポインタがなくなると、
    ///     this_type から自動で取り外される。手動で取り外す場合は、
    ///     this_type::unregister_receiving_function を呼び出す。
    /// .
    /// @retval true 成功。 in_function を登録した。
    /// @retval false
    ///     失敗。 in_function を登録しなかった。
    ///     - this_type::get_thread_id
    ///       と合致しないスレッドから呼び出すと、失敗する。
    ///     - in_receiver_key / in_selector_key の組み合わせが同じ
    ///       メッセージ受信関数がすでに追加されていると、失敗する。
    ///     - in_function が空だと、失敗する。
    public: bool register_receiving_function(
        /// [in] 登録するメッセージ受信関数に対応する、
        /// メッセージ受信オブジェクトの識別値。
        typename this_type::tag::key_type const in_receiver_key,
        /// [in] 登録するメッセージ受信関数の識別値。
        typename this_type::tag::key_type const in_selector_key,
        /// [in] メッセージを受信する優先順位。
        typename this_type::priority const in_priority,
        /// [in] 登録するメッセージ受信関数。
        typename this_type::function_shared_ptr const& in_function)
    {
        if (in_function.get() == nullptr
            || !static_cast<bool>(*in_function)
            || !this->verify_thread())
        {
            return false;
        }
        auto const local_end(this->receiving_hooks_.cend());
        auto local_lower_bound(
            std::lower_bound(
                this->receiving_hooks_.cbegin(),
                local_end,
                in_selector_key,
                typename this_type::receiving_hook::less()));
        for (auto i(local_lower_bound); i != local_end;)
        {
            auto& local_hook(*i);
            if (local_hook.selector_key_ != in_selector_key)
            {
                break;
            }
            if (local_hook.receiver_key_ == in_receiver_key
                && !local_hook.function_.expired())
            {
                // 等価なメッセージ受信フックが存在しているので失敗。
                return false;
            }
            ++i;
            if (local_hook.priority_ <= in_priority)
            {
                local_lower_bound = i;
            }
        }
        this->receiving_hooks_.emplace(
            local_lower_bound,
            in_receiver_key,
            in_selector_key,
            in_priority,
            in_function);
        return true;
    }

    /// @brief メッセージ受信関数を取り除く。
    /// @return
    ///   取り除いたメッセージ受信関数を指すスマートポインタ。
    ///   該当するメッセージ受信関数がない場合は、空となる。
    public: typename this_type::function_weak_ptr unregister_receiving_function(
        /// [in] 除去するメッセージ受信関数に対応する
        /// メッセージ受信オブジェクトの識別値。
        typename this_type::tag::key_type const in_receiver_key,
        /// [in] 除去するメッセージ受信関数の識別値。
        typename this_type::tag::key_type const in_selector_key)
    {
        if (this->verify_thread())
        {
            auto const local_end(std::end(this->receiving_hooks_));
            auto const local_find(
                this_type::find_receiving_hook_iterator(
                    std::begin(this->receiving_hooks_),
                    local_end,
                    in_receiver_key,
                    in_selector_key));
            if (local_find != local_end)
            {
                auto const local_function(std::move(local_find->function_));
                local_find->function_.reset();
                return local_function;
            }
        }
        return typename this_type::function_weak_ptr();
    }

    /// @brief メッセージ受信関数を取り除く。
    /// @return 取り除いたメッセージ受信関数の数。
    public: std::size_t unregister_receiving_function(
        /// [in] 取り除くメッセージ受信関数に対応する
        /// メッセージ受信オブジェクトの識別値。
        typename this_type::tag::key_type const in_receiver_key)
    {
        std::size_t local_count(0);
        if (this->verify_thread())
        {
            for (auto& local_hook: this->receiving_hooks_)
            {
                if (local_hook.receiver_key_ == in_receiver_key 
                    && !local_hook.function_.expired())
                {
                    ++local_count;
                    local_hook.function_.reset();
                }
            }
        }
        return local_count;
    }

    /// @brief メッセージ受信関数を検索する。
    /// @return
    ///   検索したメッセージ受信関数を指すスマートポインタ。
    ///   該当するメッセージ受信関数が見つからなかった場合は、空となる。
    public: typename this_type::function_weak_ptr find_receiving_function(
        /// [in] 検索するメッセージ受信関数に対応する
        /// メッセージ受信オブジェクトの識別値。
        typename this_type::tag::key_type const in_receiver_key,
        /// [in] 検索するメッセージ受信関数の識別値。
        typename this_type::tag::key_type const in_selector_key)
    const PSYQ_NOEXCEPT
    {
        if (this->verify_thread())
        {
            auto const local_end(std::end(this->receiving_hooks_));
            auto const local_find(
                this_type::find_receiving_hook_iterator(
                    std::begin(this->receiving_hooks_),
                    local_end,
                    in_receiver_key,
                    in_selector_key));
            if (local_find != local_end)
            {
                return local_find->function_;
            }
        }
        return typename this_type::function_weak_ptr();
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @name メッセージ転送関数
    /// @{

    /// @brief メッセージ転送関数を登録する。
    /// @todo 未実装
    private: bool register_forwarging_function(
        /// [in] 登録するメッセージ転送関数に対応する、
        /// メッセージ受信オブジェクトの識別値。
        typename this_type::tag::key_type const in_receiver_key,
        /// [in] メッセージを転送する優先順位。
        typename this_type::priority const in_priority,
        /// [in] 登録するメッセージ転送関数。
        typename this_type::function_shared_ptr const& in_function);

    /// @brief メッセージ転送関数を取り除く。
    /// @todo 未実装
    private: typename this_type::function_weak_ptr unregister_forwarding_function(
        /// [in] 取り除くメッセージ転送関数に対応する、
        /// メッセージ受信オブジェクトの識別値。
        typename this_type::tag::key_type const in_receiver_key);

    /// @brief メッセージ転送関数を検索する。
    /// @todo 未実装
    private: typename this_type::function_weak_ptr find_forwarding_function(
        /// [in] 検索するメッセージ転送関数に対応する、
        /// メッセージ受信オブジェクトの識別値。
        typename this_type::tag::key_type const in_receiver_key);
    /// @}
    //-------------------------------------------------------------------------
    /// @name メッセージの送受信
    /// @{

    /// @brief メッセージゾーンの内と外へのメッセージの送信を予約する。
    /// @details
    ///   - 引数を持たないメッセージパケットを動的メモリ割当して構築し、
    ///     メッセージゾーンの内と外への送信を予約する。
    ///   - この関数では、メッセージ送信の予約のみを行う。
    ///     メッセージを実際に送信する処理は、この関数の呼び出し後、
    ///     zone::dispatch / this_type::dispatch の順に呼び出すことで行なわれる。
    ///   - メッセージ受信処理は、メッセージパケットの送信が行われた後、
    ///     zone::dispatch / this_type::dispatch の順に呼び出すことで行なわれる。
    ///   - 同一スレッドで送信を予約したメッセージの受信順序は、
    ///     送信予約順序と同じになる。
    /// .
    /// @sa
    ///   - 構築済のメッセージパケットを送信するには、 this_type::post を使う。
    ///   - メッセージゾーン内にだけメッセージを送信するには、
    ///     this_type::post_zonal を使う。
    ///   - *this に登録されているメッセージ受信関数にのみメッセージを送信するには、
    ///     this_type::send_local を使う。
    /// @retval true 成功。メッセージ送信を予約した。
    /// @retval false
    ///   失敗。メッセージ送信を予約しなかった。 this_type::get_thread_id
    ///   と合致しないスレッドからこの関数を呼び出すと、失敗する。
    /// @todo メッセージゾーンの外へ送信する処理は未実装。
    public: bool post_external(
        /// [in] 送信するメッセージの送り状。
        typename this_type::tag const& in_tag)
    {
        return this->post(
            this_type::packet::create_external(
                typename this_type::packet::message(in_tag),
                this->get_allocator()));
    }

    /// @copydoc this_type::post_external
    /// @todo メッセージゾーンの内へ送信する処理も未実装。
    public: template<typename template_parameter>
    bool post_external(
        /// [in] 送信するメッセージの送り状。
        typename this_type::tag const& in_tag,
        /// [in] 送信するメッセージの引数。必ずPOD型。
        template_parameter&& io_parameter);

    /// @brief メッセージゾーン内へのメッセージの送信を予約する。
    /// @details
    ///   - 引数を持たないメッセージパケットを動的メモリ割当して構築し、
    ///     メッセージゾーン内への送信を予約する。
    ///   - この関数では、メッセージ送信の予約のみを行う。
    ///     メッセージを実際に送信する処理は、この関数の呼び出し後、
    ///     zone::dispatch / this_type::dispatch の順に呼び出すことで行なわれる。
    ///   - メッセージ受信処理は、メッセージパケットの送信が行われた後、
    ///     zone::dispatch / this_type::dispatch の順に呼び出すことで行なわれる。
    ///   - 同一スレッドで送信を予約したメッセージの受信順序は、
    ///     送信予約順序と同じになる。
    /// .
    /// @sa
    ///   - 構築済のメッセージパケットを送信するには、 this_type::post を使う。
    ///   - メッセージゾーンの内と外にメッセージを送信するには、
    ///     this_type::post_external を使う。
    ///   - *this に登録されているメッセージ受信関数にのみメッセージを送信するには、
    ///     this_type::send_local を使う。
    /// @retval true 成功。メッセージ送信を予約した。
    /// @retval false
    ///   失敗。メッセージ送信を予約しなかった。 this_type::get_thread_id
    ///   と合致しないスレッドからこの関数を呼び出すと、失敗する。
    public: bool post_zonal(
        /// [in] 送信するメッセージの送り状。
        typename this_type::tag const& in_tag)
    {
        return this->post(
            this_type::packet::create_zonal(
                typename this_type::packet::message(in_tag),
                this->get_allocator()));
    }

    /// @copydoc this_type::post_zonal
    public: template<typename template_parameter>
    bool post_zonal(
        /// [in] 送信するメッセージの送り状。
        typename this_type::tag const& in_tag,
        /// [in] 送信するメッセージの引数。
        template_parameter&& io_parameter)
    {
        return this->post(
            this_type::packet::create_zonal(
                this_type::message::construct(in_tag, std::move(io_parameter)),
                this->get_allocator()));
    }

    /// @brief メッセージの送信を予約する。
    /// @details
    ///   - この関数では、メッセージ送信の予約のみを行う。
    ///     メッセージを実際に送信する処理は、この関数の呼び出し後、
    ///     zone::dispatch / this_type::dispatch の順に呼び出すことで行なわれる。
    ///   - メッセージ受信処理は、メッセージの送信が行われた後、
    ///     zone::dispatch / this_type::dispatch の順に呼び出すことで行なわれる。
    ///   - 同一スレッドで送信を予約したメッセージの受信順序は、
    ///     送信予約順序と同じになる。
    /// .
    /// @retval true  成功。メッセージの送信を予約した。
    /// @retval false 失敗。メッセージの送信を予約しなかった。
    public: bool post(
        /// [in] 送信するメッセージを持つメッセージパケット。
        typename this_type::packet::shared_ptr in_packet)
    {
        if (in_packet.get() == nullptr || !this->verify_thread())
        {
            return false;
        }
        this->export_packets_.emplace_back(std::move(in_packet));
        return true;
    }

    /// @brief メッセージパケットをメッセージ受信関数へ配送する。
    /// @details
    ///   この関数と zone::dispatch
    ///   をメッセージループで定期的に呼び出し、メッセージパケットを循環させること。
    /// @sa
    ///   *this にメッセージ受信関数を登録するには、
    ///   this_type::register_receiving_function を使う。
    /// @retval true 成功。メッセージパケットを配送した。
    /// @retval false
    ///   失敗。メッセージパケットを配送しなかった。 this_type::get_thread_id
    ///   と合致しないスレッドからこの関数を呼び出すと、失敗する。
    public: bool dispatch()
    {
        if (!this->verify_thread())
        {
            return false;
        }

        // 配送するメッセージパケットを取得する。
        {
            std::lock_guard<psyq::spinlock> const local_lock(this->lock_);
            this->delivery_packets_.swap(this->import_packets_);
        }
        this_type::remove_empty_hook(this->receiving_hooks_);
        this_type::remove_empty_hook(this->forwarding_hooks_);

        // メッセージパケットを、メッセージ受信関数へ配送する。
        this_type::deliver_packets(
            this->function_caches_,
            this->receiving_hooks_,
            this->forwarding_hooks_,
            this->delivery_packets_);
        this_type::clear_packet_container(
            this->delivery_packets_, this->delivery_packets_.size());
        return true;
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @brief this_type を構築する。
    /// @warning psyq::event_driven の管理者以外は、この関数は使用禁止。
    public: dispatcher(
        /// [in] *this に対応するスレッドの識別子。
        std::thread::id in_thread_id,
        /// [in] *this が使うメモリ割当子の初期値。
        typename this_type::allocator_type const& in_allocator):
    receiving_hooks_(in_allocator),
    forwarding_hooks_(in_allocator),
    import_packets_(in_allocator),
    export_packets_(in_allocator),
    delivery_packets_(in_allocator),
    function_caches_(in_allocator),
    thread_id_(std::move(in_thread_id))
    {}

    /// @brief コピー構築子は使用禁止。
    private: dispatcher(this_type const&);
    /// @brief ムーブ構築子は使用禁止。
    private: dispatcher(this_type&&);
    /// @brief コピー代入演算子は使用禁止。
    private: this_type& operator=(this_type const&);
    /// @brief ムーブ代入演算子は使用禁止。
    private: this_type& operator=(this_type&&);

    /// @brief 現在のスレッドが処理が許可されているスレッドか判定する。
    /// @retval true  現在のスレッドは、処理が許可されている。
    /// @retval false 現在のスレッドは、処理が許可されてない。
    private: bool verify_thread() const PSYQ_NOEXCEPT
    {
        auto const local_verify(
            std::this_thread::get_id() == this->get_thread_id());
        PSYQ_ASSERT(local_verify);
        return local_verify;
    }

    //-------------------------------------------------------------------------
    /// @brief 現在のスレッドにだけ、メッセージを送信する。
    /// @details
    ///   *this に登録されているメッセージ受信関数にだけメッセージを送信し、
    ///   メッセージ受信関数が終了するまでブロックする。
    /// @sa
    ///   - *this にメッセージ受信関数を登録するには、
    ///     this_type::register_receiving_function を使う。
    ///   - メッセージゾーンの内と外にメッセージを送信するには、
    ///     this_type::post_external を使う。
    ///   - メッセージゾーン内にだけメッセージを送信するには、
    ///     this_type::post_zonal を使う。
    /// @note
    ///   以下の問題を解決したなら public にすること。
    ///   - send_local の中から send_local されるとダメ。
    ///   - send_local の中から register_receiving_function されるとダメ。
    /// @retval true 成功。メッセージを送信した。
    /// @retval false
    ///   失敗。メッセージを送信しなかった。 this_type::get_thread_id
    ///   と合致しないスレッドからこの関数を呼び出すと、失敗する。
    private: bool send_local(
        /// [in] 送信するメッセージパケット。
        typename this_type::packet const& in_packet)
    {
        if (!this->verify_thread())
        {
            return false;
        }
        this_type::deliver_packet(
            this->function_caches_,
            this->receiving_hooks_,
            this->forwarding_hooks_,
            in_packet);
        return true;
    }

    /// @brief 現在のスレッドにだけ、引数を持たないメッセージを送信する。
    /// @details
    ///   *this に登録されているメッセージ受信関数にだけ、
    ///   引数を持たないメッセージを送信し、
    ///   メッセージ受信関数が終了するまでブロックする。
    /// @sa
    ///   - *this にメッセージ受信関数を登録するには、
    ///     this_type::register_receiving_function を使う。
    ///   - メッセージゾーンの内と外にメッセージを送信するには、
    ///     this_type::post_external を使う。
    ///   - メッセージゾーン内にだけメッセージを送信するには、
    ///     this_type::post_zonal を使う。
    /// @retval true 成功。メッセージを送信した。
    /// @retval false
    ///   失敗。メッセージを送信しなかった。 this_type::get_thread_id
    ///   と合致しないスレッドからこの関数を呼び出すと、失敗する。
    private: bool send_local(
        /// [in] 送信するメッセージの送り状。
        typename this_type::tag const& in_tag)
    {
        typedef typename this_type::packet::message message;
        return this->send_local(
            typename this_type::packet::template zonal<message>(message(in_tag)));
    }

    /// @brief 現在のスレッドにだけ、任意型の引数を持つメッセージを送信する。
    /// @details
    ///   *this に登録されているメッセージ受信関数にだけ、
    ///   任意型の引数を持つメッセージを送信し、
    ///   メッセージ受信関数が終了するまでブロックする。
    /// @sa
    ///   - *this にメッセージ受信関数を登録するには、
    ///     this_type::register_receiving_function を使う。
    ///   - メッセージゾーンの内と外にメッセージを送信するには、
    ///     this_type::post_external を使う。
    ///   - メッセージゾーン内にだけメッセージを送信するには、
    ///     this_type::post_zonal を使う。
    /// @retval true 成功。メッセージを送信した。
    /// @retval false
    ///   失敗。メッセージを送信しなかった。 this_type::get_thread_id
    ///   と合致しないスレッドからこの関数を呼び出すと、失敗する。
    private: template<typename template_parameter>
    bool send_local(
        /// [in] 送信するメッセージの送り状。
        typename this_type::tag const& in_tag,
        /// [in] 送信するメッセージの引数。
        template_parameter in_parameter)
    {
        typedef
            typename this_type::packet::message::template
                parametric<template_parameter>
            message;
        return this->send_local(
            typename this_type::packet::template
                zonal<message>(message(in_tag, std::move(in_parameter))));
    }

    //-------------------------------------------------------------------------
    private: template<typename template_iterator>
    static template_iterator find_receiving_hook_iterator(
        template_iterator const in_begin,
        template_iterator const in_end,
        typename this_type::tag::key_type const in_receiver_key,
        typename this_type::tag::key_type const in_selector_key)
    {
        for (
            auto i(
                std::lower_bound(
                    in_begin,
                    in_end,
                    in_selector_key,
                    typename this_type::receiving_hook::less()));
            i != in_end;
            ++i)
        {
            auto& local_hook(*i);
            if (local_hook.selector_key_ != in_selector_key)
            {
                break;
            }
            if (local_hook.receiver_key_ == in_receiver_key)
            {
                return i;
            }
        }
        return in_end;
    }

    /// @brief 空のメッセージ関数フックを削除する。
    private: template<typename template_container>
    static void remove_empty_hook(
        /// [in,out] 空のメッセージ関数フックを削除するコンテナ。
        template_container& io_hooks)
    {
        for (auto i(std::begin(io_hooks)); i != std::end(io_hooks);)
        {
            if (i->function_.expired())
            {
                i = io_hooks.erase(i);
            }
            else
            {
                ++i;
            }
        }
    }

    //-------------------------------------------------------------------------
    /// @brief メッセージパケットを、メッセージ受信関数へ配送する。
    private: static void deliver_packets(
        /// [in,out] メッセージ受信関数のキャッシュに使うコンテナ。
        typename this_type::function_shared_ptr_container& io_functions,
        /// [in] メッセージ受信フックの辞書。
        typename this_type::receiving_hook::container const& in_receiving_hooks,
        /// [in] メッセージ転送フックの辞書。
        typename this_type::forwarding_hook::container const& in_forwarding_hooks,
        /// [in] 配送するメッセージパケットのコンテナ。
        typename this_type::packet_shared_ptr_container const& in_packets)
    {
        // メッセージパケットを走査し、メッセージ受信フックの辞書へ中継する。
        io_functions.clear();
        for (auto& local_packet_holder: in_packets)
        {
            auto const local_packet_pointer(local_packet_holder.get());
            if (local_packet_pointer != nullptr)
            {
                // メッセージ受信関数の識別値が一致するメッセージ受信フックに
                // メッセージパケットを配送する。
                this_type::deliver_packet(
                    io_functions,
                    in_receiving_hooks,
                    in_forwarding_hooks,
                    *local_packet_pointer);
            }
            else
            {
                PSYQ_ASSERT(false);
            }
        }
    }

    /// @brief メッセージパケットを、メッセージ受信関数へ配送する。
    private: static void deliver_packet(
        /// [in,out] メッセージ受信関数のキャッシュに使うコンテナ。
        typename this_type::function_shared_ptr_container& io_functions,
        /// [in] メッセージ受信フックの辞書。
        typename this_type::receiving_hook::container const& in_receiving_hooks,
        /// [in] メッセージ転送フックの辞書。
        typename this_type::forwarding_hook::container const& in_forwarding_hooks,
        /// [in] 配送するメッセージパケット。
        typename this_type::packet const& in_packet)
    {
        PSYQ_ASSERT(io_functions.empty());
        auto& local_tag(in_packet.get_message().get_tag());
        this_type::cache_receiving_functions(
            io_functions, in_receiving_hooks, local_tag);
        this_type::cache_forwarding_functions(
            io_functions, in_forwarding_hooks, local_tag);
        for (auto const& local_function: io_functions)
        {
            (*local_function)(in_packet);
        }
        io_functions.clear();
    }

    /// @brief メッセージパケットを配送するメッセージ受信関数を貯める。
    private: static void cache_receiving_functions(
        /// [in,out] メッセージ受信関数のキャッシュに使うコンテナ。
        typename this_type::function_shared_ptr_container& io_functions,
        /// [in] メッセージ受信フックの辞書。
        typename this_type::receiving_hook::container const& in_hooks,
        /// [in] 配送するメッセージパケットの送り状。
        typename this_type::tag const& in_tag)
    {
        // メッセージ受信フックの辞書を走査し、
        // メッセージ受信関数の識別値が一致するメッセージ受信フックを検索する。
        auto const local_end(std::end(in_hooks));
        for (
            auto i(
                std::lower_bound(
                    std::begin(in_hooks),
                    local_end,
                    in_tag.get_selector_key(),
                    typename this_type::receiving_hook::less()));
            i != local_end;
            ++i)
        {
            auto& local_hook(*i);
            if (local_hook.selector_key_ != in_tag.get_selector_key())
            {
                break;
            }

            // メッセージ受信関数をキャッシュに貯める。
            if (in_tag.verify_receiver_key(local_hook.receiver_key_))
            {
                auto local_function(local_hook.function_.lock());
                if (local_function.get() != nullptr)
                {
                    io_functions.emplace_back(std::move(local_function));
                }
            }
        }
    }

    /// @brief メッセージパケットを配送するメッセージ転送関数を貯める。
    private: static void cache_forwarding_functions(
        typename this_type::function_shared_ptr_container& io_functions,
        typename this_type::forwarding_hook::container const& in_hooks,
        typename this_type::tag const& in_tag)
    {
        for (auto& local_hook: in_hooks)
        {
            if (in_tag.verify_receiver_key(local_hook.receiver_key_))
            {
                /// @todo 未実装
            }
        }
    }

    //-------------------------------------------------------------------------
    /// @brief 外部とメッセージパケットを交換する。
    private: void trade_packet_container(
        /// [in,out] 輸出するメッセージパケットのコンテナ。
        typename this_type::packet_shared_ptr_container& io_export_packets,
        /// [in] 輸入するメッセージパケットのコンテナ。
        typename this_type::packet_shared_ptr_container const& in_import_packets)
    {
        std::lock_guard<psyq::spinlock> const local_lock(this->lock_);

        // メッセージパケットを輸出する。
        io_export_packets.reserve(
            io_export_packets.size() + this->export_packets_.size());
        for (auto& local_packet_holder: this->export_packets_)
        {
            io_export_packets.emplace_back(std::move(local_packet_holder));
        }
        this_type::clear_packet_container(
            this->export_packets_, this->export_packets_.size());

        // メッセージパケットを輸入する。
        this->import_packets_.insert(
            std::end(this->import_packets_),
            std::begin(in_import_packets),
            std::end(in_import_packets));
    }

    private: static void clear_packet_container(
        typename this_type::packet_shared_ptr_container& io_container,
        std::size_t const in_last_size)
    {
        if (in_last_size < 16 || io_container.capacity() < in_last_size * 2)
        {
            io_container.clear();
        }
        else
        {
            io_container = typename this_type::packet_shared_ptr_container();
            io_container.reserve(in_last_size * 2);
        }
    }

    //-------------------------------------------------------------------------
    /// @brief メッセージ受信フックの辞書。
    private: typename this_type::receiving_hook::container receiving_hooks_;
    /// @brief メッセージ転送フックの辞書。
    private: typename this_type::forwarding_hook::container forwarding_hooks_;
    /// @brief 外部から輸入したメッセージパケットのコンテナ。
    private: typename this_type::packet_shared_ptr_container import_packets_;
    /// @brief 外部へ輸出するメッセージパケットのコンテナ。
    private: typename this_type::packet_shared_ptr_container export_packets_;
    /// @brief メッセージ受信関数へ配送するメッセージパケット。
    private: typename this_type::packet_shared_ptr_container delivery_packets_;
    /// @brief メッセージ受信関数のキャッシュ。
    private: typename this_type::function_shared_ptr_container function_caches_;
    /// @brief 排他的処理に使うロックオブジェクト。
    private: psyq::spinlock lock_;
    /// @brief *this に合致するスレッドの識別子。
    private: std::thread::id const thread_id_;

}; // class psyq::event_driven::dispatcher

#include "./zone.hpp"
#endif // !defined(PSYQ_EVENT_DRIVEN_DISPATCHER_HPP_)
// vim: set expandtab:
