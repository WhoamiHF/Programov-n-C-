#include <iostream>
#include <string>
#include <vector>
#include <memory>

using namespace std;
class AbstractVal;

using valptr = std::unique_ptr<AbstractVal>;

class AbstractVal {
public:
	virtual ~AbstractVal() {}
	virtual void print() = 0;
	virtual valptr simplify() = 0;
	virtual valptr differentiate() = 0;
	virtual valptr clone() = 0;
	virtual bool isNumber() = 0;
	virtual double returnX() = 0;
	double x_= -100;
};

class AbstractOp :public AbstractVal {
public:
	bool isNumber() { return false; }
	double returnX() { return x_; }
	AbstractOp(valptr&& left, valptr&& right) : left_(move(left)), right_(move(right)) {}
protected:
	valptr left_;
	valptr right_;
};

class DoubleVal : public AbstractVal {
public:
	bool isNumber() { return true; }
	valptr differentiate() override {
		return make_unique<DoubleVal>(0.0);
	}
	DoubleVal(double x) : x_(x) {};
	void print() override { std::cout << x_; }
	valptr simplify() override { return make_unique<DoubleVal>(x_); }
	valptr clone() override {
		return std::make_unique<DoubleVal>(x_);
	}
	double returnX() { return x_; }
	double x_;
};
class VarVal : public AbstractVal {
public:
	bool isNumber() { return false; }
	valptr differentiate() override {
		return make_unique<DoubleVal>(1.0);
	}
	valptr simplify() override { return make_unique<VarVal>(); }
	void print() override { std::cout << "x"; }
	double returnX() { return x_; }
	valptr clone() override {
		return std::make_unique<VarVal>();
	}
};


class Plus :public AbstractOp {
public:
	Plus(valptr&& left, valptr&& right) :AbstractOp(move(left), move(right)) {}
	valptr differentiate() override {
		valptr leftDerivation = left_->differentiate();
		valptr rightDerivation = right_->differentiate();
		return make_unique<Plus>(move(leftDerivation), move(rightDerivation));
	}
	valptr simplify() override {
		valptr leftSimplified = left_->simplify();
		valptr rightSimplified = right_->simplify();
		if (leftSimplified->isNumber() && rightSimplified->isNumber()) {
			double sum = leftSimplified->returnX() + rightSimplified->returnX();
			return make_unique<DoubleVal>(sum);
		}
		if (leftSimplified->isNumber() && leftSimplified->returnX() == 0) {
			return rightSimplified;
		} else if (rightSimplified->isNumber() && rightSimplified->returnX() == 0) {
			return leftSimplified;
		}
		else { return make_unique<Plus>(move(leftSimplified), move(rightSimplified)); }
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
	valptr simplify() override {
		valptr leftSimplified = left_->simplify();
		valptr rightSimplified = right_->simplify();
		if (leftSimplified->isNumber() && rightSimplified->isNumber()) {
			double difference = leftSimplified->returnX() - rightSimplified->returnX();
			return make_unique<DoubleVal>(difference);
		}
		else if (rightSimplified->isNumber() && rightSimplified->returnX() == 0)
		{
			return leftSimplified;
		}
		else
		{
			return make_unique<Minus>(move(leftSimplified), move(rightSimplified));
		}
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
		valptr leftProduct = make_unique<Multiplication>(move(leftDerivation), move(rightCopy));
		valptr rightProduct = make_unique<Multiplication>(move(leftCopy), move(rightDerivation));

		return make_unique<Plus>(move(leftProduct), move(rightProduct));
	}
	valptr clone() override {
		return make_unique<Multiplication>(move(left_->clone()), move(right_->clone()));
	}
	valptr simplify() {
		valptr leftSimplified = left_->simplify();
		valptr rightSimplified = right_->simplify();
		if (leftSimplified->isNumber() && rightSimplified->isNumber()) 
		{
			double product = leftSimplified->returnX() * rightSimplified->returnX();
			return make_unique<DoubleVal>(product);
		}
		if ((leftSimplified->isNumber() && leftSimplified->returnX() == 0) || (rightSimplified->isNumber() && rightSimplified->returnX() == 0)) {
			return make_unique<DoubleVal>(0.0);
		}
		else if (leftSimplified->isNumber() && leftSimplified->returnX() == 1) {
			return rightSimplified;
		}
		else if (rightSimplified->isNumber() && rightSimplified->returnX() == 1) {
			return leftSimplified;
		}
		else { return make_unique<Multiplication>(move(leftSimplified), move(rightSimplified)); }
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
		valptr numerator = make_unique<Minus>(move(leftProduct), move(rightProduct));
		return make_unique<Division>(move(numerator), move(bottomProduct));
	}
	valptr clone() override {
		return make_unique<Division>(move(left_->clone()), move(right_->clone()));
	}
	valptr simplify() override {
		valptr leftSimplified = left_->simplify();
		valptr rightSimplified = right_->simplify();
		if (leftSimplified->isNumber() && leftSimplified->returnX() == 0)
		{
			return make_unique<DoubleVal>(0.0);
		}
		else if (leftSimplified->isNumber() && rightSimplified->isNumber()) 
		{
			double division = leftSimplified->returnX() / rightSimplified->returnX();
			return make_unique<DoubleVal>(division);
		}
		else
		{
			return make_unique<Division>(move(leftSimplified), move(rightSimplified));
		}
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
	int Priority(char op);
private:
	void popTill(int p);
	bool ProcessLine(string line);
	vector<valptr> output;
	vector<char> stack;
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
			auto derivationTree=output.back()->differentiate();
			auto simplifiedTree = derivationTree->simplify();
			simplifiedTree->print();
			cout << endl;
		}

	}
}

int Reader::Priority(char op) {
	switch (op)
	{
	case '+':
	case '-':
		return 0;
		break;
	case'*':
	case'/':
		return 1;
		break;
	default:
		return -1;
		break;
	}
}
void Reader::popTill(int p) {
	while (!stack.empty() && stack.back() != '(' && p <= Priority(stack.back())) {
		valptr elements[2];
		for (size_t i = 0; i < 2; i++)
		{
			if (!output.empty()) {
				elements[i] = move(output.back());
				output.pop_back();
			}
		}
		switch (stack.back())
		{
		case '+':
			output.push_back(make_unique<Plus>(move(elements[1]), move(elements[0])));
			break;
		case'-':
			output.push_back(make_unique<Minus>(move(elements[1]), move(elements[0])));
			break;
		case'*':
			output.push_back(make_unique<Multiplication>(move(elements[1]), move(elements[0])));
			break;
		case'/':
			output.push_back(make_unique<Division>(move(elements[1]), move(elements[0])));
			break;
		default:
			break;
		}
		stack.pop_back();
	}
}
bool Reader::ProcessLine(string line) {
	bool readingNumber = false;
	bool hasDecimalPoint = false;
	bool expectedOperator = false;
	string alreadyRead;

	for (size_t i = 0; i < line.length(); i++)
	{
		char c = line[i];
		if (c == 'x' && !expectedOperator)
		{
			output.push_back(make_unique<VarVal>());
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
				output.push_back(make_unique<DoubleVal>(added));
				alreadyRead = "";
				hasDecimalPoint = false;
			}

			if (c == '(' && !expectedOperator)
			{
				stack.push_back(c);
			}
			else if (c == ')' && expectedOperator)
			{
				popTill(-1); //floor
				if (stack.empty() || stack.back() != '(') return false;
				stack.pop_back();

			}
			else if (!isspace(c))
			{
				if (expectedOperator == false) { return false; }
				expectedOperator = false;
				int p = Priority(c);
				if (p < 0) { return false; }
				if (!stack.empty() && p <= Priority(stack.back())) {
					popTill(p);
				}
					stack.push_back(c);

			}
		}

	}
	if (readingNumber) {
		if (expectedOperator) {
			return false;
		}
		expectedOperator = true;
		double added = stod(alreadyRead);
		output.push_back(make_unique<DoubleVal>(added));
		alreadyRead = "";
	}
	if (!expectedOperator) {
		return false;
	}
	else
	{
			popTill(-1); 
			if (!stack.empty()) {
				return false;
			}
		return true;
	}
}


int main()
{
	Reader readingDevice;
	readingDevice.read();

}
