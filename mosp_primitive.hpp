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
    public: static typename self::scape::coordinates::aabb make_aabb(
        template_shape const& in_shape)
    {
        return in_shape.make_aabb();
    }

    //-------------------------------------------------------------------------
    protected: virtual void update_aabb() override
    {
        this->aabb_ = self::make_aabb(this->get_const_shape());
    };

    //-------------------------------------------------------------------------
    protected: template_shape shape_; ///< 衝突判定に使う形状。
};

#endif // !defined(PSYQ_MOSP_PRIMITIVE_HPP_)
