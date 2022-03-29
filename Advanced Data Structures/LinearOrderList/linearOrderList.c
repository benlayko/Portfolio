#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//create key_t data type
typedef long int key_t;
//Create height_t data type
typedef int height_t;

#define MIDKEY 1073741823
#define MAXKEY 2147483647
//create o_t structure
typedef struct ordered_list {
	key_t key;
    key_t data;
    long insertions;
    long nextRenumber;
    struct ordered_list ** array;
	struct ordered_list *left;
	struct ordered_list *right;
	struct ordered_list *prev;
    height_t height;
}o_t;

#define BLOCKSIZE 256

//Used in the get node function
o_t *currentblock = NULL;
int size_left;
o_t *free_list = NULL;
int nodes_taken = 0;
int nodes_returned = 0;

o_t *get_node() {
    o_t *tmp;
    nodes_taken += 1;
    if (free_list != NULL) {
        tmp = free_list;
        free_list = free_list->right;
    } else {
        if (currentblock == NULL || size_left == 0) {
            currentblock =
                    (o_t *) malloc(BLOCKSIZE * sizeof(o_t));
            size_left = BLOCKSIZE;
        }
        tmp = currentblock++;
        size_left -= 1;
    }
    return (tmp);
}

void return_node(o_t *node) {
    node->right = free_list;
    free_list = node;
    nodes_returned += 1;
}


o_t *find_iterative(o_t *tree, key_t query_key) {
    o_t *tmp_node = (o_t *) tree->array[query_key];
    return tmp_node;
}

int renumberRecursive(o_t *subTreeRoot, long chunkSize, int chunkNumber){
    if(subTreeRoot == NULL || subTreeRoot->left == NULL){
        return 0;
    } else if(subTreeRoot->right == NULL){
        subTreeRoot->key = chunkSize * chunkNumber;
        return chunkNumber + 1;
    }
    int leafs = chunkNumber;
    leafs = renumberRecursive(subTreeRoot->left, chunkSize, leafs);
    leafs = renumberRecursive(subTreeRoot->right, chunkSize, leafs);
    return leafs;
}

void renumber(o_t *ord){
    long chunkSize = MAXKEY / (ord->insertions + 2); 
    int leafs = 0;
    leafs += renumberRecursive(ord->left, chunkSize,1);
    renumberRecursive(ord->right, chunkSize, leafs);
    
    ord->nextRenumber = ord->nextRenumber * 2;
    
}


//add left and right rotation functions
void left_rotation(o_t* n) {
    o_t* tmp_node;
    o_t* tmp_prev;
    int tmp_key;
    
    tmp_node = n->left;
    tmp_key = n->key;
    tmp_prev = n->prev;
    n->left = n->right;
    n->key = n->right->key;
    n->right = n->left->right;
    n->left->right = n->left->left;
    n->left->left = tmp_node;
    n->left->key = tmp_key;

    //fix up the prev pointers
    n->prev = tmp_prev;
    n->left->prev = n;
    n->right->prev = n;
    n->left->right->prev = n->left;
    n->left->left->prev = n->left;
}

void right_rotation(o_t* n) {
    o_t* tmp_node;
    o_t* tmp_prev;
    int tmp_key;
    tmp_node = n->right;
    tmp_key = n->key;
    tmp_prev = n->prev;
    n->right = n->left;
    n->key = n->left->key;
    n->left = n->right->left;
    n->right->left = n->right->right;
    n->right->right = tmp_node;
    n->right->key = tmp_key;

    //fix up the prev pointers
    n->prev = tmp_prev;
    n->left->prev = n;
    n->right->prev = n;
    n->right->right->prev = n->right;
    n->right->left->prev = n->right;
}

//function definitions
//Notes: key_t a is the key to be inserted key_t b is already inserted
o_t *create_order();
void insert_before(o_t *ord, key_t a, key_t b);
void insert_after(o_t *ord, key_t a, key_t b);
void insert_top(o_t *ord, key_t a);
void insert_bottom(o_t *ord, key_t a);
void delete_o(o_t *ord, key_t a);
int is_before(o_t *ord, key_t a, key_t b);

//Creats the tree to be used
o_t *create_order(){
    o_t *temp = (o_t *) malloc(sizeof(o_t));
    
    //TODO this needs to be changed
    temp->key = NULL;
    temp->data = NULL;
    temp->insertions = 0;
    temp->nextRenumber = 4;
    temp->left = NULL;
    temp->right = NULL;
    temp->prev = NULL;
    temp->height = 0;
    temp->array = (o_t **) malloc(sizeof(o_t *) * 310000);
    
    return temp;
}

//Inserts key a immediately before key b
void insert_before(o_t *ord, key_t a, key_t b){
    o_t * temp = (o_t *) find_iterative(ord, b);
    if(temp != NULL){
        o_t * right = get_node();
        o_t * left = get_node();
        
        //Go up the tree to find the key immediately before b
        o_t * parentOfLeft = temp->prev;
        o_t * leftNeighbor;
        long newKey = 0;
        //This means that temp is the root
        if(parentOfLeft == NULL){
            //There are nodes to the left of temp
            if(temp->right != NULL){
                leftNeighbor = temp->left;
                while(leftNeighbor->right != NULL){
                    leftNeighbor = leftNeighbor->right;
                }
                //We know temp has a left neighbor so we are going to average their keys
                newKey = (temp->key / 2) + (leftNeighbor->key / 2);
            } else {
                //Temp is the only node, so we treat insert before just like we treat insert bottom
                newKey = (temp->key / 2);
            }
        } else {
            //The node is not the root, it has a parent
            //This is an easy insert, just look at temps sibling and average
            if(parentOfLeft->left->key != temp->key){
                leftNeighbor = parentOfLeft->left;
                while(leftNeighbor->right != NULL){
                    leftNeighbor = leftNeighbor->right;
                }
                newKey = (temp->key / 2) + (leftNeighbor->key / 2);
            } else {
                //This is more difficult, temp is the left child, so we need to keep going up until temp is not the left child
                //Go up until we find the root or until temp has something on the left of it
                while(parentOfLeft->prev != NULL && parentOfLeft->key == parentOfLeft->prev->left->key){
                    parentOfLeft = parentOfLeft->prev;
                }
                
                //Here we didn't find anything to the left of temp, so we will treat this just like insert bottom
                if(parentOfLeft->prev == NULL){
                    newKey = (temp->key / 2);
                } else {
                    leftNeighbor = parentOfLeft->left;
                    while(leftNeighbor->right != NULL){
                        leftNeighbor = leftNeighbor->right;
                    }
                    newKey = (temp->key / 2) + (leftNeighbor->key / 2);
                }
            }
        }
        
        right->key = temp->key;
        right->data = temp->data;
        right->left = temp->left;
        right->right = temp->right;
        right->prev = temp;
        
        left->key = newKey;
        left->left = (o_t *) newKey;
        left->data = a;
        left->right = NULL;
        left->prev = temp;
        
        ord->array[a] = left;
        ord->array[b] = right;
        
        temp->right = right;
        temp->left = left;
        //TODO check this bit
        temp->key = right->key;
        
        o_t * update = temp->prev;

        //Increment the insertions counter and renumber keys if needed.
        ord->insertions++;
        if(right->key - left->key < 16 ){
            renumber(ord);
        }
        
        int tmp_height;
        while (temp->prev != NULL) {
            temp = temp->prev; //moved up to the next parent
            //check if heights are equal
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
                //no rotation needed but still need to update height
                if (temp->left->height > temp->right->height)
                    temp->height = temp->left->height + 1;
                else
                    temp->height = temp->right->height + 1;
            }
        }
    }
}

void insert_after(o_t *ord, key_t a, key_t b) {
    o_t * temp = (o_t *) find_iterative(ord, b);
    if(temp != NULL){
        o_t * right = get_node();
        o_t * left = get_node();
        

        o_t * parentOfRight = temp->prev;
        o_t * rightNeighbor;
        long newKey = 0;
        //This means that temp is the root
        if(parentOfRight == NULL){
            //There are nodes to the right of temp
            if(temp->right != NULL){
                rightNeighbor = temp->right;
                while(rightNeighbor->left != NULL){
                    rightNeighbor = rightNeighbor->left;
                }
                //We know temp has a left neighbor so we are going to average their keys
                newKey = (temp->key / 2) + (rightNeighbor->key / 2);
            } else {
                //Temp is the only node, so we treat insert before just like we treat insert top
                newKey = (temp->key / 2) + (MAXKEY / 2);
            }
        } else {
            //The node is not the root, it has a parent
            //This is an easy insert, just look at temps sibling and average
            if(parentOfRight->right->key != temp->key){
                rightNeighbor = parentOfRight->right;
                while(rightNeighbor->right != NULL){
                    rightNeighbor = rightNeighbor->left;
                }
                newKey = (temp->key / 2) + (rightNeighbor->key / 2);
            } else {
                //This is more difficult, temp is the left child, so we need to keep going up until temp is not the left child
                //Go up until we find the root or until temp has something on the left of it
                while(parentOfRight->prev != NULL && parentOfRight->key == parentOfRight->prev->right->key){
                    parentOfRight = parentOfRight->prev;
                }
                
                //Here we didn't find anything to the right of temp, so we will treat this just like insert top
                if(parentOfRight->prev == NULL){
                    newKey = (temp->key / 2) + (MAXKEY / 2);
                } else {
                    rightNeighbor = parentOfRight->right;
                    while(rightNeighbor->right != NULL){
                        rightNeighbor = rightNeighbor->left;
                    }
                    newKey = (temp->key / 2) + (rightNeighbor->key / 2);
                }
            }
        }
        
        left->key = temp->key;
        left->data = temp->data;
        left->left = temp->left;
        left->right = temp->right;
        left->prev = temp;
        
        right->key = newKey;
        right->data = a;
        right->left = (o_t *) newKey;
        right->right = NULL;
        right->prev = temp;
        
        ord->array[a] = right;
        ord->array[b] = left;
        
        temp->right = right;
        temp->left = left;
        //TODO check this bit
        temp->key = right->key;
        
        //Increment the insertions counter and renumber keys if needed.
        ord->insertions++;
        if(right->key - left->key < 16 ){
            renumber(ord);
        }
        
        int tmp_height;
        while (temp->prev != NULL) {
            temp = temp->prev; //moved up to the next parent
            //check if heights are equal
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
                //no rotation needed but still need to update height
                if (temp->left->height > temp->right->height)
                    temp->height = temp->left->height + 1;
                else
                    temp->height = temp->right->height + 1;
            }
        }
    }

}

void insert_bottom(o_t *ord, key_t a){
    o_t* temp = (o_t*)malloc(sizeof(o_t));
    temp = ord;

    if (temp->key == NULL) {
        //insert new node here
        ord->left = (o_t*) MIDKEY;
        ord->right = NULL;
        ord->prev = NULL; 
        ord->key = MIDKEY;
        ord->data = a;
        ord->array[a] = ord;
        ord->height = 0;
        return;
    }

    while (temp->right != NULL) {
        temp = temp->left; 
    }

    //insert new node to the left of the bottom node
    //add new right child of parent node?
    o_t* new_node = (o_t*)malloc(sizeof(o_t));
    o_t* old_node = (o_t*)malloc(sizeof(o_t));
    
    //Calculate the new key value
    long newKey = temp->key / 2;

    new_node->key = newKey;
    old_node->key = temp->key;
    
    new_node->data = a;
    old_node->data = temp->data;

    new_node->left = (o_t*) newKey;
    new_node->right = NULL;

    old_node->left = temp->left;
    old_node->right = temp->right;

    new_node->prev = temp;
    old_node->prev = temp;

    temp->right = old_node;
    temp->left = new_node;
    
    ord->array[a] = new_node;
    ord->array[old_node->data] = old_node;
    
    temp->key = new_node->key;
    
    ord->insertions++;
    if(newKey < 64){
        renumber(ord);
    }
    //add in checks for rotation
    int tmp_height;
    while (temp->prev != NULL) {
        temp = temp->prev; //moved up to the next parent
        //check if heights are equal
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
            //no rotation needed but still need to update height
            if (temp->left->height > temp->right->height)
                temp->height = temp->left->height + 1;
            else
                temp->height = temp->right->height + 1;
        }
    }

    return;
}

void insert_top(o_t* ord, key_t a) {
    o_t* temp = (o_t*)malloc(sizeof(o_t));
    temp = ord;

    if (temp->key == NULL) {
        //insert new node here
        ord->left = (o_t*) MIDKEY;
        ord->right = NULL;
        ord->prev = NULL; 
        ord->key = MIDKEY;
        ord->data = a;
        ord->array[a] = ord;
        ord->height = 0;
        return;
    }

    while (temp->right != NULL) {
        temp = temp->right;
    }

    //insert new node to the left of the bottom node
    //add new right child of parent node?
    o_t* new_node = (o_t*)malloc(sizeof(o_t));
    o_t* old_node = (o_t*)malloc(sizeof(o_t));
    
    long newKey = (temp->key / 2) + MIDKEY;

    new_node->key = newKey;
    old_node->key = temp->key;
    
    new_node->data = a;
    old_node->data = temp->data;

    new_node->left = (o_t *) newKey;
    new_node->right = NULL;

    old_node->left = temp->left;
    old_node->right = temp->right;

    new_node->prev = temp;
    old_node->prev = temp;

    temp->right = new_node;
    temp->left = old_node;
    
    ord->array[a] = new_node;
    ord->array[old_node->data] = old_node;
    
    temp->key = new_node->key;
    
    //Increment the insertions counter and renumber keys if needed.
    ord->insertions++;
    if(newKey > MAXKEY - 32){
        renumber(ord);
    }
    
    int tmp_height;
    while (temp->prev != NULL) {
        temp = temp->prev; //moved up to the next parent
        //check if heights are equal
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
            //no rotation needed but still need to update height
            if (temp->left->height > temp->right->height)
                temp->height = temp->left->height + 1;
            else
                temp->height = temp->right->height + 1;
        }
    }
    return;
}

int is_before(o_t *ord, key_t a, key_t b){
    o_t * node1 = find_iterative(ord, a);
    o_t * node2 = find_iterative(ord, b);
    
    if(node1 == NULL || node2 == NULL){
        return 0;
    }
    
    return node1->key < node2->key;
}

void delete_o(o_t *ord, key_t a) {
    o_t *temp;
    //Check if tree is empty
    if (ord->left == NULL)
        return;
    //check tree with only one node
    else if(ord->right == NULL){
        if(a == ord->key){
            ord->left = NULL;
            ord->height = 0;
        }
    }
    else {
        temp = ord;
        long current_index = temp->key;
        while (temp->right != NULL) {
            if (a < current_index)
                temp = temp->left;
            else
                temp = temp->right;
            current_index = temp->key;
        }
        if (current_index == a) {
            if(temp->key == temp->prev->left->key){
                temp->prev = temp->prev->right;
            } else {
                temp->prev = temp->prev->left;
            }
        }
    }
    
    temp = temp->prev;
    int tmp_height;
    while (temp->prev != NULL) {
        temp = temp->prev; //moved up to the next parent
        //check if heights are equal
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
            //no rotation needed but still need to update height
            if (temp->left->height > temp->right->height)
                temp->height = temp->left->height + 1;
            else
                temp->height = temp->right->height + 1;
        }
    }
}

long p(long q)
{ return( (1247*q +104729) % 300007 );
}

int main()
{  long i; o_t *o; 
   printf("starting \n");
   o = create_order();
   for(i=100000; i>=0; i-- )
      insert_bottom( o, p(i) );
   for(i=100001; i< 300007; i+=2 )
   {  insert_after(o, p(i+1), p(i-1) );
      insert_before( o, p(i), p(i+1) );
   }
   printf("inserted 300000 elements. ");
   for(i = 250000; i < 300007; i++ )
      delete_o( o, p(i) );
   printf("deleted 50000 elements. ");
   insert_top( o, p(300006) );
   for(i = 250000; i < 300006; i++ )
      insert_before( o, p(i) , p(300006) );
   printf("reinserted. now testing order\n");
   for( i=0; i < 299000; i +=42 )
   {  if( is_before( o, p(i), p(i+23) ) != 1 )
      {  printf(" found error (1) \n"); exit(0);
      }
   }
   for( i=300006; i >57; i -=119 )
   {  if( is_before( o, p(i), p(i-57) ) != 0 )
      {  printf(" found error (0) \n"); exit(0);
      }
   }
   printf("finished. no problem found.\n");
} 