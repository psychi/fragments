/** @file
    @brief @copybrief psyq::std_array

    - std::unique_ptr がある開発環境では std::unique_ptr を、
      psyq::std_array でラップする。
    - std::unique_ptr がない開発環境では boost::array を、
      psyq::std_array でラップする。

    @author Hillco Psychi (https://twitter.com/psychi)
*/
#ifndef PSYQ_STD_ARRAY_HPP_
#define PSYQ_STD_ARRAY_HPP_

#include "./config.hpp"

#ifdef PSYQ_NO_STD_ARRAY
#define PSYQ_STD_ARRAY_BASE boost::array
#include <boost/array.hpp>
#else
/// psyq::std_array の基底クラステンプレート。
#define PSYQ_STD_ARRAY_BASE std::array
#include <array>
#endif // defined(PSYQ_NO_STD_ARRAY)

/// @cond
namespace psyq
{
    template<typename, std::size_t> class std_array;
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief std::array 互換の固定長配列。
 */
template<typename template_value, std::size_t template_size>
class psyq::std_array:
public PSYQ_STD_ARRAY_BASE<template_value, template_size>
{
    /// @brief thisが指す値の型。
    private: typedef std_array this_type;

    /// @brief this_type の基底型。
    public: typedef
        PSYQ_STD_ARRAY_BASE<template_value, template_size>
        base_type;

}; // class psyq::std_array

#endif // PSYQ_STD_ARRAY_HPP_
// vim: set expandtab:
