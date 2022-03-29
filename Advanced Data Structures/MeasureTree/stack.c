#include <stdio.h>
#include <stdlib.h>
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