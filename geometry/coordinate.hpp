/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copybrief psyq::geometry::coordinate
 */
#ifndef PSYQ_GEOMETRY_COORDINATE_HPP_
#define PSYQ_GEOMETRY_COORDINATE_HPP_

//#include "psyq/geometry/vector.hpp"
//#include "psyq/geometry/aabb.hpp"

/// psyq::geometry::coordinate で使う、デフォルトのベクトル型。
#ifndef PSYQ_GEOMETRY_COORDINATE_VECTOR_DEFAULT
#include <glm/gtx/simd_vec4.hpp> // OpenGL Mathematics
# if GLM_ARCH != GLM_ARCH_PURE
#   define PSYQ_GEOMETRY_COORDINATE_VECTOR_DEFAULT glm::simdVec4
# else
#   define PSYQ_GEOMETRY_COORDINATE_VECTOR_DEFAULT glm::vec3
# endif
//#include "psyq/geometry/glm_vec.hpp"
#endif // !defined(PSYQ_GEOMETRY_COORDINATE_VECTOR_DEFAULT)

/// psyq::geometry::coordinate で使う、デフォルトのベクトル要素型。
#ifndef PSYQ_GEOMETRY_COORDINATE_ELEMENT_DEFAULT
#define PSYQ_GEOMETRY_COORDINATE_ELEMENT_DEFAULT float
#endif // !defined(PSYQ_GEOMETRY_COORDINATE_ELEMENT_DEFAULT)

/// @cond
namespace psyq
{
    namespace geometry
    {
        template<typename, typename, unsigned> class coordinate;
        template<typename, typename> class coordinate_2d;
        template<typename, typename> class coordinate_3d;
    }
}
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 座標の型特性。
    @tparam template_vector    @copydoc psyq::geometry::coordinate::vector
    @tparam template_element   @copydoc psyq::geometry::coordinate::element
    @tparam template_dimension @copydoc psyq::geometry::coordinate::DIMENSION
 */
template<
    typename template_vector,
    typename template_element,
    unsigned template_dimension>
class psyq::geometry::coordinate
{
    /// thisが指す値の型。
    private: typedef coordinate this_type;

    /** @brief 座標を表す幾何ベクトルの型。

        以下の条件は、必ず満たしていること。
        - コピー構築子とコピー代入演算子が使える。
        - 以下に相当する二項演算子が使える。
          @code
          template_vector operator+(template_vector, template_vector);
          template_vector operator-(template_vector, template_vector);
          template_vector operator*(template_vector, template_vector);
          template_vector operator*(template_vector, template_element);
          template_vector operator/(template_vector, template_vector);
          template_vector operator/(template_vector, template_element);
          @endcode
     */
    public: typedef template_vector vector;

    /// 座標を表す幾何ベクトルの成分の型。
    public: typedef template_element element;

    /// 座標が持つ成分の数。
    public: static unsigned const DIMENSION = template_dimension;

    /// 座標の最小座標と最大座標を要素とするAABB。
    public: typedef psyq::geometry::aabb<this_type> aabb;

    //-------------------------------------------------------------------------
    /** @brief 幾何ベクトルの要素から値を取得する。
        @return 幾何ベクトルの要素の値。
        @param[in] in_vector 要素の値を取得する幾何ベクトル。
        @param[in] in_index  取得する要素のインデックス番号。
     */
    public: static typename this_type::element get_element(
        typename this_type::vector const& in_vector,
        unsigned const in_index)
    {
        return psyq::geometry::get_vector_element
            <this_type::DIMENSION, typename this_type::element>
                (in_vector, in_index);
    }

    /** @brief 幾何ベクトルの要素に値を設定する。
        @return 幾何ベクトルの要素に設定した値。
        @param[in,out] io_vector 要素に値を設定する幾何ベクトル。
        @param[in]     in_index  設定する要素のインデックス番号。
        @param[in]     in_value  設定する要素の値。
     */
    public: static typename this_type::element set_element(
        typename this_type::vector& io_vector,
        unsigned const in_index,
        typename this_type::element const in_value)
    {
        return psyq::geometry::set_vector_element<this_type::DIMENSION>(
            io_vector, in_index, in_value);
    }

    /** @brief 2つの幾何ベクトルの内積を算出する。
        @return 2つの幾何ベクトルの内積。
        @param[in] in_left  内積の左辺となる幾何ベクトル。
        @param[in] in_right 内積の右辺となる幾何ベクトル。
     */
    public: static typename this_type::element dot_product(
        typename this_type::vector const& in_left,
        typename this_type::vector const& in_right)
    {
        return psyq::geometry::dot_product_vector
            <this_type::DIMENSION, typename this_type::element>
                (in_left, in_right);
    }

    /** @brief 2つの3次元幾何ベクトルの外積を算出する。
        @return 2つの3次元幾何ベクトルの外積。
        @param[in] in_left  外積の左辺となる3次元幾何ベクトル。
        @param[in] in_right 外積の右辺となる3次元幾何ベクトル。
     */
    public: static typename this_type::vector cross_product(
        typename this_type::vector const& in_left,
        typename this_type::vector const& in_right)
    {
        static_assert(
            3 <= this_type::DIMENSION,
            "'this_type::DIMENSION is less than 3.");
        return psyq::geometry::cross_product_vector
            <typename this_type::element>(in_left, in_right);
    }

    /** @brief 幾何ベクトルの長さの2乗を算出する。
        @return 幾何ベクトルの長さの2乗。
        @param[in] in_vector 長さの2乗を算出する幾何ベクトル。
     */
    public: static typename this_type::element square_length(
        typename this_type::vector const& in_vector)
    {
        return psyq::geometry::square_vector_length
            <this_type::DIMENSION, typename this_type::element>(in_vector);
    }

    /** @brief 幾何ベクトルの長さを算出する。
        @return 幾何ベクトルの長さ。
        @param[in] in_vector 長さを算出する幾何ベクトル。
     */
    public: static typename this_type::element compute_length(
        typename this_type::vector const& in_vector)
    {
        return psyq::geometry::compute_vector_length
            <this_type::DIMENSION, typename this_type::element>(in_vector);
    }

    /** @brief 幾何ベクトルを正規化する。

        in_vector が0ベクトルだった場合は、任意の単位ベクトルを返す。

        @return 正規化した幾何ベクトル。
        @param[in] in_vector 正規化する幾何ベクトル。
     */
    public: static typename this_type::vector normalize(
        typename this_type::vector const& in_vector)
    {
        return psyq::geometry::normalize_vector
            <this_type::DIMENSION, typename this_type::element>(in_vector);
    }

    /** @brief 幾何ベクトルが正規化されているか判定する。
        @retval true  幾何ベクトルは正規化されている。
        @retval false 幾何ベクトルは正規化されていない。
        @tparam template_coordinate
            psyq::geometry::coordinate 互換の幾何ベクトル型特性。
        @param[in] in_vector 判定する幾何ベクトル。
     */
    public: static bool is_normalized(
        typename this_type::vector const& in_vector)
    {
        return psyq::geometry::is_nearly_equal(
            this_type::square_length(in_vector),
            static_cast<typename this_type::element>(1));
    }

    //-------------------------------------------------------------------------
    /** @brief 絶対座標系からモートン座標系への変換スケールを算出する。
        @param[out] out_elements 変換スケールの各要素を出力する先。
        @param[in]  in_aabb      衝突判定領域の全体を包む、絶対座標系AABB。
        @param[in]  in_level_cap 空間分割の最大深度。
     */
    protected: static void compute_scale(
        std::array<typename this_type::element, this_type::DIMENSION>& out_elements,
        typename this_type::aabb const& in_aabb,
        unsigned const in_level_cap)
    {
        auto const local_size(in_aabb.get_max() - in_aabb.get_min());
        auto const local_unit(
            static_cast<typename this_type::element>(1 << in_level_cap));
        for (unsigned i(0); i < this_type::DIMENSION; ++i)
        {
            out_elements[i] = this_type::compute_scale(
                local_unit, this_type::get_element(local_size, i));
        }
    }

    /** @brief 絶対座標系からモートン座標系への変換スケールを算出する。
        @param[in] in_morton_size モートン座標の最大値。
        @param[in] in_world_size  絶対座標系でのモートン空間の大きさ。
     */
    protected: static typename this_type::element compute_scale(
        typename this_type::element const in_morton_size,
        typename this_type::element const in_world_size)
    {
        return in_world_size < std::numeric_limits<template_element>::epsilon()?
            0: in_morton_size / in_world_size;
    }

}; // class psyq::geometry::coordinate

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 二次元座標の型特性。 psyq::mosp_space のtemplate引数に使う。
    @tparam template_vector  @copydoc psyq::geometry::coordinate::vector
    @tparam template_element @copydoc psyq::geometry::coordinate::element
 */
template<typename template_vector, typename template_element>
class psyq::geometry::coordinate_2d:
    public psyq::geometry::coordinate<template_vector, template_element, 2>
{
    /// thisが指す値の型。
    private: typedef coordinate_2d this_type;

    /// this_type の基底型。
    public: typedef
        psyq::geometry::coordinate<template_vector, template_element, 2>
            base_type;

    //-------------------------------------------------------------------------
    /** @brief 幾何ベクトルを構築する。
        @return 構築した幾何ベクトル。
        @param[in] in_element_0 幾何ベクトルの要素#0の初期値。
        @param[in] in_element_1 幾何ベクトルの要素#1の初期値。
     */
    public: static typename this_type::vector make(
        typename this_type::element const in_element_0,
        typename this_type::element const in_element_1)
    {
        return psyq::geometry::make_vector
            <this_type::DIMENSION, typename this_type::vector>
                (in_element_0, in_element_1);
    }

    /** @brief 要素が全て同じ値の幾何ベクトルを構築する。
        @return 構築した幾何ベクトル。
        @param[in] in_element 幾何ベクトルの全要素の初期値。
     */
    public: static typename this_type::vector make(
        typename this_type::element const in_element)
    {
        return this_type::make(in_element, in_element);
    }

    /** @brief 任意のランダムアクセスコンテナから幾何ベクトルを構築する。
        @return 構築した幾何ベクトル。
        @param[in] in_container 要素の初期値が格納されているコンテナ。
     */
    public: template<typename template_container>
    static typename this_type::vector make(
        template_container const& in_container)
    {
        return this_type::make(in_container[0], in_container[1]);
    }

    /** @brief 絶対座標系からモートン座標系への変換スケールを算出する。
        @param[in] in_aabb      衝突判定領域の全体を包む、絶対座標系AABB。
        @param[in] in_level_cap 空間分割の最大深度。
     */
    public: static typename this_type::vector compute_scale(
        typename base_type::aabb const& in_aabb,
        unsigned const in_level_cap)
    {
        std::array<typename base_type::element, base_type::DIMENSION>
            local_container;
        base_type::compute_scale(local_container, in_aabb, in_level_cap);
        return this_type::make(local_container);
    }

}; // class psyq::geometry::coordinate_2d

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 三次元座標の型特性。 psyq::mosp_space のtemplate引数に使う。
    @tparam template_vector  @copydoc psyq::geometry::coordinate::vector
    @tparam template_element @copydoc psyq::geometry::coordinate::element
 */
template<typename template_vector, typename template_element>
class psyq::geometry::coordinate_3d:
    public psyq::geometry::coordinate<template_vector, template_element, 3>
{
    /// thisが指す値の型。
    private: typedef coordinate_3d this_type;

    /// this_type の基底型。
    public: typedef
        psyq::geometry::coordinate<template_vector, template_element, 3>
            base_type;

    //-------------------------------------------------------------------------
    /** @brief 幾何ベクトルを構築する。
        @return 構築した幾何ベクトル。
        @param[in] in_element_0 幾何ベクトルの要素#0の初期値。
        @param[in] in_element_1 幾何ベクトルの要素#1の初期値。
        @param[in] in_element_2 幾何ベクトルの要素#2の初期値。
     */
    public: static typename this_type::vector make(
        typename this_type::element const in_element_0,
        typename this_type::element const in_element_1,
        typename this_type::element const in_element_2)
    {
        return psyq::geometry::make_vector
            <this_type::DIMENSION, typename this_type::vector>
                (in_element_0, in_element_1, in_element_2);
    }

    /// @copydoc psyq::geometry::coordinate_2d::make(this_type::element)
    public: static typename this_type::vector make(
        typename this_type::element const in_element)
    {
        return this_type::make(in_element, in_element, in_element);
    }

    /// @copydoc psyq::geometry::coordinate_2d::make(template_container const&)
    public: template<typename template_container>
    static typename this_type::vector make(
        template_container const& in_container)
    {
        return this_type::make(
            in_container[0], in_container[1], in_container[2]);
    }

    /** @brief 絶対座標系からモートン座標系への変換スケールを算出する。
        @param[in] in_aabb      衝突判定領域の全体を包む、絶対座標系AABB。
        @param[in] in_level_cap 空間分割の最大深度。
     */
    public: static typename base_type::vector compute_scale(
        typename base_type::aabb const& in_aabb,
        unsigned const in_level_cap)
    {
        std::array<typename base_type::element, base_type::DIMENSION>
            local_container;
        base_type::compute_scale(local_container, in_aabb, in_level_cap);
        return this_type::make(local_container);
    }

}; // class psyq::geometry::coordinate_3d

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    namespace test
    {
        inline void geometry_shape()
        {
            typedef psyq::geometry::coordinate_3d<
                PSYQ_GEOMETRY_COORDINATE_VECTOR_DEFAULT,
                PSYQ_GEOMETRY_COORDINATE_ELEMENT_DEFAULT>
                    coordinate_type;

            typedef psyq::geometry::ball<coordinate_type> ball_type;
            auto const local_ball(
                ball_type::make(coordinate_type::make(float(2)), 10));
            auto const local_ball_aabb(
                psyq::geometry::make_aabb(local_ball));

            typedef psyq::geometry::segment<coordinate_type> segment_type;
            segment_type const local_segment(
                local_ball.get_center(),
                coordinate_type::make(local_ball.get_radius()));
            auto const local_segment_aabb(
                psyq::geometry::make_aabb(local_segment));

            typedef psyq::geometry::ray<coordinate_type> ray_type;
            auto const local_ray(
                ray_type::make(
                    local_segment.get_origin(),
                    local_segment.get_direction()));
            auto const local_ray_aabb(
                psyq::geometry::make_aabb(local_ray));

            coordinate_type::cross_product(
                coordinate_type::make(1, 0, 0),
                coordinate_type::make(0, 1, 0));
        }
    }
}

#endif // !defined(PSYQ_GEOMETRY_COORDINATE_HPP_)
