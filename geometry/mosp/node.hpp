/** @file
    @author Hillco Psychi (https://twitter.com/psychi)
    @brief モートン空間分割木に取りつけるノードの実装。
 */
#ifndef PSYQ_GEOMETRY_MOSP_NODE_HPP_
#define PSYQ_GEOMETRY_MOSP_NODE_HPP_

#include "../../assert.hpp"

/// @cond
namespace psyq
{
    namespace geometry
    {
        namespace mosp
        {
            template<typename, typename> class node;
        } // namespace mosp
    } // namespace geometry
} // namespace psyq
/// @endcond

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
/** @brief モートン順序による空間分割木に取りつける分割空間ノード。

    使い方の概要。
    -# psyq::geometry::mosp::node インスタンスを構築し、
       psyq::geometry::mosp::node::argument_
       に衝突判定オブジェクトの識別子を設定する。
    -# psyq::geometry::mosp::node::attach_tree で、
       psyq::geometry::mosp::node インスタンス を
       psyq::geometry::mosp::tree インスタンスに取りつける。
    -# psyq::geometry::mosp::tree::detect_collision で、
       psyq::geometry::mosp::tree インスタンスに取りつけられているすべての
       psyq::geometry::mosp::node インスタンスで衝突判定を行う。
       - 2つの psyq::geometry::mosp::node インスタンスの、
         それぞれが所属している分割空間が衝突していると、
         それら2つのインスタンスの psyq::geometry::mosp::node::argument_
         を引数に、衝突関数が呼び出される。

    @tparam template_argument     @copydoc psyq::geometry::mosp::node::argument
    @tparam template_morton_order @copydoc psyq::geometry::mosp::node::order
    @ingroup psyq_geometry_mosp
 */
template<typename template_argument, typename template_morton_order>
class psyq::geometry::mosp::node
{
    /// @brief thisが指す値の型。
    private: typedef node this_type;

    /** @brief psyq::geometry::mosp::tree::detect_collision
               から呼び出される、衝突関数に渡す引数の型。
     */
    public: typedef template_argument argument;
    /// @copydoc psyq::geometry::mosp::space::order
    public: typedef template_morton_order order;

    //-------------------------------------------------------------------------
    /// @name 構築と解体
    //@{
    /** @brief 分割空間ノードを構築する。
        @param[in] in_argument 衝突関数に渡す引数。
     */
    public: explicit node(typename this_type::argument in_argument):
    handle_(nullptr),
    argument_(std::move(in_argument))
    {}

    /** @brief ムーブ構築子。
        @param[in,out] io_source ムーブ元インスタンス。
     */
    public: node(this_type&& io_source):
    handle_(io_source.handle_),
    argument_(std::move(io_source.argument_))
    {
        if (this->handle_->second != nullptr)
        {
            io_source.handle_ = nullptr;
            this->handle_->second = this;
        }
    }

    /// @brief *thisを空間分割木から取り外す。
    public: ~node()
    {
        this->detach_tree();
    }

    /** @brief ムーブ代入演算子。
        @param[in,out] io_source ムーブ元インスタンス。
     */
    public: this_type& operator=(this_type&& io_source)
    {
        if (this != &io_source)
        {
            this->~this_type();
            new(this) this_type(std::move(io_source));
        }
        return *this;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @name 空間分割木との連結
    //@{
    /** @brief 空間分割木に取りつける。

        現在取りつけられている空間分割木から*thisを切り離し、
        新しい空間分割木に*thisを取りつける。

        @param[in,out] io_tree
            *thisを取りつける空間分割木。 psyq::geometry::mosp::tree
            互換のインターフェイスを持っている必要がある。
        @param[in] in_aabb *thisに対応する衝突領域の、絶対座標系AABB。
        @sa this_type::detach_tree
        @sa this_type::is_attached
     */
    public: template<typename template_mosp_tree>
    bool attach_tree(
        template_mosp_tree& io_tree,
        typename template_mosp_tree::space::coordinate::aabb const& in_aabb)
    {
        // 新たな分割空間ハンドルを用意する。
        auto const local_handle(io_tree.make_handle(*this, in_aabb));
        if (local_handle == nullptr)
        {
            return false;
        }

        // 古い分割空間ハンドルから新たな分割空間ハンドルへ切り替える。
        if (this->handle_ != nullptr)
        {
            PSYQ_ASSERT(this == this->handle_->second);
            this->handle_->second = nullptr;
        }
        this->handle_ = local_handle;
        return true;
    }

    /** @brief 空間分割木から取り外す。

        現在取りつけられている空間分割木から、*thisを切り離す。

        @sa this_type::attach_tree
        @sa this_type::is_attached
     */
    public: void detach_tree()
    {
        auto const local_handle(this->handle_);
        if (local_handle != nullptr)
        {
            PSYQ_ASSERT(this == local_handle->second);
            local_handle->second = nullptr;
            this->handle_ = nullptr;
        }
    }

    /** @brief 空間分割木に取りつけられているか判定する。
        @retval true  *thisは空間分割木に取りつけられている。
        @retval false *thisは空間分割木に取りつけられていない。
        @sa this_type::attach_tree
        @sa this_type::detach_tree
     */
    public: bool is_attached() const
    {
        return this->handle_ != nullptr;
    }
    //@}
    //-------------------------------------------------------------------------
    /// @brief コピー構築子は使用禁止。
    private: node(this_type const&);

    /// @brief コピー代入演算子は使用禁止。
    private: this_type& operator=(const this_type&);

    //-------------------------------------------------------------------------
    /// *thisと連結している psyq::geometry::mosp::tree::node_map の要素。
    private: std::pair<template_morton_order const, this_type*>* handle_;

    /** @brief *thisに対応する、衝突判定オブジェクトの識別子。

        psyq::geometry::mosp::tree::detect_collision
        で、*thisの所属する分割空間が他のノードの分割空間と重なったとき、
        この値を引数として衝突関数が呼び出される。
     */
    public: typename this_type::argument argument_;

}; // class psyq::geometry::mosp::node

#endif // !defined(PSYQ_GEOMETRY_MOSP_NODE_HPP_)
// vim: set expandtab:
