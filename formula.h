#ifndef FORMULA_H_
#define FORMULA_H_

#include <iostream>
#include <string>
#include <vector>
#include <cassert>

#include "base.h"

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
	static const int UNDEFINED = 0;

	OpType _type;

	static Literal negate(Literal Literal);
	static void add_definition_clauses(ClauseSet& clauses, Literal lit, Clause clause);

	const char* OpName() const;

public:
	Formula(OpType type);
	virtual ~Formula();

	friend ostream& operator<<(ostream& out, const Formula& formula);

	OpType type() const;

	// Transformed into conjunction normal form (a set of clauses)
	virtual void transform(ClauseSet& clauses) = 0;

	virtual void print(ostream& out) const = 0;
};

class UnaryFormula : public Formula
{
protected:
	Formula* _op;

public:
	UnaryFormula(OpType type, Formula* op);

	virtual void transform(ClauseSet& clauses);

	virtual void print(ostream& out) const;
};

class BinaryFormula : public Formula
{
protected:
	Formula* _op1;
	Formula* _op2;

public:
	BinaryFormula(OpType type, Formula* op1, Formula* op2);

	virtual void transform(ClauseSet& clauses);

	virtual void print(ostream& out) const;
};

class Atom : public Formula
{
protected:
	const string _name;

public:
	Atom(const char* name);

	virtual void transform(ClauseSet& clauses);

	virtual void print(ostream& out) const;
};

inline Literal Formula::negate(Literal lit)
{
	assert(lit != Formula::UNDEFINED);
	return -lit;
}

inline OpType Formula::type() const
{
	return _type;
}

#endif
