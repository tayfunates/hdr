/*
 * DebevecHDRRadianceMapsImpl.h
 *
 *  Created on: Mar 22, 2016
 *      Author: tayfun
 */

#ifndef DEBEVECHDRRADIANCEMAPSIMPL_H_
#define DEBEVECHDRRADIANCEMAPSIMPL_H_

#include <core/core.hpp>
#include <string>
#include <vector>
#include "DebevecPixelSelection.h"
#include "EigenDebevecSolver.h"

enum ImageChannelType
{
	BLUE_IMG = 0,
	GREEN_IMG,
	RED_IMG,
	GREYSCALE_IMG,
	COLOR_IMG
};

class DebevecHDRRadianceMapsImpl {
public:
	DebevecHDRRadianceMapsImpl();
	virtual ~DebevecHDRRadianceMapsImpl();

	bool init(const std::string& pParamFile);
	void run();

private:
	std::vector<cv::Mat> mInputImages;
	DebevecPixelSelection mPixelSelection;
	EigenDebevecSolver mSolver;

	//Read input images into mInputImages
	bool readInputImages();

	//Parses the parameter file
	bool readParameterFile(const std::string& pParamFilePath);

	//Application parameters
	std::string mInputImageFolder;
	ImageChannelType mChannelType;
	bool mShowInputImages;
	PixelSelectionParameters mSelectionParameters;
	float mSmoothnessTerm;
	std::string mCRFValuesPath;
	std::string mSampleRadiancesPath;

	void calculateRadianceValuesForPixelsSelected(const std::vector<float>& pIrradiances, const std::set<int>& pPixelIndexes, std::vector<int>& pColorVals, std::vector<float>& pRadianceVals);

	bool writeOutputResponseCurve(const std::vector<float>& pValues);
	bool writeSampleColorRadiancePairs(const std::vector<int>& pColorValues, const std::vector<float>& pRadianceValues);
};

#endif /* DEBEVECHDRRADIANCEMAPSIMPL_H_ */
