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
	class Article
	{
	public:
		Article(string id, string title, string& words) :id_(id), title_(title),
			words_(words) {};
		string id_;
		string title_;
		string words_;
	};

public:
	using databaseIterator_t = list<Article>::iterator;
	using dataItem_t = list < pair<int, databaseIterator_t>>;
	using store_t = list<dataItem_t>;
	using wordToArticleIndex_t = unordered_map<string, store_t::iterator>;
	using sAndIt_t = vector<pair<string, dataItem_t::iterator>>;
	using wordToIt_t = unordered_map<string, sAndIt_t::iterator>;

	list<Article> mainData;
	store_t store;
	wordToArticleIndex_t wordToArticleIndex;


	void processOneLetter(char c, int charCount,string& cWord, unordered_set<string>& addedWords, databaseIterator_t newArticle)
	{
		if (isalpha(c)) {
			if (c >= 'A' && c <= 'Z')
			{
				c += 'a' - 'A';
			}

			cWord += c;
		}
		else if (cWord != "")
		{
			addWord(cWord, charCount - cWord.size(),addedWords,newArticle);
			cWord = "";
		}
	}
	void addWord(string word, int charCount,unordered_set<string>& addedWords,databaseIterator_t newArticle)
	{
		if (addedWords.find(word) == addedWords.end())
		{
			addedWords.emplace(word);
			auto existing = wordToArticleIndex.find(word);
			if (existing != wordToArticleIndex.end() && existing->second != store.end())
			{
				existing->second->push_back({ charCount, newArticle });
				//cout << cIterator->id_;
			}
			else
			{
				dataItem_t newPair;
				newPair.push_back({ charCount,newArticle });
				//cout << cIterator->id_;
				store.push_back(newPair);

				auto newItem = store.end();
				newItem--;

				wordToArticleIndex.emplace(word, newItem);
			}
		}
	}
	void readWords(string& thirdLine,databaseIterator_t newArticle)
	{
		unordered_set<string> addedWords;
		char c;
		string Cword;
		int charCount = 0;
		for (auto i = thirdLine.begin(); i != thirdLine.end(); i++)
		{
			c = *i;
			processOneLetter(c, charCount,Cword,addedWords,newArticle);
			charCount += 1;
		}
		addWord(Cword, charCount - Cword.size(),addedWords,newArticle);
	}

	void readArticles(string& articles)
	{
		ifstream myfile(articles);
		if (myfile.is_open())
		{
			string currID;
			string currTitle;
			string currText;
			string line;
			getline(myfile, line);
			while (line != "")
			{
				currID = line;
				getline(myfile, line);
				currTitle = line;
				getline(myfile, line);
				currText = line;
				mainData.push_back(Article{ currID,currTitle,currText });
				auto cIterator = mainData.end();
				cIterator--;
				readWords(line,cIterator);
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
	bool readSearchedWords(string line, sAndIt_t& searchedWordsAndIterators)
	{
		char c;
		string searchedWord;
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
			if ((searchedWord != "")&&(!isalpha(c)||i ==--line.end()))
			{
				if (wordToArticleIndex.find(searchedWord) != wordToArticleIndex.end())
				{
					addSearchedWord(searchedWord,searchedWordsAndIterators);
					searchedWord = "";
				}
				else
				{
					return 0;
				}
			}
		}
		return 1;
	}
	void addSearchedWord(string word,sAndIt_t& searchedWordsAndIterators) {
		auto existing = wordToArticleIndex.find(word);
		if (existing != wordToArticleIndex.end()) {
			searchedWordsAndIterators.push_back({ word,wordToArticleIndex.find(word)->second->begin() });
			sAndIt_t::iterator x = searchedWordsAndIterators.end();
			x--;
		}
	}
	void intersection(sAndIt_t& searchedWordsAndIterators) {
		bool isEnd = false;
		while (!isEnd) {
			string min;
			bool areEqual = true;
			bool isFirst = true;
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
					
					int x = searchedWordsAndIterators.begin()->second->first;
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
	void readCommandsFile(istream& source)
	{
		sAndIt_t searchedWordsAndIterators;
		string line;
		while (getline(source, line))
		{

			bool areAllWordsInAnyArticle = readSearchedWords(line,searchedWordsAndIterators);

			if (areAllWordsInAnyArticle && !searchedWordsAndIterators.empty())
			{
				intersection(searchedWordsAndIterators);
			}
			else if (!searchedWordsAndIterators.empty()) {
				cout << "No results" << endl;
			}
			searchedWordsAndIterators.clear();
			cout << endl;
		}
	}
};
int main(int argc, char** argv)
{
	Solver solv;
	if (argc > 1) {
		string articles = argv[1];
		solv.readArticles(articles);
	}
	string commands;
	if (argc > 2)
	{
		commands = argv[2];
		ifstream commandsFile(commands);
		if (commandsFile.is_open()) {

			solv.readCommandsFile(commandsFile);
		}
		commandsFile.close();
	}
	else
	{
		solv.readCommandsFile(cin);
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
