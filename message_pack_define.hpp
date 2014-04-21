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
/// MessagePackの直列化形式の種別。
enum msgpack_unpack_state
{
    CS_HEADER       = 0x00, // nil

    //CS_           = 0x01,
    //CS_           = 0x02, // false
    //CS_           = 0x03, // true

    CS_BIN_8        = 0x04,
    CS_BIN_16       = 0x05,
    CS_BIN_32       = 0x06,

    //CS_EXT_8        = 0x07,
    //CS_EXT_16       = 0x08,
    //CS_EXT_32       = 0x09,

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

    //CS_FIXEXT_1        = 0x14,
    //CS_FIXEXT_2        = 0x15,
    //CS_FIXEXT_4        = 0x16,
    //CS_FIXEXT_8        = 0x17,
    //CS_FIXEXT_16       = 0x18,

    CS_RAW_8        = 0x19, // str8
    CS_RAW_16       = 0x1a, // str16
    CS_RAW_32       = 0x1b, // str32
    CS_ARRAY_16     = 0x1c,
    CS_ARRAY_32     = 0x1d,
    CS_MAP_16       = 0x1e,
    CS_MAP_32       = 0x1f,

    //ACS_BIG_INT_VALUE,
    //ACS_BIG_FLOAT_VALUE,
    ACS_RAW_VALUE,
};

#endif // !defined(PSYQ_MESSAGE_PACK_DEFINE_HPP_)
