#pragma once



std::streampos getSizeToEnd(std::ifstream& is)
{
	auto currentPosition = is.tellg();
	is.seekg(0, is.end);
	auto length = is.tellg() - currentPosition;
	is.seekg(currentPosition, is.beg);
	return length;
}

bool checkSlash(std::string& str, bool first = false)
{
	int fnd = str.find_last_of("/\\");
	if (first == true)
		fnd = str.find_first_of("/\\");

	if (fnd == std::string::npos)
		return false;
	else
		return true;
}

std::string getWideStr(std::ifstream &file, bool f = false)
{
	auto buff = std::make_unique<char[]>(_MAX_PATH);
	int strlen = 0;
	for (int i = 0; i < _MAX_PATH; i++)
	{
		char temp[2];
		file.read((char*)&temp, sizeof(temp));
		buff[i] = temp[0];
		if (!(temp[0] == 0 && temp[1] == 0))
			strlen++;
		else
			break;
		
	}
	if (f == true) strlen -= 2;
	return std::string(buff.get(), strlen);
}

std::string convertWide(std::string& str)
{
	int strlen = str.length();
	std::string temp;
	for (int i = 0; i < strlen; i++)
	{
		temp += str[i];
		i++;
	}
	return temp;
}

std::string splitString(std::string& str, bool file) {

	std::string str_ret;
	int fnd = str.find_last_of("/\\");

	if (file == true) str_ret = str.substr(fnd + 1);
	if (file == false) str_ret = str.substr(0, fnd);
	return str_ret;

}

int calcOffsetFromPad(int val, int padsize)
{
	int retval = val;
	if (!(retval % padsize == 0))
	{
		do
		{
			retval++;
		} while (retval % padsize != 0);

	}
	return retval;
}