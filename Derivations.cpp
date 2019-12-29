#include <iostream>
#include <string>
#include <vector>

using namespace std;
class AbstractVal;

using valptr = std::unique_ptr<AbstractVal>;

class AbstractVal {
public:
	virtual ~AbstractVal() {}
	virtual void print() = 0;
	virtual valptr differentiate() = 0;
	virtual valptr clone() = 0;
};

class AbstractOp :public AbstractVal {
public:
	AbstractOp(valptr&& left, valptr&& right) : left_(move(left)), right_(move(right)) {}
protected:
	valptr left_;
	valptr right_;
};

class Seznam {
public:
	void clear() { pole.clear(); }
	void back() { pole.back()->print(); cout << endl; };
	void front() { pole.front()->print(); cout << endl; }

	bool isDivisionOrMultiplication() {
		bool isIt = false;
		for (auto&& x : pole) {
			if (x->isMultiplicationOrDivision()) { isIt = true; }
			if (x->isOpeningBracket()) { isIt = false; }
		}
		return isIt;
	}


	void add(valptr p) { pole.push_back(move(p)); }
	void print() {
		for (auto&& x : pole) {
			x->print();
			cout << " ";
		}
	}
	void pop() { if (!pole.empty()) pole.pop_back(); }
	void popTillStartOrBracket(Seznam& s) {
		while (!pole.empty() && !pole.back()->isOpeningBracket())
		{
			s.pole.push_back(move(pole.back()));
			pole.pop_back();
		}
	}
	Seznam() {}
	//Seznam(const Seznam& s) { clone(s); }

private:
	/*	void clone(const Seznam& s) {
			for (auto&& x : s.pole) {
				pole.push_back(x->clone());
			}
		}*/
	vector<valptr> pole;
};

class VarVal : public AbstractVal {
public:
	valptr differentiate() override {
		return make_unique<DoubleVal>(1.0);
	}
	void print() override { std::cout << "x"; }
	valptr clone() override {
		return std::make_unique<VarVal>();
	}*/
private:
};


class DoubleVal : public AbstractVal {
public:
	valptr differentiate() override {
		return make_unique<DoubleVal>(0.0);
	}
	DoubleVal(double x) : x_(x) {};
	void print() override { std::cout << x_; }
	valptr clone() override {
		return std::make_unique<DoubleVal>(x_);
	}
private:
	double x_;
};
class Plus :public AbstractOp {
public:
	Plus(valptr&& left, valptr&& right) :AbstractOp(move(left), move(right)) {}
	valptr differentiate() override {
		valptr leftDerivation = left_->differentiate();
		valptr rightDerivation = right_->differentiate();
		return make_unique<Plus>(move(leftDerivation), move(rightDerivation));
	}

	valptr clone() override {
		return make_unique<Plus>(move(left_->clone()), move(right_->clone()));
	}

	void print() override {
		cout << "(";
		left_->print();
		cout << " + ";
		right_->print();
		cout << ")";
	}
};
class Minus :public AbstractOp {
public:
	Minus(valptr&& left, valptr&& right) :AbstractOp(move(left), move(right)) {}
	valptr differentiate() override {
		valptr leftDerivation = left_->differentiate();
		valptr rightDerivation = right_->differentiate();
		return make_unique<Minus>(move(leftDerivation), move(rightDerivation));
	}
	valptr clone() override {
		return make_unique<Minus>(move(left_->clone()), move(right_->clone()));
	}

	void print() override {
		cout << "(";
		left_->print();
		cout << " - ";
		right_->print();
		cout << ")";
	}
};
class Multiplication :public AbstractOp {
public:
	Multiplication(valptr&& left, valptr&& right) :AbstractOp(move(left), move(right)) {}
	valptr differentiate() override {
		valptr leftDerivation = left_->differentiate();
		valptr rightDerivation = right_->differentiate();
		valptr leftCopy = left_->clone();
		valptr rightCopy = right_->clone();
		valptr leftProduct = make_unique<Multiplication>(move(leftDerivation),move(rightCopy));
		valptr rightProduct = make_unique<Multiplication>(move(leftCopy), move(rightDerivation));

		return make_unique<Plus>(move(leftProduct), move(rightProduct));
	}
	valptr clone() override {
		return make_unique<Multiplication>(move(left_->clone()), move(right_->clone()));
	}
	void print() override {
		cout << "(";
		left_->print();
		cout << " * ";
		right_->print();
		cout << ")";
	}
};

class Division :public AbstractOp {
public:
	Division(valptr&& left, valptr&& right) :AbstractOp(move(left), move(right)) {}
	valptr differentiate() override {
		valptr leftDerivation = left_->differentiate();
		valptr rightDerivation = right_->differentiate();
		valptr leftCopy = left_->clone();
		valptr rightCopy1 = right_->clone();
		valptr rightCopy2 = right_->clone();
		valptr rightCopy3 = right_->clone();
		valptr leftProduct = make_unique<Multiplication>(move(leftDerivation), move(rightCopy1));
		valptr rightProduct = make_unique<Multiplication>(move(leftCopy), move(rightDerivation));
		valptr bottomProduct = make_unique<Multiplication>(move(rightCopy2), move(rightCopy3));
		valptr numerator = make_unique<Plus>(move(leftProduct), move(rightProduct));
		return make_unique<Division>(move(numerator), move(bottomProduct));
	}
	valptr clone() override {
		return make_unique<Division>(move(left_->clone()), move(right_->clone()));
	}
	void print() override {
		cout << "(";
		left_->print();
		cout << " / ";
		right_->print();
		cout << ")";
	}
};
class Reader {
public:
	void read();
private:
	bool ProcessLine(string line);
	Seznam output;
	Seznam stack;
};

void Reader::read() {
	string line;
	while (getline(cin, line))
	{
		output.clear();
		stack.clear();

		if (!ProcessLine(line))
		{
			cout << "Input error" << endl;
		}
		else
		{
			output.print();
			cout << endl;
		}

	}
}

bool Reader::ProcessLine(string line) {
	bool readingNumber = false;
	bool hasDecimalPoint = false;
	bool expectedOperator = false;
	bool alreadySumOrDifference = false;
	bool alreadyMultiplicationOrDivision = false;
	string alreadyRead;
	int openedBrackets = 0;
	for (size_t i = 0; i < line.length(); i++)
	{
		char c = line[i];
		if (!isspace(c)) {
			if (c == 'x' && !expectedOperator)
			{
				output.add(make_unique<CharVal>(c));
				expectedOperator = true;
			}
			else if (c == '.' && readingNumber && !hasDecimalPoint)
			{
				alreadyRead += c;
				hasDecimalPoint = true;
			}
			else if (isdigit(c))
			{
				readingNumber = true;
				alreadyRead += c;
			}
			else
			{
				if (readingNumber)
				{
					if (expectedOperator)
					{
						return false;
					}
					readingNumber = false;
					expectedOperator = true;
					double added = stod(alreadyRead);
					output.add(make_unique<DoubleVal>(added));
					alreadyRead = "";
					hasDecimalPoint = false;
				}

				if (c == '(' && !expectedOperator)
				{
					stack.add(make_unique<CharVal>(c));
					alreadyMultiplicationOrDivision = false;
					openedBrackets += 1;
				}
				else if (c == ')' && (openedBrackets >= 1) && expectedOperator)
				{

					openedBrackets -= 1;
					stack.popTillStartOrBracket(output);
					stack.pop();
					alreadyMultiplicationOrDivision = stack.isDivisionOrMultiplication();

				}
				else if ((c == '*' || c == '/' || c == '-' || c == '+') && expectedOperator)
				{
					expectedOperator = false;

					if (alreadyMultiplicationOrDivision || ((c == '+' || c == '-') && alreadySumOrDifference)) {
						stack.popTillStartOrBracket(output);
						alreadyMultiplicationOrDivision = false;
						alreadySumOrDifference = false;
					}

					if (c == '*' || c == '/')
					{
						alreadyMultiplicationOrDivision = true;
					}
					else
					{
						alreadySumOrDifference = true;
					}
					stack.add(make_unique<CharVal>(c));
				}
				else {
					return false;
				}
			}

		}
	}
	if (readingNumber) {
		if (expectedOperator) {
			return false;
		}
		expectedOperator = true;
		readingNumber = false;
		double added = stod(alreadyRead);
		output.add(make_unique<DoubleVal>(added));
		alreadyRead = "";
	}
	if (openedBrackets != 0 || !expectedOperator) {
		return false;
	}
	else
	{
		stack.popTillStartOrBracket(output);
		return true;
	}
}


int main()
{
	Reader readingDevice;
	readingDevice.read();

}
