#include "value.h"
#include "tokenizer.h"
#include "talloc.h"
#include "parser.h"
#include "linkedlist.h"

#ifndef _INTERPRETER
#define _INTERPRETER

void interpret(Value *tree);

Value *eval(Value *expr, Frame *frame);

Value *lookUpSymbol(Value *expr, Frame *frame);

Value *evalIf(Value *expr, Frame *frame);

Value *buildBindings(Value *list, Frame *frame);

Value *evalEach(Value *args, Frame *frame);

Value *apply(Value *function, Value *args);

int checkParams(Value *params);

Value *primitiveAdd(Value *args);

Value *primitiveNull(Value *args);

Value *primitiveCar(Value *args);

Value *primitiveCdr(Value *args);

Value *primitiveCons(Value *args);

void bind(char *name, Value *(*function)(struct Value *), Frame *frame);

void printCons(Value *list);

void printDottedPair(Value *list);

Value *primitiveSubtract(Value *args);

Value *primitiveLessThan(Value *args);

Value *primitiveGreaterThan(Value *args);

Value *primitiveEqual(Value *args);

Frame *letStarFrame(Value *args, Frame *frame);

Frame *letrecFrame(Value *args, Frame *frame);

void setBang(Value *args, Frame *frame);

#endif