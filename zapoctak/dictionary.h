#pragma once
#include <string>
#include <list>
#include <set>
#include <map>
#include <deque>
#include <fstream>
#include <iostream>
#include <vector>

class dictionary;
class userInterface;

//one unit of data, dictionary contains two list of those.
class single {
public:
	single(std::string word_, std::set<std::string>& translations_, size_t id_) :word(word_), translations(translations_), score(0), id(id_) {}
	std::string word;
	int score;
	std::set<std::string> translations;
	size_t id;
};

//archive doesn't need to remember score or id
class archiveSingle {
public:
	archiveSingle(std::string word_, std::set<std::string>& translations_) :word(word_), translations(translations_){}
	std::string word;
	std::set<std::string> translations;
};

/*main class - deals with dictionary databases. Stores data in "mainData", "importantData" and archive. Builds three indexes:
1) wordToIterator - uses word as key. Used in modyfing for deleting. 
2+3) intToIteratorMain/intToIteratorImportant - uses integer as key. Used for making tests.
*/
class dictionary {
public:
	friend userInterface;

	dictionary() :sizeOfTest(3), limitUp(1), limitDown(1), gapImportant(false), gapMain(false),timeLimit(5) {}
	void read();
	std::string file;
private:

	//testing
	void createTest(userInterface& ui);
	void testWord(int index, std::list<single>& data);
	void synonyms(userInterface& ui);

	//settings
	unsigned sizeOfTest;
	int limitUp;
	int limitDown;
	int timeLimit;
	void writeSettings();

	//data storages
	std::list<single> importantData;
	std::list<single> mainData;
	std::deque<archiveSingle> archive;

	void modify(userInterface& ui);
	void deleteTranslations(std::string word, std::set<std::string> translations, std::list<single>& collection);
	void add(std::string word, std::set<std::string>& translations, std::list<single>& whereTo);


	//indexes
	std::map<int, std::list<single>::iterator> intToIteratorMain;
	std::map<int, std::list<single>::iterator> intToIteratorImportant;
	std::map<std::string, std::list<single>::iterator> wordToIterator;

	std::vector< std::list<single>::iterator> multiTranslations;

	//determines if rebuilding of a index is needed.
	bool gapMain;
	bool gapImportant;
	void rebuildIndex(std::map<int, std::list<single>::iterator>& index);

	//called by endThisProgram(), manages writting into files - archive has different type so template was needed.
	template<typename s>
	void writeIntoFile(std::string filename, s& fromWhere)
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
};

