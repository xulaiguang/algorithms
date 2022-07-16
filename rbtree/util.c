#include "rbtree.h"
#include "util.h"

#include <stdio.h>

static int max(int x, int y)
{
    return x > y ? x : y;
}

static int depth(struct rbnode *node)
{
    if (node == NULL)
        return 0;
    
    return 1 + max(depth(node->left), depth(node->right));
}

static int tree_depth(struct rbroot *root)
{
    return depth(root->node);
}

static void print_spaces(int num)
{
    for (int i = 0; i < num; i++) {
        putchar(' ');
    }
}

static void print_bars(int num)
{
    for (int i = 0; i < num; i++) {
        printf("─");
    }
}

static void print_left_link(int link_len)
{
    printf("┌");
    print_bars(link_len - 1);
}

static void print_right_link(int link_len)
{
    print_bars(link_len - 1);
    printf("┐");
}

// return 2^n
static int base2pow(int n)
{
    assert(n <= 31);
    return 1 << n;
}

const int WIDTH = 2;

static void print_node(struct rbnode *n, int link_len)
{
    if (n != NULL) {
        if (n->left != NULL) {
            print_left_link(link_len);
        } else {
            print_spaces(link_len);
        }

        printf("%*d", WIDTH, n->val);
        if (rb_is_red(n)) {
            putchar('*');
        } else {
            putchar(' ');
        }

        if (n->right != NULL) {
            print_right_link(link_len);
        } else {
            print_spaces(link_len);
        }
    } else {
        // left link(link_len) + right link(link_len) + val(WIDTH) + color('*' or ' ')
        print_spaces(link_len * 2 + WIDTH + 1);
    }
}

static void print_level(int depth, int level, struct rbnode **level_nodes)
{
    int pos = (base2pow(depth - level - 1) - 1) * WIDTH;
    int step = (base2pow(depth - level) - 1) * WIDTH;
    int link_len = pos / 2;
    int nodes_count = base2pow(level);

    print_spaces(pos - link_len);

    for (int j = 0; j < nodes_count; j++) {
        struct rbnode *n = level_nodes[j];
        print_node(n, link_len);

        // do not print the tailing spaces
        if (j == nodes_count - 1) {
            break;
        }

        // current node's right link
        // + next node's left link
        // + current node's color
        print_spaces(step - 2 * link_len - 1);
    }

    puts("");
}

#define swap(x, y) \
    do { \
        typeof(x) tmp = x; \
        x = y; \
        y = tmp; \
    } while (0)

void print_tree(struct rbroot *root)
{
    if (root->node == NULL)
        return;

    int depth = tree_depth(root);
    // printf("depth: %d\n", depth);

    int max_nodes = base2pow(depth - 1);
    struct rbnode **level_buf1 = (struct rbnode **)malloc(max_nodes * sizeof(struct rbnode *));
    struct rbnode **level_buf2 = (struct rbnode **)malloc(max_nodes * sizeof(struct rbnode *));
    assert(level_buf1 && level_buf2);

    struct rbnode **cur_level = level_buf1, **next_level = level_buf2;
    cur_level[0] = root->node;

    for (int i = 0; i < depth; i++) {
        print_level(depth, i, cur_level);

        // we've print the leaf level, get out
        if (i == depth - 1) {
            break;
        }

        int nodes_count = base2pow(i);
        for (int j = 0; j < nodes_count; j++) {
            struct rbnode *n = cur_level[j];

            if (n == NULL) {
                next_level[j * 2] = NULL;
                next_level[j * 2 + 1] = NULL;
            } else {
                next_level[j * 2] = n->left;
                next_level[j * 2 + 1] = n->right;
            }
        }
        swap(cur_level, next_level);
    }

    free(level_buf1);
    free(level_buf2);
}

void rb_inorder_traverse(struct rbnode *x)
{
    if (!x) {
        return;
    }

    rb_inorder_traverse(x->left);
    printf("%d ", x->val);
    rb_inorder_traverse(x->right);
}

static void violate_property(int num)
{
    printf("Violating property %d.\n", num);
    abort();
}

static int max_black = -1;
static int cur_black = 0;
static int last_color = RB_BLACK;
static void __rbtree_check(struct rbnode *n)
{
    int old_color = last_color;

    if (rb_is_black(n)) {
        last_color = RB_BLACK;
        cur_black++;
    } else {
        if (!rb_is_red(n)) {
            // 1) A node is either red or black
            violate_property(1);
        }
        last_color = RB_RED;
        // consecutive red node
        if (old_color == RB_RED) {
            // 4) Both children of every red node are black
            violate_property(4);
        }
    }

    if (!n) {
        if (max_black == -1) {
            max_black = cur_black;
        }
        if (max_black != cur_black) {
            // 5) Every simple path from root to leaves contains the same number
            //    of black nodes.
            violate_property(5);
        }
    } else {
        if ((n->left && (n->left->val > n->val || rb_predecessor(n)->val > n->val)) ||
                (n->right && (n->right->val < n->val || rb_successor(n)->val < n->val))) {
            printf("Violating BST property.\n");
            abort();
        }
        __rbtree_check(n->left);
        __rbtree_check(n->right);
    }

    if (rb_is_black(n))
        cur_black--;
    last_color = old_color;
}

void is_rbtree(struct rbroot *root)
{
    max_black = -1;
    cur_black = 0;
    last_color = RB_BLACK;

    if (!rb_is_black(root->node)) {
        // 2) The root is black
        violate_property(2);
    }
    __rbtree_check(root->node);
}