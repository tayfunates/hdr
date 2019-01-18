/*
 * StringUtil.cpp
 *
 *  Created on: Mar 22, 2016
 *      Author: tayfun
 */

#include "StringUtil.h"

std::string StringUtil::toStrFillZero(const int& val, const int& width)
{
	std::stringstream ss;
	ss << std::setfill('0') << std::setw(8) << val;
	return ss.str();
}


size_t StringUtil::split(const std::string &s, char delim, std::vector<std::string> &elems)
{
	if (s.empty()) return 0;

	std::stringstream ss(s);
	std::string item;
	size_t count = 0;
	while (std::getline(ss, item, delim))
	{
		elems.push_back(item);
		count++;
	}

	// split("a:",':') --> "a",""
	if (*s.rbegin() == delim)
	{
		elems.push_back("");
		count++;
	}
	return count;
}

std::string StringUtil::betweenTwoDelimeters(const std::string& str, const char& firstDelim, const char& lastDelim)
{
	unsigned last = str.find_last_of(lastDelim);
	unsigned first = str.find_last_of(firstDelim);
	std::string newString = str.substr(first+1, last-first-1);
	return newString;

}

std::string StringUtil::trim(const std::string& str)
{
	size_t startpos = str.find_first_not_of(" \t\n\r"); // Find the first character position after excluding leading blank spaces
	size_t endpos = str.find_last_not_of(" \t\n\r"); // Find the first character position from reverse af

	// if all spaces or empty return an empty string
	if ((std::string::npos == startpos) || (std::string::npos == endpos))
		return "";
	else
		return str.substr(startpos, endpos - startpos + 1);
}

std::string StringUtil::toLower(std::string str, const std::string& localeString)
{
	if (str.empty()) return "";
	std::locale loc;
	try
	{
		loc = std::locale(localeString.c_str());
	}
	catch (...) // catch (const std::runtime_error &err)
	{
		return "";
	}
	for (std::string::iterator it = str.begin(); it != str.end(); ++it)
		*it = std::use_facet<std::ctype<char> >(loc).tolower(*it);
	return str;
}
