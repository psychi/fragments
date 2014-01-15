#ifndef PSYQ_GEOMETIC_UTILITY_HPP_
#define PSYQ_GEOMETIC_UTILITY_HPP_

#ifndef PSYQ_NEARLY_EQUAL_EPSILON_MAG_DEFAULT
#define PSYQ_NEARLY_EQUAL_EPSILON_MAG_DEFAULT 1
#endif

namespace psyq
{
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
            decltype(template_vector(0, 0, 0).operator[](0))>::type
                type;
    };

    template<typename template_vector>
    typename geometric_vector_element<template_vector>::type
    square_geometric_vector_length(template_vector const& in_vector)
    {
        auto const local_x(in_vector.operator[](0));
        auto const local_y(in_vector.operator[](1));
        auto const local_z(in_vector.operator[](2));
        return local_x * local_x + local_y * local_y + local_z * local_z;
    }

    template<typename template_vector>
    typename geometric_vector_element<template_vector>::type
    calc_geometric_vector_length(template_vector const& in_vector)
    {
        return std::sqrt(psyq::square_geometric_vector_length(in_vector));
    }

    template<typename template_vector>
    bool is_normalized_geometric_vector(template_vector const& in_vector)
    {
        return psyq::is_nearly_equal(
            psyq::square_geometric_vector_length(in_vector),
            typename psyq::geometric_vector_element<template_vector>::type(1));
    }

    /// @cond
    template<typename> class geometric_sphere;
    template<typename> class geometric_segment;
    template<typename> class geometric_ray;
    template<typename> class geometric_aabb;
    template<typename> class geometric_cuboid;
    /// @endcond
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// 球。
template<typename template_vector>
class psyq::geometric_sphere
{
    private: typedef psyq::geometric_sphere<template_vector> self;

    public: typedef template_vector vector;

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

    public: template_vector const& get_center() const
    {
        return this->center_;
    }

    public: void set_center(template_vector const& in_center)
    {
        this->center_ = in_center;
    }

    public: typename psyq::geometric_vector_element<template_vector>::type
    get_radius() const
    {
        return this->radius_;
    }

    public: void set_radius(
        typename psyq::geometric_vector_element<template_vector>::type const
            in_radius)
    {
        PSYQ_ASSERT(0 <= in_radius);
        this->radius_ = in_radius;
    }

    /// 球の中心位置。
    private: template_vector center_;
    /// 球の半径。
    private: typename geometric_vector_element<template_vector>::type radius_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// 線分。
template<typename template_vector>
class psyq::geometric_segment
{
    private: typedef psyq::geometric_segment<template_vector> self;

    public: typedef template_vector vector;

    public: geometric_segment(
        template_vector const& in_origin,
        template_vector const& in_direction)
    :
        origin_(in_origin),
        direction_(in_direction)
    {}

    public: template_vector const& get_origin() const
    {
        return this->origin_;
    }

    public: void set_origin(template_vector const& in_origin)
    {
        this->origin_ = in_origin;
    }

    public: template_vector const& get_direction() const
    {
        return this->direction_;
    }

    public: void set_direction(template_vector const& in_direction)
    {
        this->direction_ = in_direction;
    }

    private: template_vector origin_;    ///< 線分の始点。
    private: template_vector direction_; ///< 線分の方向と大きさ。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// 半線分。
template<typename template_vector>
class psyq::geometric_ray: public psyq::geometric_segment<template_vector>
{
    private: typedef psyq::geometric_ray<template_vector> self;
    public: typedef psyq::geometric_segment<template_vector> super;

    public: typedef template_vector vector;

    public: geometric_ray(
        template_vector const& in_origin,
        template_vector const& in_direction)
    :
        super(in_origin, in_direction)
    {
        PSYQ_ASSERT(psyq::is_normalized_geometric_vector(in_direction));
    }

    public: void set_direction(template_vector const& in_direction)
    {
        this->direction_ = self::make_direction(in_direction);
    }

    public: static self make(
        template_vector const& in_origin,
        template_vector const& in_direction)
    {
        return self(in_origin, self::make_direction(in_direction));
    }

    private: static template_vector make_direction(
        template_vector const& in_direction)
    {
        auto const local_length(
            psyq::calc_geometric_vector_length(in_direction));
        if (local_length <= 0)
        {
            PSYQ_ASSERT(false);
            return template_vector(0, 0, 1);
        }
        return in_direction / local_length;
    }
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// axis-aligned-bounding-box.
template<typename template_vector>
class psyq::geometric_aabb
{
    private: typedef psyq::geometric_aabb<template_vector> self;

    public: typedef template_vector vector;

    /** @param[in] in_min AABBの最小座標。
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

    public: template_vector const& get_min() const
    {
        return this->min_;
    }

    public: template_vector const& get_max() const
    {
        return this->max_;
    }

    /** @brief 2つの点を含む最小のAABBを作る。
        @parma[in] in_point_a 点Aの座標。
        @parma[in] in_point_b 点Bの座標。
        @return 点Aと点Bを包むAABB。
     */
    public: static self make(
        template_vector const& in_point_a,
        template_vector const& in_point_b)
    {
        auto const local_x(
            std::minmax(in_point_a.operator[](0), in_point_b.operator[](0)));
        auto const local_y(
            std::minmax(in_point_a.operator[](1), in_point_b.operator[](1)));
        auto const local_z(
            std::minmax(in_point_a.operator[](2), in_point_b.operator[](2)));
        return self(
            template_vector(
                local_x.first, local_y.first, local_z.first),
            template_vector(
                local_x.second, local_y.second, local_z.second));
    }

    private: template_vector min_; ///< AABBの最小座標。
    private: template_vector max_; ///< AABBの最大座標。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/// 直方体。
template<typename template_vector>
class psyq::geometric_cuboid
{
    private: typedef psyq::geometric_cuboid<template_vector> self;

    public: typedef template_vector vector;

    public: explicit geometric_cuboid(
        psyq::geometric_aabb<template_vector> const& in_aabb)
    :
        center_((in_aabb.get_max() + in_aabb.get_min()) / 2),
        axis_x_(1, 0, 0),
        axis_y_(0, 1, 0),
        axis_z_(0, 0, 1),
        extent_((in_aabb.get_max() - in_aabb.get_min()) / 2)
    {}

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

    public: template_vector const& get_center() const
    {
        return this->center_;
    }

    public: template_vector const& get_axis_x() const
    {
        return this->axis_x_;
    }

    public: template_vector const& get_axis_y() const
    {
        return this->axis_y_;
    }

    public: template_vector const& get_axis_z() const
    {
        return this->axis_z_;
    }

    public: template_vector const& get_extent() const
    {
        return this->extent_;
    }

    private: template_vector center_;  ///< 直方体の中心位置。
    private: template_vector axis_x_;  ///< 直方体の向き。
    private: template_vector axis_y_;  ///< 直方体の向き。
    private: template_vector axis_z_;  ///< 直方体の向き。
    private: template_vector extent_;  ///< 直方体の大きさの1/2。
};

#endif // !defined(PSYQ_GEOMETIC_UTILITY_HPP_)
