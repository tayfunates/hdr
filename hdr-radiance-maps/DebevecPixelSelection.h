/*
 * DebevecPixelSelection.h
 *
 *  Created on: Mar 23, 2016
 *      Author: tayfun
 */

#ifndef DEBEVECPIXELSELECTION_H_
#define DEBEVECPIXELSELECTION_H_

#define Z_MIN 0
#define Z_MAX 255

#include <core/core.hpp>
#include <set>

struct PixelSelectionParameters
{
	bool mShowAvoidedPixelLocations;
	bool mShowSelectedPixelLocations;
	float mSpatialCoverageConst;
	bool mGeneratePixelLocations;
	bool mWritePixelLocations;
	std::string mOutputPixelLocationsPath;
	std::string mInputPixelLocationsPath;

	PixelSelectionParameters()
	{
		mShowAvoidedPixelLocations = false;
		mShowSelectedPixelLocations = false;
		mSpatialCoverageConst = 2.0f;
		mGeneratePixelLocations = true;
		mWritePixelLocations = false;
		mOutputPixelLocationsPath = "";
		mInputPixelLocationsPath = "";
	}

	PixelSelectionParameters(const bool& pShowAvoidedPL,
			const bool& pShowSelectedPL,
			const float& pSpatialCoverageConst,
			const bool& pGeneratePL,
			const bool& pWritePL,
			const std::string& pOutputPLPath,
			const std::string& pInputPLPath)
	{
		mShowAvoidedPixelLocations = pShowAvoidedPL;
		mShowSelectedPixelLocations = pShowSelectedPL;
		mSpatialCoverageConst = pSpatialCoverageConst;
		mGeneratePixelLocations = pGeneratePL;
		mWritePixelLocations = pWritePL;
		mOutputPixelLocationsPath = pOutputPLPath;
		mInputPixelLocationsPath = pInputPLPath;
	}


};


class DebevecPixelSelection {
public:
	//inits the pixel selection
	static bool init(const PixelSelectionParameters& pSelectionParameters);
	//Reads or finds pixel locations that will be used by the algorithm
	static bool getPixels(const std::vector<cv::Mat>& pInputImages, std::set<int>& pPixelIndexes);
private:
	static int getMinRequiredPixelCount(const int& pNumberOfImages);

	static void getAvoidedPixelLocations(const std::vector<cv::Mat>& pInputImages, const cv::Mat& pMiddleImage, std::set<int>& pAvoidedPixelLocations);
	static void generatePixelLocations(const std::vector<cv::Mat>& pInputImages, std::set<int>& pPixelIndexes);
	static void showPixelLocationsOnTheImage(const cv::Mat& pImage, std::set<int>& pPixelIndexes);
	static bool writePixelLocations(const std::set<int>& pPixelIndexes);
	static bool readPixelLocations(std::set<int>& pPixelIndexes);

	static PixelSelectionParameters mSelectionParameters;
};

#endif /* DEBEVECPIXELSELECTION_H_ */
