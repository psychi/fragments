/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copydoc psyq::message_pack::pool
 */
#ifndef PSYQ_MESSAGE_PACK_POOL_HPP_
#define PSYQ_MESSAGE_PACK_POOL_HPP_

/// psyq::message_pack::pool のチャンク容量のデフォルト値。
#ifndef PSYQ_MESSAGE_PACK_MEMORY_POOL_CHUNK_CAPACITY_DEFAULT
#define PSYQ_MESSAGE_PACK_MEMORY_POOL_CHUNK_CAPACITY_DEFAULT 4096
#endif // !defined(PSYQ_MESSAGE_PACK_SERIALIZER_STACK_CAPACITY_DEFAULT)

namespace psyq
{
    namespace message_pack
    {
        /// @cond
        template<typename = std::allocator<std::int64_t>> class pool;
        /// @endcond
    }
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief MessagePackで使うメモリ割当子。
 */
template<typename template_allocator>
class psyq::message_pack::pool
{
    private: typedef pool<template_allocator> self; ///< thisが指す値の型。

    /// 使用するメモリ割当子。
    public: typedef template_allocator allocator_type;

    private: struct chunk_header
    {
        chunk_header* next_chunk; ///< 次のメモリチャンク。
        std::size_t capacity;     ///< チャンク容量のバイト数。
        std::size_t free_size;    ///< 未使用メモリのバイト数。
    };

    private: enum: std::size_t
    {
        value_size = sizeof(typename self::allocator_type::value_type),
        header_size = sizeof(typename self::chunk_header),
    };

    //-------------------------------------------------------------------------
    /** @brief メモリプールを構築する。
        @param[in] in_default_capacity チャンク容量バイト数のデフォルト値。
        @param[in] in_allocator        メモリ割当子の初期値。
     */
    public: explicit pool(
        std::size_t const in_default_capacity
            = PSYQ_MESSAGE_PACK_MEMORY_POOL_CHUNK_CAPACITY_DEFAULT,
        typename self::allocator_type in_allocator = self::allocator_type())
    :
        chunk_list_(nullptr),
        default_capacity_(in_default_capacity),
        allocator_(std::move(in_allocator))
    {}

    /** @brief copy構築子。
        @param[in] in_source copy元。
     */
    public: pool(self const& in_source):
        chunk_list_(nullptr),
        default_capacity_(in_source.get_default_capacity()),
        allocator_(in_source.get_allocator())
    {}

    /** @brief move構築子。
        @param[in,out] io_source move元。
     */
    public: pool(self&& io_source):
        chunk_list_(std::move(in_source.chunk_list_)),
        default_capacity_(std::move(in_source.default_capacity_)),
        allocator_(std::move(in_source.allocator_))
    {
        io_source.chunk_list_ = nullptr;
    }

    /** @brief 確保したメモリをすべて破棄する。
     */
    public: ~pool()
    {
        auto local_chunk(this->chunk_list_);
        while (local_chunk != nullptr)
        {
            auto const local_capacity(local_chunk->capacity);
            auto const local_memory(
                reinterpret_cast<typename self::allocator_type::pointer>(
                    reinterpret_cast<std::int8_t*>(local_chunk)
                        - local_capacity));
            local_chunk = local_chunk->next_chunk;
            this->allocator_.deallocate(
                local_memory,
                self::alignment_count<self::value_size>(
                    local_capacity + sizeof(typename self::chunk_header)));
        }
    }

    /** @brief copy代入演算子。
        @param[in] in_source copy元。
        @return *this
     */
    public: self& operator=(self const& in_source)
    {
        this->default_capacity_ = in_source.get_default_capacity();
        this->allocator_ = in_source.get_allocator();
        return *this;
    }

    /** @brief move代入演算子。
        @param[in,out] io_source move元。
        @return *this
     */
    public: self& operator=(self&& io_source)
    {
        if (this != &io_source)
        {
            this->~self();
            new(this) self(std::move(io_source));
        }
        return *this;
    }

    //-------------------------------------------------------------------------
    public: bool operator==(self const& in_source) const PSYQ_NOEXCEPT
    {
        return this->allocator_ == in_source.get_allocator();
    }
    public: bool operator!=(self const& in_source) const PSYQ_NOEXCEPT
    {
        return this->allocator_ != in_source.get_allocator();
    }

    /** @brief メモリ割当子を取得する。
        @return メモリ割当子。
     */ 
    public: typename self::allocator_type const& get_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->allocator_;
    }

    /** @brief チャンク容量バイト数のデフォルト値を取得する。
        @return チャンク容量バイト数のデフォルト値。
        @sa self::allocate()
     */
    public: std::size_t get_default_capacity() const PSYQ_NOEXCEPT
    {
        return this->default_capacity_;
    }

    //-------------------------------------------------------------------------
    /** @brief メモリを確保する。

        確保したメモリは、 self::~pool() で解放される。

        @param[in] in_size      確保するメモリのバイト数。
        @param[in] in_alignment 確保するメモリ先頭位置のメモリ境界単位。
        @retval !=nullptr 確保したメモリの先頭位置。
        @retval ==nullptr メモリの確保に失敗した。
     */
    public: void* allocate(
        std::size_t const in_size,
        std::size_t const in_alignment = sizeof(std::int64_t))
    {
        // チャンクからメモリを分配する。
        if (this->chunk_list_ != nullptr)
        {
            auto const local_memory(
                self::partition_chunk(
                    *this->chunk_list_, in_size, in_alignment));
            if (local_memory != nullptr)
            {
                return local_memory;
            }
        }

        // 新たにチャンクを確保する。
        auto const local_free_size(
            self::alignment_size<sizeof(void*)>(
                std::max(
                    self::header_size < this->get_default_capacity()?
                        this->get_default_capacity() - self::header_size: 0,
                    std::max(in_size, in_alignment * 2))));
        void* const local_pool(
            this->allocator_.allocate(
                self::alignment_count<self::value_size>(
                    local_free_size + self::header_size)));
        if (local_pool == nullptr)
        {
            return nullptr;
        }

        // 新たに確保したチャンクからメモリを分配する。
        auto const local_chunk(
            reinterpret_cast<typename self::chunk_header*>(
                static_cast<std::int8_t*>(local_pool) + local_free_size));
        local_chunk->free_size = local_free_size;
        local_chunk->capacity = local_free_size;
        local_chunk->next_chunk = this->chunk_list_;
        this->chunk_list_ = local_chunk;
        auto const local_memory(
            self::partition_chunk(*local_chunk, in_size, in_alignment));
        PSYQ_ASSERT(local_memory != nullptr);
        return local_memory;
    }

    //-------------------------------------------------------------------------
    private: static void* partition_chunk(
        typename self::chunk_header& io_chunk,
        std::size_t const in_size,
        std::size_t const in_alignment)
    {
        if (io_chunk.free_size < in_size)
        {
            return nullptr;
        }
        void* local_pool(
            reinterpret_cast<std::int8_t*>(&io_chunk) - io_chunk.free_size);
        io_chunk.free_size -= in_size;
        if (in_alignment <= 1)
        {
            return local_pool;
        }
        void* const local_memory(
            std::align(in_alignment, in_size, local_pool, io_chunk.free_size));
        if (local_memory == nullptr)
        {
            io_chunk.free_size += in_size;
        }
        return local_memory;
    }

    private: template<std::size_t template_size>
    static std::size_t alignment_size(std::size_t const in_size)
    {
        return self::alignment_count<template_size>(in_size) * template_size;
    }

    private: template<std::size_t template_size>
    static std::size_t alignment_count(std::size_t const in_size)
    {
        return (in_size + template_size - 1) / template_size;
    }

    //-------------------------------------------------------------------------
    private: typename self::chunk_header* chunk_list_; ///< 先頭チャンク。
    private: std::size_t default_capacity_; ///< チャンク容量のデフォルト値。
    private: template_allocator allocator_; ///< @copydoc allocator_type
};

#endif // !defined(PSYQ_MESSAGE_PACK_POOL_HPP_)
