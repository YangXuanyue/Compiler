#if 0

#include "Lexer.h"
#include <fstream>

const string IN_FILE("input.txt");
const string OUT_FILE("output.txt");

using namespace std;

	template <typename T, T DefaultVal>
	class Trie {
		private:
		enum {
			SIGMA_SIZE = 128 //��ĸ���С
		};
		struct Node { //���
			int next[SIGMA_SIZE]; //��ȡһ���ַ���ת�Ƶ�����һ��״̬
			T val; //�ýڵ��ϴ洢����Ϣ
		};
		Node* nodes; //����
		int size; //�����Ŀ
		int root; //���ڵ���

		public:
		//��һ����val��Ϣ���ַ�������Trie����
		void insert(const string& word, const T& val);
		//��Trie���в�ѯĳһ���ַ���������Ϣ
		T search(const string& word);
	};

	struct Token {
		TokenType type; //����
		int symbolPos; //��ʾ�ķ����ڷ��ű���λ��
		int row, col; //�ڴ����е�����λ��
	};

	enum TokenType {
		UNKNOWN,

		CHAR,
		//...
		IF,
		//...
		ADD,
		//...
		PREPROCESSOR,
		IDENTIFIER,
		//...
	};

	const array<string, TOKEN_TYPE_NUM> tokenTypeStrs = {
		"UNKNOWN",
		"CHAR",
		//...
	};

	const array<string, TOKEN_TYPE_NUM> tokenVals = {
		"unknown",
		"char",
		//...
	};

	class DFA {
		enum {
			STATE_NUM = 50, //״̬��
			SIGMA_SIZE = 128, //��ĸ���С
		};
		struct State {
			int next[SIGMA_SIZE]; //��ȡһ����ĸ��ת�Ƶ�����һ��״̬
			TokenType output[SIGMA_SIZE]; //��ȡһ����ĸ�������ʶ���token����
			bitset<SIGMA_SIZE> retractFlg; //��Ƕ�ȡһ����ĸ���Ƿ������
		} states[STATE_NUM];
		int cur; //��ǰ״̬���
		TokenType output; //��ǰ���
		bool retractFlg; //��ǰ���˱��

		public:
		enum {
			START = 0 //��ʼ״̬���
		};
		//��ʼ��
		void init(); 
		//��ȡ��ĸc����״̬ת��
		void trans(char c);
		//��ȡ��ǰ״̬���
		int getState();
		//��ȡ��ǰ���
		TokenType getOutput();
		//�жϵ�ǰ�Ƿ������
		bool needsRetract();
	};

	class Lexer {
		friend istream& operator >> (istream& in, Lexer& rhs); //������������
		friend ostream& operator <<(ostream& out, const Lexer& rhs); //�����������

		private:
		static DFA* dfa; //DFA
		static ReservedWords* reservedWords; //�����ֱ�
		vector<Token> tokens; //ʶ�����token��
		vector<string> symbols; //���ű�
		Trie<int, -1>* posInSymbols; //��ѯ��ǰ�����ڷ��ű���λ�õ�Trie
		int rowCnt; //�м�����
		int charCnt; //�ַ�������
		array<int, TOKEN_TYPE_NUM> tokenTypeCnts; //����token�ļ�����
		vector<int> errorTokenIds; //����token���
	};

	#include "Lexer.h"

	istream& operator >> (istream& in, Lexer& rhs) {
		rhs.tokens.clear();
		rhs.symbols.clear();
		rhs.posInSymbols = new Trie<int, -1>(Lexer::MAX_TRIE_SIZE);
		rhs.dfa->init();
		rhs.charCnt = 0;
		fill(rhs.tokenTypeCnts.begin(), rhs.tokenTypeCnts.end(), 0);
		rhs.errorTokenIds.clear();
		Token curToken;
		string curSymbol;
		int curRow(1), curCol(1);
		bool reachesEOF = false;
		bool expectsNewToken = true;
		for (char c; !reachesEOF; ) {
			if ((c = in.get()) == EOF) {
				//for recognition of token just before EOF
				//since '\0' does not exist in the C-code text, 
				//use it to substitute EOF = -1
				c = '\0';
				reachesEOF = true;
			}
			if (expectsNewToken && !isBlankChar(c)) {
				curToken.row = curRow;
				curToken.col = curCol;
				expectsNewToken = false;
			}
			Lexer::dfa->trans(c);
			if (Lexer::dfa->needsRetract()) {
				in.putback(c);
			} else {
				if (Lexer::dfa->getState() != DFA::START) {
					curSymbol += c;
				} else if (!isBlankChar(c)) {
					curSymbol += c;
				}
				++rhs.charCnt;
				++curCol;
				if (c == '\n') {
					++curRow;
					curCol = 1;
				}
			}
			if (Lexer::dfa->getOutput() != UNKNOWN) {
				curToken.type = Lexer::dfa->getOutput();
				if (curToken.type == IDENTIFIER) {
					curToken.type = Lexer::reservedWords->search(curSymbol);
				}
				switch (curToken.type) {
					case INCOMPLETE_NUMERIC_CONSTANT_ERROR:
					case UNCLOSED_BLOCK_COMMENT_ERROR:
					case UNCLOSED_CHAR_CONSTANT_ERROR:
					case UNCLOSED_STRING_LITERAL_ERROR:
					case ILLEGAL_CHAR_ERROR:
						rhs.errorTokenIds.push_back(rhs.tokens.size());
					case PREPROCESSOR:
					case LINE_COMMENT:
					case BLOCK_COMMENT:
					case IDENTIFIER:
					case NUMERIC_CONSTANT:
					case CHAR_CONSTANT:
					case STRING_LITERAL:
					{
						int pos(rhs.posInSymbols->search(curSymbol));
						if (!~pos) {
							pos = rhs.symbols.size();
							rhs.posInSymbols->insert(curSymbol, pos);
							rhs.symbols.push_back(curSymbol);
						}
						curToken.symbolPos = pos;
						break;
					}
					default:
						curToken.symbolPos = curToken.type;
				}
				rhs.tokens.push_back(curToken);
				++rhs.tokenTypeCnts[curToken.type];
				curSymbol.clear();
				expectsNewToken = true;
			}
			rhs.rowCnt = curRow;
		}
		return in;

void f(){


	Lexer lexer; //�ʷ�������
	ifstream in(IN_FILE); //�����ļ�ΪC���Դ���
	ofstream out(OUT_FILE); //����ļ�Ϊ�������õ���token��
	in >> lexer;
	out << lexer;

}

#endif