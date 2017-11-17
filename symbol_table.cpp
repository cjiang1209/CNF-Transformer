#include <iostream>

#include "symbol_table.h"

SymbolTable::SymbolTable()
	: _num_vars(0)
{
}

int SymbolTable::add_symbol(string symbol)
{
	auto ans = _table.find(symbol);
	if (ans == _table.end()) {
		// New symbol
		_num_vars++;
		_table.emplace(symbol, _num_vars);
		return _num_vars;
	}
	else {
		return ans->second;
	}
}

int SymbolTable::new_var()
{
	return ++_num_vars;
}

void SymbolTable::print(ostream& out) const
{
	for (auto itr = _table.begin(); itr != _table.end(); itr++) {
		out << itr->first << ": " << itr->second << endl;
	}
}
