#ifndef PSYQ_MOSP_NODE_HPP_
#define PSYQ_MOSP_NODE_HPP_
//#include "mosp_tree.hpp"

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief  mosp_tree 空間分割木に取りつける、衝突判定オブジェクトの基底型。
    @tparam template_space @copydoc psyq::mosp_tree::space
 */
template<typename template_space>
class psyq::mosp_node
{
    /// thisが指す値の型。
    private: typedef mosp_node this_type;

    //-------------------------------------------------------------------------
    /// mosp_node で使うモートン空間の型。
    public: typedef template_space space;
    /// 空間分割木に取りつける mosp_handle 。
    public: typedef psyq::mosp_handle<this_type*, typename this_type::space::order>
        handle;

    /// @cond
    public: template<typename template_shape> class concrete;
    /// @endcond
    /// mosp_tree に取付可能な、球の衝突判定オブジェクト。
    public: typedef concrete<psyq::geometric_sphere<typename this_type::space::coordinates::vector>>
        sphere;
    /// mosp_tree に取付可能な、線分の衝突判定オブジェクト。
    public: typedef concrete<psyq::geometric_segment<typename this_type::space::coordinates::vector>>
        segment;
    /// mosp_tree に取付可能な、半線分の衝突判定オブジェクト。
    public: typedef concrete<psyq::geometric_ray<typename this_type::space::coordinates::vector>>
        ray;
    /// mosp_tree に取付可能な、直方体の衝突判定オブジェクト。
    public: typedef concrete<psyq::geometric_cuboid<typename this_type::space::coordinates::vector>>
        cuboid;

    //-------------------------------------------------------------------------
    protected: mosp_node():
        handle_(this),
        aabb_(
            typename this_type::space::coordinates::aabb(
                psyq::geometric_vector<typename this_type::space::coordinates::vector>::make(0),
                psyq::geometric_vector<typename this_type::space::coordinates::vector>::make(0)))
    {}

    /// *thisを mosp_tree から取り外す。
    public: virtual ~mosp_node() {}

    //-------------------------------------------------------------------------
    /** @brief *thisを mosp_tree 空間分割木に取りつける。

        現在取りつけられている mosp_tree から*thisを切り離し、
        新しい mosp_tree に*thisを取りつける。

        @param[in,out] io_tree *thisを取りつける mosp_tree 。
        @sa detach_tree() is_attached()
     */
    public: template<typename template_tree>
    void attach_tree(template_tree& io_tree)
    {
        // AABBを更新してから取りつける。
        if (!this->handle_.is_attached())
        {
            this->update_aabb();
        }
        this->handle_.attach_tree(io_tree, this->get_aabb());
    }

    /// @copydoc this_type::handle::detach_tree()
    protected: void detach_tree()
    {
        this->handle_.detach_tree();
    }

    /// @copydoc this_type::handle::is_attached()
    protected: bool is_attached() const
    {
        return this->handle_.is_attached();
    }

    /** @brief thisが持つAABBを取得する。
     */
    public: typename this_type::space::coordinates::aabb const& get_aabb() const
    {
        return this->aabb_;
    }

    /** @brief thisが持つAABBを更新する。
     */
    protected: virtual void update_aabb() = 0;

    //-------------------------------------------------------------------------
    /// 衝突判定オブジェクトに対応する衝突判定ハンドル。
    private: typename this_type::handle handle_;
    /// 衝突判定オブジェクトの絶対座標系AABB。
    protected: typename this_type::space::coordinates::aabb aabb_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief  mosp_tree 空間分割木に取りつける、衝突判定オブジェクト。
    @tparam template_space @copydoc psyq::mosp_tree::space
    @tparam template_shape @copydoc psyq::mosp_node_shape::shape
 */
template<typename template_space>
template<typename template_shape>
class psyq::mosp_node<template_space>::concrete:
    public psyq::mosp_node<template_space>
{
    /// thisが指す値の型。
    private: typedef concrete this_type;
    /// this_type の基底型。
    public: typedef psyq::mosp_node<template_space> base_type;

    /// 衝突判定オブジェクトの幾何形状。
    public: typedef template_shape shape;

    //-------------------------------------------------------------------------
    /** @brief 衝突判定に使う形状を構築する。
        @param[in] in_shape 衝突判定に使う形状の初期値。
     */
    public: explicit concrete(typename this_type::shape in_shape):
        shape_(std::move(in_shape))
    {}

    //-------------------------------------------------------------------------
    /** @brief 衝突判定に使う形状を取得する。
        @return 衝突判定に使う形状。
     */
    public: typename this_type::shape const& get_shape() const
    {
        return this->shape_;
    }

    /** @brief *thisを mosp_tree から取り外し、衝突判定に使う形状を取得する。

        衝突判定に使う幾何形状を更新したい場合は、
        この関数の戻り値が参照する幾何形状を書き換えた後、
        base_type::attach_tree() で mosp_tree に取りつける。

        @return 衝突判定に使う形状。
     */
    public: typename this_type::shape& get_mutable_shape()
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
            typename base_type::space::coordinates::vector,
            typename this_type::shape>
                ::make(this->get_shape());
    };

    //-------------------------------------------------------------------------
    /// 衝突判定オブジェクトの形状。
    protected: typename this_type::shape shape_;
};


//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
namespace psyq
{
    namespace test
    {
        inline void mosp_tree()
        {
            typedef psyq::mosp_tree<> psyq_mosp_tree;
            psyq_mosp_tree::node_map::allocator_type::arena::shared_ptr
                local_mosp_arena(
                    new psyq_mosp_tree::node_map::allocator_type::arena(16));
            psyq_mosp_tree local_mosp_tree(
                psyq_mosp_tree::aabb(
                    psyq_mosp_tree::vector(-65536, -65536, -65536),
                    psyq_mosp_tree::vector( 65536,  65536,  65536)),
                1024,
                psyq_mosp_tree::allocator_type(local_mosp_arena));
            psyq_mosp_tree::node::sphere local_mosp_sphere(
                psyq_mosp_tree::node::sphere::shape(
                    psyq_mosp_tree::vector(2, 3, 4), 1));
            local_mosp_sphere.attach_tree(local_mosp_tree);
            psyq_mosp_tree::node::ray local_mosp_ray(
                psyq_mosp_tree::node::ray::shape(
                    psyq_mosp_tree::vector(1, 2, 3),
                    psyq_mosp_tree::vector(4, 5, 6)));
            local_mosp_ray.attach_tree(local_mosp_tree);
            local_mosp_tree.detect_collision_batch(
                [](
                    psyq_mosp_tree::node* const in_node_0,
                    psyq_mosp_tree::node* const in_node_1)
                {
                    PSYQ_ASSERT(in_node_0 != nullptr);
                    PSYQ_ASSERT(in_node_1 != nullptr);
                    PSYQ_ASSERT(in_node_0 != in_node_1);

                    // AABBが衝突しているか判定する。
                    bool const local_aabb_collision(
                        psyq_mosp_tree::aabb::detect_collision(
                            in_node_0->get_aabb(), in_node_1->get_aabb()));
                    if (local_aabb_collision)
                    {
                    }
                });
            local_mosp_ray.attach_tree(local_mosp_tree);
        }
    }
}

#endif // !defined(PSYQ_MOSP_NODE_HPP_)
