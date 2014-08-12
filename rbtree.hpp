#ifndef PSYQ_RBTREE_HPP_
#define PSYQ_RBTREE_HPP_

//-----------------------------------------------------------------------------
#define RB_ROOT	(rb_root) { nullptr, }
#define	rb_entry(ptr, type, member) container_of(ptr, type, member)

#define rb_entry_safe(ptr, type, member) \
    ({ typeof(ptr) ____ptr = (ptr); \
        ____ptr ? rb_entry(____ptr, type, member) : nullptr; \
    })

/**
 * rbtree_postorder_for_each_entry_safe - iterate over rb_root in post order of
 * given type safe against removal of rb_node entry
 *
 * @pos:	the 'type *' to use as a loop cursor.
 * @n:		another 'type *' to use as temporary storage
 * @root:	'rb_root *' of the rbtree.
 * @field:	the name of the rb_node field within 'type'.
 */
#define rbtree_postorder_for_each_entry_safe(pos, n, root, field) \
    for (pos = rb_entry_safe(rb_first_postorder(root), typeof(*pos), field); \
         pos && ({ n = rb_entry_safe(rb_next_postorder(&pos->field), \
            typeof(*pos), field); 1; }); \
         pos = n)

//-----------------------------------------------------------------------------
#define RB_DECLARE_CALLBACKS( \
    rbstatic, rbname, rbstruct, rbfield, rbtype, rbaugmented, rbcompute) \
static inline void \
rbname ## _propagate(rb_node *rb, rb_node *stop) \
{ \
    while (rb != stop) { \
        rbstruct *node = rb_entry(rb, rbstruct, rbfield); \
        rbtype augmented = rbcompute(node); \
        if (node->rbaugmented == augmented) \
            break; \
        node->rbaugmented = augmented; \
        rb = node->rbfield.rb_parent(); \
    }\
}\
static inline void\
rbname ## _copy(rb_node *rb_old, rb_node *rb_new) \
{ \
    rbstruct *old_node = rb_entry(rb_old, rbstruct, rbfield); \
    rbstruct *new_node = rb_entry(rb_new, rbstruct, rbfield); \
    new_node->rbaugmented = old_node->rbaugmented;\
} \
static void \
rbname ## _rotate(rb_node *rb_old, rb_node *rb_new) \
{ \
    rbstruct *old_node = rb_entry(rb_old, rbstruct, rbfield); \
    rbstruct *new_node = rb_entry(rb_new, rbstruct, rbfield); \
    new_node->rbaugmented = old_node->rbaugmented; \
    old_node->rbaugmented = rbcompute(old_node); \
} \
rbstatic const struct rb_augment_callbacks rbname = { \
    rbname ## _propagate, rbname ## _copy, rbname ## _rotate \
};

//ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ
struct rb_node
{
    public: enum
    {
        RB_RED   = 0,
        RB_BLACK = 1,
    };

    //-------------------------------------------------------------------------
    public: template<typename template_value>
    static rb_node const* find(
        rb_node const* const in_root,
        template_value const& in_key)
    {
        rb_node const* local_node(in_root);
        while (local_node != nullptr)
        {
            int const local_compare(
                template_value::compare_rb_node(in_key, *local_node));
            if (local_compare < 0)
            {
                local_node = local_node->rb_left;
            }
            else if (0 < local_compare)
            {
                local_node = local_node->rb_right;
            }
            else
            {
                return local_node;
            }
        }
        return nullptr;
    }

    public: template<typename template_value>
    static bool insert(
        rb_node*& io_root,
        template_value& io_value)
    {
        rb_node** local_node_position(&io_root);
        rb_node* local_parent(nullptr);

        // Figure out where to put new node.
        while (*local_node_position != nullptr)
        {
            local_parent = *local_node_position;
            int const local_compare(
                template_value::compare_rb_node(io_value, *local_parent));
            if (local_compare < 0)
            {
                local_node_position = &((**local_node_position).rb_left);
            }
            else if (0 < local_compare)
            {
                local_node_position = &((**local_node_position).rb_right);
            }
            else
            {
                return false;
            }
        }

        // Add new node and rebalance tree.
        auto& local_node(template_value::get_rb_node(io_value));
        local_node.rb_link_node(local_parent, *local_node_position);
        rb_node::rb_insert_color(local_node, io_root);
        return true;
    }

    public: template<typename template_value>
    static rb_node* erase(
        rb_node* const io_root,
        template_value const& in_key)
    {
        auto const local_node(
            const_cast<rb_node*>(rb_node::find(io_root, in_key)));
        if (local_node == nullptr)
        {
            return nullptr;
        }
        rb_node::rb_erase(local_node, io_root);
        return local_node;
    }

    //-------------------------------------------------------------------------
    public: PSYQ_CONSTEXPR rb_node() PSYQ_NOEXCEPT: 
        _rb_parent_color(reinterpret_cast<unsigned long>(this)),
        rb_left(nullptr),
        rb_right(nullptr)
    {}

    //-------------------------------------------------------------------------
    /*  Find logical next and previous nodes in a tree
        木の、論理的な次と前のノードを取得する。
     */
    public: rb_node const* rb_next() const
    {
        if (this->RB_EMPTY_NODE())
        {
            return nullptr;
        }

        // If we have a right-hand child,
        // go down and then left as far as we can.
        // 下位ノード右側があるなら、下位ノード右側の左端を手繰る。
        if (this->rb_right != nullptr)
        {
            return this->rb_right->find_leftmost();
        }

        // No right-hand children. Everything down and left is smaller than us,
        // so any 'next' node must be in the general direction of our parent.
        // Go up the tree; any time the ancestor is a right-hand child of its
        // parent, keep going up. First time it's a left-hand child of its
        // parent, said parent is our 'next' node.
        auto local_node(this);
        for (;;)
        {
            auto const local_parent(local_node->rb_parent());
            if (local_parent == nullptr || local_node != local_parent->rb_right)
            {
                return local_parent;
            }
            local_node = local_parent;
        }
    }

    public: rb_node const* rb_prev() const
    {
        if (this->RB_EMPTY_NODE())
        {
            return nullptr;
        }

        // If we have a left-hand child,
        // go down and then right as far as we can.
        // 下位ノード左側があるなら、下位ノード左側の右端を手繰る。
        if (this->rb_left != nullptr)
        {
            return this->rb_left->find_rightmost();
        }

        // No left-hand children. Go up till we find an ancestor which
        // is a right-hand child of its parent.
        auto local_node(this);
        for (;;)
        {
            auto const local_parent(local_node->rb_parent());
            if (local_parent == nullptr || local_node != local_parent->rb_left)
            {
                return local_parent;
            }
            local_node = local_parent;
        }
    }

    // Postorder iteration - always visit the parent after its children.
    public: static rb_node const* rb_next_postorder(rb_node const* const in_node)
    {
        if (in_node == nullptr)
        {
            return nullptr;
        }
        auto const local_parent(in_node->rb_parent());

        // If we're sitting on node, we've already seen our children.
        if (local_parent != nullptr
            && in_node == local_parent->rb_left
            && local_parent->rb_right != nullptr)
        {
            // If we are the parent's left node, go to the parent's right
            // node then all the way down to the left.
            return local_parent->rb_right->rb_left_deepest_node();
        }
        else
        {
            // Otherwise we are the parent's right node,
            // and the parent should be next
            return local_parent;
        }
    }

    //-------------------------------------------------------------------------
    private: rb_node const* rb_parent() const
    {
        return rb_node::_rb_parent(this->_rb_parent_color);
    }
    private: rb_node* rb_parent()
    {
        return rb_node::_rb_parent(this->_rb_parent_color);
    }
    private: rb_node* rb_red_parent()
    {
        return reinterpret_cast<rb_node*>(this->_rb_parent_color);
    }
    private: static rb_node* _rb_parent(unsigned long const in_parent_color)
    {
        return reinterpret_cast<rb_node*>(in_parent_color & ~3);
    }

    public: unsigned long rb_color() const
    {
        return rb_node::_rb_color(this->_rb_parent_color);
    }
    private: static unsigned long _rb_color(unsigned long const in_parent_color)
    {
        return in_parent_color & 1;
    }

    public: bool rb_is_black() const
    {
        return rb_node::_rb_is_black(this->_rb_parent_color);
    }
    private: static bool _rb_is_black(unsigned long const in_parent_color)
    {
        return rb_node::_rb_color(in_parent_color) != 0;
    }

    public: bool rb_is_red() const
    {
        return rb_node::_rb_is_red(this->_rb_parent_color);
    }
    private: static bool _rb_is_red(unsigned long const in_parent_color)
    {
        return rb_node::_rb_color(in_parent_color) == 0;
    }

    // 'empty' nodes are nodes that are known not to be inserted in an rbree.
    public: bool RB_EMPTY_NODE() const
    {
        return this->_rb_parent_color == reinterpret_cast<unsigned long>(this);
    }
    public: unsigned long RB_CLEAR_NODE()
    {
        this->_rb_parent_color = reinterpret_cast<unsigned long>(this);
        return this->_rb_parent_color;
    }

    public: void rb_set_parent(rb_node* const in_parent)
    {
        this->_rb_parent_color =
            this->rb_color() | reinterpret_cast<unsigned long>(in_parent);
    }

    public: void rb_set_parent_color(rb_node* const in_parent, int const in_color)
    {
        this->_rb_parent_color =
            reinterpret_cast<unsigned long>(in_parent) | in_color;
    }

    public: void rb_link_node(rb_node* const in_parent, rb_node*& out_link)
    {
        this->_rb_parent_color = reinterpret_cast<unsigned long>(in_parent);
        this->rb_left = this->rb_right = nullptr;
        out_link = this;
    }

    //-------------------------------------------------------------------------
    private: rb_node const* find_leftmost() const
    {
        auto local_node(this);
        while (local_node->rb_left != nullptr)
        {
            local_node = local_node->rb_left;
        }
        return local_node;
    }

    private: rb_node const* find_rightmost() const
    {
        auto local_node(this);
        while (local_node->rb_right != nullptr)
        {
            local_node = local_node->rb_right;
        }
        return local_node;
    }

    /*  red-black trees properties:  http://en.wikipedia.org/wiki/Rbtree

         1) A node is either red or black
         2) The root is black
         3) All leaves (nullptr) are black
         4) Both children of every red node are black
         5) Every simple path from root to leaves contains the same number
            of black nodes.

         4 and 5 give the O(log n) guarantee, since 4 implies you cannot have two
         consecutive red nodes in a path and every red node is therefore followed by
         a black. So if B is the number of black nodes on every simple path (as per
         5), then the longest possible path due to 4 is 2B.

         We shall indicate color with case, where black nodes are uppercase and red
         nodes will be lowercase. Unknown color nodes shall be drawn as red within
         parentheses and have some accompanying text comment.
     */

    private: void rb_set_black()
    {
        this->_rb_parent_color |= RB_BLACK;
    }

    private: rb_node const* rb_left_deepest_node() const
    {
        auto local_node(this);
        for (;;)
        {
            if (local_node->rb_left != nullptr)
            {
                local_node = local_node->rb_left;
            }
            else if (local_node->rb_right != nullptr)
            {
                local_node = local_node->rb_right;
            }
            else
            {
                return local_node;
            }
        }
    }

    /*  Please note - only struct rb_augment_callbacks and the prototypes for
        rb_insert_augmented() and rb_erase_augmented() are intended to be public.
        The rest are implementation details you are not expected to depend on.

        See Documentation/rbtree.txt for documentation and samples.
     */

    public: struct rb_augment_callbacks
    {
        void (*propagate)(rb_node* const node, rb_node* const stop);
        void (*copy)(rb_node* const old_node, rb_node* const new_node);
        void (*rotate)(rb_node* const old_node, rb_node* const new_node);
    };

    //-------------------------------------------------------------------------
    public: static bool RB_EMPTY_ROOT(rb_node const* const in_root)
    {
        return in_root == nullptr;
    }

    // This function returns the first node (in sort order) of the tree.
    public: static rb_node const* rb_first(rb_node const* const in_root)
    {
        return rb_node::RB_EMPTY_ROOT(in_root)?
            nullptr: in_root->find_leftmost();
    }

    public: static rb_node const* rb_last(rb_node const* const in_root)
    {
        return rb_node::RB_EMPTY_ROOT(in_root)?
            nullptr: in_root->find_rightmost();
    }

    public: static rb_node const* rb_first_postorder(rb_node const* const in_root)
    {
        return rb_node::RB_EMPTY_ROOT(in_root)?
            nullptr: in_root->rb_left_deepest_node();
    }

    //-------------------------------------------------------------------------
    public: static void rb_insert_color(rb_node& io_node, rb_node*& io_root)
    {
        rb_node::rb_insert_augmented(
            io_node,
            io_root,
            [](rb_node* const old_node, rb_node* const new_node) {});
    }

    public: static void rb_erase(rb_node& io_node, rb_node*& io_root)
    {
        rb_node::rb_erase_augmented(
            io_node,
            io_root,
            [](rb_node* const node, rb_node* const stop) {},
            [](rb_node* const old_node, rb_node* const new_node) {},
            [](rb_node* const old_node, rb_node* const new_node) {});
    }

    // Fast replacement of a single node without remove/rebalance/add/rebalance.
    public: static void rb_replace_node(
        rb_node& in_old_node,
        rb_node& io_new_node,
        rb_node*& io_root)
    {
        // set the surrounding nodes to point to the replacement.
        rb_node::_rb_change_child(
            &in_old_node, &io_new_node, in_old_node.rb_parent(), io_root);
        if (in_old_node.rb_left != nullptr)
        {
            in_old_node.rb_left->rb_set_parent(&io_new_node);
        }
        if (in_old_node.rb_right != nullptr)
        {
            in_old_node.rb_right->rb_set_parent(&io_new_node);
        }

        // copy the pointers/colour from the victim to the replacement.
        io_new_node = in_old_node;
    }

    public: template<typename template_rotate>
    static void rb_insert_augmented(
        rb_node& io_node,
        rb_node*& io_root,
        template_rotate in_augment_rotate)
    {
        rb_node::_rb_insert(io_node, io_root, std::move(in_augment_rotate));
    }

    public: template<
        typename template_propage,
        typename template_copy,
        typename template_rotate>
    static void rb_erase_augmented(
        rb_node& io_node,
        rb_node*& io_root,
        template_propage in_augment_propagate,
        template_copy in_augment_copy,
        template_rotate in_augment_rotate)
    {
        rb_node* const local_rebalance(
            rb_node::_rb_erase_augmented(
                io_node,
                io_root,
                std::move(in_augment_propagate),
                std::move(in_augment_copy)));
        if (local_rebalance != nullptr)
        {
            rb_node::_rb_erase_color(
                *local_rebalance, io_root, std::move(in_augment_rotate));
        }
    }

    //-------------------------------------------------------------------------
    private: static void _rb_change_child(
        rb_node const* const in_old_node,
        rb_node* const in_new_node,
        rb_node* const io_parent,
        rb_node*& io_root)
    {
        if (io_parent == nullptr)
        {
            io_root = in_new_node;
        }
        else if (io_parent->rb_left == in_old_node)
        {
            io_parent->rb_left = in_new_node;
        }
        else
        {
            io_parent->rb_right = in_new_node;
        }
    }

    /*  Helper function for rotations:
        - old's parent and color get assigned to new
        - old gets assigned new as a parent and 'color' as a color.
     */
    private: static void _rb_rotate_set_parents(
        rb_node& io_old_node,
        rb_node& io_new_node,
        rb_node*& io_root,
        int const in_color)
    {
        auto const local_parent(io_old_node.rb_parent());
        io_new_node._rb_parent_color = io_old_node._rb_parent_color;
        io_old_node.rb_set_parent_color(&io_new_node, in_color);
        rb_node::_rb_change_child(&io_old_node, &io_new_node, local_parent, io_root);
    }

    //-------------------------------------------------------------------------
    private: template<typename template_rotate>
    static void _rb_insert(
        rb_node& io_node,
        rb_node*& io_root,
        template_rotate in_augment_rotate)
    {
        rb_node* local_node(&io_node);
        rb_node* local_parent(io_node.rb_red_parent());
        for (;;)
        {
            /*  Loop invariant: node is red

                If there is a black parent, we are done.
                Otherwise, take some corrective action as we don't
                want a red root or two consecutive red nodes.
             */
            if (local_parent == nullptr)
            {
                local_node->rb_set_parent_color(nullptr, rb_node::RB_BLACK);
                break;
            }
            else if (local_parent->rb_is_black())
            {
                break;
            }

            rb_node* const local_gparent(local_parent->rb_red_parent());
            rb_node* const local_gparent_right(local_gparent->rb_right);
            if (local_parent != local_gparent_right) // local_parent == local_gparent->rb_left
            {
                if (local_gparent_right != nullptr
                    && local_gparent_right->rb_is_red())
                {
                    /*  Case 1 - color flips

                              G            g
                             / \          / \
                            p   u  -->   P   U
                           /            /
                          n            N

                        However, since g's parent might be red, and
                        4) does not allow this, we need to recurse
                        at g.
                     */
                    local_gparent_right->rb_set_parent_color(
                        local_gparent, rb_node::RB_BLACK);
                    local_parent->rb_set_parent_color(
                        local_gparent, rb_node::RB_BLACK);
                    local_node = local_gparent;
                    local_parent = local_node->rb_parent();
                    local_node->rb_set_parent_color(
                        local_parent, rb_node::RB_RED);
                    continue;
                }

                rb_node* local_parent_right(local_parent->rb_right);
                if (local_node == local_parent_right)
                {
                    /*  Case 2 - left rotate at parent

                             G             G
                            / \           / \
                           p   U  -->    n   U
                            \           /
                             n         p

                        This still leaves us in violation of 4), the
                        continuation into Case 3 will fix that.
                     */
                    local_parent_right = local_node->rb_left;
                    local_parent->rb_right = local_node->rb_left;
                    local_node->rb_left = local_parent;
                    if (local_parent_right != nullptr)
                    {
                        local_parent_right->rb_set_parent_color(
                            local_parent, rb_node::RB_BLACK);
                    }
                    local_parent->rb_set_parent_color(
                        local_node, rb_node::RB_RED);
                    in_augment_rotate(local_parent, local_node);
                    local_parent = local_node;
                    local_parent_right = local_node->rb_right;
                }

                /*  Case 3 - right rotate at local_gparent

                           G           P
                          / \         / \
                         p   U  -->  n   g
                        /                 \
                       n                   U
                 */
                local_gparent->rb_left = local_parent_right; // == local_parent->rb_right
                local_parent->rb_right = local_gparent;
                if (local_parent_right != nullptr)
                {
                    local_parent_right->rb_set_parent_color(
                        local_gparent, rb_node::RB_BLACK);
                }
                rb_node::_rb_rotate_set_parents(
                    *local_gparent, *local_parent, io_root, rb_node::RB_RED);
                in_augment_rotate(local_gparent, local_parent);
                break;
            }
            else
            {
                rb_node* const local_gparent_left(local_gparent->rb_left);
                if (local_gparent_left != nullptr
                    && local_gparent_left->rb_is_red())
                {
                    // Case 1 - color flips
                    local_gparent_left->rb_set_parent_color(
                        local_gparent, rb_node::RB_BLACK);
                    local_parent->rb_set_parent_color(
                        local_gparent, rb_node::RB_BLACK);
                    local_node = local_gparent;
                    local_parent = local_node->rb_parent();
                    local_node->rb_set_parent_color(
                        local_parent, rb_node::RB_RED);
                    continue;
                }

                rb_node* local_parent_left(local_parent->rb_left);
                if (local_node == local_parent_left)
                {
                    // Case 2 - right rotate at parent
                    local_parent_left = local_node->rb_right;
                    local_parent->rb_left = local_node->rb_right;
                    local_node->rb_right = local_parent;
                    if (local_parent_left != nullptr)
                    {
                        local_parent_left->rb_set_parent_color(
                            local_parent, rb_node::RB_BLACK);
                    }
                    local_parent->rb_set_parent_color(local_node, rb_node::RB_RED);
                    in_augment_rotate(local_parent, local_node);
                    local_parent = local_node;
                    local_parent_left = local_node->rb_left;
                }

                // Case 3 - left rotate at local_gparent
                local_gparent->rb_right = local_parent_left; // == local_parent->rb_left
                local_parent->rb_left = local_gparent;
                if (local_parent_left != nullptr)
                {
                    local_parent_left->rb_set_parent_color(local_gparent, rb_node::RB_BLACK);
                }
                rb_node::_rb_rotate_set_parents(
                    *local_gparent, *local_parent, io_root, rb_node::RB_RED);
                in_augment_rotate(local_gparent, local_parent);
                break;
            }
        }
    }

    //-------------------------------------------------------------------------
    private: template<typename template_rotate>
    static void _rb_erase_color(
        rb_node& io_parent,
        rb_node*& io_root,
        template_rotate in_augment_rotate)
    {
        rb_node* local_parent(&io_parent);
        rb_node* local_node(nullptr);
        for (;;)
        {
            /*  Loop invariants:
                - local_node is black (or nullptr on first iteration)
                - local_node is not the root (local_parent is not nullptr)
                - All leaf paths going through local_parent and local_node have
                  a black node count that is 1 lower than other leaf paths.

                ループ不変条件:
                - local_node は黒。ただし、ループの最初のみnullptr。
                - local_node は最上位ノードでない。
                  つまり local_parent は、nullptrにならない。
                - local_parent と local_node を通過するすべての葉のパスは、
                  他の葉の経路よりも1つ少ない黒のノード数を持っている。
             */
            rb_node* local_sibling(local_parent->rb_right);
            if (local_node != local_sibling) // local_node == local_parent->rb_left
            {
                if (local_sibling->rb_is_red())
                {
                    /*  Case 1 - left rotate at parent.
                        ケース1 - 親ノードを左回転する。

                            P               S
                           / \             / \
                          N   s    -->    p   Sr
                             / \         / \
                            Sl  Sr      N   Sl
                     */
                    rb_node* const local_sibling_left(local_sibling->rb_left);
                    local_parent->rb_right = local_sibling_left;
                    local_sibling->rb_left = local_parent;
                    local_sibling_left->rb_set_parent_color(
                        local_parent, rb_node::RB_BLACK);
                    rb_node::_rb_rotate_set_parents(
                        *local_parent, *local_sibling, io_root, rb_node::RB_RED);
                    in_augment_rotate(local_parent, local_sibling);
                    local_sibling = local_sibling_left;
                }
                rb_node* local_sibling_right(local_sibling->rb_right);
                if (local_sibling_right == nullptr
                    || local_sibling_right->rb_is_black())
                {
                    rb_node* const local_sibling_left(local_sibling->rb_left);
                    if (local_sibling_left == nullptr
                        || local_sibling_left->rb_is_black())
                    {
                        /*  Case 2 - sibling color flip
                            (p could be either color here)
                            ケース2 - 隣接ノードの色を反転する。
                            (ノードpは、ここではいずれかの色かもしれない)

                               (p)           (p)
                               / \           / \
                              N   S    -->  N   s
                                 / \           / \
                                Sl  Sr        Sl  Sr

                            This leaves us violating 5) which
                            can be fixed by flipping p to black
                            if it was red, or by recursing at p.
                            p is red when coming from Case 1.
                            ノードpが赤だった場合に黒へ反転するか、
                            ノードpで再帰することで、5)の違反を修正できる。
                            ケース1から来るときは、ノードpは赤である。
                         */
                        local_sibling->rb_set_parent_color(
                            local_parent, rb_node::RB_RED);
                        if (local_parent->rb_is_red())
                        {
                            local_parent->rb_set_black();
                        }
                        else
                        {
                            local_node = local_parent;
                            local_parent = local_node->rb_parent();
                            if (local_parent != nullptr)
                            {
                                continue;
                            }
                        }
                        break;
                    }
                    /*  Case 3 - right rotate at sibling
                        (p could be either color here)
                        ケース3 - 隣接ノードを右回転する。
                        (ノードpは、ここではいずれかの色かもしれない)

                          (p)           (p)
                          / \           / \
                         N   S    -->  N   Sl
                            / \             \
                           sl  Sr            s
                                              \
                                               Sr
                     */
                    local_sibling_right = local_sibling_left->rb_right;
                    local_sibling->rb_left = local_sibling_right;
                    local_sibling_left->rb_right = local_sibling;
                    local_parent->rb_right = local_sibling_left;
                    if (local_sibling_right != nullptr)
                    {
                        local_sibling_right->rb_set_parent_color(
                            local_sibling, rb_node::RB_BLACK);
                    }
                    in_augment_rotate(local_sibling, local_sibling_left);
                    local_sibling_right = local_sibling;
                    local_sibling = local_sibling_left;
                }
                /*  Case 4 - left rotate at parent + color flips
                    (p and sl could be either color here.
                    After rotation, p becomes black, s acquires
                    p's color, and sl keeps its color)
                    ケース4 - 親ノードを左回転し、色を反転する。
                    ノードpとノードslは、ここではいずれかの色かもしれない。
                    回転した後、ノードpの色は黒となり、ノードsはpの色となる。
                    ノードslの色は不変。

                         (p)             (s)
                         / \             / \
                        N   S     -->   P   Sr
                           / \         / \
                         (sl) sr      N  (sl)
                 */
                rb_node* const local_sibling_reft(local_sibling->rb_left);
                local_parent->rb_right = local_sibling_reft;
                local_sibling->rb_left = local_parent;
                local_sibling_right->rb_set_parent_color(
                    local_sibling, rb_node::RB_BLACK);
                if (local_sibling_reft != nullptr)
                {
                    local_sibling_reft->rb_set_parent(local_parent);
                }
                rb_node::_rb_rotate_set_parents(
                    *local_parent, *local_sibling, io_root, rb_node::RB_BLACK);
                in_augment_rotate(local_parent, local_sibling);
                break;
            }
            else
            {
                local_sibling = local_parent->rb_left;
                if (local_sibling->rb_is_red())
                {
                    // Case 1 - right rotate at parent.
                    rb_node* const local_sibling_right(local_sibling->rb_right);
                    local_parent->rb_left = local_sibling_right;
                    local_sibling->rb_right = local_parent;
                    local_sibling_right->rb_set_parent_color(
                        local_parent, rb_node::RB_BLACK);
                    rb_node::_rb_rotate_set_parents(
                        *local_parent, *local_sibling, io_root, rb_node::RB_RED);
                    in_augment_rotate(local_parent, local_sibling);
                    local_sibling = local_sibling_right;
                }
                rb_node* local_sibling_left(local_sibling->rb_left);
                if (local_sibling_left == nullptr
                    || local_sibling_left->rb_is_black())
                {
                    rb_node* const local_sibling_right(local_sibling->rb_right);
                    if (local_sibling_right == nullptr
                        || local_sibling_right->rb_is_black())
                    {
                        // Case 2 - sibling color flip.
                        local_sibling->rb_set_parent_color(
                            local_parent, rb_node::RB_RED);
                        if (local_parent->rb_is_red())
                        {
                            local_parent->rb_set_black();
                        }
                        else
                        {
                            local_node = local_parent;
                            local_parent = local_node->rb_parent();
                            if (local_parent != nullptr)
                            {
                                continue;
                            }
                        }
                        break;
                    }
                    // Case 3 - right rotate at sibling.
                    local_sibling_left = local_sibling_right->rb_left;
                    local_sibling->rb_right = local_sibling_left;
                    local_sibling_right->rb_left = local_sibling;
                    local_parent->rb_left = local_sibling_right;
                    if (local_sibling_left != nullptr)
                    {
                        local_sibling_left->rb_set_parent_color(
                            local_sibling, rb_node::RB_BLACK);
                    }
                    in_augment_rotate(local_sibling, local_sibling_right);
                    local_sibling_left = local_sibling;
                    local_sibling = local_sibling_right;
                }
                // Case 4 - left rotate at parent + color flips.
                rb_node* const local_sibling_right(local_sibling->rb_right);
                local_parent->rb_left = local_sibling_right;
                local_sibling->rb_right = local_parent;
                local_sibling_left->rb_set_parent_color(
                    local_sibling, rb_node::RB_BLACK);
                if (local_sibling_right != nullptr)
                {
                    local_sibling_right->rb_set_parent(local_parent);
                }
                rb_node::_rb_rotate_set_parents(
                    *local_parent, *local_sibling, io_root, rb_node::RB_BLACK);
                in_augment_rotate(local_parent, local_sibling);
                break;
            }
        }
    }

    //-------------------------------------------------------------------------
    private: template<typename template_propagete, typename template_copy>
    static rb_node* _rb_erase_augmented(
        rb_node& io_node,
        rb_node*& io_root,
        template_propagete in_augment_propagate,
        template_copy in_augment_copy)
    {
        rb_node* const local_child(io_node.rb_right);
        rb_node* local_tmp(io_node.rb_left);
        rb_node* local_rebalance;

        if (local_tmp == nullptr)
        {
            /*  Case 1: io_node to erase has no more than 1 child (easy!)

                Note that if there is one child it must be red due to 5)
                and io_node must be black due to 4). We adjust colors locally
                so as to bypass _rb_erase_color() later on.

                ケース1a: io_node の下位ノードが1つ以下(簡単!)。

                右側下位ノードがある場合、下位ノードは5)に起因して赤となり、
                io_node は4)に起因して黒となる必要がある。
                _rb_erase_color() を後ほど経由させ、色を調整すること。
             */
            unsigned long const local_parent_color(io_node._rb_parent_color);
            rb_node* const local_parent(
                rb_node::_rb_parent(local_parent_color));
            rb_node::_rb_change_child(
                &io_node, local_child, local_parent, io_root);
            if (local_child != nullptr)
            {
                local_child->_rb_parent_color = local_parent_color;
                local_rebalance = nullptr;
            }
            else
            {
                local_rebalance = rb_node::_rb_is_black(local_parent_color)?
                    local_parent: nullptr;
            }
            local_tmp = local_parent;
        }
        else if (local_child == nullptr)
        {
            // Still case 1, but this time the child is io_node.rb_left.
            // ケース1b: io_node の左側下位ノードの1つだけ。
            unsigned long const local_parent_color(io_node._rb_parent_color);
            local_tmp->_rb_parent_color = local_parent_color;
            rb_node* const local_parent(
                rb_node::_rb_parent(local_parent_color));
            rb_node::_rb_change_child(
                &io_node, local_tmp, local_parent, io_root);
            local_rebalance = nullptr;
            local_tmp = local_parent;
        }
        else
        {
            rb_node* local_successor(local_child);
            rb_node* local_successor_child;
            rb_node* local_parent;
            local_tmp = local_child->rb_left;
            if (local_tmp == nullptr)
            {
                /*  Case 2: io_node's successor is its right child.
                    ケース2: io_node を引き継ぐのは、右側下位ノード。

                       (n)           (s)
                       / \           / \
                     (x) (s)  -->  (x) (c)
                           \
                           (c)
                 */
                local_parent = local_successor;
                local_successor_child = local_successor->rb_right;
                in_augment_copy(&io_node, local_successor);
            }
            else
            {
                /*  Case 3: io_node's successor is leftmost under
                    io_node's right child subtree
                    ケース3: io_node を引き継ぐのは、
                    io_node の右側下位ツリーの最左端ノード。

                       (n)           (s)
                       / \           / \
                     (x) (y)  -->  (x) (y)
                         /             /
                       (p)           (p)
                       /             /
                     (s)           (c)
                       \
                       (c)
                 */
                for (;;)
                {
                    local_parent = local_successor;
                    local_successor = local_tmp;
                    local_tmp = local_tmp->rb_left;
                    if (local_tmp == nullptr)
                    {
                        break;
                    }
                }
                local_successor_child = local_successor->rb_right;
                local_parent->rb_left = local_successor_child;
                local_successor->rb_right = local_child;
                local_child->rb_set_parent(local_successor);
                in_augment_copy(&io_node, local_successor);
                in_augment_propagate(local_parent, local_successor);
            }

            local_tmp = io_node.rb_left;
            local_successor->rb_left = local_tmp;
            local_tmp->rb_set_parent(local_successor);

            unsigned long const local_parent_color(io_node._rb_parent_color);
            local_tmp = rb_node::_rb_parent(local_parent_color);
            rb_node::_rb_change_child(
                &io_node, local_successor, local_tmp, io_root);
            if (local_successor_child != nullptr)
            {
                local_successor->_rb_parent_color = local_parent_color;
                local_successor_child->rb_set_parent_color(
                    local_parent, rb_node::RB_BLACK);
                local_rebalance = nullptr;
            }
            else
            {
                unsigned long const local_successor_parent_color(
                    local_successor->_rb_parent_color);
                local_successor->_rb_parent_color = local_parent_color;
                local_rebalance =
                    rb_node::_rb_is_black(local_successor_parent_color)?
                        local_parent: nullptr;
            }
            local_tmp = local_successor;
        }

        in_augment_propagate(local_tmp, nullptr);
        return local_rebalance;
    }

    //-------------------------------------------------------------------------
    /*  Non-augmented rbtree manipulation functions.

        We use dummy augmented callbacks here, and have the compiler optimize them
        out of the rb_insert_color() and rb_erase() function definitions.
     */

    private: static void dummy_propagate(rb_node* const node, rb_node* const stop) {}
    private: static void dummy_copy(rb_node* const old_node, rb_node* const new_node) {}
    private: static void dummy_rotate(rb_node* const old_node, rb_node* const new_node) {}

    //-------------------------------------------------------------------------
    private: unsigned long _rb_parent_color;
    private: rb_node* rb_right;
    private: rb_node* rb_left;
};
static_assert(sizeof(rb_node*) <= sizeof(unsigned long), "");
static_assert(4 <= std::alignment_of<rb_node>::value, "");

#endif // !defined(PSYQ_RBTREE_HPP_)
