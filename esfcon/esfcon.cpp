// esfcon.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <memory>
#include <windows.h>
#include <string>




struct WAVHeader{
	int        header; // RIFF
	int        filesize;
	int        waveheader; // WAVE
	int        format; // FMT
	int        sectionsize;
	short      waveformat;
	short      channels;
	int        samplespersecond;
	int        bytespersecond;
	short      blockalign;
	short      bitspersample;
	int        dataheader;
	int        datasize;

};

struct ESFHeader {
	char         header[4]; // ESF0x6
	int         datasize; // wave data
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

	if (argc == 1) {
		std::cout << "Usage: esfcon <input> <f>" << std::endl;
		return 1;
	}

	std::ifstream pFile(argv[1], std::ifstream::binary);

	if (!pFile)
	{
		std::cout << "ERROR: Could not open " << argv[1] << "!" << std::endl;
		return 1;
	}

	ESFHeader esf;
	pFile.read((char*)&esf, sizeof(ESFHeader));

	if (!(esf.header[3] == 0x6 && esf.header[2] == 'F' && esf.header[1] == 'S' && esf.header[0] == 'E')) 
	{
		std::cout << "ERROR: " << argv[1] << " is not a MK4 Sound file!" << std::endl;
		return 1;
	}

	if (pFile)
	{
		std::string output = argv[1];
		int length = output.length();
		output.replace(length - 4, 4, ".wav");
		std::ofstream oFile(output, std::ofstream::binary);

		int freq = 11025;
		if (argv[2] && (strcmp(argv[2], "f") == 0)) freq = 22050;


		// create wave header
		WAVHeader wav;
		wav.header = 'FFIR';
		wav.filesize = (int)getSizeToEnd(pFile) - ((sizeof(wav.filesize) + sizeof(wav.header))) + sizeof(WAVHeader);
		wav.waveheader = 'EVAW';
		wav.format = ' tmf';
		wav.sectionsize = 16;
		if (argv[2] && (strcmp(argv[2], "f") == 0)) wav.waveformat = 0x10; // WAVE_FORMAT_OKI_ADPCM	
		else wav.waveformat = 0x01;
		wav.channels = 1;
		wav.samplespersecond = freq;
		wav.bytespersecond = freq;
		wav.blockalign = 1;
		wav.bitspersample = 8;
		wav.dataheader = 'atad';
		wav.datasize = wav.filesize - sizeof(WAVHeader) - 8;
		// write wave header
		oFile.write((char*)&wav, sizeof(WAVHeader));

		// write file data
		auto dataSize = getSizeToEnd(pFile);
		auto dataBuff = std::make_unique<char[]>(dataSize);
		pFile.read(dataBuff.get(), dataSize);


		oFile.write(dataBuff.get(), dataSize);




	}
    return 0;
}

