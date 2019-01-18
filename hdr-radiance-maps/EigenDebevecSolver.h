/*
 * EigenDebevecSolver.h
 *
 *  Created on: Mar 26, 2016
 *      Author: tayfun
 */

#ifndef EIGENDEBEVECSOLVER_H_
#define EIGENDEBEVECSOLVER_H_

#include <core/core.hpp>
#include <Eigen/Dense>
#include <set>

class EigenDebevecSolver {
public:
	EigenDebevecSolver();
	virtual ~EigenDebevecSolver();

	bool init(const int& pImageCount, const int& pPixelCount, const int& pMaxPixVal, const int& pMinPixVal, const float& pSmoothnessTerm);
	void run(const std::vector<cv::Mat>& pInputImages, const std::set<int>& pPixelIndexes);

	const std::vector<float>& getShutterSpeeds() const { return this->mShutterSpeeds; }

	std::vector<float> mg;
	std::vector<float> mlE;

private:
	int mImageCount;
	int mPixelCount;
	int mMaxPixelVal;
	int mMinPixelVal;
	float mSmoothnessTerm;
	int mn;
	std::vector<float> mShutterSpeeds;

	Eigen::MatrixXf mA;
	Eigen::VectorXf mb;
	Eigen::VectorXf mx;

	float getPixelValWeight(const int& pVal);

};

#endif /* EIGENDEBEVECSOLVER_H_ */
