#include "formula.h"

#include <functional>

#include "symbol_table.h"

extern SymbolTable symbol_table;

Formula::Formula(OpType type)
	: _type(type), _num_refs(1)
{
}

Formula::~Formula()
{
}

const char* Formula::OpName() const
{
	switch(_type) {
	case OpType::OpVar:
		return "x";
	case OpType::OpNot:
		return "!";
	case OpType::OpAnd:
		return "&";
	case OpType::OpOr:
		return "|";
	case OpType::OpImp:
		return "=>";
	case OpType::OpIff:
		return "<=>";
	default:
		cerr << "Unsupported operator" << endl;
		exit(-1);
	}
}

ostream& operator<<(ostream& out, const Formula& formula)
{
	formula.print(out);
	return out;
}

void Formula::add_definition_clauses(ClauseSet& clauses, Literal lit, Clause clause)
{
	// a <=> (b \/ c)
	// -a \/ b \/ c
	// a \/ -b
	// a \/ -c

	clauses.push_back(clause);
	clauses.back().push_back(negate(lit));

	for (auto itr = clause.begin(); itr != clause.end(); itr++) {
		clauses.push_back({negate(*itr), lit});
	}
}

UnaryFormula::UnaryFormula(OpType type, Formula* op)
	: Formula(type), _op(op)
{
}

bool UnaryFormula::operator==(const Formula& formula) const
{
	const UnaryFormula* rhs = dynamic_cast<const UnaryFormula*>(&formula);
	if (rhs == nullptr) {
		return false;
	}
	return (this == rhs)
			|| (_type == rhs->_type && _op == rhs->_op);
}

size_t UnaryFormula::hash() const
{
	return std::hash<size_t>{}(_type) ^ (std::hash<Formula*>{}(_op) << 1);
}

void UnaryFormula::transform(ClauseSet& clauses)
{
	_op->transform(clauses);

	if (_type != OpType::OpNot) {
		exit(-1);
	}

	assert(!clauses.empty());

	ClauseSet temp;
	if (clauses.size() == 1) {
		// Only one clause
		Clause& clause = clauses.front();
		temp.reserve(clause.size());
		for (auto itr = clause.begin(); itr != clause.end(); itr++) {
			temp.push_back({negate(*itr)});
		}
	}
	else {
		// Tseitin transformation
		// !((a \/ b) /\ c)
		// Add additional variables
		// x <=> (a \/ b)
		// !(x /\ c)
		Clause top;
		for (auto itr = clauses.begin(); itr != clauses.end(); itr++) {
			assert(!itr->empty());

			if (itr->size() == 1) {
				top.push_back(negate(itr->front()));
			}
			else {
				Literal x = symbol_table.new_var();

				top.push_back(negate(x));

				add_definition_clauses(temp, x, *itr);
			}
		}
		temp.push_back(top);
	}

	clauses.swap(temp);
}

void UnaryFormula::print(ostream& out) const
{
	out << "(" << OpName() << " ";
	_op->print(out);
	out << ")";
}

BinaryFormula::BinaryFormula(OpType type, Formula* op1, Formula* op2)
	: Formula(type), _op1(op1), _op2(op2)
{
}

bool BinaryFormula::operator==(const Formula& formula) const
{
	const BinaryFormula* rhs = dynamic_cast<const BinaryFormula*>(&formula);
	if (rhs == nullptr) {
		return false;
	}
	return (this == rhs)
			|| (_type == rhs->_type && _op1 == rhs->_op1 && _op2 == rhs->_op2);
}

size_t BinaryFormula::hash() const
{
	return std::hash<size_t>{}(_type) ^ (std::hash<Formula*>{}(_op1) << 1) ^ (std::hash<Formula*>{}(_op2) << 2);
}

void BinaryFormula::transform(ClauseSet& clauses)
{
	_op1->transform(clauses);
	ClauseSet clauses1;
	clauses1.swap(clauses);
	assert(!clauses1.empty());

	_op2->transform(clauses);
	ClauseSet clauses2;
	clauses2.swap(clauses);
	assert(!clauses2.empty());

	if (_type == OpType::OpAnd) {
		clauses.swap(clauses1);
		clauses.insert(clauses.end(), clauses2.begin(), clauses2.end());
	}
	else if (_type == OpType::OpOr) {
		// Whether one operand has one unit clause only
		bool one_unit = false;
		if (clauses1.size() == 1 && clauses1.front().size() == 1) {
			one_unit = true;
		}
		else if (clauses2.size() == 1 && clauses2.front().size() == 1) {
			clauses1.swap(clauses2);
			one_unit = true;
		}

		if (one_unit) {
			Literal lit = clauses1.front().front();
			for (auto itr = clauses2.begin(); itr != clauses2.end(); itr++) {
				itr->push_back(lit);
			}
			clauses.swap(clauses2);
		}
		else {
			// a \/ b
			// Add a switching variable
			// (x => a) /\ (-x => b)
			// (-x \/ a) /\ (x \/ b)
			Literal x = symbol_table.new_var();
			for (auto itr = clauses1.begin(); itr != clauses1.end(); itr++) {
				clauses.push_back(*itr);
				clauses.back().push_back(negate(x));
			}
			for (auto itr = clauses2.begin(); itr != clauses2.end(); itr++) {
				clauses.push_back(*itr);
				clauses.back().push_back(x);
			}
		}
	}
	else if (_type == OpType::OpImp) {
		if (clauses1.size() == 1 && clauses1.front().size() == 1) {
			// a => (b /\ c)
			// (-a \/ b) /\ (-a \/ c)
			Literal lit = clauses1.front().front();
			clauses.swap(clauses2);
			for (auto itr = clauses.begin(); itr != clauses.end(); itr++) {
				itr->push_back(negate(lit));
			}
		}
		else {
			cerr << "Not supported yet" << endl;
			exit(-1);
		}
	}
	else if (_type == OpType::OpIff) {
		// Whether one operand has one unit clause only
		bool one_unit = false;
		if (clauses1.size() == 1 && clauses1.front().size() == 1) {
			one_unit = true;
		}
		else if (clauses2.size() == 1 && clauses2.front().size() == 1) {
			clauses1.swap(clauses2);
			one_unit = true;
		}

		if (one_unit) {
			if (clauses2.size() == 1) {
				add_definition_clauses(clauses, clauses1.front().front(), clauses2.front());
			}
			else {
				cerr << "Not supported yet" << endl;
				exit(-1);
			}
		}
		else {
			cerr << "Not supported yet" << endl;
			exit(-1);
		}
	}
}

void BinaryFormula::print(ostream& out) const
{
	out << "(";
	_op1->print(out);
	out << " " << OpName() << " ";
	_op2->print(out);
	out << ")";
}

Atom::Atom(const char* name)
	: Formula(OpType::OpVar), _name(name)
{
}

bool Atom::operator==(const Formula& formula) const
{
	const Atom* rhs = dynamic_cast<const Atom*>(&formula);
	if (rhs == nullptr) {
		return false;
	}
	return (this == rhs) || (_name == rhs->_name);
}

size_t Atom::hash() const
{
	return std::hash<size_t>{}(_type) ^ (std::hash<string>{}(_name) << 1);
}

void Atom::transform(ClauseSet& clauses)
{
	clauses.push_back({symbol_table.add_symbol(_name)});
}

void Atom::print(ostream& out) const
{
	out << _name;
}

unordered_set<Formula*, FormulaHasher, FormulaEqual> FormulaFactory::_subformulas;

Formula* FormulaFactory::createAtom(const char* name)
{
	Formula* formula = new Atom(name);
	auto search = _subformulas.find(formula);
	if (search == _subformulas.end()) {
		_subformulas.insert(formula);
	}
	else {
		formula->release_ref();
		formula = *search;
		formula->add_ref();
	}
	return formula;
}

Formula* FormulaFactory::createNot(Formula* op)
{
	Formula* formula = new UnaryFormula(OpType::OpNot, op);
	auto search = _subformulas.find(formula);
	if (search == _subformulas.end()) {
		_subformulas.insert(formula);
	}
	else {
		formula->release_ref();
		formula = *search;
		formula->add_ref();
	}
	return formula;
}

Formula* FormulaFactory::createBinaryFormula(OpType type, Formula* op1, Formula* op2)
{
	Formula* formula = new BinaryFormula(type, op1, op2);
	auto search = _subformulas.find(formula);
	if (search == _subformulas.end()) {
		_subformulas.insert(formula);
	}
	else {
		formula->release_ref();
		formula = *search;
		formula->add_ref();
	}
	return formula;
}

Formula* FormulaFactory::createAnd(Formula* op1, Formula* op2)
{
	createBinaryFormula(OpType::OpAnd, op1, op2);
}

Formula* FormulaFactory::createOr(Formula* op1, Formula* op2)
{
	createBinaryFormula(OpType::OpOr, op1, op2);
}

Formula* FormulaFactory::createImp(Formula* op1, Formula* op2)
{
	createBinaryFormula(OpType::OpImp, op1, op2);
}

Formula* FormulaFactory::createIff(Formula* op1, Formula* op2)
{
	createBinaryFormula(OpType::OpIff, op1, op2);
}
