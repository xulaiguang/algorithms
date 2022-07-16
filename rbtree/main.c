#include "rbtree.h"
#include "util.h"

#include <stdio.h>

void rb_insert_val(struct rbroot *root, int val)
{
    struct rbnode *node = rb_alloc_node();
    struct rbnode **link = &root->node;
    struct rbnode *cur = root->node;
    struct rbnode *parent = NULL;
    assert(node);

    printf("inserting %d\n", val);
    node->val = val;

    while (cur) {
        parent = cur;
        if (val < cur->val) {
            link = &cur->left;
            cur = cur->left;
        } else {
            link = &cur->right;
            cur = cur->right;
        }
    }

    rb_link_node(node, parent, link);
    rb_insert_balance(node, root);
}

void rb_erase_val(struct rbroot *root, int val)
{
    struct rbnode *node = root->node;
    printf("erasing %d\n", val);
    
    while (node) {
        if (val == node->val) {
            break;
        }
        if (val < node->val) {
            node = node->left;
        } else {
            node = node->right;
        }
    }

    if (node) {
        rb_erase(node, root);
        rb_free_node(node);
    }
}

typedef void (*rb_func_t)(struct rbroot *root, int val);

static void read_random_sequence(const char *filename, struct rbroot *root, rb_func_t func)
{
    char *line = NULL;
    size_t size = 0;
    FILE *f = fopen(filename, "r");
    assert(f);

    while (getline(&line, &size, f) > 0) {
        func(root, strtol(line, NULL, 10));
        print_tree(root);
        is_rbtree(root);
        free(line);
        line = NULL;
        size = 0;
    }

    fclose(f);
}

static void random_insert(const char *filename, struct rbroot *root)
{
    read_random_sequence(filename, root, rb_insert_val);
}

static void random_erase(const char *filename, struct rbroot *root)
{
    read_random_sequence(filename, root, rb_erase_val);
}

int main()
{
    struct rbroot root;
    root.node = NULL;

    is_rbtree(&root);
    print_tree(&root);

    random_insert("random_sequence.txt", &root);

    rb_inorder_traverse(root.node);
    puts("");

    print_tree(&root);

    random_erase("random_sequence.txt", &root);    

    return 0;
}
