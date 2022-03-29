//project 2 function definitions
//Ben Layko
//Andrew Haberman
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <stdbool.h>

typedef char object_t;
typedef struct {
    object_t **base;
    object_t **top;
    int size;
} stack_n_t;

stack_n_t *create_stack(int size) {
    stack_n_t *st;
    st = (stack_n_t *) malloc(sizeof(stack_n_t));
    st->base = (object_t **) malloc(size * sizeof(object_t *));
    st->size = size;
    st->top = st->base;
    return (st);
}

bool stack_empty(stack_n_t *st) {
    return (st->base == st->top);
}

int push(object_t *x, stack_n_t *st) {
    if (st->top < st->base + st->size) {
        *(st->top) = x;
        st->top += 1;
        return (0);
    } else {
        return (-1);
    }
}

object_t *pop(stack_n_t *st) {
    st->top -= 1;
    return (*(st->top));
}

object_t *top_element(stack_n_t *st) {
    return (*(st->top - 1));
}

void remove_stack(stack_n_t *st) {
    free(st->base);
    free(st);
}


//interval structure
typedef struct interval_t{
    int a;
    int b;
    struct interval_t *next;
}interval_t;

//create measure tree structure
typedef struct m_tree_t {
    int key;
    int a;//lower bound of node interval
    int b;//upper bound of node interval
    int measure;
    int leftmin;
    int rightmax;
    int height; //delete if not needed
    struct m_tree_t *left;
    struct m_tree_t *right;
    struct interval_t *list;
}m_tree_t;



m_tree_t * create_m_tree(){
    m_tree_t *tmp;
    tmp = (m_tree_t *)malloc(sizeof(m_tree_t));
    tmp->key = 0;
    tmp->a = INT_MIN;
    tmp->b = INT_MAX;
    tmp->measure = 0;
    tmp->leftmin = 0;
    tmp->rightmax = 0;
    tmp->height = 0;
    tmp->left = NULL;
    tmp->right = NULL; 

    return tmp;
}

interval_t * create_interval(int a, int b){
    interval_t *new_interval;
    new_interval = (interval_t *)malloc(sizeof(interval_t));
    new_interval->a = (a < b) ? a : b;
    new_interval->b = (a > b) ? a : b;
    new_interval->next = NULL;

    return new_interval;
}

int update_measure(m_tree_t *tree){
    int measure;

    if(tree->right == NULL){
        if(tree->left == NULL){
            return 0;
        }
        int Min = (tree->b < tree->rightmax) ? tree->b : tree->rightmax;
        int Max = (tree->a > tree->leftmin) ? tree->a : tree->leftmin;
        measure = (Min) - (Max);
    }
    else{
        if ((tree->right->leftmin < tree->a) && (tree->left->rightmax >= tree->b)) {
			measure = tree->b - tree->a;
		} else if ((tree->right->leftmin >= tree->a) && (tree->left->rightmax >= tree->b)) {
			measure = (tree->b - tree->key) + tree->left->measure;
		} else if ((tree->right->leftmin < tree->a) && (tree->left->rightmax < tree->b)) {
			measure = tree->right->measure + (tree->key - tree->a);
		} else if ((tree->right->leftmin >= tree->a) && (tree->left->rightmax < tree->b)) {
			measure = tree->right->measure + tree->left->measure;
        }
    }
    return measure;
}

void left_rotation(m_tree_t *n) {
	m_tree_t *tmp_node;
	int tmp_key;
	tmp_node = n->left;
	tmp_key = n->key;
	n->left = n->right;
	n->key = n->right->key;
	n->right = n->left->right;
	n->left->right = n->left->left;
	n->left->left = tmp_node;
	n->left->key = tmp_key;
	n->left->a = n->a;
	n->left->b = n->key;
	n->left->leftmin = (n->left->left->leftmin < n->left->right->leftmin) ? n->left->left->leftmin : n->left->right->leftmin;
	n->left->rightmax = (n->left->left->rightmax > n->left->right->rightmax) ? n->left->left->rightmax : n->left->right->rightmax;
	n->left->measure = update_measure(n->left);
}

void right_rotation(m_tree_t *n) {
	m_tree_t *tmp_node;
	int tmp_key;
	tmp_node = n->right;
	tmp_key = n->key;
	n->right = n->left;
	n->key = n->left->key;
	n->left = n->right->left;
	n->right->left = n->right->right;
	n->right->right = tmp_node;
	n->right->key = tmp_key;
	n->right->a = n->key;
	n->right->b = n->b;
	n->right->leftmin = (n->right->left->leftmin < n->right->right->leftmin) ? n->right->left->leftmin : n->right->right->leftmin;
	n->right->rightmax = (n->right->left->rightmax > n->right->right->rightmax) ? n->right->left->rightmax : n->right->right->rightmax;
	n->right->measure = update_measure(n->right);
}

int min(int a, int b){
    if(a < b) return a;
    else return b;
}
int max(int a, int b){
    if(a > b) return a;
    else return b;
}

void insert(m_tree_t *tree, int a, int b){    
    m_tree_t *temp = tree;
    interval_t *new_interval;
    int complete_flag = 0;

    if(temp->left ==NULL){
        //tree is empty insert here
        new_interval = create_interval(a, b);
        temp->left = (m_tree_t *) new_interval;
        temp->key = a;
        temp->height = 0;
        temp->leftmin = new_interval->a;
        temp->rightmax = new_interval->b;
        temp->measure = update_measure(temp); //create function with measure rules
        temp->right = NULL; 
    }//end if
    else{
        //search tree for node to insert
        m_tree_t * path_arr[100];
        int arrptr = 0; 
        while(temp->right != NULL){
            //add node to array
            path_arr[arrptr++] = temp; 
            if(a < temp->key){
                //go left
                temp = temp->left;
            }
            else{
                //go right
                temp = temp->right;
            }
        }//end while loop

        //insert interval
        //case 1 - insert into an existing linked list
        if(temp->key == a){
            interval_t *existing = (interval_t *) temp->left;
            new_interval  = create_interval(a, b);
            if(existing == NULL){
                temp->left = (m_tree_t *) new_interval;
            } else {
                while(existing->next != NULL ){
                    existing = existing->next; 
                }
                
                existing->next = new_interval;
    
                //update leftmin and rightmax
                int min, max;
                min = temp->leftmin;
                max = temp->rightmax;
                existing = (interval_t *) temp->left;
                while(existing != NULL){
                    if(existing->a < min){
                        min = existing->a;
                    }
                    if(existing->b > max){
                        max = existing->b; 
                    }
                    existing = existing->next;
                }//end while
                temp->leftmin = min;
                temp->rightmax = max;
                temp->measure = update_measure(temp);
            }
        }//end if - case 1 
        //case 2/3 - linked list doesn't exist
        else{
            m_tree_t *old, *updated;
            old = create_m_tree();
            updated = create_m_tree();

            old->left = temp->left;
            old->right = NULL;
            old->key = temp->key;
            old->height = 0;
            old->leftmin = temp->leftmin; 
            old->rightmax = temp->rightmax;

            new_interval = create_interval(a, b);
            updated->left =  (m_tree_t *) new_interval;
            updated->key = a;
            updated->height = 0;
            updated->leftmin = new_interval->a;
            updated->rightmax = new_interval->b;
            updated->right = NULL; 

            //case 2
            if(temp->key < a){
                //case 2
                temp->left = old;
                temp->right = updated;
                temp->key = a;

                updated->a = temp->key;
                updated->b = temp->b;
                updated->measure = update_measure(updated);

                old->a = temp->a;
                old->b = temp->key;
                old->measure = update_measure(old);
            } //endif case 2
            else{
                //case 3 
                temp->left = updated;
                temp->right = old;

                updated->a = temp->a;
                updated->b = temp->key;
                updated->measure = update_measure(updated);

                old->a = temp->key;
                old->b = temp->b;
                old->measure = update_measure(old);
            } //end if case 3
            temp->height = 1; 

            temp->leftmin = (temp->left->leftmin < temp->right->leftmin) ? temp->left->leftmin : temp->right->leftmin; 
            temp->rightmax = (temp->left->rightmax > temp->right->rightmax) ? temp->left->rightmax : temp->right->rightmax;
            temp->measure = update_measure(temp);
        }//end else case 2/3
        temp->measure = update_measure(temp);

        int pathptr2 = arrptr;

        while(pathptr2 > 0){
            temp = path_arr[--pathptr2];
            temp->leftmin = (temp->left->leftmin < temp->right->leftmin) ? temp->left->leftmin : temp->right->leftmin;
            temp->rightmax = (temp->left->rightmax > temp->right->rightmax) ? temp->left->rightmax : temp->right->rightmax; 
            temp->measure = update_measure(temp);
        }

        //rebalance the tree
        complete_flag = 0;
        while (arrptr > 0 && !complete_flag) {
			int tmp_height, old_height;
			temp = path_arr[--arrptr];
			old_height = temp->height;
			if (temp->left->height - temp->right->height == 2) {
				if (temp->left->left->height - temp->right->height == 1) {
					right_rotation(temp);
					temp->right->height = temp->right->left->height + 1;
					temp->height = temp->right->height + 1;
				} 
                else {
				    left_rotation(temp->left);
					right_rotation(temp);
					tmp_height = temp->left->left->height;
					temp->left->height = tmp_height + 1;
					temp->right->height = tmp_height + 1;
					temp->height = tmp_height + 2;
				}
			} 
            else if (temp->left->height - temp->right->height == -2) {
			    if (temp->right->right->height - temp->left->height == 1) {
				    left_rotation(temp);
					temp->left->height = temp->left->right->height + 1;
					temp->height = temp->left->height + 1;
				} 
                else {
				    right_rotation(temp->right);
					left_rotation(temp);
					tmp_height = temp->right->right->height;
					temp->left->height = tmp_height + 1;
					temp->right->height = tmp_height + 1;
					temp->height = tmp_height + 2;
				}
			} 
            else {
			    /* update height even if there was no rotation */
				if (temp->left->height > temp->right->height)
				    temp->height = temp->left->height + 1;
				else
				    temp->height = temp->right->height + 1;
			}
			if (temp->height == old_height)
			    complete_flag = 1;
		}
    }//end else
}

void insert_interval(m_tree_t *tree, int a, int b){
    if(a > b) return;
    else{
        insert(tree, a, b);
        insert(tree, b, a);
    }
}

void deleteFunction(m_tree_t *tree, int a, int b){
    m_tree_t *temp = tree;
    m_tree_t *upper, *other;
    m_tree_t *grandparent = NULL;
    interval_t * deleted_object;
    
    //Handle empty tree and one node case
    if(tree->left == NULL){
        return;
    } 
    else if (tree->right == NULL){
        if(tree->key == a){
            deleted_object = (interval_t *) tree->left;
            tree->left = NULL;
            tree->key = 0;
            tree->height = 0; 
            tree->a = INT_MIN;
            tree->b = INT_MAX;
            tree->leftmin = 0;
            tree->rightmax = 0;
            tree->measure = 0;
            free(tree->left);
            return;
        }
        else{
            return; 
        }
    } 
    else {
        //search tree for node to delete
        m_tree_t * path_arr[100];
        int arrptr = 0;
        while(temp->right != NULL){
            //add node to array
            path_arr[arrptr++] = temp; 
            upper = temp;

            if(a < temp->key){
                //go left
                temp = upper->left;
                other = upper->right;
            }
            else{
                //go right
                temp = upper->right;
                other = upper->left;
            }
        }//end while loop

        if(temp->key != a){
            deleted_object = NULL;
            return; //node interval doesn't exist
        }
        else{
            interval_t *old_interval, *interval_delete, *prev;
            old_interval = (interval_t *) temp->left;
            prev = NULL; 
            interval_delete = NULL;
            int counter = 1;

            //find interval to delete
            while(old_interval != NULL){
                if((old_interval->a == min(a,b)) && (old_interval->b == max(a,b))){
                    interval_delete = old_interval;
                    break;
                }
                prev = old_interval;
                old_interval = old_interval->next;
                counter++;
            }//end while searching for interval to delete

            if(interval_delete == NULL){
                return; 
            }
            else{
                if((counter == 1) && (interval_delete->next == NULL)){
                    upper->key = other->key;
                    upper->left = other->left;
                    upper->right = other->right;
                    upper->height = other->height;
                    upper->leftmin = other->leftmin;
                    upper->rightmax = other->rightmax;
                    deleted_object = (interval_t *) temp->left;
                    
                    if(upper->right != NULL){
                        upper->right->b = upper->b;
                        upper->left->a = upper->a;
                    }
                    upper->measure = other->measure;
                    free(temp->left);

                    //find the successor
                    if(tree->key == a){
                        grandparent = tree->right;
                        while(grandparent->right != NULL){
                            grandparent = grandparent->left;
                        }
                        tree->key = grandparent->key;
                        tree->left->b = tree->key;
                        tree->right->a = tree->key;
                        tree->left->measure = update_measure(tree->left);
                        tree->right->measure = update_measure(tree->right);
                    }
                }
                else if((counter == 1) && (interval_delete->next != NULL)){
                    temp->left = (m_tree_t *) interval_delete->next;
                    deleted_object = (interval_t *) interval_delete;
                    free(interval_delete);

                    int min, max;
                    old_interval = (interval_t *) temp->left;
                    min = old_interval->a;
                    max = old_interval->b;

                    while(old_interval != NULL){
                        if(old_interval->a < min){
                            min = old_interval->a;
                        }
                        if(old_interval->b > max){
                            max = old_interval->b;
                        }
                        old_interval = old_interval->next;
                    }
                    temp->leftmin = min;
                    temp->rightmax = max;
                    temp->measure = update_measure(temp);
                }
                else{
                    prev->next = interval_delete->next;
                    deleted_object = (interval_t *) interval_delete;
                    free(interval_delete);

                    int min, max;
                    old_interval = (interval_t *) temp->left;
                    min = old_interval->a;
                    max = old_interval->b;

                    while(old_interval != NULL){
                        if(old_interval->a < min){
                            min = old_interval->a;
                        }
                        if(old_interval->b > max){
                            max = old_interval->b;
                        }
                        old_interval = old_interval->next;
                    }
                    temp->leftmin = min;
                    temp->rightmax = max;
                    temp->measure = update_measure(temp);
                }
            }
        }
        int path_ptr2 = arrptr;
        while(path_ptr2 > 0){
            temp = path_arr[--path_ptr2];
            if(temp->right != NULL){
                temp->leftmin = min(temp->left->leftmin, temp->right->leftmin);
                temp->rightmax = max(temp->left->rightmax, temp->right->rightmax);
                temp->measure = update_measure(temp);
            }
            else{
                int min, max;
				interval_t *old_interval;
				old_interval = (interval_t *) temp->left;
				min = old_interval->a;
				max = old_interval->b;
				while (old_interval != NULL) {
					if (old_interval->a < min) {
						min = old_interval->a;
					}
					if (old_interval->b > max) {
						max = old_interval->b;
					}
					old_interval = old_interval->next;
				}
				temp->leftmin = min;
				temp->rightmax = max;
				temp->measure = update_measure(temp);
            }
        }
        //rebalance the tree
        int complete_flag = 0;
        arrptr -= 1; 
        while (arrptr > 0 && !complete_flag) {
            int tmp_height, old_height;
            temp = path_arr[--arrptr];
            old_height = temp->height;
            if(temp->left->height - temp->right->height == 2) {
                if(temp->left->left->height - temp->right->height== 1) {
                    right_rotation(temp);
                    temp->right->height = temp->right->left->height + 1;
                    temp->height = temp->right->height + 1;
                } else {
                    left_rotation(temp->left);
                    right_rotation(temp);
                    tmp_height = temp->left->left->height;
                    temp->left->height = tmp_height + 1;
                    temp->right->height = tmp_height + 1;
                    temp->height = tmp_height + 2;
                }
            } 
            else if (temp->left->height - temp->right->height == -2) {
                if (temp->right->right->height - temp->left->height	== 1) {
                    left_rotation(temp);
                    temp->left->height = temp->left->right->height + 1;
                    temp->height = temp->left->height + 1;
                } else {
                    right_rotation(temp->right);
                    left_rotation(temp);
                    tmp_height = temp->right->right->height;
                    temp->left->height = tmp_height + 1;
                    temp->right->height = tmp_height + 1;
                    temp->height = tmp_height + 2;
                }
            } 
            else {
                /* update height even if there was no rotation */
                if (temp->left->height > temp->right->height)
                    temp->height = temp->left->height + 1;
                else
                    temp->height = temp->right->height + 1;
            }
            if (temp->height == old_height)
                complete_flag = 1;
        }   
    }
}
void delete_interval(m_tree_t *tree, int a, int b){
    if(a > b) return;
    else{
        deleteFunction(tree, a, b);
        deleteFunction(tree, b, a);
    }
}
int query_length(m_tree_t *tree){
    return tree->measure;
}
int root_key(m_tree_t *tree){
    return tree->key;
}
void print_submeasures(m_tree_t *tree){
    stack_n_t *stack = create_stack(tree->height + 1);
    push((object_t *) tree, stack);
    while(!stack_empty(stack)){
        m_tree_t *currentNode = (m_tree_t *) pop(stack);
        if(currentNode->right == NULL){
            printf("Key: %d, L, Measure: %d\n", currentNode->key, currentNode->measure);
        }
        else{
            printf("Key: %d, I, Measure: %d\n", currentNode->key, currentNode->measure);
            push((object_t *) currentNode->right, stack);
            push((object_t *) currentNode->left, stack);
        }
    }
    remove_stack(stack);
}
int max_submeasure(m_tree_t *tree){
    int node = -1;//values not likely to be seen in trees
    int submeasure = -1; 
    stack_n_t *stack = create_stack(tree->height + 1);
    push((object_t *) tree, stack);
    while(!stack_empty(stack)){
        m_tree_t *currentNode = (m_tree_t *) pop(stack);
        if(currentNode == NULL){
            continue;
        }
        if(currentNode->right == NULL){
            //leaf node
            if(currentNode->measure > submeasure){
                node = currentNode->key;
                submeasure = currentNode->measure;
            }
        }
        else{
            push((object_t *) currentNode->right, stack);
            push((object_t *) currentNode->left, stack);
        }
    }
    remove_stack(stack);
    return node;    
}

void destroy_m_tree(m_tree_t *tree){
    stack_n_t *stack = create_stack(tree->height + 1);
    push((object_t *) tree, stack);
    while(!stack_empty(stack)){
        m_tree_t *currentNode = (m_tree_t *) pop(stack);
        if(currentNode == NULL){
            continue;
        }
        if(currentNode->right == NULL){
            //node is a leaf node
            //loop to destroy all intervals
            interval_t *curr, *prev;
            curr = (interval_t *) currentNode->left;
            while(curr != NULL){
                prev = curr;
                curr = curr->next;
                free(prev);
            }
            free(currentNode);
        }
        else{
            //not a leaf node
            push((object_t *) currentNode->right, stack);
            push((object_t *) currentNode->left, stack);
            //delete the current node
            free(currentNode);
        }
    }
    remove_stack(stack);
}