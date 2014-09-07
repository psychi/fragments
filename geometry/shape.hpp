/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief 幾何形状オブジェクト群。
    @defgroup psyq_geometry_shape 幾何形状オブジェクト
    @ingroup psyq_geometry psyq::geometry
 */
#ifndef PSYQ_GEOMETRY_SHAPE_HPP_
#define PSYQ_GEOMETRY_SHAPE_HPP_

/// @cond
namespace psyq
{
    namespace geometry
    {
        template<typename> class ball;
        template<typename> class segment;
        template<typename> class ray;
        template<typename> class box;
    } // namespace geometry
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 球。
    @tparam template_coordinate @copydoc this_type::coordinate
    @ingroup psyq_geometry_shape
 */
template<typename template_coordinate>
class psyq::geometry::ball
{
    /// thisが指す値の型。
    private: typedef ball this_type;

    /** @brief 座標系の型特性。

        psyq::geometry::coordinate 互換のインターフェイスを持っていること。
     */
    public: typedef template_coordinate coordinate;

    //-------------------------------------------------------------------------
    /** @brief 球を構築する。
        @param[in] in_center 球の中心位置。
        @param[in] in_radius 球の半径。0以上であること。
     */
    public: ball(
        typename this_type::coordinate::vector const& in_center,
        typename this_type::coordinate::element const in_radius)
    :
    center_((
        PSYQ_ASSERT(this_type::coordinate::validate(in_center)), in_center)),
    radius_((PSYQ_ASSERT(0 <= in_radius), in_radius))
    {}

    /** @brief 球を構築する。
        @param[in] in_center 球の中心位置。
        @param[in] in_radius 球の半径。0未満の場合は0になる。
     */
    public: static this_type make(
        typename this_type::coordinate::vector const& in_center,
        typename this_type::coordinate::element const in_radius)
    {
        return this_type(
            this_type::coordinate::make(in_center),
            0 < in_radius? in_radius: 0);
    }

    /** @brief 球の中心位置を取得する。
        @return 球の中心位置。
     */
    public: typename this_type::coordinate::vector const& get_center()
    const PSYQ_NOEXCEPT
    {
        return this->center_;
    }

    /** @brief 球の中心位置を設定する。
        @param[in] in_center 新たに設定する球の中心位置。
     */
    public: void set_center(
        typename this_type::coordinate::vector const& in_center)
    {
        this->center_ = this_type::coordinate::make(in_center);
    }

    /** @brief 球の半径を取得する。
        @return 球の半径。
     */
    public: typename this_type::coordinate::element get_radius()
    const PSYQ_NOEXCEPT
    {
        return this->radius_;
    }

    /** @brief 球の半径を設定する。
        @param[in] in_radius 新たに設定する球の半径。0未満の場合は0になる。
     */
    public: void set_radius(
        typename this_type::coordinate::element const in_radius)
    {
        this->radius_ = 0 < in_radius? in_radius: 0;
    }

    //-------------------------------------------------------------------------
    /** @brief 他の球と衝突しているか判定する。
        @retval true  衝突している。
        @retval false 衝突してない。
        @param[in] in_target 判定対象となる球。
     */
    public: bool detect_collision(this_type const& in_target) const
    {
        auto const local_diff(in_target.get_center() - this->get_center());
        auto const local_square_distance(
            psyq::geometry::vector::dot(local_diff, local_diff));
        auto const local_range(in_target.get_radius() + this->get_radius());
        return local_square_distance <= local_range * local_range;
    }

    //-------------------------------------------------------------------------
    /// 球の中心位置。
    private: typename this_type::coordinate::vector center_;
    /// 球の半径。
    private: typename this_type::coordinate::element radius_;

}; // class psyq::geometry::ball

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 線分。
    @tparam template_coordinate @copydoc this_type::coordinate
    @ingroup psyq_geometry_shape
 */
template<typename template_coordinate>
class psyq::geometry::segment
{
    /// thisが指す値の型。
    private: typedef segment this_type;

    /// @copydoc psyq::geometry::ball::coordinate
    public: typedef template_coordinate coordinate;

    //-------------------------------------------------------------------------
    /** @brief 線分を構築する。
        @param[in] in_origin    線分の始点位置。
        @param[in] in_direction 線分の方向ベクトル。
     */
    public: segment(
        typename this_type::coordinate::vector const& in_origin,
        typename this_type::coordinate::vector const& in_direction)
    :
    origin_((
        PSYQ_ASSERT(this_type::coordinate::validate(in_origin)), in_origin)),
    direction_((
        PSYQ_ASSERT(this_type::coordinate::validate(in_direction)), in_direction))
    {}

    /** @brief 線分を構築する。
        @param[in] in_origin    線分の始点位置。
        @param[in] in_direction 線分の方向ベクトル。
     */
    public: static this_type make(
        typename this_type::coordinate::vector const& in_origin,
        typename this_type::coordinate::vector const& in_direction)
    {
        return this_type(
            this_type::coordinate::make(in_origin),
            this_type::coordinate::make(in_direction));
    }

    /** @brief 線分の始点位置を取得する。
        @return 線分の始点位置。
     */
    public: typename this_type::coordinate::vector const& get_origin()
    const PSYQ_NOEXCEPT
    {
        return this->origin_;
    }

    /** @brief 線分の始点位置を設定する。
        @param[in] in_origin 新たに設定する線分の始点位置。
     */
    public: void set_origin(
        typename this_type::coordinate::vector const& in_origin)
    {
        this->origin_ = this_type::coordinate::make(in_origin);
    }

    /** @brief 線分の方向ベクトルを取得する。
        @return 線分の方向ベクトル。
     */
    public: typename this_type::coordinate::vector const& get_direction()
    const PSYQ_NOEXCEPT
    {
        return this->direction_;
    }

    /** @brief 線分の方向ベクトルを設定する。
        @param[in] in_direction 新たに設定する線分の方向ベクトル。
     */
    public: void set_direction(
        typename this_type::coordinate::vector const& in_direction)
    {
        this->direction_ = this_type::coordinate::make(in_direction);
    }

    //-------------------------------------------------------------------------
    /// 線分の始点。
    private: typename this_type::coordinate::vector origin_;
    /// 線分の方向と大きさ。
    private: typename this_type::coordinate::vector direction_;

}; // class psyq::geometry::segment

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief半直線。
    @tparam template_coordinate @copydoc this_type::coordinate
    @ingroup psyq_geometry_shape
 */
template<typename template_coordinate>
class psyq::geometry::ray:
    public psyq::geometry::segment<template_coordinate>
{
    /// thisが指す値の型。
    private: typedef ray this_type;

    /// this_type の基底型。
    public: typedef psyq::geometry::segment<template_coordinate> base_type;

    /// @cond
    public: class triangle_3d;
    /// @endcond
    //-------------------------------------------------------------------------
    /** @brief 半直線を構築する。
        @param[in] in_origin    半直線の始点位置。
        @param[in] in_direction 半直線の方向ベクトル。
     */
    public: ray(
        typename base_type::coordinate::vector const& in_origin,
        typename base_type::coordinate::vector const& in_direction)
    PSYQ_NOEXCEPT:
    base_type(
        (
            PSYQ_ASSERT(base_type::coordinate::validate(in_origin)),
            in_origin),
        (
            PSYQ_ASSERT(
                base_type::coordinate::validate(in_direction) &&
                psyq::geometry::vector::nearly_length(in_direction, 1)),
            in_direction))
    {}

    /** @brief 半直線の方向ベクトルを設定する。
        @param[in] in_direction
            新たに設定する半直線の方向ベクトル。
            内部で正規化するので、正規化されてなくともよい。
            ただし0ベクトルの場合は、任意の単位ベクトルが設定される。
     */
    public: void set_direction(
        typename base_type::coordinate::vector const& in_direction)
    {
        this->direction_ = psyq::geometry::vector::normalize(
            this_type::coordinate::make(in_direction));
    }

    /** @brief 半直線を構築する。
        @param[in] in_origin 半直線の始点位置。
        @param[in] in_direction
            半直線の方向ベクトル。
            内部で正規化するので、正規化されてなくともよい。
            ただし0ベクトルの場合は、任意の単位ベクトルが設定される。
     */
    public: static this_type make(
        typename base_type::coordinate::vector const& in_origin,
        typename base_type::coordinate::vector const& in_direction)
    {
        return this_type(
            this_type::coordinate::make(in_origin),
            psyq::geometry::vector::normalize(
                this_type::coordinate::make(in_direction)));
    }

}; // namespace psyq::geometry::ray

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 半直線と衝突する三角形。
    @tparam template_coordinate @copydoc this_type::coordinate
    @ingroup psyq_geometry_shape
 */
template<typename template_coordinate>
class psyq::geometry::ray<template_coordinate>::triangle_3d
{
    /// thisが指す値の型。
    private: typedef triangle_3d this_type;

    /// @copydoc psyq::geometry::ball::coordinate
    public: typedef template_coordinate coordinate;

    //-------------------------------------------------------------------------
    /** @brief 初期化。
        @param[in] in_vertex0 三角形の頂点#0
        @param[in] in_vertex1 三角形の頂点#1
        @param[in] in_vertex2 三角形の頂点#2
     */
    public: triangle_3d(
        typename this_type::coordinate::vector const& in_vertex0,
        typename this_type::coordinate::vector const& in_vertex1,
        typename this_type::coordinate::vector const& in_vertex2)
    :
    origin_(in_vertex0)
    {
        PSYQ_ASSERT(base_type::coordinate::validate(in_vertex0));
        PSYQ_ASSERT(base_type::coordinate::validate(in_vertex1));
        PSYQ_ASSERT(base_type::coordinate::validate(in_vertex2));
#if 1
        auto local_edge1(in_vertex1 - in_vertex0);
        auto local_edge2(in_vertex2 - in_vertex0);
        this->normal_ = psyq::geometry::vector::cross_3d(
            local_edge1, local_edge2);
        auto const local_nx(
            psyq::geometry::vector::cross_3d(local_edge2, this->normal_));
        auto const local_ny(
            psyq::geometry::vector::cross_3d(local_edge1, this->normal_));
        this->binormal_u_ = local_nx /
            psyq::geometry::vector::dot(local_edge1, local_nx);
        this->binormal_v_ = local_ny /
            psyq::geometry::vector::dot(local_edge2, local_ny);
#else
        auto const local_e2(
            psyq::geometry::vector::cross_3d(in_vertex0, in_vertex1));
        auto const local_d(
            psyq::geometry::vector::dot(local_e2, in_vertex2));
        if (local_d <= 0)
        {
            PSYQ_ASSERT(false);
            return;
        }
        auto const local_e1(
            psyq::geometry::vector::cross_3d(in_vertex2, in_vertex0));
        this->binormal_u_ = local_e1 / local_d;
        this->binormal_v_ = local_e2 / local_d;
        this->normal_ = psyq::geometry::vector::cross_3d(
            in_vertex1 - in_vertex0, in_vertex2 - in_vertex0);
#endif
    }

    /** @brief 半直線との衝突判定。

        以下の web page を参考にした。
        http://d.hatena.ne.jp/ototoi/20050320/p1

        衝突した場合、衝突位置は以下の式で算出できる。
        in_half_line.origin + in_half_line.direction * out_intersection.x

        また頂点毎に法線がある場合、以下の式で衝突位置の法線が算出できる。
        normal0 * (1 - out_intersection.y - out_intersection.z) +
        normal1 * out_intersection.y +
        normal2 * out_intersection.z

        @param[out] out_tuv       衝突時の[t,u,v]値。
        @param[in]  in_ray        判定する半直線。
        @param[in]  in_ray_length 半直線の長さ。
        @param[in]  in_epsilon    計算機epsilon値。
        @retval true  半直線と衝突した。
        @retval false 半直線と衝突しなかった。
     */
    public: bool intersect(
        typename this_type::coordinate::vector& out_tuv,
        psyq::geometry::ray<template_coordinate> const& in_ray,
        typename this_type::coordinate::element const in_ray_length,
        typename this_type::coordinate::element const in_epsilon = 0)
    const
    {
        auto const local_nv(
            -psyq::geometry::vector::dot(
                in_ray.get_direction(), this->get_normal()));
        if (local_nv <= in_epsilon)
        {
            return false;
        }

        auto const local_origin_diff(in_ray.get_origin() - this->get_origin());
        auto const local_t(
            psyq::geometry::vector::dot(
                local_origin_diff, this->get_normal()) / local_nv);
        if (local_t < 0 || in_ray_length < local_t)
        {
            return false;
        }

        auto const local_position(
            in_ray.get_direction() * local_t + local_origin_diff);
        auto const local_u(
            psyq::geometry::vector::dot(
                local_position, this->binormal_u_));
        if (local_u < in_epsilon)
        {
            return false;
        }
        auto const local_v(
            psyq::geometry::vector::dot(local_position, this->binormal_v_));
        if (local_v < in_epsilon || 1 - in_epsilon < local_u + local_v)
        {
            return false;
        }

        out_tuv = this_type::coordinate::make(local_t, local_u, local_v);
        return true;
    }

    public: typename this_type::coordinate::vector const& get_origin() const
    {
        return this->origin_;
    }

    public: typename this_type::coordinate::vector const& get_normal() const
    {
        return this->normal_;
    }

    //-------------------------------------------------------------------------
    /// 三角形の開始位置
    private: typename this_type::coordinate::vector origin_;
    /// 三角形の法線
    private: typename this_type::coordinate::vector normal_;
    /// 三角形の重心座標U
    private: typename this_type::coordinate::vector binormal_u_;
    /// 三角形の重心座標V 
    private: typename this_type::coordinate::vector binormal_v_;

}; // class psyq::geometry::ray::triangle_3d

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 直方体。
    @tparam template_coordinate @copydoc this_type::coordinate
    @ingroup psyq_geometry_shape
 */
template<typename template_coordinate>
class psyq::geometry::box
{
    /// thisが指す値の型。
    private: typedef box this_type;

    /// @copydoc psyq::geometry::ball::coordinate
    public: typedef template_coordinate coordinate;

    /// 直方体の軸方向の単位ベクトルの配列。
    public: typedef std::array<
        typename this_type::coordinate::vector,
        this_type::coordinate::dimension>
            axis_array;

    //-------------------------------------------------------------------------
    /** @brief 直方体を構築する。
        @param[in] in_center 直方体の中心位置。
        @param[in] in_extent 直方体の大きさの1/2。すべての要素が0以上であること。
        @param[in] in_axes   直方体の各軸方向の単位ベクトルの配列。
     */
    public: box(
        typename this_type::coordinate::vector const& in_center,
        typename this_type::coordinate::vector const& in_extent,
        typename this_type::axis_array const& in_axes)
    :
    center_((
        PSYQ_ASSERT(this_type::coordinate::validate(in_center)), in_center)),
    extent_((
        PSYQ_ASSERT(this_type::coordinate::validate(in_extent)), in_extent)),
    axes_(in_axes)
    {
        for (unsigned i(0); i < this_type::coordinate::dimension; ++i)
        {
            PSYQ_ASSERT(0 <= psyq::geometry::vector::const_at(in_extent, i));
            PSYQ_ASSERT(this_type::coordinate::validate(in_axes[i]));
            PSYQ_ASSERT(psyq::geometry::vector::nearly_length(in_axes[i], 1));
        }
    }

    /** @brief 直方体の中心位置を取得する。
        @return 直方体の中心位置。
     */
    public: typename this_type::coordinate::vector const& get_center()
    const PSYQ_NOEXCEPT
    {
        return this->center_;
    }

    /** @brief 直方体の大きさの1/2を取得する。
        @return 直方体の大きさの1/2。
     */
    public: typename this_type::coordinate::vector const& get_extent()
    const PSYQ_NOEXCEPT
    {
        return this->extent_;
    }

    /** @brief 直方体の軸方向ベクトルの配列を取得する。
        @return 直方体の軸方向の単位ベクトルの配列。
     */
    public: typename this_type::axis_array const& get_axes()
    const PSYQ_NOEXCEPT
    {
        return this->axes_;
    }

    /** @brief 直方体を作る。
        @param[in] in_center   直方体の中心位置。
        @param[in] in_extent   直方体の大きさの1/2。
        @param[in] in_rotation 直方体の回転ラジアン。
        @param[in] in_axis     直方体の回転軸。
     */
    public: static this_type make_cuboid(
        typename this_type::coordinate::vector const& in_center,
        typename this_type::coordinate::vector const& in_extent,
        typename this_type::coordinate::element const in_rotation,
        typename this_type::coordinate::vector const& in_axis)
    {
        static_assert(
            3 <= this_type::coordinate::dimension,
            "'this_type::coordinate::dimension' is less than 3.");

        // 回転軸と回転角度から四元数を算出する。
        auto const local_half_rotation(in_rotation / 2);
        auto const local_half_sin(std::sin(local_half_rotation));
        auto const local_axis(
            psyq::geometry::vector::normalize(
                this_type::coordinate::make(in_axis)));
        auto const local_qx(
            local_half_sin * psyq::geometry::vector::const_at(local_axis, 0));
        auto const local_qy(
            local_half_sin * psyq::geometry::vector::const_at(local_axis, 1));
        auto const local_qz(
            local_half_sin * psyq::geometry::vector::const_at(local_axis, 2));
        auto const local_qw(std::cos(local_half_rotation));

        // 四元数から軸ベクトルを算出する。
        auto const local_xx(local_qx * local_qx);
        auto const local_xy(local_qx * local_qy);
        auto const local_xz(local_qx * local_qz);
        auto const local_xw(local_qx * local_qw);
        auto const local_yy(local_qy * local_qy);
        auto const local_yz(local_qy * local_qz);
        auto const local_yw(local_qy * local_qw);
        auto const local_zz(local_qz * local_qz);
        auto const local_zw(local_qz * local_qw);
        typename this_type::axis_array local_axes;
        local_axes[0] = this_type::coordinate::make(
            (local_yy + local_zz) * -2 + 1,
            (local_xy + local_zw) * 2,
            (local_xz - local_yw) * 2);
        local_axes[1] = this_type::coordinate::make(
            (local_xy - local_zw) * 2,
            (local_xx + local_zz) * -2 + 1,
            (local_yz + local_xw) * 2);
        local_axes[2] = this_type::coordinate::make(
            (local_xz + local_yw) * 2,
            (local_yz - local_xw) * 2,
            (local_xx + local_yy) * -2 + 1);

        // 大きさを正規化する。
        auto local_extent(this_type::coordinate::make(in_extent));
        for (unsigned i(0); i < this_type::coordinate::dimension; ++i)
        {
            auto const local_element(
                psyq::geometry::vector::const_at(in_extent, i));
            if (local_element < 0)
            {
                psyq::geometry::vector::at(local_extent, i) = -local_element;
            }
        }
        return this_type(
            this_type::coordinate::make(in_center), local_extent, local_axes);
    }

    //-------------------------------------------------------------------------
    /// 直方体の中心位置。
    private: typename this_type::coordinate::vector center_;
    /// 直方体の大きさの1/2。
    private: typename this_type::coordinate::vector extent_;
    /// 直方体の軸方向の単位ベクトルの配列。
    private: typename this_type::axis_array axes_;

}; // class psyq::geometry::box

#endif // !defined(PSYQ_GEOMETRY_SHAPE_HPP_)
