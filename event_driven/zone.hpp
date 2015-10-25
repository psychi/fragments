/// @file
/// @author Hillco Psychi (https://twitter.com/psychi)
#ifndef PSYQ_EVENT_DRIVEN_ZONE_HPP_
#define PSYQ_EVENT_DRIVEN_ZONE_HPP_

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief メッセージを送受信する範囲の単位。
/// @details
///   - zone::equip_dispatcher で、 zone::dispatcher を用意する。
///   - zone::dispatch で、 zone::dispatcher が持つメッセージパケットを集配する。
///   .
/// @tparam template_base_message @copydoc packet::message
/// @tparam template_priority     @copydoc dispatcher::priority
/// @tparam template_allocator    @copydoc zone::allocator_type
template<
    typename template_base_message,
    typename template_priority,
    typename template_allocator>
class psyq::event_driven::dispatcher<
    template_base_message, template_priority, template_allocator>::zone
{
    /// @copydoc psyq::string::view::this_type
    private: typedef zone this_type;

    //-------------------------------------------------------------------------
    /// @brief this_type で使うメモリ割当子。
    public: typedef template_allocator allocator_type;
    /// @brief this_type で使うメッセージ配送器。
    public: typedef
        psyq::event_driven::dispatcher<
            template_base_message, template_priority, template_allocator>
        dispatcher;

    //-------------------------------------------------------------------------
    /// @copydoc this_type::dispatchers_
    private: typedef
        std::vector<
            std::weak_ptr<typename this_type::dispatcher>,
            typename this_type::allocator_type>
        weak_dispatcher_container;

    //-------------------------------------------------------------------------
    /// @brief メッセージゾーンを構築する。
    public: explicit zone(
        /// [in] *thisが使うメモリ割当子の初期値。
        typename this_type::allocator_type const& in_allocator =
            template_allocator()):
    dispatchers_(in_allocator),
    import_packets_(in_allocator),
    export_packets_(in_allocator)
    {}

    /// @brief メモリ割当子を取得する。
    /// @return *thisが使っているメモリ割当子。
    public: PSYQ_CONSTEXPR typename this_type::allocator_type get_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->dispatchers_.get_allocator();
    }

    /// @brief メッセージパケットを集配する。
    /// @details
    ///   - this_type::dispatcher::dispatch とこの関数を定期的に実行し、
    ///     メッセージパケットを循環させること。
    ///   - this_type::lock_ でロックの獲得を待ってから実行される。
    ///     ロックを獲得している他のスレッドからブロックされることに注意。
    public: void dispatch()
    {
        std::lock_guard<psyq::spinlock> local_lock(this->lock_);
        this_type::trade_packet_container(
            this->dispatchers_, this->export_packets_, this->import_packets_);
        this_type::swap_packet_container(
            this->export_packets_, this->import_packets_);
    }

    /// @brief スレッドに合致する this_type::dispatcher を用意する。
    /// @details
    ///   - スレッドに合致する this_type::dispatcher を検索する。
    ///     存在しないなら、動的メモリ割当して生成する。
    ///   - this_type::lock_ でロックの獲得を待ってから実行される。
    ///     ロックを獲得している他のスレッドからブロックされることに注意。
    ///   - 用意した this_type::dispatcher は this_type::dispatcher::weak_ptr
    ///     で弱参照するだけで、*thisでは所有権を持たない。
    ///     this_type::dispatcher の所有権は、ユーザーが管理すること。
    /// @return in_thread_id に合致する this_type::dispatcher 。
    public: typename this_type::dispatcher::shared_ptr equip_dispatcher(
        /// [in] 用意するメッセージ配送器のスレッド識別子。
        std::thread::id const& in_thread_id = std::this_thread::get_id())
    {
        std::lock_guard<psyq::spinlock> local_lock(this->lock_);
        auto const local_dispatcher(
            this_type::find_dispatcher(this->dispatchers_, in_thread_id));
        return local_dispatcher.get() != nullptr?
            local_dispatcher:
            this_type::make_dispatcher(this->dispatchers_, in_thread_id);
    }

    //-------------------------------------------------------------------------
    /// @brief コピー構築子は使用禁止。
    private: zone(this_type const&);
    /// @brief コピー代入演算子は使用禁止。
    private: this_type& operator=(this_type const&);

    /// @brief this_type::dispatcher をコンテナから検索する。
    /// @return
    ///   検索した this_type::dispatcher を強参照するスマートポインタ。
    ///   見つからなかった場合、スマートポインタは空となる。
    private: static typename this_type::dispatcher::shared_ptr find_dispatcher(
        /// [in] 検索する this_type::dispatcher のコンテナ。
        typename this_type::weak_dispatcher_container const& in_dispatchers,
        /// [in] 検索する this_type::dispatcher に対応するスレッドの識別子。
        std::thread::id const& in_thread_id)
    {
        for (auto const& local_observer: in_dispatchers)
        {
            auto const local_holder(local_observer.lock());
            auto const local_dispatcher(local_holder.get());
            if (local_dispatcher != nullptr
                && local_dispatcher->get_thread_id() == in_thread_id)
            {
                return local_holder;
            }
        }
        return typename this_type::dispatcher::shared_ptr();
    }

    /// @brief this_type::dispatcher を生成し、コンテナに追加する。
    /// @return
    ///   生成した this_type::dispatcher を強参照するスマートポインタ。
    ///   生成に失敗した場合、スマートポインタは空となる。
    private: static typename this_type::dispatcher::shared_ptr make_dispatcher(
        /// [in,out] 生成した this_type::dispatcher を追加するコンテナ。
        typename this_type::weak_dispatcher_container& io_dispatchers,
        /// [in] 生成する this_type::dispatcher に対応するスレッドの識別子。
        std::thread::id const& in_thread_id)
    {
        // this_type::dispatcher を構築する。
        auto const local_allocator(io_dispatchers.get_allocator());
        auto const local_dispatcher(
            std::allocate_shared<typename this_type::dispatcher>(
                local_allocator, in_thread_id, local_allocator));
        if (local_dispatcher.get() != nullptr)
        {
            io_dispatchers.push_back(local_dispatcher);
        }
        else
        {
            PSYQ_ASSERT(false);
        }
        return local_dispatcher;
    }

    /// @brief this_type::dispatcher が持つメッセージパケットを集配する。
    private: static void trade_packet_container(
        /// [in,out] メッセージパケットを集配する this_type::dispatcher のコンテナ。
        typename this_type::weak_dispatcher_container& io_dispatchers,
        /// [in,out] this_type::dispatcher からメッセージパケットを集めて格納するコンテナ。
        typename this_type::dispatcher::packet_shared_ptr_container&
            io_export_packets,
        /// [in] this_type::dispatcher へ配るメッセージパケットのコンテナ。
        typename this_type::dispatcher::packet_shared_ptr_container const&
            in_import_packets)
    {
        auto local_last(std::begin(io_dispatchers));
        auto const local_end(std::end(io_dispatchers));
        for (auto i(local_last); i != local_end; ++i)
        {
            auto const local_holder(i->lock());
            auto const local_dispatcher(local_holder.get());
            if (local_dispatcher != nullptr)
            {
                local_dispatcher->trade_packet_container(
                    io_export_packets, in_import_packets);
                i->swap(*local_last);
                ++local_last;
            }
        }
        io_dispatchers.erase(local_last, local_end);
    }

    /// @brief 集配されたメッセージパケットのコンテナを交換する。
    private: static void swap_packet_container(
        /// [in,out] this_type::dispatcher から集めたメッセージパケットのコンテナ。
        typename this_type::dispatcher::packet_shared_ptr_container& io_export_packets,
        /// [in,out] this_type::dispatcher へ配ったメッセージパケットのコンテナ。
        typename this_type::dispatcher::packet_shared_ptr_container& io_import_packets)
    {
        // 配信済コンテナを空にしてから交換する。
        this_type::dispatcher::clear_packet_container(
            io_import_packets, io_export_packets.size());
        io_export_packets.swap(io_import_packets);
    }

    //-------------------------------------------------------------------------
    /// @brief 各スレッドに合致する this_type::dispatcher のコンテナ。
    private: typename this_type::weak_dispatcher_container dispatchers_;
    /// @brief this_type::dispatcher から集めたメッセージパケットのコンテナ。
    private: typename this_type::dispatcher::packet_shared_ptr_container
         import_packets_;
    /// @brief this_type::dispatcher へ配るメッセージパケットのコンテナ。
    private: typename this_type::dispatcher::packet_shared_ptr_container
         export_packets_;
    /// @brief 排他的処理に使うロックオブジェクト。
    private: psyq::spinlock lock_;

}; // class psyq::event_driven::zone

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq_test
{
    template<typename template_value> struct value_wrapper
    {
        typedef value_wrapper this_type;
        typedef template_value value_type;
        typedef std::shared_ptr<this_type> shared_ptr;
        value_wrapper() {}
        value_wrapper(template_value const in_value): value(in_value) {}
        template_value value;
    };
    typedef value_wrapper<std::int32_t> integer_wrapper;
    typedef value_wrapper<double> floating_wrapper;
    inline void event_driven()
    {
        psyq::any::rtti::make<psyq_test::floating_wrapper>();

        typedef psyq::event_driven::dispatcher<> message_dispatcher;
        message_dispatcher::zone local_zone;
        auto const local_dispatcher_holder(local_zone.equip_dispatcher());
        auto& local_dispatcher(*local_dispatcher_holder);
        auto const local_method_a(
            std::allocate_shared<message_dispatcher::function>(
                local_zone.get_allocator(),
                [](message_dispatcher::packet const& in_packet)
                {
                    PSYQ_ASSERT(
                        in_packet.get_parameter_rtti()
                        == psyq::any::rtti::find<void>());
                }));
        double const local_double(0.5);
        auto const local_method_b(
            std::allocate_shared<message_dispatcher::function>(
                local_zone.get_allocator(),
                [local_double](message_dispatcher::packet const& in_packet)
                {
                    auto const local_parameter(
                        in_packet.get_parameter<psyq_test::floating_wrapper>());
                    if (local_parameter != nullptr)
                    {
                        PSYQ_ASSERT(local_parameter->value == local_double);
                    }
                    else
                    {
                        PSYQ_ASSERT(false);
                    }
                }));
        enum: message_dispatcher::tag::key_type
        {
            SENDER_KEY = 20,
            RECEIVER_KEY = 10,
            METHOD_PARAMETER_VOID = 1,
            METHOD_PARAMETER_DOUBLE,
        };
        local_dispatcher.register_receiving_function(
            RECEIVER_KEY, METHOD_PARAMETER_VOID, 0, local_method_a);
        local_dispatcher.register_receiving_function(
            RECEIVER_KEY, METHOD_PARAMETER_DOUBLE, 0, local_method_b);
        local_dispatcher.send_local_message(
            message_dispatcher::tag(
                SENDER_KEY, RECEIVER_KEY, ~0, METHOD_PARAMETER_VOID));
        local_dispatcher.send_local_message(
            message_dispatcher::tag(
                SENDER_KEY, RECEIVER_KEY, ~0, METHOD_PARAMETER_DOUBLE),
            psyq_test::floating_wrapper(local_double));
        local_dispatcher.post_message(
            message_dispatcher::tag(
                SENDER_KEY, RECEIVER_KEY, ~0, METHOD_PARAMETER_VOID));
        local_dispatcher.post_zonal_message(
            message_dispatcher::tag(
                SENDER_KEY, RECEIVER_KEY, ~0, METHOD_PARAMETER_DOUBLE),
            psyq_test::floating_wrapper(local_double));
        local_zone.dispatch();
        local_dispatcher.dispatch();
        local_zone.dispatch();
        local_dispatcher.dispatch();
        local_dispatcher.unregister_receiving_function(
            RECEIVER_KEY, METHOD_PARAMETER_VOID);
        PSYQ_ASSERT(
            local_dispatcher.unregister_receiving_function(RECEIVER_KEY) == 1);
    }
}

#endif // !defined(PSYQ_EVENT_DRIVEN_ZONE_HPP_)
// vim: set expandtab:
