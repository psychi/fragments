/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief @copydoc psyq::geometry::ray
    @ingroup psyq_geometry psyq::geometry
 */
#ifndef PSYQ_GEOMETRY_RAY_HPP_
#define PSYQ_GEOMETRY_RAY_HPP_

//#include "psyq/geometry/line.hpp"

/// @cond
namespace psyq
{
    namespace geometry
    {
        template<typename> class ray;
        template<typename> class barycentric_triangle;
    } // namespace geometry
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 半直線。
    @tparam template_coordinate @copydoc this_type::coordinate
    @ingroup psyq_geometry_shape
 */
template<typename template_coordinate>
class psyq::geometry::ray: public psyq::geometry::line<template_coordinate>
{
    /// thisが指す値の型。
    private: typedef ray this_type;

    /// this_type の基底型。
    public: typedef psyq::geometry::line<template_coordinate> base_type;

    //-------------------------------------------------------------------------
    /** @brief 半直線を構築する。
        @param[in] in_origin    半直線の原点。
        @param[in] in_direction 半直線の方向。
     */
    public: ray(
        typename base_type::point const& in_origin,
        typename base_type::direction const& in_direction)
    PSYQ_NOEXCEPT:
    base_type(in_origin, in_direction)
    {}

    /** @brief 半直線を構築する。
        @param[in] in_line 半直線として用いる直線。
     */
    public: explicit ray(base_type const& in_line): base_type(in_line) {}

}; // namespace psyq::geometry::ray

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 重心座標系を持つ三角形。

    以下の web page を参考にした。
    http://d.hatena.ne.jp/ototoi/20050320/p1

    @tparam template_coordinate @copydoc this_type::coordinate
    @ingroup psyq_geometry_shape
 */
template<typename template_coordinate>
class psyq::geometry::barycentric_triangle
{
    /// thisが指す値の型。
    private: typedef barycentric_triangle this_type;

    /// @copydoc psyq::geometry::plane::coordinate
    public: typedef template_coordinate coordinate;

    /// @cond
    public: class ray_collision;
    /// @endcond

    //-------------------------------------------------------------------------
    /** @brief 初期化。
        @param[in] in_vertex_0 三角形の頂点#0
        @param[in] in_vertex_1 三角形の頂点#1
        @param[in] in_vertex_2 三角形の頂点#2
     */
    public: static this_type make(
        typename this_type::coordinate::vector const& in_vertex_0,
        typename this_type::coordinate::vector const& in_vertex_1,
        typename this_type::coordinate::vector const& in_vertex_2)
    {
#if 1
        auto const local_edge_1(
            this_type::coordinate::make(in_vertex_1 - in_vertex_0));
        auto const local_edge_2(
            this_type::coordinate::make(in_vertex_2 - in_vertex_0));
        auto const local_normal(
            psyq::geometry::vector::cross_3d(local_edge_1, local_edge_2));
        auto const local_nx(
            psyq::geometry::vector::cross_3d(local_edge_2, local_normal));
        auto const local_ny(
            psyq::geometry::vector::cross_3d(local_edge_1, local_normal));
        auto const local_dot_1(
            psyq::geometry::vector::dot(local_edge_1, local_nx));
        auto const local_dot_2(
            psyq::geometry::vector::dot(local_edge_2, local_ny));
        PSYQ_ASSERT(local_dot_1 != 0);
        PSYQ_ASSERT(local_dot_2 != 0);
#else
        auto const local_cross_1(
            psyq::geometry::vector::cross_3d(in_vertex_2, in_vertex_0));
        auto const local_cross_2(
            psyq::geometry::vector::cross_3d(in_vertex_0, in_vertex_1));
        // in_vertex_0 か in_vertex_2 が0ベクトルだと、dが0になってしまう。
        auto const local_d(
            psyq::geometry::vector::dot(local_cross_2, in_vertex_2));
        PSYQ_ASSERT(local_d != 0);
        auto const local_inverse_d(1 / local_d);
        return this_type(
            this_type::coordinate::make(in_vertex_0),
            psyq::geometry::vector::cross_3d(
                in_vertex_1 - in_vertex_0, in_vertex_2 - in_vertex_0),
            local_cross_1 * local_inverse_d,
            local_cross_2 * local_inverse_d);
#endif
    }

    public: typename this_type::coordinate::vector const& get_origin() const
    {
        return this->origin_;
    }

    public: typename this_type::coordinate::vector const& get_normal() const
    {
        return this->normal_;
    }

    public: typename this_type::coordinate::vector const& get_binormal_u() const
    {
        return this->binormal_u_;
    }

    public: typename this_type::coordinate::vector const& get_binormal_v() const
    {
        return this->binormal_v_;
    }

    //-------------------------------------------------------------------------
    private: barycentric_triangle(
        typename this_type::coordinate::vector const& in_origin,
        typename this_type::coordinate::vector const& in_normal,
        typename this_type::coordinate::vector const& in_binormal_u,
        typename this_type::coordinate::vector const& in_binormal_v)
    :
    origin_(in_origin),
    normal_(in_normal),
    binormal_u_(in_binormal_u),
    binormal_v_(in_binormal_v)
    {}

    //-------------------------------------------------------------------------
    /// 三角形の原点
    private: typename this_type::coordinate::vector origin_;
    /// 三角形の法線
    private: typename this_type::coordinate::vector normal_;
    /// 三角形の重心座標U
    private: typename this_type::coordinate::vector binormal_u_;
    /// 三角形の重心座標V
    private: typename this_type::coordinate::vector binormal_v_;

}; // class psyq::geometry::ray::barycentric_triangle

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 三角形と半直線の衝突判定。
    @tparam template_coordinate @copydoc psyq::geometry::plane::coordinate
 */
template<typename template_coordinate>
class psyq::geometry::barycentric_triangle<template_coordinate>::ray_collision
{
    /// thisが指す値の型。
    private: typedef ray_collision this_type;

    /// @copydoc psyq::geometry::point::coordinate
    public: typedef template_coordinate coordinate;

    //-------------------------------------------------------------------------
    /** @brief 三角形と半直線の衝突判定を構築する。

        以下の web page を参考にした。
        http://d.hatena.ne.jp/ototoi/20050320/p1

        衝突した場合、衝突位置は以下の式で算出できる。
        @code
        in_ray.make_point(this->get_line_position())
        @endcode

        また頂点毎に法線がある場合、以下の式で衝突位置の法線が算出できる。
        @code
        normal_0 * (1 - this->get_u_position() - this->get_v_position())
        + normal_1 * this->get_u_position()
        + normal_2 * this->get_v_position()
        @endcode

        @param[in] in_triangle 衝突判定の左辺となる三角形。
        @param[in] in_ray      衝突判定の右辺となる半直線。
        @param[in] in_ray_end  半直線の衝突区間の終点位置。
        @param[in] in_epsilon  計算機epsilon値。 
    */
    public: static this_type make(
        psyq::geometry::barycentric_triangle<template_coordinate> const& in_triangle,
        psyq::geometry::ray<template_coordinate> const& in_ray,
        typename this_type::coordinate::element const in_ray_end,
        typename this_type::coordinate::element const in_epsilon)
    {
        auto const local_nv(
            -psyq::geometry::vector::dot(
                in_ray.direction_.get_unit(), in_triangle.get_normal()));
        if (local_nv <= in_epsilon)
        {
            return false;
        }

        auto const local_origin_difference(
            in_ray.origin_.get_position() - in_triangle.get_origin());
        auto const local_line_position(
            psyq::geometry::vector::dot(local_origin_difference, in_triangle.get_normal())
            / local_nv);
        if (local_line_position < 0 || in_ray_end < local_line_position)
        {
            return false;
        }

        auto const local_cross_position(
            in_ray.direction_.get_unit() * local_line_position
            + local_origin_difference);
        auto const local_u_position(
            psyq::geometry::vector::dot(
                local_cross_position, in_triangle.get_binormal_u()));
        if (local_u_position < in_epsilon)
        {
            return false;
        }
        auto const local_v_position(
            psyq::geometry::vector::dot(
                local_cross_position, in_triangle.get_binormal_v()));
        if (local_v_position < in_epsilon
            || 1 - in_epsilon < local_u_position + local_v_position)
        {
            return false;
        }
        return this_type(
            local_cross_position,
            local_line_position,
            local_u_position,
            local_v_position);
    }

    //-------------------------------------------------------------------------
    private: ray_collision(
        typename this_type::coordinate::vector const& in_cross_position,
        typename this_type::coordinate::element const in_line_position,
        typename this_type::coordinate::element const in_u_position,
        typename this_type::coordinate::element const in_v_position)
    :
    cross_position_(in_cross_position),
    line_position_(in_line_position),
    u_position_(in_u_position),
    v_position_(in_v_position)
    {}

    //-------------------------------------------------------------------------
    private: typename this_type::coordinate::vector cross_position_;
    /// 直線の原点から交点までの、直線上の位置。
    private: typename this_type::coordinate::element line_position_;
    /// 三角形の重心座標Uでの、交点の位置。
    private: typename this_type::coordinate::element u_position_;
    /// 三角形の重心座標Vでの、交点の位置。
    private: typename this_type::coordinate::element v_position_;

}; // psyq::geometry::barycentric_triangle::ray_collision

#endif // PSYQ_GEOMETRY_RAY_HPP_
