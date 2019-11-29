// sndtool.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"
#include <iostream>
#include <filesystem>
#include <memory>
#include <fstream>
#include <string>
#include <sstream>
#include "filef.h"

struct sound_header {
	int      sounds;
};


enum eModes {
	MODE_EXTRACT  = 1,
	MODE_CREATE
};

int main(int argc, char* argv[])
{
	if (argc == 1) {
		std::cout << "Usage: sndtool <params> <input>\n"
			"Params: \n"
			" -e   Extracts a file from input \n"
			" -c   Creates a file from input folder\n"
			" -l   Specifies list file\n";
			" -o   Specifies output folder or file\n";
		return 1;
	}

	int mode = 0;
	std::string o_param;
	std::string l_param;
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
		case 'o':
			i++;
			o_param = argv[i];
			break;
		case 'l':
			i++;
			l_param = argv[i];
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
			sound_header snd;
			pFile.read((char*)&snd, sizeof(sound_header));

			std::unique_ptr<int[]> soundOffsets = std::make_unique<int[]>(snd.sounds);

			for (int i = 0; i < snd.sounds; i++)
				pFile.read((char*)&soundOffsets[i], sizeof(int));
			

			std::unique_ptr<int[]> sizes = std::make_unique<int[]>(snd.sounds);

			for (int i = 0; i < snd.sounds; i++)
				sizes[i] = soundOffsets[i + 1] - soundOffsets[i];

			sizes[snd.sounds - 1] = (int)getSizeToEnd(pFile) - soundOffsets[snd.sounds - 2];

			if (!l_param.empty())
			{
				std::ofstream list(l_param, std::ofstream::binary);
				for (int i = 0; i < snd.sounds; i++)
				{
					std::string str;
					if (!o_param.empty())
					{
						str = o_param;
						str += "\\";
					}
					str += "sound_" + std::to_string(i);
					str += ".wav";
					list << str << std::endl;
				}
			}



			if (!o_param.empty())
			{
				if (!std::experimental::filesystem::exists(o_param))
					std::experimental::filesystem::create_directory(o_param);

				std::experimental::filesystem::current_path(
					std::experimental::filesystem::system_complete(std::experimental::filesystem::path(o_param)));
			}


			for (int i = 0; i < snd.sounds; i++)
			{
				std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(sizes[i]);
				pFile.read(dataBuff.get(), sizes[i]);

				std::string name = "sound_" + std::to_string(i);
				name += ".wav";
				std::cout << "Processing: " << name  << std::endl;

				std::ofstream oFile(name, std::ofstream::binary);
				oFile.write(dataBuff.get(), sizes[i]);
			}

		}
		std::cout << "Finished." << std::endl;
	}
	if (mode == MODE_CREATE)
	{

		if (l_param.empty())
		{
			std::cout << "ERROR: No list file was specified!" << std::endl;
			return 1;
		}

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
			// get files number
			for (const auto & file : std::experimental::filesystem::recursive_directory_iterator(folder))
			{
				filesFound++;
				if (std::experimental::filesystem::is_directory(file)) foldersFound++;

			}
			filesFound -= foldersFound;

			// get files info
			int sounds = 0;
			std::unique_ptr<unsigned int[]> size = std::make_unique<unsigned int[]>(filesFound);
			std::unique_ptr<std::string[]> path = std::make_unique<std::string[]>(filesFound);

			std::string line;


			if (!l_param.empty())
			{
				std::ifstream pList(l_param, std::ifstream::binary);
				if (!pList)
				{
					std::cout << "ERROR: Could not open list file: " << l_param << "!" << std::endl;
					return 1;
				}
				while (std::getline(pList, line))
				{
					std::stringstream ss(line);
					ss >> path[sounds];
					std::ifstream pFile(path[sounds]);
					if (pFile)
						size[sounds] = (unsigned int)getSizeToEnd(pFile);
					sounds++;
				}
			}


			std::string output;
			if (o_param.empty())
				output = "new.snd";
			else
				output = o_param;
			
			std::ofstream oFile(output, std::ofstream::binary);
			oFile.write((char*)&sounds, sizeof(int));

			int baseOffset = (sounds + 1) * sizeof(int);
			for (int i = 0; i < sounds; i++)
			{
				oFile.write((char*)&baseOffset, sizeof(int));
				baseOffset += size[i];
			}
			for (int i = 0; i < sounds; i++)
			{
				std::ifstream snd(path[i], std::ofstream::binary);
				std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(size[i]);
				snd.read(dataBuff.get(), size[i]);
				std::cout << "Processing: " << splitString(path[i],true) << std::endl;
				oFile.write(dataBuff.get(), size[i]);
			}
			
			std::cout << "Finished." << std::endl;
		}
	}

    return 0;
}

