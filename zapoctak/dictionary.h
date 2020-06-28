#pragma once
#include <string>
#include <list>
#include <set>
#include <map>
#include <deque>
#include <fstream>
#include <iostream>

class dictionary;
class single {
public:
	single(std::string word_, std::set<std::string>& translations_, size_t id_) :word(word_), translations(translations_), score(0), id(id_) {}
	std::string word;
	int score;
	std::set<std::string> translations;
	size_t id;
};

class archiveSingle {
public:
	archiveSingle(std::string word_, std::set<std::string>& translations_) :word(word_), translations(translations_){}
	std::string word;
	std::set<std::string> translations;
};

class userInterface;
class dictionary {
public:
	friend userInterface;
	dictionary() :sizeOfTest(3), limitUp(1), limitDown(1), gapImportant(false), gapMain(false),timeLimit(5) {}
	void read();
	std::string file;
private:
	void createTest(userInterface& ui);
	void testWord(int index, std::list<single>& data);
	void deleteTranslations(std::string word, std::set<std::string> translations, std::list<single>& collection);
	void add(std::string word, std::set<std::string>& translations, std::list<single>& whereTo);
	void rebuildIndex(std::map<int, std::list<single>::iterator>& index);
	void writeArchive();
	//void writeIntoFile(std::string filename, std::list<single>& fromwhere);

	//called by endThisProgram(), manages writting into files
//template<typename s>
	void writeIntoFile(std::string filename, std::list<single>& fromWhere)
	{
		std::ofstream myfile(filename);

		for (auto&& item : fromWhere)
		{
			myfile << item.word << " - ";
			for (auto it = item.translations.begin(); it != item.translations.end(); ++it)
			{
				if (it != item.translations.begin())
				{
					myfile << " , ";
				}
				myfile << *it;
			}
			myfile << " ; ";
		}
	}

	void endThisProgram();
	void modify(userInterface& ui);
	unsigned sizeOfTest;
	int limitUp;
	int limitDown;
	int timeLimit;
	std::list<single> importantData;
	std::list<single> mainData;
	std::deque<archiveSingle> archive;
	std::map<int, std::list<single>::iterator> intToIteratorMain;
	std::map<int, std::list<single>::iterator> intToIteratorImportant;
	bool gapMain;
	bool gapImportant;
	std::map<std::string, std::list<single>::iterator> wordToIterator;
};

