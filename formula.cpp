#include "formula.h"

Formula::Formula(OpType type)
	: _type(type)
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
		return nullptr;
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

void Atom::print(ostream& out) const
{
	out << _name;
}
