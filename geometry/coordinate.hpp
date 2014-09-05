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

#ifndef PSYQ_GEOMETRY_NEARLY_EQUAL_EPSILON_MAG_DEFAULT
#define PSYQ_GEOMETRY_NEARLY_EQUAL_EPSILON_MAG_DEFAULT 3
#endif // !defined(PSYQ_GEOMETRY_NEARLY_EQUAL_EPSILON_MAG_DEFAULT)

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
/** @brief 座標系の型特性の基底型。

    座標を表す幾何ベクトルの基本的な演算処理を行う。

    @note
        this_type で実装している幾何ベクトル演算処理は、
        汎用的な手法で実装している。
        幾何ベクトル処理に特化した適切な実装があるなら、
        psyq::geometry::coordinate_traits を派生させるか、
        psyq::geometry::coordinate をテンプレート特殊化し、
        互換性のある別の実装をユーザーが用意すること。

    @warning
        psyq::geometry::coordinate_traits で実装している
        幾何ベクトル演算処理を実際に使う場合は、以下の条件を満たす必要がある。
        条件を満たさない場合は、ユーザーが実装を用意すること。
        - 幾何ベクトルの成分は、連続したメモリに配置されている。
        - 幾何ベクトルの最初の成分のメモリ配置位置は、
          幾何ベクトルの先頭位置と一致する。

    @tparam template_vector    @copydoc psyq::geometry::coordinate::vector
    @tparam template_dimension @copydoc psyq::geometry::coordinate::dimension
    @ingroup psyq_geometry_coordinate
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

    /// 座標を表す成分の配列。
    public: typedef std::array<
        typename this_type::element, this_type::dimension>
            element_array;

    /// 最小座標と最大座標を要素とするAABB。
    public: typedef psyq::geometry::aabb<this_type> aabb;

    //-------------------------------------------------------------------------
    /// @name 座標の成分
    //@{
    /** @brief 座標を表す幾何ベクトルの要素から値を取得する。
        @return 座標を表す幾何ベクトルの要素の値。
        @param[in] in_vector 要素の値を取得する幾何ベクトル。
        @param[in] in_index  取得する要素のインデックス番号。
     */
    public: static typename this_type::element get_element(
        typename this_type::vector const& in_vector,
        unsigned const in_index)
    {
        static_assert(
            std::is_standard_layout<typename this_type::vector>::value,
            "'template_vector' is not standard layout type.");
        static_assert(
            this_type::dimension * sizeof(typename this_type::element) <=
                sizeof(typename this_type::vector),
            "");
        auto const local_elements(
            reinterpret_cast<typename this_type::element const*>(&in_vector));
        PSYQ_ASSERT(in_index < this_type::dimension);
        return *(local_elements + in_index);
    }

    /** @brief 座標を表す幾何ベクトルの要素に値を設定する。
        @return 座標を表す幾何ベクトルの要素に設定した値。
        @param[in,out] io_vector 要素に値を設定する幾何ベクトル。
        @param[in]     in_index  設定する要素のインデックス番号。
        @param[in]     in_value  設定する要素の値。
     */
    public: static typename this_type::element set_element(
        typename this_type::vector& io_vector,
        unsigned const in_index,
        typename this_type::element const in_value)
    {
        static_assert(
            std::is_standard_layout<typename this_type::vector>::value,
            "'template_vector' is not standard layout type.");
        static_assert(
            this_type::dimension * sizeof(typename this_type::element) <=
                sizeof(typename this_type::vector),
            "");
        auto const local_elements(
            reinterpret_cast<typename this_type::element*>(&io_vector));
        PSYQ_ASSERT(in_index < template_dimension);
        *(local_elements + in_index) = in_value;
        return in_value;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 座標の比較
    //@{
    /** @brief 2つの成分がほぼ等値か比較する。
        @retval true  ほぼ等値だった。
        @retval false 等値ではなかった。
        @param[in] in_left_value  比較する浮動小数点値の左辺値。
        @param[in] in_right_value 比較する浮動小数点値の右辺値。
        @param[in] in_epsilon_mag 誤差の範囲に使うエプシロン値の倍率。
     */
    public: static bool nearly_equal(
        typename this_type::element const in_left_value,
        typename this_type::element const in_right_value,
        unsigned const in_epsilon_mag =
            PSYQ_GEOMETRY_NEARLY_EQUAL_EPSILON_MAG_DEFAULT)
    {
        auto const local_epsilon(
            std::numeric_limits<typename this_type::element>::epsilon() * in_epsilon_mag);
        auto const local_diff(in_left_value - in_right_value);
        return -local_epsilon <= local_diff && local_diff <= local_epsilon;
    }

    /** @brief 座標を表す幾何ベクトルの長さを比較する。
        @retval true  in_vector の長さと in_length は、ほぼ等しい。
        @retval false in_vector の長さと in_length は、等しくない。
        @param[in] in_vector      判定する幾何ベクトル。
        @param[in] in_length      判定する長さ。
        @param[in] in_epsilon_mag 誤差の範囲に使うエプシロン値の倍率。
     */
    public: static bool nearly_length(
        typename this_type::vector const& in_vector,
        typename this_type::element const in_length,
        unsigned const in_epsilon_mag =
            PSYQ_GEOMETRY_NEARLY_EQUAL_EPSILON_MAG_DEFAULT)
    {
        return this_type::nearly_equal(
            this_type::dot_product(in_vector, in_vector),
            in_length * in_length,
            in_epsilon_mag);
    }

    /** @brief 座標のすべての成分が「左辺値 < 右辺値」か判定する。
        @param[in] in_left  比較の左辺値となる座標を表す幾何ベクトル。
        @param[in] in_right 比較の右辺値となる座標を表す幾何ベクトル。
        @note
            座標を表す幾何ベクトルの構築後に
            座標が使ってない幾何ベクトルの成分が変更されていた場合、
            正しい動作を保証できない。
     */
    public: static bool less_than(
        typename this_type::vector const& in_left,
        typename this_type::vector const& in_right)
    {
        return !this_type::less_than_equal(in_right, in_left);
    }

    /** @brief 座標のすべての成分が「左辺値 <= 右辺値」か判定する。
        @param[in] in_left  比較の左辺値となる座標を表す幾何ベクトル。
        @param[in] in_right 比較の右辺値となる座標を表す幾何ベクトル。
        @note
            座標を表す幾何ベクトルの構築後に
            座標が使ってない幾何ベクトルの成分が変更されていた場合、
            正しい動作を保証できない。
     */
    public: static bool less_than_equal(
        typename this_type::vector const& in_left,
        typename this_type::vector const& in_right)
    {
        for (unsigned i(0); i < this_type::dimension; ++i)
        {
            auto const local_left(this_type::get_element(in_left, i));
            auto const local_right(this_type::get_element(in_right, i));
            if (local_right < local_left)
            {
                return false;
            }
        }
        return true;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 座標の演算
    //@{
    /** @brief 2つの座標を表す幾何ベクトルの内積を算出する。
        @return 2つの座標を表す幾何ベクトルの内積。
        @param[in] in_left  内積の左辺値となる座標を表す幾何ベクトル。
        @param[in] in_right 内積の右辺値となる座標を表す幾何ベクトル。
        @note
            座標を表す幾何ベクトルの構築後に
            座標が使ってない幾何ベクトルの成分が変更されていた場合、
            正しい動作を保証できない。
     */
    public: static typename this_type::element dot_product(
        typename this_type::vector const& in_left,
        typename this_type::vector const& in_right)
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
        @note
            座標を表す幾何ベクトルの構築後に
            座標が使ってない幾何ベクトルの成分が変更されていた場合、
            正しい動作を保証できない。
     */
    public: static typename this_type::vector cross_product(
        typename this_type::vector const& in_left,
        typename this_type::vector const& in_right)
    {
        static_assert(
            this_type::dimension == 3, "'this_type::dimension' is not 3.");
        auto const local_left_0(this_type::get_element(in_left, 0));
        auto const local_left_1(this_type::get_element(in_left, 1));
        auto const local_left_2(this_type::get_element(in_left, 2));
        auto const local_right_0(this_type::get_element(in_right, 0));
        auto const local_right_1(this_type::get_element(in_right, 1));
        auto const local_right_2(this_type::get_element(in_right, 2));
        return psyq::geometry::_private::vector_maker
            <typename this_type::vector, this_type::vector_traits::size>
                ::make(
                    local_left_1 * local_right_2 - local_left_2 * local_right_1,
                    local_left_2 * local_right_0 - local_left_0 * local_right_2,
                    local_left_0 * local_right_1 - local_left_1 * local_right_0);
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 座標の大きさ
    //@{
    /** @brief 座標を表す幾何ベクトルの長さを算出する。
        @return 座標を表す幾何ベクトルの長さ。
        @param[in] in_vector 長さを算出する幾何ベクトル。
        @note
            座標を表す幾何ベクトルの構築後に
            座標が使ってない幾何ベクトルの成分が変更されていた場合、
            正しい動作を保証できない。
     */
    public: static typename this_type::element compute_length(
        typename this_type::vector const& in_vector)
    {
        return std::sqrt(this_type::dot_product(in_vector, in_vector));
    }

    /** @brief 座標を表す幾何ベクトルの長さを調整する。
        @return 長さを調整した幾何ベクトル。
        @param[in] in_vector 元となる幾何ベクトル。
        @param[in] in_length 調整後の長さ。
        @note
            座標を表す幾何ベクトルの構築後に
            座標が使ってない幾何ベクトルの成分が変更されていた場合、
            正しい動作を保証できない。
     */
    public: static typename this_type::vector arrange_length(
        typename this_type::vector const& in_vector,
        typename this_type::element const in_length = 1)
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
    //@}
}; // class psyq::geometry::coordinate_traits

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 任意型の幾何ベクトルを使う座標系。

    ユーザーが使う幾何ベクトルの専用処理をテンプレート特殊化して実装するなら、
    このクラスをテンプレート特殊化する。

    @tparam template_vector    @copydoc psyq::geometry::coordinate::vector
    @tparam template_dimension @copydoc psyq::geometry::coordinate::dimension
    @ingroup psyq_geometry_coordinate
 */
template<typename template_vector, unsigned template_dimension>
class psyq::geometry::coordinate:
public psyq::geometry::coordinate_traits<template_vector, template_dimension>
{
    /// thisが指す値の型。
    private: typedef coordinate this_type;

    /// this_type の基底型。
    public: typedef psyq::geometry::coordinate_traits<
        template_vector, template_dimension>
            base_type;

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
            <typename base_type::vector, base_type::vector_traits::size>
                ::make(in_element_0, in_element_1);
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
