/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copybrief psyq::geometry::aabb
 */
#ifndef PSYQ_GEOMETRY_AABB_HPP_
#define PSYQ_GEOMETRY_AABB_HPP_

//#include "psyq/geometry/shape.hpp"

/// @cond
namespace psyq
{
    namespace geometry
    {
        template<typename> class aabb;
    }
}
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 座標軸に平行な直方体。Axis-Aligned-Bounding-Box.
    @tparam template_coordinate @copydoc this_type::coordinate
    @ingroup psyq_geometry_shape
 */
template<typename template_coordinate>
class psyq::geometry::aabb
{
    /// thisが指す値の型。
    private: typedef aabb this_type;

    /// psyq::geometry::coordinate 互換の座標の型特性。
    public: typedef template_coordinate coordinate;

    /** @brief this_type::detect_line_collision() で使う、直線との衝突情報。

        - タプル要素#0は、直線の基準位置から直線上の衝突区間の開始位置までの距離。
          - 直線とAABBが衝突してる場合は、タプル要素#1より必ず小さい値となる。
          - 直線とAABBが衝突してない場合は、タプル要素#1以上の値となる。
        - タプル要素#1は、直線の基準位置から直線上の衝突区間の終了位置までの距離。
          - 直線とAABBが衝突してる場合は、タプル要素#0より必ず大きい値となる。
          - 直線とAABBが衝突してない場合は、タプル要素#0以下の値となる。
        - タプル要素#2が…
          - 0なら、直線上の衝突区間の開始位置を検知しなかった。
          - 負値なら、直線上の衝突区間の開始位置がAABBの最小面にある。
            「1 - (タプル要素#2)」が、軸のインデックス番号。
          - 正値なら、直線上の衝突区間の開始位置がAABBの最大面にある。
            「(タプル要素#2) - 1」が、軸のインデックス番号。
        - タプル要素#3が…
          - 0なら、直線上の衝突区間の終了位置を検知しなかった。
          - 負値なら、直線上の衝突区間の終了位置がAABBの最小面にある。
            「1 - (タプル要素#3)」が、軸のインデックス番号。
          - 正値なら、直線上の衝突区間の終了位置がAABBの最大面にある。
            「(タプル要素#3) - 1」が、軸のインデックス番号。
     */
    public: typedef std::tuple<
        typename this_type::coordinate::element,
        typename this_type::coordinate::element,
        std::int8_t,
        std::int8_t>
            line_collision;

    //-------------------------------------------------------------------------
    /** @brief AABBを構築する。
        @param[in] in_min AABBの最小座標。
        @param[in] in_max AABBの最大座標。
     */
    public: aabb(
        typename this_type::coordinate::vector const& in_min,
        typename this_type::coordinate::vector const& in_max)
    :
    min_((
        PSYQ_ASSERT(psyq::geometry::vector::less_than_equal(in_min, in_max)),
        PSYQ_ASSERT(this_type::coordinate::validate(in_min)),
        in_min)),
    max_((
        PSYQ_ASSERT(this_type::coordinate::validate(in_min)),
        in_max))
    {}

    /** @brief AABBの最小座標を取得する。
        @return AABBの最小座標。
     */
    public: typename this_type::coordinate::vector const& get_min()
    const PSYQ_NOEXCEPT
    {
        return this->min_;
    }

    /** @brief AABBの最大座標を取得する。
        @return AABBの最大座標。
     */
    public: typename this_type::coordinate::vector const& get_max()
    const PSYQ_NOEXCEPT
    {
        return this->max_;
    }

    /** @brief 2つの点を含む最小のAABBを構築する。
        @return 点Aと点Bを包むAABB。
        @param[in] in_point_a 点Aの座標。
        @param[in] in_point_b 点Bの座標。
     */
    public: static this_type make(
        typename this_type::coordinate::vector const& in_point_a,
        typename this_type::coordinate::vector const& in_point_b)
    {
        auto local_min(this_type::coordinate::make(in_point_a));
        auto local_max(this_type::coordinate::make(in_point_b));
        for (unsigned i(0); i < this_type::coordinate::dimension; ++i)
        {
            auto const local_element_a(
                psyq::geometry::vector::const_at(in_point_a, i));
            auto const local_element_b(
                psyq::geometry::vector::const_at(in_point_b, i));
            if (local_element_b < local_element_a)
            {
                psyq::geometry::vector::at(local_min, i) = local_element_b;
                psyq::geometry::vector::at(local_max, i) = local_element_a;
            }
        }
        return this_type(local_min, local_max);
    }

    //-------------------------------------------------------------------------
    /** @brief 他のAABBと衝突しているか判定する。
        @retval true  衝突している。
        @retval false 衝突してない。
        @param[in] in_target 判定対象となるAABB。
     */
    public: bool detect_collision(this_type const& in_target) const
    {
        return psyq::geometry::vector::less_than_equal(this->get_min(), in_target.get_max())
            && psyq::geometry::vector::less_than_equal(in_target.get_min(), this->get_max());
    }

    /** @brief AABBと点が衝突しているか判定する。
        @param[in] in_point 判定対象となる点。
     */
    public: bool detect_point_collision(
        typename this_type::coordinate::vector const& in_point)
    {
        PSYQ_ASSERT(this_type::coordinate::validate(in_point));
        return psyq::geometry::vector::less_than_equal(this->get_min(), in_point)
            && psyq::geometry::vector::less_than_equal(in_point, this->get_max());
    }

    /** @brief AABBと球が衝突しているか判定する。
        @param[in] in_center 判定対象となる球の中心座標。
        @param[in] in_radius 判定対象となる球の半径。
     */
    public: bool detect_ball_collision(
        typename this_type::coordinate::vector const& in_center,
        typename this_type::coordinate::element const in_radius);

    //-------------------------------------------------------------------------
    /** @brief AABBと直線が衝突しているか判定する。

        以下のウェブページを参考にして実装した。
        http://marupeke296.com/COL_3D_No18_LineAndAABB.html

        @return AABBと直線の衝突判定の結果。
        @param[in] in_line_position 直線上の基準位置。
        @param[in] in_line_normal   直線方向の正規化ベクトル。
        @param[in] in_epsilon_mag   誤差の範囲に使うエプシロン値の倍率。
     */
    public: typename this_type::line_collision detect_line_collision(
        typename this_type::coordinate::vector const& in_line_position,
        typename this_type::coordinate::vector const& in_line_normal,
        unsigned const in_epsilon_mag =
            PSYQ_GEOMETRY_NEARLY_EQUAL_EPSILON_MAG_DEFAULT)
    const
    {
        PSYQ_ASSERT(this_type::coordinate::validate(in_line_position));
        PSYQ_ASSERT(this_type::coordinate::validate(in_line_normal));
        auto const local_odd(
            this_type::make_normal_odd(
                in_line_normal,
                std::numeric_limits<typename this_type::coordinate::element>::epsilon()
                * in_epsilon_mag));
        std::int8_t local_face_max(0);
        std::int8_t local_face_min(0);
        auto local_t_max(
            (std::numeric_limits<typename this_type::coordinate::element>::max)());
        auto local_t_min(-local_t_max);
        auto const local_diff_max(
            (this->get_max() - in_line_position) * local_odd);
        auto const local_diff_min(
            (this->get_min() - in_line_position) * local_odd);
        for (unsigned i(0); i < this_type::coordinate::dimension; ++i)
        {
            if (psyq::geometry::vector::const_at(local_odd, i) != 0)
            {
                // スラブとの距離を算出する。
                auto local_t_far(
                    psyq::geometry::vector::const_at(local_diff_max, i));
                auto local_t_near(
                    psyq::geometry::vector::const_at(local_diff_min, i));
                auto local_face_far(static_cast<std::int8_t>(i + 1));
                auto local_face_near(-local_face_far);
                if (local_t_far < local_t_near)
                {
                    std::swap(local_t_near, local_t_far);
                    std::swap(local_face_near, local_face_far);
                }
                if (local_t_min < local_t_near)
                {
                    local_t_min = local_t_near;
                    local_face_min = local_face_near;
                }
                if (local_t_far < local_t_max)
                {
                    local_t_max = local_t_far;
                    local_face_max = local_face_far;
                }

                // スラブ交差判定。
                if (local_t_max <= local_t_min)
                {
                    return std::make_tuple(
                        local_t_max,
                        local_t_min,
                        local_face_max,
                        local_face_min);
                }
            }
            else
            {
                // 直線方向と軸が平行だった。
                auto const local_line(
                    psyq::geometry::vector::const_at(in_line_position, i));
                auto const local_aabb_min(
                    psyq::geometry::vector::const_at(this->get_min(), i));
                auto const local_aabb_max(
                    psyq::geometry::vector::const_at(this->get_max(), i));
                if (local_line < local_aabb_min || local_aabb_max < local_line)
                {
                    return std::make_tuple(
                        local_t_max,
                        local_t_min,
                        local_face_max,
                        local_face_min);
                }
            }
        }
        return std::make_tuple(
            local_t_min, local_t_max, local_face_min, local_face_max);
    }

    private: static typename this_type::coordinate::vector make_normal_odd(
        typename this_type::coordinate::vector const& in_nomral,
        typename this_type::coordinate::element const in_epsilon)
    {
        PSYQ_ASSERT(
            this_type::coordinate::nearly_length(in_line_normal, 1));
        typename this_type::coordinate::element_array local_odd;
        for (unsigned i(0); i < this_type::coordinate::dimension; ++i)
        {
            auto const local_element(
                psyq::geometry::vector::const_at(in_nomral, i));
            local_odd[i] = in_epsilon < std::abs(local_element)?
                1 / local_element: 0;
        }
        return this_type::coordinate::make(local_odd);
    }

    //-------------------------------------------------------------------------
    private: typename this_type::coordinate::vector min_; ///< AABBの最小座標。
    private: typename this_type::coordinate::vector max_; ///< AABBの最大座標。

}; // class psyq::geometry::aabb

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    namespace geometry
    {
        //---------------------------------------------------------------------
        /** @brief 球のAABBを構築する。
            @return 球のAABB。
            @param[in] in_ball AABBを構築する球。
            @ingroup psyq_geometry_shape
         */
        template<typename template_coordinate>
        typename template_coordinate::aabb make_aabb(
            psyq::geometry::ball<template_coordinate> const& in_ball)
        {
            auto const local_extent(
                template_coordinate::make(in_ball.get_radius()));
            return typename template_coordinate::aabb(
                in_ball.get_center() - local_extent,
                in_ball.get_center() + local_extent);
        }

        //---------------------------------------------------------------------
        /** @brief 線分のAABBを構築する。
            @return 線分のAABB。
            @param[in] in_segment AABBを構築する線分。
            @ingroup psyq_geometry_shape
         */
        template<typename template_coordinate>
        typename template_coordinate::aabb make_aabb(
            psyq::geometry::segment<template_coordinate> const& in_segment)
        {
            typename template_coordinate::element_array local_min;
            typename template_coordinate::element_array local_max;
            auto const local_end(
                in_segment.get_origin() + in_segment.get_direction());
            for (unsigned i(0); i < template_coordinate::dimension; ++i)
            {
                auto const local_direction(
                    psyq::geometry::vector::const_at(
                        in_segment.get_direction(), i));
                auto const local_origin(
                    psyq::geometry::vector::const_at(
                        in_segment.get_origin(), i));
                auto const local_end_element(
                    psyq::geometry::vector::const_at(local_end, i));
                if (local_direction < 0)
                {
                    local_min[i] = local_end_element;
                    local_max[i] = local_origin;
                }
                else
                {
                    local_min[i] = local_origin;
                    local_max[i] = local_end_element;
                }
            }
            return typename template_coordinate::aabb(
                template_coordinate::make(local_min),
                template_coordinate::make(local_max));
        }

        //---------------------------------------------------------------------
        /** @brief 半直線のAABBを構築する。
            @return 半直線のAABB。
            @param[in] in_ray AABBを構築する半直線。
            @ingroup psyq_geometry_shape
         */
        template<typename template_coordinate>
        typename template_coordinate::aabb make_aabb(
            psyq::geometry::ray<template_coordinate> const& in_ray)
        {
            typename template_coordinate::element_array local_min;
            typename template_coordinate::element_array local_max;
            for (unsigned i(0); i < template_coordinate::dimension; ++i)
            {
                auto const local_direction(
                    psyq::geometry::vector::const_at(in_ray.get_direction(), i));
                auto const local_origin(
                    psyq::geometry::vector::const_at(in_ray.get_origin(), i));
                if (local_direction < 0)
                {
                    local_min[i] = -(std::numeric_limits<typename template_coordinate::element>::max)();
                    local_max[i] = local_origin;
                }
                else if (0 < local_direction)
                {
                    local_min[i] = local_origin;
                    local_max[i] = (std::numeric_limits<typename template_coordinate::element>::max)();
                }
                else
                {
                    local_min[i] = local_origin;
                    local_max[i] = local_origin;
                }
            }
            return typename template_coordinate::aabb(
                template_coordinate::make(local_min),
                template_coordinate::make(local_max));
        }

        //---------------------------------------------------------------------
        /** @brief 直方体のAABBを構築する。
            @return 直方体のAABB。
            @param[in] in_box AABBを構築する直方体。
            @ingroup psyq_geometry_shape
         */
        template<typename template_coordinate>
        typename template_coordinate::aabb make_aabb(
            psyq::geometry::box<template_coordinate> const& in_box)
        {
            typename template_coordinate::element_array local_elements;
            auto local_half_diagonal(
                template_coordinate::make(
                    static_cast<typename template_coordinate::element>(0)));
            for (unsigned i(0); i < template_coordinate::dimension; ++i)
            {
                auto const local_axis(
                    in_box.get_axes()[i] *
                        psyq::geometry::vector::const_at(
                            in_box.get_extent(), i));
                for (unsigned j(0); j < template_coordinate::dimension; ++j)
                {
                    local_elements[j] = std::abs(
                        psyq::geometry::vector::const_at(local_axis, j));
                }
                local_half_diagonal = local_half_diagonal +
                    template_coordinate::make(local_elements);
            }
            return typename template_coordinate::aabb(
                in_box.get_center() - local_half_diagonal,
                in_box.get_center() + local_half_diagonal);
        }

        //---------------------------------------------------------------------
        /** @brief AABBのAABBを構築する。
            @return AABBのAABB。
            @param[in] in_aabb AABBを構築するAABB。
            @ingroup psyq_geometry_shape
         */
        template<typename template_coordinate>
        typename template_coordinate::aabb make_aabb(
            psyq::geometry::aabb<template_coordinate> const& in_aabb)
        {
            return in_aabb
        }

    } // namespace geometry
} // namespace psyq

#endif // !defined(PSYQ_GEOMETRY_AABB_HPP_)
