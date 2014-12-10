/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief 独自のバイナリデータ交換形式。
 */
#ifndef PSYQ_BINARC_HPP_
#define PSYQ_BINARC_HPP_

#include <memory>

#ifndef PSYQ_ASSERT
#include <assert.h>
#define PSYQ_ASSERT assert
#endif // PSYQ_ASSERT

namespace psyq
{
    class binarc_archive;
    class binarc_node;
} // namespace psyq

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::binarc_archive
{
    /// thisが指す値の型。
    private: typedef binarc_archive this_type;

    public: typedef std::shared_ptr<this_type> shared_ptr;
    public: typedef std::weak_ptr<this_type> weak_ptr;

    public: typedef std::uint32_t unit;

    public: this_type::unit const* get_body(std::size_t const in_offset) const;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class psyq::binarc_node
{
    /// thisが指す値の型。
    private: typedef binarc_node this_type;

    public: enum kind: std::uint8_t
    {
        kind_NIL,
        kind_UNSIGNED_INTEGER_28,
        kind_UNSIGNED_INTEGER_32,
        kind_UNSIGNED_INTEGER_64,
        kind_BOOLEAN,
        kind_NEGATIVE_INTEGER_28,
        kind_NEGATIVE_INTEGER_32,
        kind_NEGATIVE_INTEGER_64,
        kind_FLOATING_32,
        kind_FLOATING_64,
        kind_STRING,
        kind_EXTENDED,
        kind_ARRAY,
        kind_MAP,
    };

    //-------------------------------------------------------------------------
    /** @brief 空のノードを構築する。
     */
    public: binarc_node(): tag_(nullptr) {}

    /** @brief binarc書庫の最上位ノードを構築する。
        @param[in] in_archive 保持するbinarc書庫。
     */
    public: explicit binarc_node(psyq::binarc_archive::shared_ptr in_archive):
    archive_(std::move(in_archive))
    {
        auto const local_archive(this->archive_.get());
        if (local_archive != nullptr)
        {
            this->tag_ = local_archive->get_body(1);
            if (this->tag_ == nullptr)
            {
                this->archive_.reset();
            }
        }
        else
        {
            this->tag_ = nullptr;
        }
    }

    private: binarc_node(
        psyq::binarc_archive::unit const* const in_tag,
        psyq::binarc_archive::shared_ptr const& in_archive):
    tag_(in_tag),
    archive_(in_archive)
    {}

    public: binarc_archive::shared_ptr const& get_archive() const
    {
        return this->archive_;
    }

    public: this_type::kind get_kind() const
    {
        return this->tag_ != nullptr?
            static_cast<this_type::kind>(*(this->tag_) >> 28):
            this_type::kind_NIL;
    }

    //-------------------------------------------------------------------------
    public: template<typename template_number>
    bool get_number(template_number& out_number) const
    {
        switch (this->get_kind())
        {
        case this_type::kind_UNSIGNED_INTEGER_28:
            return this->get_unsigned_28(out_number);
        case this_type::kind_UNSIGNED_INTEGER_32:
            return this->get_unsigned_32(out_number);
        case this_type::kind_UNSIGNED_INTEGER_64:
            return this->get_unsigned_64(out_number);
        default:
            return false;
        }
    }

    private: template<typename template_type>
    bool get_unsigned_28(template_type& out_number) const
    {
        binarc_archive::unit const local_read(this->tag_ & 0x0fffffff);
        auto const local_write(static_cast<template_type>(local_read));
        if (local_read != local_write)
        {
            return false;
        }
        out_number = local_write;
        return true;
    }

    private: template<typename template_type>
    bool get_unsigned_32(template_type& out_number) const
    {
        auto const local_body(this->get_body(this_type::kind_UNSIGNED_INTEGER_32));
        if (local_body == nullptr)
        {
            return false;
        }
        auto const local_read(*local_body);
        auto const local_write(static_cast<template_type>(local_read));
        if (local_read != local_write || local_write < 0)
        {
            return false;
        }
        out_number = local_write;
        return true;
    }

    private: template<typename template_type>
    bool get_unsigned_64(template_type& out_number) const
    {
        auto const local_body(this->get_body(this_type::kind_UNSIGNED_INTEGER_64));
        if (local_body == nullptr)
        {
            return false;
        }
        auto const local_read(
            (static_cast<std::uint64_t>(*(local_body + 1)) << 32) | *local_body);
        auto const local_write(static_cast<template_type>(local_read));
        if (local_read != local_write || local_write < 0)
        {
            return false;
        }
        out_number = local_write;
        return true;
    }

    /** @brief ノードが指す文字列の要素数を取得する。
        @return
            ノードが指す文字列の要素数。
            ただし、ノードが文字列を指してない場合は、0を返す。
     */
    public: std::size_t get_string_size() const
    {
        auto const local_body(this->get_body(this_type::kind_STRING));
        return local_body != nullptr? *local_body: 0;
    }

    /** @brief ノードが指す文字列の先頭位置を取得する。
        @return
            ノードが指す文字列の先頭位置。
            ただし、ノードが文字列を指してない場合は、nullptrを返す。
     */
    public: char const* get_string_data() const
    {
        auto const local_body(this->get_body(this_type::kind_STRING));
        return local_body != nullptr?
            reinterpret_cast<char const*>(local_body + 1): nullptr;
    }

    /** @brief ノードが指す拡張バイナリの種別を取得する。
        @return
            ノードが指す拡張バイナリの種別。
            ただし、ノードが拡張バイナリを指してない場合は、0を返す。
     */
    public: psyq::binarc_archive::unit get_extended_kind() const
    {
        auto const local_body(this->get_body(this_type::kind_EXTENDED));
        return local_body != nullptr? *local_body: 0;
    }

    /** @brief ノードが指す拡張バイナリのバイト数を取得する。
        @return
            ノードが指す拡張バイナリのバイト数。
            ただし、ノードが拡張バイナリを指してない場合は、0を返す。
     */
    public: std::size_t get_extended_size() const
    {
        auto const local_body(this->get_body(this_type::kind_EXTENDED));
        return local_body != nullptr? *(local_body + 1): 0;
    }

    /** @brief ノードが指す拡張バイナリの先頭位置を取得する。
        @return
            ノードが指す拡張バイナリの先頭位置。
            ただし、ノードが拡張バイナリを指してない場合は、nullptrを返す。
     */
    public: void const* get_extended_data() const
    {
        auto const local_body(this->get_body(this_type::kind_EXTENDED));
        return local_body != nullptr && 0 < *(local_body + 1)?
            local_body + 2: nullptr;
    }

    /** @brief ノードが指す配列の要素数を取得する。
        @return
            ノードが指す配列の要素数。
            ただし、ノードが配列を指してない場合は、0を返す。
     */
    public: std::size_t get_array_size() const
    {
        auto const local_body(this->get_body(this_type::kind_ARRAY));
        return local_body != nullptr? *local_body: 0;
    }

    /** @brief ノードが指す配列の要素を取得する。
        @param[in] in_index 取得する配列要素のインデックス番号。
        @return
            配列の要素を指すノード。
            ただし、該当する配列要素が存在しない場合は、空ノードを返す。
     */
    public: this_type get_array_element(std::size_t const in_index) const
    {
        auto const local_body(this->get_body(this_type::kind_ARRAY));
        return local_body != nullptr && in_index < *local_body?
            this_type(local_body + 1 + in_index, this->archive_): this_type();
    }

    /** @brief ノードが指す辞書の要素数を取得する。
        @return
            ノードが指す辞書の要素数。
            ただし、ノードが辞書を指してない場合は、0を返す。
     */
    public: std::size_t get_map_size() const
    {
        auto const local_body(this->get_body(this_type::kind_MAP));
        return local_body != nullptr? *local_body: 0;
    }

    /** @brief ノードが指す辞書のキーを取得する。
        @param[in] in_index 取得する辞書キーのインデックス番号。
        @return
            辞書のキーを指すノード。
            ただし、該当する辞書キーが存在しない場合は、空ノードを返す。
     */
    public: this_type get_map_key(std::size_t const in_index) const
    {
        auto const local_body(this->get_body(this_type::kind_MAP));
        return local_body != nullptr && in_index < *local_body?
            this_type(local_body + 1 + in_index * 2, this->archive_):
            this_type();
    }

    /** @brief ノードが指す辞書の値を取得する。
        @param[in] in_index 取得する辞書値のインデックス番号。
        @return
            辞書の値を指すノード。
            ただし、該当する辞書値が存在しない場合は、空ノードを返す。
     */
    public: this_type get_map_value(std::size_t const in_index) const
    {
        auto const local_body(this->get_body(this_type::kind_MAP));
        return local_body != nullptr && in_index < *local_body?
            this_type(local_body + 2 + in_index * 2, this->archive_):
            this_type();
    }

    //-------------------------------------------------------------------------
    private: psyq::binarc_archive::unit const* get_body(
        psyq::binarc_archive::unit const in_kind)
    const
    {
        if (this->tag_ != nullptr)
        {
            auto const local_tag(*this->tag_);
            if ((local_tag >> 28) == in_kind)
            {
                auto const local_archive(this->archive_.get());
                if (local_archive != nullptr)
                {
                    auto const local_body(
                        local_archive->get_body(local_tag & 0x0fffffff));
                    PSYQ_ASSERT(local_body != nullptr);
                    return local_body;
                }
                PSYQ_ASSERT(false);
            }
        }
        return nullptr;
    }

    //-------------------------------------------------------------------------
    private: psyq::binarc_archive::unit const* tag_;
    private: psyq::binarc_archive::shared_ptr archive_;
};

#endif // PSYQ_BINARC_HPP_
