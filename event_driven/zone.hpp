/// @file
/// @author Hillco Psychi (https://twitter.com/psychi)
/// @namespace psyq::event_driven
/// @brief 任意の引数を持つメッセージを送信できる、イベント駆動フレームワーク。
/// @par 初期化の手順
///   -# psyq::event_driven::zone インスタンスを用意する。
///   -# psyq::event_driven::zone::equip_dispatcher で、スレッドごとに
///      psyq::event_driven::zone::dispatcher インスタンスを用意する。
///   -# 現在のスレッドに適合する
///      psyq::event_driven::zone::dispatcher インスタンスから
///      psyq::event_driven::zone::dispatcher::register_receiver
///      を呼び出し、メッセージ受信関数を登録する。
/// @par メッセージ送受信の手順
///   -# メッセージループをまわしておく。
///      - psyq::event_driven::zone::dispatch
///        を、メインスレッドから定期的に呼び出す。
///      - それぞれのスレッドに適合する
///        psyq::event_driven::zone::dispatcher インスタンスから、
///        psyq::event_driven::zone::dispatcher::dispatch を定期的に呼び出す。
///   -# 現在のスレッドに適合する psyq::event_driven::zone::dispatcher
///      インスタンスから以下のいずれかの関数を呼び出し、メッセージを送信する。
///      - メッセージゾーンの内と外にメッセージを送るなら、
///        psyq::event_driven::zone::post_external を呼び出す。
///      - メッセージゾーン内にのみメッセージを送るなら、
///        psyq::event_driven::zone::post_zonal を呼び出す。
///      - 構築済みのメッセージパケットを送るなら、
///        psyq::event_driven::zone::post を呼び出す。
///   -# psyq::event_driven::zone::dispatcher::register_receiver
///      から登録したメッセージ受信関数が呼び出される。
#ifndef PSYQ_EVENT_DRIVEN_ZONE_HPP_
#define PSYQ_EVENT_DRIVEN_ZONE_HPP_

#ifdef _MSC_VER
#include <eh.h>
#endif // defined(_MSC_VER)
#include <vector>
#include <thread>
#include <mutex>
#include "./message.hpp"
#include "./packet.hpp"
#include "./dispatcher.hpp"

#ifndef PSYQ_EVENT_DRIVEN_ZONE_PACKET_DEFAULT
#define PSYQ_EVENT_DRIVEN_ZONE_PACKET_DEFAULT\
    psyq::event_driven::packet<\
        psyq::event_driven::message<std::uint32_t, std::uint32_t>>
#endif // !defined(PSYQ_EVENT_DRIVEN_ZONE_PACKET_DEFAULT)

#ifndef PSYQ_EVENT_DRIVEN_ZONE_PRIORITY_DEFAULT
#define PSYQ_EVENT_DRIVEN_ZONE_PRIORITY_DEFAULT std::int32_t
#endif // !defined(PSYQ_EVENT_DRIVEN_ZONE_PRIORITY_DEFAULT)

#ifndef PSYQ_EVENT_DRIVEN_ZONE_ALLOCATOR_DEFAULT
#define PSYQ_EVENT_DRIVEN_ZONE_ALLOCATOR_DEFAULT std::allocator<void*>
#endif // !defined(PSYQ_EVENT_DRIVEN_ZONE_ALLOCATOR_DEFAULT)

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// @brief メッセージを送受信する範囲の単位。
/// @par 使い方の概略
///   - zone::equip_dispatcher で、 zone::dispatcher を用意する。
///   - 以下のいずれかの関数で、メッセージを送信する。
///     - zone::post_external
///     - zone::post_zonal
///     - zone::post
///   - zone::dispatch で、メッセージパケットを zone::dispatcher へ配送する。
/// @tparam template_base_message @copydoc zone::message
/// @tparam template_priority     @copydoc dispatcher::function_priority
/// @tparam template_allocator    @copydoc zone::allocator_type
template<
    typename template_packet = PSYQ_EVENT_DRIVEN_ZONE_PACKET_DEFAULT,
    typename template_priority = PSYQ_EVENT_DRIVEN_ZONE_PRIORITY_DEFAULT,
    typename template_allocator = PSYQ_EVENT_DRIVEN_ZONE_ALLOCATOR_DEFAULT>
class psyq::event_driven::zone
{
    /// @copydoc psyq::string::view::this_type
    private: typedef zone this_type;

    //-------------------------------------------------------------------------
    /// @brief this_type で使うメッセージ配送器。
    public: typedef
        psyq::event_driven::dispatcher<
            template_packet, template_priority, template_allocator>
        dispatcher;
    /// @brief 送受信するメッセージパケットの基底型。
    /// @details event_driven::packet 互換のインタフェイスを持つこと。
    public: typedef template_packet packet;
    /// @brief メッセージの基底型。
    public: typedef typename this_type::packet::message message;
    /// @brief メッセージの送り状の型。
    public: typedef typename this_type::message::tag tag;
    /// @brief this_type で使うメモリ割当子。
    public: typedef template_allocator allocator_type;

    //-------------------------------------------------------------------------
    /// @copydoc this_type::dispatchers_
    private: typedef
        std::vector<
            typename this_type::dispatcher::weak_ptr,
            typename this_type::allocator_type>
        dispatcher_weak_ptr_container;

    //-------------------------------------------------------------------------
    /// @brief メッセージゾーンを構築する。
    public: zone(
        /// [in] this_type::dispatcher の予約数。
        std::size_t const in_dispatcher_capacity,
        /// [in] this_type::packet の予約数。
        std::size_t const in_packet_capacity,
        /// [in] *thisが使うメモリ割当子の初期値。
        typename this_type::allocator_type const& in_allocator =
            template_allocator()):
    dispatchers_(in_allocator),
    delivery_packets_(in_allocator)
    {
        this->dispatchers_.reserve(in_dispatcher_capacity);
        this->delivery_packets_.reserve(in_packet_capacity);
    }

    /// @brief メモリ割当子を取得する。
    /// @return *thisが使っているメモリ割当子。
    public: PSYQ_CONSTEXPR typename this_type::allocator_type get_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->dispatchers_.get_allocator();
    }

    //-------------------------------------------------------------------------
    /// @name メッセージの送受信
    /// @{

    /// @brief this_type::dispatcher へメッセージパケットを配送する。
    /// @details
    ///   - この関数と、 this_type::equip_dispatcher からスレッドごとに取得した
    ///     this_type::dispatcher インスタンスの this_type::dispatcher::dispatch
    ///     を定期的に呼び出し、メッセージパケットを循環させること。
    ///   - this_type::lock_ でロックの獲得を待ってから実行される。
    ///     ロックを獲得している他のスレッドからブロックされることに注意。
    public: void dispatch(
        /// [in] メッセージパケットの予約数。
        std::size_t const in_capacity = 0,
        /// [in] メッセージパケットのコンテナを再構築するか。
        bool const in_rebuild = false)
    {
        std::lock_guard<psyq::spinlock> local_lock(this->lock_);
        this_type::deliver_packets(this->dispatchers_, this->delivery_packets_);
        this_type::dispatcher::clear_packets(
            this->delivery_packets_, in_capacity, in_rebuild);
    }

    /// @brief スレッドに合致する this_type::dispatcher を用意する。
    /// @details
    ///   - スレッドに合致する this_type::dispatcher を検索する。
    ///     存在しないなら、動的メモリ割当して生成する。
    ///   - 用意した this_type::dispatcher は、 this_type からは
    ///     this_type::dispatcher::weak_ptr で弱参照しており、所有権を持たない。
    ///     this_type::dispatcher の所有権は、ユーザーが管理すること。
    ///   - this_type::lock_ でロックの獲得を待ってから実行される。
    ///     ロックを獲得している他のスレッドからブロックされることに注意。
    /// .
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
            this_type::create_dispatcher(
                this->dispatchers_,
                in_thread_id,
                this->delivery_packets_.capacity(),
                PSYQ_EVENT_DRIVEN_DISPATCHER_RECEIVER_CAPACITY_DEFFAULT,
                PSYQ_EVENT_DRIVEN_DISPATCHER_FORWARDER_CAPACITY_DEFFAULT);
    }

    /// @brief メッセージゾーンの内と外へのメッセージの送信を予約する。
    /// @details
    ///   - 引数を持たないメッセージパケットを動的メモリ割当して構築し、
    ///     メッセージゾーンの内と外への送信を予約する。
    ///   - この関数では、メッセージ送信の予約のみを行う。
    ///     メッセージを実際に送信する処理は、この関数の呼び出し後、
    ///     this_type::dispatch を呼び出すことで行なわれる。
    ///   - メッセージ受信処理は、メッセージパケットの送信が行われた後、
    ///     dispatcher::dispatch を呼び出すことで行なわれる。
    ///   - 同一スレッドで送信を予約したメッセージの受信順序は、
    ///     送信予約順序と同じになる。
    ///   - this_type::lock_ でロックの獲得を待ってから実行される。
    ///     ロックを獲得している他のスレッドからブロックされることに注意。
    /// .
    /// @sa
    ///   - 構築済のメッセージパケットを送信するには、 this_type::post を使う。
    ///   - メッセージゾーン内にだけメッセージを送信するには、
    ///     this_type::post_zonal を使う。
    ///   - this_type::dispatcher
    ///     に登録されているメッセージ受信関数にのみメッセージを送信するには、
    ///     dispatcher::send_local を使う。
    /// @todo メッセージゾーンの外へ送信する処理は未実装。
    public: void post_external(
        /// [in] 送信するメッセージの送り状。
        typename this_type::tag const& in_tag)
    {
        this->post(
            this_type::packet::create_external(
                typename this_type::packet::message(in_tag),
                this->get_allocator()),
            true);
    }

    /// @copydoc this_type::post_external
    public: template<typename template_parameter>
    void post_external(
        /// [in] 送信するメッセージの送り状。
        typename this_type::tag const& in_tag,
        /// [in] 送信するメッセージの引数。POD型であること。
        template_parameter&& io_parameter)
    {
        this->post(
            this_type::packet::create_external(
                this_type::message::construct(in_tag, std::move(io_parameter)),
                this->get_allocator()),
            true);
    }

    /// @brief メッセージゾーン内へのメッセージの送信を予約する。
    /// @details
    ///   - 引数を持たないメッセージパケットを動的メモリ割当して構築し、
    ///     メッセージゾーン内への送信を予約する。
    ///   - この関数では、メッセージ送信の予約のみを行う。
    ///     メッセージを実際に送信する処理は、この関数の呼び出し後、
    ///     this_type::dispatch を呼び出すことで行なわれる。
    ///   - メッセージ受信処理は、メッセージパケットの送信が行われた後、
    ///     dispatcher::dispatch を呼び出すことで行なわれる。
    ///   - 同一スレッドで送信を予約したメッセージの受信順序は、
    ///     送信予約順序と同じになる。
    ///   - this_type::lock_ でロックの獲得を待ってから実行される。
    ///     ロックを獲得している他のスレッドからブロックされることに注意。
    /// .
    /// @sa
    ///   - 構築済のメッセージパケットを送信するには、 this_type::post を使う。
    ///   - メッセージゾーンの内と外にメッセージを送信するには、
    ///     this_type::post_external を使う。
    ///   - this_type::dispatcher
    ///     に登録されているメッセージ受信関数にのみメッセージを送信するには、
    ///     dispatcher::send_local を使う。
    public: void post_zonal(
        /// [in] 送信するメッセージの送り状。
        typename this_type::tag const& in_tag)
    {
        this->post(
            this_type::packet::create_zonal(
                typename this_type::packet::message(in_tag),
                this->get_allocator()),
            true);
    }

    /// @copydoc this_type::post_zonal
    public: template<typename template_parameter>
    void post_zonal(
        /// [in] 送信するメッセージの送り状。
        typename this_type::tag const& in_tag,
        /// [in] 送信するメッセージの引数。
        template_parameter&& io_parameter)
    {
        this->post(
            this_type::packet::create_zonal(
                this_type::message::construct(in_tag, std::move(io_parameter)),
                this->get_allocator()),
            true);
    }

    /// @brief メッセージの送信を予約する。
    /// @details
    ///   - この関数では、メッセージ送信の予約のみを行う。
    ///     メッセージを実際に送信する処理は、この関数の呼び出し後、
    ///     this_type::dispatch を呼び出すことで行なわれる。
    ///   - メッセージ受信処理は、メッセージパケットの送信が行われた後、
    ///     dispatcher::dispatch を呼び出すことで行なわれる。
    ///   - 同一スレッドで送信を予約したメッセージの受信順序は、
    ///     送信予約順序と同じになる。
    ///   - this_type::lock_ でロックの獲得を待ってから実行される。
    ///     ロックを獲得している他のスレッドからブロックされることに注意。
    /// .
    /// @retval true  成功。メッセージの送信を予約した。
    /// @retval false 失敗。メッセージの送信を予約しなかった。
    public: bool post(
        /// [in] 送信するメッセージを持つメッセージパケット。
        typename this_type::packet::shared_ptr in_packet)
    {
        return this->post(std::move(in_packet), false);
    }
    /// @}
    //-------------------------------------------------------------------------
    /// @brief コピー構築子は使用禁止。
    private: zone(this_type const&);
    /// @brief コピー代入演算子は使用禁止。
    private: this_type& operator=(this_type const&);

    /// @copydoc this_type::post
    private: bool post(
        /// [in] 送信するメッセージを持つメッセージパケット。
        typename this_type::packet::shared_ptr&& io_packet,
        /// [in] 失敗した場合にassertするかどうか。
        bool const in_assert)
    {
        if (io_packet.get() == nullptr)
        {
            PSYQ_ASSERT(!in_assert);
            return false;
        }
        std::lock_guard<psyq::spinlock> local_lock(this->lock_);
        this->delivery_packets_.emplace_back(std::move(io_packet));
        return true;
    }

    /// @brief this_type::dispatcher をコンテナから検索する。
    /// @return
    ///   検索した this_type::dispatcher を強参照するスマートポインタ。
    ///   見つからなかった場合、スマートポインタは空となる。
    private: static typename this_type::dispatcher::shared_ptr find_dispatcher(
        /// [in] 検索する this_type::dispatcher のコンテナ。
        typename this_type::dispatcher_weak_ptr_container const& in_dispatchers,
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
    private: static typename this_type::dispatcher::shared_ptr create_dispatcher(
        /// [in,out] 生成した this_type::dispatcher を追加するコンテナ。
        typename this_type::dispatcher_weak_ptr_container& io_dispatchers,
        /// [in] 生成する this_type::dispatcher に対応するスレッドの識別子。
        std::thread::id const& in_thread_id,
        /// [in] メッセージパケットの予約数。
        std::size_t const in_packet_capacity,
        /// [in] メッセージ受信関数の予約数。
        std::size_t const in_receiver_capacity,
        /// [in] メッセージ転送関数の予約数。
        std::size_t const in_forwarder_capacity)
    {
        // this_type::dispatcher を構築する。
        auto const local_allocator(io_dispatchers.get_allocator());
        auto const local_dispatcher(
            std::allocate_shared<typename this_type::dispatcher>(
                local_allocator,
                in_thread_id,
                in_packet_capacity,
                in_receiver_capacity,
                in_forwarder_capacity,
                local_allocator));
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

    /// @brief this_type::dispatcher へメッセージパケットを配送する。
    private: static void deliver_packets(
        /// [in,out] メッセージパケットを受信する this_type::dispatcher のコンテナ。
        typename this_type::dispatcher_weak_ptr_container& io_dispatchers,
        /// [in] this_type::dispatcher へ配送するメッセージパケットのコンテナ。
        typename this_type::dispatcher::packet_shared_ptr_container const&
            in_packets)
    {
        auto local_last(std::begin(io_dispatchers));
        auto const local_end(std::end(io_dispatchers));
        for (auto i(local_last); i != local_end; ++i)
        {
            auto const local_holder(i->lock());
            auto const local_dispatcher(local_holder.get());
            if (local_dispatcher != nullptr)
            {
                local_dispatcher->receive_packets(in_packets);
                i->swap(*local_last);
                ++local_last;
            }
        }
        io_dispatchers.erase(local_last, local_end);
    }

    //-------------------------------------------------------------------------
    /// @brief 各スレッドに合致する this_type::dispatcher のコンテナ。
    private: typename this_type::dispatcher_weak_ptr_container dispatchers_;
    /// @brief this_type::dispatcher へ配るメッセージパケットのコンテナ。
    private: typename this_type::dispatcher::packet_shared_ptr_container
        delivery_packets_;
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

        typedef psyq::event_driven::zone<> message_zone;
        message_zone local_zone(0, 0);
        auto const local_dispatcher_holder(local_zone.equip_dispatcher());
        auto& local_dispatcher(*local_dispatcher_holder);
        auto const local_method_a(
            std::allocate_shared<message_zone::dispatcher::function>(
                local_zone.get_allocator(),
                [](message_zone::packet const& in_packet)
                {
                    PSYQ_ASSERT(
                        in_packet.get_parameter_rtti()
                        == psyq::any::rtti::find<void>());
                }));
        double const local_double(0.5);
        auto const local_method_b(
            std::allocate_shared<message_zone::dispatcher::function>(
                local_zone.get_allocator(),
                [local_double](message_zone::packet const& in_packet)
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
        enum: message_zone::tag::key_type
        {
            SENDER_KEY = 20,
            RECEIVER_KEY = 10,
            METHOD_PARAMETER_VOID = 1,
            METHOD_PARAMETER_DOUBLE,
        };
        local_dispatcher.register_receiver(
            RECEIVER_KEY, METHOD_PARAMETER_VOID, local_method_a);
        local_dispatcher.register_receiver(
            RECEIVER_KEY, METHOD_PARAMETER_DOUBLE, local_method_b);
        /*
        local_dispatcher.send_local(
            message_zone::tag(
                SENDER_KEY, RECEIVER_KEY, METHOD_PARAMETER_VOID));
        local_dispatcher.send_local(
            message_zone::tag(
                SENDER_KEY, RECEIVER_KEY, METHOD_PARAMETER_DOUBLE),
            psyq_test::floating_wrapper(local_double));
         */
        local_zone.post_external(
            message_zone::tag(
                SENDER_KEY, RECEIVER_KEY, METHOD_PARAMETER_VOID));
        local_zone.post_zonal(
            message_zone::tag(
                SENDER_KEY, RECEIVER_KEY, METHOD_PARAMETER_DOUBLE),
            psyq_test::floating_wrapper(local_double));
        local_zone.dispatch();
        local_dispatcher.dispatch();
        local_zone.dispatch();
        local_dispatcher.dispatch();
        local_dispatcher.unregister_receiver(
            RECEIVER_KEY, METHOD_PARAMETER_VOID);
        PSYQ_ASSERT(
            local_dispatcher.unregister_receiver(RECEIVER_KEY) == 1);
    }
}

#endif // !defined(PSYQ_EVENT_DRIVEN_ZONE_HPP_)
// vim: set expandtab:
