// PAKTool.cpp : Defines the entry point for the console application.
//
// bit info:
// file struct is right after file data
// each string needs to be padded to 4

#include "stdafx.h"
#include <iostream>
#include <filesystem>
#include <fstream>
#include "filef.h"
#include <memory>
#include <string>


struct PAKHeader {
	int     header; // PAK 
	int     check; // 1
	int     files;
	int     structOffset;
};

struct PAKEntry {
	int    pathOffset;
	int    offset;
	int    fileSize;
};

enum eModes {
	MODE_EXTRACT = 1,
	MODE_CREATE
};

int main(int argc, char* argv[])
{
	if (argc < 3) {
		std::cout << "Usage: paktool <params> <file/folder>\n"
			<< "    -c  Creates archive from a folder\n"
			<< "    -e  Extracts archive\n";
		return 1;
	}

	int mode = 0;

	// params
	for (int i = 1; i < argc - 1; i++)
	{
		if (argv[i][0] != '-' || strlen(argv[i]) != 2) {
			return 1;
		}
		switch (argv[i][1])
		{
		case 'e': mode = MODE_EXTRACT;
			break;
		case 'c': mode = MODE_CREATE;
			break;
		default:
			std::cout << "ERROR: Param does not exist: " << argv[i] << std::endl;
			break;
		}
	}

	if (mode == MODE_EXTRACT)
	{
		std::ifstream pFile(argv[argc - 1], std::ifstream::binary);

		if (!pFile)
		{
			std::cout << "ERROR: Could not open " << argv[argc - 1] << "!" << std::endl;
			return 1;
		}

		if (pFile)
		{
			PAKHeader pak;
			pFile.read((char*)&pak, sizeof(PAKHeader));
			if (!(pak.header == 'PAK ')) {
				std::cout << "ERROR: " << argv[argc - 1] << " is not a valid MKA archive!" << std::endl;
				return 1;
			}

			// dump data for recreating                                   2048 - header size
			std::unique_ptr<char[]> dataBuffer = std::make_unique<char[]>(2032);
			pFile.read(dataBuffer.get(), 2032);

			std::ofstream outData("data.tmp", std::ifstream::binary);
			outData.write(dataBuffer.get(), 2032);

			// go to file struct like in ssf
			pFile.seekg(pak.structOffset, pFile.beg);

			int currentPos = (int)pFile.tellg();

			// get ents
			std::unique_ptr<PAKEntry[]> ent = std::make_unique<PAKEntry[]>(pak.files);

			for (int i = 0; i < pak.files; i++)
			{
				pFile.read((char*)&ent[i], sizeof(PAKEntry));
			}

			// get paths
			std::unique_ptr<std::string[]> path = std::make_unique<std::string[]>(pak.files);
		
			for (int i = 0; i < pak.files; i++) 
			{
				pFile.seekg(pak.structOffset + (sizeof(PAKEntry) * pak.files), pFile.beg);
				pFile.seekg(ent[i].pathOffset, pFile.cur);
				std::getline(pFile,path[i], '\0');
			}

			// extract files

			for (int i = 0; i < pak.files; i++)
			{
				pFile.seekg(ent[i].offset, pFile.beg);

				std::cout << "Processing: " << path[i] << std::endl;

				std::unique_ptr<char[]> dataBuffer = std::make_unique<char[]>(ent[i].fileSize);
				pFile.read(dataBuffer.get(), ent[i].fileSize);

				// create output
				std::experimental::filesystem::create_directories(splitString(path[i], false));

				std::ofstream oFile(path[i], std::ofstream::binary);
				oFile.write(dataBuffer.get(), ent[i].fileSize);
		       
			}
			std::cout << "Finished." << std::endl;
		}


	}
	if (mode == MODE_CREATE)
	{
		std::experimental::filesystem::path folder(argv[argc - 1]);
		if (!std::experimental::filesystem::exists(folder))
		{
			std::cout << "ERROR: Could not open directory: " << argv[argc - 1] << "!" << std::endl;
			return 1;
		}

		if (std::experimental::filesystem::exists(folder))
		{

			int filesFound = 0;
			int foldersFound = 0;
			std::cout << "Processing folder: " << argv[argc - 1] << std::endl;
			// get files number
			for (const auto & file : std::experimental::filesystem::recursive_directory_iterator(folder))
			{
				filesFound++;
				if (std::experimental::filesystem::is_directory(file)) foldersFound++;

			}
			filesFound -= foldersFound;

			// get stuff
			std::unique_ptr<std::string[]> filePaths = std::make_unique<std::string[]>(filesFound);
			std::unique_ptr<int[]> sizes = std::make_unique<int[]>(filesFound);
			int structSize = 0;
			int i = 0;
			for (const auto & file : std::experimental::filesystem::recursive_directory_iterator(folder))
			{
				if (!std::experimental::filesystem::is_directory(file))
				{
					filePaths[i] = file.path().string();
					std::ifstream tFile(filePaths[i], std::ifstream::binary);
					if (tFile)
					{
						sizes[i] = (int)getSizeToEnd(tFile);
						structSize += (int)getSizeToEnd(tFile);
					}
						
					i++;

				}
			}

		  // build header
			PAKHeader pak;
			pak.header = 'PAK ';
			pak.check = 256;
			// pad size from ssf
			pak.structOffset = 2048 + structSize;
			pak.files = filesFound;


			// output file
			std::string output = argv[argc - 1];
			output += ".pak";
			std::ofstream oFile(output, std::ofstream::binary);

			// write header
			oFile.write((char*)&pak, sizeof(PAKHeader));

			// get dump of .pak (dunno what's in that section, some windows messages, repetition of header)
			std::ifstream dataDump("data.tmp", std::ifstream::binary);
			if (!dataDump) {
				std::cout << "ERROR: Could not open data.tmp!" << std::endl;
				return 1;
			}
			auto dataSize = getSizeToEnd(dataDump);
			std::unique_ptr<char[]> tempBuffer = std::make_unique<char[]>(dataSize);
			dataDump.read(tempBuffer.get(), dataSize);
			oFile.write(tempBuffer.get(), dataSize);

			// write file data

			for (int i = 0; i < pak.files; i++)
			{
				std::ifstream pFile(filePaths[i], std::ifstream::binary);
				auto dataSize = getSizeToEnd(pFile);
				if (pFile)
				{
					std::unique_ptr<char[]> dataBuffer = std::make_unique<char[]>(dataSize);
					std::cout << "Processing: " << filePaths[i] << std::endl;
					pFile.read(dataBuffer.get(), dataSize);
					oFile.write(dataBuffer.get(), dataSize);
				}
				if (!pFile)
				{
					std::cout << "ERROR: Could not open " << argv[argc - 1] << "!" << std::endl;
					return 1;
				}

			}
			
			// write file entries
			int baseOffset = 2048;
			int basePathOffset = 0;
			for (int i = 0; i < pak.files;i++)
			{
				PAKEntry ent;
				ent.offset = baseOffset;
				ent.fileSize = sizes[i];
				ent.pathOffset = basePathOffset;
				oFile.write((char*)&ent, sizeof(PAKEntry));
				baseOffset += sizes[i];
				basePathOffset += calcOffsetFromPad(filePaths[i].length() + 1, 8);
			}

			// write paths

			for (int i = 0; i < pak.files; i++)
			{
				oFile.write(filePaths[i].c_str(), filePaths[i].length() + 1);
				int val = filePaths[i].length() + 1;
				if (!(val % 8 == 0))
				{
					do
					{
						char pad = 0;
						oFile.write((char*)&pad, sizeof(char));
						val++;
						
					} while (val % 8 != 0);

				}
			}
			std::cout << "Finished." << std::endl;
		}
	}

	return 0;
}

