#pragma once
#include <iostream>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include "stdafx.h"
#include <filesystem>
#include <sstream>



#define MKU_FILES              167


std::string GetString(std::ifstream& file)
{
	short strlen;
	file.read((char*)&strlen, sizeof(short));
	auto buff = std::make_unique<char[]>(strlen);
	file.read(buff.get(), strlen);
	return std::string(buff.get(), strlen);
}



std::streampos getSizeToEnd(std::ifstream& is)
{
	auto currentPosition = is.tellg();
	is.seekg(0, is.end);
	auto length = is.tellg() - currentPosition;
	is.seekg(currentPosition, is.beg);
	return length;
}





struct BINHeader {
	short unk1; // 64
	short unk2; // 2

};

struct IMPHeader {
	int header; // iMp0
	int check; // -1
};
