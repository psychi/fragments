/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copydoc psyq::message_pack::pool
 */
#ifndef PSYQ_MESSAGE_PACK_POOL_HPP_
#define PSYQ_MESSAGE_PACK_POOL_HPP_

//#include "psyq/message_pack/object.hpp"

/// psyq::message_pack::pool のチャンク容量のデフォルト値。
#ifndef PSYQ_MESSAGE_PACK_POOL_CHUNK_CAPACITY_DEFAULT
#define PSYQ_MESSAGE_PACK_POOL_CHUNK_CAPACITY_DEFAULT 4096
#endif // !defined(PSYQ_MESSAGE_PACK_POOL_CHUNK_CAPACITY_DEFAULT)

#ifndef PSYQ_MESSAGE_PACK_POOL_STD_ALIGN_PATCH
#define PSYQ_MESSAGE_PACK_POOL_STD_ALIGN_PATCH\
    defined(_MSC_VER) && _MSC_VER <= 1700
#endif // !defined(PSYQ_MESSAGE_PACK_POOL_STD_ALGIN_PATCH)

namespace psyq
{
    namespace message_pack
    {
        /// @cond
        template<typename = std::allocator<long long>> class pool;
        /// @endcond
    } // namespace message_pack
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief MessagePackの直列化復元に使うメモリ割当子。
    @tparam template_allocator @copydoc psyq::message_pack::pool::allocator_type
    @sa psyq::message_pack::deserializer
 */
template<typename template_allocator>
class psyq::message_pack::pool
{
    private: typedef pool<template_allocator> this_type; ///< thisが指す値の型。

    /// 使用する、 std::allocator 互換のメモリ割当子。
    public: typedef template_allocator allocator_type;

    private: struct chunk_header
    {
        chunk_header* next_chunk; ///< 次のメモリチャンク。
        std::size_t capacity;     ///< チャンク容量のバイト数。
        std::size_t free_size;    ///< 未使用メモリのバイト数。
    };

    private: enum: std::size_t
    {
        value_size = sizeof(typename this_type::allocator_type::value_type),
        header_size = sizeof(typename this_type::chunk_header),
        header_alignment = sizeof(void*),
    };

    //-------------------------------------------------------------------------
    /// @name 構築と破壊
    //@{
    /** @brief メモリプールを構築する。
        @param[in] in_default_capacity チャンク容量バイト数のデフォルト値。
        @param[in] in_allocator        メモリ割当子の初期値。
     */
    public: explicit pool(
        std::size_t const in_default_capacity =
            PSYQ_MESSAGE_PACK_POOL_CHUNK_CAPACITY_DEFAULT,
        typename this_type::allocator_type in_allocator =
            this_type::allocator_type())
    PSYQ_NOEXCEPT:
        chunk_list_(nullptr),
        default_capacity_(in_default_capacity),
        allocator_(std::move(in_allocator))
    {}

    /** @brief copy構築子。
        @param[in] in_source copy元。
     */
    public: pool(this_type const& in_source):
        chunk_list_(nullptr),
        default_capacity_(in_source.get_default_capacity()),
        allocator_(in_source.get_allocator())
    {}

    /** @brief move構築子。
        @param[in,out] io_source move元。
     */
    public: pool(this_type&& io_source) PSYQ_NOEXCEPT:
        chunk_list_(std::move(io_source.chunk_list_)),
        default_capacity_(std::move(io_source.default_capacity_)),
        allocator_(std::move(io_source.allocator_))
    {
        io_source.chunk_list_ = nullptr;
    }

    /** @brief this_type::allocate() で確保したメモリを、すべて解放する。
     */
    public: ~pool() PSYQ_NOEXCEPT
    {
        auto local_chunk(this->chunk_list_);
        while (local_chunk != nullptr)
        {
            auto const local_capacity(local_chunk->capacity);
            auto const local_memory(
                reinterpret_cast<typename this_type::allocator_type::pointer>(
                    reinterpret_cast<std::int8_t*>(local_chunk)
                        - local_capacity));
            local_chunk = local_chunk->next_chunk;
            this->allocator_.deallocate(
                local_memory,
                this_type::alignment_count<this_type::value_size>(
                    local_capacity + sizeof(typename this_type::chunk_header)));
        }
    }

    /** @brief copy代入演算子。
        @param[in] in_source copy元。
        @return *this
     */
    public: this_type& operator=(this_type const& in_source)
    {
        this->default_capacity_ = in_source.get_default_capacity();
        this->allocator_ = in_source.get_allocator();
        return *this;
    }

    /** @brief move代入演算子。
        @param[in,out] io_source move元。
        @return *this
     */
    public: this_type& operator=(this_type&& io_source) PSYQ_NOEXCEPT
    {
        if (this != &io_source)
        {
            this->~this_type();
            new(this) this_type(std::move(io_source));
        }
        return *this;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 比較
    //@{
    public: bool operator==(this_type const& in_source) const PSYQ_NOEXCEPT
    {
        return this->allocator_ == in_source.get_allocator();
    }
    public: bool operator!=(this_type const& in_source) const PSYQ_NOEXCEPT
    {
        return this->allocator_ != in_source.get_allocator();
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 状態の取得
    //@{
    /** @brief メモリ割当子を取得する。
        @return メモリ割当子。
     */
    public: typename this_type::allocator_type const& get_allocator()
    const PSYQ_NOEXCEPT
    {
        return this->allocator_;
    }

    /** @brief チャンク容量バイト数のデフォルト値を取得する。
        @return チャンク容量バイト数のデフォルト値。
        @sa this_type::allocate()
     */
    public: std::size_t get_default_capacity() const PSYQ_NOEXCEPT
    {
        return this->default_capacity_;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name メモリ確保
    //@{
    /** @brief メモリを確保する。

        確保したメモリは、 this_type::~pool() で解放される。

        @param[in] in_size      確保するメモリのバイト数。
        @param[in] in_alignment 確保するメモリ先頭位置のメモリ境界単位。
        @retval !=nullptr 確保したメモリの先頭位置。
        @retval ==nullptr メモリの確保に失敗した。
     */
    public: void* allocate(
        std::size_t const in_size,
        std::size_t const in_alignment = sizeof(std::int64_t))
    {
        if (in_size <= 0)
        {
            return nullptr;
        }

        // チャンクからメモリを分配する。
        if (this->chunk_list_ != nullptr)
        {
            auto const local_memory(
                this_type::partition_chunk(
                    *this->chunk_list_, in_size, in_alignment));
            if (local_memory != nullptr)
            {
                return local_memory;
            }
        }

        // 新たにチャンクを確保する。
        /** @todo 2014.05.09
            今のところ、新たにチャンクを確保すると、
            既存のチャンクの空き領域が使われなくなる。
            チャンクを空き領域のサイズでソートし、
            既存のチャンクから空き領域を検索するようにしたい。
         */
        auto const local_free_size(
            this_type::alignment_size<this_type::header_alignment>(
                std::max(
                    this_type::header_size < this->get_default_capacity()?
                        this->get_default_capacity() - this_type::header_size: 0,
                    std::max(in_size + in_alignment - 1, in_alignment * 2))));
        void* const local_pool(
            this->allocator_.allocate(
                this_type::alignment_count<this_type::value_size>(
                    local_free_size + this_type::header_size)));
        if (local_pool == nullptr)
        {
            return nullptr;
        }

        // 新たに確保したチャンクからメモリを分配する。
        auto const local_chunk(
            reinterpret_cast<typename this_type::chunk_header*>(
                static_cast<std::int8_t*>(local_pool) + local_free_size));
        local_chunk->free_size = local_free_size;
        local_chunk->capacity = local_free_size;
        local_chunk->next_chunk = this->chunk_list_;
        this->chunk_list_ = local_chunk;
        auto const local_memory(
            this_type::partition_chunk(*local_chunk, in_size, in_alignment));
        PSYQ_ASSERT(local_memory != nullptr);
        return local_memory;
    }
    //@}
    //-------------------------------------------------------------------------
    /** @brief チャンクからメモリを分配する。
        @param[in,out] io_chunk     メモリを分配するチャンク。
        @param[in]     in_size      分配するメモリのバイト数。
        @param[in]     in_alignment 分配するメモリの境界単位。
        @retval !=nullptr 分配したメモリの先頭位置。
        @retval ==nullptr 分配に失敗。メモリ不足。
     */
    private: static void* partition_chunk(
        typename this_type::chunk_header& io_chunk,
        std::size_t const in_size,
        std::size_t const in_alignment)
    {
        if (io_chunk.free_size < in_size)
        {
            return nullptr;
        }
        void* local_pool(
            reinterpret_cast<std::int8_t*>(&io_chunk) - io_chunk.free_size);
        auto local_free_size(io_chunk.free_size);
        void* const local_memory(
            std::align(in_alignment, in_size, local_pool, local_free_size));
#if PSYQ_MESSAGE_PACK_POOL_STD_ALIGN_PATCH
        /** @note 2014.05.12
            VisualStudio2012では、 std::align() の実装に問題がある？
            「_Space -= _Off + _Size」と実装されてたので、その対応をしておく。
         */
        local_free_size += (local_memory != nullptr? in_size: 0);
#endif // PSYQ_MESSAGE_PACK_POOL_STD_ALIGN_PATCH
        if (local_memory == nullptr || local_free_size < in_size)
        {
            return nullptr;
        }
        io_chunk.free_size = local_free_size - in_size;
        return local_memory;
    }

    private: template<std::size_t template_size>
    static std::size_t alignment_size(std::size_t const in_size)
    {
        return this_type::alignment_count<template_size>(in_size) * template_size;
    }

    private: template<std::size_t template_size>
    static std::size_t alignment_count(std::size_t const in_size)
    {
        return (in_size + template_size - 1) / template_size;
    }

    //-------------------------------------------------------------------------
    /// チャンクリストの先頭。
    private: typename this_type::chunk_header* chunk_list_;
    /// チャンク容量のデフォルト値。
    private: std::size_t default_capacity_;
    /// @copydoc allocator_type
    private: typename this_type::allocator_type allocator_;

}; // class psyq::message_pack::pool

#endif // !defined(PSYQ_MESSAGE_PACK_POOL_HPP_)
