#ifndef PSYQ_MOSP_PRIMITIVE_HPP_
#define PSYQ_MOSP_PRIMITIVE_HPP_

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<typename template_space>
class mosp_primitive
{
    private: typedef mosp_primitive<template_space> self;

    public: typedef template_space space;

    public: typedef psyq::mosp_tree<self*, template_space> tree;

    //-------------------------------------------------------------------------
    protected: mosp_primitive(): handle_(this) {}

    /// 空間分割木から取り外す。
    public: virtual ~mosp_primitive() {}

    //-------------------------------------------------------------------------
    /** @brief thisが持つ AABB を取得する。
     */
    public: typename self::space::coordinates::aabb const& get_aabb() const
    {
        return this->aabb_;
    }

    /** @brief thisが持つ AABB を更新する。
     */
    protected: virtual void update_aabb() = 0;

    //-------------------------------------------------------------------------
    protected: void detach_tree()
    {
        this->handle_.detach_tree();
    }

    //-------------------------------------------------------------------------
    /// プリミティブの絶対座標系AABB。
    protected: typename self::scape::coordinates::aabb aabb_;
    /// プリミティブに対応する衝突判定ハンドル。
    private: typename self::tree::handle handle_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
#if 0
    /** @brief 衝突を判定する形状のAABBを算出する。

        ここの実装では、インターフェイスを提供するのみで、何も行わない。
        形状ごとに特殊化したテンプレート関数を実装すること。

        @param[out] out_aabb 算出したAABBの出力先。
        @param[in]  in_shape AABBを算出する形状。
     */
    template<typename template_vector, typename template_shape>
    void calc_mosp_primitive_aabb(
        psyq::geometric_aabb<template_vector>& out_aabb,
        template_shape const&                  in_shape)
    {
        PSYQ_ASSERT(false);
    }

    /** @brief 球のAABBを算出する。
        @param[out] out_aabb  算出したAABBの出力先。
        @param[in]  in_sphere AABBを算出する球。
     */
    template<typename template_vector> void calc_mosp_primitive_aabb(
        psyq::geometric_aabb<template_vector>&         out_aabb,
        psyq::geometric_sphere<template_vector> const& in_sphere)
    {
        template_vector const local_extent(
            in_sphere.radius, in_sphere.radius, in_sphere.radius);
        out_aabb = psyq::geometric_aabb<template_vector>(
            out_aabb.minimum = in_sphere.center - local_extent;
            out_aabb.maximum = in_sphere.center + local_extent);
    }

    /** @brief 半直線のAABBを算出する。
        @param[out] out_aabb 算出したAABBの出力先。
        @param[in]  in_ray   AABBを算出する半直線。
     */
    template<typename template_vector> void calc_mosp_primitive_aabb(
        psyq::geometric_aabb<template_vector>&      out_aabb,
        psyq::geometric_ray<template_vector> const& in_sphere)
    {
        typedef typename template_vector::value_type element_type;

        // X座標の範囲を決定する。
        element_type local_min_x;
        element_type local_max_x;
        if (in_ray.get_direction()[0] < 0)
        {
            out_aabb.minimum[0] = -(std::numeric_limits<element>::max)();
            out_aabb.maximum[0] = in_ray.origin[0];
        }
        else if (0 < in_ray.get_direction()[0])
        {
            out_aabb.minimum[0] = in_ray.origin[0];
            out_aabb.maximum[0] = (std::numeric_limits<element>::max)();
        }
        else
        {
            out_aabb.minimum[0] = in_ray.origin[0];
            out_aabb.maximum[0] = in_ray.origin[0];
        }

        // Y座標の範囲を決定する。
        if (in_ray.direction[1] < 0)
        {
            out_aabb.minimum[1] = -(std::numeric_limits<element_type>::max)();
            out_aabb.maximum[1] = in_ray.origin[1];
        }
        else if (0 < in_ray.direction[1])
        {
            out_aabb.minimum[1] = in_ray.origin[1];
            out_aabb.maximum[1] = (std::numeric_limits<element_type>::max)();
        }
        else
        {
            out_aabb.minimum[1] = in_ray.origin[1];
            out_aabb.maximum[1] = in_ray.origin[1];
        }

        // Z座標の範囲を決定する。
        if (in_ray.direction[2] < 0)
        {
            out_aabb.minimum[2] = -(std::numeric_limits<element_type>::max)();
            out_aabb.maximum[2] = in_ray.origin[2];
        }
        else if (0 < in_ray.direction[2])
        {
            out_aabb.minimum[2] = in_ray.origin[2];
            out_aabb.maximum[2] = (std::numeric_limits<element_type>::max)();
        }
        else
        {
            out_aabb.minimum[2] = in_ray.origin[2];
            out_aabb.maximum[2] = in_ray.origin[2];
        }
    }

    /** @brief 直線のAABBを算出する。
        @param[out] out_aabb   算出したAABBの出力先。
        @param[in]  in_segment AABBを算出する直線。
     */
    public: template<> static void calc_aabb(
        typename self::space::coordinates::aabb& out_aabb,
        typename self::segment const&            in_segment)
    {
        auto const local_end(in_segment.origin + in_segment.direction);

        // X座標の範囲を決定する。
        if (in_segment.direction[0] < 0)
        {
            out_aabb.minimum[0] = local_end[0];
            out_aabb.maximum[0] = in_segment.origin[0];
        }
        else
        {
            out_aabb.minimum[0] = in_segment.origin[0];
            out_aabb.maximum[0] = local_end[0];
        }

        // Y座標の範囲を決定する。
        if (in_segment.direction[1] < 0)
        {
            out_aabb.minimum[1] = local_end[1];
            out_aabb.maximum[1] = in_segment.origin[1];
        }
        else
        {
            out_aabb.minimum[1] = in_segment.origin[1];
            out_aabb.maximum[1] = local_end[1];
        }

        // Z座標の範囲を決定する。
        if (in_segment.direction[2] < 0)
        {
            out_aabb.minimum[2] = local_end[2];
            out_aabb.maximum[2] = in_segment.origin[2];
        }
        else
        {
            out_aabb.minimum[2] = in_segment.origin[2];
            out_aabb.maximum[2] = local_end[2];
        }
    }

    /** @brief 直方体のAABBを算出する。
        @param[out] out_aabb 算出したAABBの出力先。
        @param[in]  in_cube  AABBを算出する直方体。
     */
    template<typename template_vector> void calc_mosp_primitive_aabb(
        psyq::geometric_aabb<template_vector>&         out_aabb,
        psyq::geometric_cuboid<template_vector> const& in_cuboid)
    {
        auto const local_abs_vector(
            [](template_vector const& in_vector)->template_vector
            {
                auto const local_x(in_vector[0]);
                auto const local_y(in_vector[1]);
                auto const local_z(in_vector[2]);
                return template_vector(
                    local_x < 0? -local_x: local_x,
                    local_y < 0? -local_y: local_y,
                    local_z < 0? -local_z: local_z);
            });
        auto const local_half_diagonal(
            local_abs_vector(
                in_cuboid.get_axis_x() * in_cuboid.get_extent()[0])
            + local_abs_vector(
                in_cuboid.get_axis_y() * in_cuboid.get_extent()[1])
            + local_abs_vector(
                in_cuboid.get_axis_z() * in_cuboid.get_extent()[2]));
        out_aabb.minimum = in_cuboid.get_center() - local_half_diagonal;
        out_aabb.maximum = in_cuboid.get_center() + local_half_diagonal;
    }
#endif
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<typename template_space, typename template_shape>
class mosp_shape: public mosp_primitive<template_space>
{
    private: typedef mosp_shape<template_space, template_shape> self;
    public: typedef mosp_primitive<template_space> super;

    public: typedef template_shape shape;

    //-------------------------------------------------------------------------
    /// 空の衝突判定オブジェクトを構築する。
    public: mosp_shape() {}

    /** @brief 衝突判定に使う形状を構築する。
        @param[in] in_shape 衝突判定に使う形状の初期値。
     */
    public: explicit mosp_shape(template_shape const& in_shape):
        shape_(in_shape)
    {}

    //-------------------------------------------------------------------------
    /** @brief 衝突判定に使う形状を取得する。
        @return 衝突判定に使う形状。
     */
    public: template_shape const& get_const_shape() const
    {
        return this->shape_;
    }

    /** @brief 衝突判定に使う形状を取得する。

        衝突判定に使う形状を直接更新したい場合は、
        この関数の戻り値の参照先の形状を更新すること。

        @return 衝突判定に使う形状。
     */
    public: template_shape& get_shape()
    {
        this->detach_tree();
        return this->shape_;
    }

    //-------------------------------------------------------------------------
    protected: virtual void update_aabb() override
    {
        super::calc_aabb(this->aabb_, this->get_const_shape());
    };

    //-------------------------------------------------------------------------
    protected: template_shape shape_; ///< 衝突判定に使う形状。
};

#endif // !defined(PSYQ_MOSP_PRIMITIVE_HPP_)
