// zapoctak.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

using namespace std; 
#include <time.h>      
#include<fstream>		
#include "dictionary.h"
#include "interface.h"



int main()
{
	//try to get better encoding
	locale cz("Czech");
	locale::global(cz);
	setlocale(LC_ALL, "");

	//preparation for random numbers
	srand(int(time(NULL)));


	dictionary Dict;
	userInterface usI;
	usI.startPosibilities(Dict);

	//reads main data,important data, archive data and settings.
	Dict.read();

	usI.showMenu(Dict);

}

