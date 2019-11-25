/*
###################################################
# S3A SI Meter
# University of Salford 2019
# Based on Tan et al. 2017
###################################################
*/



#include "S3A_SiMeter.h"

vector<vector<vector<double>>> S3A_SiMeter::GetMFCCs(float sampleTime) {
	vector<vector<vector<double>>> output;
	int offset = int(sampleTime / 0.02);
	vector<vector<double>> t_mfcc = this->targetBuffer[0].GetBlock(offset, 0);
	vector<vector<double>> m_mfcc = this->maskerBuffer[0].GetBlock(offset, 0);

	t_mfcc = Tools::Transpose(t_mfcc);
	m_mfcc = Tools::Transpose(m_mfcc);
	output.push_back(t_mfcc);
	output.push_back(m_mfcc);
	return output;
}

vector<double> S3A_SiMeter::GetFeatures(float sampleTime) {
	int offset = int(sampleTime / 0.02);
	vector<vector<double>> t_mfcc = this->targetBuffer[0].GetBlock(offset, 0);
	vector<vector<double>> m_mfcc = this->maskerBuffer[0].GetBlock(offset, 0);


	vector<double> cc;

	for (int i = 0; i < 9; i++) {
		cc.push_back(Tools::calculateCorrCoef(t_mfcc[i], m_mfcc[i]));
	}
	return cc;
}

double S3A_SiMeter::GetIntelligibility(float sampleTime) {

	//Report whether buffers have been empty for up to the value of sampleTime
	this->SignalDetector(sampleTime);
	//read out 320 samples in 160 sample hops into MFCC calc. 
	//create and destroy GISTs as need be
	//end up with 9xN (N = ((5*16k)/320 * 2) = 500) MFCC vectors

	int offset = int(sampleTime / 0.02);
	vector<vector<double>> t_mfcc = this->targetBuffer[0].GetBlock(offset, 0);
	vector<vector<double>> m_mfcc = this->maskerBuffer[0].GetBlock(offset, 0);

	float cc[9];

	if (GetMode() == Mode::Mono) {

		for (int i = 0; i < 9; i++) {
			cc[i] = float(Tools::calculateCorrCoef(t_mfcc[i], m_mfcc[i]));
		}
	}

	for (int channels = 0; channels < t_mfcc.size(); channels++) {

	}
	//input to ANN
	float eosi = (float)DWGPnet((cc));
	if (isnan(eosi)) eosi = 0;
	return eosi;
}


