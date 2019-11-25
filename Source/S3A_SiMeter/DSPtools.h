/*
###################################################
# Signal processing tools for use with the
# S3A SI Meter
# University of Salford 2020
###################################################
*/
#pragma once
#include <vector>
using namespace std;
namespace DSPtools {
	class Tools {
	public:
		static double Sum(vector<double> &x);
		static double Sum(vector<double> &x, vector<double> &z);
		static double SqSum(vector<double>& x);
		static vector<vector<double>> Transpose(vector<vector<double>> &x);
		static double calculateCorrCoef(vector <double>& X, vector<double>& Y);
		static double RMS(vector<double> &x);
		static double Mean(vector<double> &x);
		static vector<double> MeanVector(vector<vector<double>> &x);
		static double Lin2dB(double x);
		static double DB2Lin(double x);
		static void AmbisonicEncode(vector<double> &in, vector<vector<double>>&out, double theta);
		static void AmbisonicDecode(vector<vector<double>> &in, vector<vector<double>> &out, vector<double> theta);
	};
	enum FilterType
	{
		lowPass,
		highPass,
	};

	class BiQuad {

	public:
		BiQuad();
		virtual ~BiQuad();
		void SetType(FilterType t, double f, double fs);
		double Filter(double x);
	private:
		double z[2];
		double a[3];
		double b[3];
		FilterType type;
		void LP(double f, double fs);
		void HP(double f, double fs);
	};


	class InterpFilt {
	public:
		InterpFilt();
		virtual ~InterpFilt();
		static std::vector<double> Resample(std::vector<double> data, int p, int q);
	private:

	};

	class CircularBuffer {
	public:
		CircularBuffer();
		virtual ~CircularBuffer();
		void AddBlock(vector<vector<double>> x);
		void AddSample(vector<double> x);
		vector<vector<double>> GetBlock(int size, int offset);
		vector<double> GetSample(int rh);
		void SetChannels(int size);
		int GetChannels();
		void SetSize(int size);
		int GetSize();
	private:
		vector<vector<double>> buffer;
		int length;
		int read;
		int write;
		int Wrap(int r);
	};


}