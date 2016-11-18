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
		//�ж��������ս����FOLLOW���Ƿ��������������FOLLOW�����̳������޵ݹ�
		map<Symbol, map<Symbol, bool>> includes_follow_of;

	public:
		Grammar();
		//�������ļ������ķ�
		void load_from_ini();
		//������ݹ�
		void remove_left_recursion();
		//��ȡ������
		void extract_common_left_factor();
		//�Է��ս��nonterminal����FIRST��
		void construct_first(const Symbol& nonterminal);
		//�Է��ս��nonterminal����FOLLOW��
		void construct_follow(const Symbol& nonterminal);
	};

void Grammar::remove_left_recursion() {
	set<Symbol> vis_nonterminals; //�Ѷ�Ӧ����ʽ��������ݹ�ķ��ս����
	vector<Symbol> new_nonterminals; //���ս����
	for (const auto& nonterminal : nonterminals) { //�������սἯ
		set<int> new_production_idxes(production_idxes[nonterminal]);
		for (int i : production_idxes[nonterminal]) { //�������ս��nonterminal�����в���ʽ
			Symbol first_symbol(productions[i].right.front()); //����ʽ��һ������
			//��firsr_symbol��һ����������ݹ�ķ��ս��
			if (vis_nonterminals.find(first_symbol) != vis_nonterminals.end()) { 
				new_production_idxes.erase(i);
				//���ò���ʽ�ĵ�һ�����ŷֱ��滻Ϊ�÷��ŵ����в���ʽ
				productions[i].right.pop_front(); 
				for (int j : production_idxes[first_symbol]) {
					new_production_idxes.insert(productions.size());
					Production new_production(productions[i]);
					new_production.right.insert(
						new_production.right.begin(),
						productions[j].right.begin(),
						productions[j].right.end()
					);
					productions.emplace_back(std::move(new_production));
				}
			}
		}
		production_idxes[nonterminal] = new_production_idxes;
		vector<int> left_recursive_production_idxes; //����ݹ�Ĳ���ʽ��ż�
		for (int i : production_idxes[nonterminal]) { //�������ս��nonterminal�Ĳ���ʽ
			const Symbol& first_symbol(productions[i].right.front());
			//����������ݹ飬����һ��������nonterminal
			if (first_symbol == nonterminal) {
				//���ò���ʽ��nonterminal�Ĳ���ʽ����ɾ��
				new_production_idxes.erase(i);
				//�ټ���left_recursive_production_idxes��
				left_recursive_production_idxes.push_back(i);
			}
		}
		if (left_recursive_production_idxes.size()) { //��������ݹ�
			production_idxes[nonterminal] = new_production_idxes;
			Symbol new_nonterminal(boost::get<string>(nonterminal) + "\'"); //�·��ս��
			new_nonterminals.emplace_back(new_nonterminal);
			//����nonterminal��ֱ����ݹ�
			for (int i : production_idxes[nonterminal]) { 
				productions[i].right.push_back(new_nonterminal);
			}
			production_idxes[nonterminal] = std::move(new_production_idxes);
			production_idxes[new_nonterminal].insert(productions.size());
			productions.emplace_back(new_nonterminal, 
									 std::move(deque<Symbol>{EPSILON}));
			for (int i : left_recursive_production_idxes) {
				productions[i].left = new_nonterminal;
				productions[i].right.pop_front();
				productions[i].right.push_back(new_nonterminal);
				production_idxes[new_nonterminal].insert(i);
			}
		}
		vis_nonterminals.insert(nonterminal);
	}
	for (auto& new_nonterminal : new_nonterminals) {
		nonterminals.emplace_back(std::move(new_nonterminal));
	}
}

#include "Trie.h"
#include <utility>
#include <boost/lexical_cast.hpp>

using std::pair;
using boost::lexical_cast;

enum {
	MAX_TRIE_SIZE = 500
};

void Grammar::extract_common_left_factor() {
	//��ÿ������ӳ�䵽һ������
	map<Symbol, int> symbol_to_idx;
	int cur_idx(0);
	for (const auto& nonterminal : nonterminals) {
		symbol_to_idx[nonterminal] = cur_idx++;
	}
	for (const auto& terminal : terminals) {
		symbol_to_idx[terminal] = cur_idx++;
	}
	Trie<int, -1> trie(MAX_TRIE_SIZE, cur_idx);
	vector<Symbol> new_nonterminals;
	//maps each symbol in each production to its idx
	//for insertion in trie
	vector<vector<int>> int_mapped_productions(productions.size());
	for (const auto& nonterminal : nonterminals) {
		trie.clear();
		//�Է��ս��nonterminal��ÿ������ʽ������ת��Ϊ����ӳ��ֵ���飬���뵽trie��
		for (int i : production_idxes[nonterminal]) {
			for (const auto& symbol : productions[i].right) {
				int_mapped_productions[i].push_back(symbol_to_idx[symbol]);
			}
			trie.insert<vector<int>, int>(int_mapped_productions[i], i);
		}
		set<int> remaining_production_idxes(production_idxes[nonterminal]);
		//clf = common left factor
		//in pair<vector<int>, int>:
		//vector<int> contanis indexes of productions shared clf
		//int indicates the length of their clf
		vector<pair<set<int>, int>> clf_shared_productions_sets;
		for (int i : production_idxes[nonterminal]) {
			if (remaining_production_idxes.find(i)
				!= remaining_production_idxes.end()) {
				int cur(trie.root);
				int clf_len(0);
				//�ڸò���ʽ��Ӧtrie����·�����ҵ�һ��ֻ���侭���Ľ��
				for (const auto& symbol : productions[i].right) {
					int idx(symbol_to_idx[symbol]);
					int nxt(trie.nodes[cur].next[idx]);
					if (trie.nodes[nxt].vis_vals.size() == 1) {
						if (trie.nodes[cur].vis_vals.size() > 1) {
							//trie���ڵ㵽�丸���·�����ȼ�Ϊ�ò���ʽ���������ӳ���
							clf_shared_productions_sets.emplace_back(
								trie.nodes[cur].vis_vals, clf_len
							);
							//�������丸�ڵ�Ĳ���ʽ��trie��ɾ��
							for (int i : clf_shared_productions_sets.back().first) {
								remaining_production_idxes.erase(i);
								trie.erase<vector<int>, int>(int_mapped_productions[i], i);
							}
							break;
						}
					}
					cur = nxt;
					++clf_len;
				}
			}
		}
		if (clf_shared_productions_sets.size()) {
			production_idxes[nonterminal] = std::move(remaining_production_idxes);
			int new_nonterminal_suffix(1);
			for (auto& clf_shared_productions : clf_shared_productions_sets) {
				//��ÿһ���������ӵĲ���ʽ����������һ���·��ս��
				Symbol new_nonterminal(
					boost::get<string>(nonterminal)
					+ "_" + lexical_cast<string>(new_nonterminal_suffix++)
				);
				new_nonterminals.emplace_back(new_nonterminal);
				Production new_production(nonterminal);
				int clf_len(clf_shared_productions.second);
				auto clf_shared_production_idxes(
					std::move(clf_shared_productions.first)
				);
				bool has_init_new_production(false);
				//��ȡ������
				for (int i : clf_shared_production_idxes) {
					productions[i].left = new_nonterminal;
					for (int j(0); j < clf_len; ++j) {
						if (!has_init_new_production) {
							new_production.right.emplace_back(
								std::move(productions[i].right.front())
							);
						}
						productions[i].right.pop_front();
					}
					has_init_new_production = true;
					if (productions[i].right.empty()) {
						productions[i].right.emplace_back(EPSILON);
					}
					production_idxes[new_nonterminal].insert(i);
				}
				production_idxes[nonterminal].insert(productions.size());
				new_production.right.emplace_back(std::move(new_nonterminal));
				productions.emplace_back(std::move(new_production));
			}
		}
	}
	for (auto& new_nonterminal : new_nonterminals) {
		nonterminals.emplace_back(std::move(new_nonterminal));
	}
}

//������ս��nonterminal��FIRST��
void Grammar::construct_first(const Symbol& nonterminal) {
	for (int i : production_idxes[nonterminal]) { //����nonterminal�Ĳ���ʽ
		const auto& production(productions[i]);
		bool all_has_epsilon(true); //����Ƿ����ʽ���з��ŵ�FIRST������epsilon
		for (const auto& symbol : production.right) { //��������ʽ�ķ���
			//���÷������ս�����������ò���ʽ��FIRST���в��˳�
			if (symbol.which() == TERMINAL) {
				first_of_production[i].insert(symbol);
				break;
			}
			//����δ����÷��ŵ�FIRST�����ݹ鹹��÷��ŵ�FIRST��
			if (!has_constructed_first[symbol]) {
				construct_first(symbol);
			}
			//���÷��ŵ�FIRST�����뵽�ò���ʽ��FIRST����
			first_of_production[i].insert(
				first[symbol].begin(),
				first[symbol].end()
			);
			//���÷��ŵ�FIRST���к�epsilon�����������������˳�
			if (first[symbol].find(EPSILON) == first[symbol].end()) {
				all_has_epsilon = false;
				break;
			} else {
				//ȥ��epsilon
				first_of_production[i].erase(EPSILON);
			}
		}
		//������epsilon���뵽����epsilon���뵽�ò���ʽ��FIRST����
		if (all_has_epsilon) {
			first_of_production[i].insert(EPSILON);
		}
		//���ò���ʽ��FIRST�����뵽nonterminal��FIRST����
		first[nonterminal].insert(
			first_of_production[i].begin(),
			first_of_production[i].end()
		);
	}
	//����ѹ���nonterminal��FIRST��
	has_constructed_first[nonterminal] = true;
}

//������ս��nonterminal��FOLLOW��
void Grammar::construct_follow(const Symbol& nonterminal) {
	//�����ķ����в���ʽ
	for (const auto& tmp_nonterminal : nonterminals) { 
		for (int i : production_idxes[tmp_nonterminal]) {
			const auto& production(productions[i]);
			for (int j(0), k; j < production.right.size(); ++j) {
				if (production.right[j] == nonterminal) { //����ʽ�г���nonterminal
					for (k = j + 1; k < production.right.size(); ++k) {
						const auto& symbol(production.right[k]);
						if (symbol.which() == TERMINAL) { //�ս��ֱ�Ӽ��벢�˳�
							follow[nonterminal].insert(symbol);
							break;
						}
						//���μ���������ŵ�FIRST�����Ǹ÷��ŵ�FIRST������epsilon
						follow[nonterminal].insert(
							first[symbol].begin(),
							first[symbol].end()
						);
						if (first[symbol].find(EPSILON) == first[symbol].end()) {
							break;
						} else {
							follow[nonterminal].erase(EPSILON);
						}
					}
					//���޺������Ż�������Ŵ���FIRST������epsilon
					if (k == production.right.size()
						&& production.left != nonterminal) {
						const auto& another_nonterminal(production.left);
						//���nonterminal��FOLLOW������another_nonterminal��FOLLOW��
						includes_follow_of[nonterminal][another_nonterminal] = true;
						//��δ���another_nonterminal��FOLLOW������nonterminal��FOLLOW��
						if (!includes_follow_of[another_nonterminal][nonterminal]) {
							//����δ����another_nonterminal��FOLLOW����
							//�ݹ鹹��another_nonterminal��FOLLOW��
							if (!has_constructed_follow[another_nonterminal]) {
								construct_follow(another_nonterminal);
							}
							//��another_nonterminal��FOLLOW�����뵽nonterminal��FOLLOW��
							follow[nonterminal].insert(
								follow[another_nonterminal].begin(),
								follow[another_nonterminal].end()
							);
						}
					}
				}
			}
		}
	}
	//����ѹ���nonterminal��FOLLOW��
	has_constructed_follow[nonterminal] = true;
}


void Grammar::construct_follow() {
	follow[start_symbol].insert(END);
	for (const auto& nonterminal : nonterminals) {
		if (!has_constructed_follow[nonterminal]) {
			construct_follow(nonterminal);
		}
	}
	//�����������ս����FOLLOW���໥���������䲢�������˴�
	for (const auto& nonterminal : nonterminals) {
		for (const auto& another_nonterminal : nonterminals) {
			if (includes_follow_of[nonterminal][another_nonterminal]
				&& includes_follow_of[another_nonterminal][nonterminal]) {
				follow[nonterminal].insert(
					follow[another_nonterminal].begin(),
					follow[another_nonterminal].end()
				);
				follow[another_nonterminal] = follow[nonterminal];
			}
		}
	}
}

#include "Lexer\Lexer.h"

class Parser {
	//��������������parser��lexer�ʷ������Ľ���Ͻ����﷨����
	friend Parser& operator >> (const Lexer& lexer, Parser& parser);

private:
	Grammar grammar; //�ķ�
	map<Symbol, map<Symbol, int>> parsing_table; //������

	void construct_parsing_table(); //���������

public:
	enum {
		SYNCH = -1 //�������־
	};

	Parser() {
		construct_parsing_table();
	}
};

void Parser::construct_parsing_table() {
	grammar.remove_left_recursion(); //������ݹ�
	grammar.extract_common_left_factor(); //��ȡ������
	grammar.construct_first(); //����FIRST��
	grammar.construct_follow(); //����FOLLOW��
	for (const auto& nonterminal : grammar.nonterminals) { //�������ս����
		for (int i : grammar.production_idxes[nonterminal]) { //���������ʽ��
			bool has_epsilon = false; //����Ƿ�Ϊ�ղ���ʽ
			//�Ըò���ʽ��FIRST���е��ս�������ò���ʽ�����Ӧ������
			for (const auto& terminal : grammar.first_of_production[i]) {
				if (boost::get<TokenType>(terminal) != EPSILON) {
					parsing_table[nonterminal][terminal] = i;
				} else {
					has_epsilon = true;
				}
			}
			//���ǿղ���ʽ
			if (has_epsilon) {
				//�Ը÷��ս��FOLLOW���е��ս�������ò���ʽ�����Ӧ������
				for (const auto& terminal : grammar.follow[nonterminal]) {
					parsing_table[nonterminal][terminal] = i;
				}
			}
		}
		//���ڸ÷��ս��FOLLOW���е��ս��������Ӧ����Ϊ�գ��������ڴ������ͬ����ϢSYNCH
		for (const auto& terminal : grammar.follow[nonterminal]) {
			if (parsing_table[nonterminal].find(terminal)
				== parsing_table[nonterminal].end()) {
				parsing_table[nonterminal][terminal] = SYNCH;
			}
		}
	}
}

#include "Parser.h"
#include <stack>
#include <iostream>
#include <fstream>
#include <utility>

using std::stack;
using std::ofstream;
using std::cout;
using std::pair;

ofstream out("../Compiler/Parser/parser_output.txt");

Parser& operator >> (const Lexer& lexer, Parser& parser) {
	//���ʷ������õ���token��
	const vector<Token>& token_stream(lexer.get_token_stream());
	//����ջ
	stack<Symbol> parsing_stack;
	//�����
	pair<deque<Symbol>, deque<Symbol>> left_sentencial_form{
		{},{parser.grammar.get_start_symbol()}
	};
	//����β�������ķ���ʼ����ѹ�����ջ
	parsing_stack.push(END);
	parsing_stack.push(parser.grammar.get_start_symbol());
	const vector<Production>& productions(parser.grammar.get_productions());
	//ɨ��token��
	for (int i(0); i < token_stream.size(); ) {
		out << "current left sentencial form:\n\t\t\t\t";
		for (const auto& symbol : left_sentencial_form.first) {
			print_symbol(out, symbol);
			out << " ";
		}
		for (const auto& symbol : left_sentencial_form.second) {
			print_symbol(out, symbol);
			out << " ";
		}
		out << endl;
		out << "current token stream:\n\t\t\t\t";
		for (int j(i); j < token_stream.size(); ++j) {
			print_symbol(out, token_stream[j].type);
			out << " ";
		}
		out << endl;
		out << "output:\n\t\t\t\t";
		const auto& token(token_stream[i]);
		//����ǰ����ջ��Ϊ�ս��
		if (parsing_stack.top().which() == TERMINAL) {
			//�����ս���������token����һ��tokenƥ��
			if (boost::get<TokenType>(parsing_stack.top()) == token.type) {
				//ָ���ƽ�
				++i;
			} else {
				//������ִ��󣬸���������Ϣ�������ᵯջ��
				out << "error: ";
				print_symbol(out, token.type);
				out << " expected\n";
			}
			//��ջ
			parsing_stack.pop();
			//���µ�ǰ�����
			if (left_sentencial_form.second.size()) {
				left_sentencial_form.first.push_back(
					left_sentencial_form.second.front()
				);
				left_sentencial_form.second.pop_front();
			}
		} else {
			//ջ��Ϊ���ս�����ڷ������в��Ҷ�Ӧ����
			auto res(parser.parsing_table[parsing_stack.top()].find(token.type));
			if (res != parser.parsing_table[parsing_stack.top()].end()) {
				//������ǿհף���ջ
				parsing_stack.pop();
				left_sentencial_form.second.pop_front();
				if (res->second != Parser::SYNCH) {
					//�������ͬ����ϢSYNCH��
					//��ʹ�ö�Ӧ����ʽѹջ�������µ�ǰ�����
					const Production& production(productions[res->second]);
					out << production;
					if (!(production.right.front().which() == TERMINAL
						  && boost::get<TokenType>(production.right.front())
						     == EPSILON)) {
						for (int j(production.right.size() - 1); ~j; --j) {
							parsing_stack.push(production.right[j]);
						}
						left_sentencial_form.second.insert(
							left_sentencial_form.second.begin(),
							production.right.begin(),
							production.right.end()
						);
					}
				} else {
					//��ΪSYNCH����������ʾ���ѵ�ջ��
					out << "error\n";
				}
			} else {
				//������Ϊ�հף���ָ���ƽ�������������ʾ
				++i;
				out << "error\n";
			}
		}
		out << endl;
	}
	out << endl;
	return parser;
}

#endif // 1
