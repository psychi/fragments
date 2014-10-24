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

    /// @cond
    public: class triangle_3d;
    /// @endcond

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
                in_ray.direction_.get_unit(), this->get_normal()));
        if (local_nv <= in_epsilon)
        {
            return false;
        }

        auto const local_origin_diff(in_ray.origin_.get_position() - this->origin_.get_position());
        auto const local_t(
            psyq::geometry::vector::dot(
                local_origin_diff, this->get_normal()) / local_nv);
        if (local_t < 0 || in_ray_length < local_t)
        {
            return false;
        }

        auto const local_position(
            in_ray.direction_.get_unit() * local_t + local_origin_diff);
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

#endif // PSYQ_GEOMETRY_RAY_HPP_
