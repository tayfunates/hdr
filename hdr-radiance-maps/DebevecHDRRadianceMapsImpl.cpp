/*
 * DebevecHDRRadianceMapsImpl.cpp
 *
 *  Created on: Mar 22, 2016
 *      Author: tayfun
 */

#include <highgui/highgui.hpp>
#include "DebevecHDRRadianceMapsImpl.h"
#include "StringUtil.h"
#include <iostream>
#include <fstream>
#include <dirent.h>
#include <sys/stat.h>
#include <set>
#include <fstream>

DebevecHDRRadianceMapsImpl::DebevecHDRRadianceMapsImpl() {
	// TODO Auto-generated constructor stub
	this->mInputImageFolder = "";
	this->mShowInputImages = false;
	this->mChannelType = BLUE_IMG;
	this->mSelectionParameters = PixelSelectionParameters();
	this->mSmoothnessTerm = 0.5;
	this->mCRFValuesPath = "";
	this->mSampleRadiancesPath = "";
}

DebevecHDRRadianceMapsImpl::~DebevecHDRRadianceMapsImpl() {
	// TODO Auto-generated destructor stub
}

bool DebevecHDRRadianceMapsImpl::init(const std::string& pParamFile)
{
	//Inits the application by reading the parameter file
	if (!readParameterFile(pParamFile))
	{
		std::cerr << "Error: | Parameter file cannot be read" << std::endl;
		return false;
	}
	if(!readInputImages())
	{
		std::cerr << "Error: | Input image files cannot be read" << std::endl;
		return false;
	}

	if(!this->mPixelSelection.init(this->mSelectionParameters))
	{
		std::cerr << "Error | Pixel Selection class cannot be initialized!" << std::endl;
		return false;
	}
	return true;
}

void DebevecHDRRadianceMapsImpl::run()
{
	//Runs the application
	std::set<int> pixelIndexes;
	this->mPixelSelection.getPixels(this->mInputImages, pixelIndexes);

	if(!mSolver.init(static_cast<int>(this->mInputImages.size()),
				static_cast<int>(pixelIndexes.size()),
				Z_MAX, Z_MIN,
				this->mSmoothnessTerm))
	{
		std::cerr<<"Error: | Response function cannot be initialized correctly" <<std::endl;
	}

	mSolver.run(this->mInputImages, pixelIndexes);

	std::cout<<"SVD is completed. Output is being written to the files"<<std::endl;

	std::vector<int> colorVals;
	std::vector<float> radianceVals;

	calculateRadianceValuesForPixelsSelected(mSolver.mlE, pixelIndexes, colorVals, radianceVals);

	writeSampleColorRadiancePairs(colorVals, radianceVals);
	writeOutputResponseCurve(mSolver.mg);
}

void DebevecHDRRadianceMapsImpl::calculateRadianceValuesForPixelsSelected(const std::vector<float>& pIrradiances, const std::set<int>& pPixelIndexes, std::vector<int>& pColorVals, std::vector<float>& pRadianceVals)
{
	pColorVals.clear();
	pRadianceVals.clear();

	const std::vector<float>& shutterSpeeds = this->mSolver.getShutterSpeeds();
	const int cols = this->mInputImages[0].cols;
	const int rows = this->mInputImages[0].rows;

	int locIndex=0;
	for(std::set<int>::iterator it = pPixelIndexes.begin(); it != pPixelIndexes.end(); ++it)
	{
		const int index = *it;
		const int r = index / cols;
		const int c = index % cols;
		const float irradianceVal = pIrradiances[locIndex];
		for(size_t im=0;im<this->mInputImages.size();im++)
		{
			const cv::Mat& currImage = this->mInputImages[im];
			const float logShutterSpeed = log(shutterSpeeds[im]);
			const int pixelVal = static_cast<int>(currImage.at<uchar>(r, c));
			const float radianceVal = irradianceVal + logShutterSpeed;
			pColorVals.push_back(pixelVal);
			pRadianceVals.push_back(radianceVal);
		}
		locIndex++;
	}
}

struct ImageFilePathName
{
	std::string mPath;
	int mName;
};

bool ImageFilePathLess (ImageFilePathName a, ImageFilePathName b) { return (a.mName < b.mName); }

bool DebevecHDRRadianceMapsImpl::readInputImages()
{
	//Read input images path from folder path
    DIR *dir;
    class dirent *ent;
    class stat st;
    std::vector<ImageFilePathName> imageFilePaths;
    dir = opendir(this->mInputImageFolder.c_str());
    if(dir == NULL)
    {
    	std::cerr << "Error: Specified Folder: " << this->mInputImageFolder << " cannot be opened!" << std::endl;
    	return false;
    }
    readdir(dir);
    while ((ent = readdir(dir)) != NULL) {
        const std::string file_name = ent->d_name;
        const std::string full_file_name = this->mInputImageFolder + std::string("/") + file_name;
        if (file_name[0] == '.')
            continue;
        if (stat(full_file_name.c_str(), &st) == -1)
            continue;
        const bool is_directory = (st.st_mode & S_IFDIR) != 0;
        if (is_directory)
            continue;
        ImageFilePathName PathAndName;
        PathAndName.mPath = full_file_name;
        PathAndName.mName = atoi(StringUtil::betweenTwoDelimeters(full_file_name, '/', '.').c_str());

        imageFilePaths.push_back(PathAndName);
    }
    closedir(dir);
    std::sort(imageFilePaths.begin(), imageFilePaths.end(), ImageFilePathLess);
    cv::namedWindow("Input Images", cv::WINDOW_AUTOSIZE );
    for(size_t i=0;i<imageFilePaths.size();i++)
    {
    	cv::Mat savedImage;
    	if(this->mChannelType == COLOR_IMG) {
    		savedImage = cv::imread(imageFilePaths[i].mPath,CV_LOAD_IMAGE_UNCHANGED);
    		savedImage.convertTo(savedImage, CV_8UC4);
    	} else if (this->mChannelType == GREYSCALE_IMG) {
    		savedImage = cv::imread(imageFilePaths[i].mPath,CV_LOAD_IMAGE_GRAYSCALE);
    		savedImage.convertTo(savedImage, CV_8UC1);
    	} else {
    		cv::Mat inputImage = cv::imread(imageFilePaths[i].mPath,CV_LOAD_IMAGE_UNCHANGED);
    		std::vector<cv::Mat> split;
    		cv::split(inputImage,split);
    		savedImage = split[this->mChannelType];
    		savedImage.convertTo(savedImage, CV_8UC1);
    	}

    	this->mInputImages.push_back(savedImage);
    	if(this->mShowInputImages)
    	{

        	cv::imshow("Input Images", this->mInputImages[i]);
        	cv::waitKey(0);

    	}
    	cv::destroyWindow("Input Images");
    }

	return true;
}

bool DebevecHDRRadianceMapsImpl::writeOutputResponseCurve(const std::vector<float>& pValues)
{
	std::string filePath = this->mCRFValuesPath;
	std::ofstream pFile;
	pFile.open(filePath.c_str(), std::ios::out);
	if (pFile.is_open()) {
		for (size_t i=0;i<pValues.size();i++)
		{
			pFile<<pValues[i]<<std::endl;
		}

		pFile.close();
	}
	else {
		std::cerr << "Error: Specified File name: " << filePath << " cannot be opened!" << std::endl;
		return false;
	}
	return true;
}

bool DebevecHDRRadianceMapsImpl::writeSampleColorRadiancePairs(const std::vector<int>& pColorValues, const std::vector<float>& pRadianceValues)
{
	if(pColorValues.size() != pRadianceValues.size())
	{
		std::cerr<<"Error: | Input vectors must have the same size"<<std::endl;
		return false;
	}
	std::string filePath = this->mSampleRadiancesPath;
	std::ofstream pFile;
	pFile.open(filePath.c_str(), std::ios::out);
	if (pFile.is_open()) {
		for (size_t i=0;i<pColorValues.size();i++)
		{
			pFile<<pColorValues[i]<<" "<<pRadianceValues[i]<<std::endl;
		}

		pFile.close();
	}
	else {
		std::cerr << "Error: Specified File name: " << filePath << " cannot be opened!" << std::endl;
		return false;
	}
	return true;
}


bool DebevecHDRRadianceMapsImpl::readParameterFile(const std::string& pParamFilePath)
{
	std::ifstream pFile;
	pFile.open(pParamFilePath.c_str(), std::ios::in);
	if (pFile.is_open())
	{
		std::cout << "Application parameters: " << std::endl;

		std::string line;
		char delim = '=';
		std::string tag, val;
		std::vector<std::string> splitted;
		std::stringstream ss;


		splitted.clear(); ss.clear();
		std::getline(pFile, line);
		StringUtil::split(line, delim, splitted);
		tag = StringUtil::trim(splitted[0]); val = StringUtil::trim(splitted[1]);
		this->mInputImageFolder = val;
		std::cout << tag << ": " << this->mInputImageFolder << std::endl;

		splitted.clear(); ss.clear();
		std::getline(pFile, line);
		StringUtil::split(line, delim, splitted);
		tag = StringUtil::trim(splitted[0]); val = StringUtil::trim(splitted[1]);
		val = StringUtil::toLower(val);
		if(val == "blue")
		{
			this->mChannelType = BLUE_IMG;
		} else if (val == "green") {
			this->mChannelType = GREEN_IMG;
		} else if (val == "red") {
			this->mChannelType = RED_IMG;
		} else if(val == "grey") {
			this->mChannelType = GREYSCALE_IMG;
		} else {
			this->mChannelType = COLOR_IMG;
		}
		std::cout << tag << ": " << val << std::endl;

		splitted.clear(); ss.clear();
		std::getline(pFile, line);
		StringUtil::split(line, delim, splitted);
		tag = StringUtil::trim(splitted[0]); val = StringUtil::trim(splitted[1]);
		this->mShowInputImages = (StringUtil::toLower(val) == "false") ? false : true;
		std::cout << tag << ": " << val << std::endl;

		splitted.clear(); ss.clear();
		std::getline(pFile, line);
		StringUtil::split(line, delim, splitted);
		tag = StringUtil::trim(splitted[0]); val = StringUtil::trim(splitted[1]);
		this->mSelectionParameters.mShowAvoidedPixelLocations = (StringUtil::toLower(val) == "false") ? false : true;
		std::cout << tag << ": " << val << std::endl;

		splitted.clear(); ss.clear();
		std::getline(pFile, line);
		StringUtil::split(line, delim, splitted);
		tag = StringUtil::trim(splitted[0]); val = StringUtil::trim(splitted[1]);
		this->mSelectionParameters.mShowSelectedPixelLocations = (StringUtil::toLower(val) == "false") ? false : true;
		std::cout << tag << ": " << val << std::endl;

		splitted.clear(); ss.clear();
		std::getline(pFile, line);
		StringUtil::split(line, delim, splitted);
		tag = StringUtil::trim(splitted[0]); val = StringUtil::trim(splitted[1]);
		ss << val; ss >> this->mSelectionParameters.mSpatialCoverageConst;
		std::cout << tag << ": " << this->mSelectionParameters.mSpatialCoverageConst << std::endl;

		splitted.clear(); ss.clear();
		std::getline(pFile, line);
		StringUtil::split(line, delim, splitted);
		tag = StringUtil::trim(splitted[0]); val = StringUtil::trim(splitted[1]);
		this->mSelectionParameters.mGeneratePixelLocations = (StringUtil::toLower(val) == "false") ? false : true;
		std::cout << tag << ": " << val << std::endl;

		splitted.clear(); ss.clear();
		std::getline(pFile, line);
		StringUtil::split(line, delim, splitted);
		tag = StringUtil::trim(splitted[0]); val = StringUtil::trim(splitted[1]);
		this->mSelectionParameters.mWritePixelLocations = (StringUtil::toLower(val) == "false") ? false : true;
		std::cout << tag << ": " << val << std::endl;

		splitted.clear(); ss.clear();
		std::getline(pFile, line);
		StringUtil::split(line, delim, splitted);
		tag = StringUtil::trim(splitted[0]); val = StringUtil::trim(splitted[1]);
		this->mSelectionParameters.mOutputPixelLocationsPath = val;
		std::cout << tag << ": " << this->mSelectionParameters.mOutputPixelLocationsPath << std::endl;

		splitted.clear(); ss.clear();
		std::getline(pFile, line);
		StringUtil::split(line, delim, splitted);
		tag = StringUtil::trim(splitted[0]); val = StringUtil::trim(splitted[1]);
		this->mSelectionParameters.mInputPixelLocationsPath = val;
		std::cout << tag << ": " << this->mSelectionParameters.mInputPixelLocationsPath << std::endl;

		splitted.clear(); ss.clear();
		std::getline(pFile, line);
		StringUtil::split(line, delim, splitted);
		tag = StringUtil::trim(splitted[0]); val = StringUtil::trim(splitted[1]);
		ss << val; ss >> this->mSmoothnessTerm;
		std::cout << tag << ": " << this->mSmoothnessTerm << std::endl;

		splitted.clear(); ss.clear();
		std::getline(pFile, line);
		StringUtil::split(line, delim, splitted);
		tag = StringUtil::trim(splitted[0]); val = StringUtil::trim(splitted[1]);
		this->mCRFValuesPath = val;
		std::cout << tag << ": " << this->mCRFValuesPath << std::endl;

		splitted.clear(); ss.clear();
		std::getline(pFile, line);
		StringUtil::split(line, delim, splitted);
		tag = StringUtil::trim(splitted[0]); val = StringUtil::trim(splitted[1]);
		this->mSampleRadiancesPath = val;
		std::cout << tag << ": " << this->mSampleRadiancesPath<< std::endl;

		pFile.close();
	}
	else
	{
		std::cerr << "Error: Specified File name: " << pParamFilePath << " cannot be opened!" << std::endl;
		return false;
	}
	return true;
}
