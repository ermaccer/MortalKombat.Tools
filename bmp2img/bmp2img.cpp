// bmp2img.cpp : Defines the entry point for the console application.
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
	short  val1; // 64
	short  val2; // 4
	int    check; // always 1
	int    pad2[5] = {};
	short  x;
	short  y;
	int    pad3[7] = {};
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
		std::cout << "Usage bmp2img <input>" << std::endl;
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
		BMPHeader bmp;
		BMPInfoHeader bmpf;

		pFile.read((char*)&bmp, sizeof(BMPHeader));
		pFile.read((char*)&bmpf, sizeof(BMPInfoHeader));

		if (!(bmp.bfType == 'MB'))
		{
			std::cout << "ERROR: " << argv[1] << " is not a BMP image." << std::endl;
			return 1;
		}
		// check if xrgb 16 bit
		if (!(bmpf.biBitCount == 16 && bmpf.biCompression == 0))
		{
			std::cout << "ERROR: " << argv[1] << " is not a 16 bit XRGB BMP image." << std::endl;
			return 1;
		}

		// create img
		IMGHeader img;
		img.check = 1;
		img.header = 'v1.0';
		img.x = (short)bmpf.biWidth;
		img.y = (short)bmpf.biHeight;
		img.val1 = 64;
		img.val2 = 4;

		// seek after bmp headers

		pFile.seekg(sizeof(BMPHeader) + sizeof(BMPInfoHeader), pFile.beg);

		// create output
		std::string output = argv[1];
		int length = output.length();
		output.replace(length - 4, 4, ".img");
		std::ofstream oFile(output, std::ofstream::binary);

		// write header
		oFile.write((char*)&img, sizeof(IMGHeader));
		auto dataSize = getSizeToEnd(pFile);
		auto dataBuff = std::make_unique<char[]>((long)dataSize);
		pFile.read(dataBuff.get(), dataSize);
		oFile.write(dataBuff.get(), dataSize);

	}
}
