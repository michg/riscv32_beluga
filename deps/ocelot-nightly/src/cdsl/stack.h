/*
 *  stack (cdsl)
 */

#ifndef STACK_H
#define STACK_H


/* stack */
typedef struct stack2_t stack2_t;


stack2_t *stack_new(void);
void stack_free(stack2_t **);
void stack_push(stack2_t *, void *);
void *stack_pop(stack2_t *);
void *stack_peek(const stack2_t *);
int stack_empty(const stack2_t *);


#endif    /* STACK_H */

/* end of stack.h */
