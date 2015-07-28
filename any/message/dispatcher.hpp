/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_ANY_MESSAGE_DISPATCHER_HPP_
#define PSYQ_ANY_MESSAGE_DISPATCHER_HPP_

#ifdef _MSC_VER
#include <eh.h>
#endif // _MSC_VER
#include <vector>
#include <thread>
#include <mutex>
#include "../../atomic_count.hpp"
#include "./tag.hpp"
#include "./suite.hpp"
#include "./packet.hpp"

/// @cond
namespace psyq
{
    namespace any
    {
        namespace message
        {
            template<typename> class invoice;
            template<
                typename = psyq::any::message::suite<std::uint32_t, std::uint32_t, std::uint32_t>,
                typename = std::allocator<void*>>
                    class zone;
            template<typename, typename, typename> class dispatcher;
        } // namespace message
    } // namespace any
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief RPCメッセージの送り状。
    @tparam template_key @copydoc psyq::any::message::invoice::key
 */
template<typename template_key>
class psyq::any::message::invoice
{
    private: typedef invoice this_type; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
    /// @brief メッセージ送り状が使う識別値を表す型。
    public: typedef template_key key;
    static_assert(
        std::is_integral<template_key>::value,
        "'template_key' is not integral type.");

    //-------------------------------------------------------------------------
    /** @brief メッセージの送り状を構築する。
        @param[in] in_sender_key    @copydoc sender_key_
        @param[in] in_receiver_key  @copydoc receiver_key_
        @param[in] in_receiver_mask @copydoc receiver_mask_
        @param[in] in_method_key    @copydoc method_key_
        @param[in] in_method_mask   @copydoc method_mask_
     */
    public: PSYQ_CONSTEXPR invoice(
        typename this_type::key const in_sender_key,
        typename this_type::key const in_receiver_key,
        typename this_type::key const in_receiver_mask,
        typename this_type::key const in_method_key,
        typename this_type::key const in_method_mask)
    PSYQ_NOEXCEPT:
    sender_key_(in_sender_key),
    receiver_key_(in_receiver_key),
    receiver_mask_(in_receiver_mask),
    method_key_(in_method_key),
    method_mask_(in_method_mask)
    {}

    //-------------------------------------------------------------------------
    /** @brief メッセージ送信元オブジェクトの識別値を取得する。
        @return @copydoc sender_key_
     */
    public: PSYQ_CONSTEXPR typename this_type::key get_sender_key()
    const PSYQ_NOEXCEPT
    {
        return this->sender_key_;
    }

    /** @brief メッセージ受信先オブジェクトの識別値を取得する。
        @return @copydoc receiver_key_
     */
    public: PSYQ_CONSTEXPR typename this_type::key get_receiver_key()
    const PSYQ_NOEXCEPT
    {
        return this->receiver_key_;
    }

    /** @brief メッセージ受信先オブジェクトのマスクを取得する。
        @return receiver_mask_
     */
    public: PSYQ_CONSTEXPR typename this_type::key get_receiver_mask()
    const PSYQ_NOEXCEPT
    {
        return this->receiver_mask_;
    }

    /** @brief メッセージ呼出メソッドの識別値を取得する。
        @return @copydoc receiver_key_
     */
    public: PSYQ_CONSTEXPR typename this_type::key get_method_key()
    const PSYQ_NOEXCEPT
    {
        return this->method_key_;
    }

    /** @brief メッセージ呼出メソッドのマスクを取得する。
        @return receiver_mask_
     */
    public: PSYQ_CONSTEXPR typename this_type::key get_method_mask()
    const PSYQ_NOEXCEPT
    {
        return this->method_mask_;
    }

    /** @brief メッセージ受信オブジェクトに該当するか判定する。
        @param[in] in_key 判定するメッセージ受信オブジェクトの識別値。
        @retval true  メッセージ受信オブジェクトに該当する。
        @retval false メッセージ受信オブジェクトに該当しない。
     */
    public: PSYQ_CONSTEXPR bool agree_receiver_key(
        typename this_type::key const in_key)
    const PSYQ_NOEXCEPT
    {
        return this_type::agree_key(
            in_key, this->get_receiver_key(), this->get_receiver_mask());
    }

    /** @brief メッセージ呼出メソッドに該当するか判定する。
        @param[in] in_key 判定するメッセージ呼出メソッドの識別値。
        @retval true  メッセージ呼出メソッドに該当する。
        @retval false メッセージ呼出メソッドに該当しない。
     */
    public: PSYQ_CONSTEXPR bool agree_method_key(
        typename this_type::key const in_key)
    const PSYQ_NOEXCEPT
    {
        return this_type::agree_key(
            in_key, this->get_method_key(), this->get_method_mask());
    }

    private: static PSYQ_CONSTEXPR bool agree_key(
        typename this_type::key const in_key,
        typename this_type::key const in_base_key,
        typename this_type::key const in_base_mask)
    {
        return (in_key & in_base_mask) == in_base_key;
    }

    //-------------------------------------------------------------------------
    /// @brief メッセージ送信元オブジェクトの識別値。
    private: typename this_type::key sender_key_;
    /// @brief メッセージ受信先オブジェクトの識別値。
    private: typename this_type::key receiver_key_;
    /// @brief メッセージ受信先オブジェクトのマスク。
    private: typename this_type::key receiver_mask_;
    /// @brief メッセージ呼出メソッドの識別値。
    private: typename this_type::key method_key_;
    /// @brief メッセージ呼出メソッドのマスク。
    private: typename this_type::key method_mask_;

}; // class psyq::any::message::invoice

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief スレッド別のRPCメッセージ伝送器。

    - psyq::any::message::zone::equip_dispatcher で、 this_type を用意する。
    - this_type::insert_function で、メッセージ受信関数を登録する。
    - this_type::post_message で、メッセージパケットを送信する。
    - psyq::any::message::zone::flush で、
      this_type が持つメッセージパケットが集配される。
    - this_type::flush で、 this_type が持つメッセージパケットを
      メッセージ受信関数へ配信する。

    @tparam template_base_suite @copydoc psyq::any::message::packet::suite
    @tparam template_priority   @copydoc this_type::priority
    @tparam template_allocator  @copydoc this_type::allocator_type
 */
template<
    typename template_base_suite,
    typename template_priority,
    typename template_allocator>
class psyq::any::message::dispatcher
{
    private: typedef dispatcher this_type; ///< thisが指す値の型。

    //-------------------------------------------------------------------------
    /// @brief メッセージ一式を保持するパケットの基底型。
    public: typedef psyq::any::message::packet<template_base_suite> packet;

    /// @brief メッセージの送り状。
    public: typedef
        psyq::any::message::invoice<typename template_base_suite::tag::key>
        invoice;

    /// @brief メッセージの優先順位。
    public: typedef template_priority priority;

    /// @brief this_type で使うメモリ割当子。
    public: typedef template_allocator allocator_type;

    /// @brief メッセージ受信関数。
    public: typedef
        std::function<void(typename this_type::packet const&)>
        function;

    /// @brief メッセージ受信関数の所有権ありスマートポインタの型。
    public: typedef
        std::shared_ptr<typename this_type::function>
        function_shared_ptr;

    /// @brief メッセージ受信関数の所有権なしスマートポインタの型。
    public: typedef
        std::weak_ptr<typename this_type::function>
        function_weak_ptr;

    private: typedef
        std::vector<
            typename this_type::function_shared_ptr,
            typename this_type::allocator_type>
        function_shared_ptr_container;

    /// this_type::packet 保持子のコンテナ。
    private: typedef
        std::vector<
            typename this_type::packet::shared_ptr,
            typename this_type::allocator_type>
        packet_shared_ptr_container;

    //-------------------------------------------------------------------------
    /// @brief メッセージ受信フック。
    private: class hook
    {
        private: typedef hook this_type;

        public: struct method_less
        {
            bool operator()(
                typename dispatcher::hook const& in_left,
                typename dispatcher::hook const& in_right)
            const PSYQ_NOEXCEPT
            {
                return in_left.method_key_ != in_right.method_key_?
                    in_left.method_key_ < in_right.method_key_:
                    in_left.priority_ < in_right.priority_;
            }

            bool operator()(
                typename dispatcher::hook const& in_left,
                typename dispatcher::invoice::key const& in_right)
            const PSYQ_NOEXCEPT
            {
                return in_left.method_key_ < in_right;
            }

            bool operator()(
                typename dispatcher::invoice::key const& in_left,
                typename dispatcher::hook const& in_right)
            const PSYQ_NOEXCEPT
            {
                return in_left < in_right.method_key_;
            }
        };

        public: typedef
            std::vector<this_type, typename dispatcher::allocator_type>
            container;

        public: hook(
            typename dispatcher::invoice::key in_receiver_key,
            typename dispatcher::invoice::key in_method_key,
            typename dispatcher::priority in_priority,
            typename dispatcher::function_weak_ptr in_function):
        function_(std::move(in_function)),
        receiver_key_(std::move(in_receiver_key)),
        method_key_(std::move(in_method_key)),
        priority_(std::move(in_priority))
        {}

#ifdef PSYQ_NO_STD_DEFAULTED_FUNCTION
        /** @brief ムーブ構築子。
            @param[in,out] io_source ムーブ元となるインスタンス。
         */
        public: hook(this_type&& io_source):
        function_(std::move(io_source.function_)),
        receiver_key_(std::move(io_source.receiver_key_)),
        method_key_(std::move(io_source.method_key_)),
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
            this->method_key_ = std::move(io_source.method_key_);
            this->priority_ = std::move(io_source.priority_);
            return *this;
        }
#endif // !defined(PSYQ_NO_STD_DEFAULTED_FUNCTION)

        public: typename dispatcher::function_weak_ptr function_;
        public: typename dispatcher::invoice::key receiver_key_;
        public: typename dispatcher::invoice::key method_key_;
        public: typename dispatcher::priority priority_;

    }; // class hook

    //-------------------------------------------------------------------------
    /** @brief *thisが使うメモリ割当子を取得する。
        @return *thisが使うメモリ割当子。
     */
    public: typename this_type::allocator_type get_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->hooks_.get_allocator();
    }

    /** @brief *thisに合致するスレッドの識別子を取得する。
        @return *thisに合致するスレッドの識別子。
     */
    public: std::thread::id const& get_thread_id() const PSYQ_NOEXCEPT
    {
        return this->thread_id_;
    }

    //-------------------------------------------------------------------------
    /** @name メッセージ受信関数
        @{
     */
    /** @brief メッセージ受信関数を挿入する。
        @param[in] in_receiver_key メッセージ受信オブジェクトの識別値。
        @param[in] in_method_key   メッセージ受信メソッドの識別値。
        @param[in] in_priority     メッセージを受信する優先順位。
        @param[in] in_function     挿入するメッセージ受信関数。
        @retval true  成功。
        @retval false 失敗。
     */
    public: bool insert_function(
        typename this_type::invoice::key const in_receiver_key,
        typename this_type::invoice::key const in_method_key,
        typename this_type::priority const in_priority,
        typename this_type::function_shared_ptr const& in_function)
    {
        if (in_function.get() == nullptr || !this->agree_this_thread())
        {
            return false;
        }
        std::lock_guard<psyq::spinlock> const local_lock(this->lock_);
        auto const local_end(this->hooks_.end());
        auto local_lower_bound(
            std::lower_bound(
                this->hooks_.begin(),
                local_end,
                in_method_key,
                typename this_type::hook::method_less()));
        for (auto i(local_lower_bound); i != local_end;)
        {
            auto& local_hook(*i);
            if (local_hook.method_key_ != in_method_key)
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
        this->hooks_.emplace(
            local_lower_bound,
            in_receiver_key,
            in_method_key,
            in_priority,
            in_function);
        return true;
    }

    /** @brief メッセージ受信関数を削除する。
        @param[in] in_receiver_key
            削除するメッセージ受信関数に対応する
            メッセージ受信オブジェクトの識別値。
        @return 削除したメッセージ受信関数の数。
     */
    public: std::size_t remove_function(
        typename this_type::invoice::key const in_receiver_key)
    {
        std::lock_guard<psyq::spinlock> const local_lock(this->lock_);
        std::size_t local_count(0);
        for (auto& local_hook: this->hooks_)
        {
            if (local_hook.receiver_key_ == in_receiver_key 
                && !local_hook.function_.expired())
            {
                ++local_count;
                local_hook.function_.reset();
            }
        }
        return local_count;
    }

    /** @brief メッセージ受信関数を削除する。
        @param[in] in_receiver_key
            削除するメッセージ受信関数に対応する
            メッセージ受信オブジェクトの識別値。
        @param[in] in_method_key
            削除するメッセージ受信関数に対応する
            メッセージ呼出メソッドの識別値。
        @return
            削除したメッセージ受信関数を指すスマートポインタ。
            メッセージ受信関数を削除しなかった場合は、空となる。
     */
    public: typename this_type::function_weak_ptr remove_function(
        typename this_type::invoice::key const in_receiver_key,
        typename this_type::invoice::key const in_method_key)
    {
        std::lock_guard<psyq::spinlock> const local_lock(this->lock_);
        auto const local_end(this->hooks_.end());
        auto const local_find(
            this_type::find_hook_iterator(
                this->hooks_.begin(),
                local_end,
                in_receiver_key,
                in_method_key));
        if (local_find == local_end)
        {
            return typename this_type::function_weak_ptr();
        }
        auto const local_function(std::move(local_find->function_));
        local_find->function_.reset();
        return local_function;
    }

    /** @brief メッセージ受信関数を検索する。
        @param[in] in_receiver_key
            検索するメッセージ受信関数に対応する
            メッセージ受信オブジェクトの識別値。
        @param[in] in_method_key
            検索するメッセージ受信関数に対応する
            メッセージ呼出メソッドの識別値。
        @return
            発見したメッセージ受信関数を指すスマートポインタ。
            該当するメッセージ受信関数が見つからなかった場合は、空となる。
     */
    public: typename this_type::function_weak_ptr find_function(
        typename this_type::invoice::key const in_receiver_key,
        typename this_type::invoice::key const in_method_key)
    const PSYQ_NOEXCEPT
    {
        std::lock_guard<psyq::spinlock> const local_lock(this->lock_);
        auto const local_end(this->hooks_.end());
        auto const local_find(
            this_type::find_hook_iterator(
                this->hooks_.begin(),
                local_end,
                in_receiver_key,
                in_method_key));
        if (local_find == local_end)
        {
            return typename this_type::function_weak_ptr();
        }
        return local_find->function_;
    }
    /// @}
    //-------------------------------------------------------------------------
    /** @name メッセージパケットの送受信
        @{
     */
    /** @interface interface_post_message
        @brief post_message と post_zonal_message に共通する説明。

        - this_type::get_thread_id と合致しないスレッドで実行すると失敗する。
        - この関数では、メッセージパケットの送信の予約のみを行う。
          実際のメッセージパケット送信処理は、この関数の実行後、
          psyq::any::message::zone::flush / this_type::flush
          の順に実行することで行なわれる。
        - メッセージパケットの受信処理は、送信処理が行われた後、
          psyq::any::message::zone::flush / this_type::flush
          の順に実行することで行なわれる。
        - 同一スレッドで送信を予約したメッセージパケットの受信順序は、
          送信予約順序と同じになる。
     */
    /** @brief
        引数を持たないメッセージパケットを動的メモリ割当して構築し、
        メッセージゾーンの内と外への送信を予約する。

        @copydetails interface_post_message

        @sa メッセージゾーン内にだけメッセージパケットを送信するには、
            this_type::post_zonal_message を使う。
        @sa *thisに登録されているメッセージ受信フックにのみメッセージパケットを送信するには、
            this_type::send_local_message を使う。

        @retval true  成功。メッセージパケットの送信を予約した。
        @retval false 失敗。メッセージパケットの送信を予約しなかった。

        @param[in] in_invoice 送信するメッセージの送り状。
        @todo ゾーンの外へ送信する処理は未実装。
     */
    public: bool post_message(typename this_type::invoice const& in_invoice)
    {
        return this->add_export_packet(
            this_type::create_external_packet(
                typename this_type::packet::suite(in_invoice),
                this->get_allocator()));
    }

    /** @brief
        POD型の引数を持つメッセージパケットを動的メモリ割当して構築し、
        メッセージゾーンの内と外への送信を予約する。

        @copydetails post_message

        @param[in] in_parameter 送信するメッセージの引数。必ずPOD型。
        @todo 未実装。
     */
    public: template<typename template_parameter>
    bool post_message(
        typename this_type::invoice const& in_invoice,
        template_parameter in_parameter);

    /** @brief
        引数を持たないメッセージパケットを動的メモリ割当して構築し、
        メッセージゾーン内への送信を予約する。

        @copydetails interface_post_message

        @sa メッセージゾーンの内と外にメッセージパケットを送信するには、
            this_type::post_message を使う。
        @sa *thisに登録されているメッセージ受信フックにのみメッセージパケットを送信するには、
            this_type::send_local_message を使う。

        @retval true  成功。メッセージ送信を予約した。
        @retval false 失敗。メッセージ送信を予約しなかった。

        @param[in] in_invoice 送信するメッセージの送り状。
     */
    public: bool post_zonal_message(
        typename this_type::invoice const& in_invoice)
    {
        return this->add_export_packet(
            this_type::create_zonal_packet(
                typename this_type::packet::suite(in_invoice),
                this->get_allocator()));
    }

    /** @brief
        任意型の引数を持つメッセージパケットを動的メモリ割当して構築し、
        メッセージゾーン内への送信を予約する。

        @copydetails post_zonal_message
        @param[in] in_parameter 送信するメッセージの引数。
     */
    public: template<typename template_parameter>
    bool post_zonal_message(
        typename this_type::invoice const& in_invoice,
        template_parameter in_parameter)
    {
        typedef
            typename this_type::packet::suite::template
                parametric<template_parameter>
            suite;
        return this->add_export_packet(
            this_type::create_zonal_packet(
                suite(in_invoice, std::move(in_parameter)),
                this->get_allocator()));
    }

    /** @interface interface_send_local_message
        @brief send_local_message に共通する説明。

        - this_type::get_thread_id と合致しないスレッドで実行すると失敗する。
        - メッセージパケットを送信し、受信関数の終了までブロックする。

        @sa *thisにメッセージ受信関数を登録するには、
            this_type::insert_function を使う。
        @sa メッセージゾーンの内と外にメッセージパケットを送信するには、
            this_type::post_message を使う。
        @sa メッセージゾーン内にだけメッセージパケットを送信するには、
            this_type::post_zonal_message を使う。

        @retval true  成功。メッセージパケットを送信した。
        @retval false 失敗。メッセージパケットを送信しなかった。
     */
    /** @brief
        メッセージパケットを、*thisに登録されたメッセージ受信関数にだけ送信する。

        @copydetails interface_send_local_message

        @param[in] in_packet 送信するメッセージパケット。
     */
    public: bool send_local_message(
        typename this_type::packet const& in_packet)
    {
        if (!this->agree_this_thread())
        {
            return false;
        }
        this_type::deliver_packet(
            this->function_caches_, this->hooks_, in_packet);
        return true;
    }

    /** @brief
        引数を持たないメッセージパケットを、
        *thisに登録されたメッセージ受信関数にだけ送信する。

        @copydetails interface_send_local_message

        @param[in] in_invoice 送信するメッセージの送り状。
     */
    public: bool send_local_message(
        typename this_type::invoice const& in_invoice)
    {
        typedef typename this_type::packet::suite suite;
        return this->send_local_message(
            typename this_type::packet::template
                zonal<suite>(suite(in_invoice)));
    }

    /** @brief
        任意型の引数を持つメッセージパケットを、
        *thisに登録されたメッセージ受信関数にだけ送信する。

        @copydetails send_local_message(typename this_type::invoice const&)

        @param[in] in_parameter 送信するメッセージの引数。
     */
    public: template<typename template_parameter>
    bool send_local_message(
        typename this_type::invoice const& in_invoice,
        template_parameter in_parameter)
    {
        typedef
            typename this_type::packet::suite::template
                parametric<template_parameter>
            suite;
        return this->send_local_message(
            typename this_type::packet::template
                zonal<suite>(suite(in_invoice, std::move(in_parameter))));
    }

    /** @brief メッセージパケットを、メッセージ受信フックへ配信する。

        - this_type::get_thread_id と合致しないスレッドで実行すると失敗する。
        - psyq::any::message::zone::flush とこの関数を定期的に実行し、
          メッセージパケットを循環させること。

        @sa *thisにメッセージ受信フックを登録するには、
            this_type::insert_receiver を使う。

        @retval true  成功。メッセージパケットを配信した。
        @retval false 失敗。メッセージパケットを配信しなかった。
     */
    public: bool flush()
    {
        if (!this->agree_this_thread())
        {
            return false;
        }

        // 配信するメッセージパケットを取得する。
        std::lock_guard<psyq::spinlock> const local_lock(this->lock_);
        this->delivery_packets_.swap(this->import_packets_);
        this_type::remove_empty_hook(this->hooks_);

        // メッセージパケットを、メッセージ受信フックへ配信する。
        this_type::deliver_packet(
            this->function_caches_, this->hooks_, this->delivery_packets_);
        this_type::clear_packet_container(
            this->delivery_packets_, this->delivery_packets_.size());
        return true;
    }
    /// @}
    //-------------------------------------------------------------------------
    /** @brief this_type を構築する。
        @param[in] in_thread_id *thisに対応するスレッドの識別子。
        @param[in] in_allocator *thisが使うメモリ割当子の初期値。
     */
    private: dispatcher(
        std::thread::id in_thread_id,
        typename this_type::allocator_type const& in_allocator):
    hooks_(in_allocator),
    import_packets_(in_allocator),
    export_packets_(in_allocator),
    delivery_packets_(in_allocator),
    function_caches_(in_allocator),
    thread_id_(std::move(in_thread_id))
    {}

    /// @brief コピー構築子は使用禁止。
    private: dispatcher(this_type const&);

    /// @brief コピー代入演算子は使用禁止。
    private: this_type& operator=(this_type const&);

    /** @brief 現在のスレッドが処理が許可されているスレッドか判定する。
        @retval true  現在のスレッドは、処理が許可されている。
        @retval false 現在のスレッドは、処理が許可されてない。
     */
    private: bool agree_this_thread() const PSYQ_NOEXCEPT
    {
        auto const local_agree(
            std::this_thread::get_id() == this->get_thread_id());
        PSYQ_ASSERT(local_agree);
        return local_agree;
    }

    //-------------------------------------------------------------------------
    private: template<typename template_iterator>
    static template_iterator find_hook_iterator(
        template_iterator const in_begin,
        template_iterator const in_end,
        typename this_type::invoice::key const in_receiver_key,
        typename this_type::invoice::key const in_method_key)
    {
        for (
            auto i(
                std::lower_bound(
                    in_begin,
                    in_end,
                    in_method_key,
                    typename this_type::hook::method_less()));
            i != in_end;
            ++i)
        {
            auto& local_hook(*i);
            if (local_hook.method_key_ != in_method_key)
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

    /** @brief 空のメッセージ受信フックを削除する。
        @param[in,out] io_hooks 空のメッセージ受信フックを削除するコンテナ。
     */
    private: static void remove_empty_hook(
        typename this_type::hook::container& io_hooks)
    {
        for (auto i(io_hooks.begin()); i != io_hooks.end();)
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
    /** @brief メッセージパケットの送信を予約する。

        @copydetails interface_post_message

        @retval true  成功。メッセージパケットを送信した。
        @retval false 失敗。メッセージパケットを送信しなかった。

        @param[in] in_packet 送信するメッセージパケット。
     */
    private: bool add_export_packet(
        typename this_type::packet::shared_ptr in_packet)
    {
        if (in_packet.get() == nullptr || !this->agree_this_thread())
        {
            return false;
        }
        this->export_packets_.emplace_back(std::move(in_packet));
        return true;
    }

    /** @brief 外部とメッセージパケットを交換する。
        @param[in,out] io_export_packets 輸出するメッセージパケットのコンテナ。
        @param[in]     in_import_packets 輸入するメッセージパケットのコンテナ。
     */
    private: void trade_packet_container(
        typename this_type::packet_shared_ptr_container& io_export_packets,
        typename this_type::packet_shared_ptr_container const& in_import_packets)
    {
        std::lock_guard<psyq::spinlock> const local_lock(this->lock_);

        // パケットを輸出する。
        io_export_packets.reserve(
            io_export_packets.size() + this->export_packets_.size());
        for (auto& local_packet_holder: this->export_packets_)
        {
            io_export_packets.emplace_back(std::move(local_packet_holder));
        }
        this_type::clear_packet_container(
            this->export_packets_, this->export_packets_.size());

        // パケットを輸入する。
        this->import_packets_.insert(
            this->import_packets_.end(),
            in_import_packets.begin(),
            in_import_packets.end());
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

    /** @brief メッセージゾーン外パケットを生成する。
        @return
            生成したメッセージパケットの保持子。
            生成に失敗した場合は、保持子は空となる。
        @param[in,out] io_suite     パケットに設定するメッセージ一式。
        @param[in]     in_allocator 使用するメモリ割当子。
     */
    private: template<typename template_suite>
    static typename this_type::packet::shared_ptr create_external_packet(
        template_suite&& io_suite,
        typename this_type::allocator_type const& in_allocator)
    {
        typedef
            typename this_type::packet::template external<template_suite>
            external_packet;
        return this_type::create_packet<external_packet>(
            std::move(io_suite), in_allocator);
    }

    /** @brief メッセージゾーン内パケットを生成する。
        @return
            生成したメッセージパケットの保持子。
            生成に失敗した場合は、保持子は空となる。
        @param[in,out] io_suite     パケットに設定するメッセージ一式。
        @param[in]     in_allocator 使用するメモリ割当子。
     */
    private: template<typename template_suite>
    static typename this_type::packet::shared_ptr create_zonal_packet(
        template_suite&& io_suite,
        typename this_type::allocator_type const& in_allocator)
    {
        typedef
            typename this_type::packet::template zonal<template_suite>
            zonal_packet;
        return this_type::create_packet<zonal_packet>(
            std::move(io_suite), in_allocator);
    }

    /** @brief メッセージパケットを生成する。
        @return
            生成したメッセージパケットの保持子。
            生成に失敗した場合は、保持子は空となる。
        @param[in,out] io_suite     パケットに設定するメッセージ一式。
        @param[in]     in_allocator 使用するメモリ割当子。
     */
    private: template<typename template_packet>
    static typename this_type::packet::shared_ptr create_packet(
        typename template_packet::suite&& io_suite,
        typename this_type::allocator_type const& in_allocator)
    {
        // メッセージパケットに割り当てるメモリ領域を確保する。
        typename this_type::allocator_type::template
            rebind<template_packet>::other
                local_allocator(in_allocator);
        auto const local_storage(local_allocator.allocate(1));
        if (local_storage == nullptr)
        {
            PSYQ_ASSERT(false);
            return typename this_type::packet::shared_ptr();
        }

        // メッセージパケット保持子を構築する。
        return typename this_type::packet::shared_ptr(
            new(local_storage) template_packet(std::move(io_suite)),
            [=](template_packet* const io_packet)
            {
                if (io_packet != nullptr)
                {
                    auto local_deallocator(local_allocator);
                    local_deallocator.destroy(io_packet);
                    local_deallocator.deallocate(io_packet, 1);
                }
            },
            local_allocator);
    }

    /** @brief メッセージパケットを、メッセージ受信フックへ配信する。
        @param[in] in_hooks   メッセージ受信フックの辞書。
        @param[in] in_packets 配信するメッセージパケットのコンテナ。
     */
    private: static void deliver_packet(
        typename this_type::function_shared_ptr_container& io_functions,
        typename this_type::hook::container const& in_hooks,
        typename this_type::packet_shared_ptr_container const& in_packets)
    {
        // メッセージパケットを走査し、メッセージ受信フックの辞書へ中継する。
        for (auto& local_packet_holder: in_packets)
        {
            auto const local_packet_pointer(local_packet_holder.get());
            if (local_packet_pointer != nullptr)
            {
                this_type::deliver_packet(
                    io_functions, in_hooks, *local_packet_pointer);
            }
            else
            {
                PSYQ_ASSERT(false);
            }
        }
    }

    /** @brief メッセージパケットを、メッセージ受信フックへ配信する。
        @param[in] in_hooks  メッセージ受信フックの辞書。
        @param[in] in_packet 配信するメッセージパケット。
     */
    private: static void deliver_packet(
        typename this_type::function_shared_ptr_container& io_functions,
        typename this_type::hook::container const& in_hooks,
        typename this_type::packet const& in_packet)
    {
        // 受信メッセージ識別値が一致するメッセージ受信フックに、
        // メッセージパケットを配信する。
        io_functions.clear();
        auto& local_suite(in_packet.get_suite());
        auto const local_method_key(local_suite.get_call().get_method());
        for (
            auto i(
                this_type::find_hook_iterator(
                    in_hooks.begin(), 
                    in_hooks.end(),
                    local_method_key,
                    typename this_type::hook::method_less()));
            i != in_hooks.end();
            ++i)
        {
            auto const& local_hook(*i);
            if (local_hook.method_key_ != local_method_key)
            {
                break;
            }

            // メッセージ受信関数をキャッシュに追加する。
            if (local_suite.get_invoice().agree_receiver_key(local_hook.receiver_key_))
            {
                auto local_function(local_reservoir.function_.lock());
                if (local_function.get() != nullptr)
                {
                    io_functions.emplace_back(std::move(local_function));
                }
            }
        }

        for (auto const& local_function: io_functions)
        {
            (*local_function)(in_packet);
        }
        io_functions.clear();
    }

    //-------------------------------------------------------------------------
    /// @brief メッセージ受信フックの辞書。
    private: typename this_type::hook::container hooks_;

    /// @brief 外部から輸入したメッセージパケットのコンテナ。
    private: typename this_type::packet_shared_ptr_container import_packets_;

    /// @brief 外部へ輸出するメッセージパケットのコンテナ。
    private: typename this_type::packet_shared_ptr_container export_packets_;

    /// @brief メッセージ受信関数へ配信するメッセージパケット。
    private: typename this_type::packet_shared_ptr_container delivery_packets_;

    /// @brief メッセージ受信関数のキャッシュ。
    private: typename this_type::function_shared_ptr_container function_caches_;

    /// @brief 排他的処理に使うロックオブジェクト。
    private: psyq::spinlock lock_;

    /// @brief *thisに合致するスレッドの識別子。
    private: std::thread::id const thread_id_;

}; // class psyq::any::message::dispatcher

#endif // !defined(PSYQ_ANY_MESSAGE_DISPATCHER_HPP_)
// vim: set expandtab:
