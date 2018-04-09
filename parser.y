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
    $$ = FormulaFactory::createIff($1, $3);
  }
  | formula IMP formula
  {
    $$ = FormulaFactory::createImp($1, $3);
  }
  | formula OR formula
  {
    $$ = FormulaFactory::createOr($1, $3);
  }
  | formula AND formula
  {
    $$ = FormulaFactory::createAnd($1, $3);
  }
  | NOT formula
  {
    $$ = FormulaFactory::createNot($2);
  }
  | '(' formula ')'
  {
    $$ = $2;
  }
  | VAR
  {
    $$ = FormulaFactory::createAtom($1);
  }
  ;

%%