#ifndef PSYQ_GEOMETIC_AABB_HPP_
#define PSYQ_GEOMETIC_AABB_HPP_
//#include "psyq/geometric_shape.hpp"

namespace psyq
{
    /// @cond
    template<typename> class geometric_aabb;
    /// @endcond
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief axis-aligned-bounding-box. 座標軸に平行な直方体。
    @tparam template_vector @copydoc geometric_aabb::vector
 */
template<typename template_vector>
class psyq::geometric_aabb
{
    /// *thisの型。
    private: typedef psyq::geometric_aabb<template_vector> self;

    /// @copydoc psyq::geometric_vector::type
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
        PSYQ_ASSERT(
            psyq::geometric_vector_element(in_min, 0)
            <= psyq::geometric_vector_element(in_max, 0));
        PSYQ_ASSERT(
            psyq::geometric_vector_element(in_min, 1)
            <= psyq::geometric_vector_element(in_max, 1));
        PSYQ_ASSERT(
            psyq::geometric_vector_element(in_min, 2)
            <= psyq::geometric_vector_element(in_max, 2));
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

    /** @brief 2つのAABBが衝突しているか判定する。
        @param[in] in_aabb0 AABB#0
        @param[in] in_aabb1 AABB#1
        @retval true  衝突している。
        @retval false 衝突してない。
     */
    public: static bool detect_collision(
        self const& in_aabb0,
        self const& in_aabb1)
    {
        auto const local_diff0(in_aabb0.get_min() - in_aabb1.get_max());
        auto const local_diff1(in_aabb1.get_min() - in_aabb0.get_max());
#if 1
        return psyq::geometric_vector_element(local_diff0, 0) < 0
            && psyq::geometric_vector_element(local_diff0, 1) < 0
            && psyq::geometric_vector_element(local_diff0, 2) < 0
            && psyq::geometric_vector_element(local_diff1, 0) < 0
            && psyq::geometric_vector_element(local_diff1, 1) < 0
            && psyq::geometric_vector_element(local_diff1, 2) < 0;
#else
        // 浮動小数点の符号ビットがすべてマイナスか判定する。
        return (
            psyq::internal::get_float_bit_value(
                psyq::geometric_vector_element(local_diff0, 0))
            && psyq::internal::get_float_bit_value(
                psyq::geometric_vector_element(local_diff0, 1))
            && psyq::internal::get_float_bit_value(
                psyq::geometric_vector_element(local_diff0, 2))
            && psyq::internal::get_float_bit_value(
                psyq::geometric_vector_element(local_diff1, 0))
            && psyq::internal::get_float_bit_value(
                psyq::geometric_vector_element(local_diff1, 1))
            && psyq::internal::get_float_bit_value(
                psyq::geometric_vector_element(local_diff1, 2))) >> 31;
#endif
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
            @param[in] in_point_a 点Aの座標。
            @param[in] in_point_b 点Bの座標。
            @return 点Aと点Bを包むAABB。
         */
        static psyq::geometric_aabb<template_vector> make(
            template_vector const& in_point_a,
            template_vector const& in_point_b)
        {
            auto const local_x(
                std::minmax(
                    psyq::geometric_vector_element(in_point_a, 0),
                    psyq::geometric_vector_element(in_point_b, 0)));
            auto const local_y(
                std::minmax(
                    psyq::geometric_vector_element(in_point_a, 1),
                    psyq::geometric_vector_element(in_point_b, 1)));
            auto const local_z(
                std::minmax(
                    psyq::geometric_vector_element(in_point_a, 2),
                    psyq::geometric_vector_element(in_point_b, 2)));
            return psyq::geometric_aabb<template_vector>(
                psyq::geometric_vector<template_vector>::make(
                    local_x.first, local_y.first, local_z.first),
                psyq::geometric_vector<template_vector>::make(
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
            auto const local_extent(
                psyq::geometric_vector<template_vector>::make(
                    in_sphere.get_radius()));
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
            @param[in] in_segment AABBを構築する線分。
            @return 線分のAABB。
         */
        static psyq::geometric_aabb<template_vector> make(
            psyq::geometric_segment<template_vector> const& in_segment)
        {
            auto const local_end(this->get_origin() + this->get_direction());
            typedef typename psyq::geometric_vector<template_vector>::element
                vector_element;

            // X座標の範囲を決定する。
            auto const local_direction_x(
                psyq::geometric_vector_element(in_segment.get_direction(), 0));
            auto const local_origin_x(
                psyq::geometric_vector_element(in_segment.get_origin(), 0));
            auto const local_end_x(
                psyq::geometric_vector_element(local_end, 0));
            vector_element local_min_x;
            vector_element local_max_x;
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
                psyq::geometric_vector_element(in_segment.get_direction(), 1));
            auto const local_origin_y(
                psyq::geometric_vector_element(in_segment.get_origin(), 1));
            auto const local_end_y(
                psyq::geometric_vector_element(local_end, 1));
            vector_element local_min_y;
            vector_element local_max_y;
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
                psyq::geometric_vector_element(in_segment.get_direction(), 2));
            auto const local_origin_z(
                psyq::geometric_vector_element(in_segment.get_origin(), 2));
            auto const local_end_z(
                psyq::geometric_vector_element(local_end, 2));
            vector_element local_min_z;
            vector_element local_max_z;
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
                psyq::geometric_vector<template_vector>::make(
                    local_min_x, local_min_y, local_min_z),
                psyq::geometric_vector<template_vector>::make(
                    local_max_x, local_max_y, local_max_z));
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
            @param[in] in_ray AABBを構築する半線分。
            @return 半線分のAABB。
         */
        static psyq::geometric_aabb<template_vector> make(
            psyq::geometric_ray<template_vector> const& in_ray)
        {
            typedef typename psyq::geometric_vector<template_vector>::element
                vector_element;

            // X座標の範囲を決定する。
            auto const local_direction_x(
                psyq::geometric_vector_element(in_ray.get_direction(), 0));
            auto const local_origin_x(
                psyq::geometric_vector_element(in_ray.get_origin(), 0));
            vector_element local_min_x;
            vector_element local_max_x;
            if (local_direction_x < 0)
            {
                local_min_x = -(std::numeric_limits<vector_element>::max)();
                local_max_x = local_origin_x;
            }
            else if (0 < local_direction_x)
            {
                local_min_x = local_origin_x;
                local_max_x = (std::numeric_limits<vector_element>::max)();
            }
            else
            {
                local_min_x = local_origin_x;
                local_max_x = local_origin_x;
            }

            // Y座標の範囲を決定する。
            auto const local_direction_y(
                psyq::geometric_vector_element(in_ray.get_direction(), 1));
            auto const local_origin_y(
                psyq::geometric_vector_element(in_ray.get_origin(), 1));
            vector_element local_min_y;
            vector_element local_max_y;
            if (local_direction_y < 0)
            {
                local_min_y = -(std::numeric_limits<vector_element>::max)();
                local_max_y = local_origin_y;
            }
            else if (0 < local_direction_y)
            {
                local_min_y = local_origin_y;
                local_max_y = (std::numeric_limits<vector_element>::max)();
            }
            else
            {
                local_min_y = local_origin_y;
                local_max_y = local_origin_y;
            }

            // Z座標の範囲を決定する。
            auto const local_direction_z(
                psyq::geometric_vector_element(in_ray.get_direction(), 2));
            auto const local_origin_z(
                psyq::geometric_vector_element(in_ray.get_origin(), 2));
            vector_element local_min_z;
            vector_element local_max_z;
            if (local_direction_z < 0)
            {
                local_min_z = -(std::numeric_limits<vector_element>::max)();
                local_max_z = local_origin_z;
            }
            else if (0 < local_direction_z)
            {
                local_min_z = local_origin_z;
                local_max_z = (std::numeric_limits<vector_element>::max)();
            }
            else
            {
                local_min_z = local_origin_z;
                local_max_z = local_origin_z;
            }

            return psyq::geometric_aabb<template_vector>(
                psyq::geometric_vector<template_vector>::make(
                    local_min_x, local_min_y, local_min_z),
                psyq::geometric_vector<template_vector>::make(
                    local_max_x, local_max_y, local_max_z));
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
            @param[in] in_cuboid AABBを構築する直方体。
            @return 直方体のAABB。
         */
        static psyq::geometric_aabb<template_vector> make(
            psyq::geometric_cuboid<template_vector> const& in_cuboid)
        {
            auto const local_abs_vector(
                [](template_vector const& in_vector)->template_vector
                {
                    auto const local_x(
                        psyq::geometric_vector_element(in_vector, 0));
                    auto const local_y(
                        psyq::geometric_vector_element(in_vector, 1));
                    auto const local_z(
                        psyq::geometric_vector_element(in_vector, 2));
                    return psyq::geometric_vector<template_vector>::make(
                        local_x < 0? -local_x: local_x,
                        local_y < 0? -local_y: local_y,
                        local_z < 0? -local_z: local_z);
                });
            auto const local_half_diagonal(
                local_abs_vector(
                    in_cuboid.get_axis_x()
                    * psyq::geometric_vector_element(
                        in_cuboid.get_extent(), 0))
                + local_abs_vector(
                    in_cuboid.get_axis_y()
                    * psyq::geometric_vector_element(
                        in_cuboid.get_extent(), 1))
                + local_abs_vector(
                    in_cuboid.get_axis_z()
                    * psyq::geometric_vector_element(
                        in_cuboid.get_extent(), 2)));
            return psyq::geometric_aabb<template_vector>(
                in_cuboid.get_center() - local_half_diagonal,
                in_cuboid.get_center() + local_half_diagonal);
        }
    };
}

#endif // !defined(PSYQ_GEOMETIC_AABB_HPP_)
