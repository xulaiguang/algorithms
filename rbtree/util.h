#ifndef __RBTREE_UTIL_H
#define __RBTREE_UTIL_H

#include "rbtree.h"

void print_tree(struct rbroot *root);

void rb_inorder_traverse(struct rbnode *x);

void is_rbtree(struct rbroot *root);

#endif