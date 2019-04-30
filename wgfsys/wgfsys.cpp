// wgfsys.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <windows.h>
#include <string>
#include "dirent.h"




#pragma pack (push,1)
struct FSYSEntry {
	unsigned int          end;
	char                  filename[13] = {};

};
#pragma (pop)

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

	if (argc == 1) {
		std::cout << "Usage wgfsys <mode> <input> <output> \n Modes: \n create (pass folder as input) \n extract (output is folder) " << std::endl;
		return 1;
	}



	if (strcmp(argv[1], "extract") == 0)
	{

		std::ifstream pFile(argv[2], std::ifstream::binary);

		if (!pFile)
		{
			std::cout << "ERROR: Could not open " << argv[2] << "!" << std::endl;
			return 1;
		}

		if (pFile)
		{
			CreateDirectory((LPCSTR)argv[3], NULL);

			int files;
			// read headers
			pFile.read((char*)&files, sizeof(int));
			FSYSEntry* fsys;
			fsys = new FSYSEntry[files];

			if (fsys == nullptr)
				std::cout << "ERROR: Could not allocate memory!" << std::endl;

			for (int i = 0; i < files; i++)
			{
				pFile.read((char*)&fsys[i], sizeof(FSYSEntry));
			}

			pFile.seekg((sizeof(FSYSEntry) * files + sizeof(int)), pFile.beg);

			//get files
			for (int i = 0; i < files - 1; i++)
			{

				std::string output = argv[3];
				output += "\\";
				output += fsys[i].filename;
				std::ofstream oFile(output, std::ofstream::binary);

				auto dataSize = fsys[i + 1].end - fsys[i].end;
				auto dataBuff = std::make_unique<char[]>(dataSize);
				pFile.read(dataBuff.get(), dataSize);
				oFile.write(dataBuff.get(), dataSize);
			}
		}

	}
	if (strcmp(argv[1], "create") == 0)
		{
			dirent* file;
			DIR* pDir;
			int filesfound = 0;
			pDir = opendir(argv[2]);
			if (!pDir)
			{
				std::cout << "ERROR: Could not open directory " << argv[2] << "!" << std::endl;
				return 1;
			}

			// create output
			std::ofstream oFile(argv[3], std::ofstream::binary);

			// count files
			while ((file = readdir(pDir)) != 0)
			{
				filesfound++;
			}
			closedir(pDir);
			// create arrays
			int* sizes;
			std::string* names;
			FSYSEntry* fsys;

			sizes = new int[filesfound];
			names = new std::string[filesfound];
			fsys = new FSYSEntry[filesfound];

			if (fsys == nullptr || names == nullptr || sizes == nullptr)
				std::cout << "ERROR: Could not allocate memory!" << std::endl;



	
			int itr = 0;
			pDir = opendir(argv[2]);
			// get names & sizes
			while ((file = readdir(pDir)) != 0)
			{

				std::string input = file->d_name;
				if (input.length() > 13) {
					std::cout << "ERROR: " << input << " filename is too long! (13 max)" << std::endl;
					return 1;
				}
				std::string folder = argv[2];
				input.insert(0, argv[2]);
				input.insert(folder.length(), "\\");

				names[itr] = file->d_name;


				// get size
				std::ifstream pFile(input, std::ifstream::binary);
				if (pFile) sizes[itr] = (int)getSizeToEnd(pFile);

				itr++;
			}
			closedir(pDir);

			// create dat
			int temp = filesfound - 1;
			oFile.write((char*)&temp, sizeof(int));
			int totalsize = 0;

			// calc base offset
			int baseOffset = ((temp  * sizeof(FSYSEntry)) + sizeof(int));
			totalsize = baseOffset;
			//create entries
			for (int i = 2; i < filesfound; i++)
			{
				sprintf(fsys[i].filename, "%s", names[i].c_str());
				fsys[i].end = baseOffset;
				baseOffset += sizes[i];
				oFile.write((char*)&fsys[i],sizeof(FSYSEntry));
				totalsize += sizes[i];
			}
			// write archive size

			oFile.write((char*)&totalsize, sizeof(int));
			char pad[13] = {};
			oFile.write((char*)&pad, sizeof(pad));

			// write file data

			for (int i = 0; i < filesfound - 2; i++)
			{

				std::string input = names[i + 2];
				std::string folder = argv[2];
				input.insert(0, argv[2]);
				input.insert(folder.length(), "\\");

				std::ifstream pFile(input, std::ifstream::binary);

				// write data
				if (pFile)
				{
					
					auto dataSize = getSizeToEnd(pFile);
					auto dataBuff = std::make_unique<char[]>(dataSize);
					pFile.read(dataBuff.get(), dataSize);
					oFile.write(dataBuff.get(), dataSize);
				}

			}
		}
	

	return 0;
}