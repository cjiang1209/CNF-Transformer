#include <iostream>
#include <fstream>
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
	out << "p cnf " << symbol_table.num_vars() << " " << clauses.size() << "\n";
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
		out << " 0\n";
	}
}

int main(int argc, char* argv[])
{
	assert(argc >= 2);

	yyin = fopen(argv[1], "r");
	yyparse();

	Formula* formula = parsed_formula;
//	cout << *formula << "\n" << endl;

	ClauseSet clauses;
	Literal lit = formula->transform(clauses);
	clauses.push_back({lit});

	cout << "#Subformulas: " << FormulaFactory::numSubformulas() << endl;
	cout << "#Variables: " << symbol_table.num_vars() << endl;
	cout << "#Clauses: " << clauses.size() << endl;
	print_clauses(cout, clauses);
	cout << endl;

	if (argc >= 3) {
		ofstream fout(argv[2]);
		print_clauses(fout, clauses);
	}
	if (argc >= 4) {
		ofstream fout(argv[3]);
		symbol_table.print(fout);
	}

	return 0;
}
