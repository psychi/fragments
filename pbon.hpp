/** @brief PBON/JSON is a library for translating between PBON and JSON.

    PBON/JSONは、PBONとJSONの相互変換を行うためのlibrary。
    PBONは、Packed-Binary-Object-Notationの略。

    - Call pbon::json::Value::ImportJson(), import the JSON.
      pbon::json::Value::ImportJson() で、JSONを取り込む。

    - Call pbon::json::Value::ImportPbon(), import the PBON.
      pbon::json::Value::ImportPbon() で、PBONを取り込む。

    - Call pbon::json::Value::ExportJson(), export the JSON.
      pbon::json::Value::ExportJson() で、JSONを書き出す。

    - Call pbon::json::Value::ExportPbon(), export the PBON.
      pbon::json::Value::ExportPbon() で、PBONを書き出す。

    - Call pbon::Value::GetRoot(), get root value of PBON from packed-binary.
      pbon::Value::GetRoot() で、packed-binaryからPBONの最上位要素を取得する。

    @file
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PBON_JSON_HPP_
#define PBON_JSON_HPP_

namespace pbon
{

typedef std::int8_t  Int8;
typedef std::int16_t Int16;
typedef std::int32_t Int32;
typedef std::int64_t Int64;
typedef Int8  Char8;
typedef Int16 Char16;
typedef Int32 Char32;
typedef float  Float32;
typedef double Float64;

enum Type
{
	Type_NULL,
	Type_ARRAY,
	Type_OBJECT,
	Type_CHAR8 = 1 << 4,
	Type_CHAR16,
	Type_CHAR32,
	Type_INT8 = 2 << 4,
	Type_INT16,
	Type_INT32,
	Type_INT64,
	Type_FLOAT32 = (3 << 4) + 2,
	Type_FLOAT64,
};

template< typename template_Type > pbon::Type GetType()
{
	return pbon::Type_NULL;
}

template<> pbon::Type GetType< pbon::Char8 >()
{
	return pbon::Type_CHAR8;
}

template<> pbon::Type GetType< pbon::Char16 >()
{
	return pbon::Type_CHAR16;
}

template<> pbon::Type GetType< pbon::Int32 >()
{
	return pbon::Type_INT32;
}

template<> pbon::Type GetType< pbon::Int64 >()
{
	return pbon::Type_INT64;
}

template<> pbon::Type GetType< pbon::Float32 >()
{
	return pbon::Type_FLOAT32;
}

template<> pbon::Type GetType< pbon::Float64 >()
{
	return pbon::Type_FLOAT64;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief packed-binaryの要素。
    @tparam template_AttributeType @copydoc pbon::Value::Attribute
 */
template< typename template_AttributeType >
class Value
{
	/// thisが指す型。
	public: typedef Value This;

	/// packed-binaryの属性の型。
	public: typedef template_AttributeType Attribute;

	/// packed-binaryのheader情報。
	private: struct Header
	{
		pbon::Int32 Endian_;
		pbon::Int16 Type_;
		pbon::Int16 Root_;
	};

	/** @brief packed-binaryの最上位要素を取得。
	    @param[in] in_PackedBinary packed-binaryの先頭位置。
	    @retval !=NULL packed-binaryの最上位要素へのpointer。
	    @retval ==NULL 扱えないpacked-binaryだった。
	 */
	public: static const This* GetRoot(
		const void* const in_PackedBinary)
	{
		if (in_PackedBinary == NULL)
		{
			return NULL;
		}
		const This::Header& local_Header(
			*static_cast< const This::Header* >(in_PackedBinary));
		if (local_Header.Endian_ != 'pbon')
		{
			// endianが異なるので扱えない。
			return NULL;
		}
		if (local_Header.Type_ != pbon::GetType< This::Attribute >())
		{
			// 属性の型が異なるので扱えない。
			return NULL;
		}
		return This::GetAddress< This >(in_PackedBinary, local_Header.Root_);
	}

	/** @brief 上位要素を取得。
	    @retval !=NULL 上位要素へのpointer。
	    @retval ==NULL 上位要素がない。
	 */
	public: const This* GetSuper() const
	{
		if (this->Super_ == 0)
		{
			return NULL;
		}
		return This::GetAddress< This >(this, this->Super_);
	}

	/** @brief 持っている値の数を取得。
	 */
	public: std::size_t GetSize() const
	{
		return this->Size_;
	}

	/** @brief 持っている値の型を取得。
	 */
	public: pbon::Type GetType() const
	{
		return static_cast< pbon::Type >(this->Type_);
	}

	public: bool IsArray() const
	{
		return this->GetType() == pbon::Type_ARRAY;
	}

	public: bool IsObject() const
	{
		return this->GetType() == pbon::Type_OBJECT;
	}

	/** @brief 持っている値へのpointerを取得。
	    @tparam template_ValueType 持っている値の型。
	 */
	protected: template< typename template_ValueType >
	const template_ValueType* GetValue() const
	{
		return This::GetAddress< template_ValueType >(this, this->Value_);
	}

	/** @brief 相対位置からaddressを取得。
	    @tparam template_ValueType    pointerが指す値の型。
	    @tparam template_PositionType 相対位置の型。
	    @param[in] in_BaseAddress     基準位置となるpointer。
	    @param[in] in_BytePosition    基準位置からのbyte単位での相対位置。
	 */
	private: template< typename template_ValueType, typename template_PositionType >
	static const template_ValueType* GetAddress(
		const void* const           in_BaseAddress,
		const template_PositionType in_BytePosition)
	{
		return reinterpret_cast< const template_ValueType* >(
			static_cast< const char* >(in_BaseAdress) + in_BytePosition);
	}

	private: typename This::Attribute Value_; ///< 値。もしくは値への相対位置。
	private: typename This::Attribute Size_;  ///< 値の数。
	private: typename This::Attribute Type_;  ///< 値の型。
	private: typename This::Attribute Super_; ///< 上位要素への相対位置。
};
typedef pbon::Value< pbon::Int32 > Value32;

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// pbon::Value の配列。
template< typename template_AttributeType >
class Array:
	private pbon::Value< template_AttributeType >
{
	public: typedef pbon::Array< template_AttributeType > This;
	private: typedef pbon::Value< template_AttributeType > Super;
	public: typedef Super Value;

	using Super::Attribute;
	using Super::GetSuper;

	public: static const This* Cast(
		const Super* const in_Value)
	{
		if (in_Value == NULL || !in_Value->IsArray())
		{
			return NULL;
		}
		return static_cast< const This* >(in_Value);
	}

	/** @brief 持っている値の数を取得。
	 */
	public: std::size_t GetSize() const
	{
		if (!this->IsArray())
		{
			return 0;
		}
		return this->Super::GetSize();
	}

	public: const typename This::Value* GetBegin() const
	{
		if (!this->IsArray())
		{
			return NULL;
		}
		return this->GetValue< typename This::Value >();
	}

	public: const typename This::Value* GetEnd() const
	{
		if (!this->IsArray())
		{
			return NULL;
		}
		return this->GetValue< typename This::Value >() + this->GetSize();
	}

	public: const typename This::Value* At(
		const std::size_t in_Index)
	{
		if (!this->IsArray() || this->GetSize() <= in_Index)
		{
			return NULL;
		}
		return this->GetValue< This::Value >() + in_Index;
	}
};
typedef pbon::Array< pbon::Int32 > Array32;

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// pbon::Value の辞書。
template< typename template_AttributeType >
class Object:
	private Value< template_AttributeType >
{
	public: typedef pbon::Object< template_AttributeType > This;
	private: typedef pbon::Value< template_AttributeType > Super;
	public: typedef std::pair< Super, Super > Value;

	using Super::Attribute;
	using Super::GetSuper;

	public: static const This* Cast(
		const Super* const in_Value)
	{
		if (in_Value == NULL || !in_Value->IsObject())
		{
			return NULL;
		}
		return static_cast< const This* >(in_Value);
	}

	/** @brief 持っている値の数を取得。
	 */
	public: std::size_t GetSize() const
	{
		if (!this->IsObject())
		{
			return 0;
		}
		return this->Super::GetSize() / 2;
	}

	public: const typename This::Value* GetBegin() const
	{
		if (!this->IsObject())
		{
			return NULL;
		}
		return this->GetValue< typename This::Value >();
	}

	public: const typename This::Value* GetEnd() const
	{
		if (!this->IsObject())
		{
			return NULL;
		}
		return this->GetValue< typename This::Value >() + this->GetSize();
	}

	public: template< typename template_KeyType >
	const typename This::Value* LowerBound(
		const template_KeyType& in_Key) const;

	public: template< typename template_KeyType >
	const typename This::Value* UpperBound(
		const template_KeyType& in_Key) const;

	public: template< typename template_KeyType >
	const typename This::Value* Find(
		const template_KeyType& in_Key) const;
};
typedef pbon::Object< pbon::Int32 > Object32;

namespace json
{

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief JSONの要素。
    @tparam template_StringType @copydoc pbon::json::String
 */
template< typename template_StringType >
class Value
{
	/// thisが指す値の型。
	public: typedef pbon::json::Value< template_StringType > This;

	/// JSONの要素に使う std::string 互換の文字列型。
	public: typedef template_StringType String;

	//-------------------------------------------------------------------------
	/** @brief JSON形式の文字列から値を取り込む。
	    @param[in] in_JsonString 値を取り込むJSON形式文字列。
	 */
	public: void ImportJson(
		const typename This::String& in_JsonString)
	{
		this->ImportJson(in_JsonString.begin(), in_JsonString.end());
	}

	/** @brief JSON形式の文字列から値を取り込む。
	    @param[in] in_JsonBegin 値を取り込むJSON形式文字列の先頭位置。
	    @param[in] in_JsonEnd   値を取り込むJSON形式文字列の末尾位置。
	 */
	public: void ImportJson(
		const typename This::String::const_iterator& in_JsonBegin,
		const typename This::String::const_iterator& in_JsonEnd)
	{
		typename This::JsonParser local_Parser(in_JsonBegin, in_JsonEnd);
		if (!this->Parse(local_Parser))
		{
		}
	}

	/** @brief PBON形式のbyte列から値を取り込む。
	    @param[in] in_PbonBegin 値を取り込むPBON形式byte列の先頭位置。
	    @param[in] in_PbonEnd   値を取り込むPBON形式byte列の末尾位置。
	 */
	public: void ImportPbon(
		const void* const in_PbonBegin,
		const void* const in_PbonEnd);

	/** @brief JSON形式で文字列に値を書き出す。
	    @param[out] out_JsonString 値を書き出す先。
	 */
	public: void ExportJson(
		typename This::String& out_JsonString) const;

	/** @brief PBON形式でbinaryに値を書き出す。
	    @param[out] out_PbonBinary 値を書き出す先。
	 */
	public: template< typename template_Vector >
	void ExportPbon(template_Vector& out_PbonBinary) const;

	//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
	private: class JsonParser
	{
		public: typedef JsonParser This;

		private: enum { END_CHAR = -1 };

		//---------------------------------------------------------------------
		public: JsonParser(
			const typename String::const_iterator& in_Begin,
			const typename String::const_iterator& in_End):
		Current_(in_Begin),
		End_(in_End),
		LastChar_(This::END_CHAR),
		Line_(1),
		Undo_(false)
		{
			// pass
		}

		public: unsigned GetLine() const
		{
			return this->Line_;
		}

		public: int ReadChar()
		{
			if (this->Undo_)
			{
				this->Undo_ = false;
				return this->LastChar_;
			}
			if (this->Current_ == this->End_)
			{
				this->LastChar_ = This::END_CHAR;
				return This::END_CHAR;
			}
			if (this->LastChar_ == '\n')
			{
				++this->Line_;
			}
			this->LastChar_ = *this->Current_;
			++this->Current_;
			return this->LastChar_;
		}

		public: void UndoChar()
		{
			if (this->LastChar_ != This::END_CHAR)
			{
				//assert(!this->Undo_);
				this->Undo_ = true;
			}
		}

		public: void SkipWhiteSpace()
		{
			for (;;)
			{
				switch (this->ReadChar())
				{
					case ' ':
					case '\t':
					case '\n':
					case '\r':
					break;

					default:
					this->UndoChar();
					return;
				}
			}
		}

		public: bool Expect(
			const int in_ExpectChar)
		{
			this->SkipWhiteSpace();
			if (this->ReadChar() != in_ExpectChar)
			{
				this->UndoChar();
				return false;
			}
			return true;
		}

		public: bool Match(
			const char* const in_Begin)
		{
			for (const char* i = in_Begin; *i != 0; ++i)
			{
				if (this->ReadChar() != *i)
				{
					this->UndoChar();
					return false;
				}
			}
			this->SkipWhiteSpace();
			switch (this->LastChar_)
			{
				case ',':
				case ']':
				case '}':
				case This::END_CHAR:
				return true;

				default:
				return false;
			}
		}

		//---------------------------------------------------------------------
		private: typename String::const_iterator Current_;
		private: typename String::const_iterator End_;
		private: int                             LastChar_;
		private: unsigned                        Line_;
		private: bool                            Undo_;
	};

	//-------------------------------------------------------------------------
	private: bool Parse(
		typename This::JsonParser& io_Parser)
	{
		io_Parser.SkipWhiteSpace();
		const int local_Char(io_Parser.ReadChar());
		switch (local_Char)
		{
			case 'n':
			if (io_Parser.Match("ull"))
			{
				//*this = pbon::json::Value();
				return true;
			}
			return false;

			case 't':
			if (io_Parser.Match("rue"))
			{
				//out_Value = pbon::json::Value(true);
				return true;
			}
			return false;

			case 'f':
			if (io_Parser.Match("alse"))
			{
				//out_Value = pbon::json::Value(false);
				return true;
			}
			return false;

			case '"':
			return this->ParseString(io_Parser);

			case '[':
			return this->ParseArray(io_Parser);

			case '{':
			return this->ParseObject(io_Parser);

			default:
			io_Parser.UndoChar();
			if (('0' <= local_Char && local_Char <= '9') ||
				local_Char == '-')
			{
				return this->ParseNumber(io_Parser);
			}
			return false;
		}
	}

	private: bool ParseString(typename This::JsonParser& io_Parser)
	{
		return false;
	}

	private: bool ParseArray(typename This::JsonParser& io_Parser)
	{
		return false;
	}

	private: bool ParseObject(typename This::JsonParser& io_Parser)
	{
		return false;
	}

	private: bool ParseNumber(typename This::JsonParser& io_Parser)
	{
		return false;
	}
};

} // namespace json

} // namespace pbon

#endif // PBON_JSON_HPP_
