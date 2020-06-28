#pragma once
class dictionary;
class userInterface {
public:
	void startPosibilities(dictionary& dict);
	void printListOfAllFiles();
	void showMenu(dictionary& dict);
	void settings(dictionary& dict);
	void showArchive(dictionary& dict);
	void printEvery(std::list<single>& fromWhere,dictionary& dict);
};