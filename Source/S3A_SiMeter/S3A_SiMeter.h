/*
###################################################
# S3A SI Meter
# University of Salford 2019
# Based on Tan et al. 2017
###################################################
*/

#pragma once
//C++17
#include <iostream>
#include <algorithm>
#include <fstream>
#include <assert.h>
#include <complex>

//Includes
#include "./DSPtools.h"
#include "./hrtfs.h"
#include "../EditedGistSourceCode/Gist.h"
#include "DWGPnet.h"
#include "DWGPnet_02.h"


using namespace DSPtools;
class S3A_SiMeter
{
public:
	//Types
	enum Mode {
		Mono,
		Stereo
	};

	//Functions
	//ConstructorAndConfig.cpp
	S3A_SiMeter();
	S3A_SiMeter(int fs);
	S3A_SiMeter(int channels, int fs);
	void Config(int channels, int _fs);
	virtual ~S3A_SiMeter();

	//InputFunctions.cpp
	void UpdateBuffers(vector<vector<double>> targ, vector<vector<double>> mask);
	void UpdateBuffersBinaural(vector<vector<double>> targ, vector<vector<double>> mask);
	void AntiAlias(int channels, vector<vector<double>> &input, vector<BiQuad> &filters);
	
	//OutputFunctionsMono.cpp
	double GetIntelligibility(float sampleTime);
	vector<double> GetFeatures(float sampleTime);
	vector<vector<vector<double>>> GetMFCCs(float sampleTime);
	
	//OutputFunctionsStereo.cpp
	double GetIntelligibilityBinaural(float sampleTime);
	vector<double> GetFeaturesBinaural(float sampleTime);
	
	//StateFunctions.cpp
	bool TargetActive();
	bool MaskerActive();
	float GetTargetRMS();
	float GetMaskerRMS();
	void SetDetectThreshold(double x);
	Mode GetMode();
	void SetMode(Mode mode);
	

private:
	Mode mode;
	vector<vector<complex<double>>> hrtfs;
	//Functions
	//ConstructorAndConfig.cpp

	//InputFunctions.cpp
	void SignalDetector(float sampleTime);
	
	//Properties
	int fs;
	vector<CircularBuffer> targetBuffer;
	vector<CircularBuffer> maskerBuffer;
	CircularBuffer targetRMSbuffer;
	CircularBuffer maskerRMSbuffer;
	vector <BiQuad> aaT;
	vector <BiQuad> aaM;
	InterpFilt resampler;
	vector<vector<double>> b_targ;
	vector<vector<double>> b_sc;
	vector<vector<double>> b_mask;
	double detectThreshold;
	double targetRMS;
	double maskerRMS;
	bool targetActive;
	bool maskerActive;
	bool hrtfLoaded;
};

