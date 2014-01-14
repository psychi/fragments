#ifndef PSYQ_MOSP_PRIMITIVE_
#define PSYQ_MOSP_PRIMITIVE_

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<typename template_space>
class mosp_primitive
{
    private: typedef mosp_primitive<template_space> self;

    public: typedef psyq::mosp_tree<self*, template_space> tree;

    /// 衝突を判定する形状の最小座標と最大座標を要素とするAABB。
    public: struct aabb
    {
        typename template_space::vector min;
        typename template_space::vector max;
    };

    /// 衝突判定に使える球。
    public: struct sphere
    {
        sphere(
            typename template_space::vector const&            in_center,
            typename template_space::vector::value_type const in_radius)
        :
            center(in_center),
            radius(in_radius)
        {}

        typename template_space::vector             center;
        typename template_space::vector::value_type radius;
    };

    /// 衝突判定に使える半線分。
    public: struct ray
    {
        typename template_space::vector origin;
        typename template_space::vector direction;
    };

    /// 衝突判定に使える線分。
    public: struct segment: public ray {};

    /// 衝突判定に使える直方体。
    public: struct cuboid
    {
        typename template_space::vector center;  ///< 直方体の中心位置。
        typename template_space::vector axis[3]; ///< 直方体の向き。
        typename template_space::vector extent;  ///< 直方体の大きさ。
    };

    //-------------------------------------------------------------------------
    protected: mosp_primitive(): handle_(this) {}

    /// 空間分割木から取り外す。
    public: virtual ~mosp_primitive() {}

    //-------------------------------------------------------------------------
    /** @brief thisが持つ AABB を取得する。
     */
    public: typename self::aabb const& get_aabb() const
    {
        return this->aabb_;
    }

    /** @brief thisが持つ AABB を更新する。
     */
    protected: virtual void update_aabb() = 0;

    /** @brief 衝突を判定する形状の AABB を算出する。

        ここの実装では、インターフェイスを提供するのみで、何も行わない。
        形状ごとに特殊化したテンプレート関数を実装すること。

        @param[out] out_aabb 算出したAABBの出力先。
        @param[in]  in_shape AABBを算出する形状。
     */
    public: template<typename template_shape>
    static void calc_aabb(
        typename self::aabb&  out_aabb,
        template_shape const& in_shape)
    {
        PSYQ_ASSERT(false);
    }

    /** @brief 球のAABBを算出する。
        @param[out] out_aabb  算出したAABBの出力先。
        @param[in]  in_sphere AABBを算出する球。
     */
    public: template<> static void calc_aabb(
        typename self::aabb&         out_aabb,
        typename self::sphere const& in_sphere)
    {
        typename template_space::vector const local_extent(
            in_sphere.radius, in_sphere.radius, in_sphere.radius);
        out_aabb.min = in_sphere.center - local_extent;
        out_aabb.max = in_sphere.center + local_extent;
    }

    /** @brief 半直線のAABBを算出する。
        @param[out] out_aabb 算出したAABBの出力先。
        @param[in]  in_ray   AABBを算出する半直線。
     */
    public: template<> static void calc_aabb(
        typename self::aabb&      out_aabb,
        typename self::ray const& in_ray)
    {
        typedef typename template_space::vector::value_type element_type;

        // X座標の範囲を決定する。
        if (in_ray.direction.x < 0)
        {
            out_aabb.min.x = -(std::numeric_limits<element_type>::max)();
            out_aabb.max.x = in_ray.origin.x;
        }
        else if (0 < in_ray.direction.x)
        {
            out_aabb.min.x = in_ray.origin.x;
            out_aabb.max.x = (std::numeric_limits<element_type>::max)();
        }
        else
        {
            out_aabb.min.x = in_ray.origin.x;
            out_aabb.max.x = in_ray.origin.x;
        }

        // Y座標の範囲を決定する。
        if (in_ray.direction.y < 0)
        {
            out_aabb.min.y = -(std::numeric_limits<element_type>::max)();
            out_aabb.max.y = in_ray.origin.y;
        }
        else if (0 < in_ray.direction.y)
        {
            out_aabb.min.y = in_ray.origin.y;
            out_aabb.max.y = (std::numeric_limits<element_type>::max)();
        }
        else
        {
            out_aabb.min.y = in_ray.origin.y;
            out_aabb.max.y = in_ray.origin.y;
        }

        // Z座標の範囲を決定する。
        if (in_ray.direction.z < 0)
        {
            out_aabb.min.z = -(std::numeric_limits<element_type>::max)();
            out_aabb.max.z = in_ray.origin.z;
        }
        else if (0 < in_ray.direction.z)
        {
            out_aabb.min.z = in_ray.origin.z;
            out_aabb.max.z = (std::numeric_limits<element_type>::max)();
        }
        else
        {
            out_aabb.min.z = in_ray.origin.z;
            out_aabb.max.z = in_ray.origin.z;
        }
    }

    /** @brief 直線のAABBを算出する。
        @param[out] out_aabb   算出したAABBの出力先。
        @param[in]  in_segment AABBを算出する直線。
     */
    public: template<> static void calc_aabb(
        typename self::aabb&          out_aabb,
        typename self::segment const& in_segment)
    {
        auto const local_end(in_segment.origin + in_segment.direction);

        // X座標の範囲を決定する。
        if (in_segment.direction.x < 0)
        {
            out_aabb.min.x = local_end.x;
            out_aabb.max.x = in_segment.origin.x;
        }
        else
        {
            out_aabb.min.x = in_segment.origin.x;
            out_aabb.max.x = local_end.x;
        }

        // Y座標の範囲を決定する。
        if (in_segment.direction.y < 0)
        {
            out_aabb.min.y = local_end.y;
            out_aabb.max.y = in_segment.origin.y;
        }
        else
        {
            out_aabb.min.y = in_segment.origin.y;
            out_aabb.max.y = local_end.y;
        }

        // Z座標の範囲を決定する。
        if (in_segment.direction.z < 0)
        {
            out_aabb.min.z = local_end.z;
            out_aabb.max.z = in_segment.origin.z;
        }
        else
        {
            out_aabb.min.z = in_segment.origin.z;
            out_aabb.max.z = local_end.z;
        }
    }

    /** @brief 直方体のAABBを算出する。
        @param[out] out_aabb 算出したAABBの出力先。
        @param[in]  in_cube  AABBを算出する直方体。
     */
    public: template<> static void calc_aabb(
        typename self::aabb&         out_aabb,
        typename self::cuboid const& in_cuboid)
    {
        auto const local_half_diagonal(
            self::abs_vector(in_cuboid.axis[0] * in_cuboid.extent.x) +
            self::abs_vector(in_cuboid.axis[1] * in_cuboid.extent.y) +
            self::abs_vector(in_cuboid.axis[2] * in_cuboid.extent.z));
        out_aabb.min = in_cuboid.center - local_half_diagonal;
        out_aabb.max = in_cuboid.center + local_half_diagonal;
    }

    private: static typename template_space::vector abs_vector(
        typename template_space::vector const& in_vector)
    {
        return typename template_space::vector(
            in_vector.x < 0? -in_vector.x: in_vector.x,
            in_vector.y < 0? -in_vector.y: in_vector.y,
            in_vector.z < 0? -in_vector.z: in_vector.z);
    }

    //-------------------------------------------------------------------------
    protected: void detach_tree()
    {
        this->handle_.detach_tree();
    }

    //-------------------------------------------------------------------------
    protected: typename self::aabb         aabb_;
    private:   typename self::tree::handle handle_;
};

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

#endif // !defined(PSYQ_MOSP_PRIMITIVE_)
