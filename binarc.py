#!/usr/bin/python
# -*- coding: utf-8 -*-

import struct
from io import BytesIO

_BINARC_TAG_BITS_SIZE = 32
_BINARC_TAG_BITS_MASK = (1 << _BINARC_TAG_BITS_SIZE) - 1
_BINARC_TAG_FORMAT_BITS_SIZE = 4
_BINARC_TAG_FORMAT_BITS_MAX = (1 << _BINARC_TAG_FORMAT_BITS_SIZE) - 1
_BINARC_TAG_FORMAT_BITS_POSITION = _BINARC_TAG_BITS_SIZE - _BINARC_TAG_FORMAT_BITS_SIZE
_BINARC_IMMEDIATE_BITS_MASK = (1 << _BINARC_TAG_FORMAT_BITS_POSITION) - 1

_BINARC_FORMAT_NIL                =  0 # 空。
_BINARC_FORMAT_BOOLEAN            =  1 # 真偽値。
_BINARC_FORMAT_STRING             =  2 # 文字列。
_BINARC_FORMAT_EXTENDED           =  3 # 拡張バイト列。
_BINARC_FORMAT_ARRAY              =  4 # 配列。
_BINARC_FORMAT_MAP                =  5 # 辞書。
_BINARC_FORMAT_UNSIGNED_IMMEDIATE =  6 # 無符号整数。
_BINARC_FORMAT_UNSIGNED_32        =  7 # 32ビット無符号整数。
_BINARC_FORMAT_UNSIGNED_64        =  8 # 64ビット無符号整数。
_BINARC_FORMAT_NEGATIVE_IMMEDIATE =  9 # 負整数。
_BINARC_FORMAT_NEGATIVE_32        = 10 # 32ビット負整数。
_BINARC_FORMAT_NEGATIVE_64        = 11 # 64ビット負整数。
_BINARC_FORMAT_FLOATING_32        = 12 # IEEE754単精度浮動小数点数。
_BINARC_FORMAT_FLOATING_64        = 13 # IEEE754倍精度浮動小数点数。

#ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class _SerializeNode(object):

    def __init__(self, in_format, in_value, in_hash=0xffffffff):
        self._format = in_format
        self._value = in_value
        self._hash = in_hash & 0xffffffff

    #--------------------------------------------------------------------------
    ## @brief 値から、binarc形式のバイト列を構築する。
    #  @param[in] in_value binarc形式のバイト列にする値。
    #  @return binarc形式のバイト列。
    @staticmethod
    def make_binary(in_value):
        # シリアライズ木とスカラー値のバイト列を構築する。
        local_value_map = {}
        local_tree = _SerializeNode._make_node(
            local_value_map, in_value)
        local_scalar_binary = _SerializeNode._make_scalar_binary(
            local_value_map.values())
        # binarcヘッダをストリームへ出力する
        local_stream = BytesIO()
        local_stream.write(
            struct.pack(
                '<I',
                # 'RooT'
                (ord('R') << 24) |
                (ord('o') << 16) |
                (ord('o') <<  8) |
                (ord('T'))))
        assert local_stream.tell() % 4 == 0
        # タグ木をストリームへ出力する
        local_scalar_offset = local_tree._initialize_container_offset(
            local_stream.tell() // 4 + 1)
        local_scalar_offset = ((local_scalar_offset + 1) // 2) * 2
        local_tree._write_node_tag(local_stream, local_scalar_offset)
        if isinstance(local_tree._value, tuple):
            local_tree._write_tag_tree(local_stream, local_scalar_offset)
        # スカラ値のバイト列をストリームへ出力する。
        local_stream.write(
            struct.pack(
                str(local_scalar_offset * 4 - local_stream.tell()) + 'x'))
        local_stream.write(local_scalar_binary)
        return local_stream.getvalue()

    #--------------------------------------------------------------------------
    ## @brief ノードのスカラー値をパックしたバイト列を構築する。
    #  @param[in] in_node_list _SerializeNode インスタンスの配列。
    #  @return ノードのスカラー値をパックしたバイト列。
    @staticmethod
    def _make_scalar_binary(in_node_list):
        # 先に64ビット境界値をパックし、後で32ビット境界値をパックする。
        local_body_stream = BytesIO()
        local_body_stream.write(
            struct.pack(
                '<I4x',
                (ord('B') << 24) |
                (ord('O') << 16) |
                (ord('D') <<  8) |
                (ord('y'))))
        for local_node in in_node_list:
            local_node._write_64bits_scalar(local_body_stream)
        for local_node in in_node_list:
            local_node._write_32bits_scalar(local_body_stream)
        return local_body_stream.getvalue()

    ## @brief 64ビット境界のスカラー値を出力する。
    #  @param[out] out_stream スカラー値を出力するストリーム。
    def _write_64bits_scalar(self, out_stream):
        if self._format == _BINARC_FORMAT_FLOATING_64:
            local_binary_format = '<d'
        elif self._format == _BINARC_FORMAT_UNSIGNED_64:
            local_binary_format = '<Q'
        elif self._format == _BINARC_FORMAT_NEGATIVE_64:
            local_binary_format = '<q'
        else:
            return
        local_current_size = out_stream.tell()
        assert local_current_size % 8 == 0
        out_stream.write(struct.pack(local_binary_format, self._value))
        self._scalar_offset = local_current_size // 4

    ## @brief 32ビット境界のスカラー値を出力する。
    #  @param[out] out_stream スカラー値を出力するストリーム。
    def _write_32bits_scalar(self, out_stream):
        local_current_size = out_stream.tell()
        assert local_current_size % 4 == 0
        if self._format == _BINARC_FORMAT_STRING:
            local_length = len(self._value)
            assert local_length < (1 << 32)
            out_stream.write(
                struct.pack(
                    ''.join(('<II', str(((local_length + 3) // 4) * 4), 's')),
                    local_length,
                    self._hash,
                    self._value))
        else:
            if self._format == _BINARC_FORMAT_UNSIGNED_32:
                local_binary_format = '<I'
            elif self._format == _BINARC_FORMAT_NEGATIVE_32:
                local_binary_format = '<i'
            elif self._format == _BINARC_FORMAT_FLOATING_32:
                local_binary_format = '<f'
            else:
                ## @todo 拡張バイト列のパックが未実装。
                assert self._format != _BINARC_FORMAT_EXTENDED
                return
            out_stream.write(struct.pack(local_binary_format, self._value))
        self._scalar_offset = local_current_size // 4

    #--------------------------------------------------------------------------
    ## @brief コンテナのオフセット値を初期化する。
    #  @param[in] in_offset コンテナのオフセット値。
    #  @return コンテナ末尾へのオフセット値。
    def _initialize_container_offset(self, in_offset):
        if isinstance(self._value, tuple):
            self._container_offset = in_offset
            local_offset = in_offset + 1 + len(self._value)
            if self._format == _BINARC_FORMAT_MAP:
                local_offset += 1
            else:
                assert self._format == _BINARC_FORMAT_ARRAY
            for local_sub_node in self._value:
                local_offset = local_sub_node._initialize_container_offset(
                    local_offset)
            return local_offset
        else:
            return in_offset

    ## @brief ノードタグ木を出力する。
    #  @param[out] out_stream       ノードタグを出力するストリーム。
    #  @param[in]  in_scalar_offset 値バイト列の先頭位置へのオフセット値。
    def _write_tag_tree(self, out_stream, in_scalar_offset):
        assert out_stream.tell() == self._container_offset * 4
        if self._format == _BINARC_FORMAT_MAP:
            assert len(self._value) % 2 == 0
            local_length = len(self._value) / 2
            # 値コンテナ、キーコンテナの順に出力する。
            _SerializeNode._write_tag_container(
                out_stream, in_scalar_offset, self._value[:local_length])
            _SerializeNode._write_tag_container(
                out_stream, in_scalar_offset, self._value[local_length:])
        else:
            assert self._format == _BINARC_FORMAT_ARRAY
            _SerializeNode._write_tag_container(
                out_stream, in_scalar_offset, self._value)
        # 下位のコンテナを出力する。
        for local_sub_node in self._value:
            if isinstance(local_sub_node._value, tuple):
                local_sub_node._write_tag_tree(out_stream, in_scalar_offset)

    @staticmethod
    def _write_tag_container(out_stream, in_scalar_offset, in_container):
        # コンテナの要素数を出力する。
        local_length = len(in_container)
        assert local_length < ((1 << 32) - 1)
        out_stream.write(struct.pack('<I', local_length))
        # コンテナのタグ配列を出力する
        for local_sub_node in in_container:
            local_sub_node._write_node_tag(out_stream, in_scalar_offset)

    ## @brief ノードタグを出力する。
    #  @param[out] out_stream       ノードタグを出力するストリーム。
    #  @param[in]  in_scalar_offset 値バイト列の先頭位置へのオフセット値。
    def _write_node_tag(self, out_stream, in_scalar_offset):
        local_tag = self._format << _BINARC_TAG_FORMAT_BITS_POSITION
        if (self._format == _BINARC_FORMAT_UNSIGNED_IMMEDIATE or
            self._format == _BINARC_FORMAT_NEGATIVE_IMMEDIATE
        ):
            local_tag |= self._value & _BINARC_IMMEDIATE_BITS_MASK
        elif isinstance(self._value, tuple):
            assert 0 <= self._container_offset
            assert self._container_offset <= _BINARC_IMMEDIATE_BITS_MASK
            local_tag |= self._container_offset
        else:
            local_scalar_offset = in_scalar_offset + self._scalar_offset
            assert 0 <= local_scalar_offset
            assert local_scalar_offset <= _BINARC_IMMEDIATE_BITS_MASK
            local_tag |= local_scalar_offset
        assert out_stream.tell() % 4 == 0
        out_stream.write(struct.pack('<I', local_tag))

    #--------------------------------------------------------------------------
    ## @brief 値をシリアライズするノードを構築する。
    #  @param[in,out] io_value_map 値を登録する辞書。
    #  @param[in]     in_value     シリアライズする値。
    #  @return 値をシリアライズする _SerializeNode インスタンス。
    @staticmethod
    def _make_node(io_value_map, in_value):
        if isinstance(in_value, bool):
            return _SerializeNode(_BINARC_FORMAT_BOOLEAN, in_value, in_value)
        elif isinstance(in_value, (int, long)):
            return _SerializeNode._make_integer_node(io_value_map, in_value)
        elif isinstance(in_value, float):
            return _SerializeNode._make_floating_node(io_value_map, in_value)
        elif isinstance(in_value, basestring):
            local_string = in_value.encode('utf-8')
            local_node = io_value_map.get(local_string)
            if local_node is None:
                local_node = _SerializeNode(
                    _BINARC_FORMAT_STRING,
                    local_string, 
                    _SerializeNode._make_bytes_hash(local_string))
                io_value_map[local_string] = local_node
            return local_node
        elif isinstance(in_value, (list, tuple)):
            local_array = []
            for local_element in in_value:
                local_array.append(
                    _SerializeNode._make_node(io_value_map, local_element))
            return _SerializeNode(_BINARC_FORMAT_ARRAY, tuple(local_array))
        elif isinstance(in_value, dict):
            # 辞書をソートする。
            local_map = []
            for local_key, local_value in in_value.items():
                local_map.append((
                    _SerializeNode._make_node(io_value_map, local_key),
                    _SerializeNode._make_node(io_value_map, local_value)))
            local_map.sort(key=lambda x: (x[0]._hash << 4) | x[0]._format)
            local_key_array = []
            local_value_array = []
            for local_key, local_value in local_map:
                local_key_array.append(local_key)
                local_value_array.append(local_value)
            return _SerializeNode(
                _BINARC_FORMAT_MAP, 
                tuple(local_value_array + local_key_array))
        else:
            # 値が未対応の型だった。
            assert in_value is None
            return _SerializeNode(_BINARC_FORMAT_NIL, None)

    ## 浮動小数点数をシリアライズするノードを構築する。
    #  @param[in,out] io_value_map 値を登録する辞書。
    #  @param[in]     in_value     シリアライズする浮動小数点数。
    #  @return 浮動小数点数をシリアライズする _SerializeNode インスタンス。
    @staticmethod
    def _make_floating_node(io_value_map, in_value):
        local_node = io_value_map.get(in_value)
        if local_node is not None:
            return local_node
        if (in_value.is_integer() and
            -(1 << 63) <= in_value and in_value < (1 << 64)
        ):
            # 整数に変換できるなら、整数としてシリアライズする。
            return _SerializeNode._make_integer_node(io_value_map, long(in_value))

        ## @todo INFとNANの扱いをどうする？ 単精度に変換しなくてもいい？
        local_pack = struct.pack('f', in_value)
        if struct.unpack('f', local_pack)[0] == in_value:
            # 単精度浮動小数点としてシリアライズする。
            local_format = _BINARC_FORMAT_FLOATING_32
            local_hash = struct.unpack('I', local_pack)[0]
        else:
            # 倍精度浮動小数点としてシリアライズする。
            local_format = _BINARC_FORMAT_FLOATING_64
            local_pack = struct.unpack('II', struct.pack('d', in_value))
            local_hash = local_pack[0] ^ local_pack[1]
        local_node = _SerializeNode(local_format, in_value, local_hash)
        io_value_map[in_value] = local_node
        return local_node

    ## 整数をシリアライズするノードを構築する。
    #  @param[in,out] io_value_map 値を登録する辞書。
    #  @param[in]     in_value     シリアライズする整数値。
    #  @return 整数をシリアライズする _SerializeNode インスタンス。
    @staticmethod
    def _make_integer_node(io_value_map, in_value):
        local_node = io_value_map.get(in_value)
        if local_node is not None:
            return local_node
        elif in_value < -(1 << 63):
            # 64ビットより大きい整数は未対応。
            assert False, 'Integer greater than 64-bit is not supported.'
            return _SerializeNode(_BINARC_FORMAT_NIL, None)
        elif in_value < -(1 << 31):
            local_format = _BINARC_FORMAT_NEGATIVE_64
            local_hash = _SerializeNode._make_64bits_integer_hash(in_value)
        elif in_value < -(_BINARC_IMMEDIATE_BITS_MASK + 1):
            local_format = _BINARC_FORMAT_NEGATIVE_32
            local_hash = in_value
        elif in_value < 0:
            return _SerializeNode(
                _BINARC_FORMAT_NEGATIVE_IMMEDIATE, in_value, in_value)
        elif in_value <= _BINARC_IMMEDIATE_BITS_MASK:
            return _SerializeNode(
                _BINARC_FORMAT_UNSIGNED_IMMEDIATE, in_value, in_value)
        elif in_value < (1 << 32):
            local_format = _BINARC_FORMAT_UNSIGNED_32
            local_hash = in_value
        elif in_value < (1 << 64):
            local_format = _BINARC_FORMAT_UNSIGNED_64
            local_hash = _SerializeNode._make_64bits_integer_hash(in_value)
        else:
            # 64ビットより大きい整数は未対応。
            assert False, 'Integer greater than 64-bit is not supported.'
            return _SerializeNode(_BINARC_FORMAT_NIL, None)
        local_node = _SerializeNode(local_format, in_value, local_hash)
        io_value_map[in_value] = local_node
        return local_node

    #--------------------------------------------------------------------------
    ## @brief バイト列のハッシュ値を算出する。
    #  @param[in] in_bytes ハッシュ値を算出するバイト列。
    #  @return バイト列のハッシュ値。
    @staticmethod
    def _make_bytes_hash(in_bytes):
        # 32ビットFNV-1でハッシュ値を算出する。
        FNV_OFFSET_BASIS_32 = 0x811c9dc5
        FNV_PRIME_32 = 0x1000193
        local_hash = FNV_OFFSET_BASIS_32
        for local_char in in_bytes:
            local_hash = ord(local_char) ^ (
                (FNV_PRIME_32 * local_hash) & 0xffffffff);
        return local_hash

    @staticmethod
    def _make_64bits_integer_hash(in_value):
        local_pack = struct.unpack(
            '<II', struct.pack('<Q', in_value & 0xffffffffffffffff))
        return local_pack[0] ^ local_pack[1]


#ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
#------------------------------------------------------------------------------
## @brief 値から、binarc形式のバイト列を構築する。
#  @param[in] in_value binarc形式のバイト列にする値。
#  @return binarc形式のバイト列。
def pack(in_value):
    return _SerializeNode.make_binary(in_value)

#------------------------------------------------------------------------------
import binascii
if __name__ == '__main__':
    local_sample_data = {
        0xfffffff: 'UNSIGNED_IMMEDIATE',
        0xffffffff: 'UNSIGNED_32',
        0x100000000: 'UNSIGNED_64',
        -0x10000000: 'NEGATIVE_IMMEDIATE',
        -0x80000000: 'NEGATIVE_32',
        -0x100000000: 'NEGATIVE_64',
        0.5: 'FLOATING_32',
        0.1: 'FLOATING_64',
        'UNSIGNED': [  0xfffffff,  0xffffffff,  0x100000000],
        'NEGATIVE': [-0x10000000, -0x80000000, -0x100000000],
        'FLOATING': [
            0.5, 0.1,
            float(0xfffffff), float(0xffffffff),
            float(-0x010000000), float(-0x80000000)],
        'STRING': 'STRING'}
    #local_sample_data = {0x01234567: 0x89abcdef, 'abcd': 'efgh'}
    local_binary = pack(local_sample_data)
    with open('./test/sample.binarc', 'wb') as local_file:
        local_file.write(local_binary)
