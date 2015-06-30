#ifndef PSYQ_MEMORY_ARENA_HPP_
#define PSYQ_MEMORY_ARENA_HPP_

#include <array>
#include <memory>
#include "./assert.hpp"

namespace psyq
{
    namespace memory_arena
    {
        /// @cond
        template<typename, typename> class allocator;
        template<typename> class fixed_pool;
        template<std::size_t, typename> class pool_table;
        /// @endcond
    }
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief メモリ管理をメモリアリーナ経由で行うメモリ割当子。
    @tparam template_value メモリ管理する要素の型。
    @tparam template_arena @copydoc allocator::arena
    @note できれば std::allocator_traits 経由で使うようにしたい。
 */
template<typename template_value, typename template_arena>
class psyq::memory_arena::allocator: public std::allocator<template_value>
{
    template<typename, typename> friend class psyq::memory_arena::allocator;
    /// @brief thisが指す値の型。
    private: typedef allocator this_type;
    /// @brief this_type の基底型。
    public: typedef std::allocator<template_value> base_type;

    //-------------------------------------------------------------------------
    /** @brief 実際にメモリを管理するメモリアリーナ。

        - 確保するメモリの境界バイト数を定義した以下の定数を使える必要がある。
          @code
          template_arena::ALIGNMENT
          @endcode
        - std::shared_ptr<template_arena> 互換の、以下の型を使える必要がある。
          @code
          template_arena::shared_ptr
          @endcode
        - std::weak_ptr<template_arena> 互換の、以下の型を使える必要がある。
          @code
          template_arena::weak_ptr
          @endcode
        - メモリを確保するため、以下に相当するメンバ関数を使える必要がある。
          @code
          // @return 確保したメモリの先頭位置。
          // @param[in] in_size 確保するメモリのバイト数。
          void* template_arena::allocate(std::size_t in_size);
          @endcode
        - メモリを解放するため、以下に相当するメンバ関数を使える必要がある。
          @code
          // @param[in] in_memory 解放するメモリの先頭位置。
          // @param[in] in_size   解放するメモリのバイト数。
          template_arena::deallocate(void* in_memory, std::size_t in_size);
          @endcode
     */
    public: typedef template_arena arena;
    static_assert(
        this_type::arena::ALIGNMENT %
            std::alignment_of<typename base_type::value_type>::value == 0,
        "");

    //-------------------------------------------------------------------------
    /** @brief メモリ割当子を再定義する。
        @tparam template_other 再定義するメモリ割当子の要素の型。
     */
    public: template<typename template_other>
    struct rebind
    {
        /// @brief 再定義したメモリ割当子の型。
        typedef
            psyq::memory_arena::allocator<template_other, template_arena>
            other;
    };

    //-------------------------------------------------------------------------
    /// @name メモリ割当子の構築
    //@{
    /** @brief メモリ割当子を構築する。
        @param[in] in_arena メモリ管理に使うメモリアリーナの保持子。
     */ 
    public: explicit allocator(
        typename this_type::arena::shared_ptr const& in_arena)
    PSYQ_NOEXCEPT: arena_(in_arena)
    {
        PSYQ_ASSERT(!this->get_arena().expired());
    }

    /** @brief メモリ割当子を構築する。
        @param[in] in_arena メモリ管理に使うメモリアリーナの監視子。
     */ 
    public: explicit allocator(typename this_type::arena::weak_ptr in_arena)
    PSYQ_NOEXCEPT: arena_(std::move(in_arena))
    {
        PSYQ_ASSERT(!this->get_arena().expired());
    }

    /** @brief コピー構築子。
        @param[in] in_source コピー元インスタンス。
     */ 
    public: allocator(this_type const& in_source)
    PSYQ_NOEXCEPT: arena_(in_source.get_arena())
    {
        PSYQ_ASSERT(!this->get_arena().expired());
    }

    /** @brief コピー構築子。
        @param[in] in_source コピー元インスタンス。
     */ 
    public: template<typename template_other>
    allocator(
        psyq::memory_arena::allocator<template_other, template_arena> in_source)
    PSYQ_NOEXCEPT: arena_(std::move(in_source.arena_))
    {
        PSYQ_ASSERT(!this->get_arena().expired());
    }

    /** @brief コピー代入演算子。
        @param[in] in_source コピー元インスタンス。
        @return *this
     */ 
    public: this_type& operator=(this_type const& in_source) PSYQ_NOEXCEPT
    {
        PSYQ_ASSERT(!in_source.arena_.expired());
        this->arena_ = in_source.get_arena();
        return *this;
    }

    /** @brief コピー代入演算子。
        @param[in] in_source コピー元インスタンス。
        @return *this
     */ 
    public: template<typename template_other>
    this_type& operator=(
        psyq::memory_arena::allocator<template_other, template_arena> in_source)
    {
        PSYQ_ASSERT(!in_source.get_arena().expired());
        this->arena_ = std::move(in_source.arena_);
        return *this;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name メモリの確保と解放
    //@{
    /** @brief メモリを確保する。
        @retval !=nullptr 確保したメモリの先頭位置。
        @retval ==nullptr メモリの確保に失敗した。
        @param[in] in_count 構築するインスタンスの数。
        @param[in] in_hint  メモリ確保のヒント。
     */
    public: typename base_type::pointer allocate(
        typename base_type::size_type const in_count,
        std::allocator<void>::const_pointer const in_hint = 0)
    {
        auto const local_arena_holder(this->arena_.lock());
        auto const local_arena(local_arena_holder.get());
        if (local_arena == nullptr)
        {
            PSYQ_ASSERT(false);
            return nullptr;
        }
        return static_cast<typename this_type::value_type*>(
            local_arena->allocate(
                in_count * sizeof(typename this_type::value_type)));
    }

    /** @brief メモリを解放する。
        @param[in] in_pointer 解放するメモリの先頭位置。
        @param[in] in_count   解体したインスタンスの数。
     */
    public: void deallocate(
        typename base_type::pointer const in_pointer,
        typename base_type::size_type const in_count)
    {
        if (in_pointer == nullptr)
        {
            PSYQ_ASSERT(in_count <= 0);
            return;
        }
        auto const local_arena_holder(this->arena_.lock());
        auto const local_arena(local_arena_holder.get());
        if (local_arena == nullptr)
        {
            PSYQ_ASSERT(false);
            return;
        }
        local_arena->deallocate(
            in_pointer, in_count * sizeof(typename this_type::value_type));
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name メモリ割当子のプロパティ
    //@{
    /** @brief 等価なメモリ割当子か判定する

        メモリ割当子が等価なら、互いに確保したメモリが、互いに解放できる。

        @retval true  等価である。
        @retval false 不等価である。
        @param[in] in_right 比較するメモリ割当子。
     */
    public: template<typename template_other>
    bool operator==(
        psyq::memory_arena::allocator<template_other, template_arena> const& in_right)
    const PSYQ_NOEXCEPT
    {
        auto const local_left_holder(this->get_arena().lock());
        auto const local_left_arena(local_left_holder.get());
        if (local_left_arena == nullptr)
        {
            return false;
        }
        auto const local_right_holder(in_right.get_arena().lock());
        auto const local_right_arena(local_right_holder.get());
        if (local_right_arena == nullptr)
        {
            return false;
        }
        return local_left_arena == local_right_arena
            || *local_left_arena == *local_right_arena;
    }

    /** @brief 不等価なメモリ割当子か判定する。

        メモリ割当子が等価なら、互いに確保したメモリが、互いに解放できる。

        @retval true 不等価である。
        @retval false  等価である。
        @param[in] in_right 比較するメモリ割当子。
     */
    public: template<typename template_other>
    bool operator!=(
        psyq::memory_arena::allocator<template_other, template_arena> const& in_right)
    const PSYQ_NOEXCEPT
    {
        return !this->operator==(in_right);
    }

    /** @brief 実際にメモリを管理しているメモリアリーナを取得する。
        @return @copydoc arena_
     */
    public: typename this_type::arena::weak_ptr const& get_arena() const
    {
        return this->arena_;
    }
    //@}
    //-------------------------------------------------------------------------
    protected: allocator() PSYQ_NOEXCEPT {}

    //-------------------------------------------------------------------------
    /// @brief 実際にメモリを管理しているメモリアリーナ。
    private: typename this_type::arena::weak_ptr arena_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 固定長のメモリをプールして再利用する、高速なメモリアリーナ。

    - psyq::memory_arena::allocator を経由させることで、
      以下のSTLコンテナで使うことを想定している。
      - std::list
      - std::map
      - std::multimap
      - std::set
      - std::multiset
      - std::unordered_map
      - std::unordered_multimap
      - std::unordered_set
      - std::unordered_multiset

    @tparam template_allocator this_type::allocator_type
 */
template<typename template_allocator>
class psyq::memory_arena::fixed_pool
{
    /// @brief thisが指す値の型。
    private: typedef fixed_pool this_type;

    //-------------------------------------------------------------------------
    /// @brief メモリ管理に使うメモリ割当子。
    public: typedef template_allocator allocator_type;
    /// @brief this_type の保持子。
    public: typedef std::shared_ptr<this_type> shared_ptr;
    /// @brief this_type の監視子。
    public: typedef std::weak_ptr<this_type> weak_ptr;
    /// @brief メモリ確保するときのメモリ境界。
    public: static std::size_t const ALIGNMENT =
        std::alignment_of<typename this_type::allocator_type::value_type>::value;
    static_assert(
        this_type::ALIGNMENT % std::alignment_of<void*>::value == 0, "");
    /// @brief メモリ確保する単位のバイト数。
    public: static std::size_t const UNIT_SIZE =
        sizeof(typename this_type::allocator_type::value_type);
    static_assert(sizeof(void*) <= this_type::UNIT_SIZE, "");

    //-------------------------------------------------------------------------
    /// @name メモリアリーナの構築と解体
    //@{
    /** @brief メモリアリーナを構築する。

        プールするメモリブロックのバイト数は、
        this_type::allocator_type::value_type と同じになる。

        @param[in] in_allocator 使用するメモリ割当子の初期値。
     */
    public: explicit fixed_pool(
        typename this_type::allocator_type in_allocator = template_allocator())
    PSYQ_NOEXCEPT:
    idle_block_(nullptr),
    block_size_(sizeof(typename this_type::allocator_type::value_type)),
    allocator_(std::move(in_allocator))
    {}

    /** @brief メモリアリーナを構築する。
        @param[in] in_block_size プールするメモリブロックのバイト数。
        @param[in] in_allocator  使用するメモリ割当子の初期値。
     */
    public: explicit fixed_pool(
        std::size_t const in_block_size,
        typename this_type::allocator_type in_allocator = template_allocator())
    PSYQ_NOEXCEPT:
    idle_block_(nullptr),
    block_size_((PSYQ_ASSERT(sizeof(void*) <= in_block_size), in_block_size)),
    allocator_(std::move(in_allocator))
    {}

    /** @brief コピー構築子。
        @param[in] in_source コピー元インスタンス。
     */
    public: fixed_pool(this_type const& in_source)
    PSYQ_NOEXCEPT:
    idle_block_(nullptr),
    block_size_(in_source.get_block_size()),
    allocator_(in_source.get_allocator())
    {}

    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元インスタンス。
     */
    public: fixed_pool(this_type&& io_source)
    PSYQ_NOEXCEPT:
    idle_block_(io_source.idle_block_),
    block_size_(io_source.get_block_size()),
    allocator_(std::move(io_source.allocator_))
    {
        io_source.idle_block_ = nullptr;
    }

    /// @brief メモリアリーナが管理するメモリをすべて解放する。
    public: ~fixed_pool()
    {
        this->release_idle_block();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name メモリの確保と解放
    //@{
    /** @brief メモリを確保する。

        - 固定長メモリブロック以下の大きさのメモリを確保する場合は、
          空メモリブロックを再利用する。
          - sizeof(this_type::allocator_type::value_type) が、
            固定長メモリブロックの大きさとなる。
        - 空メモリブロックが存在しないか、
          固定長メモリブロックより大きいメモリを確保する場合は、
          this_type::allocator_type::allocate() を呼び出す。

        @retval !=nullptr 確保したメモリの先頭位置。
        @retval ==nullptr メモリの確保に失敗した。
        @param[in] in_size 確保するメモリのバイト数。
     */
    public: void* allocate(std::size_t const in_size)
    {
        if (this->get_block_size() < in_size)
        {
            // ブロックサイズより大きいサイズは、メモリ割当子に任せる。
            return this->allocator_.allocate(in_size);
        }
        else if (in_size <= 0)
        {
            return nullptr;
        }

        auto const local_block(this->idle_block_);
        if (local_block != nullptr)
        {
            // プールしている空メモリブロックを再利用する。
            this->idle_block_ = *static_cast<void**>(local_block);
            return local_block;
        }
        else
        {
            // 空メモリブロックがないのでメモリ確保する。
            return this->allocator_.allocate(
                this_type::calc_unit_size(this->get_block_size()));
        }
    }

    /** @brief メモリを解放する。

        - 固定長メモリブロックと同じ大きさのメモリを解放する場合は、
          空メモリブロックとしてプールする。
        - 固定長メモリブロックと異なる大きさのメモリを解放する場合は、
          this_type::allocator_type::deallocate() を呼び出す。

        @param[in] in_block 解放するメモリの先頭位置。
        @param[in] in_size  解放するメモリのバイト数。
     */
    void deallocate(
        void* const in_block,
        std::size_t const in_size)
    {
        if (this->get_block_size() < in_size)
        {
            // ブロックサイズより大きいサイズは、プールせずすぐに解放する。
            this->allocator_.deallocate(
                static_cast<typename this_type::allocator_type::pointer>(
                    in_block),
                in_size);
        }
        else if (in_block != nullptr)
        {
            // 空メモリブロックのリストに追加する。
            PSYQ_ASSERT(0 < in_size);
            *static_cast<void**>(in_block) = this->idle_block_;
            this->idle_block_ = in_block;
        }
        else
        {
            PSYQ_ASSERT(in_size <= 0);
        }
    }

    /** @brief 空メモリブロックを実際に解放する。
     */
    public: void release_idle_block()
    {
        auto local_idle_block(this->idle_block_);
        auto const local_deallocate_size(
            this_type::calc_unit_size(this->get_block_size()));
        while (local_idle_block != nullptr)
        {
            auto const local_deallocate_block(
                static_cast<typename this_type::allocator_type::value_type*>(
                    local_idle_block));
            local_idle_block = *static_cast<void**>(local_idle_block);
            this->allocator_.deallocate(
                local_deallocate_block, local_deallocate_size);
        }
        this->idle_block_ = local_idle_block;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name メモリアリーナのプロパティ
    //@{
    /** @brief 等価なメモリアリーナか判定する

        メモリアリーナが等価なら、互いに確保したメモリが、互いに解放できる。

        @retval true  等価である。
        @retval false 不等価である。
        @param[in] in_right 比較するメモリアリーナ。
     */
    public: bool operator==(this_type const& in_right) const PSYQ_NOEXCEPT
    {
        return this == &in_right
            || this->get_allocator() == in_right.get_allocator();
    }

    /** @brief 不等価なメモリアリーナか判定する

        メモリアリーナが等価なら、互いに確保したメモリが、互いに解放できる。

        @retval true  不等価である。
        @retval false 等価である。
        @param[in] in_right 比較するメモリアリーナ。
     */
    public: bool operator!=(this_type const& in_right) const PSYQ_NOEXCEPT
    {
        return !this->operator==(in_right);
    }

    /** @brief メモリ管理に使用しているメモリ割当子を取得する。
        @return メモリ管理に使用しているメモリ割当子。
     */
    public: typename this_type::allocator_type const& get_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->allocator_;
    }

    /** @brief プールするメモリブロックのバイト数を取得する。
        @return プールするメモリブロックのバイト数。
     */
    public: std::size_t get_block_size() const PSYQ_NOEXCEPT
    {
        return this->block_size_;
    }
    //@}
    //-------------------------------------------------------------------------
    private: this_type& operator=(this_type const& in_source);

    private: static std::size_t calc_unit_size(std::size_t const in_byte_size)
    {
        return
            (in_byte_size + this_type::UNIT_SIZE - 1) / this_type::UNIT_SIZE;
    }

    //-------------------------------------------------------------------------
    /// @brief 空メモリブロックのリスト。
    private: void* idle_block_;
    /// @brief プールするメモリブロックのバイトサイズ。
    private: std::size_t block_size_;
    /// @brief 使用しているメモリ割当子。
    private: typename this_type::allocator_type allocator_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<std::size_t template_size, typename template_allocator>
class psyq::memory_arena::pool_table
{
    /// @brief thisが指す値の型。
    private: typedef pool_table this_type;

    //-------------------------------------------------------------------------
    /// @brief this_type を所有するスマートポインタ。
    public: typedef std::shared_ptr<this_type> shared_ptr;
    /// @brief this_type を監視するスマートポインタ。。
    public: typedef std::weak_ptr<this_type> weak_ptr;

    /// @brief 実際にメモリ管理をする固定長メモリアリーナ。
    public: typedef psyq::memory_arena::fixed_pool<template_allocator> arena;

    /// @brief @copydoc arena::ALIGNMENT
    public: static std::size_t const ALIGNMENT = this_type::arena::ALIGNMENT;

    /// @brief 保持している固定長メモリアリーナの数。
    public: static std::size_t const ARENA_COUNT =
        (template_size + this_type::arena::UNIT_SIZE - 1) /
            this_type::arena::UNIT_SIZE;
    static_assert(0 < template_size, "");

    /// @brief 固定長メモリアリーナで再利用するメモリブロックの最大バイト数。
    public: static std::size_t const POOL_LIMIT_SIZE =
        this_type::ARENA_COUNT * this_type::arena::UNIT_SIZE;

    /// @brief 固定長メモリアリーナの配列。
    private: typedef
        std::array<typename this_type::arena, this_type::ARENA_COUNT>
        arena_array;

    //-------------------------------------------------------------------------
    /// @name メモリアリーナの構築
    //@{
    /** @brief メモリアリーナを構築する。
        @param[in] in_allocator 使用するメモリ割当子の初期値。
     */
    public: explicit pool_table(
        typename this_type::arena::allocator_type const& in_allocator =
            template_allocator())
    {
        for (std::size_t i(0); i < this_type::ARENA_COUNT; ++i)
        {
            auto& local_arena(this->arenas_.at(i));
            local_arena.~arena();
            new(&local_arena) typename this_type::arena(
                (i + 1) * this_type::arena::UNIT_SIZE, in_allocator);
        }
    }

    /** @brief コピー構築子。
        @param[in] in_source コピー元インスタンス。
     */
    public: pool_table(this_type const& in_source) PSYQ_NOEXCEPT
    {
        for (std::size_t i(0); i < this_type::ARENA_COUNT; ++i)
        {
            auto& local_arena(this->arenas_.at(i));
            local_arena.~arena();
            new(&local_arena) typename this_type::arena(in_source.arenas_.at(i));
        }
    }

    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元インスタンス。
     */
    public: pool_table(this_type&& io_source) PSYQ_NOEXCEPT
    {
        for (std::size_t i(0); i < this_type::ARENA_COUNT; ++i)
        {
            auto& local_arena(this->arenas_.at(i));
            local_arena.~arena();
            new(&local_arena) typename this_type::arena(
                std::move(io_source.arenas_.at(i)));
        }
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name メモリの確保と解放
    //@{
    /** @brief メモリを確保する。
        @retval !=nullptr 確保したメモリの先頭位置。
        @retval ==nullptr メモリの確保に失敗した。
        @param[in] in_size 確保するメモリのバイト数。
     */
    public: void* allocate(std::size_t const in_size)
    {
        auto local_arena(this->find_arena(in_size));
        if (local_arena == nullptr)
        {
            local_arena = &this->arenas_.at(0);
        }
        return local_arena->allocate(in_size);
    }

    /** @brief メモリを解放する。
        @param[in] in_block 解放するメモリの先頭位置。
        @param[in] in_size  解放するメモリのバイト数。
     */
    void deallocate(
        void* const in_block,
        std::size_t const in_size)
    {
        auto local_arena(this->find_arena(in_size));
        if (local_arena == nullptr)
        {
            local_arena = &this->arenas_.at(0);
        }
        local_arena->deallocate(in_block, in_size);
    }

    /** @brief 空メモリブロックを実際に解放する。
     */
    public: void release_idle_block()
    {
        for (auto& local_arena: this->arenas_)
        {
            local_arena.release_idle_block();
        }
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name メモリアリーナのプロパティ
    //@{
    /** @brief 等価なメモリアリーナか判定する

        メモリアリーナが等価なら、互いに確保したメモリが、互いに解放できる。

        @retval true  等価である。
        @retval false 不等価である。
        @param[in] in_right 比較するメモリアリーナ。
     */
    public: bool operator==(this_type const& in_right) const PSYQ_NOEXCEPT
    {
        return this == &in_right || this->arenas_.at(0) == in_right.arenas_.at(0);
    }

    /** @brief 不等価なメモリアリーナか判定する

        メモリアリーナが等価なら、互いに確保したメモリが、互いに解放できる。

        @retval true  不等価である。
        @retval false 等価である。
        @param[in] in_right 比較するメモリアリーナ。
     */
    public: bool operator!=(this_type const& in_right) const PSYQ_NOEXCEPT
    {
        return !this->operator==(in_right);
    }

    public: typename this_type::arena* get_arena(std::size_t const in_index)
    PSYQ_NOEXCEPT
    {
        return const_cast<typename this_type::arena*>(
            const_cast<this_type const*>(this)->get_arena(in_index));
    }

    public: typename this_type::arena const* get_arena(std::size_t const in_index)
    const PSYQ_NOEXCEPT
    {
        return in_index < this_type::ARENA_COUNT?
            &(this->arenas_.at(in_index)): nullptr;
    }

    public: typename this_type::arena* find_arena(std::size_t const in_size)
    PSYQ_NOEXCEPT
    {
        return const_cast<typename this_type::arena*>(
            const_cast<this_type const*>(this)->find_arena(in_size));
    }

    public: typename this_type::arena const* find_arena(std::size_t const in_size)
    const PSYQ_NOEXCEPT
    {
        return this_type::POOL_LIMIT_SIZE < in_size?
            nullptr:
            &this->arenas_.at(
                this_type::arena::UNIT_SIZE < in_size?
                    (in_size - 1) / this_type::arena::UNIT_SIZE: 0);
    }
    //@}
    //-------------------------------------------------------------------------
    private: typename this_type::arena_array arenas_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
#include <list>
namespace psyq_test
{
    inline void memory_arena_fixed_pool()
    {
        typedef psyq::memory_arena::pool_table<32, std::allocator<void*>>
            pool_table_arena;
        pool_table_arena::shared_ptr local_table_arena(new pool_table_arena);

        typedef psyq::memory_arena::allocator<int, pool_table_arena>
            pool_table_allocator;
        pool_table_allocator local_pool_table_allocator(local_table_arena);

        typedef std::list<int, pool_table_allocator> list;
        list local_list(local_pool_table_allocator);
        local_list.push_back(1);
        local_list.push_back(10);
        local_list.erase(local_list.begin());
    }
}

#endif // !defined(PSYQ_MEMORY_ARENA_HPP_)
// vim: set expandtab:
