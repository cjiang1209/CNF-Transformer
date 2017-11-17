#ifndef SYMBOL_TABLE_H_
#define SYMBOL_TABLE_H_

#include <string>
#include <unordered_map>

using namespace std;

class SymbolTable
{
private:
	unordered_map<string, int> _table;
	// Variable indices start from 1
	int _num_vars;

public:
	SymbolTable();

	int add_symbol(string symbol);
	int new_var();

	int num_vars() const;

	void print(ostream& out) const;
};

inline int SymbolTable::num_vars() const
{
	return _num_vars;
}

#endif
