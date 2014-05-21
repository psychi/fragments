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

#ifndef PSYQ_MESSAGE_PACK_MEMORY_POOL_STD_ALGIN_PATCH
#define PSYQ_MESSAGE_PACK_MEMORY_POOL_STD_ALGIN_PATCH\
    defined(_MSC_VER) && _MSC_VER <= 1700
#endif // !defined(PSYQ_MESSAGE_PACK_MEMORY_POOL_STD_ALGIN_PATCH)

namespace psyq
{
    namespace message_pack
    {
        /// @cond
        template<typename = std::allocator<std::int64_t>> class pool;
        /// @endcond
    }
    namespace internal
    {
        /// @cond
        template<typename = std::int8_t const*> class message_pack_istream;
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
        header_alignment = sizeof(void*),
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
        chunk_list_(std::move(io_source.chunk_list_)),
        default_capacity_(std::move(io_source.default_capacity_)),
        allocator_(std::move(io_source.allocator_))
    {
        io_source.chunk_list_ = nullptr;
    }

    /** @brief self::allocate() で確保したメモリを、すべて解放する。
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
        if (in_size <= 0)
        {
            return nullptr;
        }

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
        /** @todo 2014.05.09
            今のところ、新たにチャンクを確保すると、
            既存のチャンクの空き領域が使われなくなる。
            チャンクを空き領域のサイズでソートし、
            既存のチャンクから空き領域を検索するようにしたい。
         */
        auto const local_free_size(
            self::alignment_size<self::header_alignment>(
                std::max(
                    self::header_size < this->get_default_capacity()?
                        this->get_default_capacity() - self::header_size: 0,
                    std::max(in_size + in_alignment - 1, in_alignment * 2))));
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
    /** @brief チャンクからメモリを分配する。
        @param[in,out] io_chunk     メモリを分配するチャンク。
        @param[in]     in_size      分配するメモリのバイト数。
        @param[in]     in_alignment 分配するメモリの境界単位。
        @retval !=nullptr 分配したメモリの先頭位置。
        @retval ==nullptr 分配に失敗。メモリ不足。
     */
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
        auto local_free_size(io_chunk.free_size);
        void* const local_memory(
            std::align(in_alignment, in_size, local_pool, local_free_size));
#if PSYQ_MESSAGE_PACK_MEMORY_POOL_STD_ALGIN_PATCH
        /** @note 2014.05.12
            VisualStudio2012では、 std::align() の実装に問題がある？
            「_Space -= _Off + _Size」と実装されてたので、その対応をしておく。
         */
        local_free_size += (local_memory != nullptr? in_size: 0);
#endif // PSYQ_MESSAGE_PACK_MEMORY_POOL_STD_ALGIN_PATCH
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

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<typename template_iterator>
class psyq::internal::message_pack_istream
{
    private: typedef message_pack_istream<template_iterator> self;

    public: typedef template_iterator iterator;
    public: typedef typename
        std::iterator_traits<template_iterator>::value_type char_type;
    public: typedef std::char_traits<typename self::char_type> traits_type;
    public: typedef typename self::traits_type::int_type int_type;
    //public: typedef typename self::traits_type::pos_type pos_type;
    //public: typedef typename self::traits_type::off_type off_type;
    public: typedef std::size_t pos_type;
    public: typedef int off_type;

    public: message_pack_istream(
        typename self::iterator const in_begin,
        typename self::iterator const in_end)
    :
        current_(in_begin),
        begin_(in_begin),
        end_(in_end),
        size_(std::distance(in_begin, in_end))
    {}

    public: typename self::iterator const& begin() const
    {
        return this->begin_;
    }
    public: typename self::iterator const& end() const
    {
        return this->end_;
    }
    public: typename self::iterator const& current() const
    {
        return this->current_;
    }

    public: bool eof() const
    {
        return this->end() <= this->current();
    }
    public: bool fail() const
    {
        return false;
    }

    public: typename self::char_type get()
    {
        if (this->eof())
        {
            return 0;
        }
        auto const local_char(*this->current());
        ++this->current_;
        return local_char;
    }

    public: self& read(
        typename self::char_type* const out_buffer,
        std::size_t const in_length)
    {
        auto const local_length(
            std::min<std::size_t>(in_length, this->end() - this->current()));
        std::memcpy(
            out_buffer,
            this->current(),
            sizeof(typename self::char_type) * local_length);
        this->current_ += local_length;
        return *this;
    }

    public: typename self::pos_type tellg() const
    {
        return this->current() - this->begin();
    }

    public: self& seekg(typename self::pos_type const in_offset)
    {
        this->current_ = in_offset < this->size_?
            std::next(this->begin_, in_offset): this->end_;
        return *this;
    }
    public: self& seekg(
        typename self::off_type const in_offset,
        std::ios::seek_dir const in_direction)
    {
        switch (in_direction)
        {
        case std::ios::beg:
            if (0 < in_offset)
            {
                return this->seekg(
                    static_cast<typename self::pos_type>(in_offset));
            }
            this->current_ = this->begin_;
            break;
        case std::ios::end:
            if (0 <= in_offset)
            {
                this->current_ = this->end_;
            }
            else if (this->size_ <= unsigned(-in_offset))
            {
                this->current_ = this->begin_;
            }
            else
            {
                this->current_ = std::prev(this->begin_, -in_offset);
            }
            break;
        case std::ios::cur:
            if (in_offset < 0)
            {
                auto const local_distance(
                    std::distance(this->begin_, this->current_));
                this->current_ = -in_offset < local_distance?
                    std::prev(this->current_, -in_offset): this->begin_;
            }
            else if (0 < in_offset)
            {
                auto const local_distance(
                    std::distance(this->current_, this->end_));
                this->current_ = in_offset < local_distance?
                    std::next(this->current_, in_offset): this->end_;
            }
            break;
        }
        return *this;
    }

    private: typename self::iterator current_;
    private: typename self::iterator begin_;
    private: typename self::iterator end_;
    private: std::size_t size_;
};

#endif // !defined(PSYQ_MESSAGE_PACK_POOL_HPP_)
