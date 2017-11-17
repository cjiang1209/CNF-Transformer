#include <iostream>
#include <cassert>

#include "formula.h"

using namespace std;

extern int yyparse();
extern FILE* yyin;
extern Formula* parsed_formula;

int main(int argc, char* argv[])
{
	assert(argc == 2);

	yyin = fopen(argv[1], "r");
	yyparse();

	Formula* formula = parsed_formula;
	cout << *formula << endl;

	return 0;
}
