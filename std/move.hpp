/** @file
    @brief std::move の代替。

    std::move がある開発環境ではそれを流用し、
    std::move がない開発環境では boost::move で代用する。

    @author Hillco Psychi (https://twitter.com/psychi)
*/
#ifndef PSYQ_STD_MOVE_HPP_
#define PSYQ_STD_MOVE_HPP_

//#define PSYQ_STD_NO_MOVE
//define PSYQ_STD_NO_NULLPTR
//#define PSYQ_STD_NO_DEFAULT_DELETE
//#define PSYQ_STD_NO_UNIQUE_PTR
//#define PSYQ_STD_NO_SHARED_PTR

#ifdef PSYQ_STD_NO_NOEXCEPT
#define PSYQ_NOEXCEPT BOOST_NOEXCEPT
namespace boost
{
    template<class E> void throw_exception(E const& e)
    {
        PSYQ_ASSERT(false, "boost throw exception '%s'.\n", e.what());
    }
}
#endif // defined(PSYQ_STD_NO_NOEXCEPT)

#ifdef PSYQ_STD_NO_NULLPTR
#define PSYQ_NULLPTR NULL
namespace psyq
{
    struct std_nullptr_t
    {
        std_nullptr_t(void const* const in_null)
        {
            PSYQ_ASSERT(in_null == PSYQ_NULLPTR);
        }
    };
}
#else
#define PSYQ_NULLPTR nullptr
namespace psyq
{
    typedef std::nullptr_t std_nullptr_t;
}
#endif // defined(PSYQ_STD_NO_NULLPTR)

#ifdef PSYQ_STD_NO_MOVE
#include <boost/move/move.hpp>
#define PSYQ_MOVE(define_value) boost::move(define_value)
#define PSYQ_RV_REF(define_type) BOOST_RV_REF(define_type)
#define PSYQ_RV_REF_2_TEMPL_ARGS(\
    define_template,\
    define_argument_0,\
    define_argument_1)\
        BOOST_RV_REF_2_TEMPL_ARGS(\
            define_template, define_argument_0, define_argument_1)
#define PSYQ_RV_REF_3_TEMPL_ARGS(\
    define_template,\
    define_argument_0,\
    define_argument_1,\
    define_argument_2)\
        BOOST_RV_REF_3_TEMPL_ARGS(\
            define_template,\
            define_argument_0,\
            define_argument_1,\
            define_argument_2)
#else
/** @brief std:move を代替するマクロ関数。
    @param define_value 右辺値参照する値。
 */
#define PSYQ_MOVE(define_value) std::move(define_value)

/** @brief 右辺値参照を表すマクロ関数。
    @param define_type 右辺値参照する型。
 */
#define PSYQ_RV_REF(define_type) define_type&&

/** @brief 2つの引数を持つ型テンプレートの右辺値参照を表すマクロ関数。
    @param define_template   右辺値参照する型テンプレート。
    @param define_argument_0 型テンプレートの引数#0。
    @param define_argument_1 型テンプレートの引数#1。
 */
#define PSYQ_RV_REF_2_TEMPL_ARGS(\
    define_template,\
    define_argument_0,\
    define_argument_1)\
        define_template<define_argument_0, define_argument_1>&&

/** @brief 3つの引数を持つ型テンプレートの右辺値参照を表すマクロ関数。
    @param define_template   右辺値参照する型テンプレート。
    @param define_argument_0 型テンプレートの引数#0。
    @param define_argument_1 型テンプレートの引数#1。
    @param define_argument_2 型テンプレートの引数#2。
 */
#define PSYQ_RV_REF_3_TEMPL_ARGS(\
    define_template,\
    define_argument_0,\
    define_argument_1,\
    define_argument_2)\
        define_template<define_argument_0, define_argument_1, define_argument_2>&&
#endif // defined(PSYQ_STD_NO_MOVE)

#endif // PSYQ_STD_MOVE_HPP_
