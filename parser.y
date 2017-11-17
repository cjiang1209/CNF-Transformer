%code requires {
#include "formula.h"
}

%{
#include <iostream>

int yylex();

#define yyerror printf

#include "formula.h"

Formula* parsed_formula;

%}

%union {
  char* str;
  Formula* formula;
}

%token<str> VAR
%left IFF
%left IMP
%left OR
%left AND
%left NOT

%type<formula> formula

%%

input
  : formula
  {
    parsed_formula = $1;
    return 0;
  }
  ;

formula
  : formula IFF formula
  {
    $$ = new BinaryFormula(OpType::OpIff, $1, $3);
  }
  | formula IMP formula
  {
    $$ = new BinaryFormula(OpType::OpImp, $1, $3);
  }
  | formula OR formula
  {
    $$ = new BinaryFormula(OpType::OpOr, $1, $3);
  }
  | formula AND formula
  {
    $$ = new BinaryFormula(OpType::OpAnd, $1, $3);
  }
  | NOT formula
  {
    $$ = new UnaryFormula(OpType::OpNot, $2);
  }
  | '(' formula ')'
  {
    $$ = $2;
  }
  | VAR
  {
    $$ = new Atom($1);
  }
  ;

%%