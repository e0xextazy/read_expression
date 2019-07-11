#include <iostream>
#include <cmath>
#include <cstring>
#include <string>

using namespace std;

class Expression
{ // Abstract class
public:
	virtual Expression* diff(char x) = 0;
	virtual void print() = 0;
	virtual bool isZero() = 0; // Zero Check
	virtual bool isConst() = 0; // Constant check(Number)
	virtual float constValue() = 0; // Constant value(Number)
};

Expression* Simplify(Expression *ex);

Expression* ExpressionString(char *string);

class Number : public Expression
{ // Number
private:
	float num;
public:
	//float num;
	Number(): num(0) {}
	Number(float val): num(val) { }
	Expression* diff(char x)
	{
		Expression *d_num = new Number(0);
		return d_num;
	}
	void print() { cout << num ; }
	bool isZero() { return num == 0; }
	bool isConst() { return true; }
	float constValue() { return num; }
};

class Variable : public Expression
{ // Variable
private:
	//char var;
public:
	char var;
	Variable(): var('x') { }
	Variable(char ch): var(ch) { }
	Expression* diff(char x) 
	{
		if (var == x)
		{
			Expression *d_var = new Number(1);
			return d_var;
		}
		else return new Number(0);
	}
	void print() { cout << var ; }
	bool isZero() { return false; }
	bool isConst() { return false; }
	float constValue() { return 0; }
};

class Add : public Expression
{ // The sum of two expressions
protected:
	//Expression *left, *right;
public:
	Expression *left, *right;
	Add(Expression *arg1, Expression *arg2): left(arg1), right(arg2) { }
	Expression* diff(char x)
	{
		Expression *ld = Simplify(left->diff(x));
		Expression *rd = Simplify(right->diff(x));
		Expression *result = Simplify(new Add(Simplify(ld), Simplify(rd)));
		return result;
	}
	void print()
	{
		cout << "(";
		left->print();
		cout << "+";
		right->print();
		cout << ")";
	}
	bool isZero() { return left->isZero() && right->isZero(); }
	bool isConst() { return left->isConst() && right->isConst(); }
	float constValue() { return left->constValue() + right->constValue(); }
};

class Sub : public Expression
{ // The sub of two expressions
public:
	Expression *left, *right;
	Sub(Expression *arg1, Expression *arg2): left(arg1), right(arg2) { }
	Expression* diff(char x)
	{
		Expression *ld = Simplify(left->diff(x));
		Expression *rd = Simplify(right->diff(x));
		Expression *result = Simplify(new Sub(Simplify(ld), Simplify(rd)));
		return result;
	}
	void print()
	{
		cout << "(";
		left->print();
		cout << "-";
		right->print();
		cout << ")";
	}
	bool isZero() { return left->isZero() && right->isZero(); }
	bool isConst() { return left->isConst() && right->isConst(); }
	float constValue() { return left->constValue() - right->constValue(); }
};

class Mul : public Expression
{ // Composition (ab)' = a'b + ab';
public:
	Expression *left, *right;
	Mul(Expression *arg1, Expression *arg2): left(arg1), right(arg2) { }
	void print()
	{
		cout << "(";
		left->print();
		cout << "*";
		right->print();
		cout << ")";
	}
	Expression* diff(char x) 
	{
		Expression *ld = Simplify(new Mul(left->diff(x), right));
		Expression *rd = Simplify(new Mul(left, right->diff(x)));
		Expression *result = Simplify(new Add(ld, rd));
		return result;
	}
	bool isZero() { return left->isZero() || right->isZero(); }
	bool isConst() { return left->isConst() && right->isConst(); }
	float constValue() { return left->constValue() * right->constValue(); }
};

class Div : public Expression
{ // quotient (a/b)' = (a'b - ab')/b^2
public:
	Expression *left, *right;
	Div(Expression *arg1, Expression *arg2): left(arg1), right(arg2) { }
	void print()
	{
		cout << "(";
		left->print();
		cout << "/"; 
		right->print();
		cout << ")";
	}
	Expression *diff(char x)
	{
		Expression *dl = Simplify(left->diff(x));
		Expression *dr = Simplify(right->diff(x));
		Expression *l = Simplify(new Mul(dl, right));
		Expression *r = Simplify(new Mul(left, dr));
		Expression *bottom = Simplify(new Mul(right, right));
		Expression *result = new Div(Simplify(new Sub(l, r)), bottom);
		return result;
	}
	bool isZero() { return left->isZero(); }
	bool isConst() { return left->isConst() && (right->isConst() && !right->isZero()); }
	float constValue() { return left->constValue() / right->constValue(); }
};

class Pow : public Expression
{ //  (x^y)' = y*x^(y-1)
public:
	Expression *arg;
	Expression *degree;
	Pow(Expression *arg1, Expression *arg2): arg(arg1), degree(arg2) { }
	void print()
	{
		cout << "(";
		arg->print();
		cout << "^"; 
		degree->print();
		cout << ")";
	}
	Expression *diff(char x)
	{ //(x+6)^10 // d(osn)*1degree*2(osn)^degree-1
		Expression *dosn = Simplify(arg->diff('x'));
		Expression *newdeg = Simplify(new Sub(degree, new Number(1)));
		Expression *first = new Mul(Simplify(dosn), Simplify(new Number(degree->constValue())));
		Expression *newpow = new Pow(Simplify(arg), Simplify(new Sub(degree, new Number(1))) );
		Expression *result = Simplify( new Mul(Simplify(first),Simplify(newpow)) );
		return result;
	}
	bool isZero() { return arg->isZero(); }
	bool isConst() { return arg->isConst(); }
	float constValue() { return pow(arg->constValue(), degree->constValue()); }
};

class Exp : public Expression
{ // exponent (e^(2*x))' = (2*(e^(2*x)))
public:
	Expression *arg;
	Exp(Expression *arg_): arg(arg_) { }
	void print()
	{
		cout << "(exp";
		cout << "(";
		arg->print();
		cout << "))";
	}
	Expression *diff(char x)
	{
		Expression *Imarg = Simplify(arg);
		return Simplify(new Mul(new Exp(Imarg), Imarg->diff(x)));
	}
	bool isZero() { return false; }
	bool isConst() { return arg->isConst(); }
	float constValue() { return exp(arg->constValue()); }
};

int SearchFirstSymbol(char *str, char ch)
{ // Find the index of the first character to turn on
	int len = strlen(str);
	for (int i = 0; i < len; i++)
	{
		if (str[i] == ch)
			return i;
	}
	return -1;
}

int SearchLastSymbol(char *str, char ch)
{ // Search index of inclusion of the first from the end of the character
	int len = strlen(str);
	for (int i = len - 1; i >= 0; i--)
	{
		if (str[i] == ch)
			return i;
	}
	return -1;
}

char* SubString(char *str, int from, int to)
{ // From "from" to "to" inclusive to 
	char *result = new char[to - from + 1];
	for (int i = 0; i < to - from; i++)
	{
		result[i] = str[from + i];
	}
	result[to - from] = '\0';
	return result;
}

char* DeleteSpace(char *str)
{ // Remove spaces at line edges
	char space = ' ';
	int left = -1;
	int lenght = (int)strlen(str);
	int right = lenght;
	// Left
	for (int i = 0; i < lenght; i++)
	{
		if (str[i] != space)
		{
			break;
		}
		else
			left = i;
	}
	for (int i = lenght-1; i >= 0; i--)
	{
		if (str[i] == space)
		{
			right = i;
		}
		else
			break;
	}
	return SubString(str, left+1, right);
}

Expression* Simplify(Expression *ex)
{ // A function that simplifies the appearance of an expression.
	// NUMBER
	if (ex->isConst())
	{ // For Number
		return new Number(ex->constValue()); 
	}
	// MUL
	if (typeid(*ex) == typeid(Mul))
	{
		Mul *exM = (Mul*)ex;
		Expression *exMl = Simplify(exM->left); 
		Expression *exMr = Simplify(exM->right);
		if (typeid(*exMl) == typeid(Variable) && typeid(*exMr) == typeid(Variable))
		{ // x*x
			Variable *v = (Variable*)exMl;
			return new Pow(new Variable(v->var), new Number(2)); // x^2
		}
		if (exMr->isConst() && exMr->constValue() == 1)
		{ // x*1
			return Simplify(exMl);
		}
		if (exMl->isConst() && exMl->constValue() == 1)
		{ // 1*x
			return Simplify(exMr);
		}
		if (exMr->isZero())
		{ // x*0
			return new Number(0);
		}
		if (exMl->isZero())
		{ // 0*x
			return new Number(0);
		}
		if (exMl->isConst())
		{ // const*x
			if (typeid(*exMr) == typeid(Mul))
			{ // const*(a*b)
				Expression *a = ((Mul*)exMr)->left;
				Expression *b = ((Mul*)exMr)->right;
				if (a->isConst())
				{ // const*(const*b)
					return new Mul(Simplify(new Mul(exMl, a)), b);
				}
				if (b->isConst())
				{ // const*(a*const)
					return new Mul(a, Simplify(new Mul(exMl, b)));
				}
			}
		}
		if (exMr->isConst())
		{ // Similar to the top
			if (typeid(*exMl) == typeid(Mul))
			{
				Expression *a = ((Mul*)exMl)->left;
				Expression *b = ((Mul*)exMl)->right;
				if (a->isConst())
				{
					return new Mul(Simplify(new Mul(exMr, a)), b);
				}
				if (b->isConst())
				{
					return new Mul(a, Simplify(new Mul(exMr, b)));
				}
			}
		}
		if (typeid(*exMl) == typeid(Exp) && typeid(*exMr) == typeid(Exp))
		{ // e^a * e^b = e^(a+b)
			return new Exp(new Add(((Exp*)exMl)->arg, ((Exp*)exMr)->arg));
		}
		// x*(1/y) = (x/y) = (1/y)*x
		/*if (typeid(exMr) == typeid(Div))
		{ // x*(a1/b1)
			Expression *a1 = ((Div*)exMr)->left;
			Expression *b1 = ((Div*)exMr)->right;
			if (a1->isConst && a1->constValue == 1)
			{
				return new Div( Simplify(exMl), Simplify(b1) );
			}
		}*/
	}
	// ADD
	if (typeid(*ex) == typeid(Add))
	{ // 
		Add *exA = (Add*)ex;
		Expression *a1 = Simplify(exA->left); // a1 + b1
		Expression *b1 = Simplify(exA->right);
		if (typeid(*a1) == typeid(Variable) && typeid(*b1) == typeid(Variable))
		{ // x+x
			Variable *v = (Variable*)a1;
			return new Mul(new Number(2), new Variable(v->var));
		}
		if (a1->isZero())
		{ // 0+b1
			return Simplify(b1);
		}
		if (b1->isZero())
		{ // a1+0
			return Simplify(a1);
		}
	}
	// DIV
	if (typeid(*ex) == typeid(Div))
	{
		Div *exD = (Div*)ex;
		Expression *exDl = Simplify(exD->left);
		Expression *exDr = Simplify(exD->right);
		if (exDr->isConst() && exDr->constValue() == 1)
		{ // x/1
			return Simplify(exDl);
		}
		if (exDl->isZero())
		{ // 0/x
			return new Number(0);
		}
		if (typeid(*exDl) == typeid(Exp) && typeid(*exDr) == typeid(Exp))
		{// e^a / e^b = e^(a-b)
			return new Exp(new Sub(((Exp*)exDl)->arg, ((Exp*)exDr)->arg));
		}
	}
	// POW
	if (typeid(*ex) == typeid(Pow))
	{
		Pow *exP = (Pow*)ex;
		Expression *exPl = Simplify(exP->arg);
		Expression *exPr = Simplify(exP->degree);
		if (exPr->isZero())
		{ // x^0
			return new Number(1);
		}
		if (exPr->isConst() && exPr->constValue() == 1)
		{ // x^1 
			return Simplify(exPl);
		}
		if (exPr->isConst())
		{ // If the degree is a numeric expression
			return new Pow(Simplify(exPl), new Number(exPr->constValue()));
		}
	}
	// SUB
	if (typeid(*ex) == typeid(Sub))
	{
		Sub *exS = (Sub*)ex;
		Expression *exSl = Simplify(exS->left);
		Expression *exSr = Simplify(exS->right);
		if (exSl->isZero())
		{ // 0-x = -x
			Expression *result = Simplify( new Mul( new Number(-1), Simplify(exSr) ) );
			return result;
		}
		if (exSr->isZero())
		{ // x-0
			return Simplify(exSl);
		}
	}
	return ex;
}

int Max(int x, int y)
{
	if (x > y) return x;
	else return y;
}

bool isFloat(char *str)
{
	str = DeleteSpace(str);
	int lenght = strlen(str);
	int p = 0;
	for (int i = 0; i < lenght; i++)
	{
		if (!((str[i] >= '0' && str[i] <= '9') || (str[i] == '.'))) p = 1;
	}
	if (p == 0) return true;
	else return false;
}

Expression* GetBinaryFunc(char *left, char *func, char *right)
{
	if (func[0] == '+')
	{
		return new Add(ExpressionString(left), ExpressionString(right));
	}
	if (func[0] == '-')
	{
		return new Sub(ExpressionString(left), ExpressionString(right));
	}
	if (func[0] == '*')
	{
		return new Mul(ExpressionString(left), ExpressionString(right));
	}
	if (func[0] == '/')
	{
		return new Div(ExpressionString(left), ExpressionString(right));
	}
	if (func[0] == '^')
	{
		return new Pow(ExpressionString(left), ExpressionString(right));
	}
}

Expression* SimpleExpression(char *str)
{ // if there are no brackets
	// We are looking for a bin operation index + - * / ^
	int index = Max( Max( Max( Max( SearchFirstSymbol(str, '+'),SearchFirstSymbol(str, '-') ), SearchFirstSymbol(str, '*') ), SearchFirstSymbol(str, '/') ), SearchFirstSymbol(str, '^') );
	// If not, then either a number or a variable
	if (index == -1)
	{
		if (isFloat(str))
		{
			return new Number(stof(str));
		}
		else
		{ // .  x .
			return new Variable(DeleteSpace(str)[0]);
		}
	}
	else
	{ // Parse the left, the operation and the right side
		char *left = SubString(str, 0 , index);
		char *right = SubString(str, index+1, strlen(str));
		GetBinaryFunc(left, SubString(str, index, index+1), right);
	}
}

Expression* ExpressionString(char *string)
{
	Expression *result; // Result
	string = DeleteSpace(string); // We remove spaces on the sides
	int lenght = strlen(string);
	// Looking for external brackets
	int l = SearchFirstSymbol(string, '(');
	int r = SearchLastSymbol(string, ')');
	// If there are no brackets
	if (l == -1 && r == -1)
	{
		return Simplify(SimpleExpression(string));
	}
	// If there are brackets, consider the cases.
	// Find paired brackets for external
	int rp = -1, lp = -1;
	int Counter = 0; // counter brackets
	// We are looking for a pair for the left bracket
	for (int i = 0; i < lenght; i++)
	{
		if (string[i] == '(')
		{
			Counter++;
		}
		if (string[i] == ')')
		{
			if (Counter == 1)
			{
				lp = i;
				break;
			}
			Counter--;
		}
	}
	Counter = 0;
	for (int i = lenght-1; i >=0; i--)
	{
		if (string[i] == ')')
		{
			Counter++;
		}
		if (string[i] == '(')
		{
			if (Counter == 1)
			{
				rp = i;
				break;
			}
			Counter--;
		}
	}
	if (l == 0 && r == lenght-1 && lp == r)
	{ // (.o.) one outer brackets
		result = ExpressionString( SubString(string, 1, lenght-1) ); // Remove the external brackets and run the function again.
	}
	else
		if (lp != r)
		{ // ().o.() two pairs of external brackets, between them there must be a binary operation
			char *funcstr = SubString(string, lp+1, rp); // pick up the substring where the operation should be
			// We are looking for the binary operation index in the substring
			int index = Max( Max( Max( Max( SearchFirstSymbol(funcstr, '+'),SearchFirstSymbol(funcstr, '-') ), SearchFirstSymbol(funcstr, '*') ), SearchFirstSymbol(funcstr, '/') ), SearchFirstSymbol(funcstr, '^') );
			index = index + lp + 1; // Get the index of the operation in the whole row
			char *left = DeleteSpace( SubString(string, 0 , index) );
			char *func = DeleteSpace( SubString(string, index, index+1) );
			char *right = DeleteSpace( SubString(string, index+1, lenght) );
			result = GetBinaryFunc(left, func, right);
		}
		else
		{
			if (r != lenght-1)
			{ // ...()o.. or ()o..
				char *funcstr = SubString(string, lp+1, lenght); // pick up the substring where the operation should be
				int index = Max( Max( Max( Max( SearchFirstSymbol(funcstr, '+'),SearchFirstSymbol(funcstr, '-') ), SearchFirstSymbol(funcstr, '*') ), SearchFirstSymbol(funcstr, '/') ), SearchFirstSymbol(funcstr, '^') );
				index = index + lp + 1; // Get the index of the operation in the whole row
				char *left = DeleteSpace( SubString(string, 0, index) );
				char *func = DeleteSpace( SubString(string, index, index+1) );
				char *right = DeleteSpace( SubString(string, index+1, lenght) );
				result = GetBinaryFunc(left, func, right);
			}
			else
			{ // ..o() or exp()
				char *funcstr = SubString(string, 0, l);
				int index = Max( Max( Max( Max( SearchFirstSymbol(funcstr, '+'),SearchFirstSymbol(funcstr, '-') ), SearchFirstSymbol(funcstr, '*') ), SearchFirstSymbol(funcstr, '/') ), SearchFirstSymbol(funcstr, '^') );
				if (index == -1)
				{ // then exp()
					char *arg = DeleteSpace( SubString(string, l+1, r) );
					result = new Exp( ExpressionString(arg) );
				}
				else
				{ // then ..o()
					char *left = DeleteSpace( SubString(string, 0, index) );
					char *func = DeleteSpace( SubString(string, index, index+1) );
					char *right = DeleteSpace( SubString(string, index+1, lenght) );
					result = GetBinaryFunc(left, func, right);
				}
			}
		}
		return Simplify(result);
}

int main()
{
	// Number
	/*Expression *n = new Number(123);
	n->print();
	Expression *dn = n->diff('x');
	cout << "' = ";
	dn->print();
	cout << endl;

	// Variable
	Expression *x = new Variable('x');
	x->print();
	Expression *dx = x->diff('x');
	cout << "' = ";
	dx->print();
	cout << endl;
	
	// Add
	Expression *sum = new Add(new Variable('x'), new Variable('x'));
	sum->print();
	Expression *dsum = sum->diff('x');
	cout << "' = ";
	dsum->print();
	cout << endl;

	// Sub
	Expression *dif = new Sub(new Variable('x'), new Number(2));
	dif->print();
	Expression *ddif = dif->diff('x');
	cout << "' = ";
	ddif->print();
	cout << endl;

	// Mul
	Expression *mul = new Mul(new Variable('x'), new Div( new Number(1),new Exp(new Variable('x')) ) );
	mul->print();
	Expression *dmul = mul->diff('x');
	cout << "' = ";
	dmul->print();
	cout << endl;

	// Div
	Expression *div = new Div(new Variable('y'), new Variable('x'));
	div->print();
	Expression *ddiv = div->diff('x');
	cout << "' = ";
	ddiv->print();
	cout << endl;

	// Pow
	Expression *pow = new Pow(new Variable('x'), new Variable('b'));
	pow->print();
	Expression *dpow = pow->diff('x');
	cout << "' = ";
	dpow->print();
	cout << endl;

	// Exp
	Expression *exp = new Exp(new Mul(new Number(2), new Variable('x')));
	exp->print();
	Expression *dexp = exp->diff('x');
	cout << "' = ";
	dexp->print();
	cout << endl;

	// String
	Expression *str = ExpressionString("(exp((1)/(x)))*(exp(((5*2)/3)))");
	str->print();
	Expression *dstr = str->diff('x');
	cout << endl;
	dstr->print();

	//char *str = "123456789";
	//cout << SubString(str,3,7);*/
	// (x+y)^(1/3)
	Expression *test = ExpressionString("exp((2*x))");
	test->print();
	Expression *dtest = test->diff('x');
	cout << endl;
	dtest->print();
	Expression *ddtest = dtest->diff('x');
	cout << endl;
	ddtest->print();
	cout << endl;

	system("pause");
	return 0;
}
// 0-x = -x
// e^a/e^b = e^(a-b)
// x*(1/y) = (x/y) = (1/y)*x
