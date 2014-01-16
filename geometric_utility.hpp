#ifndef PSYQ_GEOMETIC_UTILITY_HPP_
#define PSYQ_GEOMETIC_UTILITY_HPP_

#ifndef PSYQ_NEARLY_EQUAL_EPSILON_MAG_DEFAULT
#define PSYQ_NEARLY_EQUAL_EPSILON_MAG_DEFAULT 1
#endif

namespace psyq
{
    /// @cond
    template<typename> class geometric_sphere;
    template<typename> class geometric_segment;
    template<typename> class geometric_ray;
    template<typename> class geometric_cuboid;
    template<typename> class geometric_aabb;
    /// @endcond

    /** @brief 2つの浮動小数点値がほぼ等値か比較する。
        @param[in] in_left_value  比較する浮動小数点値の左辺値。
        @param[in] in_left_value  比較する浮動小数点値の右辺値。
        @param[in] in_epsilon_mag 誤差の範囲に使うエプシロン値の倍率。
        @retval true  ほぼ等値だった。
        @retval false 等値ではなかった。
     */
    template<typename template_value>
    bool is_nearly_equal(
        template_value const in_left_value,
        template_value const in_right_value,
        unsigned const       in_epsilon_mag
            = PSYQ_NEARLY_EQUAL_EPSILON_MAG_DEFAULT)
    {
        auto const local_epsilon(
            std::numeric_limits<template_value>::epsilon() * in_epsilon_mag);
        auto const local_diff(in_left_value - in_right_value);
        return -local_epsilon <= local_diff && local_diff <= local_epsilon;
    }

    /** @brief 幾何ベクトルが持つ要素の型を特定する。
        @tparam template_vector
            要素の型を特定する幾何ベクトルの型。
            以下のメンバ関数が定義されている必要がある。
            @code
            template_vector::operator[](unsigned)
            @endcode
     */
    template<typename template_vector>
    struct geometric_vector_element
    {
        /// template_vector が持つ要素の型。
        typedef typename std::remove_reference<
            decltype(template_vector().operator[](0))>::type
                type;
    };

    /** @brief 幾何ベクトルの長さの2乗を算出する。
        @param[in] in_vector 長さの2乗を算出する幾何ベクトル。
        @return 幾何ベクトルの長さの2乗。
     */
    template<typename template_vector>
    typename geometric_vector_element<template_vector>::type
    square_geometric_vector_length(template_vector const& in_vector)
    {
        auto const local_x(in_vector.operator[](0));
        auto const local_y(in_vector.operator[](1));
        auto const local_z(in_vector.operator[](2));
        return local_x * local_x + local_y * local_y + local_z * local_z;
    }

    /** @brief 幾何ベクトルの長さを算出する。
        @param[in] in_vector 長さを算出する幾何ベクトル。
        @return 幾何ベクトルの長さ。
     */
    template<typename template_vector>
    typename geometric_vector_element<template_vector>::type
    calc_geometric_vector_length(template_vector const& in_vector)
    {
        return std::sqrt(psyq::square_geometric_vector_length(in_vector));
    }

    /** @brief 幾何ベクトルが正規化されているか判定する。
        @param[in] in_vector 判定する幾何ベクトル。
        @retval true  幾何ベクトルは正規化されている。
        @retval false 幾何ベクトルは正規化されていない。
     */
    template<typename template_vector>
    bool is_normalized_geometric_vector(template_vector const& in_vector)
    {
        return psyq::is_nearly_equal(
            psyq::square_geometric_vector_length(in_vector),
            typename psyq::geometric_vector_element<template_vector>::type(1));
    }
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 球。
    @tparam template_vector @copydoc psyq::geometric_sphehe::vector
 */
template<typename template_vector>
class psyq::geometric_sphere
{
    /// *thisの型。
    private: typedef psyq::geometric_sphere<template_vector> self;

    /// @copydoc psyq::geometric_aabb::vector
    public: typedef template_vector vector;

    /** @brief 球を構築する。
        @param[in] in_center 球の中心位置。
        @param[in] in_radius 球の半径。0以上であること。
     */
    public: geometric_sphere(
        template_vector const& in_center,
        typename geometric_vector_element<template_vector>::type const
            in_radius)
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
    public: typename psyq::geometric_vector_element<template_vector>::type
    get_radius() const
    {
        return this->radius_;
    }

    /** @brief 球の半径を設定する。
        @param[in] in_center 新たに設定する球の半径。0未満の場合は0になる。
     */
    public: void set_radius(
        typename psyq::geometric_vector_element<template_vector>::type const
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
        typename geometric_vector_element<template_vector>::type const
            in_radius)
    {
        return self(in_center, 0 < in_radius? in_radius: 0);
    }

    /// 球の中心位置。
    private: template_vector center_;
    /// 球の半径。
    private: typename geometric_vector_element<template_vector>::type radius_;
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

    /// @copydoc psyq::geometric_aabb::vector
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

    /// @copydoc psyq::geometric_aabb::vector
    public: typedef template_vector vector;

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
        PSYQ_ASSERT(psyq::is_normalized_geometric_vector(in_direction));
    }

    /** @brief 半線分の方向ベクトルを設定する。
        @param[in] in_direction
            新たに設定する線分の方向ベクトル。
            内部で正規化するので、正規化されてなくともよい。
            ただし0ベクトルの場合はassertし、任意の単位ベクトルが設定される。
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
            ただし0ベクトルの場合はassertし、任意の単位ベクトルが設定される。
     */
    public: static self make(
        template_vector const& in_origin,
        template_vector const& in_direction)
    {
        return self(in_origin, self::make_direction(in_direction));
    }

    /** @brief 半線分の方向ベクトルを構築する。

        in_direction が0ベクトルだった場合はassertし、
        任意の単位ベクトルを返す。

        @param[in] in_direction 方向ベクトル。
        @return 正規化した方向ベクトル。
     */
    private: static template_vector make_direction(
        template_vector const& in_direction)
    {
        auto const local_length(
            psyq::calc_geometric_vector_length(in_direction));
        if (local_length <= 0)
        {
            PSYQ_ASSERT(false);
            return template_vector(1, 0, 0);
        }
        return in_direction / local_length;
    }
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

    /// @copydoc psyq::geometric_aabb::vector
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
        PSYQ_ASSERT(psyq::is_normalized_geometric_vector(in_axis_x));
        PSYQ_ASSERT(psyq::is_normalized_geometric_vector(in_axis_y));
        PSYQ_ASSERT(psyq::is_normalized_geometric_vector(in_axis_z));
        PSYQ_ASSERT(0 <= in_extent.operator[](0));
        PSYQ_ASSERT(0 <= in_extent.operator[](1));
        PSYQ_ASSERT(0 <= in_extent.operator[](2));
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

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief axis-aligned-bounding-box. 座標軸に平行な直方体。
    @tparam template_vector @copydoc geometric_aabb::vector
 */
template<typename template_vector>
class psyq::geometric_aabb
{
    /// *thisの型。
    private: typedef psyq::geometric_aabb<template_vector> self;

    /// 座標を表す幾何ベクトルの型。
    public: typedef template_vector vector;

    /** @brief AABBを構築する。
        @param[in] in_min AABBの最小座標。
        @param[in] in_max AABBの最大座標。
     */
    public: geometric_aabb(
        template_vector const& in_min,
        template_vector const& in_max)
    :
        min_(in_min),
        max_(in_max)
    {
        PSYQ_ASSERT(in_min.operator[](0) <= in_max.operator[](0));
        PSYQ_ASSERT(in_min.operator[](1) <= in_max.operator[](1));
        PSYQ_ASSERT(in_min.operator[](2) <= in_max.operator[](2));
    }

    /** @brief AABBの最小座標を取得する。
        @return AABBの最小座標。
     */
    public: template_vector const& get_min() const
    {
        return this->min_;
    }

    /** @brief AABBの最大座標を取得する。
        @return AABBの最大座標。
     */
    public: template_vector const& get_max() const
    {
        return this->max_;
    }

    private: template_vector min_; ///< AABBの最小座標。
    private: template_vector max_; ///< AABBの最大座標。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    //-------------------------------------------------------------------------
    /** @brief 任意の幾何形状からAABBを構築するためのクラス。
        @tparam template_vector @copydoc psyq::geometric_shape_aabb::vector
        @tparam template_shape  @copydoc psyq::geometric_shape_aabb::shape
     */
    template<typename template_vector, typename template_shape>
    struct geometric_shape_aabb
    {
        /// AABBの座標を表す幾何ベクトルの型。
        typedef template_vector vector;

        /// AABBを構築する幾何形状の型。
        typedef template_shape shape;

        /** @brief 任意の幾何形状からAABBを構築する。

            ただし、ここではインターフェイスの定義しか行わない。
            テンプレート特殊化した geometric_shape_aabb で実装を行うこと。

            @param[in] in_shape AABBを構築する幾何形状。
            @return 幾何形状のAABB。
         */
        static psyq::geometric_aabb<template_vector> make(
            template_shape const& in_shape);
    };

    //-------------------------------------------------------------------------
    /** @brief AABBを構築するためのクラス。
        @tparam template_vector @copydoc psyq::geometric_shape_aabb::vector
     */
    template<typename template_vector>
    struct geometric_shape_aabb<
        template_vector, psyq::geometric_aabb<template_vector>>
    {
        /// @copydoc geometric_shape_aabb::vector
        typedef template_vector vector;

        /// AABBの型。
        typedef psyq::geometric_aabb<template_vector> shape;

        static psyq::geometric_aabb<template_vector> make(
            psyq::geometric_aabb<template_vector> const& in_aabb)
        {
            return in_aabb;
        }

        /** @brief 2つの点を含む最小のAABBを構築する。
            @parma[in] in_point_a 点Aの座標。
            @parma[in] in_point_b 点Bの座標。
            @return 点Aと点Bを包むAABB。
         */
        static psyq::geometric_aabb<template_vector> make(
            template_vector const& in_point_a,
            template_vector const& in_point_b)
        {
            auto const local_x(
                std::minmax(
                    in_point_a.operator[](0), in_point_b.operator[](0)));
            auto const local_y(
                std::minmax(
                    in_point_a.operator[](1), in_point_b.operator[](1)));
            auto const local_z(
                std::minmax(
                    in_point_a.operator[](2), in_point_b.operator[](2)));
            return psyq::geometric_aabb<template_vector>(
                template_vector(
                    local_x.first, local_y.first, local_z.first),
                template_vector(
                    local_x.second, local_y.second, local_z.second));
        }
    };

    //-------------------------------------------------------------------------
    /** @brief 球からAABBを構築するためのクラス。
        @tparam template_vector @copydoc psyq::geometric_shape_aabb::vector
     */
    template<typename template_vector>
    struct geometric_shape_aabb<
        template_vector, psyq::geometric_sphere<template_vector>>
    {
        /// @copydoc geometric_shape_aabb::vector
        typedef template_vector vector;

        /// AABBを構築する球の型。
        typedef psyq::geometric_sphere<template_vector> shape;

        /** @brief 球のAABBを構築する。
            @param[in] in_sphere AABBを構築する球。
            @return 球のAABB。
         */
        static psyq::geometric_aabb<template_vector> make(
            psyq::geometric_sphere<template_vector> const& in_sphere)
        {
            template_vector const local_extent(
                in_sphere.get_radius(),
                in_sphere.get_radius(),
                in_sphere.get_radius());
            return psyq::geometric_aabb<template_vector>(
                in_sphere.get_center() - local_extent,
                in_sphere.get_center() + local_extent);
        }
    };

    //-------------------------------------------------------------------------
    /** @brief 線分からAABBを構築するためのクラス。
        @tparam template_vector @copydoc psyq::geometric_shape_aabb::vector
     */
    template<typename template_vector>
    struct geometric_shape_aabb<
        template_vector, psyq::geometric_segment<template_vector>>
    { 
        /// @copydoc geometric_shape_aabb::vector
       typedef template_vector vector;

        /// AABBを構築する線分の型。
        typedef psyq::geometric_segment<template_vector> shape;

        /** @brief 線分のAABBを構築する。
            @param[in] in_sphere AABBを構築する線分。
            @return 線分のAABB。
         */
        static psyq::geometric_aabb<template_vector> make(
            psyq::geometric_segment<template_vector> const& in_segment)
        {
            auto const local_end(this->get_origin() + this->get_direction());
            typedef typename
                psyq::geometric_vector_element<template_vector>::type
                    element_type;

            // X座標の範囲を決定する。
            auto const local_direction_x(
                in_segment.get_direction().operator[](0));
            auto const local_origin_x(in_segment.get_origin().operator[](0));
            auto const local_end_x(local_end.operator[](0));
            element_type local_min_x;
            element_type local_max_x;
            if (local_direction_x < 0)
            {
                local_min_x = local_end_x;
                local_max_x = local_origin_x;
            }
            else
            {
                local_min_x = local_origin_x;
                local_max_x = local_end_x;
            }

            // Y座標の範囲を決定する。
            auto const local_direction_y(
                in_segment.get_direction().operator[](1));
            auto const local_origin_y(in_segment.get_origin().operator[](1));
            auto const local_end_y(local_end.operator[](1));
            element_type local_min_y;
            element_type local_max_y;
            if (local_direction_y < 0)
            {
                local_min_y = local_end_y;
                local_max_y = local_origin_y;
            }
            else
            {
                local_min_y = local_origin_y;
                local_max_y = local_end_y;
            }

            // Z座標の範囲を決定する。
            auto const local_direction_z(
                in_segment.get_direction().operator[](2));
            auto const local_origin_z(in_segment.get_origin().operator[](2));
            auto const local_end_z(local_end.operator[](2));
            element_type local_min_z;
            element_type local_max_z;
            if (local_direction_z < 0)
            {
                local_min_z = local_end_z;
                local_max_z = local_origin_z;
            }
            else
            {
                local_min_z = local_origin_z;
                local_max_z = local_end_z;
            }

            return psyq::geometric_aabb<template_vector>(
                template_vector(local_min_x, local_min_y, local_min_z),
                template_vector(local_max_x, local_max_y, local_max_z));
        }
    };

    //-------------------------------------------------------------------------
    /** @brief 半線分からAABBを構築するためのクラス。
        @tparam template_vector @copydoc psyq::geometric_shape_aabb::vector
     */
    template<typename template_vector>
    struct geometric_shape_aabb<
        template_vector, psyq::geometric_ray<template_vector>>
    {
        /// @copydoc geometric_shape_aabb::vector
        typedef template_vector vector;

        /// AABBを構築する半線分の型。
        typedef psyq::geometric_ray<template_vector> shape;

        /** @brief 半線分のAABBを構築する。
            @param[in] in_sphere AABBを構築する半線分。
            @return 半線分のAABB。
         */
        static psyq::geometric_aabb<template_vector> make(
            psyq::geometric_ray<template_vector> const& in_ray)
        {
            typedef typename
                psyq::geometric_vector_element<template_vector>::type
                    element_type;

            // X座標の範囲を決定する。
            auto const local_direction_x(in_ray.get_direction().operator[](0));
            auto const local_origin_x(in_ray.get_origin().operator[](0));
            element_type local_min_x;
            element_type local_max_x;
            if (local_direction_x < 0)
            {
                local_min_x = -(std::numeric_limits<element_type>::max)();
                local_max_x = local_origin_x;
            }
            else if (0 < local_direction_x)
            {
                local_min_x = local_origin_x;
                local_max_x = (std::numeric_limits<element_type>::max)();
            }
            else
            {
                local_min_x = local_origin_x;
                local_max_x = local_origin_x;
            }

            // Y座標の範囲を決定する。
            auto const local_direction_y(in_ray.get_direction().operator[](1));
            auto const local_origin_y(in_ray.get_origin().operator[](1));
            element_type local_min_y;
            element_type local_max_y;
            if (local_direction_y < 0)
            {
                local_min_y = -(std::numeric_limits<element_type>::max)();
                local_max_y = local_origin_y;
            }
            else if (0 < local_direction_y)
            {
                local_min_y = local_origin_y;
                local_max_y = (std::numeric_limits<element_type>::max)();
            }
            else
            {
                local_min_y = local_origin_y;
                local_max_y = local_origin_y;
            }

            // Z座標の範囲を決定する。
            auto const local_direction_z(in_ray.get_direction().operator[](2));
            auto const local_origin_z(in_ray.get_origin().operator[](2));
            element_type local_min_z;
            element_type local_max_z;
            if (local_direction_z < 0)
            {
                local_min_z = -(std::numeric_limits<element_type>::max)();
                local_max_z = local_origin_z;
            }
            else if (0 < local_direction_z)
            {
                local_min_z = local_origin_z;
                local_max_z = (std::numeric_limits<element_type>::max)();
            }
            else
            {
                local_min_z = local_origin_z;
                local_max_z = local_origin_z;
            }

            return psyq::geometric_aabb<template_vector>(
                template_vector(local_min_x, local_min_y, local_min_z),
                template_vector(local_max_x, local_max_y, local_max_z));
        }
    };

    //-------------------------------------------------------------------------
    /** @brief 直方体からAABBを構築するためのクラス。
        @tparam template_vector @copydoc psyq::geometric_shape_aabb::vector
     */
    template<typename template_vector>
    struct geometric_shape_aabb<
        template_vector, psyq::geometric_cuboid<template_vector>>
    {
        /// @copydoc geometric_shape_aabb::vector
        typedef template_vector vector;

        /// AABBを構築する直方体の型。
        typedef psyq::geometric_cuboid<template_vector> shape;

        /** @brief 直方体のAABBを構築する。
            @param[in] in_sphere AABBを構築する直方体。
            @return 直方体のAABB。
         */
        static psyq::geometric_aabb<template_vector> make(
            psyq::geometric_cuboid<template_vector> const& in_cuboid)
        {
            auto const local_abs_vector(
                [](template_vector const& in_vector)->template_vector
                {
                    auto const local_x(in_vector.operator[](0));
                    auto const local_y(in_vector.operator[](1));
                    auto const local_z(in_vector.operator[](2));
                    return template_vector(
                        local_x < 0? -local_x: local_x,
                        local_y < 0? -local_y: local_y,
                        local_z < 0? -local_z: local_z);
                });
            auto const local_half_diagonal(
                local_abs_vector(
                    in_cuboid.get_axis_x()
                    * in_cuboid.get_extent().operator[](0))
                + local_abs_vector(
                    in_cuboid.get_axis_y()
                    * in_cuboid.get_extent().operator[](1))
                + local_abs_vector(
                    in_cuboid.get_axis_z()
                    * in_cuboid.get_extent().operator[](2)));
            return psyq::geometric_aabb<template_vector>(
                in_cuboid.get_center() - local_half_diagonal,
                in_cuboid.get_center() + local_half_diagonal);
        }
    };
}

#endif // !defined(PSYQ_GEOMETIC_UTILITY_HPP_)
