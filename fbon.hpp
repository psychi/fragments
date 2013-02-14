/// @file
/// @author Psychi Hillco ( https://twitter.com/psychi )
#ifndef FBON_HPP_
#define FBON_HPP_

/// fbon(Frozen Binary Object Notation)は、jsonをfrozen-binary化したもの。
namespace fbon
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

template< typename template_Type > fbon::Type GetType()
{
	return fbon::Type_NULL;
}

template<> fbon::Type GetType< fbon::Char8 >()
{
	return fbon::Type_CHAR8;
}

template<> fbon::Type GetType< fbon::Char16 >()
{
	return fbon::Type_CHAR16;
}

template<> fbon::Type GetType< fbon::Int32 >()
{
	return fbon::Type_INT32;
}

template<> fbon::Type GetType< fbon::Int64 >()
{
	return fbon::Type_INT64;
}

template<> fbon::Type GetType< fbon::Float32 >()
{
	return fbon::Type_FLOAT32;
}

template<> fbon::Type GetType< fbon::Float64 >()
{
	return fbon::Type_FLOAT64;
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief frozen-binaryの要素。
    @tparam template_AttributeType @copydoc fbon::Element::Attribute
 */
template< typename template_AttributeType >
class Element
{
	/// thisの型。
	public: typedef Element This;

	/// frozen-binaryの属性の型。
	public: typedef template_AttributeType Attribute;

	/// frozen-binaryのheader情報。
	private: struct Header
	{
		fbon::Int32 Endian_;
		fbon::Int16 Type_;
		fbon::Int16 Root_;
	};

	/** @brief frozen-binaryの最上位要素を取得。
	    @param[in] in_FrozenBinary frozen-binaryの先頭位置。
	    @retval !=NULL frozen-binaryの最上位要素へのpointer。
	    @retval ==NULL 扱えないfrozen-binaryだった。
	 */
	public: static const This* GetRoot(
		const void* const in_FrozenBinary)
	{
		if (in_FrozenBinary == NULL)
		{
			return NULL;
		}
		const This::Header& local_Header(
			*static_cast< const This::Header* >(in_FrozenBinary));
		if (local_Header.Endian_ != 'fbon')
		{
			// endianが異なるので扱えない。
			return NULL;
		}
		if (local_Header.Type_ != fbon::GetType< This::Attribute >())
		{
			// 属性の型が異なるので扱えない。
			return NULL;
		}
		return This::GetAddress< This >(in_FrozenBinary, local_Header.Root_);
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
	public: fbon::Type GetType() const
	{
		return static_cast< fbon::Type >(this->Type_);
	}

	public: bool IsSequence() const
	{
		return this->GetType() == fbon::Type_SEQUENCE;
	}

	public: bool IsMapping() const
	{
		return this->GetType() == fbon::Type_MAPPING;
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

	private: typename This::Attribute Value_;
	private: typename This::Attribute Size_;
	private: typename This::Attribute Type_;
	private: typename This::Attribute Super_;
};
typedef fbon::Element< fbon::Int32 > Element32;

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// fbon::Element の配列。
template< typename template_AttributeType >
class Sequence:
	private fbon::Element< template_AttributeType >
{
	public: typedef fbon::Sequence< template_AttributeType > This;
	private: typedef fbon::Element< template_AttributeType > Super;
	public: typedef Super Value;

	using Super::Attribute;
	using Super::GetSuper;

	public: static const This* Cast(
		const Super* const in_Element)
	{
		if (in_Element == NULL || !in_Element->IsSequence())
		{
			return NULL;
		}
		return static_cast< const This* >(in_Element);
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
typedef fbon::Sequence< fbon::Int32 > Sequence32;

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// fbon::Element の辞書。
template< typename template_AttributeType >
class Mapping:
	private Element< template_AttributeType >
{
	public: typedef fbon::Mapping< template_AttributeType > This;
	private: typedef fbon::Element< template_AttributeType > Super;
	public: typedef std::pair< Super, Super > Value;

	using Super::Attribute;
	using Super::GetSuper;

	public: static const This* Cast(
		const Super* const in_Element)
	{
		if (in_Element == NULL || !in_Element->IsMapping())
		{
			return NULL;
		}
		return static_cast< const This* >(in_Element);
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
typedef fbon::Mapping< fbon::Int32 > Mapping32;

} // namespace fbon

#endif // FBON_HPP_
