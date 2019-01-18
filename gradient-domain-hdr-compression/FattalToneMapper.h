/*
 * FattalToneMapper.h
 *
 *  Created on: May 11, 2016
 *      Author: tayfun
 */

#ifndef FATTALTONEMAPPER_H_
#define FATTALTONEMAPPER_H_

#include <core/core.hpp>
#include <string>
#include <vector>

class FattalToneMapper {
public:
	FattalToneMapper();
	virtual ~FattalToneMapper();

	bool init(const std::string& pParamFile);
	void run();

private:
	cv::Mat m_HDRRGB;				//< Input HDR Image with RGB channels
	cv::Mat m_InpLuminances;		//< Input Luminances
	cv::Mat m_InpLogLuminances;		//< Input Log Luminances
	cv::Mat m_OutLogLuminances;		//< Compressed Log Luminances
	cv::Mat m_OutLuminances;		//< Compressed Luminances
	cv::Mat m_AttenuationMap;		//< Attenuation Function

	//Parses the parameter file
	bool readParameterFile(const std::string& pParamFilePath);
	bool readHDRInput();

	void showHDRImage(const cv::Mat& pHDRInput); //Applies linear scaling

	//Application parameters
	std::string m_HDRIInputPath;
	std::string m_LDRIOutputPath;
	float m_GradMagnitudeAlphaScale;
	float m_Beta;
	float m_ColorSaturationConstant;

	//Methods used for calculation of gradient attenuation function
	bool calculateGradientAttenuationMap();

	//Methods used for calculation of output luminance map
	bool calculateOutputLuminanceMap();

};

#endif /* FATTALTONEMAPPER_H_ */
