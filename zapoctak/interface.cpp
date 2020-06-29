#include "dictionary.h"
#include "interface.h"
#include <string>
#include <iostream>
#include <filesystem>
#include <sstream>
using namespace std;

//uses printListOfAllFiles to get possible databases and lets user to choose.
//used by main
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

//prints every word and its translations from archive then calls showMenu which is the main menu
//used by showA
void userInterface::showArchive(dictionary& dict) {
	//system("cls");
	for (auto&& item : dict.archive) {
		cout << item.word << ":";
		for (auto&& tr : item.translations) {
			cout << tr << " ";
		}
		cout << endl;
	}
	cout << endl;
	showMenu(dict);
}

//outputs every single item its score and its translations from collection - mainData or importantData
//afterwards returns to the main menu. Used by showI or showB.
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


//chooses randomly one of words with multiple translations, then chooses one random translation and couts the translation
// user has to write other posible translation of the choosen word. If succesful the word is removed from synonyms test.
//called in showMenu
void userInterface::synonyms(dictionary& dict) {
	while (dict.multiTranslations.size() != 0) {
		cout << "type 'br3ak' for returning to the main menu" << endl;
		int i = rand() % dict.multiTranslations.size();
		int j = rand() % (dict.multiTranslations[i]->translations.size());
		auto it = next(dict.multiTranslations[i]->translations.begin(), j);
		cout << "fill synonym for: " << *it << endl;
		string answer;
		cin >> answer;
		if (answer == "br3ak") { break; }
		bool correct = false;
		cout << "Group of synonyms: ";
		for (auto&& item : dict.multiTranslations[i]->translations) {
			cout << item << " ";
			if (item == answer)
			{
				correct = true;
			}
		}
		cout << endl;
		if (correct) {
			cout << "correct answer!" << endl;
			dict.wordToMultiple.erase(dict.multiTranslations[i]->word);
			dict.multiTranslations.erase(dict.multiTranslations.begin() + i);
		}
		else {
			cout << "wrong answer!" << endl;
		}
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

//called from showMenu, manages moving oldest words from archive back to main database, user is requiered to
//enter a number smaller than or equal to archive siz - or is asked again and again.
void userInterface::import(dictionary& dict) {
	cout << "The size of archive is " << dict.archive.size() << endl;
	cout << "insert how many words do you wish to move from archive back to main database" << endl;
	size_t answer;
	cin >> answer;
	while (answer > dict.archive.size()) {
		cout << "invalid number, try again" << endl;
		cin >> answer;
	}
	cout << "elements moved: ";
	for (size_t i = 0; i < answer; i++) {
		auto elem = dict.archive.front();
		cout << elem.word<<" ";
		dict.add(elem.word, elem.translations, dict.mainData);
		dict.archive.pop_front();
	}
	cout << endl <<endl;
	showMenu(dict);
}

/*called from createTest. Checks if inputed value corresponds to at least one of correct translations.
checks if the word is learned and if so then moves the word to archive.
also checks if the word is problematic and if so then moves the word to important.
checks also rapidity (time elapsed) of answer*/
void userInterface::testWord(int index, list<single>& data,dictionary& dict)
{
	list<single>::iterator elem;
	if (&data == &dict.mainData)
	{
		elem = dict.intToIteratorMain.find(index)->second;
	}
	else
	{
		elem = dict.intToIteratorImportant.find(index)->second;
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
	if (correct && elapsed_secs <= dict.timeLimit)
	{
		cout << "Correct answer!" << endl;
		elem->score++;
		if (elem->score >= dict.limitUp)
		{
			if (&data == &dict.importantData) {
				string word = elem->word;
				set<string> trs;
				for (auto&& item : elem->translations)
				{
					trs.emplace(item);
				}
				dict.deleteTranslations(elem->word, set<string>(), data);
				dict.add(word, trs, dict.mainData);
			}
			else
			{
				dict.archive.push_back(archiveSingle(elem->word, elem->translations));
				dict.deleteTranslations(elem->word, set<string>(), data);
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
		if (elem->score <= -dict.limitDown && (&data == &dict.mainData))
		{
			string word = elem->word;
			set<string> trs;
			for (auto&& item : elem->translations)
			{
				trs.emplace(item);
			}
			dict.deleteTranslations(elem->word, set<string>(), dict.mainData);
			dict.add(word, trs, dict.importantData);

		}
	}
}

/*called by userInterface::showMenu(), manages adding and deleting words+translations.
  uses string stream so the spaces are needed in between string parts.
*/
void userInterface::modify(dictionary& dict)
{
	cout << "[add/delete] [main/important] [word] - [translation1 , translation2 ...]" << endl;
	cout << "type 'menu' to return to the main menu" << endl;
	string row;
	while (getline(cin, row))
	{
		stringstream sRow(row);
		string command;
		sRow >> command;
		if (command == "menu") {
			showMenu(dict);
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
			dict.add(word, translations, dict.mainData);
		}
		else if (command == "add" && secondCommand == "important")
		{
			dict.add(word, translations, dict.importantData);
		}
		else if (command == "delete" && secondCommand == "main")
		{
			dict.deleteTranslations(word, translations, dict.mainData);
		}
		else if (command == "delete" && secondCommand == "important")
		{
			for (auto&& item : translations) {
				cout << ":" << item << ":";
			}
			dict.deleteTranslations(word, translations, dict.importantData);
		}
	}
}

//shows menu and gets input from the user then calls correct function
//called by main
void userInterface::showMenu(dictionary& dict)
{
	cout << "type 'start' to start translating test" << endl;
	cout << "type 'synonyms' to start practising synonyms" << endl;
	cout << "type 'change' to modify database" << endl;
	cout << "type 'showB' to list all regular words" << endl;
	cout << "type 'showI' to list all important words" << endl;
	cout << "type 'showA' to list all archive words" << endl;
	cout << "type 'import' to import n words from archive" << endl;
	cout << "type 'settings' to change settings" << endl;
	cout << "type 'end' to end program" << endl;
	string command;
	cin >> command;
	if (command == "start")
	{
		dict.createTest(*this);
	}
	else if (command == "synonyms") {
		synonyms(dict);
	}
	else if (command == "import") {
		import(dict);
	}
	else if (command == "change")   
	{
		modify(dict);
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