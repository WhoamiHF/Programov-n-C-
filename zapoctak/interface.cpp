#include "dictionary.h"
#include "interface.h"
#include <string>
#include <iostream>
#include <filesystem>
using namespace std;

//uses printListOfAllFiles to get possible databases and lets user to choose.
void userInterface::startPosibilities(dictionary& dict)
{
	printListOfAllFiles();
	cout << "Please input language from which you want to translate" << endl;
	string first;
	cin >> first;
	cout << "Please input language to which you want to translate" << endl;
	string second;
	cin >> second;
	dict.file = first + "-" + second;
}

//prints every word and its translations from archive
void userInterface::showArchive(dictionary& dict) {
	//system("cls");
	for (auto&& item : dict.archive) {
		cout << item.word << ":";
		for (auto&& tr : item.translations) {
			cout << tr;
		}
		cout << endl;
	}
	cout << endl;
	showMenu(dict);
}

//outputs every single from collection - mainData or importantData
void userInterface::printEvery(list<single>& fromWhere,dictionary& dict)
{
	for (auto&& item : fromWhere)
	{
		cout << item.word << ':';
		cout << item.score << endl;
		for (auto&& tr : item.translations)
		{
			cout << tr << ',';
		}
		cout << endl << endl;
	}
	showMenu(dict);
}

//called from startPosiblities - checks which files are in same folder. Gets just the language parts of the files 
//and removes duplicates. It prints every file which is ".txt" and contains ","
void userInterface::printListOfAllFiles()
{
	namespace fs = std::filesystem;
	cout << "Possibilities:" << endl;
	string m;
	set<string> fileNames;
	std::string path = ".";
	for (const auto& entry : fs::directory_iterator(path))
	{
		string fn = entry.path().filename().string();
		if ((fn.find(".txt") == fn.size() - 4) && (fn.find(",") != string::npos))
		{
			fn = fn.substr(0, fn.find(","));
			fileNames.emplace(fn);
		}
	}
	for (auto&& fn : fileNames)
	{
		cout << fn << endl;
	}
}

//shows menu and gets input from the user then calls correct function
void userInterface::showMenu(dictionary& dict)
{
	cout << "type 'start' to start test" << endl;
	cout << "type 'change' to modify database" << endl;
	cout << "type 'showB' to list all regular words" << endl;
	cout << "type 'showI' to list all important words" << endl;
	cout << "type 'showA' to list all archive words" << endl;
	cout << "type 'settings' to change settings" << endl;
	cout << "type 'end' to end program" << endl;
	string command;
	cin >> command;
	if (command == "start")
	{
		dict.createTest(*this);
	}
	else if (command == "change")   
	{
		dict.modify(*this);
	}
	else if (command == "settings") {
		settings(dict);
	}
	else if (command == "showB") {
		printEvery(dict.mainData,dict);
	}
	else if (command == "showI") {
		printEvery(dict.importantData,dict);
	}
	else if (command == "showA") {
		showArchive(dict);
	}
	else if (command == "end")
	{
		dict.endThisProgram();
	}
	else
	{
		cout << "Please type valid command!" << endl << endl;
		showMenu(dict);
	}
}

//enables to change important settings. During ending of the program it is written into file so its persistent.
void userInterface::settings(dictionary& dict) {
	int choice(0);
	while (choice != 5) {
		cout << "choose number" << endl;
		cout << "1 - change the limit for learned" << endl;
		cout << "2 - change the limit for problematic " << endl;
		cout << "3 - change the size of a test" << endl;
		cout << "4 - change the time limit for one word" << endl;
		cout << "5 - back to the main menu" << endl;
		cin >> choice;
		int newValue;
		switch (choice)
		{
		case 1:
			cout << "insert new limit for learned" << endl;
			cin >> newValue;
			dict.limitUp = newValue;
			break;
		case 2:
			cout << "insert new limit for problematic" << endl;
			cin >> newValue;
			dict.limitDown = newValue;
			break;
		case 3:
			cout << "insert new size of a test" << endl;
			cin >> newValue;
			dict.sizeOfTest = newValue;
			break;
		case 4:
			cout << "insert new time limit" << endl;
			cin >> newValue;
			dict.timeLimit = newValue;
			break;
		default:
			cout << "please choose correct number" << endl;
			break;
		}
	}
	showMenu(dict);
}