/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief
 */
#ifndef PSYQ_MESSAGE_PACK_ZONE_HPP_
#define PSYQ_MESSAGE_PACK_ZONE_HPP_

#ifndef MSGPACK_ZONE_CHUNK_SIZE
#define MSGPACK_ZONE_CHUNK_SIZE 8192
#endif

#ifndef MSGPACK_ZONE_ALIGN
#define MSGPACK_ZONE_ALIGN sizeof(double)
#endif

//-----------------------------------------------------------------------------
struct msgpack_zone_chunk;

struct msgpack_zone_finalizer
{
    void (*func)(void* data);
    void* data;
};

struct msgpack_zone_finalizer_array
{
    msgpack_zone_finalizer* tail;
    msgpack_zone_finalizer* end;
    msgpack_zone_finalizer* array;
};

struct msgpack_zone_chunk_list
{
    std::size_t free;
    char* ptr;
    msgpack_zone_chunk* head;
};

struct msgpack_zone
{
    msgpack_zone_chunk_list chunk_list;
    msgpack_zone_finalizer_array finalizer_array;
    std::size_t chunk_size;
};

//-----------------------------------------------------------------------------
bool msgpack_zone_init(msgpack_zone* zone, size_t chunk_size);
void msgpack_zone_destroy(msgpack_zone* zone);

//-----------------------------------------------------------------------------
msgpack_zone* msgpack_zone_new(std::size_t const in_chunk_size);
void msgpack_zone_free(msgpack_zone* const io_zone);

//-----------------------------------------------------------------------------
bool msgpack_zone_push_finalizer_expand(
    msgpack_zone* zone,
    void (*func)(void* data),
    void* data);
bool msgpack_zone_is_empty(msgpack_zone* zone);
void msgpack_zone_clear(msgpack_zone* zone);

//-----------------------------------------------------------------------------
/** @brief ゾーンを拡張し、メモリを割り当てる。
    @param[in,out] io_zone 拡張するゾーン。
    @param[in]     in_size 拡張するバイト数。
    @return 割り当てたメモリ。
 */
void* msgpack_zone_malloc_expand(
    msgpack_zone* const io_zone,
    std::size_t const in_size);

//-----------------------------------------------------------------------------
/** @brief ゾーンから、メモリを割り当てる。

    割り当てるメモリの大きさは、メモリ境界単位となるよう調整されない。

    @param[in,out] io_zone メモリを割り当てるゾーン。
    @param[in]     in_size 割り当てるメモリのバイト数。
    @return 割り当てたメモリ。
 */
inline void* msgpack_zone_malloc_no_align(
    msgpack_zone* const io_zone,
    std::size_t const in_size)
{
    msgpack_zone_chunk_list& local_chunk_list(io_zone->chunk_list);
    if (local_chunk_list.free < in_size)
    {
        // チャンクが足りないので、ゾーンを拡張する。
        return msgpack_zone_malloc_expand(io_zone, in_size);
    }

    // チャンクからメモリを割り当てる。
    char* const local_ptr(local_chunk_list.ptr);
    local_chunk_list.free -= in_size;
    local_chunk_list.ptr  += in_size;
    return local_ptr;
}

//-----------------------------------------------------------------------------
/** @brief ゾーンから、メモリを割り当てる。

    割り当てるメモリの大きさは、必ずメモリ境界単位となるように調整される。

    @param[in,out] io_zone メモリを割り当てるゾーン。
    @param[in]     in_size 割り当てるメモリのバイト数。
    @return 割り当てたメモリ。
 */
inline void* msgpack_zone_malloc(
    msgpack_zone* const io_zone,
    std::size_t const in_size)
{
    return msgpack_zone_malloc_no_align(
        io_zone,
        (in_size + MSGPACK_ZONE_ALIGN - 1) & ~(MSGPACK_ZONE_ALIGN - 1));
}

//-----------------------------------------------------------------------------
/** @brief
    @param[in,out] io_zone 後始末関数を追加するゾーン。
    @param[in]     in_func 追加する後始末関数
    @param[in]     in_data 追加する後始末関数の引数
 */
inline bool msgpack_zone_push_finalizer(
    msgpack_zone* const io_zone,
    void (*in_func)(void* in_data),
    void* const in_data)
{
    auto& local_finalizer_array(io_zone->finalizer_array);
    auto const local_finalizer_tail(local_finalizer_array.tail);
    if (local_finalizer_array.end == local_finalizer_tail)
    {
        return msgpack_zone_push_finalizer_expand(io_zone, in_func, in_data);
    }

    local_finalizer_tail->func = in_func;
    local_finalizer_tail->data = in_data;
    ++local_finalizer_array.tail;
    return true;
}

//-----------------------------------------------------------------------------
inline void msgpack_zone_swap(msgpack_zone* io_left, msgpack_zone* io_right)
{
    auto local_left(*io_left);
    *io_left = *io_right;
    *io_right = local_left;
}

#endif // !defined(PSYQ_MESSAGE_PACK_ZONE_HPP_)
