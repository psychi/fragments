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

    public: typedef psyq::geometry::vector_processor<template_vector>
        processor;

    /// @copydoc psyq::geometry::generic_vector_processor::traits
    public: typedef typename this_type::processor::traits traits;

    /** @brief 座標を表す幾何ベクトルの型。

        - 座標を現す幾何ベクトルでは、幾何ベクトルの成分のうち、
          最初の psyq::geometry::coordinate_traits::dimension 個のみを使用する。
        - 座標を表す幾何ベクトルは、 psyq::geometry::coordinate_2d::make() か
          psyq::geometry::coordinate_3d::make() で構築すること。
        - 座標を表す幾何ベクトルを構築した後で、
          座標が使ってない幾何ベクトルの成分を変更すると、
          座標を表す幾何ベクトルの処理が正しく動作しなくなるので注意。
        - 以下の条件を満たしていること。
          - コピー構築子とコピー代入演算子が使える。
          - element 型の引数を成分数だけ渡す構築子が使える。
          - 以下に相当する二項演算子が使える。
            @code
            vector operator+(vector, vector);
            vector operator-(vector, vector);
            vector operator*(vector, vector);
            vector operator*(vector, element);
            vector operator/(vector, vector);
            vector operator/(vector, element);
            @endcode
     */
    public: typedef typename this_type::traits::type vector;

    /// 座標を表す成分の型。
    public: typedef typename this_type::traits::element element;

    public: enum: unsigned
    {
        /// 座標を表す成分の数。
        dimension = template_dimension,
    };
    static_assert(
        template_dimension <= this_type::traits::size,
        "'template_dimension' is greater than 'traits::size'");

    /// 座標を表す成分の配列。
    public: typedef std::array<
        typename this_type::element, this_type::dimension>
            element_array;

    //-------------------------------------------------------------------------
    public: static typename this_type::vector make(
        typename this_type::vector const& in_vector)
    {
        auto local_vector(in_vector);
        for (
            unsigned i(this_type::dimension);
            i < this_type::traits::size;
            ++i)
        {
            this_type::processor::at(local_vector, i) = 0;
        }
        return local_vector;
    }

    public: static bool validate(
        typename this_type::vector const& in_vector)
    {
        return (this_type::traits::size <= 0
                || 0 < this_type::dimension
                || this_type::processor::const_at(in_vector, 0) == 0)
            && (this_type::traits::size <= 1
                || 1 < this_type::dimension
                || this_type::processor::const_at(in_vector, 1) == 0)
            && (this_type::traits::size <= 2
                || 2 < this_type::dimension
                || this_type::processor::const_at(in_vector, 2) == 0)
            && (this_type::traits::size <= 3
                || 3 < this_type::dimension
                || this_type::processor::const_at(in_vector, 3) == 0);
    }

}; // class psyq::geometry::coordinate

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 二次元座標の型特性。 psyq::mosp_space のtemplate引数に使う。
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
     */
    public: static typename base_type::vector make(
        typename base_type::element const in_element_0,
        typename base_type::element const in_element_1)
    {
        return psyq::geometry::_private::vector_maker
            <typename base_type::vector, base_type::traits::size>
                ::make(in_element_0, in_element_1);
    }

    /** @brief 座標を表す幾何ベクトルを構築する。
        @return 構築した幾何ベクトル。
        @param[in] in_vector 座標の初期値となる幾何ベクトル。
     */
    public: static typename base_type::vector make(
        typename base_type::vector const& in_vector)
    {
        return base_type::make(in_vector);
    }

    /** @brief 要素が全て同じ値の座標を表す幾何ベクトルを構築する。
        @return 構築した幾何ベクトル。
        @param[in] in_element 座標の全要素の初期値。
     */
    public: static typename base_type::vector make(
        typename base_type::element const in_element)
    {
        return this_type::make(in_element, in_element);
    }

    /** @brief ランダムアクセスが可能な任意型のコンテナから、
               座標を表す幾何ベクトルを構築する。
        @return 構築した幾何ベクトル。
        @param[in] in_container 座標要素の初期値が格納されているコンテナ。
     */
    public: template<typename template_container>
    static typename base_type::vector make(
        template_container const& in_container)
    {
        return this_type::make(in_container[0], in_container[1]);
    }

}; // class psyq::geometry::coordinate_2d

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 三次元座標の型特性。 psyq::mosp_space のtemplate引数に使う。
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
    /** @brief 座標を表す幾何ベクトルを構築する。
        @return 構築した幾何ベクトル。
        @param[in] in_element_0 座標の要素#0の初期値。
        @param[in] in_element_1 座標の要素#1の初期値。
        @param[in] in_element_2 座標の要素#2の初期値。
     */
    public: static typename base_type::vector make(
        typename base_type::element const in_element_0,
        typename base_type::element const in_element_1,
        typename base_type::element const in_element_2)
    {
        return psyq::geometry::_private::vector_maker
            <typename base_type::vector, base_type::traits::size>
                ::make(in_element_0, in_element_1, in_element_2);
    }

    /// @copydoc psyq::geometry::coordinate_2d::make(base_type::vector const&)
    public: static typename base_type::vector make(
        typename base_type::vector const& in_vector)
    {
        return base_type::make(in_vector);
    }

    /// @copydoc psyq::geometry::coordinate_2d::make(base_type::element)
    public: static typename base_type::vector make(
        typename base_type::element const in_element)
    {
        return this_type::make(in_element, in_element, in_element);
    }

    /// @copydoc psyq::geometry::coordinate_2d::make(template_container const&)
    public: template<typename template_container>
    static typename base_type::vector make(
        template_container const& in_container)
    {
        return this_type::make(
            in_container[0], in_container[1], in_container[2]);
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
                ball_type::make(template_coordinate::make(float(2)), 10));
            auto const local_ball_aabb(
                psyq::geometry::make_aabb(local_ball));

            typedef psyq::geometry::segment<template_coordinate> segment_type;
            segment_type const local_segment(
                local_ball.get_center(),
                template_coordinate::make(local_ball.get_radius()));
            auto const local_segment_aabb(
                psyq::geometry::make_aabb(local_segment));

            typedef psyq::geometry::ray<template_coordinate> ray_type;
            auto const local_ray(
                ray_type::make(
                    local_segment.get_origin(),
                    local_segment.get_direction()));
            auto const local_ray_aabb(
                psyq::geometry::make_aabb(local_ray));

            typedef psyq::geometry::box<template_coordinate> box_type;
            auto const local_box(
                box_type::make_cuboid(
                    local_segment.get_origin(),
                    local_segment.get_direction(),
                    60 * 3.1415926535f / 180,
                    template_coordinate::make(float(1))));
            auto const local_box_aabb(
                psyq::geometry::make_aabb(local_box));
        }
    }
}

#endif // !defined(PSYQ_GEOMETRY_COORDINATE_HPP_)
