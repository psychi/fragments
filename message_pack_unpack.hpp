/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief
 */
#ifndef PSYQ_MESSAGE_PACK_UNPACK_HPP_
#define PSYQ_MESSAGE_PACK_UNPACK_HPP_

//#include "psyq/message_pack_template.hpp"

/**
 * @defgroup msgpack_unpack Deserializer
 * @ingroup msgpack
 * @{
 */

struct msgpack_unpacked
{
    msgpack_zone* zone;
    msgpack_object data;
};

bool msgpack_unpack_next(
    msgpack_unpacked* result,
    const char* data,
    size_t len,
    size_t* off);

/** @} */


/**
 * @defgroup msgpack_unpacker Streaming deserializer
 * @ingroup msgpack
 * @{
 */

struct msgpack_unpacker
{
    char* buffer;
    size_t used;
    size_t free;
    size_t off;
    size_t parsed;
    msgpack_zone* zone;
    size_t initial_buffer_size;
    void* ctx;
};

#ifndef MSGPACK_UNPACKER_INIT_BUFFER_SIZE
#define MSGPACK_UNPACKER_INIT_BUFFER_SIZE (64*1024)
#endif

/** @brief ストリームデシリアライザを初期化する。

    初期化されたストリームデシリアライザは、
    msgpack_unpacker_destroy() を使って破棄すること。
 */
bool msgpack_unpacker_init(msgpack_unpacker* mpac, size_t initial_buffer_size);

/** @brief ストリームデシリアライザを破棄する。

    msgpack_unpacker_init() で構築したストリームデシリアライザを破棄する。
 */
void msgpack_unpacker_destroy(msgpack_unpacker* mpac);


/**
 * Creates a streaming deserializer.
 * The created deserializer must be destroyed by msgpack_unpacker_free(msgpack_unpacker*).
 */
msgpack_unpacker* msgpack_unpacker_new(size_t initial_buffer_size);

/**
 * Frees a streaming deserializer created by msgpack_unpacker_new(size_t).
 */
void msgpack_unpacker_free(msgpack_unpacker* mpac);


#ifndef MSGPACK_UNPACKER_RESERVE_SIZE
#define MSGPACK_UNPACKER_RESERVE_SIZE (32*1024)
#endif

/**
 * Reserves free space of the internal buffer.
 * Use this function to fill the internal buffer with
 * msgpack_unpacker_buffer(msgpack_unpacker*),
 * msgpack_unpacker_buffer_capacity(const msgpack_unpacker*) and
 * msgpack_unpacker_buffer_consumed(msgpack_unpacker*).
 */
static inline bool msgpack_unpacker_reserve_buffer(msgpack_unpacker* mpac, size_t size)
{
    if(mpac->free >= size) { return true; }
#if 0
    return msgpack_unpacker_expand_buffer(mpac, size);
#else
    return false;
#endif
}

/**
 * Gets pointer to the free space of the internal buffer.
 * Use this function to fill the internal buffer with
 * msgpack_unpacker_reserve_buffer(msgpack_unpacker*, size_t),
 * msgpack_unpacker_buffer_capacity(const msgpack_unpacker*) and
 * msgpack_unpacker_buffer_consumed(msgpack_unpacker*).
 */
inline char* msgpack_unpacker_buffer(msgpack_unpacker* mpac)
{
    return mpac->buffer + mpac->used;
}

/**
 * Gets size of the free space of the internal buffer.
 * Use this function to fill the internal buffer with
 * msgpack_unpacker_reserve_buffer(msgpack_unpacker*, size_t),
 * msgpack_unpacker_buffer(const msgpack_unpacker*) and
 * msgpack_unpacker_buffer_consumed(msgpack_unpacker*).
 */
inline size_t msgpack_unpacker_buffer_capacity(const msgpack_unpacker* mpac)
{
    return mpac->free;
}

/**
 * Notifies the deserializer that the internal buffer filled.
 * Use this function to fill the internal buffer with
 * msgpack_unpacker_reserve_buffer(msgpack_unpacker*, size_t),
 * msgpack_unpacker_buffer(msgpack_unpacker*) and
 * msgpack_unpacker_buffer_capacity(const msgpack_unpacker*).
 */
static inline void msgpack_unpacker_buffer_consumed(msgpack_unpacker* mpac, size_t size)
{
    mpac->used += size;
    mpac->free -= size;
}

/**
 * Deserializes one object.
 * Returns true if it successes. Otherwise false is returned.
 * @param pac  pointer to an initialized msgpack_unpacked object.
 */
bool msgpack_unpacker_next(msgpack_unpacker* mpac, msgpack_unpacked* pac);

/**
 * Initializes a msgpack_unpacked object.
 * The initialized object must be destroyed by msgpack_unpacked_destroy(msgpack_unpacker*).
 * Use the object with msgpack_unpacker_next(msgpack_unpacker*, msgpack_unpacked*) or
 * msgpack_unpack_next(msgpack_unpacked*, const char*, size_t, size_t*).
 */
static inline void msgpack_unpacked_init(msgpack_unpacked* result)
{
    std::memset(result, 0, sizeof(msgpack_unpacked));
}

/**
 * Destroys a streaming deserializer initialized by msgpack_unpacked().
 */
inline void msgpack_unpacked_destroy(msgpack_unpacked* result)
{
	if(result->zone != NULL) {
		msgpack_zone_free(result->zone);
		result->zone = NULL;
		memset(&result->data, 0, sizeof(msgpack_object));
	}
}

/**
 * Releases the memory zone from msgpack_unpacked object.
 * The released zone must be freed by msgpack_zone_free(msgpack_zone*).
 */
inline msgpack_zone* msgpack_unpacked_release_zone(msgpack_unpacked* result)
{
	if(result->zone != NULL) {
		msgpack_zone* z = result->zone;
		result->zone = NULL;
		return z;
	}
	return NULL;
}



int msgpack_unpacker_execute(msgpack_unpacker* mpac);

msgpack_object msgpack_unpacker_data(msgpack_unpacker* mpac);

msgpack_zone* msgpack_unpacker_release_zone(msgpack_unpacker* mpac);

void msgpack_unpacker_reset_zone(msgpack_unpacker* mpac);

void msgpack_unpacker_reset(msgpack_unpacker* mpac);

static inline size_t msgpack_unpacker_message_size(const msgpack_unpacker* mpac)
{
    return mpac->parsed - mpac->off + mpac->used;
}


/** @} */


// obsolete
typedef enum {
	MSGPACK_UNPACK_SUCCESS				=  2,
	MSGPACK_UNPACK_EXTRA_BYTES			=  1,
	MSGPACK_UNPACK_CONTINUE				=  0,
	MSGPACK_UNPACK_PARSE_ERROR			= -1,
} msgpack_unpack_return;

// obsolete
msgpack_unpack_return msgpack_unpack(
    char const*  const in_data,
    std::size_t  const in_length,
    std::size_t* const io_offset,
    msgpack_zone&      out_zone,
    msgpack_object&    out_result);


inline size_t msgpack_unpacker_parsed_size(const msgpack_unpacker* mpac)
{
    return mpac->parsed;
}

bool msgpack_unpacker_flush_zone(msgpack_unpacker* mpac);

bool msgpack_unpacker_expand_buffer(msgpack_unpacker* mpac, size_t size);

#endif // !defined(PSYQ_MESSAGE_PACK_UNPACK_HPP_)
