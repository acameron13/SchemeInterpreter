#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "value.h"
#include "talloc.h"
#include "linkedlist.h"

// Create a new NULL_TYPE value node.
Value *makeNull(){
  Value *nullValue = talloc(sizeof(Value));
  nullValue->type = NULL_TYPE;
  return nullValue;
}

// Create a new CONS_TYPE value node.
Value *cons(Value *newCar, Value *newCdr){
  Value *consVal = talloc(sizeof(Value));
  consVal->type = CONS_TYPE;
  consVal->c.car = newCar;
  consVal->c.cdr = newCdr;
  return consVal;
}

// Display the contents of the linked list to the screen in some kind of
// readable format
void display(Value *list){
  Value *cur = list;
  Value *car;
  printf("(");
  while((cdr(cur))->type != NULL_TYPE){
    car = cur->c.car;
    switch(car->type){
      case CONS_TYPE:
        display(car);
        break;
      case INT_TYPE:
        printf("%i, ", car->i);
        break;
      case DOUBLE_TYPE:
        printf("%f, ", car->d);
        break;
      case STR_TYPE:
        printf("%s, ", car->s);
        break;
      case NULL_TYPE:
        printf("null, ");
        break;
      case PTR_TYPE:
        printf("%p, ", car->p);
        break;
      case OPEN_TYPE:
        break;
      case CLOSE_TYPE:
        break;
      case BOOL_TYPE:
        break;
      case SYMBOL_TYPE:
        break;
      case VOID_TYPE:
        break;
      case CLOSURE_TYPE:
        break;
      case PRIMITIVE_TYPE:
        break;
      case UNSPECIFIED_TYPE:
        break;
    }
    cur = cdr(cur);
  }
  car = cur->c.car;
    switch(car->type){
      case CONS_TYPE:
        display(car);
        break;
      case INT_TYPE:
        printf("%i)\n", car->i);
        break;
      case DOUBLE_TYPE:
        printf("%f)\n", car->d);
        break;
      case STR_TYPE:
        printf("%s)\n", car->s);
        break;
      case NULL_TYPE:
        printf("null)\n");
        break;
      case PTR_TYPE:
        printf("%p)\n", car->p);
        break;
      case OPEN_TYPE:
        break;
      case CLOSE_TYPE:
        break;
      case BOOL_TYPE:
        break;
      case SYMBOL_TYPE:
        break;
      case VOID_TYPE:
        break;
      case CLOSURE_TYPE:
        break;
      case PRIMITIVE_TYPE:
        break;
      case UNSPECIFIED_TYPE:
        break;
    }
}

// Return a new list that is the reverse of the one that is passed in. No stored
// data within the linked list should be duplicated; rather, a new linked list
// of CONS_TYPE nodes should be created, that point to items in the original
// list.
Value *reverse(Value *list){
  Value *head;
  Value *oldCur = list;
  Value *oldNext = list->c.cdr;
  Value *newCur;
  Value *newNext;

  if(list->type == NULL_TYPE){
    head = makeNull();
    return head;
  }
  
  newNext = cons(oldCur->c.car, makeNull());
  while((oldCur->c.cdr)->type != NULL_TYPE){
    newCur = cons(oldNext->c.car, newNext);

    oldCur = oldNext;
    oldNext = oldNext->c.cdr;
    newNext = newCur;    
  }
  head = newNext;
  return head;
}

// Utility to make it less typing to get car value. Use assertions to make sure
// that this is a legitimate operation.
Value *car(Value *list){
  assert(list != NULL);
  Value *car = list->c.car;
  return car;
}

// Utility to make it less typing to get cdr value. Use assertions to make sure
// that this is a legitimate operation.
Value *cdr(Value *list){
  assert(list != NULL);
  Value *cdr = list->c.cdr;
  return cdr; 
}

// Utility to check if pointing to a NULL_TYPE value. Use assertions to make sure
// that this is a legitimate operation.
bool isNull(Value *value){
  assert(value != NULL);
  if (value->type == NULL_TYPE){
    return true;
  }
  return false;
}

// Measure length of list. Use assertions to make sure that this is a legitimate
// operation.
int length(Value *value){
  int length = 0;
  assert(value != NULL);

  while(isNull(value) != true){
    length++;
    value = cdr(value);
  }
  
  return length;
}
