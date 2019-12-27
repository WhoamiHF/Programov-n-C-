#include <iostream>
#include <string>
#include <vector>

using namespace std;
class AbstractVal;

using valptr = std::unique_ptr<AbstractVal>;

class AbstractVal {
public:
	virtual ~AbstractVal() {}
	virtual bool isOpeningBracket() = 0;
	virtual bool isMultiplicationOrDivision() = 0;
	virtual void print() = 0;
	virtual valptr clone() = 0;
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
			s.pole.push_back(pole.back()->clone());
			pole.pop_back();
		}
	}
	Seznam() {}
	Seznam(const Seznam& s) { clone(s); }
	Seznam& operator+=(const Seznam& s) {
		/*if (this == &s) { return *this; }

		pole.clear();*/
		clone(s);
		return *this;
	}

private:
	void clone(const Seznam& s) {
		for (auto&& x : s.pole) {
			pole.push_back(x->clone());
		}
	}
	vector<valptr> pole;
};

class CharVal : public AbstractVal {
public:
	bool isOpeningBracket() override
	{
		if (x_ == '(')
		{
			return true;
		}
		else
		{
			return false;
		}

	}
	bool isMultiplicationOrDivision() override
	{
		if (x_ == '*' || x_ == '/')
		{
			return true;

		}
		else
		{
			return false;
		}
	}
	CharVal(char x) : x_(x) {};
	void print() override { std::cout << x_; }
	std::unique_ptr<AbstractVal> clone() override {
		return std::make_unique<CharVal>(*this);
	}
private:
	char x_;
};

class DoubleVal : public AbstractVal {
public:
	DoubleVal(double x) : x_(x) {};
	bool isMultiplicationOrDivision() override { return false; }
	bool isOpeningBracket() override { return false; }
	void print() override { std::cout << x_; }
	std::unique_ptr<AbstractVal> clone() override {
		return std::make_unique<DoubleVal>(*this);
	}
private:
	double x_;
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
		if (isalpha(c)) {
			if (c == 'x' && !expectedOperator)
			{
				output.add(make_unique<CharVal>(c));
				expectedOperator = true;
			}
			else
			{
				return false;
			}
		}
		if (c == '.' && readingNumber && !hasDecimalPoint)
		{
			alreadyRead += c;
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
			}
			if (c == '(')
			{
				if (expectedOperator)
				{
					return false;
				}
				stack.add(make_unique<CharVal>(c));
				alreadyMultiplicationOrDivision = false;
				openedBrackets += 1;
			}
			else if (c == ')')
			{

				openedBrackets -= 1;
				if (openedBrackets < 0 || !expectedOperator) {
					return false;
				}
				else
				{
					stack.popTillStartOrBracket(output);
					stack.pop();
					alreadyMultiplicationOrDivision = stack.isDivisionOrMultiplication();
				}

			}
			else if (c == '*' || c == '/' || c == '-' || c == '+')
			{
				if (!expectedOperator)
				{
					return false;
				}
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
