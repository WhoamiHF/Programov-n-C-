#pragma once
class dictionary;

//class for comunication with user - input or output to console.
class userInterface {
public:
	void startPosibilities(dictionary& dict);
	void printListOfAllFiles();
	void showMenu(dictionary& dict);
	void settings(dictionary& dict);
	void showArchive(dictionary& dict);
	void testWord(int index, std::list<single>& data, dictionary& dict);
	void synonyms(dictionary& dict);
	void modify(dictionary& dict);
	void printEvery(std::list<single>& fromWhere,dictionary& dict);
	void import(dictionary& dict);
};