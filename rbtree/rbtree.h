#ifndef __RBTREE_H
#define __RBTREE_H

#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>

struct rbnode {
    struct rbnode *parent;
    struct rbnode *left;
    struct rbnode *right;
    int color;
    int val;
};

struct rbroot {
    struct rbnode *node;
};

#define RB_RED      0x0
#define RB_BLACK    0x1

static inline bool rb_is_black(struct rbnode *node)
{
    // NIL node is black
    return node ? node->color == RB_BLACK : true;
}

static inline bool rb_is_red(struct rbnode *node)
{
    // NIL node is black
    return node ? node->color == RB_RED : false;
}

static inline void rb_set_black(struct rbnode *node)
{
    assert(node);
    node->color = RB_BLACK;
}

static inline void rb_set_red(struct rbnode *node)
{
    assert(node);
    node->color = RB_RED;
}

static inline struct rbnode *rb_alloc_node()
{
    struct rbnode *node = (struct rbnode *)malloc(sizeof(struct rbnode));
    assert(node);
    return node;
}

static inline void rb_free_node(struct rbnode *node)
{
    assert(node);
    free(node);
}

static inline struct rbnode *rb_parent(struct rbnode *node)
{
    assert(node);
    return node->parent;
}

static inline struct rbnode *rb_grandparent(struct rbnode *node)
{
    assert(node);
    assert(rb_parent(node));
    return rb_parent(rb_parent(node));
}

static inline struct rbnode *rb_uncle(struct rbnode *node)
{
    assert(node);
    assert(rb_grandparent(node));
    return rb_parent(node) == rb_grandparent(node)->left ? rb_grandparent(node)->right : rb_grandparent(node)->left;
}

/*
 * rotate left
 * n is the pivot, meaning that it becomes the new parent node after rotating
 *
 *    a             a
 *    |             |
 *    b      -->    n 
 *   / \           / \
 *  x   n         b   z
 *     / \       / \
 *    y   z     x   y
 *
 * need to change three pairs of pointer
 */
static inline void rb_rotate_left(struct rbnode *n, struct rbroot *root)
{
    assert(n);
    assert(root);
    struct rbnode *b = rb_parent(n);
    assert(b);
    assert(b->right == n);
    struct rbnode *a = rb_parent(b);
    struct rbnode *y = n->left;

    // b->right = n --> b->right = y
    b->right = y;
    if (y) {
        y->parent = b;
    }

    // a --- b --> a --- n
    // --- means a->left, a->right or root->node
    struct rbnode **link;
    if (a) {
        link = a->left == b ? &(a->left) : &(a->right);
    } else {
        link = &(root->node);
    }
    *link = n;
    n->parent = a;

    // n->left = y --> n->left = b
    n->left = b;
    b->parent = n;
}

/*
 * rotate right
 *
 * n is the pivot, meaning that it becomes the new parent node after rotating
 *
 *     a             a
 *     |             |
 *     b      -->    n 
 *    / \           / \
 *   n   z         x   b
 *  / \               / \
 * x   y             y   z
 *
 * need to change three pairs of pointer
 */
static inline void rb_rotate_right(struct rbnode *n, struct rbroot *root)
{
    assert(n);
    assert(root);
    struct rbnode *b = rb_parent(n);
    assert(b);
    assert(b->left == n);
    struct rbnode *a = rb_parent(b);
    struct rbnode *y = n->right;

    // b->left = n --> b->left = y
    b->left = y;
    if (y) {
        y->parent = b;
    }

    // a --- b --> a --- n
    // --- means a->left, a->right or root->node
    struct rbnode **link;
    if (a) {
        link = a->left == b ? &(a->left) : &(a->right);
    } else {
        link = &(root->node);
    }
    *link = n;
    n->parent = a;

    // n->right = y --> n->right = b
    n->right = b;
    b->parent = n;
}

static inline struct rbnode *rb_predecessor(struct rbnode *node)
{
    assert(node && node->left);
    struct rbnode *predecessor = node->left;

    while (predecessor->right) {
        predecessor = predecessor->right;
    }

    return predecessor;
}

static inline struct rbnode *rb_successor(struct rbnode *node)
{
    assert(node && node->right);
    struct rbnode *successor = node->right;

    while (successor->left) {
        successor = successor->left;
    }

    return successor;
}

extern void rb_link_node(struct rbnode *node, struct rbnode *parent, struct rbnode **rblink);
extern void rb_insert_balance(struct rbnode *n, struct rbroot *root);
extern void rb_erase(struct rbnode *n, struct rbroot *root);

#endif