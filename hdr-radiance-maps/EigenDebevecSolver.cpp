/*
 * EigenDebevecSolver.cpp
 *
 *  Created on: Mar 26, 2016
 *      Author: tayfun
 */

#include "EigenDebevecSolver.h"
#include <iostream>

EigenDebevecSolver::EigenDebevecSolver() {
	// TODO Auto-generated constructor stub
	this->mImageCount = 0;
	this->mPixelCount = 0;
	this->mMaxPixelVal = 0;
	this->mMinPixelVal = 0;
	this->mSmoothnessTerm = 0.0f;
}

EigenDebevecSolver::~EigenDebevecSolver() {
	// TODO Auto-generated destructor stub
}

bool EigenDebevecSolver::init(const int& pImageCount, const int& pPixelCount, const int& pMaxPixVal, const int& pMinPixVal, const float& pSmoothnessTerm)
{
	if(!pImageCount || !pPixelCount) return false;

	this->mImageCount = pImageCount;
	this->mPixelCount = pPixelCount;
	this->mMaxPixelVal = pMaxPixVal;
	this->mMinPixelVal = pMinPixVal;
	this->mSmoothnessTerm = pSmoothnessTerm;
	this->mn = this->mMaxPixelVal - this->mMinPixelVal + 1;;

	const int ARow = mImageCount * mPixelCount + mn - 1;
	const int ACol = mn + mPixelCount;

	this->mA = Eigen::MatrixXf::Zero(ARow, ACol);
	this->mb = Eigen::VectorXf::Zero(ARow);
	this->mg = std::vector<float>(mn);
	this->mlE = std::vector<float>(mPixelCount);

	const int middleImageIndex = (mImageCount+1) / 2;
	const float middleShutterSpeed = 1.0f;
	this->mShutterSpeeds = std::vector<float>(mImageCount);
	this->mShutterSpeeds[middleImageIndex] = middleShutterSpeed;

	for(int i=middleImageIndex-1;i>=0;i--)
	{
		this->mShutterSpeeds[i] = this->mShutterSpeeds[i+1] / 2.0f;
	}

	for(int i=middleImageIndex+1;i<mImageCount;i++)
	{
		this->mShutterSpeeds[i] = this->mShutterSpeeds[i-1] * 2.0f;
	}

	return true;
}

void EigenDebevecSolver::run(const std::vector<cv::Mat>& pInputImages, const std::set<int>& pPixelIndexes)
{
	if(pInputImages.empty() || pInputImages[0].empty()) return;

	const int cols = pInputImages[0].cols;

	int k=0;
	int locIndex=0;
	for(std::set<int>::iterator it = pPixelIndexes.begin(); it != pPixelIndexes.end(); ++it)
	{
		const int index = *it;
		const int y = index / cols;
		const int x = index % cols;
		for(int j=0;j<mImageCount;j++)
		{
			const cv::Mat& currImg = pInputImages[j];
			const int pixelVal = static_cast<int>(currImg.at<uchar>(y, x));
			const float log_t = log(mShutterSpeeds[j]);
			float weight  = getPixelValWeight(pixelVal+1);
			mA(k, pixelVal) = weight;
			mA(k, mn+locIndex) = -1.0f * weight;
			mb(k) = weight * log_t;
			k = k + 1;
		}
		locIndex++;
	}

	//Setting the middle value
	mA(k, (mn+1) / 2) = 1.0f;
	k = k + 1;

	for(int i=0; i< mn - 2; i++)
	{
		const float weight = getPixelValWeight(i+1);
		mA(k, i) = mSmoothnessTerm * 1.0f * weight;
		mA(k, i+1) = mSmoothnessTerm * -2.0f * weight;
		mA(k, i+2) = mSmoothnessTerm * 1.0f * weight;
		k = k + 1;
	}

	std::cout<<"SVD is on progress..."<<std::endl;
	mx = mA.jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(mb);

	for(int i=0;i<mn;i++)
	{
		mg[i] = mx(i);
	}
	for(int i=0;i<mPixelCount;i++)
	{
		mlE[i] = mx(mn+i);
	}
}

float EigenDebevecSolver::getPixelValWeight(const int& pVal)
{
	const int midVal = 0.5f * (mMinPixelVal + mMaxPixelVal);
	return ((pVal <= midVal) ? static_cast<int>(pVal) - static_cast<int>(mMinPixelVal) : static_cast<int>(mMaxPixelVal) - static_cast<int>(pVal));

}
