/* Copyright (c) 2013, Hillco Psychi, All rights reserved.

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
   AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
   THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
   PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
   OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
   WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
   OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
   ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief 幾何形状オブジェクト群。
 */
#ifndef PSYQ_GEOMETIC_SHAPE_HPP_
#define PSYQ_GEOMETIC_SHAPE_HPP_
//#include "psyq/geometric_vector.hpp"

namespace psyq
{
    /// @cond
    template<typename> class geometric_sphere;
    template<typename> class geometric_segment;
    template<typename> class geometric_ray;
    template<typename> class geometric_cuboid;
    /// @endcond
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 球。
    @tparam template_vector @copydoc psyq::geometric_sphere::vector
 */
template<typename template_vector>
class psyq::geometric_sphere
{
    /// *thisの型。
    private: typedef psyq::geometric_sphere<template_vector> self;

    /// @copydoc psyq::geometric_vector::type
    public: typedef template_vector vector;

    /** @brief 球を構築する。
        @param[in] in_center 球の中心位置。
        @param[in] in_radius 球の半径。0以上であること。
     */
    public: geometric_sphere(
        template_vector const& in_center,
        typename geometric_vector<template_vector>::element const in_radius)
     :
        center_(in_center),
        radius_(in_radius)
    {
        PSYQ_ASSERT(0 <= in_radius);
    }

    /** @brief 球の中心位置を取得する。
        @return 球の中心位置。
     */
    public: template_vector const& get_center() const
    {
        return this->center_;
    }

    /** @brief 球の中心位置を設定する。
        @param[in] in_center 新たに設定する球の中心位置。
     */
    public: void set_center(template_vector const& in_center)
    {
        this->center_ = in_center;
    }

    /** @brief 球の半径を取得する。
        @return 球の半径。
     */
    public: typename psyq::geometric_vector<template_vector>::element
    get_radius() const
    {
        return this->radius_;
    }

    /** @brief 球の半径を設定する。
        @param[in] in_radius 新たに設定する球の半径。0未満の場合は0になる。
     */
    public: void set_radius(
        typename psyq::geometric_vector<template_vector>::element const
            in_radius)
    {
        this->radius_ = 0 < in_radius? in_radius: 0;
    }

    /** @brief 球を構築する。
        @param[in] in_center 球の中心位置。
        @param[in] in_radius 球の半径。0未満の場合は0になる。
     */
    public: static self make(
        template_vector const& in_center,
        typename psyq::geometric_vector<template_vector>::element const
            in_radius)
    {
        return self(in_center, 0 < in_radius? in_radius: 0);
    }

    /// 球の中心位置。
    private: template_vector center_;
    /// 球の半径。
    private: typename psyq::geometric_vector<template_vector>::element radius_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 線分。
    @tparam template_vector @copydoc psyq::geometric_segment::vector
 */
template<typename template_vector>
class psyq::geometric_segment
{
    /// *thisの型。
    private: typedef psyq::geometric_segment<template_vector> self;

    /// @copydoc psyq::geometric_vector::type
    public: typedef template_vector vector;

    /** @brief 線分を構築する。
        @param[in] in_origin    線分の始点位置。
        @param[in] in_direction 線分の方向ベクトル。
     */
    public: geometric_segment(
        template_vector const& in_origin,
        template_vector const& in_direction)
    :
        origin_(in_origin),
        direction_(in_direction)
    {}

    /** @brief 線分の始点位置を取得する。
        @return 線分の始点位置。
     */
    public: template_vector const& get_origin() const
    {
        return this->origin_;
    }

    /** @brief 線分の始点位置を設定する。
        @param[in] in_origin 新たに設定する線分の始点位置。
     */
    public: void set_origin(template_vector const& in_origin)
    {
        this->origin_ = in_origin;
    }

    /** @brief 線分の方向ベクトルを取得する。
        @return 線分の方向ベクトル。
     */
    public: template_vector const& get_direction() const
    {
        return this->direction_;
    }

    /** @brief 線分の方向ベクトルを設定する。
        @param[in] in_direction 新たに設定する線分の方向ベクトル。
     */
    public: void set_direction(template_vector const& in_direction)
    {
        this->direction_ = in_direction;
    }

    private: template_vector origin_;    ///< 線分の始点。
    private: template_vector direction_; ///< 線分の方向と大きさ。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 半線分。
    @tparam template_vector @copydoc psyq::geometric_ray::vector
 */
template<typename template_vector>
class psyq::geometric_ray: public psyq::geometric_segment<template_vector>
{
    /// *thisの型。
    private: typedef psyq::geometric_ray<template_vector> self;
    /// *thisの上位型。
    public: typedef psyq::geometric_segment<template_vector> super;

    /// @copydoc psyq::geometric_vector::type
    public: typedef template_vector vector;

    /// @cond
    public: class triangle;
    /// @endcond

    /** @brief 半線分を構築する。
        @param[in] in_origin 半線分の始点位置。
        @param[in] in_direction
            半線分の方向の単位ベクトル。正規化されていること。
     */
    public: geometric_ray(
        template_vector const& in_origin,
        template_vector const& in_direction)
    :
        super(in_origin, in_direction)
    {
        PSYQ_ASSERT(psyq::geometric_vector_is_normalized(in_direction));
    }

    /** @brief 半線分の方向ベクトルを設定する。
        @param[in] in_direction
            新たに設定する線分の方向ベクトル。
            内部で正規化するので、正規化されてなくともよい。
            ただし0ベクトルの場合は、任意の単位ベクトルが設定される。
     */
    public: void set_direction(template_vector const& in_direction)
    {
        this->direction_ = self::make_direction(in_direction);
    }

    /** @brief 半線分を構築する。
        @param[in] in_origin 半線分の始点位置。
        @param[in] in_direction
            半線分の方向ベクトル。
            内部で正規化するので、正規化されてなくともよい。
            ただし0ベクトルの場合は、任意の単位ベクトルが設定される。
     */
    public: static self make(
        template_vector const& in_origin,
        template_vector const& in_direction)
    {
        return self(in_origin, self::make_direction(in_direction));
    }

    /** @brief 半線分の方向ベクトルを構築する。

        in_direction が0ベクトルだった場合は、任意の単位ベクトルを返す。

        @param[in] in_direction 方向ベクトル。
        @return 正規化した方向ベクトル。
     */
    private: static template_vector make_direction(
        template_vector const& in_direction)
    {
        auto const local_length(psyq::geometric_vector_length(in_direction));
        if (local_length <= 0)
        {
            //PSYQ_ASSERT(false);
            auto local_direction(
                psyq::geometric_vector<template_vector>::make(0));
            psyq::geometric_vector_element(local_direction, 0, 1);
            return local_direction;
        }
        return in_direction / local_length;
    }
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 半直線と衝突する三角形。
    @tparam template_vector @copydoc psyq::geometric_cuboid::vector
 */
template<typename template_vector>
class psyq::geometric_ray<template_vector>::triangle
{
    /// *thisの型。
    private: typedef triangle self;

    /// @copydoc psyq::geometric_vector::type
    public: typedef template_vector vector;

    /** @brief 初期化。
        @param[in] in_vertex0 三角形の頂点#0
        @param[in] in_vertex1 三角形の頂点#1
        @param[in] in_vertex2 三角形の頂点#2
     */
    public: triangle(
        template_vector const& in_vertex0,
        template_vector const& in_vertex1,
        template_vector const& in_vertex2)
    :
        origin_(in_vertex0)
    {
#if 1
        auto local_edge1(in_vertex1 - in_vertex0);
        auto local_edge2(in_vertex2 - in_vertex0);
        this->normal_
            = psyq::geometric_vector_cross(local_edge1, local_edge2);
        auto const local_nx(
            psyq::geometric_vector_cross(local_edge2, this->normal_));
        auto const local_ny(
            psyq::geometric_vector_cross(local_edge1, this->normal_));
        this->binormal_u_ = local_nx
            / psyq::geometric_vector_dot(local_edge1, local_nx);
        this->binormal_v_ = local_ny
            / psyq::geometric_vector_dot(local_edge2, local_ny);
#else
        auto const local_e2(
            psyq::geometric_vector_cross(in_vertex0, in_vertex1));
        auto const local_d(
            psyq::geometric_vector_dot(local_e2, in_vertex2));
        if (local_d <= 0)
        {
            PSYQ_ASSERT(false);
            return;
        }
        auto const local_e1(
            psyq::geometric_vector_cross(in_vertex2, in_vertex0));
        this->binormal_u_ = local_e1 / local_d;
        this->binormal_v_ = local_e2 / local_d;
        this->normal_ = psyq::geometric_vector_cross(
            in_vertex1 - in_vertex0, in_vertex2 - in_vertex0);
        return true;
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
    bool intersect(
        template_vector& out_tuv,
        psyq::geometric_ray<template_vector> const& in_ray,
        typename psyq::geometric_vector<template_vector>::element const
            in_ray_length,
        typename psyq::geometric_vector<template_vector>::element const
            in_epsilon = 0)
    const
    {
        auto const local_nv(
            -psyq::geometric_vector_dot(
                in_ray.get_direction(), this->get_normal()));
        if (local_nv <= in_epsilon)
        {
            return false;
        }

        auto const local_origin_diff(in_ray.get_origin() - this->get_origin());
        auto const local_t(
            psyq::geometric_vector_dot(local_origin_diff, this->get_normal())
            / local_nv);
        if (local_t < 0 || in_ray_length < local_t)
        {
            return false;
        }

        auto const local_position(
            in_ray.get_direction() * local_t + local_origin_diff);
        auto const local_u(
            psyq::geometric_vector_dot(local_position, this->binormal_u_));
        if (local_u < in_epsilon)
        {
            return false;
        }
        auto const local_v(
            psyq::geometric_vector_dot(local_position, this->binormal_v_));
        if (local_v < in_epsilon || 1 - in_epsilon < local_u + local_v)
        {
            return false;
        }

        out_tuv = psyq::geometric_vector<template_vector>::make(
            local_t, local_u, local_v);
        return true;
    }

    template_vector const& get_origin() const
    {
        return this->origin_;
    }

    template_vector const& get_normal() const
    {
        return this->normal_;
    }

    private: template_vector origin_;     ///< 三角形の開始位置
    private: template_vector normal_;     ///< 三角形の法線
    private: template_vector binormal_u_; ///< 三角形の重心座標U
    private: template_vector binormal_v_; ///< 三角形の重心座標V 
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 直方体。
    @tparam template_vector @copydoc psyq::geometric_cuboid::vector
 */
template<typename template_vector>
class psyq::geometric_cuboid
{
    /// *thisの型。
    private: typedef psyq::geometric_cuboid<template_vector> self;

    /// @copydoc psyq::geometric_vector::type
    public: typedef template_vector vector;

    //public: typedef std::array<template_vector, 3> axis_array;

    /** @brief 直方体を構築する。
        @param[in] in_center 直方体の中心位置。
        @param[in] in_axis_x
            直方体のX軸方向ベクトル。正規化されていること。
            in_axis_y と in_axis_z の両方と直交していること。
        @param[in] in_axis_y
            直方体のY軸方向ベクトル。正規化されていること。
            in_axis_x と in_axis_z の両方と直交していること。
        @param[in] in_axis_z
            直方体のZ軸方向ベクトル。正規化されていること。
            in_axis_x と in_axis_y の両方と直交していること。
        @param[in] in_extent
            直方体の大きさの1/2。すべての要素が0以上であること。
     */
    public: geometric_cuboid(
        template_vector const& in_center,
        template_vector const& in_axis_x,
        template_vector const& in_axis_y,
        template_vector const& in_axis_z,
        template_vector const& in_extent)
    :
        center_(in_center),
        axis_x_(in_axis_x),
        axis_y_(in_axis_y),
        axis_z_(in_axis_z),
        extent_(in_extent)
    {
        PSYQ_ASSERT(psyq::geometric_vector_is_normalized(in_axis_x));
        PSYQ_ASSERT(psyq::geometric_vector_is_normalized(in_axis_y));
        PSYQ_ASSERT(psyq::geometric_vector_is_normalized(in_axis_z));
        PSYQ_ASSERT(0 <= psyq::geometric_vector_element(in_extent, 0));
        PSYQ_ASSERT(0 <= psyq::geometric_vector_element(in_extent, 1));
        PSYQ_ASSERT(0 <= psyq::geometric_vector_element(in_extent, 2));
    }

    /** @brief 直方体の中心位置を取得する。
        @return 直方体の中心位置。
     */
    public: template_vector const& get_center() const
    {
        return this->center_;
    }

    /** @brief 直方体のX軸方向ベクトルを取得する。
        @return 直方体のX軸方向の単位ベクトル。
     */
    public: template_vector const& get_axis_x() const
    {
        return this->axis_x_;
    }

    /** @brief 直方体のY軸方向ベクトルを取得する。
        @return 直方体のY軸方向の単位ベクトル。
     */
    public: template_vector const& get_axis_y() const
    {
        return this->axis_y_;
    }

    /** @brief 直方体のZ軸方向ベクトルを取得する。
        @return 直方体のZ軸方向の単位ベクトル。
     */
    public: template_vector const& get_axis_z() const
    {
        return this->axis_z_;
    }

    /** @brief 直方体の大きさの1/2を取得する。
        @return 直方体の大きさの1/2。
     */
    public: template_vector const& get_extent() const
    {
        return this->extent_;
    }

    private: template_vector center_; ///< 直方体の中心位置。
    private: template_vector axis_x_; ///< 直方体のX軸方向の単位ベクトル。
    private: template_vector axis_y_; ///< 直方体のY軸方向の単位ベクトル。
    private: template_vector axis_z_; ///< 直方体のZ軸方向の単位ベクトル。
    private: template_vector extent_; ///< 直方体の大きさの1/2。
};

#endif // !defined(PSYQ_GEOMETIC_SHAPE_HPP_)
