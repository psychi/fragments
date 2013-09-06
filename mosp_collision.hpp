/** @file
    @brief morton順序を用いた空間分割木による衝突判定。

    使い方は、 psyq::mosp_handle と psyq::mosp_tree
    の詳細説明を参照すること。

    以下のweb-pageを参考にして実装した。
    http://marupeke296.com/COL_2D_No8_QuadTree.html
 */
#ifndef PSYQ_MOSP_COLLISION_HPP_
#define PSYQ_MOSP_COLLISION_HPP_
#include <glm/glm.hpp> // OpenGL Mathematics
//#include "psyq/bit_algorithm.hpp"

namespace psyq
{
    /// @cond
    template<typename> class mosp_handle;
    template<typename> class mosp_node;
    template<typename> class mosp_coordinates_xyz;
    template<typename> class mosp_coordinates_xz;
    template<typename> class mosp_space;
    template<typename> class mosp_space_2d;
    template<typename> class mosp_space_3d;
    template<
        typename, typename = mosp_space_2d<mosp_coordinates_xz<glm::vec3>>>
            class mosp_tree;
    /// @endcond
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief morton順序による空間分割木を用いた衝突判定のhandle。

    使い方の概要。
    -# 衝突する物体の識別値を、 mosp_handle::object_ に代入しておく。
    -# mosp_handle::Attach() を呼び出し、
       衝突判定handleを空間分割木に取りつける。
    -# mosp_tree::detect_collision() を呼び出し、衝突を判定する。

    @tparam template_collision_object @copydoc mosp_handle::collision_object
 */
template<typename template_collision_object>
class psyq::mosp_handle
{
    /// *thisの型。
    private: typedef mosp_handle<template_collision_object> self;

    /// 衝突する物体の識別値の型。
    public: typedef template_collision_object collision_object;

    //-------------------------------------------------------------------------
    /** @brief 衝突判定handleを構築する。
        @param[in] in_object thisに対応する、衝突物体の識別値の初期値。
     */
    public: explicit mosp_handle(
        template_collision_object in_object)
    :
        object_(std::move(in_object)),
        node_(nullptr)
    {}

    /// copy-constructorは使用禁止。
    private: mosp_handle(self const&);

    /** @brief move-constructor。
        @param[in,out] io_source 移動元となるinstance。
     */
    public: mosp_handle(
        self&& io_source)
    :
        object_(std::move(io_source.object_)),
        node_(io_source.node_)
    {
        if (io_source.node_ != nullptr)
        {
            io_source.node_ = nullptr;
            this->node_->handle_ = this;
        }
    }

    /// @brief 空間分割木からthisを取り外す。
    public: ~mosp_handle()
    {
        this->detach_tree();
    }

    /// copy代入演算子は使用禁止。
    private: self& operator=(const self&);

    /** @brief move代入演算子。
        @param[in,out] io_source 移動元となるinstance。
     */
    public: self& operator=(self&& io_source)
    {
        if (this != &io_source)
        {
            this->~self();
            new(this) self(std::move(io_source));
        }
        return *this;
    }

    //-------------------------------------------------------------------------
    /** @brief 空間分割木にthisを取りつける。

        現在取りつけられている空間分割木とthisを切り離し、
        新しい空間分割木にthisを取りつける。

        @param[in,out] io_tree
            新たに取りつける、 mosp_tree template型の空間分割木。
        @param[in] in_min 衝突判定handleに対応する絶対座標系AABBの最小値。
        @param[in] in_max 衝突判定handleに対応する絶対座標系AABBの最大値。
        @sa detach_tree()
     */
    public: template<typename template_mosp_tree>
    bool attach_tree(
        template_mosp_tree&                               io_tree,
        typename template_mosp_tree::space::vector const& in_min,
        typename template_mosp_tree::space::vector const& in_max)
    {
        // 新しいnodeを用意する。
        auto const local_node(io_tree.make_node(in_min, in_max));
        if (local_node == nullptr)
        {
            return false;
        }
        local_node->handle_ = this;

        // 古いnodeから新しいnodeへ切り替える。
        if (this->node_ != nullptr)
        {
            PSYQ_ASSERT(this == this->node_->get_handle());
            this->node_->handle_ = nullptr;
        }
        this->node_ = local_node;
        return true;
    }

    /** @brief 空間分割木からthisを取り外す。

        現在取りつけられている空間分割木とthisを切り離す。

        @sa attach_tree()
     */
    public: void detach_tree()
    {
        if (this->node_ != nullptr)
        {
            PSYQ_ASSERT(this == this->node_->get_handle());
            this->node_->handle_ = nullptr;
            this->node_ = nullptr;
        }
    }

    public: mosp_node<template_collision_object> const* get_node() const
    {
        return this->node_;
    }

    //-------------------------------------------------------------------------
    /** @brief thisに対応する、衝突物体の識別値。

        この値を引数として、衝突判定callback関数が呼び出される。

        @sa mosp_tree::detect_collision()
     */
    public: template_collision_object object_;

    /// thisに対応する空間分割木のnode。
    private: mosp_node<template_collision_object>* node_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief このclassをuserが使うのは禁止。空間分割木のnodeの型。
    @tparam template_collision_object @copydoc mosp_handle::collision_object
 */
template<typename template_collision_object>
class psyq::mosp_node
{
    /// *thisの型。
    private: typedef mosp_node<template_collision_object> self;
    friend class mosp_handle<template_collision_object>;

    //-------------------------------------------------------------------------
    /// @brief 単独のnodeを構築する。
    public: mosp_node()
    :
        handle_(nullptr)
    {
        this->next_ = this;
    }

    /// copy-constructorは使用禁止。
    private: mosp_node(self const&);

    /// destructor
    public: ~mosp_node()
    {
        PSYQ_ASSERT(this->is_alone());
        auto const local_handle(this->handle_);
        if (local_handle != nullptr)
        {
            local_handle->detach_tree();
            PSYQ_ASSERT(this->handle_ == nullptr);
        }
    }

    /// copy代入演算子は使用禁止。
    private: self& operator=(self const&);

    /** @brief thisに対応する衝突判定handleを取得する。
        @retval !=nullptr thisに対応する衝突判定handleへのポインタ。
        @retval ==nullptr thisに対応する衝突判定handleが存在しない。
     */
    public: mosp_handle<template_collision_object> const* get_handle()
    const
    {
        return this->handle_;
    }

    /** @brief 単独のnodeか判定する。
     */
    public: bool is_alone() const
    {
        return this == &this->get_next();
    }

    /** @brief 次のnodeを取得する。
        @return 次のnode。
     */
    public: const self& get_next() const
    {
        return *this->next_;
    }

    /// @copydoc get_next
    public: self& get_next()
    {
        return *this->next_;
    }

    /** @brief 単独のnodeを、thisの次に挿入する。

        挿入するnodeは、単独である必要がある。
        io_insert_nodeが他のnodeと連結している場合は、挿入に失敗する。

        @param[in,out] io_insert_node 挿入するnode。
        @retval true  挿入に成功。
        @retval false 挿入に失敗したので、何も行わなかった。
        @sa remove_next is_alone
     */
    public: bool insert_next(
        self& io_insert_node)
    {
        if (!io_insert_node.is_alone())
        {
            // 単独の循環ノードでないと挿入できない。
            PSYQ_ASSERT(false);
            return false;
        }
        io_insert_node.next_ = &this->get_next();
        this->next_ = &io_insert_node;
        return true;
    }

    /** @brief thisの次にあるnodeを削除する。

        削除したnodeは、単独のnodeとなる。

        @return 削除したnode。
     */
    public: self& remove_next()
    {
        auto const local_next(&this->get_next());
        this->next_ = &local_next->get_next();
        local_next->next_ = local_next;
        return *local_next;
    }

    //-------------------------------------------------------------------------
    private: self* next_; ///< 次のnode。
    /// thisに対応する衝突判定handle。
    private: mosp_handle<template_collision_object>* handle_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief vectorのXYZ成分を用いるmorton座標。

    psyq::mosp_space のtemplate引数に使う。

    @tparam template_vector @copydoc psyq::mosp_coordinates_xyz::vector
 */
template<typename template_vector>
class psyq::mosp_coordinates_xyz
{
    /// *thisの型。
    private: typedef mosp_coordinates_xyz<template_vector> self;

    public: enum: unsigned
    {
        ELEMENT0_INDEX = 0, ///< morton座標の成分#0のindex番号。
        ELEMENT1_INDEX = 1, ///< morton座標の成分#1のindex番号。
        ELEMENT2_INDEX = 2, ///< morton座標の成分#2のindex番号。
    };

    /// morton座標を表すvectorの型。glm::vec3互換であること。
    public: typedef template_vector vector;

    /// morton座標を表すvectorの成分の型。
    public: typedef typename template_vector::value_type element;

    /** @brief 絶対座標系からmorton座標への変換scaleを算出する。
        @param[in] in_morton_size morton座標の最大値。
        @param[in] in_world_size  絶対座標系でのmorton空間の大きさ。
     */
    public: static typename self::element calc_scale(
        typename self::element const in_morton_size,
        typename self::element const in_world_size)
    {
        if (in_world_size < std::numeric_limits<self::element>::epsilon())
        {
            return 0;
        }
        return in_morton_size / in_world_size;
    }

    /** @brief 絶対座標系空間からmorton座標空間への変換scaleを算出する。
        @param[in] in_min   衝突判定を行う領域全体を包む絶対座標系AABBの最小値。
        @param[in] in_max   衝突判定を行う領域全体を包む絶対座標系AABBの最大値。
        @param[in] in_level 空間分割の最深level。
     */
    public: static typename self::vector calc_scale(
        typename self::vector const& in_min,
        typename self::vector const& in_max,
        unsigned const               in_level)
    {
        auto const local_size(in_max - in_min);
        auto const local_unit(
            static_cast<typename self::element>(1 << in_level));
        return self::vector(
            self::calc_scale(local_unit, local_size.operator[](0)),
            self::calc_scale(local_unit, local_size.operator[](1)),
            self::calc_scale(local_unit, local_size.operator[](2)));
    }
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief vectorのXZ成分を用いるmorton座標。

    psyq::mosp_space のtemplate引数に使う。

    @tparam template_vector @copydoc psyq::mosp_coordinates_xyz::vector
 */
template<typename template_vector>
class psyq::mosp_coordinates_xz:
    public psyq::mosp_coordinates_xyz<template_vector>
{
    /// *thisの型。
    private: typedef mosp_coordinates_xz<template_vector> self;
    /// *thisの上位型。
    public: typedef mosp_coordinates_xyz<template_vector> super;

    public: enum: unsigned
    {
        ELEMENT0_INDEX = 0, ///< morton座標の成分#0のindex番号。
        ELEMENT1_INDEX = 2, ///< morton座標の成分#1のindex番号。
    };
    private: enum: unsigned
    {
        ELEMENT2_INDEX = 2, ///< morton座標の成分#2は使用できない。
    };
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief psyq::mosp_tree のtemplate引数に使う、morton空間の基底型。
    @tparam template_coordinates @copydoc psyq::mosp_space::coordinates
 */
template<typename template_coordinates>
class psyq::mosp_space
{
    /// *thisの型。
    private: typedef mosp_space<template_coordinates> self;

    /** @brief 衝突判定に使うmorton座標の型。

        mosp_coordinates_xyz や mosp_coordinates_xz を使う。
     */
    public: typedef template_coordinates coordinates;

    /// 衝突判定に使うvectorの型。
    public: typedef typename template_coordinates::vector vector;

    /// morton順序の型。
    public: typedef std::uint32_t order;

    /** @brief 衝突判定を行う領域を設定する。
        @param[in] in_min   衝突判定を行う領域の絶対座標系最小値。
        @param[in] in_max   衝突判定を行う領域の絶対座標系最大値。
        @param[in] in_level 空間分割の最深レベル。
     */
    protected: mosp_space(
        typename self::vector const& in_min,
        typename self::vector const& in_max,
        unsigned const               in_level)
    :
        min_(in_min),
        max_(in_max),
        scale_(self::coordinates::calc_scale(in_min, in_max, in_level))
    {
        PSYQ_ASSERT(this->min_.operator[](0) <= this->max_.operator[](0));
        PSYQ_ASSERT(this->min_.operator[](1) <= this->max_.operator[](1));
        PSYQ_ASSERT(this->min_.operator[](2) <= this->max_.operator[](2));
    }

    protected: typename self::coordinates::element transform_element(
        typename self::vector const& in_vector,
        unsigned const               in_element_index)
    const
    {
        auto local_element(in_vector[in_element_index]);
        auto const local_min(this->min_[in_element_index]);
        if (local_element < local_min)
        {
            return 0;
        }
        auto const local_max(this->max_[in_element_index]);
        if (local_max < local_element)
        {
            local_element = local_max;
        }
        return (local_element - local_min) * this->scale_[in_element_index];
    }

    protected: static typename self::order clamp_axis_order(
        typename self::coordinates::element const in_element,
        typename self::order const                in_max)
    {
        if (in_element < 1)
        {
            return 0;
        }
        const auto local_order(static_cast<typename self::order>(in_element));
        if (local_order < in_max)
        {
            return local_order;
        }
        return in_max;
    }

    /// 衝突判定を行う領域の、絶対座標系での最小座標。
    private: typename self::vector min_;

    /// 衝突判定を行う領域の、絶対座標系での最大座標。
    private: typename self::vector max_;

    /// 最小となる分割空間の、絶対座標系での大きさの逆数。
    private: typename self::vector scale_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 2次元座標で衝突判定を行うmorton空間。

    psyq::mosp_tree のtemplate引数に使う。

    @tparam template_coordinates @copydoc mosp_space::coordinates
 */
template<typename template_coordinates = psyq::mosp_coordinates_xz>
class psyq::mosp_space_2d:
    public psyq::mosp_space<template_coordinates>
{
    /// *thisの型。
    private: typedef mosp_space_2d<template_coordinates> self;

    /// *thisの上位型。
    public: typedef mosp_space<template_coordinates> super;

    /// 衝突判定に使う座標の成分の数。
    public: static unsigned const DIMENSION = 2;

    /// @copydoc mosp_space::mosp_space
    public: mosp_space_2d(
        typename super::vector const& in_min,
        typename super::vector const& in_max,
        unsigned const                in_level)
    :
        super(in_min, in_max, in_level)
    {
        // pass
    }

    /** @brief 2次元座標上の点から、線形4分木のmorton順序を算出する。
        @param[in] in_point 2次元座標上の点。
        @param[in] in_max   morton座標の最大値。
        @return 2次元座標に対応するmorton順序。
     */
    public: typename super::order calc_order(
        typename super::vector const& in_point,
        typename super::order const   in_max)
    const
    {
        auto const local_element0(
            this->transform_element(
                in_point, super::coordinates::ELEMENT0_INDEX));
        auto const local_element1(
            this->transform_element(
                in_point, super::coordinates::ELEMENT1_INDEX));
        return
            self::separate_bits(local_element0, in_max) << 0 |
            self::separate_bits(local_element1, in_max) << 1;
    }

    /** @brief morton座標を、軸ごとのbitに分割する。
        @param[in] in_element morton座標の成分値。
        @param[in] in_max     morton座標の最大値。
     */
    private: static typename super::order separate_bits(
        typename super::coordinates::element const in_element,
        typename super::order const                in_max)
    {
        auto local_bits(super::clamp_axis_order(in_element, in_max));
        local_bits = (local_bits | (local_bits << 8)) & 0x00ff00ff;
        local_bits = (local_bits | (local_bits << 4)) & 0x0f0f0f0f;
        local_bits = (local_bits | (local_bits << 2)) & 0x33333333;
        local_bits = (local_bits | (local_bits << 1)) & 0x55555555;
        return local_bits;
    }
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 3次元座標で衝突判定を行うmoront空間。

    mosp_tree のtemplate引数に使う。

    @tparam template_coordinates @copydoc mosp_space::coordinates
 */
template<typename template_coordinates = psyq::mosp_coordinates_xyz>
class psyq::mosp_space_3d:
    public psyq::mosp_space<template_coordinates>
{
    /// *thisの型。
    private: typedef mosp_space_3d<template_coordinates> self;

    /// *thisの上位型。
    public: typedef mosp_space<template_coordinates> super;

    /// @copydoc mosp_space_2d::DIMENSION
    public: static unsigned const DIMENSION = 3;

    /// @copydoc mosp_space::mosp_space
    public: mosp_space_3d(
        typename super::vector const& in_min,
        typename super::vector const& in_max,
        unsigned const                in_level)
    :
        super(in_min, in_max, in_level)
    {
        // pass
    }

    /** @brief 3次元座標上の点から、線形8分木のmorton順序を算出する。
        @param[in] in_point 3次元座標上の点。
        @param[in] in_max   morton順序の最大値。
        @return 3次元座標に対応するmorton順序。
     */
    public: typename super::order calc_order(
        typename super::vector const& in_point,
        typename self::order const    in_max)
    const
    {
        auto const local_element0(
            this->transform_element(
                in_point, super::coordinates::ELEMENT0_INDEX));
        auto const local_element1(
            this->transform_element(
                in_point, super::coordinates::ELEMENT1_INDEX));
        auto const local_element2(
            this->transform_element(
                in_point, super::coordinates::ELEMENT2_INDEX));
        return
            self::separate_bits(local_element0, in_max) << 0 |
            self::separate_bits(local_element1, in_max) << 1 |
            self::separate_bits(local_element2, in_max) << 2;
    }

    /// @copydoc mosp_space_2d::separate_bits
    private: static typename super::order separate_bits(
        typename super::coordinates::element const in_element,
        typename self::order const                 in_max)
    {
        auto local_bits(super::clamp_axis_order(in_element, in_max));
        local_bits = (local_bits | (local_bits << 8)) & 0x0000f00f;
        local_bits = (local_bits | (local_bits << 4)) & 0x000c30c3;
        local_bits = (local_bits | (local_bits << 2)) & 0x00249249;
        return local_bits;
    }
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief morton順序による空間分割木。

    使い方の概要。
    -# psyq::mosp_tree::mosp_tree() を呼び出し、
       衝突判定を行う領域を空間分割木に設定する。
    -# psyq::mosp_handle::attach_tree() を呼び出し、
       衝突判定handleを空間分割木に取りつける。
    -# psyq::mosp_tree::detect_collision() を呼び出し、衝突を判定する。

    @tparam template_collision_object @copydoc psyq::mosp_handle::collision_object
    @tparam template_space            @copydoc psyq::mosp_tree::space

    @sa psyq::mosp_handle::attach_tree() psyq::mosp_handle::detach_tree()
 */
template<
    typename template_collision_object,
    typename template_space>
class psyq::mosp_tree
{
    private: typedef mosp_tree self; ///< *thisの型。

    /// 空間分割木に取りつける、衝突判定handleの型。
    public: typedef mosp_handle<template_collision_object> handle;
    /// @cond
    friend handle;
    /// @endcond

    public: typedef mosp_node<template_collision_object> node;

    /** @brief 使用するmorton空間の型。

        mosp_space_2d や mosp_space_3d を使う。
     */
    public: typedef template_space space;

    /// 分割空間の辞書。
    private: typedef std::map<
        typename self::space::order, typename self::node*> cell_map;
    /// node のmemory割当子。
    private: typedef typename self::cell_map::allocator_type::template
        rebind<typename self::node>::other
            node_allocator;
    /// 対応できる空間分割の最深レベル。
    public: static const unsigned LEVEL_LIMIT =
        (8 * sizeof(typename self::space::order) - 1) / self::space::DIMENSION;

    //-------------------------------------------------------------------------
    /// @brief 衝突判定領域のない空間分割木を構築する。
    public: mosp_tree()
    :
        space_(
            typename self::space::vector(0, 0, 0),
            typename self::space::vector(0, 0, 0),
            0),
        idle_node_(nullptr),
        level_cap_(0),
        detect_collision_(false)
    {
        // pass
    }

    /// copy-constructorは使用禁止。
    private: mosp_tree(self const&);

    /** @brief move-constructor。
        @param[in,out] io_source 移動元となるinstance。
     */
    public: mosp_tree(
        self&& io_source)
    :
        cells_(std::move(io_source.cells_)),
        space_(io_source.space_),
        idle_node_(nullptr),
        level_cap_(io_source.level_cap_),
        detect_collision_(false)
    {
        if (io_source.detect_collision_)
        {
            // 衝突判定中はmoveできない。
            PSYQ_ASSERT(false);
            this->level_cap_ = 0;
        }
        else
        {
            io_source.level_cap_ = 0;
        }
    }

    /// @copydoc mosp_space::mosp_space
    public: mosp_tree(
        typename self::space::vector const& in_min,
        typename self::space::vector const& in_max,
        unsigned const                      in_level = self::LEVEL_LIMIT)
    :
        space_(in_min, in_max, in_level),
        idle_node_(nullptr),
        detect_collision_(false)
    {
        // 空間分割の最深levelを決定。
        if (in_level <= self::LEVEL_LIMIT)
        {
            this->level_cap_ = in_level;
        }
        else
        {
            // 最高level以上の空間は作れない。
            PSYQ_ASSERT(false);
            this->level_cap_ = self::LEVEL_LIMIT;
        }
    }

    /** @brief 取りつけられている衝突判定handleをすべて取り外し、
               空間分割木を破棄する。
     */
    public: ~mosp_tree()
    {
        // 衝突判定中は、破棄できない。
        PSYQ_ASSERT(!this->detect_collision_);
        typename self::node_allocator local_allocator(
            this->cells_.get_allocator());
        self::delete_cell(this->cells_.get_allocator(), this->idle_node_);
        for (auto i(this->cells_.begin()); i != this->cells_.end(); ++i)
        {
            self::delete_cell(this->cells_.get_allocator(), i->second);
        }
    }

    /// copy代入演算子は使用禁止。
    private: self& operator=(self const&);

    /** @brief move代入演算子。
        @param[in,out] io_source 移動元となるinstance。
     */
    public: self& operator=(self&& io_source)
    {
        if (this != &io_source)
        {
            this->~self();
            new(this) self(std::move(io_source));
        }
        return *this;
    }

    //-------------------------------------------------------------------------
    /** @brief 空間分割木を用いて、衝突を判定する。

        mosp_handle::attach_tree() によって空間分割木に取りつけられた
        衝突判定ハンドルのうち、分割空間が重なった衝突判定ハンドルの持つ
        mosp_handle::collision_object を引数として、
        衝突判定コールバック関数を呼び出す。

        @param[in] in_detect_callback
            2つの mosp_handle::collision_object を引数とする、
            衝突判定コールバック関数。
            2つの衝突判定ハンドルの分割空間が重なったとき、呼び出される。

        @retval ture 成功。衝突判定を行った。
        @retval false
            失敗。衝突判定を行わなかった。
            原因は、すでに衝突判定を行なってる最中だから。
     */
    public: template<typename template_detect_callback>
    bool detect_collision(
        template_detect_callback const& in_detect_callback)
    {
        if (!this->detect_collision_)
        {
            this->detect_collision_ = true;
            for (auto i(this->cells_.begin()); i != this->cells_.end();)
            {
                i = this->detect_collision_super_cells(i, in_detect_callback);
            }
            this->detect_collision_ = false;
        }
        return !this->detect_collision_;
    }

    //-------------------------------------------------------------------------
    private: template<typename template_detect_callback>
    typename self::cell_map::iterator detect_collision_super_cells(
        typename self::cell_map::iterator const& in_cell,
        template_detect_callback const&          in_detect_callback)
    {
        // (1) この分割空間で、衝突判定を行う。
        auto const local_cell(in_cell->second);
        auto local_last_node(local_cell);
        auto local_node(&local_cell->get_next());
        unsigned local_node_count(0);
        while (local_node != local_cell)
        {
            auto const local_exist_handle(
                this->detect_collision_node_and_cell(
                    *local_node, *local_node, *local_cell, in_detect_callback));
            if (local_exist_handle)
            {
                // 同じ分割空間の次のnodeへ移行する。
                local_last_node = local_node;
                ++local_node_count;
            }
            else
            {
                // 空handleのnodeだったので、分割空間から削除する。
                this->collect_idle_node(local_last_node->remove_next());
            }
            local_node = &local_last_node->get_next();
        }
        if (&local_cell->get_next() == local_cell)
        {
            return this->cells_.erase(in_cell);
        }

        // (2) この分割空間と上位の分割空間で、衝突判定を行う。
        for (auto local_super_order(in_cell->first); 0 < local_super_order;)
        {
            // 上位の分割空間に移動する。
            local_super_order =
                (local_super_order - 1) >> self::space::DIMENSION;
            auto const local_super_iterator(
                this->cells_.find(local_super_order));
            if (local_super_iterator != this->cells_.end())
            {
                // 上位の分割空間と衝突判定を行う。
                auto const local_super_cell(local_super_iterator->second);
                this->detect_collision_cell_and_cell(
                    *local_cell, *local_super_cell, in_detect_callback);
                if (&local_cell->get_next() == local_cell)
                {
                    return this->cells_.erase(in_cell);
                }
                if (&local_super_cell->get_next() == local_super_cell)
                {
                    this->cells_.erase(local_super_iterator);
                }
            }
        }
        auto local_iterator(in_cell);
        return ++local_iterator;
    }

    //-------------------------------------------------------------------------
    private: template<typename template_detect_callback>
    void detect_collision_cell_and_cell(
        typename self::node&            io_cell0,
        typename self::node&            io_cell1,
        template_detect_callback const& in_detect_callback)
    {
        auto local_last_node(&io_cell0);
        auto local_node(&io_cell0.get_next());
        while (local_node != &io_cell0)
        {
            auto const local_exist_handle(
                this->detect_collision_node_and_cell(
                    *local_node, io_cell1, io_cell1, in_detect_callback));
            if (local_exist_handle)
            {
                // 同じ分割空間の次のnodeへ移行する。
                local_last_node = local_node;
            }
            else
            {
                // 空handleのnodeだったので、分割空間から削除する。
                this->collect_idle_node(local_last_node->remove_next());
            }
            local_node = &local_last_node->get_next();
        }
    }

    /** @brief nodeと分割空間で、衝突判定を行う。
        @param[in,out] io_node             衝突判定を行うnode。
        @param[in,out] io_cell_begin_back  衝突判定を行う分割空間の先頭の直前のnode。
        @param[in]     in_cell_end         衝突判定を行う分割空間の末尾node。
        @param[in]     in_detect_callback  衝突判定callback関数。
        @retval true  io_node は空になってない。
        @retval false io_node が空になった。
     */
    private: template<typename template_detect_callback>
    bool detect_collision_node_and_cell(
        typename self::node&            io_node,
        typename self::node&            io_cell_begin_back,
        typename self::node const&      in_cell_end,
        template_detect_callback const& in_detect_callback)
    {
        auto local_node_handle(io_node.get_handle());
        if (local_node_handle == nullptr)
        {
            return false;
        }

        auto local_Lastcell(&io_cell_begin_back);
        auto local_cell(&io_cell_begin_back.get_next());
        while (local_cell != &in_cell_end)
        {
            auto const local_cell_handle(local_cell->get_handle());
            if (local_cell_handle != nullptr)
            {
                // 衝突判定。
                in_detect_callback(
                    local_node_handle->object_, local_cell_handle->object_);

                // 衝突判定callback関数の中で handle::detach_tree()
                // される可能性があるので、再取得する。
                local_node_handle = io_node.get_handle();
                if (local_node_handle == nullptr)
                {
                    return false;
                }
                local_Lastcell = local_cell;
            }
            else
            {
                // 空handleのnodeだったので、分割空間から削除する。
                this->collect_idle_node(local_Lastcell->remove_next());
            }
            local_cell = &local_Lastcell->get_next();
        }
        return true;
    }

    //-------------------------------------------------------------------------
    /** @brief AABBを包む最小の分割空間のnodeを構築する。
        @param[in] in_min 絶対座標系AABBの最小値。
        @param[in] in_max 絶対座標系AABBの最大値。
        @retval !=nullptr AABBを包む最小の分割空間のnode。
        @retval ==nullptr 失敗。
     */
    private: typename self::node* make_node(
        typename self::space::vector const& in_min,
        typename self::space::vector const& in_max)
    {
        if (this->detect_collision_)
        {
            // detect_collision() の実行中はnodeを作れない。
            PSYQ_ASSERT(false);
            return nullptr;
        }

        // morton順序に対応する分割空間を用意する。
        auto const local_morton_order(this->calc_order(in_min, in_max));
        auto& local_cell(this->cells_[local_morton_order]);
        if (local_cell == nullptr)
        {
            local_cell = this->distribute_idle_node();
            if (local_cell == nullptr)
            {
                PSYQ_ASSERT(false);
                this->cells_.erase(local_morton_order);
                return nullptr;
            }
        }

        // morton順序に対応する空間分割木nodeを用意する。
        auto const local_node(this->distribute_idle_node());
        if (local_node == nullptr || !local_cell->insert_next(*local_node))
        {
            PSYQ_ASSERT(local_node != nullptr);
            PSYQ_ASSERT(false);
            return nullptr;
        }
        return local_node;
    }

    //-------------------------------------------------------------------------
    private: typename self::space::order calc_order(
        typename self::space::vector const& in_min,
        typename self::space::vector const& in_max)
    const
    {
        auto const local_level_cap(this->level_cap_);
        if (local_level_cap <= 0)
        {
            return 0;
        }

        // 衝突物体のAABBを包む、最小の分割空間のmorton順序を算出する。
        auto const local_axis_order_max((1 << local_level_cap) - 1);
        auto const local_min_morton(
            this->space_.calc_order(in_min, local_axis_order_max));
        auto const local_max_morton(
            this->space_.calc_order(in_max, local_axis_order_max));
        auto const local_morton_distance(local_max_morton ^ local_min_morton);
        auto const local_nlz(psyq::count_leading_bit0(local_morton_distance));
        auto const local_level(
            local_morton_distance != 0?
                (sizeof(local_max_morton) * 8 + 1 - local_nlz)
                    / self::space::DIMENSION:
                1);
        auto const local_num_cells(
            1 << ((local_level_cap - local_level) * self::space::DIMENSION));
        auto const local_base(
            (local_num_cells - 1) / ((1 << self::space::DIMENSION) - 1));
        auto const local_shift(local_level * self::space::DIMENSION);
        // VisualStudioだと、32以上のbit-shift演算に不具合があるっぽい。
        PSYQ_ASSERT(local_shift < sizeof(local_max_morton) * 8);
        return local_base + (local_max_morton >> local_shift);
    }

    //-------------------------------------------------------------------------
    /** @brief 空handleのnodeを配布する。
        @retval !=nullptr 配布された空きhandleのnode。
        @retval ==nullptr 失敗。
        @sa collect_idle_node()
     */
    private: typename self::node* distribute_idle_node()
    {
        auto local_node(this->idle_node_);
        if (local_node == nullptr)
        {
            // 空handleのnodeがなかったので、新たにnodeを生成する。
            typename self::node_allocator local_allocator(
                this->cells_.get_allocator());
            return new(local_allocator.allocate(1)) typename self::node;
        }

        // 空handleのnodeを取り出す。
        local_node = &local_node->remove_next();
        if (local_node == this->idle_node_)
        {
            this->idle_node_ = nullptr;
        }
        return local_node;
    }

    /** @brief 空handleのnodeを回収する。
        @param[in,out] io_idle_node 回収するnode。
        @sa distribute_idle_node()
     */
    private: void collect_idle_node(
        typename self::node& io_idle_node)
    {
        if (!io_idle_node.is_alone())
        {
            PSYQ_ASSERT(false);
        }
        else if (
            this->idle_node_ != nullptr &&
            !this->idle_node_->insert_next(io_idle_node))
        {
            PSYQ_ASSERT(false);
        }
        else
        {
            this->idle_node_ = &io_idle_node;
        }
    }

    private: static void delete_cell(
        typename self::node_allocator io_allocator,
        typename self::node* const    io_cell)
    {
        if (io_cell != nullptr)
        {
            for (;;)
            {
                auto const local_cell(&io_cell->remove_next());
                local_cell->~node();
                io_allocator.deallocate(local_cell, 1);
                if (io_cell == local_cell)
                {
                    break;
                }
            }
        }
    }

    //-------------------------------------------------------------------------
    private: typename self::cell_map cells_; ///< 分割空間の辞書。
    private: typename self::space space_; ///< 使用するmorton空間。
    private: typename self::node* idle_node_; ///< 空handleのnode。
    private: std::size_t level_cap_; ///< 空間分割の最深level。
    private: bool detect_collision_; ///< detect_collision() を実行中かどうか。
};

#endif // !PSYQ_MOSP_COLLISION_HPP_
