// archivetool.cpp : Defines the entry point for the console application.
//

#include "archive.h"


int main(int argc, char* argv[])
{

	if (argc == 1 || argc == 2) {
		std::cout << "Usage archivetool <mode> <input> <export_table> \n Modes: \n create  \n extract (outputs to whatever folder is in .bin) \n \n \n export_table \n true - creates mku_files.txt for adjusting offsets later \n  Note: this param can be ignored" << std::endl;
		return 1;
	}


	if (strcmp(argv[1], "extract") == 0 )
	{

		std::ifstream pFile(argv[2], std::ifstream::binary);

		if (!pFile)
			printf("ERROR: Could not open %s!", argv[2]);

		if (pFile)
		{
			BINHeader bin;
			pFile.read((char*)&bin, sizeof(BINHeader));
			if (!(bin.unk1 == 64 || bin.unk2 == 2))
			{
				std::cout << "ERROR: " << argv[2] << " is not a MKU archive!" << std::endl;
				return 1;
			}
				

			// extraction
			unsigned int offsets[MKU_FILES + 1];
			int name_places[MKU_FILES + 2];
			std::string names[MKU_FILES + 2];
			// adjusting to repack
			unsigned int offsets_places[MKU_FILES];

			std::string archive;


			// grab archive name
			pFile.seekg(32, pFile.beg);
			archive = GetString(pFile);

			// reset cursor
			pFile.seekg(0, pFile.beg);


			// get offsets offsets
			auto data = std::make_unique<unsigned char[]>((int)getSizeToEnd(pFile));
			int itr = 0;
			while (itr < MKU_FILES)
			{
				pFile.read((char*)data.get(), 1);
				if (data[0] == 25)
				{
					pFile.seekg(1, pFile.cur);
					pFile.read((char*)data.get(), 1);
					if (data[0] == 89)
					{
						pFile.read((char*)data.get(), 1);
						if (data[0] == 161)
						{
							offsets_places[itr] = (int)pFile.tellg();
							itr++;
						}


					}


				}
			}

			// read offsets
			for (int i = 0; i <MKU_FILES; i++)
			{
				int offset;
				pFile.seekg(offsets_places[i], pFile.beg);
				pFile.read((char*)&offset, sizeof(int));
				offsets[i] = offset;
			}




			// reset cursor 
			pFile.seekg(0, pFile.beg);
			itr = 0;

			// read name offsets
			while (itr < MKU_FILES + 2)
			{
				pFile.read((char*)data.get(), 1);
				if (data[0] == 199)
				{
					pFile.seekg(1, pFile.cur);
					pFile.read((char*)data.get(), 1);
					if (data[0] == 238)
					{
						pFile.read((char*)data.get(), 1);
						if (data[0] == 1)
						{
							name_places[itr] = (int)pFile.tellg();
							itr++;
						}


					}


				}
			}



			// get filenames
			for (int i = 0; i < MKU_FILES + 2; i++)
			{

				pFile.seekg(name_places[i], pFile.beg);
				names[i] = GetString(pFile);

			}



			// create mku_files.txt
			
			if (strcmp(argv[3], "true") == 0 && argv[2])
			{
				std::cout << "Creating mku_files.txt" << std::endl;
				std::ofstream pTable;
				pTable.open("mku_files.txt", std::ofstream::trunc | std::ofstream::out);
					for (int i = 0; i < MKU_FILES; i++)
					{
						pTable << names[i + 2] << " " << offsets_places[i] << std::endl;
					}
					pTable.close();
			}





			// extracting!
			std::ifstream pArchive(archive, std::ifstream::binary);

			if (!pArchive)
			{
				std::cout << "ERROR: Could not open " << archive << "!" << std::endl;
				return 1;
			}

			if (pArchive)
			{

				IMPHeader imp;

				pArchive.read((char*)&imp, sizeof(IMPHeader));

				if (!(imp.header == '0pMi' || imp.check == -1))
				{
					std::cout << "ERROR: " << archive << " is not a valid IMP archive!" << std::endl;
				}

				// art
				std::string output = names[1];
				std::experimental::filesystem::create_directory(output);
				offsets[MKU_FILES] = (int)getSizeToEnd(pArchive);



				//get files
				for (int i = 0; i < MKU_FILES; i++)
				{

					pArchive.seekg(offsets[i], pFile.beg);

					std::string input = names[i + 2];
					std::string folder = output;
					// enter folder
					input.insert(0, output);
					input.insert(folder.length(), "\\");

					std::ofstream oFile(input, std::ofstream::binary);
					auto dataSize = offsets[i + 1] - offsets[i];
					std::cout << "Procesing: " << input << std::endl;


					auto dataBuff = std::make_unique<char[]>(dataSize);
					pArchive.read(dataBuff.get(), dataSize);
					oFile.write(dataBuff.get(), dataSize);
				}

			}
			pArchive.close();
			pFile.close();

		}
	}
	
	if (strcmp(argv[1], "create") == 0)
	{

		std::ifstream pFile(argv[2], std::ifstream::binary);
		std::ifstream pTable("mku_files.txt", std::ifstream::binary);

		if (!pFile)
			printf("ERROR: Could not open %s!", argv[2]);

		if (!pTable)
			printf("ERROR: Could not open mku_files.txt!");

		if (pFile && pTable)
		{
			BINHeader bin;
			pFile.read((char*)&bin, sizeof(BINHeader));
			if (!(bin.unk1 == 64 || bin.unk2 == 2))
			{
				std::cout << "ERROR: " << argv[2] << " is not a MKU archive!" << std::endl;
				return 1;
			}


			unsigned int offsets[MKU_FILES];
			unsigned int new_offsets[MKU_FILES];
			std::string names[MKU_FILES];
			std::string archive,folder;

			pFile.seekg(32, pFile.beg);
			archive = GetString(pFile);
			pFile.seekg(94, pFile.beg);
			folder = GetString(pFile);
			pFile.seekg(0, pFile.beg);
			std::ofstream oArchive(archive, std::ofstream::binary);

			IMPHeader imp;
			imp.header = '0pMi';
			imp.check = -1;

			oArchive.write((char*)&imp, sizeof(IMPHeader));
			

			std::string line;
			int i = 0;
			while (std::getline(pTable, line))
			{
				std::stringstream ss(line);
				ss >> names[i] >> offsets[i];
				i++;
			}

			if (i > MKU_FILES)
			{
				std::cout << "ERROR: Illegal number of files" << std::endl;
				return 1;
			}
			// write data
			int baseOffset = sizeof(IMPHeader);
		
			for (int i = 0; i < MKU_FILES; i++)
			{
				std::string input = names[i];
				input.insert(0, folder);
				input.insert(folder.length(), "\\");
				
				std::ifstream pInput(input, std::ifstream::binary);
				if (!pInput)
				{
					std::cout << "ERROR: Failed to open " << input << "!" << std::endl;
					return 1;
				}
				// write data
				if (pInput)
				{
					new_offsets[i] = baseOffset;
					auto dataSize = getSizeToEnd(pInput);
					auto dataBuff = std::make_unique<char[]>(dataSize);
					pInput.read(dataBuff.get(), dataSize);
					std::cout << "Procesing: " << input << std::endl;
					oArchive.write(dataBuff.get(), dataSize);
					baseOffset += (int)dataSize;
					
				}


			}
			
			pFile.close();
			// adjust archive.bin, not really repacking
			std::ofstream oFile(argv[2], std::ofstream::binary | std::ofstream::in | std::ofstream::out);
			for (int i = 0; i < MKU_FILES; i++)
			{
				oFile.seekp(offsets[i], oFile.beg);
				oFile.write((char*)&new_offsets[i], sizeof(int));
			}

			// write pad on the end
			int pad[2] = {};
			oArchive.write((char*)&pad, sizeof(pad));
			std::cout << "Finished." << std::endl;
		}
	}
    return 0;
}

