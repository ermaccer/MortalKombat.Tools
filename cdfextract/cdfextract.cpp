// cdfextract.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <memory>

struct CDFHeader {
	char                  archivename[12];
	int                   files;

};


struct CDFEntry {
    int                   unk;
	char                  filename[12];
	int                   offset;

};

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
	if (argc != 2) {
		std::cout << "Usage cdfextract <input>" << std::endl;
		return 1;
	}

	std::ifstream pFile(argv[1], std::ifstream::binary);

	if (!pFile)
	{
		std::cout << "ERROR: Could not open: " << argv[1] << "!" << std::endl;
		return 1;
	}

	if (pFile)
	{
		CDFHeader cdf;
		pFile.read((char*)&cdf, sizeof(CDFHeader));


		std::unique_ptr<CDFEntry[]> ent = std::make_unique<CDFEntry[]>(cdf.files + 1);
	
		// get entries
		for (int i = 0; i < cdf.files; i++)
		{
			pFile.read((char*)&ent[i], sizeof(CDFEntry));
		}

		// extract data
		ent[cdf.files].offset = (int)getSizeToEnd(pFile);
		for (int i = 0; i < cdf.files; i++)
		{
			pFile.seekg(ent[i].offset, pFile.beg);

			int dataSize = ent[i + 1].offset - ent[i].offset;
			std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(dataSize);

			pFile.read(dataBuff.get(), dataSize);

			// create output
			std::string output = cdf.archivename;
			output += "_out\\";
			std::experimental::filesystem::create_directory(output);
			output += ent[i].filename;
			std::ofstream oFile(output, std::ofstream::binary);

			std::cout << "Processing: " << ent[i].filename << std::endl;
			oFile.write(dataBuff.get(), dataSize);
		}
		std::cout << "Finished." << std::endl;


	}
    return 0;
}

