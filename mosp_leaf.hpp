#ifndef PSYQ_MOSP_LEAF_HPP_
#define PSYQ_MOSP_LEAF_HPP_
//#include "mosp_tree.hpp"

namespace psyq
{
    /// @cond
    template<typename = PSYQ_MOSP_SPACE_DEFAULT> class mosp_leaf;
    template<typename, typename> class mosp_leaf_shape;
    /// @endcond
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief  mosp_tree 空間分割木に取りつける、衝突判定オブジェクトの基底型。
    @tparam template_space @copydoc psyq::mosp_leaf::space
 */
template<typename template_space>
class psyq::mosp_leaf
{
    /// thisが指す値の型。
    private: typedef mosp_leaf this_type;

    /// @copydoc psyq::mosp_tree::space
    public: typedef template_space space;
    /// mosp_leaf で使うAABBの型。
    public: typedef typename this_type::space::coordinates::aabb aabb;
    /// mosp_leaf で使う幾何ベクトルの型。
    public: typedef typename this_type::space::coordinates::vector vector;
    /// mosp_leaf を取りつける、 mosp_tree 空間分割木。
    public: typedef psyq::mosp_tree<this_type*, template_space> tree;

    //-------------------------------------------------------------------------
    protected: mosp_leaf():
        handle_(this),
        aabb_(
            typename this_type::aabb(
                psyq::geometric_vector<typename this_type::vector>::make(0),
                psyq::geometric_vector<typename this_type::vector>::make(0)))
    {}

    /// *thisを mosp_tree から取り外す。
    public: virtual ~mosp_leaf() {}

    //-------------------------------------------------------------------------
    /** @brief *thisを mosp_tree 空間分割木に取りつける。

        現在取りつけられている mosp_tree から*thisを切り離し、
        新しい mosp_tree に*thisを取りつける。

        @param[in,out] io_tree *thisを取りつける mosp_tree 。
        @sa detach_tree() is_attached()
     */
    public: void attach_tree(typename this_type::tree& io_tree)
    {
        // AABBを更新してから取りつける。
        if (!this->handle_.is_attached())
        {
            this->update_aabb();
        }
        this->handle_.attach_tree(io_tree, this->get_aabb());
    }

    /// @copydoc mosp_handle::detach_tree()
    protected: void detach_tree()
    {
        this->handle_.detach_tree();
    }

    /// @copydoc mosp_handle::is_attached()
    protected: bool is_attached() const
    {
        return this->handle_.is_attached();
    }

    /** @brief thisが持つAABBを取得する。
     */
    public: typename this_type::aabb const& get_aabb() const
    {
        return this->aabb_;
    }

    /** @brief thisが持つAABBを更新する。
     */
    protected: virtual void update_aabb() = 0;

    public: static void detect_collision(
        this_type const* const in_leaf0,
        this_type const* const in_leaf1)
    {
        PSYQ_ASSERT(in_leaf0 != nullptr);
        PSYQ_ASSERT(in_leaf1 != nullptr);
        PSYQ_ASSERT(in_leaf0 != in_leaf1);

        // AABBが衝突しているか判定する。
        const auto local_aabb_collision(
            this_type::aabb::detect_collision(
                in_leaf0->get_aabb(), in_leaf1->get_aabb()));
        if (!local_aabb_collision)
        {
            return;
        }
    }

    //-------------------------------------------------------------------------
    /// 衝突判定オブジェクトに対応する衝突判定ハンドル。
    private: typename this_type::tree::handle handle_;
    /// 衝突判定オブジェクトの絶対座標系AABB。
    protected: typename this_type::aabb aabb_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief  mosp_tree 空間分割木に取りつける、衝突判定オブジェクト。
    @tparam template_space @copydoc psyq::mosp_leaf::space
    @tparam template_shape @copydoc psyq::mosp_leaf_shape::shape
 */
template<typename template_space, typename template_shape>
class psyq::mosp_leaf_shape: public psyq::mosp_leaf<template_space>
{
    /// thisが指す値の型。
    private: typedef mosp_leaf_shape this_type;
    /// this_type の基底型。
    public: typedef psyq::mosp_leaf<template_space> base_type;

    /// 衝突判定オブジェクトの幾何形状。
    public: typedef template_shape shape;

    //-------------------------------------------------------------------------
    /** @brief 衝突判定に使う形状を構築する。
        @param[in] in_shape 衝突判定に使う形状の初期値。
     */
    public: explicit mosp_leaf_shape(template_shape const& in_shape):
        shape_(in_shape)
    {}

    //-------------------------------------------------------------------------
    /** @brief 衝突判定に使う形状を取得する。
        @return 衝突判定に使う形状。
     */
    public: template_shape const& get_shape() const
    {
        return this->shape_;
    }

    /** @brief *thisを mosp_tree から取り外し、衝突判定に使う形状を取得する。

        衝突判定に使う幾何形状を更新したい場合は、
        この関数の戻り値が参照する幾何形状を書き換えた後、
        base_type::attach_tree() で mosp_tree に取りつける。

        @return 衝突判定に使う形状。
     */
    public: template_shape& get_mutable_shape()
    {
        if (this->is_attached())
        {
            this->detach_tree();
        }
        else
        {
            this->update_aabb();
        }
        return this->shape_;
    }

    //-------------------------------------------------------------------------
    protected: virtual void update_aabb() override
    {
        this->aabb_ = psyq::geometric_shape_aabb<typename base_type::vector, template_shape>
            ::make(this->get_shape());
    };

    //-------------------------------------------------------------------------
    protected: template_shape shape_; ///< 衝突判定オブジェクトの形状。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    /// mosp_tree に取付可能な、球の衝突判定オブジェクト。
    typedef psyq::mosp_leaf_shape<
        psyq::mosp_leaf<>::space,
        psyq::geometric_sphere<psyq::mosp_leaf<>::vector>>
            mosp_sphere_leaf;

    /// mosp_tree に取付可能な、線分の衝突判定オブジェクト。
    typedef psyq::mosp_leaf_shape<
        psyq::mosp_leaf<>::space,
        psyq::geometric_segment<psyq::mosp_leaf<>::vector>>
            mosp_segment_leaf;

    /// mosp_tree に取付可能な、半線分の衝突判定オブジェクト。
    typedef psyq::mosp_leaf_shape<
        psyq::mosp_leaf<>::space,
        psyq::geometric_ray<psyq::mosp_leaf<>::vector>>
            mosp_ray_leaf;

    /// mosp_tree に取付可能な、直方体の衝突判定オブジェクト。
    typedef psyq::mosp_leaf_shape<
        psyq::mosp_leaf<>::space,
        psyq::geometric_cuboid<psyq::mosp_leaf<>::vector>>
            mosp_cuboid_leaf;
}

#endif // !defined(PSYQ_MOSP_LEAF_HPP_)
