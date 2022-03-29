#include <stdio.h>
#include <stdlib.h>
#include "stack.c"

typedef int key_t;
typedef int height_t;
typedef struct tr_n_t {
    key_t key; // key is the distance from parent node
    struct tr_n_t *left;
    struct tr_n_t *right;
    height_t height;
    /* possibly additional information */ } tree_node_t;


#define BLOCKSIZE 256

tree_node_t *currentblock = NULL;
int size_left;
tree_node_t *free_list = NULL;
int nodes_taken = 0;
int nodes_returned = 0;
key_t current_index = 0; // Used as index cursor

tree_node_t *get_node();
void return_node(tree_node_t *node);
tree_node_t *create_tree(void);
object_t *find_iterative(tree_node_t *tree, key_t query_key);
object_t *find_recursive(tree_node_t *tree, key_t query_key);
void right_rotation(tree_node_t *node);
void left_rotation(tree_node_t *node);
int balancedInsert(tree_node_t *tree, key_t new_key, object_t *new_object);
object_t *updateNodeRecursive(tree_node_t *tree, key_t query_key, object_t *new_object);
object_t *replaceNodeRecursive(tree_node_t *tree, key_t query_key, object_t *new_object);
object_t *_balancedDelete(tree_node_t *tree, key_t delete_key);
void balancing(stack_n_t *stack);
void balancingDelete(stack_n_t *stack);

tree_node_t *get_node() {
    tree_node_t *tmp;
    nodes_taken += 1;
    if (free_list != NULL) {
        tmp = free_list;
        free_list = free_list->right;
    } else {
        if (currentblock == NULL || size_left == 0) {
            currentblock =
                    (tree_node_t *) malloc(BLOCKSIZE * sizeof(tree_node_t));
            size_left = BLOCKSIZE;
        }
        tmp = currentblock++;
        size_left -= 1;
    }
    return (tmp);
}

void return_node(tree_node_t *node) {
    node->right = free_list;
    free_list = node;
    nodes_returned += 1;
}

tree_node_t *create_tree(void) {
    tree_node_t *tmp_node;
    tmp_node = get_node();
    tmp_node->left = NULL;
    return (tmp_node);
}

object_t *find_iterative(tree_node_t *tree, key_t query_key) {
    tree_node_t *tmp_node;
    if (tree->left == NULL)
        return (NULL);
    else {
        tmp_node = tree;
        current_index += tmp_node->key;
        while (tmp_node->right != NULL) {
            if (query_key < current_index)
                tmp_node = tmp_node->left;
            else
                tmp_node = tmp_node->right;
            current_index += tmp_node->key;
        }
        if (current_index == query_key) {
            current_index = 0;
            return (object_t *)tmp_node->left ;
        } else {
            current_index = 0;
            return (NULL);
        }
    }
}

/* We need to find recursively until reach leaves, which means
 * ->right == NULL
 * object pointer always stored in ->left
 * */
object_t *find_recursive(tree_node_t *tree, key_t query_key) {
    current_index += tree->key;
    if (tree->left == NULL ||
        (tree->right == NULL && current_index != query_key)) {
        // Case 1 first: root node is the leaf, then return NULL
        // Case 1 second: reach leaf, but the leaf node is not what we want
        current_index = 0; // Move cursor back to root
        return (NULL);
    } else if (tree->right == NULL && current_index == query_key) {
        // Case 2: reach leaf, and the query_key is the one we are finding
        current_index = 0; // Move cursor back to root
        return (object_t *)tree->left;
    } else {
        // Case 3: Haven't reached leaf
        if (query_key < current_index)
            return (find_recursive(tree->left, query_key));
        else
            return (find_recursive(tree->right, query_key));
    }
}

//Used for re-balancing
void right_rotation(tree_node_t *node) {
    /* Changes:
     * New_node = Old_node->left
     * New_node->right = Old_node
     * New_node->right->left = Old_node->left->right
     * New_node->key = Old_node->key + Old_node->left->key
     * New_node->right->key = -1 * Old_node->left->key
     * New_node->right->left->key = Old_node->left->key + Old_node->left->right->key
     * */
    tree_node_t *tmp_node;
    key_t old_node_key, old_node_left_key, old_node_left_right_key;
    tmp_node = node->right;
    old_node_key = node->key;
    old_node_left_key = node->left->key;
    old_node_left_right_key = node->left->right->key;

    node->right = node->left;
    node->left = node->right->left;
    node->right->left = node->right->right;
    node->right->right = tmp_node;

    node->key = old_node_key + old_node_left_key;
    node->right->key = -1 * old_node_left_key;
    node->right->left->key = old_node_left_key + old_node_left_right_key;
}

void left_rotation(tree_node_t *node) {
    /* Changes:
     * New_node = Old_node->right
     * New_node->left = Old_node
     * New_node->left->right = Old_node->right->left
     * New_node->key = Old_node->key + Old_node->right->key
     * New_node->left->key = -1 * Old_node->right->key
     * New_node->left->right->key = Old_node->right->key + Old_node->right->left->key
     * */
    tree_node_t *tmp_node;
    key_t old_node_key, old_node_right_key, old_node_right_left_key;
    tmp_node = node->left;
    old_node_key = node->key;
    old_node_right_key = node->right->key;
    old_node_right_left_key = node->right->left->key;

    node->left = node->right;
    node->right = node->left->right;
    node->left->right = node->left->left;
    node->left->left = tmp_node;

    node->key = old_node_key + old_node_right_key;
    node->left->key = -1 * old_node_right_key;
    node->left->right->key = old_node_right_key + old_node_right_left_key;
}

//This code was heavily inspired by the code provided in the textbook
int balancedInsert(tree_node_t *tree, key_t new_key, object_t *new_object) {
    tree_node_t *tmp_node;
    bool finished = false;
    //This is for when the tree is empty.
    if (tree->left == NULL) {
        tree->left = (tree_node_t *) new_object;
        tree->key = new_key;
        tree->height = 0;
        tree->right = NULL;
    } else {
        stack_n_t *stack = create_stack(tree->height + 1);
        stack_n_t *stack_minus = create_stack(tree->height + 1);
        stack_n_t *stack_add = create_stack(tree->height + 1);
        int num_stack_minus = 0;
        int num_stack_add = 0;
        bool previous_right = true;
        tmp_node = tree;
        current_index = tree->key;
        while (tmp_node->right != NULL) { // Find the leaf
            push((object_t *) tmp_node, stack);
            if (new_key < current_index) {
                if (previous_right) {
                    push((object_t *) tmp_node, stack_add);
                    previous_right = false;
                    num_stack_add++;
                }
                tmp_node = tmp_node->left;
                current_index += tmp_node->key;
            } else {
                if (!previous_right) {
                    push((object_t *) tmp_node, stack_minus);
                    previous_right = true;
                    num_stack_minus++;
                }
                tmp_node = tmp_node->right;
                current_index += tmp_node->key;
            }
        }
        //Test if the key is new or not
        if (current_index == new_key) { // if index exist
            tree_node_t *old_leaf, *new_leaf;
            old_leaf = get_node(); // Add a new node here
            old_leaf->left = tmp_node->left;
            old_leaf->right = NULL;
            old_leaf->height = 0;
            new_leaf = get_node();
            new_leaf->left = (tree_node_t *) new_object;
            new_leaf->right = NULL;
            new_leaf->height = 0;
            // old leaf go left, new leaf go right for 1 only
            tmp_node->left = new_leaf;
            tmp_node->right = old_leaf;
            if (previous_right)
                tmp_node->key++; // parent node will increase 1 line distance since right one increase 1
            tmp_node->right->key = 0; // new leaf go right, new parent_node index will be the same as new one
            tmp_node->left->key = -1; // old leaf go left, new parent_node index will be 1 larger than old
            tmp_node->height = 1;
            // Last step: since line exist, left curve nodes distance minus 1, right curve nodes distance add 1
            while (!stack_empty(stack_minus) && (num_stack_minus > 0)) {
                tmp_node = (tree_node_t *) pop(stack_minus);
                tmp_node->key--;
                num_stack_minus--;
            }
            remove_stack(stack_minus);
            while (!stack_empty(stack_add) && (num_stack_add > 0)) {
                tmp_node = (tree_node_t *) pop(stack_add);
                tmp_node->key++;
                num_stack_add--;
            }
            remove_stack(stack_add);
        } else {
            tree_node_t *old_leaf, *new_leaf;
            old_leaf = get_node();
            old_leaf->left = tmp_node->left;
            old_leaf->right = NULL;
            old_leaf->height = 0;
            new_leaf = get_node();
            new_leaf->left = (tree_node_t *) new_object;
            new_leaf->right = NULL;
            new_leaf->height = 0;
            if (current_index < new_key) { // new_leaf go right
                tmp_node->left = old_leaf;
                tmp_node->right = new_leaf;
            } else { // new_leaf go left
                tmp_node->left = new_leaf;
                tmp_node->right = old_leaf;
            }
            tmp_node->key = tmp_node->key + (new_key - current_index);
            tmp_node->right->key = 0;
            tmp_node->left->key = current_index - new_key;
            tmp_node->height = 1;
        }
        balancing(stack);
    }
    current_index = 0;
    return (0);
}

object_t *updateNodeRecursive(tree_node_t *tree, key_t query_key, object_t *new_object) {
    current_index += tree->key;
    if (tree->left == NULL || (tree->right == NULL && current_index != query_key)) {
        balancedInsert(tree, query_key, new_object);
        current_index = 0;
        return (NULL);
    } else if (tree->right == NULL && current_index == query_key) {
        object_t *temp = (object_t *) tree->left;
        tree->left = (tree_node_t *) new_object;
        current_index = 0;
        return temp;
    } else {
        if (query_key < current_index) {
            return (updateNodeRecursive(tree->left, query_key, new_object));
        } else {
            return (updateNodeRecursive(tree->right, query_key, new_object));
        }
    }
}

object_t *replaceNodeRecursive(tree_node_t *tree, key_t query_key, object_t *new_object) {
    current_index += tree->key;
    if (tree->left == NULL || (tree->right == NULL && current_index != query_key)) {
        current_index = 0;
        return (NULL);
    } else if (tree->right == NULL && current_index == query_key) {
        tree->left = (tree_node_t *) new_object;
        current_index = 0;
        return (object_t *) tree->left;
    } else {
        if (query_key < current_index) {
            return (updateNodeRecursive(tree->left, query_key, new_object));
        } else {
            return (updateNodeRecursive(tree->right, query_key, new_object));
        }
    }
}

object_t *_balancedDelete(tree_node_t *tree, key_t delete_key) {
    tree_node_t *tmp_node, *upper_node, *other_node;
    object_t *deleted_object;
    bool finished = false;
    if (tree->left == NULL)
        return (NULL);
    else if (tree->right == NULL) {
        current_index += tree->key;
        if (current_index == delete_key) {
            deleted_object = (object_t *) tree->left;
            tree->left = NULL;
            tree->height = 0;
            current_index = 0;
            return (deleted_object);
        } else {
            current_index = 0;
            return (NULL);
        }
    } else {
        tmp_node = tree;
        current_index += tmp_node->key;
        stack_n_t *stack = create_stack(tree->height + 1);
        stack_n_t *stack_minus = create_stack(tree->height + 1);
        stack_n_t *stack_add = create_stack(tree->height + 1);
        int num_stack_minus = 0;
        int num_stack_add = 0;
        bool previous_right = true; // From root, default is previous right
        while (tmp_node->right != NULL) {
            push((object_t *) tmp_node, stack);
            upper_node = tmp_node;
            if (delete_key < current_index) {
                if (previous_right) {
                    push((object_t *) tmp_node, stack_minus);
                    previous_right = false;
                    num_stack_minus++;
                }
                tmp_node = upper_node->left;
                other_node = upper_node->right;
                current_index += tmp_node->key;
            } else {
                if (!previous_right) {
                    push((object_t *) tmp_node, stack_add);
                    previous_right = true;
                    num_stack_add++;
                }
                tmp_node = upper_node->right;
                other_node = upper_node->left;
                current_index += tmp_node->key;
            }
        }
        if (current_index != delete_key) {
            current_index = 0;
            return (NULL);
        } else {
            upper_node->key += other_node->key;
            upper_node->left = other_node->left;
            upper_node->right = other_node->right;
            upper_node->height = 0;
            deleted_object = (object_t *) tmp_node->left;

            // Last step: since line reduce by 1, left curve nodes add 1, right curve nodes minus 1
            while (!stack_empty(stack_minus) && (num_stack_minus > 0)) {
                tmp_node = (tree_node_t *) pop(stack_minus);
                tmp_node->key--;
                num_stack_minus--;
            }
            remove_stack(stack_minus);
            while (!stack_empty(stack_add) && (num_stack_add > 0)) {
                tmp_node = (tree_node_t *) pop(stack_add);
                tmp_node->key++;
                num_stack_add--;
            }
            remove_stack(stack_add);

            balancing(stack);

            current_index = 0;
            return (deleted_object);
        }
    }
}

void balancing(stack_n_t *stack) {
    tree_node_t *tmp_node;
    bool finished = false;

    while (!stack_empty(stack) && !finished) {
        int tmp_height, old_height;
        tmp_node = (tree_node_t *) pop(stack);
        old_height = tmp_node->height;
        if (tmp_node->right != NULL) {
            if ((tmp_node->left->height) - (tmp_node->right->height) == 2) {
                if ((tmp_node->left->left->height) - (tmp_node->right->height) == 1) {
                    right_rotation(tmp_node);
                    tmp_node->right->height = tmp_node->right->left->height + 1;
                    tmp_node->height = tmp_node->right->height + 1;
                } else {
                    left_rotation(tmp_node->left);
                    right_rotation(tmp_node);
                    tmp_height = tmp_node->left->left->height;
                    tmp_node->left->height = tmp_height + 1;
                    tmp_node->right->height = tmp_height + 1;
                    tmp_node->height = tmp_height + 2;
                }
            } else if ((tmp_node->left->height) - (tmp_node->right->height) == -2) {
                if ((tmp_node->right->right->height) - (tmp_node->left->height) == 1) {
                    left_rotation(tmp_node);
                    tmp_node->left->height = tmp_node->left->right->height + 1;
                    tmp_node->height = tmp_node->left->height + 1;
                } else {
                    right_rotation(tmp_node->right);
                    left_rotation(tmp_node);
                    tmp_height = tmp_node->right->right->height;
                    tmp_node->left->height = tmp_height + 1;
                    tmp_node->right->height = tmp_height + 1;
                    tmp_node->height = tmp_height + 2;
                }
            } else {
                if (tmp_node->left->height > tmp_node->right->height) {
                    tmp_node->height = tmp_node->left->height + 1;
                } else {
                    tmp_node->height = tmp_node->right->height + 1;
                }
            }
            if (tmp_node->height == old_height) {
                finished = true;
            }
        }
    }
    remove_stack(stack);
}

void balancingDelete(stack_n_t *stack) {
    tree_node_t *tmp_node;
    bool finished = false;
    int counter = 0;

    while (!stack_empty(stack) && !finished) {
        int tmp_height, old_height;
        tmp_node = (tree_node_t *) pop(stack);
        counter++;
        old_height = tmp_node->height;
        if (counter <= 1) {
            continue;
        } else if (tmp_node->right != NULL) {
            if ((tmp_node->left->height) - (tmp_node->right->height) == 2) {
                if ((tmp_node->left->left->height) - (tmp_node->right->height) == 1) {
                    right_rotation(tmp_node);
                    tmp_node->right->height = tmp_node->right->left->height + 1;
                    tmp_node->height = tmp_node->right->height + 1;
                } else {
                    left_rotation(tmp_node->left);
                    right_rotation(tmp_node);
                    tmp_height = tmp_node->left->left->height;
                    tmp_node->left->height = tmp_height + 1;
                    tmp_node->right->height = tmp_height + 1;
                    tmp_node->height = tmp_height + 2;
                }
            } else if ((tmp_node->left->height) - (tmp_node->right->height) == -2) {
                if ((tmp_node->right->right->height) - (tmp_node->left->height) == 1) {
                    left_rotation(tmp_node);
                    tmp_node->left->height = tmp_node->left->right->height + 1;
                    tmp_node->height = tmp_node->left->height + 1;
                } else {
                    right_rotation(tmp_node->right);
                    left_rotation(tmp_node);
                    tmp_height = tmp_node->right->right->height;
                    tmp_node->left->height = tmp_height + 1;
                    tmp_node->right->height = tmp_height + 1;
                    tmp_node->height = tmp_height + 2;
                }
            } else {
                if (tmp_node->left->height > tmp_node->right->height) {
                    tmp_node->height = tmp_node->left->height + 1;
                } else {
                    tmp_node->height = tmp_node->right->height + 1;
                }
            }
            if (tmp_node->height == old_height) {
                finished = true;
            }
        }
    }
    remove_stack(stack);
}
