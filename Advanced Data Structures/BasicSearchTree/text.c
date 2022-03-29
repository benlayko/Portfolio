#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "basicsearchtree.c"


typedef struct txt_t{
    struct tree_node_t * root;
    int length;
}text_t;

int MAX_LENGTH = 100;
/**
    creates an empty text, whose length is 0.
    */
text_t * create_text(){
    text_t *text;
    text->root = create_tree();
    text->length = 0;
    return text;
}
/**
    returns the number of lines of the current text.
    */
int length_text(text_t * txt){
    return txt->length;
}

/**
    gets the line of number index, if such a line exists, and
    returns NULL else.
    */
char * get_line( text_t *txt, int index){
    return find_recursive(txt->root, index);
}

/**
     appends new line as new last line if less than
    maxlength, otherwise it returns null.
    */
void append_line( text_t *txt, char * new_line){
    if(strlen(new_line) <= MAX_LENGTH){
        txt->root = insert(txt->root,txt->length, new_line);
        txt->length = txt->length + 1;
    } else {
        return NULL;
    }
}
/**
    sets the line of number index, if such
    a line exists, to new line, and returns a pointer to the previous line of that number. If no line of
    that number exists, it does not change the structure and returns NULL.
    */
char * set_line( text_t *txt, int index, char * new_line){
    return NULL;
}

/**
    inserts the line before the line of
    number index, if such a line exists, to new line, renumbering all lines after that line. If no such
    line exists, it appends new line as new last line. If the length of line is greater than maxlength, it
    is rejected and NULL is returned.
    */
void insert_line( text_t *txt, int index, char * new_line){
    return NULL;
}
/**
    deletes the line of number index, renumbering all
    lines after that line, and returns a pointer to the deleted line.
    */
char * delete_line( text_t *txt, int index){
    return NULL;
}
/**
    appends substring to the first line in
    the text that has enough space to accommodate the substring (i.e. length of existing line + length
    of substring ≤ maxlength). Otherwise, it appends substring as a new last line.
    */
void append_to_line( text_t *txt, char * new_substring){
    return NULL;
}