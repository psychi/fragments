/// @file
/// @author Hillco Psychi (https://twitter.com/psychi)
#ifndef PBON_HPP_
#define PBON_HPP_

/// pbon(Packed Binary Object Notation)は、jsonをbinaryにpackしたもの。
namespace pbon
{

typedef char Char8;
typedef wchar_t Char16;
typedef char Int8;
typedef std::int16_t Int16;
typedef std::int32_t Int32;
typedef std::int64_t Int64;
typedef float Float32;
typedef double Float64;

enum Type
{
	Type_NULL,
	Type_SEQUENCE,
	Type_MAPPING,
	Type_CHAR8 = 1 << 4,
	Type_CHAR16,
	Type_INT8 = 2 << 4,
	Type_INT16,
	Type_INT32,
	Type_INT64,
	Type_FLOAT32 = 3 << 4,
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
	/// thisの型。
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

	public: bool IsSequence() const
	{
		return this->GetType() == pbon::Type_SEQUENCE;
	}

	public: bool IsMapping() const
	{
		return this->GetType() == pbon::Type_MAPPING;
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
class Sequence:
	private pbon::Value< template_AttributeType >
{
	public: typedef pbon::Sequence< template_AttributeType > This;
	private: typedef pbon::Value< template_AttributeType > Super;
	public: typedef Super Value;

	using Super::Attribute;
	using Super::GetSuper;

	public: static const This* Cast(
		const Super* const in_Value)
	{
		if (in_Value == NULL || !in_Value->IsSequence())
		{
			return NULL;
		}
		return static_cast< const This* >(in_Value);
	}

	/** @brief 持っている値の数を取得。
	 */
	public: std::size_t GetSize() const
	{
		if (!this->IsSequence())
		{
			return 0;
		}
		return this->Super::GetSize();
	}

	public: const typename This::Value* GetBegin() const
	{
		if (!this->IsSequence())
		{
			return NULL;
		}
		return this->GetValue< typename This::Value >();
	}

	public: const typename This::Value* GetEnd() const
	{
		if (!this->IsSequence())
		{
			return NULL;
		}
		return this->GetValue< typename This::Value >() + this->GetSize();
	}

	public: const typename This::Value* At(
		const std::size_t in_Index)
	{
		if (!this->IsSequence() || this->GetSize() <= in_Index)
		{
			return NULL;
		}
		return this->GetValue< This::Value >() + in_Index;
	}
};
typedef pbon::Sequence< pbon::Int32 > Sequence32;

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// pbon::Value の辞書。
template< typename template_AttributeType >
class Mapping:
	private Value< template_AttributeType >
{
	public: typedef pbon::Mapping< template_AttributeType > This;
	private: typedef pbon::Value< template_AttributeType > Super;
	public: typedef std::pair< Super, Super > Value;

	using Super::Attribute;
	using Super::GetSuper;

	public: static const This* Cast(
		const Super* const in_Value)
	{
		if (in_Value == NULL || !in_Value->IsMapping())
		{
			return NULL;
		}
		return static_cast< const This* >(in_Value);
	}

	/** @brief 持っている値の数を取得。
	 */
	public: std::size_t GetSize() const
	{
		if (!this->IsMapping())
		{
			return 0;
		}
		return this->Super::GetSize() / 2;
	}

	public: const typename This::Value* GetBegin() const
	{
		if (!this->IsMapping())
		{
			return NULL;
		}
		return this->GetValue< typename This::Value >();
	}

	public: const typename This::Value* GetEnd() const
	{
		if (!this->IsMapping())
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
typedef pbon::Mapping< pbon::Int32 > Mapping32;

namespace json
{

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
class Value
{
	public: typedef Value This;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template< typename template_IteratorType >
class Parser
{
	public: typedef Parser< template_IteratorType > This;

	public: typedef template_IteratorType Iterator;

	private: enum { END_CHAR = -1 };

	//---------------------------------------------------------------------
	public: Parser(
		const template_IteratorType& in_Begin,
		const template_IteratorType& in_End):
	Current_(in_Begin),
	End_(in_End),
	LastChar_(This::END_CHAR),
	Line_(1),
	Undo_(false)
	{
		// pass
	}

	//---------------------------------------------------------------------
	private: void Parse(
		pbon::json::Value& out_Value)
	{
		this->SkipWhiteSpace();
		const int local_Char(this->TakeChar());
		switch (local_Char)
		{
			case 'n':
			if (this->Match("ull"))
			{
				out_Value = pbon::json::Value();
				return true;
			}
			return false;

			case 't':
			if (this->Match("rue"))
			{
				//out_Value = pbon::json::Value(true);
				return true;
			}
			return false;

			case 'f':
			if (this->Match("alse"))
			{
				//out_Value = pbon::json::Value(false);
				return true;
			}
			return false;

			case '"':
			return this->ParseString(out_Value);

			case '[':
			return this->ParseArray(out_Value);

			case '{':
			return this->ParseObject(out_Value);

			default:
			this->UndoChar();
			if (('0' <= local_Char && local_Char <= '9') ||
				local_Char == '-')
			{
				return this->ParseNumber(out_Value);
			}
			return false;
		}
	}

	private: bool ParseString(pbon::json::Value& out_Value);
	private: bool ParseArray(pbon::json::Value& out_Value);
	private: bool ParseObject(pbon::json::Value& out_Value);
	private: bool ParseNumber(pbon::json::Value& out_Value);

	//---------------------------------------------------------------------
	public: unsigned GetLine() const
	{
		return this->Line_;
	}

	private: int TakeChar()
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

	private: void UndoChar()
	{
		if (this->LastChar_ != This::END_CHAR)
		{
			//assert(!this->Undo_);
			this->Undo_ = true;
		}
	}

	private: void SkipWhiteSpace()
	{
		for (;;)
		{
			switch (this->TakeChar())
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

	private: bool Expect(
		const int in_ExpectChar)
	{
		this->SkipWhiteSpace();
		if (this->TakeChar() != in_ExpectChar)
		{
			this->UndoChar();
			return false;
		}
		return true;
	}

	private: bool Match(
		const char* const in_Begin)
	{
		for (const char* i = in_Begin; *i != 0; ++i)
		{
			if (this->TakeChar() != *i)
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
	private: template_IteratorType Current_;
	private: template_IteratorType End_;
	private: int                   LastChar_;
	private: unsigned              Line_;
	private: bool                  Undo_;
};

} // namespace json

} // namespace pbon

#endif // PBON_HPP_
