#ifndef FORMULA_H_
#define FORMULA_H_

#include <iostream>
#include <string>

using namespace std;

enum OpType
{
	OpVar,
	OpNot,
	OpAnd,
	OpOr,
	OpImp,
	OpIff
};

class Formula
{
protected:
	OpType _type;

	const char* OpName() const;

public:
	Formula(OpType type);
	virtual ~Formula();

	friend ostream& operator<<(ostream& out, const Formula& formula);

	OpType type() const;

	virtual void print(ostream& out) const = 0;
};

class UnaryFormula : public Formula
{
protected:
	Formula* _op;

public:
	UnaryFormula(OpType type, Formula* op);

	virtual void print(ostream& out) const;
};

class BinaryFormula : public Formula
{
protected:
	Formula* _op1;
	Formula* _op2;

public:
	BinaryFormula(OpType type, Formula* op1, Formula* op2);

	virtual void print(ostream& out) const;
};

class Atom : public Formula
{
protected:
	const string _name;

public:
	Atom(const char* name);

	virtual void print(ostream& out) const;
};

inline OpType Formula::type() const
{
	return _type;
}

#endif
