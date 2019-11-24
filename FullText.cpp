// FullText.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
using namespace std;
#include <fstream>
#include <string>
#include <set>
#include <unordered_set>
#include <unordered_map>

class Solver {
	class Mydatabase
	{
	public:
		Mydatabase(string id, string title, string& words) :id_(id), title_(title),
			words_(words) {};
		string id_;
		string title_;
		string words_;
	};

public:
	list<Mydatabase> mainData;
	using databaseIterator_t = list<Mydatabase>::iterator;
	databaseIterator_t cIterator;
	set<string> cSet;
	using dataItem_t = list < pair<int, databaseIterator_t>>;

	using store_t = list<dataItem_t>;
	using wordToArticleIndex_t = unordered_map<string, store_t::iterator>;
	store_t store;
	wordToArticleIndex_t wordToArticleIndex;
	string Cword;
	bool areAllWordsInAnyArticle = true;
	using sAndIt_t = list<pair<string, dataItem_t::iterator>>;
	sAndIt_t searchedWordsAndIterators;
	using wordToIt_t = unordered_map<string, sAndIt_t::iterator>;
	wordToIt_t wordToItIndex;
	string searchedWord;
	string firstWord="";

	void processIt(char c, int charCount)
	{
		if (isalpha(c)) {
			if (c >= 'A' && c <= 'Z')
			{
				c += 'a' - 'A';
			}

			Cword += c;
		}
		else if (Cword != "")
		{
			addWord(Cword, charCount - Cword.size());
			Cword = "";
		}
	}
	void addWord(string word, int charCount)
	{
		if (cSet.find(word) == cSet.end())
		{
			cSet.emplace(word);
			auto existing = wordToArticleIndex.find(word);
			if (existing != wordToArticleIndex.end() && existing->second != store.end())
			{
				existing->second->push_back({ charCount, cIterator });
				//cout << cIterator->id_;
			}
			else
			{
				dataItem_t newPair;
				newPair.push_back({ charCount,cIterator });
				//cout << cIterator->id_;
				store.push_back(newPair);

				auto newItem = store.end();
				newItem--;

				wordToArticleIndex.emplace(word, newItem);
			}
		}
	}
	void readWords(string& thirdLine)
	{
		char c;
		int charCount = 0;
		for (auto i = thirdLine.begin(); i != thirdLine.end(); i++)
		{
			c = *i;
			processIt(c, charCount);
			charCount += 1;
		}
		addWord(Cword, charCount - Cword.size());
	}

	void readArticles(string& articles)
	{
		ifstream myfile(articles);
		if (myfile.is_open())
		{
			string cID;
			string cTitle;
			string cText;
			string line;
			getline(myfile, line);
			while (line != "")
			{
				cSet.clear();
				cID = line;
				getline(myfile, line);
				cTitle = line;
				getline(myfile, line);
				cText = line;
				mainData.push_back(Mydatabase{ cID,cTitle,cText });
				cIterator = mainData.end();
				cIterator--;
				readWords(line);
				getline(myfile, line);
			}
			myfile.close();
		}
	}

	void find(string word)
	{
		auto iteratorIntoWordToArticleIndex = wordToArticleIndex.find(word);
		if (iteratorIntoWordToArticleIndex != wordToArticleIndex.end() &&
			iteratorIntoWordToArticleIndex->second != store.end())
		{
			auto path = iteratorIntoWordToArticleIndex->second;
			for (auto&& item : *path)
			{
				cout << "[" << item.second->id_ << "]" << " " << item.second->title_ << endl;
				string s = item.second->words_.substr(item.first, 75);
				cout << s << "..." << endl;
			}
		}
	}
	void readSearchedWords(string line)
	{
		char c;
		for (auto i = line.begin(); i != line.end(); i++)
		{
			c = *i;
			if (isalpha(c)) {
				if (c >= 'A' && c <= 'Z')
				{
					c += 'a' - 'A';
				}

				searchedWord += c;
			}
			else if ((searchedWord != "") && (wordToItIndex.find(searchedWord)==wordToItIndex.end()))
			{
				if (wordToArticleIndex.find(searchedWord) != wordToArticleIndex.end())
				{
					addSearchedWord(searchedWord);
				}
				else
				{
					areAllWordsInAnyArticle = false;
				}
			}
		}
	}
	void addSearchedWord(string word) {
		auto existing = wordToArticleIndex.find(word);
		if (existing != wordToArticleIndex.end()) {
			if (firstWord == "")
			{
				firstWord = word;
			}
			searchedWordsAndIterators.push_back({ word,wordToArticleIndex.find(word)->second->begin() });
			sAndIt_t::iterator x = searchedWordsAndIterators.end();
			x--;
			wordToItIndex.insert({ word,x });
			searchedWord = "";
		}
	}
	void intersection() {
		bool isEnd = false;
		while (!isEnd) {
			string min;
			bool areEqual = true;
			bool isFirst = true;;
			for (auto&& item : searchedWordsAndIterators)
			{
				if (item.second != (wordToArticleIndex.find(item.first))->second->end()) {
					if (isFirst)
					{
						isFirst = false;
						min = item.second->second->id_;
					}
					else if (item.second->second->id_ < min)
					{
						min = item.second->second->id_;
						areEqual = false;
					}
					else if (item.second->second->id_ > min)
					{
						areEqual = false;
					}
				}
				else
				{
					isEnd = true;
				}

			}
			if (!isEnd)
			{
				if (areEqual)
				{
					cout << "[" << searchedWordsAndIterators.begin()->second->second->id_ << "] ";
					cout << searchedWordsAndIterators.begin()->second->second->title_ << endl;
					int x =wordToItIndex.find(firstWord)->second->second->first;
					auto path = searchedWordsAndIterators.begin()->second->second;
					cout << path->words_.substr(x, 75) << "..." << endl;

				}
				for (auto&& item : searchedWordsAndIterators)
				{
					if (item.second->second->id_ == min)
					{
						item.second++;
					}
				}
			}
		}
		
	}
	void readCommandsFile(string source, bool isFile)
	{
		string line;
		if (isFile) {
			ifstream commandsFile(source);
			if (commandsFile.is_open()) {

				while (getline(commandsFile, line))
				{
					searchedWord = "";
					firstWord = "";
					areAllWordsInAnyArticle = true;
					readSearchedWords(line);
					if (searchedWord != "")
					{
						if (wordToArticleIndex.find(searchedWord) != wordToArticleIndex.end())
						{
							addSearchedWord(searchedWord);
						}
						else
						{
							areAllWordsInAnyArticle = false;
						}
					}
					if (areAllWordsInAnyArticle && !searchedWordsAndIterators.empty()) 
					{
						intersection();
					}
					else if(!searchedWordsAndIterators.empty()) {
						cout << "No results" << endl;
					}
					searchedWordsAndIterators.clear();
					wordToItIndex.clear();
					cout << endl;
				}
			}
		}
		else
		{
			while (getline(cin, line))
			{
				areAllWordsInAnyArticle = true;
				readSearchedWords(line);
				intersection();
				searchedWordsAndIterators.clear();
				wordToItIndex.clear();
			}
		}

	}
};
int main(int argc, char** argv)
{
	Solver solv;
	string articles = argv[1];
	solv.readArticles(articles);

	//solv.find("on");
	string commands;
	if (argc > 2)
	{
		commands = argv[2];
		solv.readCommandsFile(commands, true);
	}
	else {
		solv.readCommandsFile("", false);
	}



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
