// zapoctak.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

using namespace std;
#include <vector>
#include <list>
#include <iostream>
#include <filesystem>
#include <map>
#include <corecrt_wstring.h>
#include <string>
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include<fstream>
#include <set>
namespace fs = std::filesystem;
class single {
public:
	single(string word_, set<string>& translations_) :word(word_), translations(translations_), score(0) {}
	string word;
	int score;
	set<string> translations;
};

class dictionary {
public:
	dictionary() :sizeOfTest(2),limitUp(2),limitDown(2) {}
	void read(string f);
	void createTest();
	void testWord(int index,vector<single>& data);
	void showMenu();
	void deleteTranslations(string word, set<string>& translations);
	void add(string word, set<string>& translations, vector<single>& whereTo);
	void printEvery(vector<single>& fromWhere);
	vector<single> mainData;
	void printListOfAllFiles();
	string startPosibilities();
	vector<single> importantData;
	int sizeOfTest;
	int limitUp;
	int limitDown;
	map<string, vector<single>::iterator> wordToIndex;
};

void dictionary::showMenu() {
	cout << "type 'start' to start test" << endl;
	cout << "type 'change' to modify database" << endl;
	cout << "type 'end' to end program" << endl;
	//string 
}
void dictionary::add(string word, set<string>& translations, vector<single>& whereTo) {
	if (wordToIndex.find(word) != wordToIndex.end()) {
		auto element = wordToIndex.find(word)->second->translations;
		for (auto item : translations) {
			element.emplace(item);
		}
	}
	else {
		whereTo.push_back(single(word, translations));
		wordToIndex.emplace(word, whereTo.end()--);
	}
}

void dictionary::printEvery(vector<single>& fromWhere) {
	for (auto&& item : fromWhere) {
		cout << item.word << ':';
		cout << item.score << endl;
		for (auto&& tr : item.translations) {
			cout << tr << ',';
		}
		cout << endl;
		cout << endl;
	}
}

void dictionary::createTest() {
	set<int> setOfChoosen;
	while (setOfChoosen.size() < sizeOfTest) {
		int x = rand() % mainData.size();
		if (setOfChoosen.find(x) == setOfChoosen.end()) {
			setOfChoosen.emplace(x);
		}
	}
	for (auto item : setOfChoosen) {
		testWord(item,mainData);
	}
	cout << "Test finished!" << endl;
	printEvery(mainData);
}

void dictionary::deleteTranslations(string word,set<string>& translations ) {
	auto elem = wordToIndex.find(word);
	if (elem != wordToIndex.end()) {
		auto tr = elem->second->translations;
		list<string>::iterator delIt;
		for (auto&& translationToDelete : translations) {
			auto delIt = tr.find(translationToDelete);
			if (delIt != tr.end()) {
				tr.erase(delIt);
			}
		}

		if (tr.empty()) {
			cout << "You have deleted every translation." << endl;
			wordToIndex.erase(elem);
		}
		else {
			cout << " these translations are left:";
			for (auto item : tr) {
				cout << item<<" ";
			}
			cout << endl;
		}
	}
}
void dictionary::testWord(int index,vector<single>& data) {
	cout << "translate: " << data[index].word << endl;
	string answer;
	cin >> answer;
	bool correct = false;
	cout << "Possible answer(s): ";
	for (auto item : data[index].translations) {
		cout << item << " ";
		if (item == answer) {
			correct = true;
		}
	}
	cout << endl;
	if (correct) {
		cout << "Correct answer!" << endl;
		data[index].score++;
		if (data[index].score >= limitUp) {
			//deleteWord
		}
	}
	else {
		cout << "Wrong answer!" << endl;
		data[index].score--;
		if (data[index].score <= limitDown) {
			//move to important
		}
	}
}
void dictionary::read(string f) {
	ifstream myfile;
	myfile.open(f);
	string s;
	string word("");
	set<string> translations;
	while (myfile >> s) {
		char t = s[s.size() - 1];
		if (t == ',') {
			s = s.substr(0, s.size() - 1);
			translations.emplace(s);
			if (f[f.size() - 5] == 'B')
			{
				add(word, translations, mainData);
			}
			else
			{
				add(word, translations, importantData);
			}
			word = "";
			translations.clear();
		}
		else {
			if (word == "") {
				word = s;
			}
			else
			{
				translations.emplace(s);
			}
		}
	}
}

/*void printFile(string filename) {
	ifstream myfile;
	myfile.open(filename);
	string line;
	cout << "posibilites:" << endl;
	while (getline(myfile, line)) {
		cout << line << endl;
	}
}*/

void dictionary::printListOfAllFiles() {
	cout << "Possibilities:" << endl;
	string m;
	std::string path = ".";
	for (const auto& entry : fs::directory_iterator(path)) {
		string fn = entry.path().filename().string();
		if (fn.find(".txt") == fn.size() - 4) {
			fn = fn.substr(0, fn.find(","));
			std::cout << fn << std::endl;
		}
	}
}

string dictionary::startPosibilities() {
	printListOfAllFiles();
	cout << "Please input language from which you want to translate" << endl;
	string first;
	cin >> first;
	cout << "Please input language to which you want to translate" << endl;
	string second;
	cin >> second;
	return first + "-" + second;
}
int main()
{

	locale cz("Czech");
	locale::global(cz);
	setlocale(LC_ALL, "");

	srand(time(NULL));


	dictionary Dict;
	string file = Dict.startPosibilities();
	Dict.read(file + ",B.txt");
	Dict.read(file + ",S.txt");
	Dict.sizeOfTest = 3;
	Dict.createTest();


}

