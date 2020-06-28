// zapoctak.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

using namespace std; 
#include <time.h>      
#include<fstream>		
#include "dictionary.h"
#include "interface.h"



int main()
{

	locale cz("Czech");
	locale::global(cz);
	setlocale(LC_ALL, "");

	srand(int(time(NULL)));


	dictionary Dict;
	userInterface usI;
	usI.startPosibilities(Dict);
	Dict.read();
	usI.showMenu(Dict);

}

