/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief
 */
#ifndef PSYQ_MESSAGE_PACK_OBJECT_HPP_
#define PSYQ_MESSAGE_PACK_OBJECT_HPP_

enum msgpack_object_type
{
    MSGPACK_OBJECT_NIL              = 0x0, ///< 空
    MSGPACK_OBJECT_BOOLEAN          = 0x1, ///< 真偽値
    MSGPACK_OBJECT_POSITIVE_INTEGER = 0x2, ///< 無符号整数
    MSGPACK_OBJECT_NEGATIVE_INTEGER = 0x3, ///< 有符号整数
    MSGPACK_OBJECT_DOUBLE           = 0x4, ///< IEEE形式の浮動小数点実数
    MSGPACK_OBJECT_RAW              = 0x5, ///< バイト列
    MSGPACK_OBJECT_ARRAY            = 0x6, ///< 配列
    MSGPACK_OBJECT_MAP              = 0x7, ///< 連想配列
};

struct msgpack_object;
struct msgpack_object_kv;

/// MessagePackオブジェクトの配列。
struct msgpack_object_array
{
    msgpack_object* data; ///< オブジェクト配列の先頭位置。
    std::size_t size;  ///< オブジェクト配列の要素数。
} ;

/// MessagePackオブジェクトの連想配列。
struct msgpack_object_map
{
    msgpack_object_kv* data; ///< オブジェクト連想配列の先頭位置。
    std::size_t size;        ///< オブジェクト連想配列の要素数。
};

/// MessagePackバイト列。
struct msgpack_object_raw
{
    char const* data; ///< バイト列の先頭位置。
    std::size_t size; ///< 要素の数。
};

/// MessagePackオブジェクトの値。
union msgpack_object_union
{
    bool boolean;                   ///< 真偽値
    std::uint64_t positive_integer; ///< 0以上の整数
    std::int64_t negative_integer;  ///< 0未満の整数
    double floating_point;          ///< 浮動小数点実数
    msgpack_object_array array;     ///< MessagePackオブジェクト配列
    msgpack_object_map map;         ///< MessagePackオブジェクト連想配列。
    msgpack_object_raw raw;         ///< バイト列
};

/// MessagePackオブジェクト。
struct msgpack_object
{
    msgpack_object_union via; ///< オブジェクトが持つ値。
    msgpack_object_type type; ///< オブジェクトが持つ値の種別。
};

/// MessagePackオブジェクト連想配列の要素。
struct msgpack_object_kv
{
    msgpack_object key; ///< 要素のキー。
    msgpack_object val; ///< 要素の値。
};

void msgpack_object_print(FILE* out, msgpack_object o);

bool msgpack_object_equal(const msgpack_object x, const msgpack_object y);

#endif // PSYQ_MESSAGE_PACK_OBJECT_HPP_
