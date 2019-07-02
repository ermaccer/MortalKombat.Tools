// ssfx.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <filesystem>
#include <sstream>

void changeEndINT(int *value)
{
	*value = (*value & 0x000000FFU) << 24 | (*value & 0x0000FF00U) << 8 |
		(*value & 0x00FF0000U) >> 8 | (*value & 0xFF000000U) >> 24;
}


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


struct MKOHeader {
	int     unk[2]; // script number (?) and some big number
	int     scriptNameSize;
	int     scriptStringSize; // padded to 8 like .pak
};


std::streampos getSizeToEnd(std::ifstream& is)
{
	auto currentPosition = is.tellg();
	is.seekg(0, is.end);
	auto length = is.tellg() - currentPosition;
	is.seekg(currentPosition, is.beg);
	return length;
}

enum eModes {
	MODE_EXTRACT = 1,
	MODE_CREATE,
	PAD_SMALL,
	PAD_BIG,
	PARAM_GAMECUBE,
	MODE_TEXTURE_INFO,
};


enum eTexture {
	PSP,
	PS2,
	GC
};


int main(int argc, char* argv[])
{

	if (argc == 1) {
		std::cout << "Usage: ssfx <params> <input>\n"
			"Params: \n"
			" -e   Extracts a file, creates table and order file for it\n"
			" -c   Creates a file from input folder\n"
			" -g   Allows to work with Gamecube/WII files\n"
			" -b   Switches large pad value\n"
			" -s   Switches small pad value (used for .dats/.secs)\n"
			" -t   Specifies table file\n"
			" -l   Specifies list with filenames to use\n"
			" -T   Analyzes texture file\n"
			"Examples:\n"
			"ssfx -e -b frost.ssf - Extracts main .ssf for frost\n"
			"ssfx -c -b -t frost.txt frost - Creates main .ssf for frost from a folder\n"
			"ssfx -e -b -l filelist.txt frost.ssf - Extracts main .ssf using name list\n"
			"ssfx -c -s -l filelist.txt -t table.txt folder - Creates small .ssf using name list\n"
	    	"ssfx -T RAIN_002 - Gives information (name, size, offsets) about a texture\n";
		return 1;
	}

	int mode = 0;
	int param = 0;
	int pad = 0;
	int extra = 0;
	std::string table;
	std::string list;

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
		case 'g': param = PARAM_GAMECUBE;
			break;
		case 'b': pad = PAD_BIG;
			break;
		case 's': pad = PAD_SMALL;
			break;
		case 't':
			i++;
			table = argv[i];
			break;
		case 'l':
			i++;
			list = argv[i];
			break;
		case 'T': mode = MODE_TEXTURE_INFO;
			break;
		default:
			std::cout << "ERROR: Param does not exist: " << argv[i] << std::endl;
			break;
		}
	}


	if (mode == MODE_EXTRACT)
	{
		std::ifstream pFile(argv[argc - 1], std::ifstream::binary);

		if (!(pad == PAD_BIG || pad == PAD_SMALL))
		{
			std::cout << "ERROR: Pad value was not specified!" << std::endl;
			return 1;
		}

		if (!pFile)
		{
			std::cout << "ERROR: Could not open " << argv[argc - 1] << "!" << std::endl;
			return 1;
		}
		
		if (pFile)
		{
			// read headers
			SSFHeader ssf;
			pFile.read((char*)&ssf, sizeof(ssf));
			

			if (param == PARAM_GAMECUBE)
			{
				changeEndINT(&ssf.header);
				changeEndINT(&ssf.files);
				changeEndINT(&ssf.filesize);
				changeEndINT(&ssf.val1);
				changeEndINT(&ssf.val2);
				changeEndINT(&ssf.val3);
				changeEndINT(&ssf.val4);
				if (ssf.header != 'SEC ') {
					std::cout << "ERROR: " << argv[argc - 1] << " is not a SEC type archive!" << std::endl;
					return 1;
				}

			}
			else if (ssf.header != 'SEC ')
			{
				std::cout << "ERROR: " << argv[argc - 1] << " is not a SEC type archive!" << std::endl;
				if (ssf.header == ' CES') std::cout << "INFO: Use -g switch for this file!" << std::endl;
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
				if (param == PARAM_GAMECUBE)
				{
					changeEndINT(&ssfe[i].size);
					changeEndINT(&ssfe[i].offset);
					changeEndINT(&ssfe[i].type);
					changeEndINT(&ssfe[i].pad);
				}
			}

			// skip pad
			int padsize;
			if (pad == PAD_BIG)
			{
				if (!list.empty())
				{
					std::ifstream pList(list,std::ifstream::binary);
					if (!pList)
					{
						std::cout << "ERROR: Could not open list file: " << list << "!" << std::endl;
						return 1;
					}
					int i = 0;
					while (std::getline(pList, line))
					{
						std::stringstream ss(line);
						ss >> dataValues[i];
						i++;
					}
				}
			}

			if (pad == PAD_SMALL)
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
				
			if (pad == PAD_BIG) {
				padsize = 2048 - (sizeof(SSFHeader) + (sizeof(SSFEntry) * ssf.files));
				char*  pad = new char[padsize];
				pFile.read((char*)&pad, sizeof(pad));
				delete[] pad;
			}

			//get files
			for (int i = 0; i < ssf.files; i++)
			{

				std::string output = argv[argc - 1];
				output.erase(output.length() - 3.3);
				output.insert(output.length(), "\\");
				std::experimental::filesystem::create_directory(output);
				if (pad == PAD_BIG && list.empty())
					output.insert(output.length(), std::to_string(i) + ".dat");
				if (pad == PAD_BIG && !list.empty())
				    output.insert(output.length(), dataValues[i]);
				if (pad == PAD_SMALL)
					output.insert(output.length(), dataValues[i]);



				std::ofstream oFile(output, std::ofstream::binary);
				pFile.seekg(ssfe[i].offset, pFile.beg);
				std::cout << "Extracting: " << output << " Size: " << ssfe[i].size << std::endl;
				auto dataSize = ssfe[i].size;
				auto dataBuff = std::make_unique<char[]>(dataSize);
				pFile.read(dataBuff.get(), dataSize);
				oFile.write(dataBuff.get(), dataSize);
			}


			std::ofstream pTable,oOrder;
			std::string table = argv[argc - 1];
			table.erase(table.length() - 4, 4);
			if (pad == PAD_SMALL)
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
			if (pad == PAD_SMALL)
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
	if (mode == MODE_CREATE)
	{
		if (table.empty()) {
			std::cout << "ERROR: Table File was not specified!" << std::endl;
			return 1;
		}

		std::ifstream pFile(table, std::ifstream::binary);
		if (!(pad == PAD_BIG || pad == PAD_SMALL))
		{
			std::cout << "ERROR: Pad value was not specified!" << std::endl;
			return 1;
		}

		if (!pFile)
		{
			std::cout << "ERROR: Could not open " << table << "!" << std::endl;
			return 1;
		}

		std::ifstream pOrder;

		std::ifstream tNames(table, std::ifstream::binary);
		if (pad == PAD_SMALL)
		{
			std::string order = table;
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
			if (pad == PAD_SMALL)
			{
				while (std::getline(pOrder, line))
				{
					std::stringstream ss(line);
					ss >> dataValues[i];
					headersize += dataValues[i].length() + 1;
					dataValuesSize += dataValues[i].length() + 1;
					i++;
				}
			}
			if (pad == PAD_BIG && !list.empty())
			{
				std::ifstream pList(list, std::ifstream::binary);
				if (!pList)
				{
					std::cout << "ERROR: Could not open list file: " << list << "!" << std::endl;
					return 1;
				}
				while (std::getline(pList, line))
				{
					std::stringstream ss(line);
					ss >> dataValues[i];
					i++;
				}
			}


			// skip pad
			int padsize;
			if (pad == PAD_SMALL) padsize = 0;
			else padsize = 2048 - (sizeof(SSFHeader) + (sizeof(SSFEntry) * files));


			// get file sizes

			for (int i = 0; i < files; i++)
			{
				std::string input;
				if (pad == PAD_BIG && !list.empty())
					input = dataValues[i];
				if (pad == PAD_BIG && list.empty())
				input = std::to_string(i) + ".dat";
				if (pad == PAD_SMALL)
					input = dataValues[i];

				std::string folder = argv[argc - 1];
				input.insert(0, argv[argc - 1]);
				input.insert(folder.length(), "\\");
				names[i] = input;
				std::ifstream pInput(input, std::ifstream::binary);

				// get data
				if (pInput)
				{
					sizes[i] = (int)getSizeToEnd(pInput);
					int buffedsize = 0;
					if (pad == PAD_BIG)
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
					if (pad == PAD_SMALL)

						filesize += sizes[i];

					
				}
			}
		
			

			filesize += padsize;
			if (pad == PAD_SMALL) filesize += 484;
			if (pad == PAD_BIG) filesize += 320;
		   // create ssf header
			SSFHeader ssf;
			ssf.header = 'SEC ';
			ssf.files = files;
			ssf.filesize = filesize;
			ssf.val1 = filetypes[0];
			ssf.val2 = filetypes[1];
			ssf.val3 = filetypes[2];
			ssf.val4 = filetypes[3];
			std::string fileout = table;
			fileout.replace(fileout.length() - 4, 4, ".ssf");
			std::ofstream oArchive(fileout, std::ofstream::binary | std::ofstream::out);

			if (param == PARAM_GAMECUBE)
			{
				changeEndINT(&ssf.header);
				changeEndINT(&ssf.files);
				changeEndINT(&ssf.filesize);
				changeEndINT(&ssf.val1);
				changeEndINT(&ssf.val2);
				changeEndINT(&ssf.val3);
				changeEndINT(&ssf.val4);
			}

			oArchive.write((char*)&ssf, sizeof(SSFHeader));
			
			int baseoffset = sizeof(SSFEntry) * files + dataValuesSize + sizeof(SSFHeader) + padsize;

			if (!(baseoffset % 512 == 0))
			{
				do
				{
					baseoffset++;
				} while (baseoffset % 512 != 0);

			}
			if (pad == PAD_BIG) baseoffset = 2048;
			// create fileentries
			int currentpad = 0;
			for (int i = 0; i < files; i++)
			{
				SSFEntry ent;
				ent.offset = baseoffset;
				ent.pad = 0x7C801A4F;

				if (pad == PAD_SMALL)
				{
					ent.pad = currentpad;
					baseoffset += sizes[i];
				}
				else baseoffset += adjsizes[i];
				ent.size = sizes[i];
				ent.type = filetypes[i + 4];
				if (param == PARAM_GAMECUBE)
				{
					changeEndINT(&ent.size);
					changeEndINT(&ent.offset);
					changeEndINT(&ent.type);
					changeEndINT(&ent.pad);
				}
				oArchive.write((char*)&ent, sizeof(SSFEntry));
				currentpad += dataValues[i].length() + 1;
				headersize += sizeof(SSFEntry);
			}

			for (int i = 0; i < files; i++)
			{
				oArchive.write(dataValues[i].c_str(), dataValues[i].length() + 1);
			}

			 oArchive.seekp(headersize, oArchive.beg);

			if (pad == PAD_SMALL)
			{
				int size = headersize;
					do
					{

						char pad = 0xFF;
						oArchive.write((char*)&pad, sizeof(pad));
						size++;
					} while (size % 512 != 0);
			}



			if (pad == PAD_BIG)
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
					if (pad == PAD_BIG)
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
	if (mode == MODE_TEXTURE_INFO)
	{
		std::ifstream pFile(argv[argc - 1], std::ifstream::binary);

		if (!pFile)
		{
			std::cout << "ERROR: Could not open " << argv[argc - 1] << "!" << std::endl;
			return 1;
		}

		if (pFile)
		{

			int texture = PSP;
			// get texture name
			char strlen;
			pFile.read((char*)&strlen, sizeof(char));
			// most likely ps2
			if (strlen == 0)
			{
				pFile.seekg(sizeof(int), pFile.beg);
				pFile.read((char*)&strlen, sizeof(char));
				texture = PS2;
			}
			auto buff = std::make_unique<char[]>(strlen);
			pFile.read(buff.get(), strlen);
			std::string name(buff.get(), strlen);

			// get texture size;
			int x, y;

			pFile.read((char*)&x, sizeof(int));
			pFile.read((char*)&y, sizeof(int));

			if (param == PARAM_GAMECUBE)
			{
				changeEndINT(&x);
				changeEndINT(&y);
				texture = GC;
			}
			
			if (x > 4096 || y > 4096)
			{
				std::cout << "ERROR: Invalid texture size. Add or remove -g switch." << std::endl;
				return 1;
			}



			std::cout << "Texture Information\n"
				"Name: " << name << std::endl <<
			"X: " << x << " Y: " << y << std::endl;

			if (texture == PSP)
			{
				std::cout << "Platform: PSP" << std::endl;
				std::cout << "Possible offsets\n"
					// header size
					<< "Palette: " << 0xD0 << std::endl
					// header size + pal size
					<< "Graphics: " << 0xD0 + 1024 << std::endl;
			}
			if (texture == PS2)
			{
				std::cout << "Platform: PS2" << std::endl;
				std::cout << "Possible offsets\n"
					// header + image data + some pad?
					<< "Palette: " << 0x250 + x * y  + 128<< std::endl
					// header size
					<< "Graphics: " << 0x250 << std::endl;
			}
			if (texture == GC)
				std::cout << "Platform: GC/WII" << std::endl;
		   
		}
	}

	

	return 0;
}