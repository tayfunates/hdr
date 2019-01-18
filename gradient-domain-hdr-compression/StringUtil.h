/*
 * StringUtil.h
 *
 *  Created on: Mar 22, 2016
 *      Author: tayfun
 */

#ifndef STRINGUTIL_H_
#define STRINGUTIL_H_

#include <string>
#include <iomanip>
#include <sstream>
#include <vector>

class StringUtil {
public:
	template<typename T>
	static std::string toStr(const T& val) {
		std::stringstream ss;
		ss << val;
		return ss.str();
	}

	template<typename T>
	static std::string toStr(const T& val, const int& width, const int& precision) {
		std::stringstream ss;
		ss << std::fixed << std::setw(width) << std::setprecision(precision) << val;
		return ss.str();
	}

	static std::string toStrFillZero(const int& val, const int& width);


	static size_t split(const std::string &s, char delim, std::vector<std::string> &elems);


	static std::string trim(const std::string& str);

	static std::string toLower(std::string str, const std::string& localeString = "");

	static std::string betweenTwoDelimeters(const std::string& str, const char& firstDelim, const char& lastDelim);

};

#endif /* STRINGUTIL_H_ */
