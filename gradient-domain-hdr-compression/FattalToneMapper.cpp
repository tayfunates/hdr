/*
 * FattalToneMapper.cpp
 *
 *  Created on: May 11, 2016
 *      Author: tayfun
 */

#include <highgui/highgui.hpp>
#include <imgproc/imgproc.hpp>
#include "FattalToneMapper.h"
#include "HDRLoader.h"
#include "StringUtil.h"
#include "Laplace.h"
#include <iostream>
#include <fstream>

#define LINEAR_TRANS_ALPHA_1(min, max) ((1.0) / ((max)-(min)))
#define LINEAR_TRANS_BETA_1(min, max) (((-1.0) * (min)) / ((max) - (min)))
#define LINEAR_TRANS_ALPHA_255(min, max) ((255) / ((max)-(min)))
#define LINEAR_TRANS_BETA_255(min, max) (((-255) * (min)) / ((max) - (min)))
#define LINEAR_TRANS_ALPHA_100(min, max) ((100.0) / ((max)-(min)))
#define LINEAR_TRANS_BETA_100(min, max) (((-100.0) * (min)) / ((max) - (min)))

FattalToneMapper::FattalToneMapper() {
	// TODO Auto-generated constructor stub
	this->m_HDRIInputPath = "";
	this->m_LDRIOutputPath = "";
	this->m_GradMagnitudeAlphaScale = 0.1f;
	this->m_Beta = 0.90;
	this->m_ColorSaturationConstant = 0.5;

}

FattalToneMapper::~FattalToneMapper() {
	// TODO Auto-generated destructor stub
}

bool FattalToneMapper::init(const std::string& pParamFile)
{
	//Inits the application by reading the parameter file
	if (!readParameterFile(pParamFile))
	{
		std::cerr << "Error: | Parameter file cannot be read" << std::endl;
		return false;
	}
	if(!readHDRInput())
	{
		std::cerr<<"Cannot load HDR File"<<std::endl;
		return false;
	}


	return true;
}

bool FattalToneMapper::readHDRInput()
{
	HDRLoaderResult loadRes;
	bool load = HDRLoader::load(this->m_HDRIInputPath.c_str(), loadRes);

	if(!load || loadRes.width <= 0 || loadRes.height <= 0 || !loadRes.cols )
	{

		return false;
	}

	this->m_HDRRGB = cv::Mat (cv::Size(loadRes.width, loadRes.height), CV_32FC3 ,loadRes.cols);
	//Conversion to 0-1
	double min;
	double max;
	cv::minMaxIdx(this->m_HDRRGB, &min, &max);
	this->m_HDRRGB.convertTo(this->m_HDRRGB,CV_32FC3, LINEAR_TRANS_ALPHA_1(min, max), LINEAR_TRANS_BETA_1(min, max));

	cv::Mat imgXYZ;
	cv::cvtColor(this->m_HDRRGB, imgXYZ, CV_BGR2XYZ);

	std::vector<cv::Mat> spl;
	cv::split(imgXYZ,spl);

	this->m_InpLuminances = spl[1].clone();

	//Conversion to 0-100
	cv::minMaxIdx(this->m_InpLuminances, &min, &max);
	this->m_InpLuminances.convertTo(this->m_InpLuminances,CV_32FC3, LINEAR_TRANS_ALPHA_1(min, max), LINEAR_TRANS_BETA_1(min, max));

	//Take the logarithm of the image
	cv::log(this->m_InpLuminances,this->m_InpLogLuminances);

	return true;
}

void FattalToneMapper::run()
{
	cv::Mat attenuationMap;
	calculateGradientAttenuationMap();
	calculateOutputLuminanceMap();

	cv::Mat ImLin = this->m_InpLuminances;
	cv::Mat ImLout = this->m_OutLuminances;
	cv::Mat invLin;
	cv::pow(ImLin, -1.0, invLin);

	cv::Mat outRGB;
    std::vector<cv::Mat> chOut(this->m_HDRRGB.channels());
	std::vector<cv::Mat> spl;
	cv::split(this->m_HDRRGB,spl);
	for(size_t c=0; c<this->m_HDRRGB.channels() ; c++)
	{
		cv::Mat ImCout;
		cv::Mat ImCin = spl[c];
		cv::Mat temp;
		cv::pow(ImCin.mul(invLin), this->m_ColorSaturationConstant, temp);
		ImCout = temp.mul(ImLout);

		double min;
		double max;
		cv::minMaxIdx(ImCout, &min, &max);
		ImCout.convertTo(ImCout,CV_32FC1, LINEAR_TRANS_ALPHA_1(min, max), LINEAR_TRANS_BETA_1(min, max));
		chOut[this->m_HDRRGB.channels()-1-c] = ImCout.clone();
	}
	merge(chOut, outRGB);

	showHDRImage(this->m_HDRRGB);
	showHDRImage(this->m_InpLuminances);
	showHDRImage(this->m_OutLuminances);
	showHDRImage(this->m_AttenuationMap);
	showHDRImage(outRGB);
}

bool FattalToneMapper::readParameterFile(const std::string& pParamFilePath)
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
		this->m_HDRIInputPath = val;
		std::cout << tag << ": " << this->m_HDRIInputPath << std::endl;

		splitted.clear(); ss.clear();
		std::getline(pFile, line);
		StringUtil::split(line, delim, splitted);
		tag = StringUtil::trim(splitted[0]); val = StringUtil::trim(splitted[1]);
		this->m_LDRIOutputPath = val;
		std::cout << tag << ": " << this->m_LDRIOutputPath << std::endl;

		splitted.clear(); ss.clear();
		std::getline(pFile, line);
		StringUtil::split(line, delim, splitted);
		tag = StringUtil::trim(splitted[0]); val = StringUtil::trim(splitted[1]);
		ss << val; ss >> this->m_GradMagnitudeAlphaScale;
		std::cout << tag << ": " << this->m_GradMagnitudeAlphaScale << std::endl;

		splitted.clear(); ss.clear();
		std::getline(pFile, line);
		StringUtil::split(line, delim, splitted);
		tag = StringUtil::trim(splitted[0]); val = StringUtil::trim(splitted[1]);
		ss << val; ss >> this->m_Beta;
		std::cout << tag << ": " << this->m_Beta << std::endl;

		splitted.clear(); ss.clear();
		std::getline(pFile, line);
		StringUtil::split(line, delim, splitted);
		tag = StringUtil::trim(splitted[0]); val = StringUtil::trim(splitted[1]);
		ss << val; ss >> this->m_ColorSaturationConstant;
		std::cout << tag << ": " << this->m_ColorSaturationConstant << std::endl;

		pFile.close();
	}
	else
	{
		std::cerr << "Error: Specified File name: " << pParamFilePath << " cannot be opened!" << std::endl;
		return false;
	}
	return true;
}

bool FattalToneMapper::calculateGradientAttenuationMap()
{
	cv::Mat dst, tmp;
	tmp = this->m_InpLogLuminances;
	dst = tmp;

	std::vector<cv::Mat> gaussPyramid;
	/// Gaussian pyramid loop
	while(tmp.cols >= 32 && tmp.rows >= 32 )
	{
		gaussPyramid.push_back(tmp);
		GaussianBlur( tmp, dst, cv::Size( 3, 3 ), 0, 0 );
		cv::resize(dst, dst, cv::Size( tmp.cols/2, tmp.rows/2 ), 0, 0,cv::INTER_LINEAR);
		tmp = dst;
	}

	const int pyrSize = static_cast<int>(gaussPyramid.size());
	std::vector<cv::Mat> scalingMapPyramid(pyrSize);
	for(int i=pyrSize-1 ; i>=0 ; i--)
	{
		cv::Mat gradX = cv::Mat(gaussPyramid[i].rows,gaussPyramid[i].cols, CV_32F, float(0));
		cv::Mat gradY = cv::Mat(gaussPyramid[i].rows,gaussPyramid[i].cols, CV_32F, float(0));;

		float gradScale = 1.0f / pow(2.0f, i+1);

		 for( int y=0 ; y<gaussPyramid[i].rows ; y++ )
		  {
			for( int x=0 ; x<gaussPyramid[i].cols ; x++ )
			{
			  float gx, gy;
			  int xm, ym, xp, yp;
			  ym = (y == 0 ? 0 : y-1);
			  xm = (x == 0 ? 0 : x-1);
			  yp = (y+1 == gaussPyramid[i].rows ? y : y+1);
			  xp = (x+1 == gaussPyramid[i].cols ? x : x+1);

			  gx = gaussPyramid[i].at<float>(y, xm) - gaussPyramid[i].at<float>(y, xp);
			  gx *= gradScale;

			  gy = gaussPyramid[i].at<float>(yp, x) - gaussPyramid[i].at<float>(ym, x);
			  gy *= gradScale;

			  gradX.at<float>(y, x) = gx;
			  gradY.at<float>(y, x) = gy;
			}
		  }

		cv::Mat gradMagnitude;
		cv::magnitude(gradX, gradY, gradMagnitude);

		cv::Scalar avgMag = cv::mean(gradMagnitude);
		float alpha = avgMag.val[0] * this->m_GradMagnitudeAlphaScale;

		cv::Mat alphaScaledMagnitude = gradMagnitude * (1.0f / alpha);
		cv::Mat inverseAlphaScaledMagnitude;
		cv::pow(alphaScaledMagnitude, -1.0, inverseAlphaScaledMagnitude);
		cv::Mat betaScaledMagnitude;
		cv::pow(alphaScaledMagnitude, this->m_Beta, betaScaledMagnitude);
		cv::Mat scalingMap = betaScaledMagnitude.mul(inverseAlphaScaledMagnitude);

		if(i == pyrSize-1)
		{
			scalingMapPyramid[i] = scalingMap;
		}
		else
		{
			cv::Mat temp = scalingMapPyramid[i+1];
			cv::resize(temp, temp, scalingMap.size(), 0, 0,cv::INTER_LINEAR);
			GaussianBlur( temp, temp, cv::Size( 3, 3 ), 0, 0 );
			scalingMapPyramid[i] = scalingMap.mul(temp);
		}
	}

	this->m_AttenuationMap = scalingMapPyramid[0].clone();
	return true;
}

bool FattalToneMapper::calculateOutputLuminanceMap()
{
	cv::Mat gradX = cv::Mat(this->m_InpLogLuminances.rows,this->m_InpLogLuminances.cols, CV_32F, float(0));
	cv::Mat gradY = cv::Mat(this->m_InpLogLuminances.rows,this->m_InpLogLuminances.cols, CV_32F, float(0));

    for(int y=0 ; y<this->m_InpLogLuminances.rows ; y++ )
    {
        for(int x=0 ; x<this->m_InpLogLuminances.cols ; x++ )
        {
          int yp1 = (y+1 >= this->m_InpLogLuminances.rows ? this->m_InpLogLuminances.rows-2 : y+1);
          int xp1 = (x+1 >= this->m_InpLogLuminances.cols ? this->m_InpLogLuminances.cols-2 : x+1);

          gradX.at<float>(y, x) =
          		(this->m_InpLogLuminances.at<float>(y, xp1) - this->m_InpLogLuminances.at<float>(y, x)) * 0.5 *
          		(this->m_AttenuationMap.at<float>(y, xp1) + this->m_AttenuationMap.at<float>(y, x));

          gradY.at<float>(y, x) =
          		(this->m_InpLogLuminances.at<float>(yp1, x) - this->m_InpLogLuminances.at<float>(y, x)) * 0.5 *
          		(this->m_AttenuationMap.at<float>(yp1, x) + this->m_AttenuationMap.at<float>(y, x));
        }
    }

	cv::Mat GGradAdd = cv::Mat(this->m_InpLogLuminances.rows,this->m_InpLogLuminances.cols, CV_32F, float(0));
	for(int y=0 ; y<this->m_InpLogLuminances.rows ; y++ )
	{
		for(int x=0 ; x<this->m_InpLogLuminances.cols ; x++ )
		{
			GGradAdd.at<float>(y, x) = gradX.at<float>(y, x) + gradY.at<float>(y, x);
			if(x>0) GGradAdd.at<float>(y, x) -= gradX.at<float>(y, x-1);
			if(y>0) GGradAdd.at<float>(y, x) -= gradY.at<float>(y-1, x);
			if(x==0) GGradAdd.at<float>(y, x) += gradX.at<float>(y, x);
			if(y==0) GGradAdd.at<float>(y, x) += gradY.at<float>(y, x);
		}
	}

	size_t n1=GGradAdd.rows, n2=GGradAdd.cols;
	double h1=1.0, h2=1.0, a1=1.0, a2=1.0;
	pde::types::boundary bdtype=pde::types::Neumann;
	double bdvalue=0.0;

	pde::fftw_threads(1);   // number of threads for internal fft routine

	boost::multi_array<double,2> F(boost::extents[n1][n2]);
	boost::multi_array<double,2> U;
	double trunc;  // return value for poisolve(), indicates truncation error

	for(size_t i=0; i<n1; i++) {
	  for(size_t j=0; j<n2; j++) {
		 F[i][j]=GGradAdd.at<float>(i, j);
	  }
	}

	if(bdtype==pde::types::Neumann) {
	  // find compatible boundary value, otherwise truncation error>0
	  bdvalue=pde::neumann_compat(F,a1,a2,h1,h2);
	}
	// Poisson equation solver
	trunc=pde::poisolve(U,F,a1,a2,h1,h2,bdvalue,bdtype,false);

	this->m_OutLogLuminances = GGradAdd.clone();
	for(size_t i=0; i<n1; i++) {
	  for(size_t j=0; j<n2; j++) {
		  this->m_OutLogLuminances.at<float>(i, j) = U[i][j];
	  }
	}
	cv::exp(this->m_OutLogLuminances, this->m_OutLuminances);
	//Renormalization
	double min;
	double max;
	cv::minMaxIdx(this->m_OutLuminances, &min, &max);
	this->m_OutLuminances.convertTo(this->m_OutLuminances,CV_32FC1, LINEAR_TRANS_ALPHA_1(min, max), LINEAR_TRANS_BETA_1(min, max));
	// minor cleanup in case fftw-threads were used
	pde::fftw_clean();
	return true;
}


void FattalToneMapper::showHDRImage(const cv::Mat& pHDRInput)
{
	if(pHDRInput.channels() == 3)
	{
		double min;
		double max;
		cv::minMaxIdx(pHDRInput, &min, &max);
		cv::Mat adjMap;
		// expand your range to 0..255. Similar to histEq();
		pHDRInput.convertTo(adjMap,CV_8UC3, LINEAR_TRANS_ALPHA_255(min, max), LINEAR_TRANS_BETA_255(min, max));
		cv::imshow("Out", adjMap);
		cv::imwrite(this->m_LDRIOutputPath, adjMap);
		cv::waitKey(0);
	}
	else
	{
		double min;
		double max;
		cv::minMaxIdx(pHDRInput, &min, &max);
		cv::Mat adjMap;
		// expand your range to 0..255. Similar to histEq();
		pHDRInput.convertTo(adjMap,CV_8UC1, LINEAR_TRANS_ALPHA_255(min, max), LINEAR_TRANS_BETA_255(min, max));
		cv::imshow("Out", adjMap);
		cv::waitKey(0);
	}
}

