/// @file
/// @author Hillco Psychi (https://twitter.com/psychi)
#ifndef PSYQ_EVENT_DRIVEN_DISPATCHER_HPP_
#define PSYQ_EVENT_DRIVEN_DISPATCHER_HPP_

#ifndef PSYQ_EVENT_DRIVEN_DISPATCHER_RECEIVER_CAPACITY_DEFFAULT
#define PSYQ_EVENT_DRIVEN_DISPATCHER_RECEIVER_CAPACITY_DEFFAULT 32
#endif // !defined(PSYQ_EVENT_DRIVEN_DISPATCHER_RECEIVER_CAPACITY_DEFFAULT)

#ifndef PSYQ_EVENT_DRIVEN_DISPATCHER_FORWARDER_CAPACITY_DEFFAULT
#define PSYQ_EVENT_DRIVEN_DISPATCHER_FORWARDER_CAPACITY_DEFFAULT 0
#endif // !defined(PSYQ_EVENT_DRIVEN_DISPATCHER_FORWARDER_CAPACITY_DEFFAULT)

#ifndef PSYQ_EVENT_DRIVEN_RECEIVER_PRIORITY_DEFAULT
#define PSYQ_EVENT_DRIVEN_RECEIVER_PRIORITY_DEFAULT 0
#endif // !defined(PSYQ_EVENT_DRIVEN_RECEIVER_PRIORITY_DEFAULT)

#ifndef PSYQ_EVENT_DRIVEN_FORWARDER_PRIORITY_DEFAULT
#define PSYQ_EVENT_DRIVEN_FORWARDER_PRIORITY_DEFAULT 0
#endif // !defined(PSYQ_EVENT_DRIVEN_FORWARDER_PRIORITY_DEFAULT)

/// @cond
namespace psyq
{
    namespace event_driven
    {
        template<typename, typename, typename> class zone;
        template<typename, typename, typename> class dispatcher;
    } // namespace event_driven
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief スレッド別のメッセージ配送器。
/// @par 使い方の概略
///   - zone::equip_dispatcher
///     で、スレッドごとにメッセージ配送器のインスタンスを用意する。
///   - dispatcher::register_receiver で、メッセージ受信関数を登録する。
///   - 以下のいずれかの関数で、メッセージを送信する。
///     - zone::post_external
///     - zone::post_zonal
///     - zone::post
///   - zone::dispatch
///     で、メッセージゾーンが持つメッセージパケットをメッセージ配送器へ配送する。
///   - dispatcher::dispatch
///     で、メッセージ配送器が持つメッセージパケットをメッセージ受信関数へ配送する。
/// @tparam template_packet    @copydoc dispatcher::packet
/// @tparam template_priority  @copydoc dispatcher::function_priority
/// @tparam template_allocator @copydoc dispatcher::allocator_type
template<
    typename template_packet,
    typename template_priority,
    typename template_allocator>
class psyq::event_driven::dispatcher
{
    /// @copydoc psyq::string::view::this_type
    private: typedef dispatcher this_type;
    friend class psyq::event_driven::zone<
        template_packet, template_priority, template_allocator>;

    //-------------------------------------------------------------------------
    /// this_type を強参照するスマートポインタ。
    public: typedef std::shared_ptr<this_type> shared_ptr;
    /// this_type を弱参照するスマートポインタ。
    public: typedef std::weak_ptr<this_type> weak_ptr;
    /// @brief 送受信するメッセージパケットの基底型。
    /// @details event_driven::packet 互換のインタフェイスを持つこと。
    public: typedef template_packet packet;
    /// @brief this_type で使うメモリ割当子。
    public: typedef template_allocator allocator_type;

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
    /// @brief メッセージ受信関数の優先順位。
    public: typedef template_priority function_priority;

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
    /// @brief メッセージ転送フック。
    private: class forwarding_hook
    {
        private: typedef forwarding_hook this_type;

        public: typedef
            std::vector<this_type, typename dispatcher::allocator_type>
            container;

        public: struct less
        {
            bool operator()(
                this_type const& in_left,
                this_type const& in_right)
            const PSYQ_NOEXCEPT
            {
                return in_left.receiver_key_ != in_right.receiver_key_?
                    in_left.receiver_key_ < in_right.receiver_key_:
                    in_left.priority_ < in_right.priority_;
            }

            bool operator()(
                this_type const& in_left,
                typename dispatcher::packet::message::tag::key_type const& in_right)
            const PSYQ_NOEXCEPT
            {
                return in_left.receiver_key_ < in_right;
            }

            bool operator()(
                typename dispatcher::packet::message::tag::key_type const& in_left,
                this_type const& in_right)
            const PSYQ_NOEXCEPT
            {
                return in_left < in_right.receiver_key_;
            }

        }; // struct less

        public: forwarding_hook(
            typename dispatcher::packet::message::tag::key_type const in_receiver_key,
            typename dispatcher::function_weak_ptr&& io_function,
            typename dispatcher::function_priority const in_priority):
        function_(std::move(io_function)),
        receiver_key_(in_receiver_key),
        priority_(in_priority)
        {}

#ifdef PSYQ_NO_STD_DEFAULTED_FUNCTION
        /// @brief ムーブ構築子。
        public: forwarding_hook(
            /// [in,out] ムーブ元となるインスタンス。
            this_type&& io_source):
        function_(std::move(io_source.function_)),
        receiver_key_(std::move(io_source.receiver_key_)),
        priority_(std::move(io_source.priority_))
        {}

        /// @brief ムーブ代入演算子。
        /// @return *this
        public: this_type& operator=(
            /// [in,out] ムーブ元となるインスタンス。
            this_type&& io_source)
        {
            this->function_ = std::move(io_source.function_);
            this->receiver_key_ = std::move(io_source.receiver_key_);
            this->priority_ = std::move(io_source.priority_);
            return *this;
        }
#endif // !defined(PSYQ_NO_STD_DEFAULTED_FUNCTION)

        public: typename dispatcher::function_weak_ptr function_;
        public: typename dispatcher::packet::message::tag::key_type receiver_key_;
        public: typename dispatcher::function_priority priority_;

    }; // class forwarding_hook

    //-------------------------------------------------------------------------
    /// @brief メッセージ受信フック。
    private: class receiving_hook: public forwarding_hook
    {
        private: typedef receiving_hook this_type;
        public: typedef forwarding_hook base_type;

        public: typedef
            std::vector<this_type, typename dispatcher::allocator_type>
            container;

        public: struct less
        {
            bool operator()(
                this_type const& in_left,
                this_type const& in_right)
            const PSYQ_NOEXCEPT
            {
                return in_left.selector_key_ != in_right.selector_key_?
                    in_left.selector_key_ < in_right.selector_key_:
                    in_left.priority_ < in_right.priority_;
            }

            bool operator()(
                this_type const& in_left,
                typename dispatcher::packet::message::tag::key_type const& in_right)
            const PSYQ_NOEXCEPT
            {
                return in_left.selector_key_ < in_right;
            }

            bool operator()(
                typename dispatcher::packet::message::tag::key_type const& in_left,
                this_type const& in_right)
            const PSYQ_NOEXCEPT
            {
                return in_left < in_right.selector_key_;
            }

        }; // struct less

        public: receiving_hook(
            typename dispatcher::packet::message::tag::key_type const in_receiver_key,
            typename dispatcher::packet::message::tag::key_type const in_selector_key,
            typename dispatcher::function_weak_ptr&& io_function,
            typename dispatcher::function_priority const in_priority):
        base_type(in_receiver_key, std::move(io_function), in_priority),
        selector_key_(in_selector_key)
        {}

#ifdef PSYQ_NO_STD_DEFAULTED_FUNCTION
        /// @brief ムーブ構築子。
        public: receiving_hook(
            /// [in,out] ムーブ元となるインスタンス。
            this_type&& io_source):
        base_type(std::move(io_source)),
        selector_key_(std::move(io_source.selector_key_))
        {}

        /// @brief ムーブ代入演算子。
        /// @return *this
        public: this_type& operator=(
            /// [in,out] ムーブ元となるインスタンス。
            this_type&& io_source)
        {
            static_cast<base_type&>(*this) = std::move(io_source);
            this->selector_key_ = std::move(io_source.selector_key_);
            return *this;
        }
#endif // !defined(PSYQ_NO_STD_DEFAULTED_FUNCTION)

        public: typename dispatcher::packet::message::tag::key_type selector_key_;

    }; // class receiving_hook

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
    public: std::thread::id const& get_thread_id() const PSYQ_NOEXCEPT
    {
        return this->thread_id_;
    }

    //-------------------------------------------------------------------------
    /// @name メッセージの受信
    /// @{

    /// @brief メッセージ受信関数へメッセージパケットを配送する。
    /// @details
    ///   この関数と zone::dispatch
    ///   をメッセージループで定期的に呼び出し、メッセージパケットを循環させること。
    /// @sa
    ///   *this にメッセージ受信関数を登録するには、
    ///   this_type::register_receiver を使う。
    /// @retval true 成功。メッセージパケットを配送した。
    /// @retval false
    ///   失敗。メッセージパケットを配送しなかった。 this_type::get_thread_id
    ///   と合致しないスレッドからこの関数を呼び出すと、失敗する。
    public: bool dispatch(
        /// [in] メッセージパケットの予約数。
        std::size_t const in_capacity = 0,
        /// [in] コンテナを再構築するか。
        bool const in_rebuild = false)
    {
        if (!this->verify_thread())
        {
            return false;
        }

        // 配送するメッセージパケットを取得する。
        PSYQ_ASSERT(this->delivery_packets_.empty());
        {
            std::lock_guard<psyq::spinlock> const local_lock(this->lock_);
            this->delivery_packets_.swap(this->receiving_packets_);
            this_type::clear_packets(
                this->receiving_packets_, in_capacity, in_rebuild);
        }

        // メッセージ受信関数へメッセージパケットを配送する。
        this_type::remove_empty_hook(this->receiving_hooks_);
        this_type::remove_empty_hook(this->forwarding_hooks_);
        this_type::deliver_packets(
            this->function_caches_,
            this->receiving_hooks_,
            this->forwarding_hooks_,
            this->delivery_packets_);

        // コンテナを整理する。
        this_type::clear_packets(
            this->delivery_packets_, in_capacity, in_rebuild);
        if (in_rebuild)
        {
            this->receiving_hooks_.shrink_to_fit();
            this->forwarding_hooks_.shrink_to_fit();
            this->function_caches_.shrink_to_fit();
            this->function_caches_.reserve(this->receiving_hooks_.capacity());
        }
        return true;
    }

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
    ///     this_type::unregister_receiver を呼び出す。
    /// .
    /// @retval true 成功。 in_function を登録した。
    /// @retval false
    ///     失敗。 in_function を登録しなかった。
    ///     - this_type::get_thread_id
    ///       と合致しないスレッドから呼び出すと、失敗する。
    ///     - in_receiver_key / in_selector_key の組み合わせが同じ
    ///       メッセージ受信関数がすでに追加されていると、失敗する。
    ///     - in_function が空だと、失敗する。
    public: bool register_receiver(
        /// [in] 登録するメッセージ受信関数に対応する、
        /// メッセージ受信オブジェクトの識別値。
        typename this_type::packet::message::tag::key_type const in_receiver_key,
        /// [in] 登録するメッセージ受信関数の識別値。
        typename this_type::packet::message::tag::key_type const in_selector_key,
        /// [in] 登録するメッセージ受信関数。
        typename this_type::function_shared_ptr const& in_function,
        /// [in] メッセージを受信する優先順位。
        typename this_type::function_priority const in_priority
            = PSYQ_EVENT_DRIVEN_RECEIVER_PRIORITY_DEFAULT)
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
            typename this_type::function_weak_ptr(in_function),
            in_priority);
        return true;
    }

    /// @brief メッセージ受信関数を取り除く。
    /// @return
    ///   取り除いたメッセージ受信関数を指すスマートポインタ。
    ///   該当するメッセージ受信関数がない場合は、空となる。
    public: typename this_type::function_weak_ptr unregister_receiver(
        /// [in] 除去するメッセージ受信関数に対応する
        /// メッセージ受信オブジェクトの識別値。
        typename this_type::packet::message::tag::key_type const in_receiver_key,
        /// [in] 除去するメッセージ受信関数の識別値。
        typename this_type::packet::message::tag::key_type const in_selector_key)
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
    public: std::size_t unregister_receiver(
        /// [in] 取り除くメッセージ受信関数に対応する
        /// メッセージ受信オブジェクトの識別値。
        typename this_type::packet::message::tag::key_type const in_receiver_key)
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
    public: typename this_type::function_weak_ptr find_receiver(
        /// [in] 検索するメッセージ受信関数に対応する
        /// メッセージ受信オブジェクトの識別値。
        typename this_type::packet::message::tag::key_type const in_receiver_key,
        /// [in] 検索するメッセージ受信関数の識別値。
        typename this_type::packet::message::tag::key_type const in_selector_key)
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
    /// @name メッセージの転送
    /// @{

    /// @brief メッセージ転送関数を登録する。
    /// @todo 未実装
    private: bool register_forwarder(
        /// [in] 登録するメッセージ転送関数に対応する、
        /// メッセージ受信オブジェクトの識別値。
        typename this_type::packet::message::tag::key_type const in_receiver_key,
        /// [in] 登録するメッセージ転送関数。
        typename this_type::function_shared_ptr const& in_function,
        /// [in] メッセージを転送する優先順位。
        typename this_type::function_priority const in_priority
            = PSYQ_EVENT_DRIVEN_FORWARDER_PRIORITY_DEFAULT);

    /// @brief メッセージ転送関数を取り除く。
    /// @todo 未実装
    private: typename this_type::function_weak_ptr unregister_forwarder(
        /// [in] 取り除くメッセージ転送関数に対応する、
        /// メッセージ受信オブジェクトの識別値。
        typename this_type::packet::message::tag::key_type const in_receiver_key);

    /// @brief メッセージ転送関数を検索する。
    /// @todo 未実装
    private: typename this_type::function_weak_ptr find_forwarder(
        /// [in] 検索するメッセージ転送関数に対応する、
        /// メッセージ受信オブジェクトの識別値。
        typename this_type::packet::message::tag::key_type const in_receiver_key);
    /// @}
    //-------------------------------------------------------------------------
    /// @brief this_type を構築する。
    /// @warning psyq::event_driven の管理者以外は、この関数は使用禁止。
    public: dispatcher(
        /// [in] *this に対応するスレッドの識別子。
        std::thread::id const& in_thread_id,
        /// [in] メッセージパケットの予約数。
        std::size_t const in_packet_capacity,
        /// [in] メッセージ受信関数の予約数。
        std::size_t const in_receiver_capacity,
        /// [in] メッセージ転送関数の予約数。
        std::size_t const in_forwarder_capacity,
        /// [in] *this が使うメモリ割当子の初期値。
        typename this_type::allocator_type const& in_allocator):
    receiving_hooks_(in_allocator),
    forwarding_hooks_(in_allocator),
    receiving_packets_(in_allocator),
    delivery_packets_(in_allocator),
    function_caches_(in_allocator),
    thread_id_(in_thread_id)
    {
        this->receiving_hooks_.reserve(in_receiver_capacity);
        this->forwarding_hooks_.reserve(in_forwarder_capacity);
        this->receiving_packets_.reserve(in_packet_capacity);
        this->delivery_packets_.reserve(in_packet_capacity);
        this->function_caches_.reserve(in_receiver_capacity);
    }

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
    ///     this_type::register_receiver を使う。
    ///   - メッセージゾーンの内と外にメッセージを送信するには、
    ///     this_type::post_external を使う。
    ///   - メッセージゾーン内にだけメッセージを送信するには、
    ///     this_type::post_zonal を使う。
    /// @note
    ///   以下の問題を解決したなら public にすること。
    ///   - send_local の中から send_local されるとダメ。
    ///   - send_local の中から register_receiver されるとダメ。
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
    ///     this_type::register_receiver を使う。
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
        typename this_type::packet::message::tag const& in_tag)
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
    ///     this_type::register_receiver を使う。
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
        typename this_type::packet::message::tag const& in_tag,
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
        typename this_type::packet::message::tag::key_type const in_receiver_key,
        typename this_type::packet::message::tag::key_type const in_selector_key)
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
    /// @brief 外部からメッセージパケットを受信する。
    private: void receive_packets(
        /// [in] 受信するメッセージパケットのコンテナ。
        typename this_type::packet_shared_ptr_container const&
            in_receiving_packets)
    {
        std::lock_guard<psyq::spinlock> const local_lock(this->lock_);
        this->receiving_packets_.insert(
            std::end(this->receiving_packets_),
            std::begin(in_receiving_packets),
            std::end(in_receiving_packets));
    }

    private: static void clear_packets(
        typename this_type::packet_shared_ptr_container& io_packets,
        std::size_t const in_capacity,
        bool const in_rebuild)
    {
        if (in_rebuild)
        {
            io_packets = typename this_type::packet_shared_ptr_container(
                io_packets.get_allocator());
        }
        else
        {
            io_packets.clear();
        }
        io_packets.reserve(in_capacity);
    }

    /// @brief メッセージ受信関数へメッセージパケットを配送する。
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
        this_type::cache_receivers(
            io_functions, in_receiving_hooks, local_tag);
        this_type::cache_forwarders(
            io_functions, in_forwarding_hooks, local_tag);
        for (auto const& local_function: io_functions)
        {
            (*local_function)(in_packet);
        }
        io_functions.clear();
    }

    /// @brief メッセージパケットを配送するメッセージ受信関数を貯める。
    private: static void cache_receivers(
        /// [in,out] メッセージ受信関数のキャッシュに使うコンテナ。
        typename this_type::function_shared_ptr_container& io_functions,
        /// [in] メッセージ受信フックの辞書。
        typename this_type::receiving_hook::container const& in_hooks,
        /// [in] 配送するメッセージパケットの送り状。
        typename this_type::packet::message::tag const& in_tag)
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
    private: static void cache_forwarders(
        typename this_type::function_shared_ptr_container& io_functions,
        typename this_type::forwarding_hook::container const& in_hooks,
        typename this_type::packet::message::tag const& in_tag)
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
    /// @brief メッセージ受信フックの辞書。
    private: typename this_type::receiving_hook::container receiving_hooks_;
    /// @brief メッセージ転送フックの辞書。
    private: typename this_type::forwarding_hook::container forwarding_hooks_;
    /// @brief 外部から受信したメッセージパケットのコンテナ。
    private: typename this_type::packet_shared_ptr_container receiving_packets_;
    /// @brief メッセージ受信関数へ配送するメッセージパケットのコンテナ。
    private: typename this_type::packet_shared_ptr_container delivery_packets_;
    /// @brief メッセージ受信関数のキャッシュ。
    private: typename this_type::function_shared_ptr_container function_caches_;
    /// @brief 排他的処理に使うロックオブジェクト。
    private: psyq::spinlock lock_;
    /// @brief *this に合致するスレッドの識別子。
    private: std::thread::id const thread_id_;

}; // class psyq::event_driven::dispatcher

#endif // !defined(PSYQ_EVENT_DRIVEN_DISPATCHER_HPP_)
// vim: set expandtab:
