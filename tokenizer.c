#include <stdlib.h>
#include <stdio.h>
#include "value.h"
#include "tokenizer.h"
#include "talloc.h"
#include "linkedlist.h"

// Read all of the input from stdin, and return a linked list consisting of the tokens.
//boolean, integer, float, string, symbol, open, close, comments
Value *tokenize(){
  char charRead;
  char charRead2;
  char sign = 'n'; //n for null
  Value *list = makeNull();
  Value *revList;
  Value *newVal;
  Value *curList;
  char *newS;
  char *strToNum = talloc(sizeof(char) *3);
  strToNum[1] = '\0';
  char *garbage;
  int i;
  
  
  charRead = (char)fgetc(stdin);
  while(charRead != EOF){
    //printf("charRead: %c\n", charRead);
    newVal = talloc(sizeof(Value));
    strToNum[0] = charRead;
    //string token
    if(charRead == '"'){
      newVal->type = STR_TYPE;
      newS = talloc(sizeof(char)*301);
      newS[300] = '\0';
      charRead = (char)fgetc(stdin);
      i = 0;
      while(charRead != '"'){
        newS[i] = charRead;
        charRead = (char)fgetc(stdin);
        i++;
      }
      newS[i] = '\0';
      newVal->s = newS;
      list = cons(newVal, list);
    }
    //open token
    else if(charRead == '('){
      newVal->type = OPEN_TYPE;
      list = cons(newVal, list);
    }
    //close token
    else if(charRead == ')'){
      newVal->type = CLOSE_TYPE;
      list = cons(newVal, list);
    }
    //boolean token
    else if(charRead == '#'){
      charRead = (char)fgetc(stdin);
      if(charRead == 't'){
        newVal->type = BOOL_TYPE;
        newVal->i = 1;
      }else if(charRead == 'f'){
        newVal->type = BOOL_TYPE;
        newVal->i = 0;
      }else{
        printf("Error: invalid boolean\n");
        texit(0);
      }
      list = cons(newVal, list);
    }
    //integer and float tokens
    else if(((strtol(strToNum, &garbage, 10) != 0) || ((charRead == '-') || (charRead == '+')) || charRead == '0') && sign != 's'){

      //signed numbers
      if((charRead == '-') || (charRead == '+')){
        sign = charRead;
        charRead = (char)fgetc(stdin);
        strToNum[0] = charRead;
        if((strtol(strToNum, &garbage, 10) == 0) && (charRead != '0')){
          //not a number, must be a symbol
          ungetc(charRead, stdin);
          charRead = sign;
          sign = 's'; //s for symbol
          continue;
        }
      }
      
      i = 1; 
      newS = talloc(sizeof(char)*301);
      newS[0] = charRead;
      newS[300] = '\0';
      charRead = (char)fgetc(stdin);
      strToNum[0] = charRead;
      while((strtol(strToNum, &garbage, 10) != 0) || (charRead == '0')){
        newS[i] = charRead;
        charRead = (char)fgetc(stdin);
        strToNum[0] = charRead;
        i++;
      }
      //float token
      if(charRead == '.'){
        newS[i] = charRead;
        i++;
        newVal->type = DOUBLE_TYPE;
        charRead = (char)fgetc(stdin);
        strToNum[0] = charRead;
        while((strtol(strToNum, &garbage, 10) != 0) || (charRead == '0')){
          newS[i] = charRead;
          charRead = (char)fgetc(stdin);
          strToNum[0] = charRead;
          i++;
        }
        newS[i] = '\0';
        newVal->d = strtod(newS, &garbage);
        if(sign == '-'){
          newVal->d = -(newVal->d);
        }
        sign = 'n'; //reset sign to null
      } 
      //integer token
      else{
        newVal->type = INT_TYPE;
        newS[i] = '\0';
        newVal->i = strtol(newS, &garbage, 10);
        if(sign == '-'){
          newVal->i = -(newVal->i);
        }
        sign = 'n'; //reset sign to null
      }
      charRead = (char)ungetc(charRead, stdin);
      list = cons(newVal, list);
    }
    //comments
    else if(charRead == ';'){
      charRead = (char)fgetc(stdin);
      while(charRead != '\n'){
        charRead = (char)fgetc(stdin);
      }
    }
    //empty line
    else if(charRead == '\n'){
      //do nothing
    }
    //symbol token
    else if(charRead != ' '){
        if(charRead == '@'){
          printf("Error: symbol does not start with an allowed first character\n");
          texit(0);
        }
        newVal->type = SYMBOL_TYPE;
        newS = talloc(sizeof(char)*301);
        newS[300] = '\0';
        newS[0] = charRead;
        charRead = (char)fgetc(stdin);
        i = 1;
        while((charRead != ' ') && (charRead != ')') && (charRead != '\n')){
          newS[i] = charRead;
          charRead = (char)fgetc(stdin);
          i++;
        }
        if((newS[0] == '\'') && (charRead == ' ')){
          printf("Error: incorrect first character after quote\n");
          texit(0);
        }

        ungetc(charRead, stdin);
        newS[i] = '\0';
        newVal->s = newS;
        list = cons(newVal, list);
        sign = 'n'; //reset to null
      } 
      charRead = (char)fgetc(stdin);
  }

  revList = reverse(list);
  return revList;
}

// Displays the contents of the linked list as tokens, with type information
void displayTokens(Value *list){
  Value *cur = list;
  Value *car;
  while(cdr(cur)->type != NULL_TYPE){
    car = cur->c.car;
    switch(car->type){
      case CONS_TYPE:
        printf("cons type\n");
        break;
      case INT_TYPE:
        printf("%i:integer\n", car->i);
        break;
      case DOUBLE_TYPE:
        printf("%f:double\n", car->d);
        break;
      case STR_TYPE:
        printf("\"%s\":string\n", car->s);
        break;
      case NULL_TYPE:
        break;
      case PTR_TYPE:
        break;
      case OPEN_TYPE:
        printf("(:open\n");
        break;
      case CLOSE_TYPE:
        printf("):close\n");
        break;
      case BOOL_TYPE:
        if(car->i == 1){
          printf("#t:boolean\n");
        }else if(car->i == 0){
          printf("#f:boolean\n");
        }
        break;
      case SYMBOL_TYPE:
        printf("%s:symbol\n", car->s);
        break;
      case VOID_TYPE:
        break;
      case CLOSURE_TYPE:
        printf("closure type\n");
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
        break;
      case INT_TYPE:
        printf("%i:integer\n", car->i);
        break;
      case DOUBLE_TYPE:
        printf("%f:float\n", car->d);
        break;
      case STR_TYPE:
        printf("\"%s\":string\n", car->s);
        break;
      case NULL_TYPE:
        break;
      case PTR_TYPE:
        break;
      case OPEN_TYPE:
        printf("(:open\n");
        break;
      case CLOSE_TYPE:
        printf("):close\n");
        break;
      case BOOL_TYPE:
        if(car->i == 1){
          printf("#t:boolean\n");
        }else if(car->i == 0){
          printf("#f:boolean\n");
        }
        break;
      case SYMBOL_TYPE:
        printf("%s:symbol\n", car->s);
        break;
      case VOID_TYPE:
        break;
      case CLOSURE_TYPE:
        printf("closure type\n");
        break;
      case PRIMITIVE_TYPE:
        break;
      case UNSPECIFIED_TYPE:
        break;
    }
}


