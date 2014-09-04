/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copybrief psyq::geometry::coordinate
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
        template<typename, unsigned> class coordinate_traits;
        template<typename, unsigned> class coordinate;
        template<typename> class coordinate_2d;
        template<typename> class coordinate_3d;
    } // namespace geometry
} // psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 座標の型特性の基底型。

    @tparam template_vector    @copydoc psyq::geometry::coordinate::vector
    @tparam template_dimension @copydoc psyq::geometry::coordinate::dimension
 */
template<typename template_vector, unsigned template_dimension>
class psyq::geometry::coordinate_traits
{
    /// thisが指す値の型。
    private: typedef coordinate_traits this_type;

    /** @brief 座標を表す幾何ベクトルの型特性。

        template_vector でテンプレート特殊化した
        psyq::geometry::vector_traits を用意しておくこと。
     */
    public: typedef psyq::geometry::vector_traits<template_vector>
        vector_traits;

    /** @brief 座標を表す幾何ベクトルの型。

        以下の条件は、必ず満たしていること。
        - コピー構築子とコピー代入演算子が使える。
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
    public: typedef typename this_type::vector_traits::type vector;

    /// 座標を表す幾何ベクトルの成分の型。
    public: typedef typename this_type::vector_traits::element element;

    public: enum: unsigned
    {
        /// 座標を表す幾何ベクトルの成分の数。
        dimension = template_dimension,
    };

    /// 最小座標と最大座標を要素とするAABB。
    public: typedef psyq::geometry::aabb<this_type> aabb;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 任意型の幾何ベクトルを使う座標のベクトル処理。

    座標を表す幾何ベクトルの基本的な処理を行う。
    この実装では汎用的な手法でベクトル処理を実装しているが、
    他に適切な実装手法があるなら、
    テンプレート特殊化などで互換性のある別の実装をユーザーが用意すること。

    @tparam template_vector    @copydoc psyq::geometry::coordinate::vector
    @tparam template_dimension @copydoc psyq::geometry::coordinate::dimension
 */
template<typename template_vector, unsigned template_dimension>
class psyq::geometry::coordinate:
    public psyq::geometry::coordinate_traits<template_vector, template_dimension>
{
    /// thisが指す値の型。
    private: typedef coordinate this_type;

    /// this_type の基底型。
    public: typedef psyq::geometry::coordinate_traits<vector, dimension>
        base_type;

    //-------------------------------------------------------------------------
    /** @brief 幾何ベクトルの要素から値を取得する。
        @return 幾何ベクトルの要素の値。
        @param[in] in_vector 要素の値を取得する幾何ベクトル。
        @param[in] in_index  取得する要素のインデックス番号。
     */
    public: static typename base_type::element get_element(
        typename base_type::vector const& in_vector,
        unsigned const in_index)
    {
        static_assert(
            true,//std::is_standard_layout<typename base_type::vector>::value,
            "'template_vector' is not standard layout type.");
        static_assert(
            this_type::dimension * sizeof(typename base_type::element) <=
                sizeof(typename base_type::vector),
            "");
        auto const local_elements(
            reinterpret_cast<typename base_type::element const*>(&in_vector));
        PSYQ_ASSERT(in_index < this_type::dimension);
        return *(local_elements + in_index);
    }

    /** @brief 幾何ベクトルの要素に値を設定する。
        @return 幾何ベクトルの要素に設定した値。
        @param[in,out] io_vector 要素に値を設定する幾何ベクトル。
        @param[in]     in_index  設定する要素のインデックス番号。
        @param[in]     in_value  設定する要素の値。
     */
    public: static typename base_type::element set_element(
        typename base_type::vector& io_vector,
        unsigned const in_index,
        typename base_type::element const in_value)
    {
        static_assert(
            true,//std::is_standard_layout<typename base_type::vector>::value,
            "'template_vector' is not standard layout type.");
        static_assert(
            this_type::dimension * sizeof(typename base_type::element) <=
                sizeof(typename base_type::vector),
            "");
        auto const local_elements(
            reinterpret_cast<typename base_type::element*>(&io_vector));
        PSYQ_ASSERT(in_index < template_dimension);
        *(local_elements + in_index) = in_value;
        return in_value;
    }

    //-------------------------------------------------------------------------
    /** @brief 2つの幾何ベクトルの内積を算出する。
        @return 2つの幾何ベクトルの内積。
        @param[in] in_left  内積の左辺となる幾何ベクトル。
        @param[in] in_right 内積の右辺となる幾何ベクトル。
     */
    public: static typename base_type::element dot_product(
        typename base_type::vector const& in_left,
        typename base_type::vector const& in_right)
    {
        auto local_dot(
            this_type::get_element(in_left, 0) *
            this_type::get_element(in_right, 0));
        for (unsigned i(1); i < this_type::dimension; ++i)
        {
            local_dot +=
                this_type::get_element(in_left, i) *
                this_type::get_element(in_right, i);
        }
        return local_dot;
    }

    /** @brief 2つの3次元幾何ベクトルの外積を算出する。
        @return 2つの3次元幾何ベクトルの外積。
        @param[in] in_left  外積の左辺となる3次元幾何ベクトル。
        @param[in] in_right 外積の右辺となる3次元幾何ベクトル。
     */
    public: static typename base_type::vector cross_product(
        typename base_type::vector const& in_left,
        typename base_type::vector const& in_right)
    {
        auto const local_left_0(this_type::get_element(in_left, 0));
        auto const local_left_1(this_type::get_element(in_left, 1));
        auto const local_left_2(this_type::get_element(in_left, 2));
        auto const local_right_0(this_type::get_element(in_right, 0));
        auto const local_right_1(this_type::get_element(in_right, 1));
        auto const local_right_2(this_type::get_element(in_right, 2));
        return psyq::geometry::_private::vector_maker
            <typename base_type::vector, base_type::vector_traits::size>
                ::make(
                    local_left_1 * local_right_2 - local_left_2 * local_right_1,
                    local_left_2 * local_right_0 - local_left_0 * local_right_2,
                    local_left_0 * local_right_1 - local_left_1 * local_right_0);
    }

    //-------------------------------------------------------------------------
    /** @brief 幾何ベクトルの長さを算出する。
        @return 幾何ベクトルの長さ。
        @param[in] in_vector 長さを算出する幾何ベクトル。
     */
    public: static typename base_type::element compute_length(
        typename base_type::vector const& in_vector)
    {
        return std::sqrt(this_type::dot_product(in_vector, in_vector));
    }

    /** @brief 幾何ベクトルの長さを調整する。
        @return 長さを調整した幾何ベクトル。
        @param[in] in_vector 元となる幾何ベクトル。
        @param[in] in_length 調整後の長さ。
     */
    public: static typename base_type::vector arrange_length(
        typename base_type::vector const& in_vector,
        typename base_type::element const in_length)
    {
        auto const local_square_length(
            this_type::dot_product(in_vector, in_vector));
        auto local_vector(in_vector);
        if (0 < local_square_length)
        {
            auto const local_scale(
                in_length / std::sqrt(local_square_length));
            for (unsigned i(0); i < this_type::dimension; ++i)
            {
                this_type::set_element(
                    local_vector,
                    i,
                    local_scale * this_type::get_element(in_vector, i));
            }
        }
        else
        {
            this_type::set_element(local_vector, 0, in_length);
        }
        return local_vector;
    }

    //-------------------------------------------------------------------------
    /** @brief 絶対座標系からモートン座標系への変換スケールを算出する。
        @param[out] out_elements 変換スケールの各要素を出力する先。
        @param[in]  in_aabb      衝突判定領域の全体を包む、絶対座標系AABB。
        @param[in]  in_level_cap 空間分割の最大深度。
     */
    protected: static void compute_mosp_scale(
        std::array<typename base_type::element, this_type::dimension>& out_elements,
        typename this_type::aabb const& in_aabb,
        unsigned const in_level_cap)
    {
        auto const local_size(in_aabb.get_max() - in_aabb.get_min());
        auto const local_unit(
            static_cast<typename base_type::element>(1 << in_level_cap));
        for (unsigned i(0); i < this_type::dimension; ++i)
        {
            out_elements[i] = this_type::compute_mosp_scale(
                local_unit, this_type::get_element(local_size, i));
        }
    }

    /** @brief 絶対座標系からモートン座標系への変換スケールを算出する。
        @param[in] in_morton_size モートン座標の最大値。
        @param[in] in_world_size  絶対座標系でのモートン空間の大きさ。
     */
    protected: static typename base_type::element compute_mosp_scale(
        typename base_type::element const in_morton_size,
        typename base_type::element const in_world_size)
    {
        return in_world_size < std::numeric_limits<typename base_type::element>::epsilon()?
            0: in_morton_size / in_world_size;
    }

}; // class psyq::geometry::coordinate

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 二次元座標の型特性。 psyq::mosp_space のtemplate引数に使う。
    @tparam template_vector @copydoc psyq::geometry::coordinate::vector
 */
template<typename template_vector>
class psyq::geometry::coordinate_2d:
    public psyq::geometry::coordinate<template_vector, 2>
{
    /// thisが指す値の型。
    private: typedef coordinate_2d this_type;

    /// this_type の基底型。
    public: typedef psyq::geometry::coordinate<vector, 2> base_type;

    //-------------------------------------------------------------------------
    /** @brief 幾何ベクトルを構築する。
        @return 構築した幾何ベクトル。
        @param[in] in_element_0 幾何ベクトルの要素#0の初期値。
        @param[in] in_element_1 幾何ベクトルの要素#1の初期値。
     */
    public: static typename base_type::vector make(
        typename base_type::element const in_element_0,
        typename base_type::element const in_element_1)
    {
        return psyq::geometry::_private::vector_maker
            <typename base_type::vector, base_type::vector_traits::size>
                ::make(in_element_0, in_element_1);
    }

    /** @brief 要素が全て同じ値の幾何ベクトルを構築する。
        @return 構築した幾何ベクトル。
        @param[in] in_element 幾何ベクトルの全要素の初期値。
     */
    public: static typename base_type::vector make(
        typename base_type::element const in_element)
    {
        return this_type::make(in_element, in_element);
    }

    /** @brief 任意のランダムアクセスコンテナから幾何ベクトルを構築する。
        @return 構築した幾何ベクトル。
        @param[in] in_container 要素の初期値が格納されているコンテナ。
     */
    public: template<typename template_container>
    static typename base_type::vector make(
        template_container const& in_container)
    {
        return this_type::make(in_container[0], in_container[1]);
    }

    //-------------------------------------------------------------------------
    /** @brief 絶対座標系からモートン座標系への変換スケールを算出する。
        @param[in] in_aabb      衝突判定領域の全体を包む、絶対座標系AABB。
        @param[in] in_level_cap 空間分割の最大深度。
     */
    public: static typename base_type::vector compute_mosp_scale(
        typename base_type::aabb const& in_aabb,
        unsigned const in_level_cap)
    {
        std::array<typename base_type::element, base_type::dimension>
            local_container;
        base_type::compute_mosp_scale(local_container, in_aabb, in_level_cap);
        return this_type::make(local_container);
    }

}; // class psyq::geometry::coordinate_2d

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 三次元座標の型特性。 psyq::mosp_space のtemplate引数に使う。
    @tparam template_vector @copydoc psyq::geometry::coordinate::vector
 */
template<typename template_vector>
class psyq::geometry::coordinate_3d:
    public psyq::geometry::coordinate<template_vector, 3>
{
    /// thisが指す値の型。
    private: typedef coordinate_3d this_type;

    /// this_type の基底型。
    public: typedef psyq::geometry::coordinate<vector, 3> base_type;

    //-------------------------------------------------------------------------
    /** @brief 幾何ベクトルを構築する。
        @return 構築した幾何ベクトル。
        @param[in] in_element_0 幾何ベクトルの要素#0の初期値。
        @param[in] in_element_1 幾何ベクトルの要素#1の初期値。
        @param[in] in_element_2 幾何ベクトルの要素#2の初期値。
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

    //-------------------------------------------------------------------------
    /** @brief 絶対座標系からモートン座標系への変換スケールを算出する。
        @param[in] in_aabb      衝突判定領域の全体を包む、絶対座標系AABB。
        @param[in] in_level_cap 空間分割の最大深度。
     */
    public: static typename base_type::vector compute_mosp_scale(
        typename base_type::aabb const& in_aabb,
        unsigned const in_level_cap)
    {
        std::array<typename base_type::element, base_type::dimension>
            local_container;
        base_type::compute_mosp_scale(local_container, in_aabb, in_level_cap);
        return this_type::make(local_container);
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

            template_coordinate::cross_product(
                template_coordinate::make(1, 0, 0),
                template_coordinate::make(0, 1, 0));
        }
    }
}

#endif // !defined(PSYQ_GEOMETRY_COORDINATE_HPP_)
