#include "dictionary.h"
#include "interface.h"
#include <string>
#include <fstream>
#include <iostream>
#include <unordered_set>
using namespace std;


//called from userInterface::showMenu(), writes changed dictionary into files and ends program
// writeIntoFile is in dictionary.h because it's a template which is templated by type of container passed as second argument
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



/*manages adding words and it's translations
 is called by: userInterface::import,userInterface::testWord,userInterface::modify and dictionary::read
 adds to wordToIterator index and to intToIteratorMain or intToIteratorImportant depending on importance of the word
 */
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
		if (translations.size() > 1 && wordToMultiple.find(word) == wordToMultiple.end()) {
			multiTranslations.push_back(newItem);
			wordToMultiple.emplace(word, multiTranslations.size() - 1);			
		}
	}
}



/*creates test, according to chances there should be more important words than from important (60:40) assuming both
databases are large enough. Then tests each of them (see userInterface::testWord)
uses integer as key and that's the reason for intToIterator indices. Afterwards returns to the main menu.*/
void dictionary::createTest(userInterface& ui)
{
	/*for (auto&& item : multiTranslations) {
		cout << item->word;
	}*/
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
		ui.testWord(item, mainData,*this);
	}
	for (auto item : setOfChoosenImportant)
	{
		ui.testWord(item, importantData,*this);
	}
	cout << "Test finished!" << endl;
	ui.showMenu(*this);
}

//deals with gaps in indices. Needed for making test - picking random number, called by createTest.
void dictionary::rebuildIndex(map<int, list<single>::iterator>& index) {
	map<int, list<single>::iterator> tmp;
	for (auto&& item : index) {
		tmp.emplace(tmp.size(), item.second);
	}
	index = tmp;
}


//deletes listed translations,is called from userInterface::modify and also from userInterface::testWord.
//if the list of translations is empty then every translation is deleted and the word is removed.
void dictionary::deleteTranslations(string word, set<string> translations, list<single>& collection)
{

	auto elem = wordToIterator.find(word);
	if (elem != wordToIterator.end())
	{
		auto tr = elem->second->translations;
		bool wasInMultiple = tr.size() >1;
		list<string>::iterator delIt;
		for (auto&& translationToDelete : translations)
		{
			auto delIt = tr.find(translationToDelete);
			if (delIt != tr.end()) {
				tr.erase(delIt);
			}
		}
		if ((tr.empty() || translations.size() <= 1) && wasInMultiple) {
			auto multipleIt = wordToMultiple.find(word);
			multiTranslations.erase(multiTranslations.begin()+multipleIt->second);
			wordToMultiple.erase(multipleIt);
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