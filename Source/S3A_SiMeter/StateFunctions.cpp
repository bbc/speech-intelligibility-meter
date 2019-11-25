/*
###################################################
# S3A SI Meter
# University of Salford 2019
# Based on Tan et al. 2017
###################################################
*/



#include "S3a_SiMeter.h"

bool S3A_SiMeter::TargetActive() {
	return this->targetActive;
}

bool S3A_SiMeter::MaskerActive() {
	return this->maskerActive;
}

float  S3A_SiMeter::GetTargetRMS() {
	return (float(this->targetRMS));
}

float S3A_SiMeter::GetMaskerRMS() {
	return (float(this->maskerRMS));
}

void S3A_SiMeter::SetDetectThreshold(double x) {
	this->detectThreshold = x;
}

S3A_SiMeter::Mode S3A_SiMeter::GetMode() {
	return mode;
}
void S3A_SiMeter::SetMode(S3A_SiMeter::Mode x) {
	this->mode = x;
}
