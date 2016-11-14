#pragma once

#include <vector>
#include "Symbol.h"
#include "Production.h"
#include <map>
#include <set>
#include <iostream>

class Parser;

using std::vector;
using std::map;
using std::set;
using std::cout;
using std::endl;

class Grammar {
	friend class Parser;

private:
	vector<Symbol> nonterminals, terminals;
	Symbol start_symbol;
	vector<Production> productions;
	map<Symbol, set<int>> production_idxes;
	map<Symbol, set<Symbol>> first, follow;
	vector<set<Symbol>> first_of_production;
	map<Symbol, bool> has_constructed_first, has_constructed_follow;

	void print_productions();
	void print_first();
	void print_follow();
	void eliminate_left_recursion();
	void construct_first();
	void construct_first(Symbol nonterminal);
	void construct_follow();
	void construct_follow(Symbol nonterminal);

public:
	Grammar();
	~Grammar();

	const Symbol& get_start_symbol() const {
		//cout << start_symbol << endl;
		return start_symbol;
	}

	const vector<Production>& get_productions() const {
		return productions;
	}
/*
	const vector<Symbol>& get_nonterminals() {
		return nonterminals;
	}

	const vector<Symbol>& get_nonterminals() {
		return nonterminals;
	}

	const map<Symbol, set<Symbol>>& get_first() {
		return first;
	}

	const map<Symbol, set<Symbol>>& get_follow() {
		return follow;
	}
*/
};
