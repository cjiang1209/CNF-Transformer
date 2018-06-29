#ifndef FORMULA_H_
#define FORMULA_H_

#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include <cassert>

#include "base.h"

using namespace std;

enum OpType
{
	OpNone,
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
	// Number of references
	int _num_refs;

	static Literal negate(Literal Literal);

	const char* OpName() const;

public:
	Formula(OpType type);
	virtual ~Formula();

	virtual bool operator==(const Formula& formula) const = 0;
	friend ostream& operator<<(ostream& out, const Formula& formula);

	OpType type() const;
	void add_ref();
	void release_ref();
	int numRefs() const;
	virtual size_t hash() const = 0;

	virtual void flatten(OpType parent_type, vector<Formula*>& ops) = 0;
	// Transformed into conjunction normal form (a set of clauses)
	virtual Literal transform(ClauseSet& clauses) = 0;

	virtual void print(ostream& out) const = 0;
};

class UnaryFormula : public Formula
{
protected:
	Formula* _op;

	Formula* _flat;

public:
	UnaryFormula(OpType type, Formula* op);
	~UnaryFormula();

	virtual bool operator==(const Formula& formula) const override;

	virtual size_t hash() const override;

	virtual void flatten(OpType parent_type, vector<Formula*>& ops) override;
	virtual Literal transform(ClauseSet& clauses) override;

	virtual void print(ostream& out) const override;
};

class BinaryFormula : public Formula
{
protected:
	Formula* _op1;
	Formula* _op2;

	Formula* _flat;
	Literal _cache;

public:
	BinaryFormula(OpType type, Formula* op1, Formula* op2);
	~BinaryFormula();

	virtual bool operator==(const Formula& formula) const override;

	virtual size_t hash() const override;

	virtual void flatten(OpType parent_type, vector<Formula*>& ops) override;
	virtual Literal transform(ClauseSet& clauses) override;

	virtual void print(ostream& out) const override;
};

class Atom : public Formula
{
protected:
	const string _name;

public:
	Atom(const char* name);

	virtual bool operator==(const Formula& formula) const override;

	virtual size_t hash() const override;

	virtual void flatten(OpType parent_type, vector<Formula*>& ops) override;
	virtual Literal transform(ClauseSet& clauses) override;

	virtual void print(ostream& out) const override;
};

class NaryFormula : public Formula
{
protected:
	vector<Formula*> _ops;

	Literal _cache;

public:
	NaryFormula(OpType type, const vector<Formula*>& ops);
	~NaryFormula();

	virtual bool operator==(const Formula& formula) const override;

	virtual size_t hash() const override;

	virtual void flatten(OpType parent_type, vector<Formula*>& ops);
	virtual Literal transform(ClauseSet& clauses) override;

	virtual void print(ostream& out) const override;
};

struct FormulaHasher
{
	size_t operator()(Formula* formula) const
	{
		return formula->hash();
	}
};

struct FormulaEqual
{
	bool operator()(Formula* lhs, Formula* rhs ) const
	{
		return *lhs == *rhs;
	}
};

class FormulaFactory
{
private:
	static unordered_set<Formula*, FormulaHasher, FormulaEqual> _subformulas;

	static Formula* createBinaryFormula(OpType type, Formula* op1, Formula* op2);

public:
	static Formula* createAtom(const char* name);
	static Formula* createNot(Formula* op);
	static Formula* createAnd(Formula* op1, Formula* op2);
	static Formula* createOr(Formula* op1, Formula* op2);
	static Formula* createImp(Formula* op1, Formula* op2);
	static Formula* createIff(Formula* op1, Formula* op2);
	static Formula* createNaryFormula(OpType type, const vector<Formula*>& ops);

	static void clear();

	static int numSubformulas();
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

inline void Formula::add_ref()
{
	_num_refs++;
}

inline void Formula::release_ref()
{
	_num_refs--;
	if (_num_refs == 0) {
		delete this;
	}
}

inline int Formula::numRefs() const
{
	return _num_refs;
}

inline void FormulaFactory::clear()
{
	_subformulas.clear();
}

inline int FormulaFactory::numSubformulas()
{
	return _subformulas.size();
}

#endif
