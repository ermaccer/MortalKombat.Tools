// fsysextract.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <windows.h>
#include <string>


struct FSYSEntry {
	unsigned int          unknown;
	unsigned int          offset;
	unsigned int          size;

};


// no more files allowed anyway
FSYSEntry fsys[799];


int main(int argc, char* argv[])
{

	if (argc != 2) {
		std::cout << "Usage fsysextract <input>" << std::endl;
		return 1;
	}

	std::ifstream pFile(argv[1], std::ifstream::binary);

	if (!pFile)
		printf("ERROR: Could not open %s!", argv[1]);



	if (pFile)
	{
		CreateDirectory((LPCSTR)"extract", NULL);

		// read headers
		pFile.read((char*)&fsys, sizeof(fsys));
		// skip padding
		char   pad[2700];
		pFile.read((char*)&pad, sizeof(pad));

		//get files
		for (int i = 0; i < 799; i++)
		{

			std::string output = "extract\\";
			output.insert(output.length(), std::to_string(i) + ".dat");
			std::ofstream oFile(output, std::ofstream::binary);
			pFile.seekg(fsys[i].offset, pFile.beg);
			auto dataSize = fsys[i].size;
			auto dataBuff = std::make_unique<char[]>(dataSize);
			pFile.read(dataBuff.get(), dataSize);
			oFile.write(dataBuff.get(), dataSize);
		}
	}

	return 0;
}