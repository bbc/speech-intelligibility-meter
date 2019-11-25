/*
###################################################
# S3A SI Meter
# University of Salford 2019
# Based on Tan et al. 2017
###################################################
*/



#include "S3A_SiMeter.h"

double S3A_SiMeter::GetIntelligibilityBinaural(float sampleTime) {

	//Report whether buffers have been empty for up to the value of sampleTime
	this->SignalDetector(sampleTime);
	//read out 320 samples in 160 sample hops into MFCC calc. 
	//create and destroy GISTs as need be
	//end up with 9xN (N = ((5*16k)/320 * 2) = 500) MFCC vectors

	int offset = int(sampleTime / 0.02);
	vector<vector<double>> t_mfccL = this->targetBuffer[0].GetBlock(offset, 0);
	vector<vector<double>> m_mfccL = this->maskerBuffer[0].GetBlock(offset, 0);
	vector<vector<double>> t_mfccR = this->targetBuffer[1].GetBlock(offset, 0);
	vector<vector<double>> m_mfccR = this->maskerBuffer[1].GetBlock(offset, 0);

	float cc[9];

	for (int i = 0; i < 9; i++) {

		float x = float(Tools::calculateCorrCoef(t_mfccL[i], m_mfccL[i]));
		float y = float(Tools::calculateCorrCoef(t_mfccR[i], m_mfccR[i]));
		if (isnan(x))x = 0;
		if (isnan(y))y = 0;
		if (isinf(x))x = 0;
		if (isinf(y))y = 0;
		cc[i] = max(x, y);
	}
	double sum = 0;
	for (int channels = 0; channels < 9; channels++) {
		sum += cc[channels];
	}
	//input to ANN
	float eosi = (float)DWGPnet_02((cc));
	if (sum == 0) {
		eosi = 0;
	}
	return eosi;
}

vector<double> S3A_SiMeter::GetFeaturesBinaural(float sampleTime) {
	int offset = int(sampleTime / 0.02);

	vector<vector<double>> t_mfccL = this->targetBuffer[0].GetBlock(offset, 0);
	vector<vector<double>> m_mfccL = this->maskerBuffer[0].GetBlock(offset, 0);
	vector<vector<double>> t_mfccR = this->targetBuffer[1].GetBlock(offset, 0);
	vector<vector<double>> m_mfccR = this->maskerBuffer[1].GetBlock(offset, 0);

	vector<double> cc;
	
	for (int i = 0; i < 9; i++) {
		
		float x = float(Tools::calculateCorrCoef(t_mfccL[i], m_mfccL[i]));
		float y = float(Tools::calculateCorrCoef(t_mfccR[i], m_mfccR[i]));
		if (isnan(x))x = 0;
		if (isinf(x))x = 0;

		if (isnan(y))y = 0;
		if (isinf(y))y = 0;
		
		cc.push_back(max(x, y));
	}

	return cc;
}