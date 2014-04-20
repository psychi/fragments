/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief
 */
#ifndef PSYQ_MESSAGE_PACK_DEFINE_HPP_
#define PSYQ_MESSAGE_PACK_DEFINE_HPP_

#ifndef MSGPACK_EMBED_STACK_SIZE
#define MSGPACK_EMBED_STACK_SIZE 32
#endif // !defined(MSGPACK_EMBED_STACK_SIZE)

//-----------------------------------------------------------------------------
enum msgpack_unpack_state
{
    CS_HEADER       = 0x00,  // nil

    //CS_           = 0x01,
    //CS_           = 0x02,  // false
    //CS_           = 0x03,  // true

    //CS_           = 0x04,
    //CS_           = 0x05,
    //CS_           = 0x06,
    //CS_           = 0x07,

    //CS_           = 0x08,
    //CS_           = 0x09,
    CS_FLOAT        = 0x0a,
    CS_DOUBLE       = 0x0b,
    CS_UINT_8       = 0x0c,
    CS_UINT_16      = 0x0d,
    CS_UINT_32      = 0x0e,
    CS_UINT_64      = 0x0f,
    CS_INT_8        = 0x10,
    CS_INT_16       = 0x11,
    CS_INT_32       = 0x12,
    CS_INT_64       = 0x13,

    //CS_           = 0x14,
    //CS_           = 0x15,
    //CS_BIG_INT_16   = 0x16,
    //CS_BIG_INT_32   = 0x17,
    //CS_BIG_FLOAT_16 = 0x18,
    //CS_BIG_FLOAT_32 = 0x19,
    CS_RAW_16       = 0x1a,
    CS_RAW_32       = 0x1b,
    CS_ARRAY_16     = 0x1c,
    CS_ARRAY_32     = 0x1d,
    CS_MAP_16       = 0x1e,
    CS_MAP_32       = 0x1f,

    //ACS_BIG_INT_VALUE,
    //ACS_BIG_FLOAT_VALUE,
    ACS_RAW_VALUE,
};

//-----------------------------------------------------------------------------
enum msgpack_container_type
{
    CT_ARRAY_ITEM, ///< 配列の要素。
    CT_MAP_KEY,    ///< 連想配列の要素のキー。
    CT_MAP_VALUE,  ///< 連想配列の要素の値。
};

#endif // !defined(PSYQ_MESSAGE_PACK_DEFINE_HPP_)
