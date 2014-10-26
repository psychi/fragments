/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copybrief psyq::geometry::coordinate
    @defgroup psyq_geometry_coordinate 座標系の型特性
    @ingroup psyq_geometry psyq::geometry
 */
#ifndef PSYQ_GEOMETRY_COORDINATE_HPP_
#define PSYQ_GEOMETRY_COORDINATE_HPP_

#include <type_traits>
//#include "psyq/geometry/vector.hpp"
//#include "psyq/geometry/shape.hpp"
//#include "psyq/geometry/aabb.hpp"

/// @cond
namespace psyq
{
    namespace geometry
    {
        template<typename, unsigned> class coordinate;
        template<typename> class coordinate_2d;
        template<typename> class coordinate_3d;
    } // namespace geometry
} // psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 座標系の型特性の基底型。

    @tparam template_vector    @copydoc psyq::geometry::coordinate::vector
    @tparam template_dimension @copydoc psyq::geometry::coordinate::dimension
    @ingroup psyq_geometry_coordinate
 */
template<typename template_vector, unsigned template_dimension>
class psyq::geometry::coordinate
{
    /// thisが指す値の型。
    private: typedef coordinate this_type;

    /** @brief 座標を表す幾何ベクトルの型特性。

        template_vector でテンプレート特殊化した
        psyq::geometry::vector::traits を用意しておくこと。
     */
    public: typedef psyq::geometry::vector::traits<template_vector>
        vector_traits;

    /** @brief 座標を表す幾何ベクトルの型。

        - 以下の条件を満たしていること。
          - コピー構築子とコピー代入演算子が使える。
          - this_type::vector_traits::size 個の this_type::element
            型の引数を渡す構築子が使える。
          - 以下に相当する二項演算子が使える。
            @code
            vector operator+(vector, vector);
            vector operator-(vector, vector);
            vector operator*(vector, vector);
            vector operator*(vector, element);
            vector operator/(vector, vector);
            vector operator/(vector, element);
            @endcode
        - 座標を表す幾何ベクトルでは、幾何ベクトルの成分のうち、
          最初の psyq::geometry::coordinate::dimension 個のみを使用する。
          それら以外の成分を変更すると、有効な座標として機能しなくなる。
        - 有効な座標を表す幾何ベクトルは、以下の関数で構築できる。
          - psyq::geometry::coordinate::make
          - psyq::geometry::coordinate_2d::make
          - psyq::geometry::coordinate_2d::make_filled
          - psyq::geometry::coordinate_3d::make
          - psyq::geometry::coordinate_3d::make_filled
        - 有効な座標を表す幾何ベクトルを構築した後で、
          座標が使ってない幾何ベクトルの成分を変更すると、
          有効な座標として機能しなくなる。
          psyq::geometry::coordinate::validate
          で、幾何ベクトルが有効な座標を表しているか判定できる。
     */
    public: typedef typename this_type::vector_traits::type vector;

    /// 座標を表す成分の型。
    public: typedef typename this_type::vector_traits::element element;

    public: enum: unsigned
    {
        /// 座標を表す成分の数。
        dimension = template_dimension,
    };
    static_assert(
        template_dimension <= this_type::vector_traits::size,
        "'template_dimension' is greater than 'vector_traits::size'");

    /// 座標を表す成分の配列の型。
    public: typedef std::array<
        typename this_type::element, this_type::dimension>
            element_array;

    //-------------------------------------------------------------------------
    /** @brief 有効な座標を表す幾何ベクトルを構築する。
        @return 有効な座標を表す幾何ベクトル。
        @param[in] in_vector 元となる幾何ベクトル。
        @sa this_type::validate で、幾何ベクトルが有効な座標を表しているか判定できる。
     */
    public: static typename this_type::vector make(
        typename this_type::vector const& in_vector)
    {
        auto local_vector(in_vector);
        for (
            unsigned i(this_type::dimension);
            i < this_type::vector_traits::size;
            ++i)
        {
            psyq::geometry::vector::at(local_vector, i) = 0;
        }
        return local_vector;
    }

    /** @brief 幾何ベクトルが有効な座標を表しているか判定する。

        座標を表す幾何ベクトルを構築した後で、
        座標が使ってない幾何ベクトルの成分を変更すると、
        座標として正常に動作しなくなってしまうので、その判定をする。

        @retval true  幾何ベクトルは有効な座標を表している。
        @retval false 幾何ベクトルは有効な座標を表していない。
        @param[in] in_vector
     */
    public: static bool validate(
        typename this_type::vector const& in_vector)
    {
        return (this_type::vector_traits::size <= 0
                || 0 < this_type::dimension
                || psyq::geometry::vector::const_at(in_vector, 0) == 0)
            && (this_type::vector_traits::size <= 1
                || 1 < this_type::dimension
                || psyq::geometry::vector::const_at(in_vector, 1) == 0)
            && (this_type::vector_traits::size <= 2
                || 2 < this_type::dimension
                || psyq::geometry::vector::const_at(in_vector, 2) == 0)
            && (this_type::vector_traits::size <= 3
                || 3 < this_type::dimension
                || psyq::geometry::vector::const_at(in_vector, 3) == 0);
    }

}; // class psyq::geometry::coordinate

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 2次元座標の型特性。
    @tparam template_vector @copydoc psyq::geometry::coordinate::vector
    @ingroup psyq_geometry_coordinate
 */
template<typename template_vector>
class psyq::geometry::coordinate_2d:
public psyq::geometry::coordinate<template_vector, 2>
{
    /// thisが指す値の型。
    private: typedef coordinate_2d this_type;

    /// this_type の基底型。
    public: typedef psyq::geometry::coordinate<template_vector, 2> base_type;

    /// 最小座標と最大座標を要素とするAABB。
    public: typedef psyq::geometry::aabb<this_type> aabb;

    //-------------------------------------------------------------------------
    /** @brief 座標を表す幾何ベクトルを構築する。
        @return 構築した幾何ベクトル。
        @param[in] in_element_0 座標の要素#0の初期値。
        @param[in] in_element_1 座標の要素#1の初期値。
        @sa base_type::validate で、幾何ベクトルが有効な座標を表しているか判定できる。
     */
    public: static typename base_type::vector make(
        typename base_type::element const in_element_0,
        typename base_type::element const in_element_1)
    {
        return psyq::geometry::_private::vector_maker
            <typename base_type::vector, base_type::vector_traits::size>
                ::make(in_element_0, in_element_1);
    }

    /// @copydoc psyq::geometry::coordinate::make
    public: static typename base_type::vector make(
        typename base_type::vector const& in_vector)
    {
        return base_type::make(in_vector);
    }

    /** @brief ランダムアクセスが可能な任意型のコンテナから、
               有効な座標を表す幾何ベクトルを構築する。
        @return 構築した幾何ベクトル。
        @param[in] in_container 座標の初期値が格納されているコンテナ。
        @sa base_type::validate で、幾何ベクトルが有効な座標を表しているか判定できる。
     */
    public: template<typename template_container>
    static typename base_type::vector make(
        template_container const& in_container)
    {
        return this_type::make(in_container[0], in_container[1]);
    }

    /** @brief 成分が全て同じ値の座標を表す幾何ベクトルを構築する。
        @return 構築した幾何ベクトル。
        @param[in] in_element 座標の全成分の初期値。
        @sa base_type::validate で、幾何ベクトルが有効な座標を表しているか判定できる。
     */
    public: static typename base_type::vector make_filled(
        typename base_type::element const in_element)
    {
        return this_type::make(in_element, in_element);
    }

}; // class psyq::geometry::coordinate_2d

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 3次元座標の型特性。
    @tparam template_vector @copydoc psyq::geometry::coordinate::vector
    @ingroup psyq_geometry_coordinate
 */
template<typename template_vector>
class psyq::geometry::coordinate_3d:
public psyq::geometry::coordinate<template_vector, 3>
{
    /// thisが指す値の型。
    private: typedef coordinate_3d this_type;

    /// this_type の基底型。
    public: typedef psyq::geometry::coordinate<template_vector, 3> base_type;

    /// 最小座標と最大座標を要素とするAABB。
    public: typedef psyq::geometry::aabb<this_type> aabb;

    //-------------------------------------------------------------------------
    /** @brief 有効な座標を表す幾何ベクトルを構築する。
        @return 構築した幾何ベクトル。
        @param[in] in_element_0 座標の要素#0の初期値。
        @param[in] in_element_1 座標の要素#1の初期値。
        @param[in] in_element_2 座標の要素#2の初期値。
        @sa base_type::validate で、幾何ベクトルが有効な座標を表しているか判定できる。
     */
    public: static typename base_type::vector make(
        typename base_type::element const in_element_0,
        typename base_type::element const in_element_1,
        typename base_type::element const in_element_2)
    {
        return psyq::geometry::_private::vector_maker
            <typename base_type::vector, base_type::vector_traits::size>
                ::make(in_element_0, in_element_1, in_element_2);
    }

    /// @copydoc psyq::geometry::coordinate::make
    public: static typename base_type::vector make(
        typename base_type::vector const& in_vector)
    {
        return base_type::make(in_vector);
    }

    /// @copydoc psyq::geometry::coordinate_2d::make(template_container const&)
    public: template<typename template_container>
    static typename base_type::vector make(
        template_container const& in_container)
    {
        return this_type::make(
            in_container[0], in_container[1], in_container[2]);
    }

    /// @copydoc psyq::geometry::coordinate_2d::make_filled(base_type::element)
    public: static typename base_type::vector make_filled(
        typename base_type::element const in_element)
    {
        return this_type::make(in_element, in_element, in_element);
    }

}; // class psyq::geometry::coordinate_3d

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    namespace test
    {
        template<typename template_coordinate>
        void geometry_coordinate()
        {
            typedef psyq::geometry::ball<template_coordinate> ball_type;
            auto const local_ball(
                ball_type::make(template_coordinate::make_filled(2), 10));
            auto const local_ball_aabb(
                psyq::geometry::make_aabb(local_ball));

            auto local_point(
                ball_type::point::make(template_coordinate::make_filled(2)));
            local_point = ball_type::point::make(template_coordinate::make_filled(3));

            typedef psyq::geometry::line<template_coordinate> line_type;
            line_type const local_line(
                local_ball.center_,
                line_type::direction::make(
                    template_coordinate::make_filled(local_ball.get_radius())));
            auto const local_line_aabb(
                psyq::geometry::make_aabb(local_line));

            typedef psyq::geometry::ray<template_coordinate> ray_type;
            ray_type const local_ray(local_line);
            auto const local_ray_aabb(
                psyq::geometry::make_aabb(local_ray));

            typedef psyq::geometry::box<template_coordinate> box_type;
            auto const local_box(
                box_type::make_cuboid(
                    local_line.origin_.get_position(),
                    local_line.direction_.get_unit(),
                    60 * 3.1415926535f / 180,
                    template_coordinate::make_filled(1)));
            auto const local_box_aabb(
                psyq::geometry::make_aabb(local_box));

            typedef psyq::geometry::barycentric_triangle<template_coordinate> triangle_type;
            auto const local_triangle(
                triangle_type::make(
                    template_coordinate::make(0, 0, 0),
                    template_coordinate::make(1, 0, 0),
                    template_coordinate::make(0, 1, 0)));
        }
    }
}

#endif // !defined(PSYQ_GEOMETRY_COORDINATE_HPP_)
