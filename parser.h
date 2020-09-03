#include "value.h"

#ifndef _PARSER
#define _PARSER

// Takes a list of tokens from a Racket program, and returns a pointer to a
// parse tree representing that program.
Value *parse(Value *tokens);

void printSExp(Value *tree);

void printValue(Value *val);
// Prints the tree to the screen in a readable fashion. It should look just like
// Racket code; use parentheses to indicate subtrees.
void printTree(Value *tree);


#endif

