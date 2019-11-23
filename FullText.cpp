// FullText.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
using namespace std;
#include <fstream>
#include <string>
#include <set>
#include <unordered_map>

class Solver {
	class Mydatabase {
	public:
		Mydatabase(string id, string title, string& words) :id_(id), title_(title),
			words_(words) {};
		string id_;
		string title_;
		string words_;
	};
public:
	set<Mydatabase> v;
	using dataItem_t = pair<string, list<string>>;
	using mainData_t = list<dataItem_t>;
	using reverseIndex_t = unordered_multimap<string, mainData_t::iterator>;
	mainData_t myData;
	reverseIndex_t reverseIndex;
	string word;




	void processIt(char c) {
		if (isalpha(c)) {
			if (c >= 'A' && c <= 'Z') {
				c += 'a' - 'A';
			}

			word += c;
		}
		else if (word == "") {

			if (reverseIndex.find(word) == reverseIndex.end()) {

			}
			word = "";
		}
	}
	void readWords(string& thirdLine) {
		string word = "";
		set<string> wordSet;
		char c;
		for (auto i = thirdLine.begin(); i != thirdLine.end(); i++)
		{
			c = *i;
			processIt(c);
		}
	}
	void readArticles(string& articles, set<Mydatabase>& database) {
		ifstream myfile(articles);
		if (myfile.is_open())
		{
			string line;
			string cID;
			string cTitle;
			string cText;

			getline(myfile, line);
			while (line != "")
			{
				cID = line;
				getline(myfile, line);
				cTitle = line;
				getline(myfile, line);
				cText = line;
				readWords(line);
				//add(cID,CTitle,wordVector);
				//wordSet.clear();
				getline(myfile, line);
			}
			myfile.close();
		}
	}
};
int main(int argc, char** argv)
{
	
	string articles = argv[1];
	string commands;
	if (argc > 1) { commands = argv[2]; }


}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
