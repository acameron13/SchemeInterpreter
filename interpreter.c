#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "interpreter.h"
#include "value.h"
#include "tokenizer.h"
#include "talloc.h"
#include "parser.h"
#include "linkedlist.h"


Frame *parent;

void interpret(Value *tree){
  Value *cur = tree;
  Value *sub_tree;
  Value *result;

  parent = talloc(sizeof(Frame));
  parent->bindings = makeNull();
  parent->parent = NULL;

  assert(tree!= NULL && "Evaluation Error: null pointer\n");

  bind("+", primitiveAdd, parent);
  bind("null?", primitiveNull, parent);
  bind("car", primitiveCar, parent);
  bind("cdr", primitiveCdr, parent);
  bind("cons", primitiveCons, parent);
  bind("-", primitiveSubtract, parent);
  bind("<", primitiveLessThan, parent);
  bind(">", primitiveGreaterThan, parent);
  bind("=", primitiveEqual, parent);

  while(cur->type != NULL_TYPE){
    sub_tree = car(cur);
    result = eval(sub_tree, parent);
    //printf("%i\n", result->type);
    if(result->type == VOID_TYPE){
      //do nothing
    } else if (result->type == CONS_TYPE){
      printCons(result);
      printf("\n");
    }else if(result->type == CLOSURE_TYPE){
      printf("#<procedure>\n");
    } else{
      //printf("type: %i\n", result->type);
      printValue(result);
      printf("\n");
    }
    cur = cdr(cur);
  }
}


Value *eval(Value *expr, Frame *frame){
  Value *first;
  Value *args;
  Value *newVal;
  Frame *newF;
  Value *result;
  Value *closure;
  Value *evaledOperator;
  Value *evaledArgs;
  
  switch(expr->type){
    case INT_TYPE:
      return expr;
      break;
    case DOUBLE_TYPE:
      return expr;
      break;
    case STR_TYPE:
      return expr;
      break;
    case CONS_TYPE:
      first = car(expr);
      args = cdr(expr);
      
      assert(first != NULL && "Evaluation Error: null pointer\n");
      
      if(first->type == SYMBOL_TYPE){
        if (!strcmp(first->s,"if")) {
          result = evalIf(args,frame);
          return result;
        }else if (!strcmp(first->s, "let")){
          if(cdr(args)->type == NULL_TYPE){
            printf("Evaluation Error: no args following the bindings in let\n");
            texit(0);
          } else if((car(args)->type != NULL_TYPE) && (car(args)->type != CONS_TYPE)){
            printf("Evaluation Error: bad form in let\n");
            texit(0);
          }
          if(args->c.car->c.car != NULL){
            newF = talloc(sizeof(Frame));
            newF->bindings = buildBindings(car(args), frame);
            newF->parent = frame;
            args = cdr(args);
            while(cdr(args)->type != NULL_TYPE){
              eval(car(args), newF);
              args = cdr(args);
            }
            return eval(car(args), newF);
          } else {
            args = cdr(args);
            while(cdr(args)->type != NULL_TYPE){
              eval(car(args), frame);
              args = cdr(args);
            }
            return eval(car(args), frame);
          }
        }else if (!strcmp(first->s, "let*")){
          if(cdr(args)->type == NULL_TYPE){
            printf("Evaluation Error: no args following the bindings in let*\n");
            texit(0);
          } else if((car(args)->type != NULL_TYPE) && (car(args)->type != CONS_TYPE)){
            printf("Evaluation Error: bad form in let*\n");
            texit(0);
          }
          //arguments in let*
          if(args->c.car->c.car != NULL){
            newF = letStarFrame(car(args), frame);
            args = cdr(args);
            while(cdr(args)->type != NULL_TYPE){
              eval(car(args), newF);
              args = cdr(args);
            }
            return eval(car(args), newF);
          }
          //no arguments in let* 
          else {
            args = cdr(args);
            while(cdr(args)->type != NULL_TYPE){
              eval(car(args), frame);
              args = cdr(args);
            }
            return eval(car(args), frame);
          }
        }else if (!strcmp(first->s, "letrec")){
          if(cdr(args)->type == NULL_TYPE){
            printf("Evaluation Error: no args following the bindings in letrec\n");
            texit(0);
          } else if((car(args)->type != NULL_TYPE) && (car(args)->type != CONS_TYPE)){
            printf("Evaluation Error: bad form in letrec\n");
            texit(0);
          }
          //arguments in letrec
          if(args->c.car->c.car != NULL){
            newF = letrecFrame(car(args), frame);
            args = cdr(args);
            while(cdr(args)->type != NULL_TYPE){
              eval(car(args), newF);
              args = cdr(args);
            }
            return eval(car(args), newF);
          }
          //no arguments in letrec 
          else {
            args = cdr(args);
            while(cdr(args)->type != NULL_TYPE){
              eval(car(args), frame);
              args = cdr(args);
            }
            return eval(car(args), frame);
          }
        }else if(!strcmp(first->s, "quote")){
          if (cdr(args)->type != NULL_TYPE){
            printf("Evaluation Error: bad form in quote\n");
            texit(0);
          }
          return car(args);
        }else if(!strcmp(first->s, "define")){
          if(args->type == NULL_TYPE){
            printf("Evaluation Error: no arguments following define\n");
            texit(0);
          } else if(car(args)->type != SYMBOL_TYPE){
            printf("Evaluation Error: define must bind to a symbol\n");
            texit(0);
          } else if(cdr(args)->type == NULL_TYPE){
            printf("Evaluation Error: no value following %s in define\n", car(args)->s);
            texit(0);
          }
          newVal = eval(car(cdr(args)), frame);
          //modify global frame with a binding of var to expr
          parent->bindings = cons(newVal, parent->bindings);
          parent->bindings = cons(car(args), parent->bindings);
          //return a void type
          result = makeNull();
          result->type = VOID_TYPE;
          return result;
          //(lambda params body)
        }else if (!strcmp(first->s, "set!")){
          if(args->type == NULL_TYPE){
            printf("Evaluation Error: no arguments following set!\n");
            texit(0);
          } else if(car(args)->type != SYMBOL_TYPE){
            printf("Evaluation Error: set! must bind to a symbol\n");
            texit(0);
          } else if(cdr(args)->type == NULL_TYPE){
            printf("Evaluation Error: no value following %s in set!\n", car(args)->s);
            texit(0);
          }
          setBang(args, frame);

          //return a void type
          result = makeNull();
          result->type = VOID_TYPE;
          return result;
        }else if(!strcmp(first->s, "lambda")){
          if(args->type == NULL_TYPE){
            printf("Evaluation error: no args following lambda.\n");
            texit(0);
          }
          else if(cdr(args)->type == NULL_TYPE){
            printf("Evaluation error: no code in lambda following parameters.\n");
            texit(0);
          }
          closure = talloc(sizeof(Value));
          closure->type = CLOSURE_TYPE;
          closure->cl.frame = frame;
          checkParams(car(args));
          closure->cl.paramNames = car(args);
          closure->cl.functionCode = car(cdr(args));
          return closure;
        }else if(!strcmp(first->s, "begin")){
          if(args->type == NULL_TYPE){
            result = makeNull();
            result->type = VOID_TYPE;
            return result;
          }
          while(cdr(args)->type != NULL_TYPE){
            eval(car(args), frame);
            args = cdr(args);
          }
          return eval(car(args), frame);
        }else if(!strcmp(first->s, "and")){
          result = talloc(sizeof(Value));
          result->type = BOOL_TYPE;
          
          if(args->type == NULL_TYPE){
            printf("Evaluation error: no args following and.\n");
            texit(0);
          }

          while(args->type != NULL_TYPE){
            newVal = eval(car(args), frame);
            if(newVal->i == 0){
              result->i = 0;
              return result;
            }
            args = cdr(args);
          }
          result->i = 1;
          return result;
        }else if(!strcmp(first->s, "or")){
          result = talloc(sizeof(Value));
          result->type = BOOL_TYPE;
          result->i = 0;

          if(args->type == NULL_TYPE){
            printf("Evaluation error: no args following or.\n");
            texit(0);
          }

          while(args->type != NULL_TYPE){
            newVal = eval(car(args), frame);
            if(newVal->i == 1){
              result->i = 1;
              return result;
            }
            args = cdr(args);
          }
          return result;
        }else {
          evaledOperator = eval(first, frame);
          evaledArgs = evalEach(args, frame);
          return apply(evaledOperator, evaledArgs);
        }
      } else {
        evaledOperator = eval(first, frame);
        evaledArgs = evalEach(args, frame);
        return apply(evaledOperator, evaledArgs);
      }
      break;
    case NULL_TYPE:
      break;
    case PTR_TYPE:
      break;
    case OPEN_TYPE:
      break;
    case CLOSE_TYPE:
      break;
    case BOOL_TYPE:
      return expr;
      break;
    case SYMBOL_TYPE:
      return lookUpSymbol(expr, frame);
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
  return makeNull();
}

Value *lookUpSymbol(Value *expr, Frame *frame){
  //search current frame
  Value *list = frame->bindings;
  Value *curSym;
  Value *val;

  while(list->type != NULL_TYPE){
    curSym = car(list);
    if(curSym->type == SYMBOL_TYPE){
      if(!strcmp(curSym->s, expr->s)){
        val = car(cdr(list));
        return val;
      }
    }
    list = cdr(list);
  }
  //if it doesn't exist, recursively search the parents
  if(frame->parent == NULL){
    printf("Evaluation Error: symbol %s not defined\n", expr->s);
    texit(0);
  }
  return lookUpSymbol(expr, frame->parent);
}

Value *evalIf(Value *expr, Frame *frame){
  Value *test = car(expr);
  Value *trueExpr;
  Value *falseExpr;
  Value *result;
  
  //printf("EvalIf\n");

  if((expr->c.cdr->type == NULL_TYPE) || (expr->c.cdr->c.cdr->type == NULL_TYPE)){
    printf("Evaluation error: not enough arguments in if statement\n");
    texit(0);
  }

  trueExpr = car(cdr(expr));
  falseExpr = car(cdr(cdr(expr)));
  result = eval(test, frame);

  if(result->i == 1){
    return eval(trueExpr, frame);
  } else if(result->i == 0){
    return eval(falseExpr, frame);
  } else {
    printf("Evaluation error: invalid boolean expression\n");
    texit(0);
  }
  return makeNull();
}

Value *buildBindings(Value *list, Frame *frame){
  Value *cur = list;
  Value *var_pair;
  Value *var;
  Value *val;
  Value *bindings = makeNull();
  Value *vars_defined = makeNull();
  Value *curVar = vars_defined;

  if(cdr(list)->type == NULL_TYPE){
    if(car(list)->type == NULL_TYPE){
      return bindings;
    } else if(car(list)->type != CONS_TYPE){
      printf("Evaluation Error: not enough arguments in let\n");
      texit(0);
    } 
  }

  while(list->type != NULL_TYPE){
    var_pair = car(list);
    var = car(var_pair);
    if(var->type == NULL_TYPE){
      printf("Evaluation error: null binding in let.\n");
      texit(0);
    } else if(var->type != SYMBOL_TYPE){
      printf("Evaluation error: left side of a let pair doesn't have a variable.\n");
      texit(0);
    }
    while(curVar->type != NULL_TYPE){
      if(!strcmp(car(curVar)->s, var->s)){
        printf("Evaluation error: duplicate variable in let\n");
        texit(0);
      }
      curVar = cdr(curVar);
    }
    val = eval(car(cdr(var_pair)), frame);
    if(val->type == SYMBOL_TYPE){
      val = lookUpSymbol(val, frame);
    }
    vars_defined = cons(var, vars_defined);
    curVar = vars_defined;
    bindings = cons(val, bindings);
    bindings = cons(var, bindings);
    list = cdr(list);
  }
  return bindings;
}

Value *evalEach(Value *args, Frame *frame){
  Value *list = reverse(args);
  Value *cur;
  Value *evaledArgs = makeNull();
  Value *curArg;

  while(list->type != NULL_TYPE){
    cur = car(list);
    curArg = eval(cur, frame);
    evaledArgs = cons(curArg, evaledArgs);
    list = cdr(list);
  }
  return evaledArgs;
}

Value *apply(Value *function, Value *args){
  Frame *newF = talloc(sizeof(Frame));
  Value *params;
  Value *formalParam;
  Value *actualParam;

  if(function->type == PRIMITIVE_TYPE){
    return function->pf(args);
  }
  else if(function->cl.paramNames != NULL){
    if(args == NULL){
      printf("Evaluation Error: not enough arguments\n");
      texit(0);
    }
    if(car(function->cl.paramNames)->type == NULL_TYPE){
      params = makeNull();
    } else {
    params = function->cl.paramNames;
    formalParam = car(params);
    actualParam = car(args);
    }
  }else {
    params = makeNull();
  }

  newF->parent = function->cl.frame;
  newF->bindings = makeNull();

  while(params->type != NULL_TYPE){
    newF->bindings = cons(actualParam, newF->bindings);
    newF->bindings = cons(formalParam, newF->bindings);
    params = cdr(params);
    formalParam = car(params);
    if(args->type != NULL_TYPE){
      args = cdr(args);
      actualParam = car(args);
    }
  }
  return eval(function->cl.functionCode, newF);

}

int checkParams(Value *params){
  Value *vars_defined = makeNull();
  Value *curVar = car(params);
  Value *var_list;

  while(params->type != NULL_TYPE){
    if (curVar->type != SYMBOL_TYPE && curVar->type != NULL_TYPE){
      printf("Evaluation error: formal parameters for lambda must be symbols.\n");
      texit(0);
    } else {
      var_list = vars_defined;
      while(var_list->type != NULL_TYPE){
        if(!strcmp(curVar->s, car(var_list)->s)){
          printf("Evaluation error: duplicate identifier in lambda.\n");
          texit(0);
        }
        var_list = cdr(var_list);
      }
      vars_defined = cons(curVar, vars_defined);
      params = cdr(params);
      curVar = car(params);
    }
  }
  return 1;
}

Value *primitiveAdd(Value *args){
  Value *result = talloc(sizeof(Value));
  int iSum = 0;
  double dSum = 0;
  result->type = INT_TYPE;
  
  while(args->type != NULL_TYPE){
    if (car(args)->type == DOUBLE_TYPE){
      dSum = dSum + (double)iSum + car(args)->d;
      iSum = 0;
      result->type = DOUBLE_TYPE;
    }else if(car(args)->type == INT_TYPE){
      iSum = iSum + car(args)->i;
    }else{
      printf("Evaluation Error: + must take numbers.\n");
      texit(0);
    }
    args = cdr(args);
  }
  if(result->type == DOUBLE_TYPE){
    result->d = dSum;
  }else if (result->type == INT_TYPE){
    result->i = iSum;
  }
  return result;
}

Value *primitiveSubtract(Value *args){
  Value *result = talloc(sizeof(Value));
  int iSum;
  double dSum;
  result->type = INT_TYPE;

  if(args->type != CONS_TYPE){
    printf("Debug Error: non-cons type passed in to subtract\n");
    texit(0);
  }

  if(car(args)->type == INT_TYPE){
    iSum = car(args)->i;
    dSum = 0;
  } else if(car(args)->type == DOUBLE_TYPE){
    dSum = car(args)->d;
    iSum = 0;
  }

  args = cdr(args);
  
  while(args->type != NULL_TYPE){
    if (car(args)->type == DOUBLE_TYPE){
      dSum = dSum - (double)iSum - car(args)->d;
      iSum = 0;
      result->type = DOUBLE_TYPE;
    }else if(car(args)->type == INT_TYPE){
      iSum = iSum - car(args)->i;
    }else{
      printf("Evaluation Error: + must take numbers.\n");
      texit(0);
    }
    args = cdr(args);
  }
  if(result->type == DOUBLE_TYPE){
    result->d = dSum;
  }else if (result->type == INT_TYPE){
    result->i = iSum;
  }
  return result;
}

Value *primitiveLessThan(Value *args){
  Value *result = talloc(sizeof(Value));
  result->type = BOOL_TYPE;

  if((args->type != CONS_TYPE) || (length(args)!=2)){
    printf("Evaluation Error: wrong number of arguments to <.\n");
    texit(0);
  } 

  if(car(args)->type == INT_TYPE && car(cdr(args))->type == INT_TYPE){
    if (car(args)->i < car(cdr(args))->i){
      result->i = 1;
    } else{
      result->i = 0;
    }
  }else if(car(args)->type == INT_TYPE && car(cdr(args))->type == DOUBLE_TYPE){
    if (car(args)->i < car(cdr(args))->d){
      result->i = 1;
    } else{
      result->i = 0;
    }
  }else if(car(args)->type == DOUBLE_TYPE && car(cdr(args))->type == INT_TYPE){
    if (car(args)->d < car(cdr(args))->i){
      result->i = 1;
    } else{
      result->i = 0;
    }
  }else if(car(args)->type == DOUBLE_TYPE && car(cdr(args))->type == DOUBLE_TYPE){
    if (car(args)->d < car(cdr(args))->d){
      result->i = 1;
    } else{
      result->i = 0;
    }
  } else {
    printf("Evaluation Error: incorrect type arguments to <.\n");
    texit(0);
  }
  return result;
}

Value *primitiveGreaterThan(Value *args){
  Value *result = talloc(sizeof(Value));
  result->type = BOOL_TYPE;

  if((args->type != CONS_TYPE) || (length(args)!=2)){
    printf("Evaluation Error: wrong number of arguments to >.\n");
    texit(0);
  }

  if(car(args)->type == INT_TYPE && car(cdr(args))->type == INT_TYPE){
    if (car(args)->i > car(cdr(args))->i){
      result->i = 1;
    } else{
      result->i = 0;
    }
  }else if(car(args)->type == INT_TYPE && car(cdr(args))->type == DOUBLE_TYPE){
    if (car(args)->i > car(cdr(args))->d){
      result->i = 1;
    } else{
      result->i = 0;
    }
  }else if(car(args)->type == DOUBLE_TYPE && car(cdr(args))->type == INT_TYPE){
    if (car(args)->d > car(cdr(args))->i){
      result->i = 1;
    } else{
      result->i = 0;
    }
  }else if(car(args)->type == DOUBLE_TYPE && car(cdr(args))->type == DOUBLE_TYPE){
    if (car(args)->d > car(cdr(args))->d){
      result->i = 1;
    } else{
      result->i = 0;
    }
  } else {
    printf("Evaluation Error: incorrect type arguments to >.\n");
    texit(0);
  }
  return result;
}

Value *primitiveEqual(Value *args){
  Value *result = talloc(sizeof(Value));
  result->type = BOOL_TYPE;

  if((args->type != CONS_TYPE) || (length(args)!=2)){
    printf("Evaluation Error: wrong number of arguments to =.\n");
    texit(0);
  }

  if(car(args)->type == INT_TYPE && car(cdr(args))->type == INT_TYPE){
    if (car(args)->i == car(cdr(args))->i){
      result->i = 1;
    } else{
      result->i = 0;
    }
  }else if(car(args)->type == INT_TYPE && car(cdr(args))->type == DOUBLE_TYPE){
    if (car(args)->i == car(cdr(args))->d){
      result->i = 1;
    } else{
      result->i = 0;
    }
  }else if(car(args)->type == DOUBLE_TYPE && car(cdr(args))->type == INT_TYPE){
    if (car(args)->d == car(cdr(args))->i){
      result->i = 1;
    } else{
      result->i = 0;
    }
  }else if(car(args)->type == DOUBLE_TYPE && car(cdr(args))->type == DOUBLE_TYPE){
    if (car(args)->d == car(cdr(args))->d){
      result->i = 1;
    } else{
      result->i = 0;
    }
  }else{
    printf("Evaluation Error: incorrect type arguments to =.\n");
    texit(0);
  }
  return result;
}

Value *primitiveNull(Value *args){
  Value *result = talloc(sizeof(Value));
  result->type = BOOL_TYPE;

  if((args->type == NULL_TYPE) || (cdr(args)->type != NULL_TYPE)){
    printf("Evaluation Error: Incorrect arguments supplied to null.\n");
    texit(0);
  }
 
  if((args->type == CONS_TYPE) && (length(args) == 1) && (car(args)->type == NULL_TYPE)){
    result->i = 1;
  } else {
    result->i = 0;
  }
  return result;
}

Value *primitiveCar(Value *args){
  Value *result;

  if ((args->type != CONS_TYPE) || (length(args) != 1) || (car(args)->type != CONS_TYPE)){
    printf("Evaluation Error: incorrect arguments for car\n");
    texit(0);
  }

  result = car(car(args));

  return result;
}

Value *primitiveCdr(Value *args){
  Value *result;

  if (args->type != CONS_TYPE || length(args) != 1){
    printf("Evaluation Error: incorrect arguments for cdr\n");
    texit(0);
  }
  result = cdr(car(args));
  return result;
}

Value *primitiveCons(Value *args){
  Value *result;
  Value *first;
  Value *rest;

  if (args->type != CONS_TYPE || args->type == NULL_TYPE || length(args) != 2){
    printf("Evaluation Error: incorrect parameters to cons\n");
    texit(0);
  }
  first = car(args);
  rest = car(cdr(args));

  result = cons(first, rest);
  return result;
}

void bind(char *name, Value *(*function)(struct Value *), Frame *frame) {
    // Add primitive functions to top-level bindings list
    Value *nameVal = talloc(sizeof(Value));
    Value *value = talloc(sizeof(Value));

    value->type = PRIMITIVE_TYPE;
    value->pf = function;
    nameVal->type = SYMBOL_TYPE;
    nameVal->s = name;
    frame->bindings = cons(value, frame->bindings);
    frame->bindings = cons(nameVal, frame->bindings);
}

void printCons(Value *list){
  Value *first;
  Value *rest;
  
  if (list->type != CONS_TYPE){
    printf("Debug error: how did you get here?\n");
    texit(0);
  }
  printf("(");
  if((cdr(list)->type != CONS_TYPE) && (cdr(list)->type != NULL_TYPE)){
    printDottedPair(list);
    printf(")");
    return;
  }
  
  
  while(cdr(list)->type != NULL_TYPE){
    first = car(list);
    //if the car of the list is a cons type
    if(first->type == CONS_TYPE){
      //dotted pair
      if((cdr(first)->type != CONS_TYPE) && (cdr(first)->type != NULL_TYPE)){
        printf("(");
        printDottedPair(first);
        printf(")");
      }
      else{
        printCons(first);
      }
    } else {
        printValue(first);
    }
    printf(" ");
    rest = cdr(list);
    //if the last thing is a dotted pair
    if((cdr(rest)->type != CONS_TYPE) && (cdr(rest)->type != NULL_TYPE)){
      printDottedPair(rest);
      printf(")");
      return;
    } else{
      list = cdr(list);
    }
  }
  first = car(list);
  if(first->type == CONS_TYPE){
      printCons(first);
  } else {
      printValue(first);
  }
  printf(")");
}

void printDottedPair(Value *list){
  if (list->type != CONS_TYPE){
    printf("Debug error: how did you get here?\n");
    texit(0);
  }

  printValue(car(list));
  printf(" . ");
  printValue(cdr(list));
}

Frame *letStarFrame(Value *args, Frame *frame){
  Frame *newF;
  Value *var_pair;
  Value *cur = args;
  Value *var;
  Value *val;
  Value *bindings = makeNull();
  Value *vars_defined = makeNull();
  Value *curVar = vars_defined;

  //error checking
  if(cdr(args)->type == NULL_TYPE){
    if(car(args)->type == NULL_TYPE){
      newF->bindings = bindings;
      newF->parent = frame;
      return newF;
    } else if(car(args)->type != CONS_TYPE){
      printf("Evaluation Error: not enough arguments in let*\n");
      texit(0);
    } 
  }
  while(args->type != NULL_TYPE){
    newF = talloc(sizeof(Frame));
    //frame stuff
    var_pair = car(args);
    var = car(var_pair);
    if(var->type == NULL_TYPE){
      printf("Evaluation error: null binding in let.\n");
      texit(0);
    } else if(var->type != SYMBOL_TYPE){
      printf("Evaluation error: left side of a let pair doesn't have a variable.\n");
      texit(0);
    }
    while(curVar->type != NULL_TYPE){
       if(!strcmp(car(curVar)->s, var->s)){
         printf("Evaluation error: duplicate variable in let\n");
        texit(0);
      }
      curVar = cdr(curVar);
    }
    val = eval(car(cdr(var_pair)), frame);
    if(val->type == SYMBOL_TYPE){
      val = lookUpSymbol(val, frame);
    }
    vars_defined = cons(var, vars_defined);
    curVar = vars_defined;
    bindings = cons(val, bindings);
    bindings = cons(var, bindings);
    newF->parent = frame;
    newF->bindings = bindings;
    frame = newF;
    bindings = makeNull();
    args = cdr(args);
  }
  return newF;
}

Frame *letrecFrame(Value *args, Frame *frame){
  Frame *newF = talloc(sizeof(Frame));
  Value *bindings = makeNull();
  Value *list = args;
  Value *var_pair;
  Value *var;
  Value *val;
  Value *evaled_list = makeNull();

  while(list->type != NULL_TYPE){
    var_pair = car(list);
    var = car(var_pair);
    if(var->type == NULL_TYPE){
      printf("Evaluation error: null binding in let.\n");
      texit(0);
    } else if(var->type != SYMBOL_TYPE){
      printf("Evaluation error: left side of a let pair doesn't have a variable.\n");
      texit(0);
    }
    val = talloc(sizeof(Value));
    val->type = UNSPECIFIED_TYPE;
    bindings = cons(val, bindings);
    bindings = cons(var, bindings);
    list = cdr(list);
  }
  newF->bindings = bindings;
  list = args;

  while(list->type != NULL_TYPE){
    var_pair = car(list);
    val = car(cdr(var_pair));
    val = eval(val, newF);
    if(val->type == SYMBOL_TYPE){
      val = lookUpSymbol(val, newF);
    }
    evaled_list = cons(val, evaled_list);
    list = cdr(list);
  }

  list = newF->bindings;
  bindings = makeNull();
  while(list->type != NULL_TYPE){
    val = car(evaled_list);
    var = car(list);
    bindings = cons(val, bindings);
    bindings = cons(var, bindings);
    evaled_list = cdr(evaled_list);
    list = cdr(list);
    if(list->type == CONS_TYPE){
      list = cdr(list);
    }
  }
  newF->bindings = bindings;
  newF->parent = frame;
  return newF;
}

void setBang(Value *args, Frame *frame){
  Value *newVal = eval(car(cdr(args)), frame);
  Value *symToChange = car(args);
  Value *list = frame->bindings;
  Value *curSym;
  Value *val;
  Value *newBindings = makeNull();
  int found = 0;

  while(list->type != NULL_TYPE){
    curSym = car(list);
    val = car(cdr(list));
    if(curSym->type == SYMBOL_TYPE){
      if(!strcmp(curSym->s, symToChange->s)){
        found = 1;
        val = newVal;
      }
    }
    newBindings = cons(val, newBindings);
    newBindings = cons(curSym, newBindings);
    list = cdr(list);
    if(list->type != NULL_TYPE){
      list = cdr(list);
    }
  }

  //if it doesn't exist, recursively search the parents
  if(found == 0){
    if(frame->parent == NULL){
      printf("Evaluation Error: symbol %s not defined\n", symToChange->s);
      texit(0);
    }
    setBang(args, frame->parent);
  }else{
    frame->bindings = newBindings;
  }
}
