#include <cstdlib>
//#include "psyq/message_pack_zone.hpp"

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
struct msgpack_zone_chunk
{
    msgpack_zone_chunk* next;
    /* data ... */
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
//-----------------------------------------------------------------------------
inline bool init_chunk_list(
    msgpack_zone_chunk_list& out_chunk_list,
    std::size_t const in_chunk_size)
{
    msgpack_zone_chunk* const local_chunk(
        static_cast<msgpack_zone_chunk*>(
            std::malloc(sizeof(msgpack_zone_chunk) + in_chunk_size)));
    if (local_chunk == nullptr)
    {
        return false;
    }

    out_chunk_list.head = local_chunk;
    out_chunk_list.free = in_chunk_size;
    out_chunk_list.ptr = reinterpret_cast<char*>(local_chunk)
        + sizeof(msgpack_zone_chunk);
    local_chunk->next = nullptr;
    return true;
}

static inline void destroy_chunk_list(msgpack_zone_chunk_list* cl)
{
	msgpack_zone_chunk* c = cl->head;
	while(true) {
		msgpack_zone_chunk* n = c->next;
		free(c);
		if(n != nullptr) {
			c = n;
		} else {
			break;
		}
	}
}

static inline void clear_chunk_list(msgpack_zone_chunk_list* cl, size_t chunk_size)
{
	msgpack_zone_chunk* c = cl->head;
	while(true) {
		msgpack_zone_chunk* n = c->next;
		if(n != nullptr) {
			free(c);
			c = n;
		} else {
			cl->head = c;
			break;
		}
	}
	cl->head->next = nullptr;
	cl->free = chunk_size;
	cl->ptr  = ((char*)cl->head) + sizeof(msgpack_zone_chunk);
}

//-----------------------------------------------------------------------------
void* msgpack_zone_malloc_expand(
    msgpack_zone* const io_zone,
    std::size_t const in_size)
{
    msgpack_zone_chunk_list& local_chunk_list(io_zone->chunk_list);
    std::size_t local_size(io_zone->chunk_size);
    while (local_size < in_size)
    {
        local_size *= 2;
    }

    msgpack_zone_chunk* const local_chunk(
        static_cast<msgpack_zone_chunk*>(
            std::malloc(sizeof(msgpack_zone_chunk) + local_size)));
    if (local_chunk == nullptr)
    {
        return nullptr;
    }
    char* const local_ptr(
        reinterpret_cast<char*>(local_chunk) + sizeof(msgpack_zone_chunk));
    local_chunk->next = local_chunk_list.head;
    local_chunk_list.head = local_chunk;
    local_chunk_list.free = local_size - in_size;
    local_chunk_list.ptr  = local_ptr + in_size;
    return local_ptr;
}

//-----------------------------------------------------------------------------
inline void init_finalizer_array(
    msgpack_zone_finalizer_array& out_finalizer_array)
{
    out_finalizer_array.tail = nullptr;
    out_finalizer_array.end = nullptr;
    out_finalizer_array.array = nullptr;
}

static inline void call_finalizer_array(msgpack_zone_finalizer_array* fa)
{
	msgpack_zone_finalizer* fin = fa->tail;
	for(; fin != fa->array; --fin) {
		(*(fin-1)->func)((fin-1)->data);
	}
}

static inline void destroy_finalizer_array(msgpack_zone_finalizer_array* fa)
{
	call_finalizer_array(fa);
	free(fa->array);
}

static inline void clear_finalizer_array(msgpack_zone_finalizer_array* fa)
{
	call_finalizer_array(fa);
	fa->tail = fa->array;
}

bool msgpack_zone_push_finalizer_expand(msgpack_zone* zone,
		void (*func)(void* data), void* data)
{
	msgpack_zone_finalizer_array* const fa = &zone->finalizer_array;

	const size_t nused = fa->end - fa->array;

	size_t nnext;
	if(nused == 0) {
		nnext = (sizeof(msgpack_zone_finalizer) < 72/2) ?
				72 / sizeof(msgpack_zone_finalizer) : 8;

	} else {
		nnext = nused * 2;
	}

	msgpack_zone_finalizer* tmp =
		(msgpack_zone_finalizer*)realloc(fa->array,
				sizeof(msgpack_zone_finalizer) * nnext);
	if(tmp == nullptr) {
		return false;
	}

	fa->array  = tmp;
	fa->end    = tmp + nnext;
	fa->tail   = tmp + nused;

	fa->tail->func = func;
	fa->tail->data = data;

	++fa->tail;

	return true;
}


bool msgpack_zone_is_empty(msgpack_zone* zone)
{
	msgpack_zone_chunk_list* const cl = &zone->chunk_list;
	msgpack_zone_finalizer_array* const fa = &zone->finalizer_array;
	return cl->free == zone->chunk_size && cl->head->next == nullptr &&
		fa->tail == fa->array;
}


void msgpack_zone_destroy(msgpack_zone* zone)
{
	destroy_finalizer_array(&zone->finalizer_array);
	destroy_chunk_list(&zone->chunk_list);
}

void msgpack_zone_clear(msgpack_zone* zone)
{
	clear_finalizer_array(&zone->finalizer_array);
	clear_chunk_list(&zone->chunk_list, zone->chunk_size);
}

bool msgpack_zone_init(msgpack_zone* zone, size_t chunk_size)
{
	zone->chunk_size = chunk_size;

	if(!init_chunk_list(zone->chunk_list, chunk_size)) {
		return false;
	}

	init_finalizer_array(zone->finalizer_array);

	return true;
}

//-----------------------------------------------------------------------------
/** @brief ゾーンを生成する。
    @param[in] in_chunk_size ゾーンが持つチャンクのバイト数。
 */
msgpack_zone* msgpack_zone_new(std::size_t const in_chunk_size)
{
    msgpack_zone* const local_zone(
        static_cast<msgpack_zone*>(
            std::malloc(sizeof(msgpack_zone) + in_chunk_size)));
    if (local_zone != nullptr)
    {
        local_zone->chunk_size = in_chunk_size;
        if (init_chunk_list(local_zone->chunk_list, in_chunk_size))
        {
            init_finalizer_array(local_zone->finalizer_array);
            return local_zone;
        }
        std::free(local_zone);
    }
    return nullptr;
}

//-----------------------------------------------------------------------------
/** @brief ゾーンを破棄する。
    @param[in,out] io_zone 破棄するゾーン。
 */
void msgpack_zone_free(msgpack_zone* const io_zone)
{
    if (io_zone != nullptr)
    {
        msgpack_zone_destroy(io_zone);
        std::free(io_zone);
    }
}
