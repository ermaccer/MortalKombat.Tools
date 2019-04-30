// fsyscreate.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "fsys.h"
#include "dirent.h"
#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <windows.h>
#include <string>

std::streampos getSizeToEnd(std::ifstream& is)
{
	auto currentPosition = is.tellg();
	is.seekg(0, is.end);
	auto length = is.tellg() - currentPosition;
	is.seekg(currentPosition, is.beg);
	return length;
}



int main(int argc, char* argv[])
{

	if (argc != 3) {
		std::cout << "Usage fsyscreate <folder> <output>" << std::endl;
		return 1;
	}
	dirent* file;
	DIR* pDir;
	pDir = opendir(argv[1]);
	int filesfound = 0;

	if (!pDir)
	{
		std::cout << "ERROR: Could not open directory " << argv[1] << "!" << std::endl;
		return 1;
	}

	std::ofstream oFile(argv[2], std::ofstream::binary);

	// count files
	while ((file = readdir(pDir)) != 0)
	{
		filesfound++;
	}

	closedir(pDir);
	// create arrays
	int* offsets, *sizes;
	std::string* names;
	FSYSEntry* fsys;

	offsets = new int[filesfound];
	sizes = new int[filesfound];
	names = new std::string[filesfound];
	fsys = new FSYSEntry[filesfound];

	if (offsets == nullptr || names == nullptr || sizes == nullptr)
		std::cout << "ERROR: Could not allocate memory!" << std::endl;

	// get names & sizes

	for (int i = 2; i < filesfound; i++)
	{
		std::string input = std::to_string(i - 2) + ".dat";
		std::string folder = argv[1];
		// enter folder
		input.insert(0, argv[1]);
		input.insert(folder.length(), "\\");
		names[i] = input;
		std::ifstream pFile(names[i], std::ifstream::binary);
		if (pFile) sizes[i] = (int)getSizeToEnd(pFile);
	}

	char pad[2700] = {};



	// calc base offset
	int baseOffset = ((799 * sizeof(FSYSEntry)) + sizeof(pad));
	//writing entries
	for (int i = 2; i < filesfound; i++)
	{

		fsys[i].size = sizes[i];
		fsys[i].unknown = bigValues[i - 2];
		baseOffset += fsys[i - 1].size;
		fsys[i].offset = baseOffset;
		oFile.write((char*)&fsys[i], sizeof(FSYSEntry));
	}
	// write pad

	oFile.write((char*)&pad, sizeof(pad));

	for (int i = 0; i < filesfound; i++)
	{
		std::ifstream pFile(names[i], std::ifstream::binary);

		// write data
		if (pFile)
		{
			auto dataSize = getSizeToEnd(pFile);
			auto dataBuff = std::make_unique<char[]>(dataSize);
			pFile.read(dataBuff.get(), dataSize);
			oFile.write(dataBuff.get(), dataSize);
		}

	}

	return 0;
}