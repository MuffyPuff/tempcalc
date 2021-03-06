#ifndef MUFEXPRPARSER_H
#define MUFEXPRPARSER_H

#include <QObject>
#include <QList>
#include <QRegularExpression>
#include <QStack>
#include <QQueue>

class MufExprParser : public QObject
{
	Q_OBJECT
public:
	enum class Assoc {
		NONE,
		LEFT,
		RIGHT,
		PREFIX,
		POSTFIX
	};
	Q_ENUM(Assoc)
	enum class TokenType {
		B, // binary op
		E, // expression
		b, // brace
		U, // unary
		v, // value
		x, // variable
		NONE
	};
	Q_ENUM(TokenType)
	struct str_tok_t {
		QString         s;
		Assoc           assoc;
		int             prec;
		int             rightPrec;
		int             nextPrec;
		TokenType       type;
		friend bool operator>(const str_tok_t& lhs, const str_tok_t& rhs);
		friend bool operator<(const str_tok_t& lhs, const str_tok_t& rhs);
		friend bool operator>=(const str_tok_t& lhs, const str_tok_t& rhs);
		friend bool operator<=(const str_tok_t& lhs, const str_tok_t& rhs);
		friend bool operator==(const str_tok_t& lhs, const str_tok_t& rhs);
		friend bool operator!=(const str_tok_t& lhs, const str_tok_t& rhs);
	};
//	struct pat_t {
//		QString         str;
//		TokenType       type;
//		Assoc           assoc;
//		int             prec;
//		int             rightPrec;
//		int             nextPrec;
//		QRegularExpression re;
//	};
	struct pat_t {
		QString         str;
		str_tok_t*      op;
		QRegularExpression re;
	};

	class ExprTree
	{
	public:
		ExprTree()
		{
			this->op = op_sent;
			this->operands.clear();
			this->numeric = true;
		}
//		        : left(nullptr), right(nullptr) {}
		ExprTree(const ExprTree& rhs)
		{
			this->op = rhs.op;
			this->operands.clear();
//			this->operands = rhs.operands;
			for (int i = 0; i < rhs.operands.size(); ++i) {
				this->operands.append(rhs.operands.at(i));
			}
			this->numeric = true;
//			this->left  = nullptr;
//			this->right = nullptr;
//			if (rhs.left != nullptr) {
//				this->left = new ExprTree(*rhs.left);
//			}
//			if (rhs.right != nullptr) {
//				this->right = new ExprTree(*rhs.right);
//			}
		}
		ExprTree(str_tok_t v) // set value as operator
//		        : left(nullptr), right(nullptr)
		{
			this->op = v;
			this->operands.clear();
			this->numeric = true;
		}
		ExprTree(str_tok_t _op, ExprTree _operand)
		{
			this->op = _op;
			this->operands.clear();
			this->operands.push_back(new ExprTree(_operand));
			this->numeric = true;
//			if (_op.assoc == Assoc::PREFIX) {
//				op = _op;
//				left = nullptr;
//				right = new ExprTree(_operand);
//			} else if (_op.assoc == Assoc::POSTFIX) {
//				op = _op;
//				left = new ExprTree(_operand);
//				right = nullptr;
//			} else {
//				qDebug("ExprTree construct error");
//				qDebug() << _op.s << _op.type;
//			}
		}
		ExprTree(str_tok_t _op, ExprTree* _operand)
		{
			this->op = _op;
			this->operands.clear();
			this->operands.push_back(_operand);
			this->numeric = true;
//			if (_op.assoc == Assoc::PREFIX) {
//				op = _op;
//				left = nullptr;
//				right = _operand;
//			} else if (_op.assoc == Assoc::POSTFIX) {
//				op = _op;
//				left = _operand;
//				right = nullptr;
//			} else {
//				qDebug("ExprTree construct error");
//				qDebug() << _op.s << _op.type;
//			}
		}
		ExprTree(str_tok_t _op, ExprTree _left, ExprTree _right)
//		        : left(new ExprTree(_left)), right(new ExprTree(_right))
		{
			this->op = _op;
			this->operands.clear();
			this->operands.push_back(new ExprTree(_left));
			this->operands.push_back(new ExprTree(_right));
			this->numeric = true;
		}
		ExprTree(str_tok_t _op, ExprTree* _left, ExprTree* _right)
//		        : left(_left), right(_right)
		{
			this->op = _op;
			this->operands.clear();
			this->operands.push_back(_left);
			this->operands.push_back(_right);
			this->numeric = true;
		}
		~ExprTree()
		{
			this->op = op_sent;
			for (auto& el : operands) {
				delete el;
				el = nullptr;
			}
//			if (left != nullptr) {
//				delete left;
//			}
//			if (right != nullptr) {
//				delete right;
//			}
		}
		// TODO: add int check
		// check if it's worth to convert
		int             negative();
		// check if it can be reduced to a value
		bool            isValue();
		bool            isFrac();
		bool            hasFrac();
		void            toFrac();
		double          eval();
		bool            isInt();
		bool            isOdd();
		bool            isEven();
		QString         print();
		QString         toLatex();
		void            reduce();
		void            negate();
		void            multiply(const int& factor);
		void            multiply(const QString& var);
		QStringList     var();

		str_tok_t       op;    // can be any operator or value
//		ExprTree*       left;  // can be null
//		ExprTree*       right; // can be null
		QList<ExprTree*> operands;

	private:
		void            prefixUnary();
		void            setValue(const double& v);
		void            setValue(const QString& v);
		void            setChild(const int& i);

	public:
		bool            numeric;
	};

public:
	explicit MufExprParser(QObject* parent = nullptr);
	~MufExprParser()
	{
		if (tree != nullptr) {
			delete tree;
			tree = nullptr;
		}
	}

private:
	int             init();

	bool            expect(const QString& tok_s); // true if next.s == tok_s
	bool            expect(const str_tok_t& tok); // true if next == tok
	bool            expect(const TokenType& tok_t); // true if next.type == tok_t
	void            consume(); // removes next token
	str_tok_t       next();    // returns next token

	bool            tokenize(QString input);
	pat_t*          match(QString s,
	                      QList<pat_t>& p,
	                      str_tok_t* t,
	                      QString* e);

	ExprTree*       exprTD(int p);
	ExprTree*       pTD();
	void            pushOperator(const str_tok_t& op);
	void            popOperator();

	ExprTree*       exprRD(int p);
	ExprTree*       pRD();
	bool            exprSY();
	bool            pSY();
	bool            exprR();
	bool            pR();


signals:

public slots:

	bool            exprRecognise(QString input);
	QString         exprParseSY(QString input);
	QString         exprParseRD(QString input);
	QString         exprParseTD(QString input);
public:
	QString         operator()(QString input);

private:
	static str_tok_t tok_end; // end token
	static str_tok_t op_sent; // sentinel
	static str_tok_t op_rbr;
	static str_tok_t op_lbr;
	static str_tok_t op_exp;
	static str_tok_t op_fac;
	static str_tok_t op_mul;
	static str_tok_t op_div;
	static str_tok_t op_mod;
	static str_tok_t op_add;
	static str_tok_t op_sub;
	static str_tok_t op_equ;
	static str_tok_t op_and;
	static str_tok_t op_or;
	static str_tok_t op_neg;
	static str_tok_t arg_pr;
	static str_tok_t arg_num;
	static str_tok_t arg_var;
	pat_t           pat_eos; // end of string pattern
	QList<pat_t>    pat_ops; // list of operator patterns
	QList<pat_t>    pat_arg; // list of arg? patterns

	QQueue<str_tok_t> tokens; // tokens to parse
	QStack<str_tok_t> mOperators;
	QStack<ExprTree> mOperands;
//	QStack<str_tok_t> stack;

public:
//	QQueue<str_tok_t> queue;
	ExprTree*       tree;

	bool            reduce;

	friend bool operator>(const str_tok_t& lhs, const str_tok_t& rhs);
};

#endif // MUFEXPRPARSER_H
