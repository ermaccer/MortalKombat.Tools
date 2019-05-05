// ssfx.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <filesystem>
#include <sstream>


struct SSFHeader {
	int                   header; //  CES
	int                   val1;
	int                   val2;
	int                   val3;
	int                   files;
	int                   val4;
	int                   filesize;

};

struct SSFEntry {
	int            type;
	int            offset;
	int            size;
	int            pad; // 0x7C801A4F
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

	if (argc == 1 || argc == 2 || argc == 3) {
		std::cout << "Usage ssfx <mode> <input> <param> <pad>" << std::endl;
		return 1;
	}

	if (!(strcmp(argv[1], "extract") == 0 || strcmp(argv[1], "create") == 0))
	{
		std::cout << "ERROR: Invalid mode" << std::endl;
		return 1;
	}

	if (strcmp(argv[1], "extract") == 0)
	{
		std::ifstream pFile(argv[2], std::ifstream::binary);
		bool isbig;
		if (!pFile)
		{
			std::cout << "ERROR: Coult not open " << argv[2] << "!" << std::endl;
			return 1;
		}
		



		if (pFile)
		{
			// read headers
			SSFHeader ssf;
			pFile.read((char*)&ssf, sizeof(ssf));

			if (ssf.header != 'SEC ')
			{
				std::cout << "ERROR: " << argv[2] << "is not a SEC type archive!" << std::endl;
				return 1;
			}

			auto ssfe = std::make_unique<SSFEntry[]>(ssf.files);
			auto dataValues = std::make_unique<std::string[]>(ssf.files);
			int values = 0;
			std::string line;
			// read entries
			for (int i = 0; i < ssf.files; i++)
			{
				pFile.read((char*)&ssfe[i], sizeof(SSFEntry));
			}

			if (!(strcmp(argv[3], "small") == 0 || strcmp(argv[3], "big") == 0))
			{
				std::cout << "ERROR: Invalid value for <pad>!" << std::endl;
				return 1;
			}
			if ((strcmp(argv[3], "small") == 0)) isbig = false;
			if ((strcmp(argv[3], "big") == 0)) isbig = true;

			// skip pad
			int padsize;
			if (isbig == false)
			{
				padsize = 0;

				while (std::getline(pFile, line,'\0'))
				{
					
					if (values > ssf.files - 1)
					{
						break;
					}
					std::stringstream ss(line);
					ss >> dataValues[values];
					values++;
				}
			}
				
			if (isbig == true) {
				padsize = 2048 - (sizeof(SSFHeader) + (sizeof(SSFEntry) * ssf.files));
				char*  pad = new char[padsize];
				pFile.read((char*)&pad, sizeof(pad));
				delete[] pad;
			}




			//get files
			for (int i = 0; i < ssf.files; i++)
			{

				std::string output = argv[2];
				output.erase(output.length() - 3.3);
				output.insert(output.length(), "\\");
				std::experimental::filesystem::create_directory(output);
				if (isbig == false)
					output.insert(output.length(), dataValues[i]);
				else output.insert(output.length(), std::to_string(i) + ".dat");


				std::ofstream oFile(output, std::ofstream::binary);
				pFile.seekg(ssfe[i].offset, pFile.beg);
				std::cout << "Extracting: " << output << " Size: " << ssfe[i].size << std::endl;
				auto dataSize = ssfe[i].size;
				auto dataBuff = std::make_unique<char[]>(dataSize);
				pFile.read(dataBuff.get(), dataSize);
				oFile.write(dataBuff.get(), dataSize);
			}


			std::ofstream pTable,oOrder;
			std::string table = argv[2];
			table.erase(table.length() - 4, 4);
			if (isbig == false)
			{
				std::string nameorder = table;
				nameorder.insert(nameorder.length(), "_order.txt");
				std::ofstream oOrder(nameorder, std::ofstream::trunc | std::ofstream::out);


				for (int i = 0; i < ssf.files; i++)
				{
					oOrder << dataValues[i] << std::endl;
				}
			}

			table.insert(table.length(), ".txt");

			pTable.open(table, std::ofstream::trunc | std::ofstream::out);
			pTable << ssf.files << std::endl;
			pTable << ssf.val1 << std::endl;
			pTable << ssf.val2 << std::endl;
			pTable << ssf.val3 << std::endl;
			pTable << ssf.val4 << std::endl;
			

			for (int i = 0; i < ssf.files; i++)
			{
				pTable << ssfe[i].type << std::endl;
			}
			if (isbig == false)
			{
				for (int i = 0; i < values; i++)
				{
					oOrder << dataValues[i] << std::endl;
				}
			}
			pTable.close();
		}
		std::cout << "Finished." << std::endl;
	}
	if (strcmp(argv[1], "create") == 0)
	{
		if (argv[3] == 0 || argv[4] == 0)
		{
			std::cout << "ERROR: Not enough arguments!" << std::endl;
			return 1;
		}

		if (!(strcmp(argv[4], "small") == 0 || strcmp(argv[4], "big") == 0))
		{
			std::cout << "ERROR: Invalid value for <pad>!" << std::endl;
			return 1;
		}
		
		bool isbig;
		std::ifstream pFile(argv[3], std::ifstream::binary);

		if (!pFile)
		{
			std::cout << "ERROR: Coult not open " << argv[3] << "!" << std::endl;
			return 1;
		}

		if ((strcmp(argv[4], "small") == 0)) isbig = false;
		if ((strcmp(argv[4], "big") == 0)) isbig = true;
		std::ifstream pOrder;

		std::ifstream tNames(argv[3], std::ifstream::binary);
		if (isbig == false)
		{
			std::string order = argv[3];
			order.insert(order.length() - 4, "_order");
			pOrder.open(order, std::ifstream::binary);
			if (!pOrder)
			{
				std::cout << "ERROR: Could not open order file: " << order << "!" << std::endl;
				return 1;
			}
		}

		if (pFile)
		{

			
			int filesize = sizeof(SSFHeader);
			int headersize = sizeof(SSFHeader);
			int files;
			int val[4];

			std::string line;

			// get file amount
			std::getline(pFile, line);
			std::stringstream ss(line);
			ss >> files;


			auto filetypes = std::make_unique<int[]>(files + 4);
			auto sizes = std::make_unique<int[]>(files);
			auto adjsizes = std::make_unique<int[]>(files);

			auto dataValues = std::make_unique<std::string[]>(files);
			auto dataValuesSize = 0;
			auto names = std::make_unique<std::string[]>(files);
			int i = 0;
			while (std::getline(pFile, line))
			{
				std::stringstream ss(line);
				ss >> filetypes[i];
				i++;
			}
			i = 0;
			while (std::getline(pOrder, line))
			{
				std::stringstream ss(line);
				ss >> dataValues[i];
				headersize += dataValues[i].length() + 1;
				dataValuesSize += dataValues[i].length() + 1;
				i++;
			}

			// skip pad
			int padsize;
			if (isbig == false) padsize = 0;
			else padsize = 2048 - (sizeof(SSFHeader) + (sizeof(SSFEntry) * files));


			// get file sizes

			for (int i = 0; i < files; i++)
			{
				std::string input;
				input = std::to_string(i) + ".dat";
				if (isbig == false)
					input = dataValues[i];

				std::string folder = argv[2];
				input.insert(0, argv[2]);
				input.insert(folder.length(), "\\");
				names[i] = input;
				std::ifstream pInput(input, std::ifstream::binary);

				// get data
				if (pInput)
				{
					sizes[i] = (int)getSizeToEnd(pInput);
					int buffedsize = 0;
					if (isbig == true)
					{
						if (!(sizes[i] % 2048 == 0))
						{
							do
							{
								buffedsize++;
								char pad = 0;
								sizes[i]++;
							} while (sizes[i] % 2048 != 0);

						}
						adjsizes[i]  = sizes[i];
						sizes[i] -= buffedsize;
						filesize += adjsizes[i];

					}
					if (isbig == false)

						filesize += sizes[i];

					
				}
			}
		
			

			filesize += padsize;
			if (isbig == false) filesize += 484;
			if (isbig == true) filesize += 320;
		   // create ssf header
			SSFHeader ssf;
			ssf.header = 'SEC ';
			ssf.files = files;
			ssf.filesize = filesize;
			ssf.val1 = filetypes[0];
			ssf.val2 = filetypes[1];
			ssf.val3 = filetypes[2];
			ssf.val4 = filetypes[3];
			std::string fileout = argv[3];
			fileout.replace(fileout.length() - 4, 4, ".ssf");
			std::ofstream oArchive(fileout, std::ofstream::binary | std::ofstream::out);

			oArchive.write((char*)&ssf, sizeof(SSFHeader));
			
			int baseoffset = sizeof(SSFEntry) * files + dataValuesSize + sizeof(SSFHeader) + padsize;

			if (!(baseoffset % 512 == 0))
			{
				do
				{
					baseoffset++;
				} while (baseoffset % 512 != 0);

			}
			if (isbig == true) baseoffset = 2048;
			// create fileentries
			int currentpad = 0;
			for (int i = 0; i < files; i++)
			{
				SSFEntry ent;
				ent.offset = baseoffset;
				ent.pad = 0x7C801A4F;

				if (isbig == false)
				{
					ent.pad = currentpad;
					baseoffset += sizes[i];
				}
				else baseoffset += adjsizes[i];
				ent.size = sizes[i];
				ent.type = filetypes[i + 4];
				oArchive.write((char*)&ent, sizeof(SSFEntry));
				currentpad += dataValues[i].length() + 1;
				headersize += sizeof(SSFEntry);
			}

			for (int i = 0; i < files; i++)
			{
				oArchive.write(dataValues[i].c_str(), dataValues[i].length() + 1);
			}

			 oArchive.seekp(headersize, oArchive.beg);

			if (isbig == false)
			{
				int size = headersize;
					do
					{

						char pad = 0xFF;
						oArchive.write((char*)&pad, sizeof(pad));
						size++;
					} while (size % 512 != 0);
			}



			if (isbig == true)
			{
				for (int i = 0; i < padsize; i++)
				{
					char pad = 0;
					oArchive.write((char*)&pad, sizeof(char));
				}
			}


			for (int i = 0; i < files; i++)
			{
				std::ifstream pArcFile(names[i], std::ifstream::binary);

				// write data
				if (pArcFile)
				{
					auto dataSize = getSizeToEnd(pArcFile);
					int size = (int)dataSize;
					auto dataBuff = std::make_unique<char[]>(dataSize);
					pArcFile.read(dataBuff.get(), dataSize);
					std::cout << "Processing: " << names[i] << std::endl;
					oArchive.write(dataBuff.get(), dataSize);
					if (isbig == true)
					{
						if (!(size % 2048 == 0))
						{
							do
							{
								char pad = 0;
								oArchive.write((char*)&pad, sizeof(char));
								size++;
							} while (size % 2048 != 0);
						}
					}
				}
           	}	
			std::cout << "Finished." << std::endl;
		}
	}
	

	

	return 0;
}