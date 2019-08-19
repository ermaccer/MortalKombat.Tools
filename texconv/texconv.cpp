// texconv.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <string>


#include "image.h"
#include "filef.h"

enum eModes {
	MODE_EXTRACT = 1,
	PLATFORM_PSP,
	PLATFORM_PS2,
	EXTRA_ADJUST_PAL
};



int main(int argc, char* argv[])
{

	if (argc == 1) {
		std::cout << "Usage: texconv <params> <input>\n"
			"Params: \n"
			" -e              Converts input to tga\n"
			" -o              Specifies output file, by default it reads from file if possible\n"
			" -p <platform>   Platform: PSP,PS2\n"
			" -a              Use this option if PS2 textures have wrong palette"
			"TIP: pal color 0 is the transparency color, simply change it \nto actual transparency in any image editor for best result";
		return 1;
	}

	int mode = 0;
	int param = 0;
	int extra = 0;
	int platform;
	std::string p_param;
	std::string o_param;
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
		case 'a': extra = EXTRA_ADJUST_PAL;
			break;
		case 'p':
			i++;
			p_param = argv[i];
			break;
		case 'o':
			i++;
			o_param = argv[i];
			break;
		default:
			std::cout << "ERROR: Param does not exist: " << argv[i] << std::endl;
			break;
		}
	}

	if (p_param == "PSP" || p_param == "psp") platform = PLATFORM_PSP;
	if (p_param == "PS2" || p_param == "ps2") platform = PLATFORM_PS2;

	if (mode == MODE_EXTRACT)
	{
		std::ifstream pFile(argc[argv - 1], std::ifstream::binary);

		if (!pFile)
		{
			std::cout << "ERROR: Could not open " << argc[argv - 1] << "!" << std::endl;
			return 1;
		}

		if (pFile)
		{
			if (p_param.empty())
			{
				std::cout << "ERROR: Platform missing!" << std::endl;
				return 1;
			}

			if (platform == PLATFORM_PSP)
			{
				// get image data
				char strlen;
				std::string name;
				pFile.read((char*)&strlen, sizeof(char));
				name = argv[argc - 1];

				if (strlen)
				{
					auto buff = std::make_unique<char[]>(strlen);
					pFile.read(buff.get(), strlen);
					std::string temp(buff.get(), strlen);
					name = temp;
				}



				// get texture size;
				int x, y;

				pFile.read((char*)&x, sizeof(int));
				pFile.read((char*)&y, sizeof(int));

				

				if (x > 4096 || y > 4096)
				{
					std::cout << "ERROR: Invalid texture size. Most likely Gamecube/WII file or not a texture at all." << std::endl;
					return 1;
				}


				image_data image;
				image.width = x;
				image.height = y;
				image.bits = 8;

				// get pal

				pFile.seekg(PSP_HEADER_SIZE, pFile.beg);

				std::unique_ptr<pal_psp_data[]> palBuff = std::make_unique<pal_psp_data[]>(PSP_PAL_SIZE / sizeof(pal_psp_data));

				for (int i = 0; i < PSP_PAL_SIZE/sizeof(pal_psp_data); i++)
				{
					pFile.read((char*)&palBuff[i], sizeof(pal_psp_data));
					//std::cout << (int)col.r << "_" << (int)col.g << "_" << (int)col.b <<"_" << (int)col.a << std::endl;
				}

				// get to data
				pFile.seekg(PSP_HEADER_SIZE + PSP_PAL_SIZE, pFile.beg);

				int dataSize = image.width * image.height;

				std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(dataSize);
				pFile.read(dataBuff.get(), dataSize);

				std::unique_ptr<char[]> dataOut = std::make_unique<char[]>(dataSize);

				std::string output = name + ".bmp";
				if (!o_param.empty())
					output = o_param;

				std::ofstream oFile(output, std::ofstream::binary);

				// unswizzle
			    Unswizzlers::PSP((unsigned int*)dataOut.get(), (unsigned int*)dataBuff.get(), image);

				std::cout << "Processing texture: " << output << std::endl;

				// create bmp
				bmp_header bmp;
				bmp_info_header bmpf;
				bmp.bfType = 'MB';
				bmp.bfSize = (int)getSizeToEnd(pFile);
				bmp.bfReserved1 = 0;
				bmp.bfReserved2 = 0;
				bmp.bfOffBits = sizeof(bmp_header) + sizeof(bmp_info_header) + PSP_PAL_SIZE;
				bmpf.biSize = sizeof(bmp_info_header);
				bmpf.biWidth = image.width;
				bmpf.biHeight = image.height;
				bmpf.biPlanes = 1;
				bmpf.biBitCount = 8;
				bmpf.biCompression = 0;
				bmpf.biXPelsPerMeter = 2835;
				bmpf.biYPelsPerMeter = 2835;
				bmpf.biClrUsed = 256;
				bmpf.biClrImportant = 0;

				// write headers
				oFile.write((char*)&bmp, sizeof(bmp_header));
				oFile.write((char*)&bmpf, sizeof(bmp_info_header));

				// write colors
				for (int i = 0; i < PSP_PAL_SIZE / sizeof(pal_psp_data); i++)
				{
					// swap red and blue
					oFile.write((char*)&palBuff[i].b, sizeof(char));
					oFile.write((char*)&palBuff[i].g, sizeof(char));
					oFile.write((char*)&palBuff[i].r, sizeof(char));
					oFile.write((char*)&palBuff[i].a, sizeof(char));
				}
				
				oFile.write(dataOut.get(), dataSize);
				std::cout << "Finished." << std::endl;
			}
			if (platform == PLATFORM_PS2)
			{
				// get image data
				char strlen;
				std::string name;
				pFile.read((char*)&strlen, sizeof(char));
				name = argv[argc - 1];

				if (strlen)
				{
					auto buff = std::make_unique<char[]>(strlen);
					pFile.read(buff.get(), strlen);
					std::string temp(buff.get(), strlen);
					name = temp;
				}



				// get texture size;
				int x, y;

				pFile.read((char*)&x, sizeof(int));
				pFile.read((char*)&y, sizeof(int));



				if (x > 4096 || y > 4096)
				{
					std::cout << "ERROR: Invalid texture size. Most likely Gamecube/WII file or not a texture at all." << std::endl;
					return 1;
				}


				image_data image;
				image.width = x;
				image.height = y;
				image.bits = 8;

				// get pal

				int adjust = 128;
				if (extra == EXTRA_ADJUST_PAL)
				{
					adjust = 80;
					std::cout << "INFO: Adjust option selected." << std::endl;
				}


				pFile.seekg(PS2_HEADER_SIZE + (image.height * image.width) + adjust, pFile.beg);
				printf("Pal offset: %X\n", PS2_HEADER_SIZE + (image.height * image.width) + adjust);

				std::unique_ptr<int[]> palBuff = std::make_unique<int[]>(PSP_PAL_SIZE / sizeof(pal_psp_data));

				for (int i = 0; i < PSP_PAL_SIZE / sizeof(pal_psp_data); i++)
				{
					pFile.read((char*)&palBuff[i], sizeof(int));
					//std::cout << (int)col.r << "_" << (int)col.g << "_" << (int)col.b <<"_" << (int)col.a << std::endl;
				}

				// get to data
				pFile.seekg(PS2_HEADER_SIZE, pFile.beg);

				int dataSize = image.width * image.height;

				std::unique_ptr<char[]> dataBuff = std::make_unique<char[]>(dataSize);
				pFile.read(dataBuff.get(), dataSize);

				std::unique_ptr<char[]> dataOut = std::make_unique<char[]>(dataSize);

				std::string output = name + ".bmp";
				if (!o_param.empty())
					output = o_param;

				std::ofstream oFile(output, std::ofstream::binary);


				Unswizzlers::PS2((unsigned char*)dataOut.get(), (unsigned char*)dataBuff.get(), image);

				std::cout << "Processing texture: " << output << std::endl;

				// create bmp
				bmp_header bmp;
				bmp_info_header bmpf;
				bmp.bfType = 'MB';
				bmp.bfSize = (int)getSizeToEnd(pFile);
				bmp.bfReserved1 = 0;
				bmp.bfReserved2 = 0;
				bmp.bfOffBits = sizeof(bmp_header) + sizeof(bmp_info_header) + PSP_PAL_SIZE;
				bmpf.biSize = sizeof(bmp_info_header);
				bmpf.biWidth = image.width;
				bmpf.biHeight = image.height;
				bmpf.biPlanes = 1;
				bmpf.biBitCount = 8;
				bmpf.biCompression = 0;
				bmpf.biXPelsPerMeter = 2835;
				bmpf.biYPelsPerMeter = 2835;
				bmpf.biClrUsed = 256;
				bmpf.biClrImportant = 0;

				// write headers
				oFile.write((char*)&bmp, sizeof(bmp_header));
				oFile.write((char*)&bmpf, sizeof(bmp_info_header));


				// convert colors
				for (int i = 0; i < 256; i++)
				{
					int j;
					if ((i & 0x18) == 0x10)
					{
						int tmp;
						j = i ^ 0x18;
						tmp = palBuff[i];
						palBuff[i] = palBuff[j];
						palBuff[j] = tmp;
					}
				}


				// write colors
				for (int i = 0; i < 256; i++)
				{
					// swap rgb to bgr
					char* ptr = (char*)&palBuff[i];
					char red = *(char*)(ptr);
					char blue = *(char*)(ptr + 2);
					*(char*)(ptr) = blue;
					*(char*)(ptr + 2) = red;
					oFile.write((char*)&palBuff[i], sizeof(int));

				}

				oFile.write(dataOut.get(), dataSize);
				std::cout << "Finished." << std::endl;
			}


		}
	}
    return 0;
}

