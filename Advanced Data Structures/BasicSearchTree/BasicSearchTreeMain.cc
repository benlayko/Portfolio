#include "basicsearchtree.c"
#include <iostream>
#include <stdlib.h>
#include <string.h>

struct text_t {
    tree_node_t *_text;
    int maxlength;
    int size;
};

/* creates an empty text, whose length is 0 */
text_t *create_text() {
    text_t *txt = new text_t;
    txt->size = 0;
    txt->_text = create_tree();
    return txt;
}

/* returns the number of lines of the current text */
int set_max_length(text_t *txt, int max_length) {
    txt->maxlength = max_length;
    return txt->maxlength;
}

// Returns the number of lines of the current text
int length_text(text_t *txt) {
    if (txt == NULL)
        return -1;
    return txt->size;
}

//
int get_line_size(text_t *txt, int index) {
    if (length_text(txt) < index)
        return -1;
    return static_cast<int>(strlen((char *) find_iterative(txt->_text, index)));
}

/* Gets the line of number index, if such a line exists, and returns NULL else
 * So the algo will be adding/deducting the dist, and if the index can be counted,
 * then return the node when it counts to it.
 * */
char *get_line(text_t *txt, int index) {
    return (char *) find_iterative(txt->_text, index);
}

/* appends new line as new last line if less than maxlength, otherwise it returns null. */
void append_line(text_t *txt, char *new_line) {
    txt->size++;
    if (static_cast<int>(strlen(new_line)) <= txt->maxlength)
        balancedInsert(txt->_text, txt->size, (object_t *)new_line);
}

/* sets the line of number index, if such a line exists, to new line, and returns
 * a pointer to the previous line of that number. If no line of that number exists,
 * it does not change the structure and returns NULL.
 * */
char *set_line(text_t *txt, int index, char *new_line) {
    return (char *) updateNodeRecursive(txt->_text, index, (object_t *) new_line);
}

char *replace_line(text_t *txt, int index, char *new_line) {
    return (char *) replaceNodeRecursive(txt->_text, index, (object_t *) new_line);
}

/* inserts the line before the line of number index, if such a line exists,
 * to new line, renumbering all lines after that line. If no such line exists,
 * it appends new line as new last line. If the length of line is greater than maxlength,
 * it is rejected and NULL is returned.
 * */
void insert_line(text_t *txt, int index, char *new_line) {
    if (static_cast<int>(strlen(new_line)) <= txt->maxlength)
        balancedInsert(txt->_text, index, (object_t *)new_line);
    txt->size++;
}

/* Deletes the line of number index, renumbering all lines after that line,
 * and returns a pointer to the deleted line.
 * */
char *delete_line(text_t *txt, int index) {
    char *old = (char *)_balancedDelete(txt->_text, index);
    if (old != NULL) {
        txt->size--;
    }
    return old;
}

/* appends substring to the first line in the text that has enough space to
 * accommodate the substring (i.e. length of existing line + length of substring ≤ maxlength).
 * Otherwise, it appends substring as a new last line.
 */
void append_to_line(text_t *txt, char *new_substring) {
    int new_substring_size = static_cast<int>(strlen(new_substring));
    int txt_size = length_text(txt);
    int i = 1;

    while (i <= txt_size) {
        int line_size = get_line_size(txt, i);

        if (line_size + new_substring_size <= txt->maxlength) {
            char *new_string = new char[line_size + new_substring_size + 1];
            strcpy(new_string, get_line(txt, i));
            strcat(new_string, new_substring);
            replace_line(txt, i, new_string);
            return;
        }
        i++;
    }

    append_line(txt, new_substring);

    return;
}
