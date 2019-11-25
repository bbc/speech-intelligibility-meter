/*
###################################################
# Signal processing tools for use with the
# S3A SI Meter
# University of Salford 2020
###################################################
*/
#include "DSPtools.h"
#include <cmath>
#include <iostream>

#define TAU 6.28318530718
#define sqrtTwo 1.41421356237

using namespace DSPtools;
BiQuad::BiQuad() {

}

BiQuad::~BiQuad() {

}
void BiQuad::SetType(FilterType t, double f, double fs) {

	switch (t) {
	case lowPass:
		BiQuad::LP(f, fs);
		break;
	case highPass:
		BiQuad::HP(f, fs);
		break;
	default:
		break;
	}
}

void BiQuad::LP(double f, double fs) {
	double V = pow(10, (0 / 20));
	double K = tan(3.141593 * f / fs);
	double Q = 0.707;
	double norm = 1 / (1 + K / Q + K * K);

	a[0] = K * K * norm;
	a[1] = 2 * a[0];
	a[2] = a[0];
	b[0] = 1;
	b[1] = 2 * (K * K - 1) * norm;
	b[2] = (1 - K / Q + K * K) * norm;
}

void BiQuad::HP(double f, double fs) {
	double V = pow(10, (0 / 20));
	double K = tan(3.141593 * f / fs);
	double Q = 0.707;
	double norm = 1 / (1 + K / Q + K * K);

	a[0] = norm;
	a[1] = -2 * a[0];
	a[2] = a[0];
	b[0] = 1;
	b[1] = 2 * (K * K - 1) * norm;
	b[2] = (1 - K / Q + K * K) * norm;
}

double BiQuad::Filter(double x) {

	double y = this->BiQuad::a[0] * x + this->BiQuad::z[1];
	z[1] = this->BiQuad::z[0] + x * this->BiQuad::a[1] - y * this->BiQuad::b[1];
	z[0] = x * this->BiQuad::a[2] - y * this->BiQuad::b[2];
	return y;
}

InterpFilt::InterpFilt() {

}

InterpFilt::~InterpFilt() {

}

std::vector<double> InterpFilt::Resample(std::vector<double> data, int p, int q) {
	std::vector<double> y;
	double pq = double(q) / double(p);
	double interp = pq - floor(pq);
	double ipq = (1 / pq);
	if (p != q) {
		
		int lim = ((floor(data.size() * ipq)) - ceil(pq));
		for (int i = 0; i < lim; i++) {
			int j = i * floor(pq);
			int k = j + ceil(pq);
			double h = (data[j] * (1 - interp)) + (data[k] * interp);
			
			y.push_back(h);
		
		}
		return y;
	}
	else return data;
}

double Tools::Sum(vector<double> &x) {
	double y = 0;
	for (int i = 0; i < x.size(); i++) {
		y += x[i];
	}

	return y;
}
double Tools::Sum(vector<double> &x, vector<double> &z) {
	double y = 0;
	for (int i = 0; i < x.size(); i++) {
		y += (x[i] * z[i]);
	}

	return y;
}

double Tools::SqSum(vector<double> &x) {
	double y = 0;
	for (int i = 0; i < x.size(); i++) {
		y += pow(x[i], 2);
	}

	return y;
}
vector<vector<double>> Tools::Transpose(vector<vector<double>> &x) {
	vector<vector<double>> y;
	for (int i = 0; i < x[0].size(); i++) {
		vector<double> z;
		for (int j = 0; j < x.size(); j++) {
			z.push_back(x[j][i]);
		}
		y.push_back(z);
	}
	return(y);
}

double Tools::calculateCorrCoef(vector <double>& X, vector<double>& Y) {
	if (X.size() == Y.size()) {
		double sumX = Sum(X);
		double sumY = Sum(Y);

		double meanX = sumX / double(X.size());
		double meanY = sumY / double(Y.size());

		double sumXY = Sum(X, Y);
		double n = double(X.size());
		double ss_x = SqSum(X);
		double ss_y = SqSum(Y);
		double corrCoef = (sumXY - (n*meanX*meanY)) / (sqrt(ss_x - (n*pow(meanX, 2))) * sqrt(ss_y - (n*pow(meanY, 2))));

		return corrCoef;
	}
	else {
		return 0;
	}
}

double Tools::RMS(vector<double> &x) {
	double y = SqSum(x) / x.size();
	return y;
}

double Tools::Mean(vector<double> &x) {
	double y = Sum(x) / x.size();
	return y;
}

double Tools::Lin2dB(double x) {
	double y = 20 * log10(x);
	return y;
}

double Tools::DB2Lin(double x) {
	double y = pow(10,x/20);
	return y;
}
vector<double> Tools::MeanVector(vector<vector<double>> &x) {
	vector<double> y;
	for (int i = 0; i < x.size(); i++) {
		y.push_back(Tools::Sum(x[i]) / x[i].size());
	}
	return y;
}

void Tools::AmbisonicEncode(vector<double> &in, vector<vector<double>> &out, double theta) {
	out.push_back(in);
	for (int i = 0; i < 2; i++) {
		vector<double> x;
		for (int j = 0; j < in.size(); j++) {
			x.push_back(in[i] * cos(theta*0.01745) + (0.5*3.14159265359*float(i)));
		}
		out.push_back(x);
	}
}

void Tools::AmbisonicDecode(vector<vector<double>> &in, vector<vector<double>> &out, vector<double> theta) {
	vector<vector<double>> locs;
	for (int i = 0; i < theta.size(); i++) {
		vector<double> x = { 1 / sqrtTwo, cos(theta[i] * 0.01745), sin(theta[i] * 0.01745) };
		locs.push_back(x);
	}
	for (int j = 0; j < 2; j++) {
		vector<double> y;
		for (int k = 0; k < in[j].size(); k++) {
			double x = locs[j][0] * in[0][k] + locs[j][1] * in[1][k] + locs[j][2] * in[2][k];
			y.push_back(x);
		}
	}
}

CircularBuffer::CircularBuffer() {
	//assume 5s @ 16k
	//if you use this for MFCCs
	//Change it
	int fs = 16000;
	int t = 3;
	this->read = 0;
	this->write = this->read;
	this->length = t * fs;
	
	//tempValues for minimum usage before setup
	this->CircularBuffer::buffer.resize(2);
	for (int i = 0; i<this->CircularBuffer::buffer.size(); i++)
		this->CircularBuffer::buffer[i].resize(int(this->length));

}

//clear all buffers before destruction
CircularBuffer::~CircularBuffer() {
	this->buffer.resize(0);
}

void CircularBuffer::SetChannels(int size) {
	this->buffer.resize(size);
}
int CircularBuffer::GetChannels() {
	int s = this->buffer.size();
	return s;
}
void CircularBuffer::SetSize(int size) {
	for (int i = 0; i<this->buffer.size(); i++)
		this->buffer[i].resize(size);
}
int CircularBuffer::GetSize() {
	int s = this->buffer[0].size();
	return s;
}

void CircularBuffer::AddBlock(vector<vector<double>> x) {
	for (int i = 0; i < x.size(); i++) {
		
		for (int j = 0; j < x[0].size(); j++) {
			
			this->buffer[i][Wrap(this->write + j)] = x[i][j];
		}
	}
	this->write = Wrap(this->write + x[0].size());
}

void CircularBuffer::AddSample(vector<double> x) {
	for (int i = 0; i < x.size(); i++)
		this->buffer[i][this->write] = x[i];
	this->write = Wrap(this->write + 1);
}

vector<vector<double>> CircularBuffer::GetBlock(int size, int offset) {
	vector<vector<double>> out;
	int rh = this->write - size + offset;
	
	for (int i = 0; i < this->buffer.size(); i++) {
		vector<double> x;
		for (int j = 0; j < size; j++) {
			int k = rh + j;
			x.push_back(this->buffer[i][Wrap(k)]);
			if (x[j] == 0)
			{
			//	cout << "ZeroValue in buffer\r";
			}
		}
		
		out.push_back(x);
	}
	return out;
}

vector<double> CircularBuffer::GetSample(int rh) {
	vector<double> out;
	for (int i = 0; i < out.size(); i++)
		out.push_back(this->buffer[i][Wrap(write-rh)]);
	return out;
}


int CircularBuffer::Wrap(int r) {
	if (r < 0) {
		r += buffer[0].size();
		r = CircularBuffer::Wrap(r);
	}
	 else if (r >= buffer[0].size()) {
		r -= buffer[0].size();
		r = CircularBuffer::Wrap(r);
	}
	
	return r;
}
