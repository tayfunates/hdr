/*
 * DebevecPixelSelection.cpp
 *
 *  Created on: Mar 23, 2016
 *      Author: tayfun
 */

#include <imgproc/imgproc.hpp>
#include <highgui/highgui.hpp>
#include <iostream>
#include "DebevecPixelSelection.h"
#include <fstream>

PixelSelectionParameters DebevecPixelSelection::mSelectionParameters;

bool DebevecPixelSelection::init(const PixelSelectionParameters& pSelectionParameters)
{
	DebevecPixelSelection::mSelectionParameters = pSelectionParameters;
	return true;
}

bool DebevecPixelSelection::getPixels(const std::vector<cv::Mat>& pInputImages, std::set<int>& pPixelIndexes)
{
	const int imageCount = (int) pInputImages.size();
	if(imageCount < 2)
	{
		std::cerr<<"Error: | Number of images is not enough!"<<std::endl;
		return false;
	}
	pPixelIndexes.clear();

	const int middleImageIndex = (imageCount+1) / 2;
	const cv::Mat& middleImage = pInputImages[middleImageIndex];

	if(!mSelectionParameters.mGeneratePixelLocations)
	{
		readPixelLocations(pPixelIndexes);
	}
	else
	{
		generatePixelLocations(pInputImages, pPixelIndexes);
	}

	if(mSelectionParameters.mWritePixelLocations && mSelectionParameters.mGeneratePixelLocations)
	{
		writePixelLocations(pPixelIndexes);
	}

	if(mSelectionParameters.mShowSelectedPixelLocations)
	{
		showPixelLocationsOnTheImage(middleImage, pPixelIndexes);
	}

	return true;
}

void DebevecPixelSelection::generatePixelLocations(const std::vector<cv::Mat>& pInputImages, std::set<int>& pPixelIndexes)
{
	const int imageCount = (int) pInputImages.size();
	const int minRequiredPixelCount = getMinRequiredPixelCount(imageCount);
	std::cout<<"Minimum Number of Pixel Locations Required: "<<minRequiredPixelCount<<std::endl;
	const int middleImageIndex = (imageCount+1) / 2;
	const cv::Mat& middleImage = pInputImages[middleImageIndex];
	const int pixelCountInImages = middleImage.cols * middleImage.rows;
	const int imagePatchCount = minRequiredPixelCount;
	const int imagePatchSize = pixelCountInImages / imagePatchCount;

	//Apply Canny edge detection and dilation to middle image to avoid high intensity variance
	//Also pixel locations must have pixel values in each image with inreasing values
	std::set<int> highVarPixelLocs;
	getAvoidedPixelLocations(pInputImages, middleImage, highVarPixelLocs);

	int seed = static_cast<int>(time(0));
	srand(seed);

	int maxFirstIteration = 10;
	int noFirstIteration = 0;
	while((int) pPixelIndexes.size() < minRequiredPixelCount)
	{
		//For each patch insert a pixel to cover the image spatially until minRequiredPixelCount is satisfied
		for(int i=0;i<imagePatchCount;i++)
		{
			bool isAvoided = true;
			bool isAlreadyAdded = true;
			int pixelIndex = -1;
			while(isAvoided || isAlreadyAdded)
			{
				pixelIndex = i*imagePatchSize+(rand() % imagePatchSize);
				isAvoided = highVarPixelLocs.find(pixelIndex) != highVarPixelLocs.end();
				isAlreadyAdded = pPixelIndexes.find(pixelIndex) != pPixelIndexes.end();
			}

			pPixelIndexes.insert(pixelIndex);

			if(static_cast<int>(pPixelIndexes.size()) > minRequiredPixelCount)
			{
				break;
			}
		}
		if(noFirstIteration == maxFirstIteration)
		{
			break;
		}
		noFirstIteration++;
	}

	std::cout<<"Initial number of selected pixel count: "<<pPixelIndexes.size()<<std::endl;

	//Spatial covering is done above, Now it is time to do the color covering
	std::set<uchar> nonCoveredPixelValueSet;
	for(uchar i=Z_MIN;i<Z_MAX;i++)
	{
		nonCoveredPixelValueSet.insert(i);
	}
	for (std::set<int>::iterator it = pPixelIndexes.begin(); it != pPixelIndexes.end(); ++it)
	{
		const int index = *it;
		const int row = index / middleImage.cols;
		const int col = index % middleImage.cols;

		for(size_t i=0;i<pInputImages.size();i++)
		{
			const cv::Mat& currImg = pInputImages[i];
			const uchar pixelVal = currImg.at<uchar>(row, col);
			const bool tobeCovered = nonCoveredPixelValueSet.find(pixelVal) != nonCoveredPixelValueSet.end();
			if(tobeCovered) nonCoveredPixelValueSet.erase(pixelVal);

		}
	}
	std::cout<<"There are "<<nonCoveredPixelValueSet.size()<<" non covered pixel values from initial selection are: ";
	for (std::set<uchar>::iterator it = nonCoveredPixelValueSet.begin(); it != nonCoveredPixelValueSet.end(); ++it)
	{
		std::cout<<(int)*it<<" ";
	} std::cout<<std::endl;


	bool terminateColorCovering = false;
	//Try to fill non covered pixel values by selecting pixel locations from all images
	const int numberOfPasses = 2;
	for(int pass=0;pass<numberOfPasses;pass++)
	{

		const int locationIncrement = (pass==0) ? pixelCountInImages / 30 : 0; //In the first pass I am trying to do color covering by applying spatial covering

		for(size_t i=0;i<pInputImages.size();i++)
		{
			const cv::Mat& currImg = pInputImages[i];

			for(int pI=0;pI<pixelCountInImages;pI++)
			{
				const int row = pI / middleImage.cols;
				const int col = pI % middleImage.cols;
				const uchar pixelVal = currImg.at<uchar>(row, col);
				const bool isANonCoveredPixelValue = nonCoveredPixelValueSet.find(pixelVal) != nonCoveredPixelValueSet.end();
				const bool isAvoided = highVarPixelLocs.find(pI) != highVarPixelLocs.end();
				const bool isAlreadyAdded = pPixelIndexes.find(pI) != pPixelIndexes.end();

				 if(isANonCoveredPixelValue && !isAvoided && !isAlreadyAdded)
				 {

					 pPixelIndexes.insert(pI);
					 nonCoveredPixelValueSet.erase(pixelVal);

					 std::cout<<"Pixel Value: "<< (int) pixelVal << " is found at pixel index: "<<pI<<std::endl;
					 pI += locationIncrement;
				 }
				 terminateColorCovering = nonCoveredPixelValueSet.empty();
				 if(terminateColorCovering) break;
			}
			if(terminateColorCovering) break;
		}
		if(terminateColorCovering) break;
	}



	std::cout<<"The following: "<<nonCoveredPixelValueSet.size()<<" pixel values cannot be found in the input image set or they are to be avoided: -- ";
	for (std::set<uchar>::iterator it = nonCoveredPixelValueSet.begin(); it != nonCoveredPixelValueSet.end(); ++it)
	{
		std::cout<<(int)*it<<" ";
	} std::cout<<std::endl;
	std::cout<<"Total number of selected pixel count: "<<pPixelIndexes.size()<<std::endl;
}

int DebevecPixelSelection::getMinRequiredPixelCount(const int& pNumberOfImages)
{
	return ((Z_MAX - Z_MIN) * mSelectionParameters.mSpatialCoverageConst) / (pNumberOfImages - 1);
}

void DebevecPixelSelection::getAvoidedPixelLocations(const std::vector<cv::Mat>& pInputImages, const cv::Mat& pMiddleImage, std::set<int>& pAvoidedPixelLocations)
{
	cv::Mat detectedEdges, dilatedEdges, avoidedPixels;
	int lowThreshold = 50;
	int kernel_size = 3;
	int ratio = 3;
	int dilation_size = 11;
	cv::Mat dilationElement = cv::getStructuringElement(2, cv::Size(2 * dilation_size + 1, 2 * dilation_size + 1),cv::Point(dilation_size, dilation_size));
	cv::blur( pMiddleImage, detectedEdges, cv::Size(kernel_size,kernel_size) );
	cv::Canny( detectedEdges, detectedEdges, lowThreshold, lowThreshold*ratio, kernel_size );
	cv::dilate(detectedEdges, dilatedEdges, dilationElement);
	avoidedPixels = cv::Scalar::all(0);
	pMiddleImage.copyTo( avoidedPixels, dilatedEdges);

	for(int i=0; i<avoidedPixels.rows; i++) {
		for(int j=0; j<avoidedPixels.cols; j++) {
			 if(avoidedPixels.at<uchar>(i,j) != 0)
			 {
				pAvoidedPixelLocations.insert(avoidedPixels.cols * i + j);
			 }
		}
	}

//	for(int i=0; i<pMiddleImage.rows; i++) {
//		for(int j=0; j<pMiddleImage.cols; j++) {
//			const int index = pMiddleImage.cols * i + j;
//			for(size_t ims = 0;ims<pInputImages.size()-1;ims++)
//			{
//				const uchar curVal = pInputImages[ims].at<uchar>(i, j);
//				const uchar nextVal = pInputImages[ims+1].at<uchar>(i, j);
//
//				if(nextVal<=curVal)
//				{
//					pAvoidedPixelLocations.insert(index);
//					avoidedPixels.at<uchar>(i, j) = 255;
//					break;
//				}
//
//			}
//
//		}
//	}

	if(mSelectionParameters.mShowAvoidedPixelLocations)
	{
		cv::namedWindow("Avoided Pixels", cv::WINDOW_AUTOSIZE );
		cv::imshow( "Avoided Pixels", avoidedPixels );
		cv::waitKey(0);
		cv::destroyWindow("Avoided Pixels");
		//cv::imwrite("avoided_Minolta_DiMAGE_A1.png", avoidedPixels);
	}
}

void DebevecPixelSelection::showPixelLocationsOnTheImage(const cv::Mat& pImage, std::set<int>& pPixelIndexes)
{
	cv::Mat selectedPixels = pImage.clone();
	std::vector<cv::Mat> colorChannels;
	colorChannels.push_back(selectedPixels);
	colorChannels.push_back(selectedPixels);
	colorChannels.push_back(selectedPixels);
	cv::Mat selectedPixelsColor;
	cv::merge(colorChannels, selectedPixelsColor);
	for (std::set<int>::iterator it = pPixelIndexes.begin(); it != pPixelIndexes.end(); ++it)
	{
		const int index = *it;
		const int row = index / selectedPixelsColor.cols;
		const int col = index % selectedPixelsColor.cols;
		cv::Point center(col, row);
		cv::circle(selectedPixelsColor, center, 5, cv::Scalar(255,255,0), 4, 8, 0);
	}

	cv::namedWindow("Selected Pixels", cv::WINDOW_AUTOSIZE );
	cv::imshow( "Selected Pixels", selectedPixelsColor);
	cv::waitKey(0);
	cv::destroyWindow("Selected Pixels");

	//cv::imwrite("selected_Minolta_DiMAGE_A1.png", selectedPixelsColor);
}

bool DebevecPixelSelection::writePixelLocations(const std::set<int>& pPixelIndexes)
{
	std::ofstream pFile;
	pFile.open(mSelectionParameters.mOutputPixelLocationsPath.c_str(), std::ios::out);
	if (pFile.is_open()) {
		const size_t numberOfLocations = pPixelIndexes.size();
		pFile<<numberOfLocations<<std::endl;
		for (std::set<int>::iterator it = pPixelIndexes.begin(); it != pPixelIndexes.end(); ++it)
		{
			const int index = *it;
			pFile<<index<<std::endl;
		}

		pFile.close();
	}
	else {
		std::cerr << "Error: Specified File name: " << mSelectionParameters.mOutputPixelLocationsPath << " cannot be opened!" << std::endl;
		return false;
	}
	return true;
}

bool DebevecPixelSelection::readPixelLocations(std::set<int>& pPixelIndexes)
{
	std::ifstream pFile;
	pFile.open(mSelectionParameters.mInputPixelLocationsPath.c_str(), std::ios::in);

	if (pFile.is_open()) {
		size_t numberOfLocations=0;
		pFile>>numberOfLocations;
		for(size_t i=0;i<numberOfLocations;i++)
		{
			int index;
			pFile>>index;
			pPixelIndexes.insert(index);
		}
		pFile.close();
	}
	else {
		std::cerr << "Error: Specified File name: " << mSelectionParameters.mInputPixelLocationsPath << " cannot be opened!" << std::endl;
		return false;
	}
	return true;
}

