﻿/** @file
    @brief morton順序を用いた空間分割木による衝突判定。

    使い方は、 psyq::mosp_handle と psyq::mosp_tree
    の詳細説明を参照すること。

    以下のweb-pageを参考にして実装した。
    http://marupeke296.com/COL_2D_No8_QuadTree.html
 */
#ifndef PSYQ_MOSP_TREE_HPP_
#define PSYQ_MOSP_TREE_HPP_
//#include "psyq/bit_algorithm.hpp"
//#include "psyq/geometric_utility.hpp"

/// psyq::mosp_coordinates で使う、defaultのvector型。
#ifndef PSYQ_MOSP_VECTOR_DEFAULT
#include <glm/glm.hpp> // OpenGL Mathematics
#define PSYQ_MOSP_VECTOR_DEFAULT glm::vec3
#endif

#ifndef PSYQ_MOSP_SPACE_DEFAULT
#define PSYQ_MOSP_SPACE_DEFAULT psyq::mosp_space_2d<>
#endif

namespace psyq
{
    /// @cond
    template<typename, unsigned, unsigned, unsigned> class mosp_coordinates;
    template<typename> class mosp_space;
    template<typename = mosp_coordinates<PSYQ_MOSP_VECTOR_DEFAULT, 0, 2, 1>>
        class mosp_space_2d;
    template<typename = mosp_coordinates<PSYQ_MOSP_VECTOR_DEFAULT, 0, 1, 2>>
        class mosp_space_3d;
    template<typename, typename> class mosp_handle;
    template<typename, typename = PSYQ_MOSP_SPACE_DEFAULT> class mosp_tree;
    /// @endcond
}

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 衝突判定に使うmorton座標。 psyq::mosp_space のtemplate引数に使う。
    @tparam template_vector   @copydoc psyq::mosp_coordinates::vector
    @tparam template_element0 @copydoc psyq::mosp_coordinates::ELEMENT0_INDEX
    @tparam template_element1 @copydoc psyq::mosp_coordinates::ELEMENT1_INDEX
    @tparam template_element2 @copydoc psyq::mosp_coordinates::ELEMENT2_INDEX
 */
template<
    typename template_vector,
    unsigned template_element0,
    unsigned template_element1,
    unsigned template_element2>
class psyq::mosp_coordinates
{
    /// *thisの型。
    private: typedef mosp_coordinates<
        template_vector,
        template_element0,
        template_element1,
        template_element2>
            self;

    public: enum: unsigned
    {
        /// morton座標の成分#0のindex番号。
        ELEMENT0_INDEX = template_element0,
        /// morton座標の成分#1のindex番号。
        ELEMENT1_INDEX = template_element1,
        /// morton座標の成分#2のindex番号。
        ELEMENT2_INDEX = template_element2,
    };

    /// morton座標を表すvectorの型。 glm::vec3 互換であること。
    public: typedef template_vector vector;

    /// morton座標を表すvectorの成分の型。
    public: typedef typename
        psyq::geometric_vector<template_vector>::element element;

    /// morton座標のAABB。
    public: typedef psyq::geometric_aabb<template_vector> aabb;

    /** @brief 絶対座標系からmorton座標への変換scaleを算出する。
        @param[in] in_morton_size morton座標の最大値。
        @param[in] in_world_size  絶対座標系でのmorton空間の大きさ。
     */
    public: static typename self::element calc_scale(
        typename self::element const in_morton_size,
        typename self::element const in_world_size)
    {
        if (in_world_size
            < std::numeric_limits<typename self::element>::epsilon())
        {
            return 0;
        }
        return in_morton_size / in_world_size;
    }

    /** @brief 絶対座標系空間からmorton座標空間への変換scaleを算出する。
        @param[in] in_aabb  衝突判定領域の全体を包む、絶対座標系AABB。
        @param[in] in_level 空間分割の最深level。
     */
    public: static typename self::vector calc_scale(
        typename self::aabb const& in_aabb,
        unsigned const             in_level)
    {
        auto const local_size(in_aabb.get_max() - in_aabb.get_min());
        auto const local_unit(
            static_cast<typename self::element>(1 << in_level));
        return psyq::geometric_vector<self::vector>::make(
            self::calc_scale(
                local_unit, psyq::geometric_vector_element(local_size, 0)),
            self::calc_scale(
                local_unit, psyq::geometric_vector_element(local_size, 1)),
            self::calc_scale(
                local_unit, psyq::geometric_vector_element(local_size, 2)));
    }
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
    /// 衝突判定に使う psyq::mosp_coordinates の型。
    public: typedef template_coordinates coordinates;
    /// morton順序の型。
    public: typedef std::size_t order;

    //-------------------------------------------------------------------------
    /** @brief 衝突判定を行う領域を設定する。
        @param[in] in_aabb  衝突判定を行う領域の全体を包む、絶対座標系AABB。
        @param[in] in_level 空間分割の最深レベル。
     */
    protected: mosp_space(
        typename self::coordinates::aabb const& in_aabb,
        unsigned const                          in_level)
    :
        aabb_(in_aabb),
        scale_(self::coordinates::calc_scale(in_aabb, in_level))
    {}

    /** @brief 衝突判定を行う領域の全体を包む、絶対座標系AABBを取得する。
        @return 衝突判定を行う領域の全体を包む、絶対座標系AABB。
     */
    public: typename self::coordinates::aabb const& get_aabb() const
    {
        return this->aabb_;
    }

    /** @brief 絶対座標系ベクトルの要素を、morton座標に変換する。
        @param[in] in_vector        変換する絶対座標系ベクトル。
        @param[in] in_element_index 変換する要素のインデックス番号。
        @return morton座標でのベクトル要素の値。
     */
    protected: typename self::coordinates::element transform_element(
        typename self::coordinates::vector const& in_vector,
        unsigned const                            in_element_index)
    const
    {
        auto local_element(
            psyq::geometric_vector_element(in_vector, in_element_index));
        auto const local_min(
            psyq::geometric_vector_element(
                this->aabb_.get_min(), in_element_index));
        if (local_element < local_min)
        {
            return 0;
        }
        auto const local_max(
            psyq::geometric_vector_element(
                this->aabb_.get_max(), in_element_index));
        if (local_max < local_element)
        {
            local_element = local_max;
        }
        return (local_element - local_min)
            * psyq::geometric_vector_element(this->scale_, in_element_index);
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

    //-------------------------------------------------------------------------
    /// 衝突判定を行う全体の領域の、絶対座標系AABB。
    private: typename self::coordinates::aabb aabb_;

    /// 最小となる分割空間の、絶対座標系での大きさの逆数。
    private: typename self::coordinates::vector scale_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief 2次元座標で衝突判定を行うmorton空間。

    mosp_tree のtemplate引数に使う。

    @tparam template_coordinates @copydoc mosp_space::coordinates
 */
template<typename template_coordinates>
class psyq::mosp_space_2d: public psyq::mosp_space<template_coordinates>
{
    /// *thisの型。
    private: typedef mosp_space_2d<template_coordinates> self;
    /// *thisの上位型。
    public: typedef mosp_space<template_coordinates> super;

    public: enum: unsigned
    {
        DIMENSION = 2, ///< 衝突判定に使う座標の成分の数。
    };

    //-------------------------------------------------------------------------
    /// @copydoc mosp_space::mosp_space
    public: mosp_space_2d(
        typename super::coordinates::aabb const& in_aabb,
        unsigned const                           in_level)
    :
        super(in_aabb, in_level)
    {}

    /** @brief 2次元座標上の点から、線形4分木のmorton順序を算出する。
        @param[in] in_point 2次元座標上の点。
        @param[in] in_max   morton座標の最大値。
        @return 2次元座標に対応するmorton順序。
     */
    public: typename super::order calc_order(
        typename super::coordinates::vector const& in_point,
        typename super::order const                in_max)
    const
    {
        auto const local_element0(
            this->transform_element(
                in_point, super::coordinates::ELEMENT0_INDEX));
        auto const local_element1(
            this->transform_element(
                in_point, super::coordinates::ELEMENT1_INDEX));
        return self::separate_bits(local_element0, in_max) << 0
            |  self::separate_bits(local_element1, in_max) << 1;
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
template<typename template_coordinates>
class psyq::mosp_space_3d: public psyq::mosp_space<template_coordinates>
{
    /// *thisの型。
    private: typedef mosp_space_3d<template_coordinates> self;

    /// *thisの上位型。
    public: typedef mosp_space<template_coordinates> super;

    public: enum: unsigned
    {
        DIMENSION = 3, ///< @copydoc mosp_space_2d::DIMENSION
    };

    //-------------------------------------------------------------------------
    /// @copydoc mosp_space::mosp_space
    public: mosp_space_3d(
        typename super::coordinates::aabb const& in_aabb,
        unsigned const                           in_level)
    :
        super(in_aabb, in_level)
    {}

    /** @brief 3次元座標上の点から、線形8分木のmorton順序を算出する。
        @param[in] in_point 3次元座標上の点。
        @param[in] in_max   morton順序の最大値。
        @return 3次元座標に対応するmorton順序。
     */
    public: typename super::order calc_order(
        typename super::coordinates::vector const& in_point,
        typename self::order const                 in_max)
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
        return self::separate_bits(local_element0, in_max) << 0
            |  self::separate_bits(local_element1, in_max) << 1
            |  self::separate_bits(local_element2, in_max) << 2;
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
/** @brief morton順序による空間分割木を用いた衝突判定ハンドル。

    使い方の概要。
    -# 衝突する物体の識別値を、 mosp_handle::object_ に代入しておく。
    -# mosp_handle::attach_tree() を呼び出し、
       mosp_handle を mosp_tree 空間分割木に取りつける。
    -# mosp_tree::detect_collision() を呼び出し、衝突判定を行う。

    @tparam template_collision_object @copydoc mosp_handle::collision_object
    @tparam template_morton_order     @copydoc mosp_handle::order
 */
template<typename template_collision_object, typename template_morton_order>
class psyq::mosp_handle
{
    /// *thisの型。
    private: typedef psyq::mosp_handle<
        template_collision_object, template_morton_order>
            self;

    /// 衝突判定オブジェクトの識別子。
    public: typedef template_collision_object collision_object;
    /// @copydoc psyq::morton_space::order
    public: typedef template_morton_order order;

    //-------------------------------------------------------------------------
    /** @brief 衝突判定ハンドルを構築する。
        @param[in] in_object *thisに対応する、衝突判定オブジェクトの識別子。
     */
    public: explicit mosp_handle(template_collision_object in_object):
        cell_(nullptr),
        object_(std::move(in_object))
    {}

    /// copy-constructorは使用禁止。
    private: mosp_handle(self const&);

    /** @brief move-constructor。
        @param[in,out] io_source 移動元となるinstance。
     */
    public: mosp_handle(self&& io_source):
        cell_(io_source.cell_),
        object_(std::move(io_source.object_))
    {
        if (io_source.cell_ != nullptr)
        {
            io_source.cell_ = nullptr;
            this->cell_->second = this;
        }
    }

    /// @brief *thisを mosp_tree 空間分割木から取り外す。
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
    /** @brief *thisを mosp_tree 空間分割木に取りつける。

        現在取りつけられている mosp_tree から*thisを切り離し、
        新しい mosp_tree に*thisを取りつける。

        @param[in,out] io_tree *thisを取りつける mosp_tree 。
        @param[in]     in_aabb *thisに対応する衝突領域の、絶対座標系AABB。
        @sa detach_tree() is_attached()
     */
    public: template<typename template_mosp_tree>
    bool attach_tree(
        template_mosp_tree&                                          io_tree,
        typename template_mosp_tree::space::coordinates::aabb const& in_aabb)
    {
        // 新たな分割空間へのリンクを用意する。
        auto const local_cell(io_tree.make_cell(in_aabb));
        if (local_cell == nullptr)
        {
            return false;
        }

        // 古いリンクから新しいリンクへ切り替える。
        if (this->cell_ != nullptr)
        {
            PSYQ_ASSERT(this == this->cell_->second);
            this->cell_->second = nullptr;
        }
        this->cell_ = local_cell;
        local_cell->second = this;
        return true;
    }

    /** @brief *thisを mosp_tree 空間分割木から取り外す。

        現在取りつけられている mosp_tree から、*thisを切り離す。

        @sa attach_tree() is_attached()
     */
    public: void detach_tree()
    {
        if (this->cell_ != nullptr)
        {
            PSYQ_ASSERT(this == this->cell_->second);
            this->cell_->second = nullptr;
            this->cell_ = nullptr;
        }
    }

    /** @brief *thisが mosp_tree 空間分割木に取りつけられているか判定する。
        @retval true  *thisは mosp_tree に取りつけられている。
        @retval false *thisは mosp_tree に取りつけられていない。
        @sa attach_tree() detach_tree()
     */
    public: bool is_attached() const
    {
        return this->cell_ != nullptr;
    }

    //-------------------------------------------------------------------------
    /// *thisに対応する分割空間。
    private: std::pair<template_morton_order const, self*>* cell_;

    /** @brief thisに対応する、衝突判定オブジェクトの識別子。

        この値を引数として、 mosp_tree::detect_collision()
        の引数に指定した、衝突callback関数が呼び出される。
     */
    public: template_collision_object object_;
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief morton順序による空間分割木。

    使い方の概要は、以下の通り。
    -# mosp_tree::mosp_tree() を呼び出し、
       衝突判定を行う領域を空間分割木に設定する。
    -# mosp_handle::attach_tree() を呼び出し、
       mosp_handle を mosp_tree に取りつける。
    -# mosp_tree::detect_collision() を呼び出し、衝突判定を行う。

    @tparam template_collision_object @copydoc mosp_handle::collision_object
    @tparam template_space            @copydoc mosp_tree::space

    @note mosp_tree::handle_map に任意の辞書template型を指定できるようにしたい。
 */
template<typename template_collision_object, typename template_space>
class psyq::mosp_tree
{
    private: typedef mosp_tree self; ///< *thisの型。

    /// 使用するmorton空間。 morton_space_2d や morton_space_3d を使う。
    public: typedef template_space space;

    /// 空間分割木に取りつける mosp_handle 。
    public: typedef psyq::mosp_handle<
        template_collision_object, typename template_space::order>
            handle;
    /// @cond
    friend handle;
    /// @endcond

    /// 分割空間の辞書。
    private: typedef std::multimap<
        typename self::space::order, typename self::handle*>
            cell_map;

    public: enum: unsigned
    {
        /// 対応できる空間分割の限界の分割レベル。
        LEVEL_LIMIT = (8 * sizeof(typename self::space::order) - 1)
            / self::space::DIMENSION
    };

    //-------------------------------------------------------------------------
    /** @brief 衝突判定を行う領域を設定する。
        @param[in] in_aabb  衝突判定を行う領域の全体を包む、絶対座標系AABB。
        @param[in] in_level 空間分割の最深レベル。
     */
    public: explicit mosp_tree(
        typename self::space::coordinates::aabb const& in_aabb,
        unsigned const in_level = self::LEVEL_LIMIT)
    :
        space_(in_aabb, in_level),
        level_cap_(in_level),
        detect_collision_(false)
    {
        // 限界レベルより深い分割空間は作れない。
        if (self::LEVEL_LIMIT < in_level)
        {
            PSYQ_ASSERT(false);
            this->level_cap_ = self::LEVEL_LIMIT;
        }
    }

    /// copy-constructorは使用禁止。
    private: mosp_tree(self const&);

    /** @brief move-constructor。
        @param[in,out] io_source 移動元となるinstance。
     */
    public: mosp_tree(self&& io_source):
        space_(io_source.space_),
        cell_map_(std::move(io_source.cell_map_)),
        level_cap_(io_source.level_cap_),
        detect_collision_(false)
    {
        io_source.cell_map_.clear();
        if (io_source.detect_collision_)
        {
            // 衝突判定中はmoveできない。
            PSYQ_ASSERT(false);
            this->cell_map_.swap(io_source.cell_map_);
        }
    }

    /** @brief 取りつけられてる mosp_handle をすべて取り外し、
               空間分割木を破棄する。
     */
    public: ~mosp_tree()
    {
        // 衝突判定中は、破棄できない。
        PSYQ_ASSERT(!this->detect_collision_);

        // ハンドルをすべて切り離す。
        for (auto& local_value: this->cell_map_)
        {
            auto const local_handle(local_value.second);
            if (local_handle != nullptr)
            {
                local_handle->detach_tree();
            }
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
    /** @brief thisに取りつけられたすべての mosp_handle の衝突判定を行う。

        mosp_handle::attach_tree() によってthisに取りつけられた
        mosp_handle のうち、分割空間が重なった2つの mosp_handle の持つ
        mosp_handle::collision_object を引数に、衝突callback関数を呼び出す。

        @param[in] in_detect_callback
            衝突callback関数。引数として、2つの mosp_handle::collision_object
            を受け取ること。戻り値はなくてよい。
            2つの mosp_handle が所属する分割空間が重なったとき、呼び出される。

        @retval ture 成功。衝突判定を行った。
        @retval false
            失敗。衝突判定を行わなかった。
            原因は、すでに衝突判定を行なってる最中だから。
     */
    public: template<typename template_detect_callback>
    bool detect_collision(template_detect_callback const& in_detect_callback)
    {
        // 衝突判定を開始する。
        auto const local_cell_map(this->begin_detect());
        if (local_cell_map == nullptr)
        {
            return false;
        }

        // 衝突判定を行う。
        for (auto i(local_cell_map->begin()); i != local_cell_map->end(); ++i)
        {
            self::detect_collision_map(*local_cell_map, i, in_detect_callback);
        }

        // 衝突判定を終了する。
        this->end_detect();
        return true;
    }

    //-------------------------------------------------------------------------
    /** @brief 空間分割木の衝突判定を開始する。

        end_detect() と対になるように呼び出すこと。

        @retval !=nullptr 衝突判定を行うcellの辞書。 detect_cell() で使う。
        @retval ==nullptr 失敗。すでに begin_detect() が呼び出されていた。
     */
    private: typename self::cell_map const* begin_detect()
    {
        // ひとつのインスタンスで多重に detect_collision() できない。
        if (this->detect_collision_)
        {
            return nullptr;
        }
        this->detect_collision_ = true;

        // 空になった要素を削除する。
        for (auto i(this->cell_map_.begin()); i != this->cell_map_.end();)
        {
            auto const local_handle(i->second);
            if (local_handle != nullptr)
            {
                ++i;
            }
            else
            {
                i = this->cell_map_.erase(i);
            }
        }
        return &this->cell_map_;
    }

    /** @brief 空間分割木の衝突判定を終了する。

        begin_detect() と対になるように呼び出すこと。
     */
    private: void end_detect()
    {
        PSYQ_ASSERT(this->detect_collision_);
        this->detect_collision_ = false;
    }

    /** @brief 分割空間と分割空間の辞書とで、衝突判定を行う。

        begin_detect() と end_detect() の間で呼び出すこと。

        @param[in] in_cell_map      衝突判定を行う分割空間の辞書。
        @param[in] in_cell_iterator 衝突の対象となる分割空間の反復子。
        @param[in] in_detect_callback
            衝突callback関数。戻り値はなくてよい。
            引数として、2つの mosp_handle::collision_object を受け取ること。
            2つの mosp_handle が所属する分割空間が重なったとき、呼び出される。
     */
    private: template<typename template_detect_callback>
    static void detect_collision_map(
        typename self::cell_map const&                 in_cell_map,
        typename self::cell_map::const_iterator const& in_cell_iterator,
        template_detect_callback const&                in_detect_callback)
    {
        if (in_cell_iterator == in_cell_map.end())
        {
            return;
        }

        // 同じ分割空間の内部だけで、衝突判定を行う。
        auto const local_next_cell(std::next(in_cell_iterator));
        if (local_next_cell != in_cell_map.end()
            && in_cell_iterator->first == local_next_cell->first)
        {
            self::detect_collision_container(
                local_next_cell,
                in_cell_map.end(),
                *in_cell_iterator,
                in_detect_callback);
        }

        // cellと上位のcellとで、衝突判定を行う。
        for (
            auto local_super_order(in_cell_iterator->first);
            0 < local_super_order;)
        {
            // 上位のcellへ移動し、衝突判定を行う。
            local_super_order
                = (local_super_order - 1) >> self::space::DIMENSION;
            auto const local_super_iterator(
                in_cell_map.find(local_super_order));
            if (local_super_iterator != in_cell_map.end())
            {
                self::detect_collision_container(
                    local_super_iterator,
                    in_cell_map.end(),
                    *in_cell_iterator,
                    in_detect_callback);
            }
        }
    }

    /** @brief 分割空間と分割空間コンテナとで、衝突判定を行う。
        @param[in] in_container_begin 衝突判定を行う分割空間コンテナの先頭位置。
        @param[in] in_container_end   衝突判定を行う分割空間コンテナの終端位置。
        @param[in] in_cell            衝突の対象となる分割空間。
        @param[in] in_detect_callback
            衝突callback関数。戻り値はなくてよい。
            引数として、2つの mosp_handle::collision_object を受け取ること。
            2つの mosp_handle が所属する分割空間が重なったとき、呼び出される。
     */
    private: template<typename template_detect_callback>
    static void detect_collision_container(
        typename self::cell_map::const_iterator const& in_container_begin,
        typename self::cell_map::const_iterator const& in_container_end,
        typename self::cell_map::value_type const&     in_cell,
        template_detect_callback const&                in_detect_callback)
    {
        auto local_cell_handle(in_cell.second);
        if (local_cell_handle == nullptr)
        {
            return;
        }
        PSYQ_ASSERT(in_container_begin != in_container_end);
        auto const local_container_order(in_container_begin->first);
        for (
            auto local_container_cell(in_container_begin);
            local_container_cell != in_container_end
            && local_container_order == local_container_cell->first;
            ++local_container_cell)
        {
            auto const local_container_handle(local_container_cell->second);
            if (local_container_handle == nullptr)
            {
                continue;
            }

            // 衝突callback関数を呼び出す。
            in_detect_callback(
                local_cell_handle->object_,
                local_container_handle->object_);

            // 衝突callback関数の中で handle::detach_tree()
            // される場合があるので、 handle を再取得する。
            local_cell_handle = in_cell.second;
            if (local_cell_handle == nullptr)
            {
                return;
            }
        }
    }

    //-------------------------------------------------------------------------
    /** @brief AABBを包む最小の分割空間へのリンクを構築する。
        @param[in] in_min 絶対座標系AABBの最小値。
        @param[in] in_max 絶対座標系AABBの最大値。
        @retval !=nullptr AABBを包む最小の分割空間。
        @retval ==nullptr 失敗。
     */
    private: typename self::cell_map::value_type* make_cell(
        typename self::space::coordinates::aabb const& in_aabb)
    {
        if (this->detect_collision_)
        {
            // detect_collision() の実行中は分割空間を作れない。
            PSYQ_ASSERT(false);
            return nullptr;
        }

        // morton順序に対応する分割空間を用意する。
        auto const local_morton_order(
            self::calc_order(this->level_cap_, this->space_, in_aabb));
        auto const local_cell_iterator(
            this->cell_map_.emplace(local_morton_order, nullptr));
        return &(*local_cell_iterator);
    }

    //-------------------------------------------------------------------------
    /** @brief AABBを包む最小の分割空間のmorton順序を算出する。
        @param[in] in_level_cap 空間分割の最深level。
        @param[in] in_space     使用するmorton空間。
        @param[in] in_min       絶対座標系AABBの最小値。
        @param[in] in_max       絶対座標系AABBの最大値。
        @return AABBを包む最小の分割空間のmorton順序。
     */
    private: static typename self::space::order calc_order(
        std::size_t const                              in_level_cap,
        typename self::space const&                    in_space,
        typename self::space::coordinates::aabb const& in_aabb)
    {
        if (in_level_cap <= 0)
        {
            return 0;
        }

        // 衝突物体のAABBを包む、最小の分割空間のmorton順序を算出する。
        unsigned const local_axis_order_max((1 << in_level_cap) - 1);
        auto const local_min_morton(
            in_space.calc_order(in_aabb.get_min(), local_axis_order_max));
        auto const local_max_morton(
            in_space.calc_order(in_aabb.get_max(), local_axis_order_max));
        auto const local_morton_distance(local_max_morton ^ local_min_morton);
        unsigned local_level;
        if (local_morton_distance != 0)
        {
            auto const local_clz(
                psyq::count_leading_0bits(local_morton_distance));
            local_level = (sizeof(local_morton_distance) * 8 + 1 - local_clz)
                / self::space::DIMENSION;
        }
        else
        {
            local_level = 1;
        }
        unsigned const local_cell_count(
            1 << ((in_level_cap - local_level) * self::space::DIMENSION));
        unsigned const local_base(
            (local_cell_count - 1) / ((1 << self::space::DIMENSION) - 1));
        auto const local_shift(local_level * self::space::DIMENSION);
        /** @note
            bit数以上のbit-right-shift演算は、
            C言語の仕様として未定義の動作となる。
            http://hexadrive.sblo.jp/article/56575654.html
         */
        PSYQ_ASSERT(local_shift < sizeof(local_max_morton) * 8);
        return local_base + (local_max_morton >> local_shift);
    }

    //-------------------------------------------------------------------------
    private: typename self::space space_; ///< @copydoc space
    private: typename self::cell_map cell_map_; ///< @copydoc cell_map
    private: std::size_t level_cap_; ///< 空間分割の最深level。
    private: bool detect_collision_; ///< detect_collision() を実行中かどうか。
};

#endif // !defined(PSYQ_MOSP_TREE_HPP_)
