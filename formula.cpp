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
		cerr << "ERROR: Unsupported operator." << endl;
		exit(-1);
	}
}

ostream& operator<<(ostream& out, const Formula& formula)
{
	formula.print(out);
	return out;
}

UnaryFormula::UnaryFormula(OpType type, Formula* op)
	: Formula(type), _op(op), _flat(nullptr)
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

void UnaryFormula::flatten(OpType parent_type, vector<Formula*>& ops)
{
	if (_flat != nullptr) {
		_flat->add_ref();
		return ops.push_back(_flat);
	}

	vector<Formula*> ops1;
	_op->flatten(_type, ops1);
	assert(ops1.size() == 1);
	_op->release_ref();
	_op = ops1.front();
	_flat = this;
	_flat->add_ref();
	ops.push_back(_flat);
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
	: Formula(type), _op1(op1), _op2(op2), _flat(nullptr), _cache(UNDEFINED)
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

//
// Before:
//      &
//     / \
//    &   c
//   / \
//  a   b
//
// After:
//      &
//     /|\
//    a b c
//
void BinaryFormula::flatten(OpType parent_type, vector<Formula*>& ops)
{
	if (_flat != nullptr) {
		// The formula has been flattened
		_flat->add_ref();
		ops.push_back(_flat);
		return;
	}

	if ((_type == OpType::OpAnd || _type == OpType::OpOr) && _type == parent_type && _num_refs == 1) {
		// The subformula occurs only once
		// Continue to push operands up
		_op1->flatten(_type, ops);
		_op2->flatten(_type, ops);
		return;
	}

	vector<Formula*> ops1;
	_op1->flatten(_type, ops1);
	_op2->flatten(_type, ops1);
	if (ops1.size() == 2) {
		// In-place update
		_op1->release_ref();
		_op1 = ops1[0];
		_op2->release_ref();
		_op2 = ops1[1];
		_flat = this;
		_flat->add_ref();
	}
	else {
		_flat = FormulaFactory::createNaryFormula(_type, ops1);
	}
	ops.push_back(_flat);
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

void Atom::flatten(OpType parent_type, vector<Formula*>& ops)
{
	add_ref();
	ops.push_back(this);
}

Literal Atom::transform(ClauseSet& clauses)
{
	return symbol_table.add_symbol(_name);
}

void Atom::print(ostream& out) const
{
	out << _name;
}

NaryFormula::NaryFormula(OpType type, const vector<Formula*>& ops)
	: Formula(type), _ops(ops), _cache(UNDEFINED)
{
	if (_type != OpType::OpAnd && _type != OpType::OpOr) {
		cerr << "ERROR: Unsupported operator " << OpName() << " in N-ary formulas." << endl;
		exit(-1);
	}
	if (ops.size() <= 2) {
		cerr << "ERROR: N-ary formulas require more than two operands." << endl;
		exit(-1);
	}
}

NaryFormula::~NaryFormula()
{
	for (Formula* op : _ops) {
		op->release_ref();
	}
}

bool NaryFormula::operator==(const Formula& formula) const
{
	const NaryFormula* rhs = dynamic_cast<const NaryFormula*>(&formula);
	if (rhs == nullptr || _type == rhs->_type || _ops.size() != rhs->_ops.size()) {
		return false;
	}
	if (this != rhs) {
		for (int i = 0; i < _ops.size(); i++) {
			if (_ops[i] != rhs->_ops[i]) {
				return false;
			}
		}
	}
	return true;
}

size_t NaryFormula::hash() const
{
	size_t h = std::hash<size_t>{}(_type);
	for (int i = 0; i < _ops.size(); i++) {
		h ^= (std::hash<Formula*>{}(_ops[i]) << ((i + 1) % (sizeof(size_t) * 8)));
	}
	return h;
}

void NaryFormula::flatten(OpType parent_type, vector<Formula*>& ops)
{
	// A n-ary formula is always flat
	add_ref();
	ops.push_back(this);
}

Literal NaryFormula::transform(ClauseSet& clauses)
{
	if (_cache != UNDEFINED) {
		return _cache;
	}

	vector<Literal> lits;
	for (Formula* op : _ops) {
		lits.push_back(op->transform(clauses));
	}

	_cache = symbol_table.new_var();

	if (_type == OpType::OpAnd) {
		for (Literal& lit : lits) {
			clauses.push_back({negate(_cache), lit});
		}
	}
	else if (_type == OpType::OpOr) {
		lits.push_back(negate(_cache));
		clauses.push_back(lits);
	}

	return _cache;
}

void NaryFormula::print(ostream& out) const
{
	out << "(";
	for (int i = 0; i < _ops.size(); i++) {
		if (i > 0) {
			out << " " << OpName() << " ";
		}
		_ops[i]->print(out);
	}
	out << ")";
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

Formula* FormulaFactory::createNaryFormula(OpType type, const vector<Formula*>& ops)
{
	return new NaryFormula(type, ops);
}
