#if 0
#pragma once

#include <boost/variant.hpp>
#include <string>
#include "TokenType.h"
#include <iostream>

using boost::variant;

using std::string;

using std::ostream;

enum SymbolType {
	NONTERMINAL, //string
	TERMINAL //TokenType
};

typedef variant<string, TokenType> Symbol;

void print_symbol(ostream& out, const Symbol& symbol);

#pragma once

#include <iostream>
#include <queue>
#include "Symbol.h"

using std::ostream;
using std::endl;

using std::deque;

struct Production {
	Symbol left;
	deque<Symbol> right;
};

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
	private:
		//���ս�������ս����
		vector<Symbol> nonterminals, terminals;
		//��ʼ����
		Symbol start_symbol;
		//����ʽ��
		vector<Production> productions;
		//ÿ�����ս��Ϊ��˵Ĳ���ʽ��ż�
		map<Symbol, set<int>> production_idxes;
		//ÿ������ʽ�Ҷ��ķ����Ŵ���FIRST��
		vector<set<Symbol>> first_of_production;
		//FIRST����FOLLOW��
		map<Symbol, set<Symbol>> first, follow;
		//��ÿ�����ս���Ƿ��ѹ�����FIRST����FOLLOW���ı�־������ݹ�
		map<Symbol, bool> has_constructed_first, has_constructed_follow;

	public:
		Grammar();
		//������ݹ�
		void remove_left_recursion();
		//��ȡ������
		void extract_common_left_factor();
		//�Է��ս��nonterminal����FIRST��
		void construct_first(const Symbol& nonterminal);
		//�Է��ս��nonterminal����FOLLOW��
		void construct_follow(const Symbol& nonterminal);
	};




#endif // 1
