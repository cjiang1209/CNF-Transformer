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

UnaryFormula::UnaryFormula(OpType type, Formula* op)
	: Formula(type), _op(op)
{
}

UnaryFormula::~UnaryFormula()
{
	_op->release_ref();
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

Literal UnaryFormula::transform(ClauseSet& clauses)
{
	if (_type != OpType::OpNot) {
		exit(-1);
	}

	return negate(_op->transform(clauses));
}

void UnaryFormula::print(ostream& out) const
{
	out << "(" << OpName() << " ";
	_op->print(out);
	out << ")";
}

BinaryFormula::BinaryFormula(OpType type, Formula* op1, Formula* op2)
	: Formula(type), _op1(op1), _op2(op2), _cache(UNDEFINED)
{
}

BinaryFormula::~BinaryFormula()
{
	_op1->release_ref();
	_op2->release_ref();
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

Literal BinaryFormula::transform(ClauseSet& clauses)
{
	if (_cache != UNDEFINED) {
		return _cache;
	}

	Literal lit1 = _op1->transform(clauses);
	Literal lit2 = _op2->transform(clauses);

	_cache = symbol_table.new_var();

	if (_type == OpType::OpAnd) {
		clauses.push_back({negate(_cache), lit1});
		clauses.push_back({negate(_cache), lit2});
	}
	else if (_type == OpType::OpOr) {
		clauses.push_back({negate(_cache), lit1, lit2});
	}
	else if (_type == OpType::OpImp) {
		clauses.push_back({negate(_cache), negate(lit1), lit2});
	}
	else if (_type == OpType::OpIff) {
		clauses.push_back({negate(_cache), negate(lit1), lit2});
		clauses.push_back({negate(_cache), lit1, negate(lit2)});
	}

	return _cache;
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

Literal Atom::transform(ClauseSet& clauses)
{
	return symbol_table.add_symbol(_name);
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
	return createBinaryFormula(OpType::OpAnd, op1, op2);
}

Formula* FormulaFactory::createOr(Formula* op1, Formula* op2)
{
	return createBinaryFormula(OpType::OpOr, op1, op2);
}

Formula* FormulaFactory::createImp(Formula* op1, Formula* op2)
{
	return createBinaryFormula(OpType::OpImp, op1, op2);
}

Formula* FormulaFactory::createIff(Formula* op1, Formula* op2)
{
	return createBinaryFormula(OpType::OpIff, op1, op2);
}
