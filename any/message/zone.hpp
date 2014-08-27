/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @namespace psyq::any::message
    @brief psyq::any::rtti を使った、プロセス間スレッド間のRPCメッセージ。

     初期化の手順
    -# psyq::any::message::zone インスタンスを用意する。
    -# psyq::any::message::zone::equip_transmitter()
       で、それぞれのスレッドに合致する psyq::any::message::transmitter
       インスタンスを用意し、保持しておく。
    -# メッセージ受信関数を設定した
       psyq::any::message::receiver インスタンスを用意した後、
       psyq::any::message::transmitter::register_receiver() で登録し、
       保持しておく。

    メッセージ送受信の手順
    -# それぞれのスレッドに合致する psyq::any::message::transmitter で
       psyq::any::message::transmitter::post_message() を実行し、
       メッセージパケットを送信する。
    -# psyq::any::message::zone::flush() をメインスレッドで定期的に実行し、
       それぞれのスレッドに合致する psyq::any::message::transmitter
       からメッセージパケットを集配する。
    -# それぞれのスレッドに合致する psyq::any::message::transmitter で
       psyq::any::message::transmitter::flush() を定期的に実行すると、
       psyq::any::message::receiver にメッセージパケットが配信され、
       psyq::any::message::receiver::get_functor()
       で取得できるメッセージ受信関数が呼び出される。
 */
#ifndef PSYQ_ANY_MESSAGE_ZONE_HPP_
#define PSYQ_ANY_MESSAGE_ZONE_HPP_

//#include "psyq/any/message/transmitter.hpp"

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief RPCメッセージを送受信する範囲の単位。

    - this_type::equip_transmitter() で、 this_type::transmitter を用意する。
    - this_type::flush() で、
      this_type::transmitter が持つメッセージパケットを集配する。

    @tparam template_base_suite @copydoc psyq::any::message::packet::suite
    @tparam template_allocator  @copydoc psyq::any::message::zone::allocator_type
 */
template<typename template_base_suite, typename template_allocator>
class psyq::any::message::zone
{
    private: typedef zone this_type; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
    /// this_type で使うメモリ割当子。
    public: typedef template_allocator allocator_type;
    /// this_type で使うメッセージ伝送器。
    public: typedef psyq::any::message::transmitter<
        template_base_suite, template_allocator>
            transmitter;
    /// this_type で使うメッセージ受信器。
    public: typedef typename this_type::transmitter::receiver receiver;
    /// @copydoc transmitters_
    private: typedef std
        ::vector<typename this_type::transmitter::weak_ptr, template_allocator>
            weak_transmitter_container;

    //-------------------------------------------------------------------------
    /// @name this_type の構築
    //@{
    /** @brief this_type を構築する。
        @param[in] in_allocator *thisが使うメモリ割当子の初期値。
     */
    public: explicit zone(
        typename this_type::allocator_type const& in_allocator =
            template_allocator())
    :
        transmitters_(in_allocator),
        import_packets_(in_allocator),
        export_packets_(in_allocator)
    {}
    //@}
    /// コピー構築子は使用禁止。
    private: zone(this_type const&);
    /// コピー代入演算子は使用禁止。
    private: this_type& operator=(this_type const&);

    //-------------------------------------------------------------------------
    /// @name this_type::transmitter の取得
    //@{
    /** @brief このスレッドに合致する this_type::transmitter を用意する。

        - スレッドに合致する this_type::transmitter を検索する。
          存在しないなら、動的メモリ確保して生成する。
        - this_type::lock_ でロックの獲得を待ってから実行される。
          ロックを獲得している他のスレッドからブロックされることに注意。
        - 用意した this_type::transmitter は this_type::transmitter::weak_ptr
          で監視しているだけで、 this_type では所有権を持たない。
          this_type::transmitter の所有権は、ユーザーが管理すること。

        @return スレッドに合致する this_type::transmitter 。
     */
    public: typename this_type::transmitter::shared_ptr equip_transmitter()
    {
        return this->equip_transmitter(std::this_thread::get_id());
    }

    /** @brief スレッドに合致する this_type::transmitter を用意する。

        @copydetails equip_transmitter()
        @param[in] in_thread_id スレッド識別子。
     */
    public: typename this_type::transmitter::shared_ptr equip_transmitter(
        std::thread::id const& in_thread_id)
    {
        std::lock_guard<psyq::spinlock> local_lock(this->lock_);
        auto const local_transmitter(
            this_type::find_transmitter(this->transmitters_, in_thread_id));
        if (local_transmitter.get() != nullptr)
        {
            return local_transmitter;
        }
        typename this_type::receiver::tag::key const local_message_address(0);
        return this_type::make_transmitter(
            this->transmitters_, in_thread_id, local_message_address);
    }
    //@}
    /** @brief this_type::transmitter をコンテナから検索する。
        @return
            検索した this_type::transmitter の保持子。
            見つからなかった場合、保持子は空となる。
        @param[in] in_transmitters this_type::transmitter を検索するコンテナ。
        @param[in] in_thread_id
            検索する this_type::transmitter に対応するスレッドの識別子。
     */
    private: static typename this_type::transmitter::shared_ptr find_transmitter(
        typename this_type::weak_transmitter_container const& in_transmitters,
        std::thread::id const& in_thread_id)
    {
        for (auto const& local_observer: in_transmitters)
        {
            auto const local_holder(local_observer.lock());
            auto const local_transmitter(local_holder.get());
            if (local_transmitter != nullptr
                && local_transmitter->get_thread_id() == in_thread_id)
            {
                return local_holder;
            }
        }
        return typename this_type::transmitter::shared_ptr();
    }

    /** @brief this_type::transmitter を生成し、コンテナに追加する。
        @return
            生成した this_type::transmitter の保持子。
            生成に失敗した場合、保持子は空となる。
        @param[in,out] io_transmitters
            生成した this_type::transmitter を追加するコンテナ。
        @param[in] in_thread_id
            生成する this_type::transmitter に対応するスレッドの識別子。
        @param[in] in_mesasge_address
            生成する this_type::transmitter に与えるメッセージ送受信アドレス。
     */
    private: static typename this_type::transmitter::shared_ptr make_transmitter(
        typename this_type::weak_transmitter_container& io_transmitters,
        std::thread::id const& in_thread_id,
        typename this_type::receiver::tag::key const in_mesasge_address)
    {
        // this_type::transmitter に割り当てるメモリを確保する。
        typename this_type::transmitter::allocator_type::template
            rebind<typename this_type::transmitter>::other
                local_allocator(io_transmitters.get_allocator());
        void* const local_storage(local_allocator.allocate(1));
        if (local_storage == nullptr)
        {
            PSYQ_ASSERT(false);
            return typename this_type::transmitter::shared_ptr();
        }

        // this_type::transmitter を構築する。
        typename this_type::transmitter::shared_ptr const local_transmitter(
            new(local_storage) typename this_type::transmitter(
                in_thread_id, in_mesasge_address, local_allocator),
            [local_allocator](typename this_type::transmitter* const io_transmitter)
            {
                auto local_deallocator(local_allocator);
                local_deallocator.destroy(io_transmitter);
                local_deallocator.deallocate(io_transmitter, 1);
            },
            local_allocator);
        if (local_transmitter.get() != nullptr)
        {
            io_transmitters.push_back(local_transmitter);
        }
        else
        {
            PSYQ_ASSERT(false);
        }
        return local_transmitter;
    }

    //-------------------------------------------------------------------------
    /// @name メッセージパケットの送受信
    //@{
    /** @brief this_type::transmitter が持つメッセージパケットを集配する。

        - this_type::transmitter::flush() とこの関数を定期的に実行し、
          メッセージパケットを循環させること。
        - this_type::lock_ でロックの獲得を待ってから実行される。
          ロックを獲得している他のスレッドからブロックされることに注意。
     */
    public: void flush()
    {
        std::lock_guard<psyq::spinlock> local_lock(this->lock_);
        this_type::trade_packet_container(
            this->transmitters_, this->export_packets_, this->import_packets_);
        this_type::swap_packet_container(
            this->export_packets_, this->import_packets_);
    }
    //@}
    /** @brief this_type::transmitter が持つメッセージパケットを集配する。
        @param[in,out] io_transmitters
            メッセージパケットを集配する this_type::transmitter のコンテナ。
        @param[in,out] io_export_packets
            this_type::transmitter からメッセージパケットを集めて格納するコンテナ。
        @param[in] in_import_packets
            this_type::transmitter へ配るメッセージパケットのコンテナ。
     */
    private: static void trade_packet_container(
        typename this_type::weak_transmitter_container& io_transmitters,
        typename this_type::transmitter::shared_packet_container& io_export_packets,
        typename this_type::transmitter::shared_packet_container const& in_import_packets)
    {
        auto local_last_iterator(io_transmitters.begin());
        for (auto i(local_last_iterator); i != io_transmitters.end(); ++i)
        {
            auto const local_holder(i->lock());
            auto const local_transmitter(local_holder.get());
            if (local_transmitter != nullptr)
            {
                local_transmitter->trade_packet_container(
                    io_export_packets, in_import_packets);
                i->swap(*local_last_iterator);
                ++local_last_iterator;
            }
        }
        io_transmitters.erase(local_last_iterator, io_transmitters.end());
    }

    /** @brief 集配されたメッセージパケットのコンテナを交換する。
        @param[in,out] io_export_packets
            this_type::transmitter から集めたメッセージパケットのコンテナ。
        @param[in,out] io_import_packets
            this_type::transmitter へ配ったメッセージパケットのコンテナ。
     */
    private: static void swap_packet_container(
        typename this_type::transmitter::shared_packet_container& io_export_packets,
        typename this_type::transmitter::shared_packet_container& io_import_packets)
    {
        // 配信済コンテナを空にしてから交換する。
        this_type::transmitter::clear_packet_container(
            io_import_packets, io_export_packets.size());
        io_export_packets.swap(io_import_packets);
    }

    //-------------------------------------------------------------------------
    /// @name this_type のプロパティ
    //@{
    /** @brief *thisが使うメモリ割当子を取得する。
        @return *thisが使うメモリ割当子。
     */
    public: PSYQ_CONSTEXPR typename this_type::allocator_type get_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->transmitters_.get_allocator();
    }
    //@}
    //-------------------------------------------------------------------------
    /// 各スレッドに合致する this_type::transmitter のコンテナ。
    private: typename this_type::weak_transmitter_container transmitters_;
    /// this_type::transmitter から輸入したメッセージパケットのコンテナ。
    private: typename this_type::transmitter::shared_packet_container import_packets_;
    /// this_type::transmitter へ輸出するメッセージパケットのコンテナ。
    private: typename this_type::transmitter::shared_packet_container export_packets_;
    /// 排他的処理に使うロックオブジェクト。
    private: psyq::spinlock lock_;

}; // class psyq::any::message::zone

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    namespace test
    {
        inline void any_message()
        {
            psyq::any::rtti::make<psyq::test::floating_wrapper>();

            typedef psyq::any::message::zone<> message_zone;
            message_zone local_zone;
            auto const local_transmitter_holder(local_zone.equip_transmitter());
            auto& local_transmitter(*local_transmitter_holder);
            message_zone::receiver::shared_ptr const local_method_a(
                new message_zone::receiver(
                    [](message_zone::receiver::packet const& in_packet)
                    {
                        PSYQ_ASSERT(
                            in_packet.get_parameter_rtti()
                            == psyq::any::rtti::find<void>());
                    },
                    local_transmitter.get_message_address()));
            double const local_double(0.5);
            message_zone::receiver::shared_ptr const local_method_b(
                new message_zone::receiver(
                    [local_double]
                    (message_zone::receiver::packet const& in_packet)
                    {
                        auto const local_parameter(
                            in_packet.get_parameter<psyq::test::floating_wrapper>());
                        if (local_parameter != nullptr)
                        {
                            PSYQ_ASSERT(local_parameter->value == local_double);
                        }
                        else
                        {
                            PSYQ_ASSERT(false);
                        }
                    },
                    local_transmitter.get_message_address()));

            enum: message_zone::receiver::call::key
            {
                METHOD_PARAMETER_VOID = 1,
                METHOD_PARAMETER_DOUBLE,
            };
            local_transmitter.register_receiver(
                local_method_a, METHOD_PARAMETER_VOID);
            local_transmitter.register_receiver(
                local_method_b, METHOD_PARAMETER_DOUBLE);
            local_transmitter.send_local_message(
                local_transmitter.make_receiver_tag(
                    local_transmitter.get_message_address()),
                message_zone::receiver::call(METHOD_PARAMETER_VOID));
            local_transmitter.send_local_message(
                local_transmitter.make_receiver_tag(
                    local_transmitter.get_message_address()),
                message_zone::receiver::call(METHOD_PARAMETER_DOUBLE),
                floating_wrapper(0.5));
            local_transmitter.post_message(
                local_transmitter.make_receiver_tag(
                    local_transmitter.get_message_address()),
                message_zone::receiver::call(METHOD_PARAMETER_VOID));
            local_transmitter.post_zonal_message(
                local_transmitter.make_receiver_tag(
                    local_transmitter.get_message_address()),
                message_zone::receiver::call(METHOD_PARAMETER_DOUBLE),
                floating_wrapper(0.5));
            local_zone.flush();
            local_transmitter.flush();
            local_zone.flush();
            local_transmitter.flush();
            local_transmitter.unregister_receiver(nullptr);
            local_transmitter.unregister_receiver(nullptr, 0);
        }
    }
}

#endif // !defined(PSYQ_ANY_MESSAGE_ZONE_HPP_)
