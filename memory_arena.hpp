#ifndef PSYQ_MEMORY_ARENA_HPP_
#define PSYQ_MEMORY_ARENA_HPP_

namespace psyq
{
    namespace memory_arena
    {
        /// @cond
        template<typename, typename> class allocator;
        template<typename> class fixed_pool;
        /// @endcond
    }
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief メモリアリーナでメモリを管理するメモリ割当子。
    @tparam template_value @copydoc this_type::value_type
    @tparam template_arena @copydoc this_type::arena
 */
template<typename template_value, typename template_arena>
class psyq::memory_arena::allocator
{
    /// thisが指す値の型。
    private: typedef allocator this_type;
    template<typename, typename> friend class psyq::memory_arena::allocator;

    //-------------------------------------------------------------------------
    /// メモリを割り当てる値の型。
    public: typedef template_value value_type;

    /** @brief 実際にメモリを管理するメモリアリーナ。

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
          // @param[in] in_size      確保するメモリのバイト数。
          // @param[in] in_alignment 確保するメモリの境界バイト数。
          // @param[in] in_offset    確保するメモリのオフセットバイト数。
          void* template_arena::allocate(std::size_t in_size, std::size_t in_alignment, std::size_t in_offset);
          @endcode
        - メモリを解放するため、以下に相当するメンバ関数を使える必要がある。
          @code
          // @param[in] in_memory 解放するメモリの先頭位置。
          // @param[in] in_size   解放するメモリのバイト数。
          template_arena::deallocate(void* in_memory, std::size_t in_size);
          @endcode
     */
    public: typedef template_arena arena;

    //-------------------------------------------------------------------------
    /// @name メモリ割当子の構築
    //@{
    /** @brief メモリ割当子を構築する。
        @param[in] in_arena メモリ管理に使うメモリアリーナの保持子。
     */ 
    public: explicit allocator(
        typename this_type::arena::shared_ptr const& in_arena)
    PSYQ_NOEXCEPT:
        arena_(in_arena)
    {
        PSYQ_ASSERT(!this->get_arena().expired());
    }

    /** @brief メモリ割当子を構築する。
        @param[in] in_arena メモリ管理に使うメモリアリーナの監視子。
     */ 
    public: explicit allocator(typename this_type::arena::weak_ptr in_arena)
    PSYQ_NOEXCEPT:
        arena_(std::move(in_arena))
    {
        PSYQ_ASSERT(!this->get_arena().expired());
    }

    /** @brief コピー構築子。
        @param[in] in_source コピー元インスタンス。
     */ 
    public: allocator(this_type const& in_source) PSYQ_NOEXCEPT:
        arena_(in_source.get_arena())
    {
        PSYQ_ASSERT(!this->get_arena().expired());
    }

    /** @brief コピー構築子。
        @param[in] in_source コピー元インスタンス。
     */ 
    public: template<typename template_other>
    allocator(
        psyq::memory_arena::allocator<template_other, template_arena> in_source)
    PSYQ_NOEXCEPT:
        arena_(std::move(in_source.arena_))
    {
        PSYQ_ASSERT(!this->get_arena().expired());
    }

    /** @brief コピー代入演算子。
        @param[in] in_source コピー元インスタンス。
        @return *this
     */ 
    public: this_type& operator=(this_type const& in_source) PSYQ_NOEXCEPT
    {
        this->arena_ = in_source.get_arena();
        PSYQ_ASSERT(!this->arena_.expired());
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
        this->arena_ = std::move(in_source.arena_);
        PSYQ_ASSERT(!this->get_arena().expired());
        return *this;
    }
    //@}
    protected: allocator() PSYQ_NOEXCEPT {}

    //-------------------------------------------------------------------------
    /// @name メモリの確保と解放
    //@{
    /** @brief メモリを確保する。
        @retval !=nullptr 確保したメモリの先頭位置。
        @retval ==nullptr メモリの確保に失敗した。
        @param[in] in_size 確保する値の数。
        @param[in] in_hint メモリ確保のヒント。
     */
    public: typename this_type::value_type* allocate(
        std::size_t const in_size,
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
                in_size * sizeof(typename this_type::value_type),
                std::alignment_of<typename this_type::value_type>::value,
                0));
    }

    /** @brief メモリを解放する。
        @param[in] in_pointer 解放する値の先頭位置。
        @param[in] in_size    解放する値の数。
     */
    public: void deallocate(
        typename this_type::value_type* const in_pointer,
        std::size_t const in_size)
    {
        if (in_pointer == nullptr)
        {
            PSYQ_ASSERT(in_size <= 0);
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
            in_pointer, in_size * sizeof(typename this_type::value_type));
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
    this_type& operator==(
        psyq::memory_arena::allocator<template_other, template_arena> const& in_right)
    const
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
    this_type& operator!=(
        psyq::memory_arena::allocator<template_other, template_arena> const& in_right)
    const
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
    /// 実際にメモリを管理しているメモリアリーナ。
    private: typename this_type::arena::weak_ptr arena_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 固定長のメモリブロックを貯めこむメモリアリーナ。

    psyq::memory_arena::allocator に使う。

    @tparam template_allocator this_type::allocator_type
 */
template<typename template_allocator>
class psyq::memory_arena::fixed_pool
{
    /// thisが指す値の型。
    private: typedef fixed_pool this_type;

    //-------------------------------------------------------------------------
    /// メモリ管理に使うメモリ割当子。
    public: typedef template_allocator allocator_type;
    /// this_type の保持子。
    public: typedef std::shared_ptr<this_type> shared_ptr;
    /// this_type の監視子。
    public: typedef std::weak_ptr<this_type> weak_ptr;
    /// メモリ確保するときのメモリ境界。
    public: static std::size_t const ALIGNMENT =
        std::alignment_of<typename this_type::allocator_type::value_type>::value;
    static_assert(
        this_type::ALIGNMENT % std::alignment_of<void*>::value == 0, "");
    /// メモリ確保する単位のバイト数。
    private: static std::size_t const UNIT_SIZE =
        sizeof(typename this_type::allocator_type::value_type);

    //-------------------------------------------------------------------------
    /// @name メモリアリーナの構築と解体
    //@{
    /** @brief メモリアリーナを構築する。

        確保するメモリブロックのバイト数は、
        this_type::allocator_type::value_type と同じになる。

        @param[in] in_allocator 使用するメモリ割当子の初期値。
     */
    public: explicit fixed_pool(
        typename this_type::allocator_type in_allocator = template_allocator())
    PSYQ_NOEXCEPT:
        idle_block_(nullptr),
        idle_count_(0),
        block_size_(sizeof(typename this_type::allocator_type::value_type)),
        allocator_(std::move(in_allocator))
    {}

    /** @brief メモリアリーナを構築する。
        @param[in] in_block_size 確保するメモリブロックのバイト数。
        @param[in] in_allocator  使用するメモリ割当子の初期値。
     */
    public: explicit fixed_pool(
        std::size_t const in_block_size,
        typename this_type::allocator_type in_allocator = template_allocator())
    PSYQ_NOEXCEPT:
        idle_block_(nullptr),
        idle_count_(0),
        block_size_(
            (PSYQ_ASSERT(sizeof(void*) <= in_block_size), in_block_size)),
        allocator_(std::move(in_allocator))
    {}

    /** @brief コピー構築子。
        @param[in] in_source コピー元インスタンス。
     */
    public: fixed_pool(this_type const& in_source) PSYQ_NOEXCEPT:
        idle_block_(nullptr),
        idle_count_(0),
        block_size_(in_source.get_block_size()),
        allocator_(in_source.get_allocator())
    {}

    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元インスタンス。
     */
    public: fixed_pool(this_type&& io_source) PSYQ_NOEXCEPT:
        idle_block_(io_source.idle_block_),
        idle_count_(io_source.get_idle_count()),
        block_size_(io_source.get_block_size()),
        allocator_(std::move(io_source.allocator_))
    {
        io_source.idle_block_ = nullptr;
        io_source.idle_count_ = 0;
    }

    /// メモリアリーナが管理するメモリをすべて解放する。
    public: ~fixed_pool()
    {
        this->release_idle_block(0);
    }
    //@}
    private: this_type& operator=(this_type const& in_source);

    //-------------------------------------------------------------------------
    /// @name メモリの確保と解放
    //@{
    /** @brief メモリを確保する。
        @retval !=nullptr 確保したメモリの先頭位置。
        @retval ==nullptr メモリの確保に失敗した。
        @param[in] in_size      確保するメモリのバイト数。
        @param[in] in_alignment 確保するメモリの境界バイト数。
        @param[in] in_offset    確保するメモリのオフセットバイト数。
     */
    public: void* allocate(
        std::size_t const in_size,
        std::size_t const in_alignment = this_type::ALIGNMENT,
        std::size_t const in_offset = 0)
    {
        if (in_offset != 0)
        {
            PSYQ_ASSERT(false);
            return nullptr;
        }
        if (this_type::ALIGNMENT % in_alignment != 0)
        {
            PSYQ_ASSERT(false);
            return nullptr;
        }
        if (in_size <= 0)
        {
            return nullptr;
        }
        if (this->get_block_size() < in_size)
        {
            return this->allocator_.allocate(in_size);
        }
        if (this->idle_block_ != nullptr)
        {
            auto const local_block(this->idle_block_);
            this->idle_block_ = *static_cast<void**>(this->idle_block_);
            return local_block;
        }
        return this->allocator_.allocate(
            this_type::calc_unit_size(this->get_block_size()));
    }

    /** @brief メモリを解放する。
        @param[in] in_block 解放するメモリの先頭位置。
        @param[in] in_size  解放するメモリのバイト数。
     */
    void deallocate(
        void* const in_block,
        std::size_t const in_size)
    {
        if (this->get_block_size() < in_size)
        {
            this->allocator_.deallocate(
                static_cast<typename this_type::allocator_type::value_type*>(in_block),
                in_size);
        }
        else if (in_block != nullptr && 0 < in_size)
        {
            *static_cast<void**>(in_block) = this->idle_block_;
            this->idle_block_ = in_block;
            ++this->idle_count_;
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
    public: bool operator==(this_type const& in_right) const
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
    public: bool operator!=(this_type const& in_right) const
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

    /** @brief 空メモリブロックの数を取得する。
        @return 空メモリブロックの数。
     */
    public: std::size_t get_idle_count() const PSYQ_NOEXCEPT
    {
        return this->idle_count_;
    }

    /** @brief メモリブロックのバイト数を取得する。
        @return メモリブロックのバイト数。
     */
    public: std::size_t get_block_size() const PSYQ_NOEXCEPT
    {
        return this->block_size_;
    }

    /** @brief 空メモリブロックを実際に解放する。
        @param[in] in_keep_count 残しておく空メモリブロックの数。
     */
    public: void release_idle_block(std::size_t const in_keep_count = 0)
    {
        auto local_idle_block(this->idle_block_);
        auto local_idle_count(this->get_idle_count());
        auto const local_deallocate_size(
            this_type::calc_unit_size(this->get_block_size()));
        while (local_idle_block != nullptr && in_keep_count < local_idle_count)
        {
            auto const local_deallocate_block(
                static_cast<typename this_type::allocator_type::value_type*>(
                    local_idle_block));
            local_idle_block = *static_cast<void**>(local_idle_block);
            this->allocator_.deallocate(
                local_deallocate_block, local_deallocate_size);
            --local_idle_count;
        }
        this->idle_block_ = local_idle_block;
        this->idle_count_ = local_idle_count;
    }
    //@}
    private: static std::size_t calc_unit_size(std::size_t const in_byte_size)
    {
        return
            (in_byte_size + this_type::UNIT_SIZE - 1) / this_type::UNIT_SIZE;
    }

    //-------------------------------------------------------------------------
    private: void* idle_block_;
    private: std::size_t idle_count_;
    private: std::size_t block_size_;
    private: typename this_type::allocator_type allocator_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    namespace test
    {
        inline void memory_arena_fixed_pool()
        {
            typedef psyq::memory_arena::fixed_pool<std::allocator<double>>
                fixed_pool_arena;
            typedef psyq::memory_arena::allocator<int, fixed_pool_arena>
                fixed_pool_allocator;
            typedef std::list<int, fixed_pool_allocator> fixed_pool_list;
            fixed_pool_arena::shared_ptr local_fixed_arena(
                new fixed_pool_arena(64));
            fixed_pool_allocator local_fixed_pool_allocator(local_fixed_arena);
            std::allocator_traits<fixed_pool_allocator>::allocate(
                local_fixed_pool_allocator, 0);
            std::allocator_traits<fixed_pool_allocator>::deallocate(
                local_fixed_pool_allocator, nullptr, 0);
            fixed_pool_list local_list(local_fixed_pool_allocator);
            local_list.push_back(10);
        }
    }
}

#endif // !defined(PSYQ_MEMORY_ARENA_HPP_)
