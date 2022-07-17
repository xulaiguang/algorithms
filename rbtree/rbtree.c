#include "rbtree.h"
#include <endian.h>

/*
 * red-black trees properties:  http://en.wikipedia.org/wiki/Rbtree
 *
 *  1) A node is either red or black
 *  2) The root is black
 *  3) All leaves (NULL) are black
 *  4) Both children of every red node are black
 *  5) Every simple path from root to leaves contains the same number
 *     of black nodes.
 *
 *  4 and 5 give the O(log n) guarantee, since 4 implies you cannot have two
 *  consecutive red nodes in a path and every red node is therefore followed by
 *  a black. So if B is the number of black nodes on every simple path (as per
 *  5), then the longest possible path due to 4 is 2B.
 *
 *  We shall indicate color with case, where black nodes are uppercase and red
 *  nodes will be lowercase. Unknown color nodes shall be drawn as red within
 *  parentheses and have some accompanying text comment.
 */

/*
 *         G
 *       |   |
 *       p  (u)
 *      / \
 *     n   s
 *        / \
 *       c   d
 *
 * n: node
 * p: node's parent
 * g: node's grandparent
 * u: node's uncle
 * s: node's sibling
 * c: node's close nephew
 * d: node's distant nephew
 *
 * lower case(n): red color
 * upper case(N): black color
 * (): can be either red or black
 * / : left child
 * \ : right child
 * | : can be either left or right
 */

/* 
 * link node to its parent.
 * parent can be NULL.
 * rblink can be &root->node
 * if parent != NULL, rblink points to &parent->left or &parent->right
 */
void rb_link_node(struct rbnode *node, struct rbnode *parent, struct rbnode **rblink)
{
    assert(node);
    assert(!parent || (parent && rblink && (rblink == &parent->left || rblink == &parent->right)));

    node->parent = parent;
    node->color = RB_RED;
    node->left = node->right = NULL;

    *rblink = node;
}

/*
 * balance rbtree after linking node n into root
 * node's color is red
 * node have already link to its parent
 */
void rb_insert_balance(struct rbnode *n, struct rbroot *root)
{
    struct rbnode *p = rb_parent(n);
    struct rbnode *g, *u;

    // loop invariant: node is red
    while (true) {
        // case 1
        // node is the root
        if (!p) {
            rb_set_black(n);
            break;
        }
    
        // case 2
        // parent is black
        if (rb_is_black(p)) {
            break;
        }
    
        // case 3
        // parent is red & uncle is red
        // flip colors
        // | means that the child can be left or right
        //     G            g
        //    | |          | |
        //    p u   -->    P U
        //    |            |
        //    n            n
        g = rb_grandparent(n);
        assert(g);
        u = rb_uncle(n);
        if (rb_is_red(u)) {
            rb_set_black(p);
            rb_set_black(u);
            rb_set_red(g);

            n = g;
            p = rb_parent(n);
            continue;
        }
    
        // case 4
        // uncle is black
        // node is on the different side of parent
        if (n == p->right && p == g->left) {
            // rotate left
            //     G              G
            //    / \            / \
            //   p   U   -->    n   U
            //    \            /
            //     n          p
            rb_rotate_left(n, root);

            n = n->left;
            p = rb_parent(n);
        } else if (n == p->left && p == g->right) {
            // rotate right
            //     G              G
            //    / \            / \
            //   U   p   -->    U   n
            //      /                \
            //     n                  p
            rb_rotate_right(n, root);

            n = n->right;
            p = rb_parent(n);
        }
    
        // case 5
        // u is black & node is on the same side of parent
        if (n == p->left && p == g->left) {
            // p rotate right
            //     G              P
            //    / \            / \
            //   p   U   -->    n   g
            //  /                    \
            // n                      U
            rb_rotate_right(p, root);
        } else {
            // p rotate left
            //     G                 P
            //    / \               / \
            //   U   p      -->    g   n
            //        \           / 
            //         n         U 
            rb_rotate_left(p, root);
        }
        rb_set_black(p);
        rb_set_red(g);

        break;
    }
}

// node has at most one child
static void rb_unlink_node(struct rbnode *node, struct rbroot *root)
{
    assert(node && root);
    assert(!(node->left && node->right));

    struct rbnode *child = node->left ? node->left : node->right;
    struct rbnode *p = rb_parent(node);

    if (child) {
        child->parent = p;
    }

    if (p) {
        struct rbnode **link = p->left == node ? &(p->left) : &(p->right);
        *link = child;
    } else {
        root->node = child;
    }
}

static void rb_replace_to_predecessor(struct rbnode *node, struct rbroot *root)
{
    assert(node->left && node->right);

    struct rbnode *predecessor = rb_predecessor(node);
    assert(predecessor);
    assert(predecessor->right == NULL);

    struct rbnode **link_to_node = NULL;
    struct rbnode *node_p = rb_parent(node);
    struct rbnode *predecessor_p = rb_parent(predecessor);
    struct rbnode *node_left = node->left;
    if (node_p) {
        link_to_node = node_p->left == node ? &(node_p->left) : &(node_p->right);
    } else {
        link_to_node = &(root->node);
    }
    // predecessor <-> parent
    *link_to_node = predecessor;
    predecessor->parent = node_p;

    // predecessor <-> right
    node->right->parent = predecessor;
    predecessor->right = node->right;

    // node <-> right
    node->right = NULL;
    // node <-> left
    node->left = predecessor->left;
    if (predecessor->left) {
        predecessor->left->parent = node;
    }

    // node <-> parent
    // predecessor <-> left
    if (predecessor == node_left) {
        /*
         *          node
         *         /
         *   predecessor
         */
        node->parent = predecessor;
        predecessor->left = node;
    } else {
        /*
         *          node
         *         /
         *  node_left
         *         \
         *         ...
         *           \
         *           predecessor
         */
        node->parent = predecessor_p;
        predecessor_p->right = node;

        predecessor->left = node_left;
        node_left->parent = predecessor;
    }

    // swap color of node and predecessor
    int node_color = node->color;
    node->color = predecessor->color;
    predecessor->color = node_color;
}

struct rbnode *rb_erase_node(struct rbnode *node, struct rbroot *root)
{
    assert(node && root);

    // node has two children, exchange it and its predecessor
    if (node->left && node->right) {
        rb_replace_to_predecessor(node, root);
    }

    // node has at most one child
    assert(!(node->left && node->right));

    struct rbnode *p = rb_parent(node);
    struct rbnode *c = node->left ? node->left : node->right;

    // unlink node, remove node from tree root
    rb_unlink_node(node, root);

    /*
     * simple 1
     *       P         P
     *       |   -->  
     *       n
     */
    if (rb_is_red(node)) {
        assert(node->left == NULL && node->right == NULL);
        assert(p);
        return NULL;
    }

    /*
     * simple 2
     *    (p)          (p)
     *     |            |
     *     N   -->      C
     *     |
     *     c
     *
     */
    if (rb_is_red(c)) {
        assert(c->left == NULL && c->right == NULL);
        rb_set_black(c);
        return NULL;
    }

    // node is black
    // node's children are black
    // need to rebalance
    assert(node->left == NULL && node->right == NULL);

    return p;
}

void rb_erase_balance(struct rbnode *p, struct rbroot *root)
{
    assert(p && root);
    struct rbnode *n = NULL;

    /*
     * Loop invariants:
     * - node is black (or NULL on first iteration)
     * - All leafs(NIL) paths going through parent and node have a
     *   black node count that is 1 lower than other leaf paths
     */
    while (true) {
        // case 1
        if (p == NULL) {
            break;
        }

        struct rbnode *s = p->left == n ? p->right : p->left;
        assert(s);
        struct rbnode *sc = p->left == n ? s->left : s->right;
        struct rbnode *sd = p->left == n ? s->right : s->left;

        /*
         * case 2
         * P, S, C, D are black
         *      P               P
         *    |   |           |   |
         *    N   S   --->    N   s
         *       | |             | |
         *       C D             C D
         */
        if (rb_is_black(p)
                && rb_is_black(s)
                && rb_is_black(sc)
                && rb_is_black(sd)) {
            rb_set_red(s);
            n = p;
            p = rb_parent(p);
            continue;
        }

        /*
         * case 3
         * s is red
         * due to property 4, P, C, D are black
         *
         *     P               S
         *    / \             /  \
         *   N   s    -->    p    D
         *      / \         / \
         *     C   D       D   C
         *
         *
         *     P               S
         *    / \             /  \
         *   s   N    -->    D    p
         *  / \                  / \
         * C   D                C   N
         *
         */
        if (rb_is_red(s)) {
            assert(rb_is_black(p) && rb_is_black(sc) && rb_is_black(sd));
            assert(sc && sd);

            rb_set_black(s);
            rb_set_red(p);

            if (p->left == n) {
                rb_rotate_left(s, root);
            } else {
                rb_rotate_right(s, root);
            }

            s = sc;

            if (p->left == n) {
                sc = s->left;
                sd = s->right;
            } else {
                sc = s->right;
                sd = s->left;
            }
        }

        /*
         * case 4
         * p is red, C, D are black
         * due to property 4, S is black
         *
         *      p               P
         *    |   |           |   |
         *    N   S   --->    N   s
         *       | |             | |
         *       C D             C D
         */
        if (rb_is_red(p)
                && rb_is_black(s)
                && rb_is_black(sc)
                && rb_is_black(sd)) {
            assert(s);
            rb_set_black(p);
            rb_set_red(s);
            break;
        }

        /*
         * case 5
         * c is red and D is black
         * due to property 4, S is black
         *
         *    (p)             (p)
         *    / \             / \
         *   N   S    -->    N   C
         *      / \               \
         *     c   D               s
         *                          \
         *                           D
         *
         *    (p)             (p)
         *    / \             / \
         *   S   N    -->    C   N
         *  / \             /
         * D   c           s
         *                /
         *               D
         *
         */
        if (rb_is_red(sc) && rb_is_black(sd)) {
            assert(rb_is_black(s));

            rb_set_red(s);
            rb_set_black(sc);
            if (p->left == n) {
                rb_rotate_right(sc, root);
            } else {
                rb_rotate_left(sc, root);
            }
            s = sc;
            if (p->left == n) {
                sc = s->left;
                sd = s->right;
            } else {
                sc = s->right;
                sd = s->left;
            }
        }

        /*
         * case 6
         * d is red
         * due to property 4, S is black
         *
         *      (p)*            (s)*
         *      / \             / \
         *     N   S    -->    P   D
         *        / \         / \
         *      (c)  d       N  (c)
         *  
         *  
         *      (p)*            (s)*
         *      / \             / \
         *     S   N    -->    D   P
         *    / \                 / \
         *   d  (c)             (c)  N
         *
         */
        assert(rb_is_black(s));
        assert(rb_is_red(sd));
        if (p->left == n) {
            rb_rotate_left(s, root);
        } else {
            rb_rotate_right(s, root);
        }

        s->color = p->color;
        rb_set_black(p);
        rb_set_black(sd);
        break;
    }
}

void rb_erase(struct rbnode *node, struct rbroot *root)
{
    struct rbnode *p = rb_erase_node(node, root);

    if (p)
        rb_erase_balance(p, root);
}