/*
 * main.cpp
 *
 *  Created on: May 11, 2016
 *      Author: tayfun
 */

#include "FattalToneMapper.h"
#include <iostream>

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		std::cerr << "Error: | Application requires a parameter file!" << std::endl;
		return (-1);
	}
	const std::string paramFile(argv[1]);

	FattalToneMapper toneMapper;
	bool initialized = toneMapper.init(paramFile);
	if (!initialized)
	{
		return (-1);
	}
	toneMapper.run();
	return (0);
}




