#include <iostream>
#include <cassert>

#include "base.h"
#include "formula.h"
#include "symbol_table.h"

using namespace std;

extern int yyparse();
extern FILE* yyin;
extern Formula* parsed_formula;

SymbolTable symbol_table;

void print_clauses(ostream& out, ClauseSet& clauses)
{
	for (auto itr = clauses.begin(); itr != clauses.end(); itr++) {
		bool first = true;
		for (auto sitr = itr->begin(); sitr != itr->end(); sitr++) {
			if (first) {
				first = false;
			}
			else {
				out << " ";
			}
			out << *sitr;
		}
		out << endl;
	}
}

int main(int argc, char* argv[])
{
	assert(argc == 2);

	yyin = fopen(argv[1], "r");
	yyparse();

	Formula* formula = parsed_formula;
	cout << *formula << "\n" << endl;

	ClauseSet clauses;
	formula->transform(clauses);

	cout << "#Variables: " << symbol_table.num_vars() << endl;
	cout << "#Clauses: " << clauses.size() << endl;
	print_clauses(cout, clauses);
	cout << endl;

	symbol_table.print(cout);

	return 0;
}
