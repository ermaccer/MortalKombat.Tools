// toceditor.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include "filef.h"


struct mk_toc_header_end {
	int unk[2]; // always 0?
};


struct mk_toc_entry {
	int  namePointer;
	int  previousSize;
	int  size;
};

struct toc_file {
	int header;
	int offset; 
	int entries;
};

enum eModes {
	MODE_DUMP = 1,
	MODE_INJECT,
};


int main(int argc, char* argv[])
{
	if (argc == 1)
	std::cout << "MK ToC Editor - edit file tables in 3D era Mortal Kombat games\n"
		"Usage: toceditor <params> <file>\n"
		"Params: \n"
		" -d   Dumps specified TOC block from executable\n"
		" -i   Inject previously dumped TOC block into executable\n"
		" -n   Specifies how many entries should be dumped\n"
		" -f   Specifies file to inject\n"
		" -p   Specifies position (offset), hex format only\n";


	int mode = 0;
	int entries = 0;
	int offset = 0;
	std::string f_param;

	// params
	for (int i = 1; i < argc - 1; i++)
	{
		if (argv[i][0] != '-' || strlen(argv[i]) != 2) {
			return 1;
		}
		switch (argv[i][1])
		{
		case 'd': mode = MODE_DUMP;
			break;
		case 'i': mode = MODE_INJECT;
			break;
		case 'p':
			i++;
			sscanf_s(argv[i], "%x", &offset);
			break;
		case 'n':
			i++;
			sscanf_s(argv[i], "%d", &entries);
			break;
		case 'f':
			i++;
			f_param = argv[i];
			break;
		default:
			std::cout << "ERROR: Param does not exist: " << argv[i] << std::endl;
			return 1;
			break;
		}
	}

	if (mode == MODE_DUMP)
	{
		std::ifstream pFile(argv[argc - 1], std::ifstream::binary);

		if (!pFile)
		{
			std::cout << "ERROR: Could not open " << argv[argc - 1] << "!" << std::endl;
			return 1;
		}

		if (offset == 0)
		{
			std::cout << "ERROR: Position not specified!" << std::endl;
			return 1;
		}

		if (!entries)
		{
			std::cout << "ERROR: Invalid number of entries specified!" << std::endl;
			return 1;
		}

		pFile.seekg(offset, pFile.beg);

		std::unique_ptr<mk_toc_entry[]> tocEntries = std::make_unique<mk_toc_entry[]>(entries);
		for (int i = 0; i < entries; i++)
			pFile.read((char*)&tocEntries[i], sizeof(mk_toc_entry));

		std::ofstream dump("toc.bin", std::ofstream::binary);
		toc_file header;
		header.header = 'DCOT';
		header.entries = entries;
		header.offset = offset;

		dump.write((char*)&header, sizeof(toc_file));

		for (int i = 0; i < entries; i++)
			dump.write((char*)&tocEntries[i], sizeof(mk_toc_entry));

		std::cout << "Saved as toc.bin." << std::endl;
		
	}
	if (mode == MODE_INJECT)
	{
		std::ofstream oFile(argv[argc - 1], std::ofstream::binary | std::ofstream::in | std::ofstream::out);

		if (!oFile)
		{
			std::cout << "ERROR: Could not open " << argv[argc - 1] << "for writing!" << std::endl;
			return 1;
		}

		if (f_param.empty())
		{
			std::cout << "ERROR: Input file not specified!" << std::endl;
			return 1;
		}

		std::ifstream pFile(f_param, std::ifstream::binary);

		if (!pFile)
		{
			std::cout << "ERROR: Could not open " << f_param.c_str() << "!" << std::endl;
			return 1;
		}
		toc_file header;
		pFile.read((char*)&header, sizeof(toc_file));

		if (!(header.header == (int)'DCOT'))
		{
			std::cout << "ERROR: Input file is not a valid ToC dump!" << std::endl;
			return 1;
		}

		std::unique_ptr<mk_toc_entry[]> tocEntries = std::make_unique<mk_toc_entry[]>(header.entries);
		for (int i = 0; i < header.entries; i++)
			pFile.read((char*)&tocEntries[i], sizeof(mk_toc_entry));

		
		int baseSize = 2048;

		for (int i = 0; i < header.entries; i++)
		{
			tocEntries[i].previousSize = baseSize;
			baseSize += calcOffsetFromPad(tocEntries[i].size, 2048);

		}

		oFile.seekp(header.offset, oFile.beg);
		for (int i = 0; i < header.entries; i++)
			oFile.write((char*)&tocEntries[i], sizeof(mk_toc_entry));

		std::cout << "Finished." << std::endl;
	}

}
