#include "fileio.h"
#include "posixfile.h"
#include <iostream>

int main(int argc, char *argv[])
{
	std::cout << "Exe path: " << FileIO::Location::GetExecutablePath() << std::endl;
	std::cout << "Save path: " << FileIO::Location::GetSaveGameFolder() << std::endl;
}


