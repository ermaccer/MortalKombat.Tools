// img2bmp.cpp : Defines the entry point for the console application.
//
#include <iostream>
#include <fstream>
#include <iostream>
#include <memory>
#include "stdafx.h"

struct BMPInfoHeader {
	int   biSize;
	int    biWidth;
	int    biHeight;
	short    biPlanes;
	short    biBitCount;
	int   biCompression;
	int   biSizeImage;
	int    biXPelsPerMeter;
	int    biYPelsPerMeter;
	int   biClrUsed;
	int   biClrImportant;
};

#pragma pack(push,1)
struct BMPHeader {
	short  bfType;
	int  bfSize;
	short  bfReserved1;
	short  bfReserved2;
	int bfOffBits;
};
#pragma pack(pop)



struct IMGHeader {
	int    header; //0.1v
	int    pad1;
	int    check; // always 1
	int    pad2[5];
	short  x;
	short  y;
	int    pad3[7];
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
		std::cout << "Usage img2bmp <input>" << std::endl;
		return 1;
	}

	std::ifstream pFile(argv[1], std::ifstream::binary);

	if (!pFile)
	{
		std::cout << "ERROR: Could not open " << argv[1] << "!" << std::endl;
		return 1;
	}


	if (pFile)
	{
		IMGHeader img;
		pFile.read((char*)&img, sizeof(IMGHeader));
		

		if (img.header != 'v1.0')
		{
			std::cout << "ERROR: " << argv[1] << " is not a MK4 Image file." << std::endl;
			return 1;
		}

		
		// create bmp
		BMPHeader bmp;
		BMPInfoHeader bmpf;
		bmp.bfType = 'MB';
		bmp.bfSize = (int)getSizeToEnd(pFile);
		bmp.bfReserved1 = 0;
		bmp.bfReserved2 = 0;
		bmp.bfOffBits = sizeof(BMPHeader) + sizeof(BMPInfoHeader);
		bmpf.biSize = sizeof(BMPInfoHeader);
		bmpf.biWidth = (int)img.x;
		bmpf.biHeight = (int)img.y;
		bmpf.biPlanes = 1;
		bmpf.biBitCount = 16;
		bmpf.biCompression = 0;
		bmpf.biXPelsPerMeter = 2835;
		bmpf.biYPelsPerMeter = 2835;
		bmpf.biClrUsed = 0;
		bmpf.biClrImportant = 0;
	
		// set after header to get bmp data
		pFile.seekg(sizeof(IMGHeader), pFile.beg);

		// create output
		std::string output = argv[1];
		int length = output.length();
		output.replace(length - 4, 4, ".bmp");
		std::ofstream oFile(output, std::ofstream::binary);

		// write header
		oFile.write((char*)&bmp, sizeof(BMPHeader));
		oFile.write((char*)&bmpf, sizeof(BMPInfoHeader));
		auto dataSize = getSizeToEnd(pFile);
		auto dataBuff = std::make_unique<char[]>((long)dataSize);
		pFile.read(dataBuff.get(), dataSize);
		oFile.write(dataBuff.get(), dataSize);
	}
	return 0;
}