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
    CS_HEADER     = 0x00, // nil

    CS_NEVER_USED = 0x01,
    CS_FALSE      = 0x02,
    CS_TRUE       = 0x03,

    CS_BIN8       = 0x04,
    CS_BIN16      = 0x05,
    CS_BIN32      = 0x06,

    CS_EXT_BIN8   = 0x07,
    CS_EXT_BIN16  = 0x08,
    CS_EXT_BIN32  = 0x09,

    CS_FLOAT32    = 0x0a,
    CS_FLOAT64    = 0x0b,
    CS_UINT8      = 0x0c,
    CS_UINT16     = 0x0d,
    CS_UINT32     = 0x0e,
    CS_UINT64     = 0x0f,
    CS_INT8       = 0x10,
    CS_INT16      = 0x11,
    CS_INT32      = 0x12,
    CS_INT64      = 0x13,

    CS_FIX_EXT1   = 0x14,
    CS_FIX_EXT2   = 0x15,
    CS_FIX_EXT4   = 0x16,
    CS_FIX_EXT8   = 0x17,
    CS_FIX_EXT16  = 0x18,

    CS_STR8       = 0x19,
    CS_STR16      = 0x1a,
    CS_STR32      = 0x1b,
    CS_ARRAY16    = 0x1c,
    CS_ARRAY32    = 0x1d,
    CS_MAP16      = 0x1e,
    CS_MAP32      = 0x1f,

    //ACS_BIG_INT_VALUE,
    //ACS_BIG_FLOAT_VALUE,
    ACS_RAW_STRING,
    ACS_RAW_BINARY,
    ACS_RAW_EXTENDED_BINARY,
};

#endif // !defined(PSYQ_MESSAGE_PACK_DEFINE_HPP_)
