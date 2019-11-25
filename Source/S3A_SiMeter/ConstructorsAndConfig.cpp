/*
###################################################
# S3A SI Meter
# University of Salford 2019
# Based on Tan et al. 2017
###################################################
*/

#include "S3A_SiMeter.h"

S3A_SiMeter::S3A_SiMeter()
{
	//create and init all private members
	//overload this to provide flexibility in input channels
	Config(1, 48000);
}

S3A_SiMeter::S3A_SiMeter(int _fs)
{
	//create and init all private members
	//overload this to provide flexibility in input channels
	Config(1, _fs);
}

S3A_SiMeter::S3A_SiMeter(int channels, int _fs)
{
	//create and init all private members
	//overload this to provide flexibility in input channels
	Config(channels, _fs);
}

S3A_SiMeter::~S3A_SiMeter()
{
	//not using pointers at the moment
	//delete[] this->targetBuffer;
	//delete[] this->maskerBuffer;

	//delete everything
}

void S3A_SiMeter::Config(int channels, int _fs) {

	hrtfLoaded = false;
	this->hrtfs = HRTFs::GetHRTFs();
	hrtfLoaded = true;
	this->fs = _fs;

	this->aaT.resize(2 * channels);

	this->targetBuffer;
	this->maskerBuffer;
	for (int i = 0; i < channels; i++) {
		CircularBuffer mt;
		mt.SetChannels(9);
		mt.SetSize(3000);
		targetBuffer.push_back(mt);
		maskerBuffer.push_back(mt);
	}

	for (int i = 0; i < aaT.size(); i++)
		this->aaT[i].SetType(FilterType::lowPass, 7500, this->fs);

	this->aaM.resize(2 * channels);

	for (int i = 0; i < aaM.size(); i++)
		this->aaM[i].SetType(FilterType::lowPass, 7500, this->fs);
	//create short audio buffers
	for (int i = 0; i < channels; i++) {
		vector<double> v;
		v.push_back(0);
		this->b_targ.push_back(v);
		this->b_sc.push_back(v);
		this->b_mask.push_back(v);
	}


	this->targetRMSbuffer.SetChannels(channels);
	this->targetRMSbuffer.SetSize(3000);

	this->maskerRMSbuffer.SetChannels(channels);
	this->maskerRMSbuffer.SetSize(3000);

	this->SetDetectThreshold(-60);

}

