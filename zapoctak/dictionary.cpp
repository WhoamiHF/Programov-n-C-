#include "dictionary.h"
#include "interface.h"
#include <string>
#include <fstream>
#include <iostream>
#include <sstream>
#include <unordered_set>
#include <ctime>
using namespace std;


//called from userInterface::showMenu(), writes changed dictionary into files and ends program
// writeIntoFile is in dictionary.h because it's a teplate which is templated by type of container passed as second argument
void dictionary::endThisProgram()
{
	writeIntoFile(file + ",B.txt", mainData);
	writeIntoFile(file + ",I.txt", importantData);
	writeIntoFile(file + ",A.txt", archive);
	writeSettings();
	exit(1);
}
void dictionary::writeSettings() {
	std::ofstream myfile("settings.txt");
	myfile << limitUp << " " << limitDown << " " << sizeOfTest << " " << timeLimit << endl;
}

void dictionary::synonyms(userInterface& ui) {
	int i = rand() % multiTranslations.size();
	int j =rand() % (multiTranslations[i]->translations.size());
	auto it = next(multiTranslations[i]->translations.begin(), j);
	cout << *it;
}
//called by userInterface::showMenu(), manages adding and deleting words+translations
void dictionary::modify(userInterface& ui)
{
	cout << "[add/delete] [main/important] [word] - [translation1 , translation2 ...]" << endl;
	string row;
	while (getline(cin, row))
	{
		stringstream sRow(row);
		string command;
		sRow >> command;
		if (command == "menu") {
			ui.showMenu(*this);
			break;
		}
		string secondCommand;
		sRow >> secondCommand;
		set<string> translations;
		string word = "";
		string s = "";
		sRow >> word;
		while (sRow >> s && s != "-")
		{
			word += s;
		}
		string translation = "";
		sRow >> translation;
		while (sRow >> s)
		{
			if (s == ",")
			{
				translations.emplace(translation);
				sRow >> translation;
			}
			else {
				translation = translation + " " + s;
			}
		}
		if (translation != "") {
			translations.emplace(translation);
		}
		if (command == "add" && secondCommand == "main")
		{
			add(word, translations, mainData);
		}
		else if (command == "add" && secondCommand == "important")
		{
			add(word, translations, importantData);
		}
		else if (command == "delete" && secondCommand == "main")
		{
			deleteTranslations(word, translations, mainData);
		}
		else if (command == "delete" && secondCommand == "important")
		{
			for (auto&& item : translations) {
				cout << ":"<< item<<":";
			}
			deleteTranslations(word, translations, importantData);
		}
	}
}
//manages adding words and it's translations, adds to corresponding indices (/indexes)
void dictionary::add(string word, set<string>& translations, list<single>& whereTo)
{
	if (wordToIterator.find(word) != wordToIterator.end())
	{
		auto element = wordToIterator.find(word)->second->translations;
		for (auto item : translations)
		{
			element.emplace(item);
		}
	}
	else
	{
		whereTo.push_back(single(word, translations, whereTo.size()));
		auto newItem = whereTo.end();
		newItem--;
		wordToIterator.emplace(word, newItem);
		if (&whereTo == &mainData)
		{
			intToIteratorMain.emplace(mainData.size() - 1, newItem);

		}
		else
		{
			intToIteratorImportant.emplace(importantData.size() - 1, newItem);
		}
		if (translations.size() > 1) {
			multiTranslations.push_back(newItem);
		}
	}
}



//creates test, according to chances there should be more important words than from from (60:40), then tests each of them
//uses integer as key. Afterwards returns to the main menu.
void dictionary::createTest(userInterface& ui)
{
	for (auto&& item : multiTranslations) {
		cout << item->word;
	}
	if (gapImportant) 
	{
		rebuildIndex(intToIteratorImportant);
		gapImportant = false;
	}
	if (gapMain)
	{
		rebuildIndex(intToIteratorMain);
		gapMain = false;
	}

	unordered_set<int> setOfChoosenMain;
	unordered_set<int> setOfChoosenImportant;
	while (setOfChoosenMain.size() + setOfChoosenImportant.size() < sizeOfTest
		&& (mainData.size() > setOfChoosenMain.size() || importantData.size() > setOfChoosenImportant.size()))
	{
		int x = rand() % 10;
		if (x > 5 && mainData.size() > setOfChoosenMain.size())
		{
			int y = rand() % mainData.size();
			if (setOfChoosenMain.find(y) == setOfChoosenMain.end() && intToIteratorMain.find(y) != intToIteratorMain.end())
			{
				setOfChoosenMain.emplace(y);
			}
		}
		else if (importantData.size() != setOfChoosenImportant.size())
		{
			int y = rand() % importantData.size();
			if (setOfChoosenImportant.find(y) == setOfChoosenImportant.end() && intToIteratorImportant.find(y) != intToIteratorImportant.end())
			{
				setOfChoosenImportant.emplace(y);
			}
		}
	}
	for (auto item : setOfChoosenMain)
	{
		testWord(item, mainData);
	}
	for (auto item : setOfChoosenImportant)
	{
		testWord(item, importantData);
	}
	cout << "Test finished!" << endl;
	ui.showMenu(*this);
}

void dictionary::rebuildIndex(map<int, list<single>::iterator>& index) {
	map<int, list<single>::iterator> tmp;
	for (auto&& item : index) {
		tmp.emplace(tmp.size(), item.second);
	}
	index = tmp;
}


//deletes listed translations,is called from modify and also from testWord.
//if the list of translations is empty then every translation is deleted and the word is removed.
void dictionary::deleteTranslations(string word, set<string> translations, list<single>& collection)
{
	auto elem = wordToIterator.find(word);
	if (elem != wordToIterator.end())
	{
		auto tr = elem->second->translations;
		list<string>::iterator delIt;
		for (auto&& translationToDelete : translations)
		{
			auto delIt = tr.find(translationToDelete);
			if (delIt != tr.end()) {
				tr.erase(delIt);
			}
		}

		if (tr.empty() || translations.empty())
		{
			if (&collection == &mainData)
			{
				auto tm = intToIteratorMain.find(elem->second->id);
				if (tm != intToIteratorMain.end())
				{
					auto lastElement = intToIteratorMain.end()--;
					if (tm != lastElement) { gapMain = true; }
					intToIteratorMain.erase(tm);
				}
			}
			else
			{
				auto tm = intToIteratorImportant.find(elem->second->id);
				if (tm != intToIteratorImportant.end())
				{
					auto lastElement = intToIteratorImportant.end()--;
					if (tm != lastElement) { gapImportant =true; }
					intToIteratorImportant.erase(tm);
				}
			}
			collection.erase(elem->second);
			wordToIterator.erase(elem);
		}
	}
	else
	{
		cout << "this word wasn't in the database" << endl;
	}
}
//called from createTest. Checks if inputed value corresponds to at least one of correct translations.
//checks if the word is learned and if so then moves the word to archive.
//also checks if the word is problematic and if so then moves the word to important.
void dictionary::testWord(int index, list<single>& data)
{
	list<single>::iterator elem;
	if (&data == &mainData)
	{
		elem = intToIteratorMain.find(index)->second;
	}
	else
	{
		elem = intToIteratorImportant.find(index)->second;
	}
	cout << "translate: " << elem->word << endl;
	string answer;
	clock_t begin = clock();
	cin >> answer;
	clock_t end = clock();
	bool correct = false;
	float elapsed_secs = float(end - begin) / CLOCKS_PER_SEC;
	cout << "Time elapsed: " << elapsed_secs << endl;
	cout << "Possible answer(s): ";
	for (auto item : elem->translations)
	{
		cout << item << " ";
		if (item == answer) {
			correct = true;
		}
	}
	cout << endl;
	if (correct && elapsed_secs <= timeLimit)
	{
		cout << "Correct answer!" << endl;
		elem->score++;
		if (elem->score >= limitUp)
		{
			if (&data == &importantData) {
				string word = elem->word;
				set<string> trs;
				for (auto&& item : elem->translations)
				{
					trs.emplace(item);
				}
				deleteTranslations(elem->word, set<string>(), data);
				add(word, trs, mainData);
			}
			else
			{
				archive.push_back(archiveSingle(elem->word, elem->translations));
				deleteTranslations(elem->word, set<string>(), data);
			}
		}
	}
	else
	{
		if (!correct)
		{
			cout << "Wrong answer!" << endl;
		}
		else
		{
			cout << "Sorry too slow!" << endl;
		}
		elem->score--;
		if (elem->score <= -limitDown && (&data==&mainData))
		{
			string word = elem->word;
			set<string> trs;
			for (auto&& item : elem->translations)
			{
				trs.emplace(item);
			}
			deleteTranslations(elem->word, set<string>(), mainData);
			add(word, trs, importantData);
			
		}
	}
}

//initialization - reads settings and three databases - main,important and archive.
void dictionary::read()
{
	ifstream settingsFile("settings.txt");
	if (settingsFile.is_open()) {
		settingsFile >> limitUp;
		settingsFile >> limitDown;
		settingsFile >> sizeOfTest;
		settingsFile >> timeLimit;
		settingsFile.close();
	}

	string f;
	for (size_t i = 0; i <= 2; i++)
	{
		if (i == 0)
		{
			f = file + ",B.txt";
		}
		else if (i==1)
		{
			f = file + ",I.txt";
		}
		else {
			f = file + ",A.txt";
		}

		ifstream myfile;
		myfile.open(f);
		string s;
		string word("");
		string translation;
		set<string> translations;
		bool readingWord = true;
		while (myfile >> s)
		{
			if (s == "-" && readingWord)
			{
				readingWord = false;
			}
			else if (s == "," && !readingWord)
			{
				translations.emplace(translation);
				translation = "";
			}
			else if (s == ";" && !readingWord)
			{
				translations.emplace(translation);
				readingWord = true;
				if (f[f.size() - 5] == 'B')
				{
					add(word, translations, mainData);
				}
				else if (f[f.size() - 5] == 'I')
				{
					add(word, translations, importantData);
				}
				else {
					archive.push_back(archiveSingle(word, translations));
				}
				word = "";
				translation = "";
				translations.clear();
			}
			else if (readingWord)
			{
				if (word != "")
				{
					word = word + " " + s;
				}
				else
				{
					word = s;
				}
			}
			else
			{
				if (translation != "")
				{
					translation = translation + " " + s;
				}
				else
				{
					translation = s;
				}
			}
		}
		myfile.close();
	}
}