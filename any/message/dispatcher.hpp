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
            template<typename, typename> class listener;
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
        @param[in] in_sender_key   @copydoc sender_key_
        @param[in] in_receiver_key  @copydoc receiver_key_
        @param[in] in_receiver_mask @copydoc receiver_mask_
        @param[in] in_function_key @copydoc function_key_
     */
    public: PSYQ_CONSTEXPR invoice(
        typename this_type::key const in_sender_key,
        typename this_type::key const in_receiver_key,
        typename this_type::key const in_receiver_mask,
        typename this_type::key const in_function_key)
    PSYQ_NOEXCEPT:
    sender_key_(in_sender_key),
    zone_key_(0),
    zone_mask_(0),
    dispatcher_key_(0),
    dispatcher_mask_(0),
    receiver_key_(in_receiver_key),
    receiver_mask_(in_receiver_mask),
    function_key_(in_function_key)
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

    /** @brief メッセージ受信先オブジェクトの識別値マスクを取得する。
        @return receiver_mask_
     */
    public: PSYQ_CONSTEXPR typename this_type::key get_receiver_mask()
    const PSYQ_NOEXCEPT
    {
        return this->receiver_mask_;
    }

    /** @brief メッセージ受信関数の識別値を取得する。
        @return @copydoc receiver_key_
     */
    public: PSYQ_CONSTEXPR typename this_type::key get_function_key()
    const PSYQ_NOEXCEPT
    {
        return this->function_key_;
    }

    /** @brief *this に合致するメッセージ受信器か判定する。
        @retval true  合致する。
        @retval false 合致しない。
        @param[in] in_key 判定するメッセージ受信器の識別値。
     */
    public: PSYQ_CONSTEXPR bool verify_receiver_key(
        typename this_type::key const in_key)
    const PSYQ_NOEXCEPT
    {
        return this_type::verify_key(
            in_key, this->get_receiver_key(), this->get_receiver_mask());
    }

    private: static PSYQ_CONSTEXPR bool verify_key(
        typename this_type::key const in_key,
        typename this_type::key const in_target_key,
        typename this_type::key const in_target_mask)
    PSYQ_NOEXCEPT
    {
        return (in_key & in_target_mask) == in_target_key;
    }

    //-------------------------------------------------------------------------
    /// @brief メッセージ送信元の識別値。
    private: typename this_type::key sender_key_;
    /// @brief メッセージを中継するゾーンの識別値。
    private: typename this_type::key zone_key_;
    /// @brief メッセージを中継するゾーンの識別値マスク。
    private: typename this_type::key zone_mask_;
    /// @brief メッセージを中継する分配器の識別値。
    private: typename this_type::key dispatcher_key_;
    /// @brief メッセージを中継する分配器の識別値マスク。
    private: typename this_type::key dispatcher_mask_;
    /// @brief メッセージ受信器の識別値。
    private: typename this_type::key receiver_key_;
    /// @brief メッセージ受信器の識別値マスク。
    private: typename this_type::key receiver_mask_;
    /// @brief メッセージ受信関数の識別値。
    private: typename this_type::key function_key_;

}; // class psyq::any::message::invoice

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief メッセージ受信器。
 */
template<typename template_base_suite, typename template_allocator>
class psyq::any::message::listener
{
    private: typedef listener this_type;

    /// @brief メッセージ一式を保持するパケットの基底型。
    public: typedef psyq::any::message::packet<template_base_suite> packet;

    /// @brief メッセージの送り状。
    public: typedef
        psyq::any::message::invoice<typename template_base_suite::tag::key>
        invoice;

    /// @brief this_type で使うメモリ割当子。
    public: typedef template_allocator allocator_type;

    /// @brief メッセージ受信関数。
    public: typedef
        std::function<void(typename this_type::packet const&)>
        function;

    //-------------------------------------------------------------------------
    private: class function_hook
    {
        private: typedef function_hook this_type;

        public: typedef
            std::vector<this_type, typename listener::allocator_type>
            container;

        public: struct less
        {
            bool operator()(
                this_type const& in_left,
                this_type const& in_right)
            const PSYQ_NOEXCEPT
            {
                return in_left.key_ < in_right.key_;
            }

            bool operator()(
                this_type const& in_left,
                typename listener::invoice::key const& in_right)
            const PSYQ_NOEXCEPT
            {
                return in_left.key_ < in_right;
            }

            bool operator()(
                typename listener::invoice::key const& in_left,
                this_type const& in_right)
            const PSYQ_NOEXCEPT
            {
                return in_left < in_right.key_;
            }

        }; // struct less

        public: function_hook(
            typename listener::invoice::key in_key,
            typename listener::function in_function):
        function_(std::move(in_function)),
        key_(std::move(in_key))
        {}

#ifdef PSYQ_NO_STD_DEFAULTED_FUNCTION
        /** @brief ムーブ構築子。
            @param[in,out] io_source ムーブ元となるインスタンス。
         */
        public: function_hook(this_type&& io_source):
        function_(std::move(io_source.function_)),
        key_(std::move(io_source.key_))
        {}

        /** @brief ムーブ代入演算子。
            @param[in,out] io_source ムーブ元となるインスタンス。
            @return *this
         */
        public: this_type& operator=(this_type&& io_source)
        {
            this->function_ = std::move(io_source.function_);
            this->key_ = std::move(io_source.key_);
            return *this;
        }
#endif // !defined(PSYQ_NO_STD_DEFAULTED_FUNCTION)

        public: typename listener::function function_;
        public: typename listener::invoice::key key_;
    };

    //-------------------------------------------------------------------------
    /// @copydoc this_type::call_stack_
    private: class function_call
    {
        private: typedef function_call this_type;

        /** @brief メッセージ受信関数を呼び出す。
            @param[in,out] io_stack_top 最上位のメッセージ受信関数呼び出しスタック。
            @param[in,out] io_function  呼び出すメッセージ受信関数。
            @param[in] in_packet
                呼び出すメッセージ受信関数の引数に渡すメッセージパケット。
         */
        public: function_call(
            typename this_type*& io_stack_top,
            typename listener::function& io_function,
            typename listener::packet const& in_packet):
        original_function_(&io_function),
        next_(io_stack_top)
        {
            PSYQ_ASSERT(static_cast<bool>(io_function));
            io_stack_top = this;
            auto const local_call_stack(
                this_type::find(this->next_, io_function));
            if (local_call_stack != nullptr)
            {
                this->calling_function_ = nullptr;
                *local_call_stack->calling_function_(in_packet);
            }
            else
            {
                auto local_function(std::move(io_function));
                this->calling_function_ = &local_function;
                io_function = [](typename listener::packet const&)
                {
                    // この関数が呼び出されることはない。
                    PSYQ_ASSERT(false);
                };
                local_function(in_packet);
                this->calling_function_ = nullptr;
                if (this->original_function_ != nullptr)
                {
                    *this->original_function_ = std::move(local_function);
                }
            }
            io_stack_top = this->next_;
        }

        public: static this_type* find(
            this_type* const in_stack_top,
            typename this_type::function const& in_function)
        PSYQ_NOEXCEPT
        {
            for (auto i(in_stack_top); i != nullptr; i = i->next_)
            {
                if (i->original_function_ == &in_function
                    && i->calling_function_ != nullptr)
                {
                    return i;
                }
            }
            return nullptr;
        }

        public: static bool remove(
            this_type* const in_stack_top,
            typename listener::function const& in_function)
        PSYQ_NOEXCEPT
        {
            auto const local_call_stack(
                this_type::find(in_stack_top, in_function));
            if (local_call_stack != nullptr)
            {
                local_call_stack->original_function_ = nullptr;
                return true;
            }
            return false;
        }

        public: static void clear(this_type* const in_stack_top) PSYQ_NOEXCEPT
        {
            for (auto i(in_stack_top); i != nullptr; i = i->next_)
            {
                i->original_function_ = nullptr;
            }
        }

        private: typename listener::function* const calling_function_;
        private: typename listener::function* const original_function_;
        private: this_type* const next_;

    }; // class function_call

    //-------------------------------------------------------------------------
    public: listener(
        typename this_type::invoice::key in_key,
        std::thread::id in_thread_id,
        typename this_type::function in_forward_function,
        std::size_t const in_function_capacity,
        typename this_type::allocator_type const& in_allocator):
    hooks_(in_allocator),
    forward_function_(std::move(in_forward_function)),
    call_stack_(nullptr),
    thread_id_(std::move(in_thread_id)),
    key_(std::move(in_key))
    {
        this->hooks_.reserve(in_function_capacity);
    }

    public: listener(this_type const& in_source):
    hooks_(this_type::copy_hooks(in_source.hooks_, in_source.call_stack_)),
    forward_function_(in_source.forward_function_),
    call_stack_(nullptr),
    thread_id_(in_source.thread_id_),
    key_(in_source.key_)
    {}

    public: ~listener()
    {
        this_type::function_call::clear(this->call_stack_);
    }

    public: this_type& operator=(this_type const& in_source)
    {
        if (&in_source == this) {}
        else if (this->call_stack_ != nullptr)
        {
            PSYQ_ASSERT(false);
        }
        else
        {
            this->hooks_ = this_type::copy_hooks(
                in_source.hooks_, in_source.call_stack_);
            this->forward_function_ = in_source.forward_function_;
            this->thread_id_ = in_source.thread_id_;
            this->key_ = in_source.key_;
        }
        return *this;
    }

    public: this_type& operator=(this_type&& io_source)
    {
        if (&in_source == this) {}
        else if (this->call_stack_ != nullptr)
        {
            PSYQ_ASSERT(false);
        }
        else
        {
            this->hooks_ = std::move(io_source.hooks_);
            this->forward_function_ = std::move(io_source.forward_function_);
            this->thread_id_ = in_source.thread_id_;
            this->key_ = in_source.key_;

            io_source.hooks_ = typename this_type::function_hook::container(
                io_source.hooks_.get_allocator());
            io_source.forward_function_ = nullptr;
        }
        return *this;
    }

    private: static typename this_type::function_hook::container copy_hooks(
        typename this_type::function_hook::container const& in_hooks,
        typename this_type::function_call* const in_call_stack)
    {
        typename this_type::function_hook::container
            local_hooks(in_hooks.get_allocator());
        local_hooks.reserve(in_hooks.size());
        for (auto& local_hook: in_hooks)
        {
            if (static_cast<bool>(local_hook.function_))
            {
                auto const local_call_stack(
                    this_type::function_call::find(
                        in_call_stack, local_hook.function_));
                local_hooks.push_back(
                    local_call_stack != nullptr?
                        *local_call_stack->calling_function_:
                        local_hook.function_);
            }
        }
        return local_hooks;
    }

    //-------------------------------------------------------------------------
    /** @brief *this が使うメモリ割当子を取得する。
        @return *this が使うメモリ割当子。
     */
    public: typename this_type::allocator_type get_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->hooks_.get_allocator();
    }

    /** @brief メッセージ受信器の識別値を取得する。
        @return @copydoc this_type::key_
     */
    public: typename this_type::invoice::key get_key() const PSYQ_NOEXCEPT
    {
        return this->key_;
    }

    /** @brief *this に合致するスレッドの識別値を取得する。
        @return @copydoc this_type::thread_id_
     */
    public: std::thread::id const& get_thread_id() const PSYQ_NOEXCEPT
    {
        return this->thread_id_;
    }

    /** @brief メッセージ受信関数の数を取得する。
        @return メッセージ受信関数の数。
     */
    public: std::size_t count_functions() const PSYQ_NOEXCEPT
    {
        return this->hooks_.size();
    }

    /** @brief メッセージ転送関数を取得する。
        @return
            設定されているメッセージ転送関数。
            メッセージを転送しない場合は、呼び出し不可能な関数を返す。
     */
    public: typename this_type::function get_forward_function()
    const PSYQ_NOEXCEPT
    {
        return this->forward_function_;
    }

    /** @brief メッセージ転送関数を設定する。
        @retval 成功。 in_function をメッセージ転送関数に設定した。
        @retval 失敗。何もしなかった。
        @param[in] in_function
            新たに設定するメッセージ転送関数。
            メッセージ転送しない場合は、呼び出し不可能な関数を渡す。
     */
    private: bool set_forward_function(
        typename this_type::function in_function = function())
    PSYQ_NOEXCEPT
    {
        if (this->verify_thread())
        {
            this_type::function_call::remove(
                this->call_stack_, this->forward_function_);
            this->forward_function_ = std::move(in_function);
            return true;
        }
        return false;
    }

    //-------------------------------------------------------------------------
    /** @brief メッセージ受信関数を追加する。
        @retval true
            成功。メッセージ受信関数を追加した。追加したメッセージ受信関数は、
            this_type::remove_function で除去できる。
        @retval false
            失敗。メッセージ受信関数を追加しなかった。
            - in_function_key
              に合致するメッセージ受信関数がすでに追加済だと、失敗する。
            - in_function が呼び出し不可能だと、失敗する。
        @param[in] in_function_key 追加するメッセージ受信関数の識別値。
        @param[in] in_function     追加するメッセージ受信関数。
     */
    public: bool add_function(
        typename this_type::invoice::key const in_function_key,
        typename this_type::function in_function)
    {
        if (this->verify_thread() && static_cast<bool>(in_function))
        {
            auto const local_end(this->hooks_.end());
            auto const local_lower_bound(
                std::lower_bound(
                    this->hooks_.begin(),
                    local_end,
                    in_function_key,
                    typename this_type::function_hook::less()));
            if (local_lower_bound == local_end
                || local_lower_bound->key_ != in_function_key)
            {
                this->hooks_.emplace(
                    local_lower_bound,
                    in_function_key,
                    std::move(in_function));
                return true;
            }
        }
        return false;
    }

    /** @brief メッセージ受信関数を除去する。
        @retval true
            成功。 in_function_key に合致するメッセージ受信関数を除去した。
        @retval false 失敗。何もしなかった。
        @param[in] in_function_key 除去するメッセージ受信関数の識別値。
     */
    public: bool remove_function(
        typename this_type::invoice::key const in_function_key)
    {
        if (this->verify_thread())
        {
            auto const local_function(
                this_type::find_hook_function(
                    this->hooks_.begin(), this->hooks_.end(), in_function_key));
            if (local_function != nullptr)
            {
                this_type::function_call::remove(
                    this->call_stack_, *local_function);
                *local_function = nullptr;
                return true;
            }
        }
        return false;
    }

    /** @brief メッセージ受信関数を検索する。
        @retval true  in_function_key に合致するメッセージ受信関数がある。
        @retval false in_function_key に合致するメッセージ受信関数がない。
        @param[in] in_function_key 検索するメッセージ受信関数の識別値。
     */
    public: bool find_function(
        typename this_type::invoice::key const in_function_key)
    const
    {
        if (this->verify_thread())
        {
            auto const local_function(
                this_type::find_hook_function(
                    this->hooks_.begin(), this->hooks_.end(), in_function_key));
            if (local_function != nullptr)
            {
                auto const local_call_stack(
                    this_type::function_call::find(
                        this->call_stack_, *local_function));
                return local_call_stack != nullptr?
                    local_call_stack->calling_function_: local_function;
            }
        }
        return false;
    }

    /** @brief メッセージ受信関数を呼び出す。
        @retval 正 in_packet に合致するメッセージ受信関数を呼び出した。
        @retval 負
            in_packet に合致するメッセージ受信関数がないので、
            代わりにメッセージ転送関数を呼び出した。
        @retval 0
            in_packet に合致するメッセージ受信関数がなく、
            メッセージ転送関数も呼び出し不可能なので、何もしなかった。
        @param[in] in_packet メッセージ受信関数の引数に渡すメッセージパケット。
     */
    public: std::int8_t call_function(
        typename this_type::packet const& in_packet)
    {
        auto const& local_invoice(in_packet.get_suite().get_invoice());
        return local_invoice.verify_receiver_key(this->get_key())?
            this->call_function(in_packet, local_invoice.get_function_key()): 0;
    }

    /** @brief メッセージ受信関数を呼び出す。
        @retval 正 in_function_key に合致するメッセージ受信関数を呼び出した。
        @retval 負
            in_function_key に合致するメッセージ受信関数がないので、
            代わりにメッセージ転送関数を呼び出した。
        @retval 0
            in_function_key に合致するメッセージ受信関数がなく、
            メッセージ転送関数も呼び出し不可能なので、何もしなかった。
        @param[in] in_packet       メッセージ受信関数に渡すメッセージパケット。
        @param[in] in_function_key 呼び出すメッセージ受信関数の識別値。
     */
    public: std::int8_t call_function(
        typename this_type::packet const& in_packet,
        typename this_type::invoice::key const in_function_key)
    {
        if (this->verify_thread())
        {
            // メッセージ受信関数を検索して呼び出す。
            auto const local_function(
                this_type::find_hook_function(
                    this->hooks_.begin(), this->hooks_.end(), in_function_key));
            if (local_function != nullptr)
            {
                typename this_type::function_call(
                    this->call_stack_, *local_function, in_packet);
                return 1;
            }

            // 合致するメッセージ受信関数がなかったので、メッセージを転送する。
            if (static_cast<bool>(this->forward_function_))
            {
                typename this_type::function_call(
                    this->call_stack_, this->forward_function_, in_packet);
                return -1;
            }
        }
        return 0;
    }

    //-------------------------------------------------------------------------
    /** @brief メッセージ受信関数をすべて除去する。
        @retval true  成功。メッセージ受信関数をすべて除去した。
        @retval false 失敗。何もしなかった。
     */
    public: bool clear_functions()
    {
        if (this->verify_thread())
        {
            this_type::function_call::clear(this->call_stack_);
            this->hooks_.clear();
            return true;
        }
        return false;
    }

    public: bool shrink_functions()
    {
        if (this->verify_thread() && this->call_stack_ == nullptr)
        {
            return true;
        }
        return false;
    }

    //-------------------------------------------------------------------------
    /** @brief 現在のスレッドが処理が許可されているスレッドか判定する。
        @retval true  現在のスレッドは、処理が許可されている。
        @retval false 現在のスレッドは、処理が許可されてない。
     */
    private: bool verify_thread() const PSYQ_NOEXCEPT
    {
        auto const local_verify(
            std::this_thread::get_id() == this->get_thread_id());
        PSYQ_ASSERT(local_verify);
        return local_verify;
    }

    private: template<typename template_iterator>
    static auto find_hook(
        template_iterator const& in_begin,
        template_iterator const& in_end,
        typename this_type::invoice::key const in_function_key)
    ->decltype(&(*in_begin))
    {
        auto const local_lower_bound(
            std::lower_bound(
                in_begin,
                in_end,
                in_function_key,
                typename this_type::function_hook::less()));
        if (local_lower_bound != in_end)
        {
            auto& local_hook(*local_lower_bound);
            if (local_hook.key_ == in_function_key)
            {
                return &local_hook;
            }
        }
        return nullptr;
    }

    private: template<typename template_iterator>
    static auto find_hook_function(
        template_iterator const& in_begin,
        template_iterator const& in_end,
        typename this_type::invoice::key const in_function_key)
    ->decltype(&in_begin->function_)
    {
        auto const local_hook(
            this_type::find_hook(in_begin, in_end, in_function_key));
        return local_hook != nullptr && static_cast<bool>(local_hook->function_)?
            &local_hook->function_: nullptr;
    }

    //-------------------------------------------------------------------------
    /// @brief メッセージ受信関数の辞書。
    private: typename this_type::function_hook::container hooks_;

    /// @brief メッセージ転送関数。
    private: typename this_type::function forward_function_;

    /// @brief メッセージ受信関数の呼び出しスタック。
    private: typename this_type::function_call* call_stack_;

    /// @brief *this に合致するスレッドの識別値。
    private: std::thread::id thread_id_;

    /// @brief メッセージ受信器の識別値。
    private: typename this_type::invoice::key key_;

}; // class psyq::any::message::listener

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief スレッド別のRPCメッセージ分配器。

    - psyq::any::message::zone::equip_dispatcher で、 this_type を用意する。
    - this_type::add_function で、メッセージ受信関数を追加する。
    - this_type::post_message で、メッセージを送信する。
    - psyq::any::message::zone::flush で、
      this_type の持つメッセージパケットが集配される。
    - this_type::flush で、 this_type が持つメッセージパケットを、
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
                return in_left.function_key_ != in_right.function_key_?
                    in_left.function_key_ < in_right.function_key_:
                    in_left.priority_ < in_right.priority_;
            }

            bool operator()(
                typename dispatcher::hook const& in_left,
                typename dispatcher::invoice::key const& in_right)
            const PSYQ_NOEXCEPT
            {
                return in_left.function_key_ < in_right;
            }

            bool operator()(
                typename dispatcher::invoice::key const& in_left,
                typename dispatcher::hook const& in_right)
            const PSYQ_NOEXCEPT
            {
                return in_left < in_right.function_key_;
            }
        };

        public: typedef
            std::vector<this_type, typename dispatcher::allocator_type>
            container;

        public: hook(
            typename dispatcher::invoice::key const in_receiver_key,
            typename dispatcher::invoice::key const in_function_key,
            typename dispatcher::priority const in_priority,
            typename dispatcher::function_weak_ptr in_function):
        function_(std::move(in_function)),
        receiver_key_(in_receiver_key),
        function_key_(in_function_key),
        priority_(in_priority)
        {}

#ifdef PSYQ_NO_STD_DEFAULTED_FUNCTION
        /** @brief ムーブ構築子。
            @param[in,out] io_source ムーブ元となるインスタンス。
         */
        public: hook(this_type&& io_source):
        function_(std::move(io_source.function_)),
        receiver_key_(std::move(io_source.receiver_key_)),
        function_key_(std::move(io_source.function_key_)),
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
            this->function_key_ = std::move(io_source.function_key_);
            this->priority_ = std::move(io_source.priority_);
            return *this;
        }
#endif // !defined(PSYQ_NO_STD_DEFAULTED_FUNCTION)

        public: typename dispatcher::function_weak_ptr function_;
        public: typename dispatcher::invoice::key receiver_key_;
        public: typename dispatcher::invoice::key function_key_;
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
    /** @brief メッセージ受信関数を追加する。

        - 追加に成功した後は、 in_receiver_key / in_function_key
          の組み合わせと合致するメッセージを受信するたび、
          in_function が呼び出される。
        - 追加した in_function は this_type::function_weak_ptr
          で監視しているだけで、 this_type からは所有権を持たない。
          in_function の所有権は、ユーザーが管理すること。
        - in_function の所有権を持つスマートポインタがなくなると、
          this_type から自動で取り外される。手動で取り外す場合は、
          this_type::remove_function を使う。

        @retval true 成功。メッセージ受信関数を挿入した。
        @retval false
            失敗。メッセージ受信関数を追加しなかった。
            - this_type::get_thread_id と合致しないスレッドから呼び出すと、失敗する。
            - in_receiver_key / in_function_key の組み合わせが同じ
              メッセージ受信関数がすでに挿入されていると、失敗する。
            - in_function が空だと、失敗する。

        @param[in] in_receiver_key メッセージ受信オブジェクトの識別値。
        @param[in] in_function_key メッセージ受信関数の識別値。
        @param[in] in_priority     メッセージを受信する優先順位。
        @param[in] in_function     追加するメッセージ受信関数。
     */
    public: bool add_function(
        typename this_type::invoice::key const in_receiver_key,
        typename this_type::invoice::key const in_function_key,
        typename this_type::priority const in_priority,
        typename this_type::function_shared_ptr const& in_function)
    {
        if (in_function.get() == nullptr || !this->verify_thread())
        {
            return false;
        }
        auto const local_end(this->hooks_.end());
        auto local_lower_bound(
            std::lower_bound(
                this->hooks_.begin(),
                local_end,
                in_function_key,
                typename this_type::hook::method_less()));
        for (auto i(local_lower_bound); i != local_end;)
        {
            auto& local_hook(*i);
            if (local_hook.function_key_ != in_function_key)
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
            in_function_key,
            in_priority,
            in_function);
        return true;
    }

    /** @brief メッセージ受信関数を除去する。
        @param[in] in_receiver_key
            除去するメッセージ受信関数に対応する
            メッセージ受信オブジェクトの識別値。
        @param[in] in_function_key
            除去するメッセージ受信関数に対応する
            メッセージ呼出メソッドの識別値。
        @return
            除去したメッセージ受信関数を指すスマートポインタ。
            該当するメッセージ受信関数がない場合は、空となる。
     */
    public: typename this_type::function_weak_ptr remove_function(
        typename this_type::invoice::key const in_receiver_key,
        typename this_type::invoice::key const in_function_key)
    {
        std::lock_guard<psyq::spinlock> const local_lock(this->lock_);
        auto const local_end(this->hooks_.end());
        auto const local_find(
            this_type::find_hook_iterator(
                this->hooks_.begin(),
                local_end,
                in_receiver_key,
                in_function_key));
        if (local_find == local_end)
        {
            return typename this_type::function_weak_ptr();
        }
        auto const local_function(std::move(local_find->function_));
        local_find->function_.reset();
        return local_function;
    }

    /** @brief メッセージ受信関数を除去する。
        @param[in] in_receiver_key
            除去するメッセージ受信関数に対応する
            メッセージ受信オブジェクトの識別値。
        @return 除去したメッセージ受信関数の数。
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

    /** @brief メッセージ受信関数を検索する。
        @param[in] in_receiver_key
            検索するメッセージ受信関数に対応する
            メッセージ受信オブジェクトの識別値。
        @param[in] in_function_key
            検索するメッセージ受信関数に対応する
            メッセージ呼出メソッドの識別値。
        @return
            検索したメッセージ受信関数を指すスマートポインタ。
            該当するメッセージ受信関数が見つからなかった場合は、空となる。
     */
    public: typename this_type::function_weak_ptr find_function(
        typename this_type::invoice::key const in_receiver_key,
        typename this_type::invoice::key const in_function_key)
    const PSYQ_NOEXCEPT
    {
        std::lock_guard<psyq::spinlock> const local_lock(this->lock_);
        auto const local_end(this->hooks_.end());
        auto const local_find(
            this_type::find_hook_iterator(
                this->hooks_.begin(),
                local_end,
                in_receiver_key,
                in_function_key));
        if (local_find == local_end)
        {
            return typename this_type::function_weak_ptr();
        }
        return local_find->function_;
    }
    /// @}
    //-------------------------------------------------------------------------
    /** @name メッセージの送受信
        @{
     */
    /** @interface interface_post_message
        @brief post_message と post_zonal_message に共通する説明。

        - this_type::get_thread_id と合致しないスレッドから呼び出すと、失敗する。
        - この関数では、メッセージの送信の予約のみを行う。
          実際のメッセージ送信処理は、この関数の呼び出し後、
          psyq::any::message::zone::flush / this_type::flush
          の順に呼び出すことで行なわれる。
        - メッセージ受信処理は、送信処理が行われた後、
          psyq::any::message::zone::flush / this_type::flush
          の順に呼び出すことで行なわれる。
        - 同一スレッドで送信を予約したメッセージの受信順序は、
          送信予約順序と同じになる。

        @sa メッセージゾーン内にだけメッセージを送信するには、
            this_type::post_zonal_message を使う。
        @sa *this に登録されているメッセージ受信関数にのみメッセージを送信するには、
            this_type::send_local_message を使う。

        @retval true  成功。メッセージ送信を予約した。
        @retval false 失敗。メッセージ送信を予約しなかった。
     */
    /** @brief メッセージゾーンの内と外へのメッセージの送信を予約する。

        引数を持たないメッセージパケットを動的メモリ割当して構築し、
        メッセージゾーンの内と外への送信を予約する。

        @copydetails interface_post_message

        @param[in] in_invoice 送信するメッセージの送り状。
        @todo メッセージゾーンの外へ送信する処理は未実装。
     */
    public: bool post_message(typename this_type::invoice const& in_invoice)
    {
        return this->add_export_packet(
            this_type::create_external_packet(
                typename this_type::packet::suite(in_invoice),
                this->get_allocator()));
    }

    /** @brief メッセージゾーンの内と外へのメッセージの送信を予約する。

        POD型の引数を持つメッセージパケットを動的メモリ割当して構築し、
        メッセージゾーンの内と外への送信を予約する。

        @copydetails interface_post_message

        @param[in] in_invoice   送信するメッセージの送り状。
        @param[in] in_parameter 送信するメッセージの引数。必ずPOD型。
        @todo 未実装。
     */
    public: template<typename template_parameter>
    bool post_message(
        typename this_type::invoice const& in_invoice,
        template_parameter in_parameter);

    /** @brief メッセージゾーン内へのメッセージの送信を予約する。

        引数を持たないメッセージパケットを動的メモリ割当して構築し、
        メッセージゾーン内への送信を予約する。

        @copydetails interface_post_message

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

    /** @brief メッセージゾーン内へのメッセージの送信を予約する。

        任意型の引数を持つメッセージパケットを動的メモリ割当して構築し、
        メッセージゾーン内への送信を予約する。

        @copydetails interface_post_message

        @param[in] in_invoice   送信するメッセージの送り状。
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
        - メッセージを送信し、受信関数の終了までブロックする。

        @sa *thisにメッセージ受信関数を登録するには、
            this_type::add_function を使う。
        @sa メッセージゾーンの内と外にメッセージを送信するには、
            this_type::post_message を使う。
        @sa メッセージゾーン内にだけメッセージを送信するには、
            this_type::post_zonal_message を使う。

        @retval true  成功。メッセージを送信した。
        @retval false 失敗。メッセージを送信しなかった。
     */
    /** @brief メッセージを送信する。

        *this に挿入されているメッセージ受信関数にだけ、メッセージを送信する。

        @copydetails interface_send_local_message

        @param[in] in_packet 送信するメッセージのパケット。
     */
    public: bool send_local_message(
        typename this_type::packet const& in_packet)
    {
        if (!this->verify_thread())
        {
            return false;
        }
        this_type::deliver_packet(
            this->function_caches_, this->hooks_, in_packet);
        return true;
    }

    /** @brief 引数を持たないメッセージを送信する。

        引数を持たないメッセージを、
        *this に挿入されているメッセージ受信関数にだけ送信する。

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

    /** @brief 任意型の引数を持つメッセージを送信する。

        任意型の引数を持つメッセージを、
        *this に挿入されているメッセージ受信関数にだけ送信する。

        @copydetails interface_send_local_message

        @param[in] in_invoice   送信するメッセージの送り状。
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

    /** @brief メッセージパケットを、メッセージ受信関数へ配信する。

        - this_type::get_thread_id と合致しないスレッドから呼び出すと失敗する。
        - psyq::any::message::zone::flush とこの関数を定期的に呼び出し、
          メッセージパケットを循環させること。

        @sa *this にメッセージ受信関数を登録するには、
            this_type::add_function を使う。

        @retval true  成功。メッセージパケットを配信した。
        @retval false 失敗。メッセージパケットを配信しなかった。
     */
    public: bool flush()
    {
        if (!this->verify_thread())
        {
            return false;
        }

        // 配信するメッセージパケットを取得する。
        this->delivery_packets_.swap(this->import_packets_);
        this_type::remove_empty_hook(this->hooks_);

        // メッセージパケットを、メッセージ受信関数へ配信する。
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

    /// @brief ムーブ構築子は使用禁止。
    private: dispatcher(this_type&&);

    /// @brief コピー代入演算子は使用禁止。
    private: this_type& operator=(this_type const&);

    /// @brief ムーブ代入演算子は使用禁止。
    private: this_type& operator=(this_type&&);

    /** @brief 現在のスレッドが処理が許可されているスレッドか判定する。
        @retval true  現在のスレッドは、処理が許可されている。
        @retval false 現在のスレッドは、処理が許可されてない。
     */
    private: bool verify_thread() const PSYQ_NOEXCEPT
    {
        auto const local_verify(
            std::this_thread::get_id() == this->get_thread_id());
        PSYQ_ASSERT(local_verify);
        return local_verify;
    }

    //-------------------------------------------------------------------------
    private: template<typename template_iterator>
    static template_iterator find_hook_iterator(
        template_iterator const in_begin,
        template_iterator const in_end,
        typename this_type::invoice::key const in_receiver_key,
        typename this_type::invoice::key const in_function_key)
    {
        for (
            auto i(
                std::lower_bound(
                    in_begin,
                    in_end,
                    in_function_key,
                    typename this_type::hook::method_less()));
            i != in_end;
            ++i)
        {
            auto& local_hook(*i);
            if (local_hook.function_key_ != in_function_key)
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

        @retval true  成功。メッセージパケットの送信を予約した。
        @retval false 失敗。メッセージパケットの送信を予約しなかった。

        @param[in] in_packet 送信するメッセージパケット。
     */
    private: bool add_export_packet(
        typename this_type::packet::shared_ptr in_packet)
    {
        if (in_packet.get() == nullptr || !this->verify_thread())
        {
            return false;
        }
        this->export_packets_.emplace_back(std::move(in_packet));
        return true;
    }

    /** @brief メッセージゾーン外パケットを生成する。
        @return
            生成したメッセージパケットのスマートポインタ。
            生成に失敗した場合は、スマートポインタは空となる。
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
            生成したメッセージパケットのスマートポインタ。
            生成に失敗した場合は、スマートポインタは空となる。
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
            生成したメッセージパケットのスマートポインタ。
            生成に失敗した場合は、スマートポインタは空となる。
        @param[in,out] io_suite     パケットに設定するメッセージ一式。
        @param[in]     in_allocator 使用するメモリ割当子。
     */
    private: template<typename template_packet>
    static typename this_type::packet::shared_ptr create_packet(
        typename template_packet::suite&& io_suite,
        typename this_type::allocator_type const& in_allocator)
    {
        // メッセージパケットに割り当てるメモリ領域を動的割当する。
        typename this_type::allocator_type::template
            rebind<template_packet>::other
                local_allocator(in_allocator);
        auto const local_storage(local_allocator.allocate(1));
        if (local_storage == nullptr)
        {
            PSYQ_ASSERT(false);
            return typename this_type::packet::shared_ptr();
        }

        // メッセージパケットのスマートポインタを構築する。
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

    /** @brief メッセージパケットを、メッセージ受信関数へ配信する。
        @param[in] in_hooks   メッセージ受信フックの辞書。
        @param[in] in_packets 配信するメッセージパケットのコンテナ。
     */
    private: static void deliver_packet(
        typename this_type::function_shared_ptr_container& io_functions,
        typename this_type::hook::container const& in_hooks,
        typename this_type::packet_shared_ptr_container const& in_packets)
    {
        // メッセージパケットを走査し、メッセージ受信フックの辞書へ中継する。
        io_functions.clear();
        for (auto& local_packet_holder: in_packets)
        {
            auto const local_packet_pointer(local_packet_holder.get());
            if (local_packet_pointer != nullptr)
            {
                this_type::deliver_packet(
                    io_functions, in_hooks, *local_packet_pointer);
                for (auto const& local_function: io_functions)
                {
                    (*local_function)(*local_packet_pointer);
                }
                io_functions.clear();
            }
            else
            {
                PSYQ_ASSERT(false);
            }
        }
    }

    /** @brief メッセージパケットを、メッセージ受信関数へ配信する。
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
        auto& local_invoice(in_packet.get_suite().get_invoice());
        for (
            auto i(
                this_type::find_hook_iterator(
                    in_hooks.begin(), 
                    in_hooks.end(),
                    local_invoice.get_function_key(),
                    typename this_type::hook::method_less()));
            i != in_hooks.end();
            ++i)
        {
            auto const& local_hook(*i);
            if (local_hook.function_key_ != local_invoice.get_function_key())
            {
                break;
            }

            // メッセージ受信関数をキャッシュに追加する。
            if (local_invoice.verify_receiver_key(local_hook.receiver_key_))
            {
                auto local_function(local_reservoir.function_.lock());
                if (local_function.get() != nullptr)
                {
                    io_functions.emplace_back(std::move(local_function));
                }
            }
        }
    }

    //-------------------------------------------------------------------------
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
