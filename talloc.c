#include <stdlib.h>
#include "value.h"
#include "talloc.h"

Value *head = NULL;

// Replacement for malloc that stores the pointers allocated. It should store
// the pointers in some kind of list; a linked list would do fine, but insert
// here whatever code you'll need to do so; don't call functions in the
// pre-existing linkedlist.h. Otherwise you'll end up with circular
// dependencies, since you're going to modify the linked list to use talloc.
void *talloc(size_t size){
  void *ptr = malloc(size);
  Value *ptrVal;
  
  if(head == NULL){
    head = malloc(sizeof(Value));
    head->type = CONS_TYPE;
    head->c.car = ptr;
    head->c.cdr = NULL; 
    return ptr;
  }

  ptrVal = malloc(sizeof(Value));
  ptrVal->type = CONS_TYPE;
  ptrVal->c.car = ptr;
  ptrVal->c.cdr = head->c.cdr;
  head->c.cdr = ptrVal;
  
  return ptr;
}

// Free all pointers allocated by talloc, as well as whatever memory you
// allocated in lists to hold those pointers.
void tfree(){
  Value *cur = head;
  Value *next;

  while(cur != NULL){
    free(cur->c.car);
    next = cur->c.cdr;
    free(cur);
    cur = next;
  }
  head = NULL;
}

// Replacement for the C function "exit", that consists of two lines: it calls
// tfree before calling exit. It's useful to have later on; if an error happens,
// you can exit your program, and all memory is automatically cleaned up.
void texit(int status){
  tfree();
  exit(status);
}