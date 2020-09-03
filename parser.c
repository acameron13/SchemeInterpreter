#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "parser.h"
#include "linkedlist.h"
#include "talloc.h"
#include "tokenizer.h"

// Takes a list of tokens from a Racket program, and returns a pointer to a
// parse tree representing that program.
Value *parse(Value *tokens){
  Value *s_list = makeNull();
  Value *stack = makeNull();
  Value *curCons = tokens;
  Value *curTok;
  Value *sub_list = makeNull();
  int depth = 0;

  assert(curCons != NULL && "Error (parse): null pointer");
  //while there are more tokens, create s-expression lists on the stack
  while(curCons->type != NULL_TYPE){
    curTok = car(curCons);
    //keep track of depth (aka # of matching open/close parens)
    if(curTok->type == OPEN_TYPE){
      depth++;
    }
    //push onto stack unless it is a close paren
    if(curTok->type != CLOSE_TYPE){
      stack = cons(curTok, stack);
    }
    //if it is a close paren, pop off of stack until you hit an open paren. Then, push that list onto the stack
    else if(curTok->type == CLOSE_TYPE){
      //empty open-close parens
      if(car(stack)->type == OPEN_TYPE){
        sub_list = makeNull();
      }

      while(car(stack)->type != OPEN_TYPE){
        sub_list = cons(car(stack), sub_list);

        //if the program reaches an empty stack without a matching open paren, then it is a syntax error
        if(cdr(stack)->type == NULL_TYPE){
          printf("Syntax Error: too many close parentheses\n");
          texit(0);
        }

        stack = cdr(stack);
      }
      depth--;
      stack = cdr(stack);
      stack = cons(sub_list, stack);
      sub_list = makeNull();
    }
    curCons = cdr(curCons);
  }

  if(depth != 0){
    printf("Syntax error: not enough close parentheses\n");
    texit(0);
  }

  //put s-expression lists together
  while(stack->type != NULL_TYPE){
    s_list = cons(car(stack), s_list);
    stack = cdr(stack);
  }
  return s_list;
}


// Prints the tree to the screen in a readable fashion. It should look just like
// Racket code; use parentheses to indicate subtrees.
void printSExp(Value *tree){
  Value *cur = tree;
  Value *car;

  printf("(");

  //empty S-Expression
  if((cur->c.cdr->type == NULL_TYPE) && (cur->c.car->type == NULL_TYPE)){
    printf(")");
    return;
  }

  while((cur->c.cdr)->type != NULL_TYPE){
    car = cur->c.car;
    if(car->type == CONS_TYPE){
      printSExp(car);
      printf(" ");
    } else {
      printValue(car);
      printf(" ");
    }      
    cur = cur->c.cdr;
  }
  car = cur->c.car;
  if(car->type == CONS_TYPE){
    printSExp(car);
    printf(")");
  } else {
    printValue(car);
    printf(")");
  }      
}

void printValue(Value *val){
  switch(val->type){
    case CONS_TYPE:
        break;
      case INT_TYPE:
        printf("%i", val->i);
        break;
      case DOUBLE_TYPE:
        printf("%f", val->d);
        break;
      case STR_TYPE:
        printf("\"%s\"", val->s);
        break;
      case NULL_TYPE:
        printf("()");
        break;
      case PTR_TYPE:
        break;
      case OPEN_TYPE:
        break;
      case CLOSE_TYPE:
        break;
      case BOOL_TYPE:
        if(val->i == 1){
          printf("#t");
        }else if(val->i == 0){
          printf("#f");
        }
        break;
      case SYMBOL_TYPE:
        printf("%s", val->s);
        break;
      case VOID_TYPE:
        break;
      case CLOSURE_TYPE:
        printf("closure type\n");
        break;
      case PRIMITIVE_TYPE:
        break;
      case UNSPECIFIED_TYPE:
        printf("#<unspecified>");
        break;
  }
}


// Prints the tree to the screen in a readable fashion. It should look just like
// Racket code; use parentheses to indicate subtrees.
void printTree(Value *tree){
  Value *cur = tree;
  Value *car;

  //empty trees
  if((cur->c.cdr->type == NULL_TYPE) && (cur->c.car->type == NULL_TYPE)){
    printf("(");
    printf(")");
    return;
  }

  while(cdr(cur)->type != NULL_TYPE){
    car = cur->c.car;
    //if there are sub-trees
    if(car->type == CONS_TYPE){
      printSExp(car);
      printf("\n");
    } 
    //if the tree is only one level deep
    else{
      printValue(car);
      printf(" ");
    }
    cur = cdr(cur);
  }
  car = cur->c.car;
  //if there are sub-trees
  if(car->type == CONS_TYPE){
    printSExp(car);
  } 
  //if the tree is only one level deep
  else{
    printValue(car);
  }
}

