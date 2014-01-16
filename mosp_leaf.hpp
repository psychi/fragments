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
template<typename template_space>
class psyq::mosp_leaf
{
    private: typedef psyq::mosp_leaf<template_space> self;

    /// @copydoc psyq::mosp_tree::space
    public: typedef template_space space;
    /// mosp_leaf で使うAABBの型。
    public: typedef typename self::space::coordinates::aabb aabb;
    /// mosp_leaf で使う幾何ベクトルの型。
    public: typedef typename self::space::coordinates::vector vector;
    /// mosp_leaf を取りつける mosp_tree 空間分割木の型。
    public: typedef psyq::mosp_tree<self*, template_space> tree;

    //-------------------------------------------------------------------------
    protected: mosp_leaf():
        handle_(this),
        aabb_(
            typename self::aabb(
                typename self::vector(0, 0, 0),
                typename self::vector(0, 0, 0)))
    {}

    /// mosp_tree から取り外す。
    public: virtual ~mosp_leaf() {}

    //-------------------------------------------------------------------------
    /** @brief thisを mosp_tree に取りつける。
        @param[in,out] io_tree thisを取りつける mosp_tree 。
     */
    public: void attach_tree(typename self::tree& io_tree)
    {
        // AABBを更新してから取りつける。
        if (!this->handle_.is_attached())
        {
            this->update_aabb();
        }
        this->handle_.attach_tree(io_tree, this->get_aabb());
    }

    protected: void detach_tree()
    {
        this->handle_.detach_tree();
    }

    protected: bool is_attached() const
    {
        return this->handle_.is_attached();
    }

    /** @brief thisが持つAABBを取得する。
     */
    public: typename self::aabb const& get_aabb() const
    {
        return this->aabb_;
    }

    /** @brief thisが持つAABBを更新する。
     */
    protected: virtual void update_aabb() = 0;

    //-------------------------------------------------------------------------
    /// 衝突判定プリミティブに対応する衝突判定ハンドル。
    private: typename self::tree::handle handle_;
    /// 衝突判定プリミティブの絶対座標系AABB。
    protected: typename self::aabb aabb_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
template<typename template_space, typename template_shape>
class psyq::mosp_leaf_shape: public psyq::mosp_leaf<template_space>
{
    private: typedef psyq::mosp_leaf_shape<template_space, template_shape>
        self;
    public: typedef psyq::mosp_leaf<template_space> super;

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

    /** @brief thisを mosp_tree から取り外し、衝突判定に使う形状を取得する。

        衝突判定に使う幾何形状を更新したい場合は、
        この関数の戻り値の参照先の幾何形状を書き換えた後、
        super::attach_tree() で mosp_tree に取りつける。

        @return 衝突判定に使う形状への参照。
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
        this->aabb_ = psyq::geometric_shape_aabb<
            typename super::vector, template_shape>
                ::make(this->get_shape());
    };

    //-------------------------------------------------------------------------
    protected: template_shape shape_; ///< 衝突判定プリミティブの形状。
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    typedef psyq::mosp_leaf_shape<
        psyq::mosp_leaf<>::space,
        psyq::geometric_sphere<psyq::mosp_leaf<>::vector>>
            mosp_sphere_leaf;
    typedef psyq::mosp_leaf_shape<
        psyq::mosp_leaf<>::space,
        psyq::geometric_segment<psyq::mosp_leaf<>::vector>>
            mosp_segment_leaf;
    typedef psyq::mosp_leaf_shape<
        psyq::mosp_leaf<>::space,
        psyq::geometric_ray<psyq::mosp_leaf<>::vector>>
            mosp_ray_leaf;
    typedef psyq::mosp_leaf_shape<
        psyq::mosp_leaf<>::space,
        psyq::geometric_cuboid<psyq::mosp_leaf<>::vector>>
            mosp_cuboid_leaf;
}

#endif // !defined(PSYQ_MOSP_LEAF_HPP_)
