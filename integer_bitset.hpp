/* Copyright (c) 2013, Hillco Psychi, All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
   OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
   ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
 */
#ifndef PSYQ_INTEGER_BITSET_HPP_
#define PSYQ_INTEGER_BITSET_HPP_
#include <string>
//#include "psyq/bit_algorithm.hpp"

namespace psyq
{
	/// @cond
	template<typename> class integer_bitset;
	/// @endcond
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::bitset 互換のビット集合。任意の整数型をビット集合として扱う。
    @tparam template_integer @copydoc integer_bitset::integer
 */
template<typename template_integer>
class psyq::integer_bitset
{
	private: typedef integer_bitset<template_integer> this_type;

	/// @brief ビット集合として扱う整数型。
	public: typedef template_integer integer;

	public: static std::size_t const SIZE = sizeof(template_integer) * 8;

	//-------------------------------------------------------------------------
	public: explicit integer_bitset(
		const template_integer in_bitset = 0)
	:
		integer_(in_bitset)
	{}

	public: template<typename template_string>
	explicit integer_bitset(
		template_string const&                     in_string,
		typename template_string::size_type const  in_begin = 0,
		typename template_string::size_type const  in_count =
			template_string::npos,
		typename template_string::value_type const in_zero = '0',
		typename template_string::value_type const in_one = '1')
	{
		auto const local_size(in_string.size());
		if (local_size <= in_begin)
		{
			this->integer_ = 0;
			return;
		}
		auto local_end(in_begin + in_count);
		if (in_count == template_string::npos || local_size < local_end)
		{
			local_end = local_size;
		}
		auto const local_data(in_string.data());
		this->integer_ = this_type::convert_from_string(
			local_data + in_begin, local_data + local_end, in_zero, in_one);
	}

	public: template<typename template_char>
	explicit integer_bitset(
		template_char const* const in_string,
		std::size_t const          in_count =
			std::basic_string<template_char>::npos,
		template_char const        in_zero = '0',
		template_char const        in_one = '1')
	{
		if (in_string == nullptr)
		{
			this->integer_ = 0;
			return;
		}
		auto const local_size(
			std::char_traits<template_char>::length(in_string));
		auto local_end(in_count);
		if (in_count == std::basic_string<template_char>::npos ||
			local_size < local_end)
		{
			local_end = local_size;
		}
		this->integer_ = this_type::convert_from_string(
			in_string, in_string + local_end, in_zero, in_one);
	}

	//-------------------------------------------------------------------------
	public: bool operator[](std::size_t const in_position) const
	{
		PSYQ_ASSERT(this_type::is_valid_position(in_position));
		return psyq::get_bit(this->integer_, in_position);
	}

	/// 未実装。
	private: bool operator[](std::size_t const in_position);

	//-------------------------------------------------------------------------
	public: this_type operator~() const
	{
		return this_type(~this->integer_);
	}

	public: this_type operator&(this_type const& in_bitset) const
	{
		return this_type(this->integer_ & in_bitset.integer_);
	}

	public: this_type& operator&=(this_type const& in_bitset)
	{
		this->integer_ &= in_bitset.integer_;
		return *this;
	}

	public: this_type operator|(this_type const& in_bitset) const
	{
		return this_type(this->integer_ | in_bitset.integer_);
	}

	public: this_type& operator|=(this_type const& in_bitset)
	{
		this->integer_ |= in_bitset.integer_;
		return *this;
	}

	public: this_type operator^(this_type const& in_bitset) const
	{
		return this_type(this->integer_ ^ in_bitset.integer_);
	}

	public: this_type& operator^=(this_type const& in_bitset)
	{
		this->integer_ ^= in_bitset.integer_;
		return *this;
	}

	public: this_type operator<<(std::size_t const in_shift) const
	{
		return this_type(this->integer_ << in_shift);
	}

	public: this_type& operator<<=(std::size_t const in_shift) const
	{
		this->integer_ <<= in_shift;
		return *this;
	}

	public: this_type operator>>(std::size_t const in_shift) const
	{
		return this_type(this->integer_ >> in_shift);
	}

	public: this_type& operator>>=(std::size_t const in_shift) const
	{
		this->integer_ >>= in_shift;
		return *this;
	}

	//-------------------------------------------------------------------------
	public: bool all() const
	{
		return this->integer_ + 1 == 0;
	}

	public: bool any() const
	{
		return this->integer_ != 0;
	}

	public: bool none() const
	{
		return this->integer_ == 0;
	}

	public: std::size_t count() const
	{
		return psyq::count_bit1(this->integer_);
	}

	public: std::size_t size() const
	{
		return this_type::SIZE;
	}

	public: bool test(std::size_t const in_position) const
	{
		return this_type::is_valid_position(in_position)
			&& this->operator[](in_position);
	}

	public: this_type& set()
	{
		this->integer_ = static_cast<template_integer>(-1);
		return *this;
	}

	public: this_type& set(std::size_t const in_position)
	{
		PSYQ_ASSERT(this_type::is_valid_position(in_position));
		this->integer_ = psyq::set_bit(this->integer_, in_position);
		return *this;
	}

	public: this_type& set(
		std::size_t const in_position,
		bool        const in_value)
	{
		PSYQ_ASSERT(this_type::is_valid_position(in_position));
		this->integer_ = psyq::set_bit(this->integer_, in_position, in_value);
		return *this;
	}

	public: this_type& reset(
		std::size_t const in_position)
	{
		PSYQ_ASSERT(this_type::is_valid_position(in_position));
		this->integer_ = psyq::reset_bit(this->integer_, in_position);
		return *this;
	}

	public: this_type& flip()
	{
		this->integer_ = ~this->integer_;
		return *this;
	}

	public: this_type& flip(
		std::size_t const in_position)
	{
		PSYQ_ASSERT(this_type::is_valid_position(in_position));
		this->integer_ = psyq::flip_bit(this->integer_, in_position);
		return *this;
	}

	//-------------------------------------------------------------------------
	public: unsigned long to_ulong() const
	{
		return static_cast<unsigned long>(this->to_integer());
	}

	public: unsigned long long to_ullong() const
	{
		return this->to_integer();
	}

	public: template_integer to_integer() const
	{
		return this->integer_;
	}

	//-------------------------------------------------------------------------
	private: template<typename template_char>
	static template_integer convert_from_string(
		template_char const* const in_begin,
		template_char const* const in_end,
		template_char const        in_zero,
		template_char const        in_one)
	{
		template_integer local_bit(1);
		template_integer local_integer(0);
		auto local_iterator(in_end);
		while (in_begin < local_iterator)
		{
			--local_iterator;
			if (*local_iterator == in_one)
			{
				local_integer |= local_bit;
				local_bit <<= 1;
			}
			else if (*local_iterator == in_zero)
			{
				local_bit <<= 1;
			}
		}
		return local_integer;
	}

	private: static bool is_valid_position(const std::size_t in_position)
	{
		return in_position < this_type::SIZE;
	}

	//-------------------------------------------------------------------------
	private: template_integer integer_;
};

#endif // PSYQ_INTEGER_BITSET_HPP_
