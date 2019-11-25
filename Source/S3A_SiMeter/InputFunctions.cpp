/*
###################################################
# S3A SI Meter
# University of Salford 2019
# Based on Tan et al. 2017
###################################################
*/



#include "S3A_SiMeter.h"
void S3A_SiMeter::AntiAlias(int channels, vector<vector<double>> &input, vector<BiQuad> &filters) {
	for (int i = 0; i < channels; i++) {
		for (int j = 0; j < input[i].size(); j++) {
			int n = 0 * channels;
			input[i][j] = filters[n].Filter(filters[n + 1].Filter(input[i][j]));
		}
	}
}

void  S3A_SiMeter::SignalDetector(float sampleTime) {

	int size = int(sampleTime / 0.02);
	//get mean RMS for the available time window
	vector<vector<double>> tt = this->targetRMSbuffer.GetBlock(size, 0);
	vector<vector<double>> mm = this->maskerRMSbuffer.GetBlock(size, 0);

	vector<double> tRMS = Tools::MeanVector(tt);
	vector<double> mRMS = Tools::MeanVector(mm);

	//Mono signal detection as for now. All channels meaned;
	this->targetRMS = Tools::Lin2dB(Tools::Mean(tRMS)*10);
	this->maskerRMS = Tools::Lin2dB(Tools::Mean(mRMS)*10);

	if (this->targetRMS < detectThreshold) {
		this->targetActive = false;
	}
	else {
		this->targetActive = true;
	}

	if (this->maskerRMS < detectThreshold) {
		this->maskerActive = false;
	}
	else {
		this->maskerActive = true;
	}
}


void S3A_SiMeter::UpdateBuffers(vector<vector<double>> targ, vector<vector<double>> mask)
{

	int windowSize = 320;
	int hopSize = 160;
	Gist<double> t_gist(windowSize, 16000);
	Gist<double> m_gist(windowSize, 16000);

	//identify num channels in target
	int nTarg = targ.size();

	//identify num channels in masker
	int nMask = mask.size();

	//antialias inputs
	AntiAlias(nTarg, targ, aaT);
	AntiAlias(nMask, mask, aaM);

	//resample input vectors to 16k
	vector<vector<double>> r_t;
	vector<vector<double>> r_m;

	for (int i = 0; i < nTarg; i++) {
		r_t.push_back(resampler.Resample(targ[i], 16000, fs));

		r_m.push_back(resampler.Resample(mask[i], 16000, fs));
	}

	//Update Buffers
	for (int i = 0; i < r_m.size(); i++) {
		for (int j = 0; j < r_m[i].size(); j++) {

			this->b_targ[i].push_back(r_t[i][j]);
			this->b_sc[i].push_back(r_m[i][j]);
			this->b_mask[i].push_back(r_m[i][j] + r_t[i][j]);

			if (b_targ[i].size() >= 320) {
				vector<double> t;
				t.push_back(Tools::RMS(b_targ[i]));
				vector<vector<double>> tc;
				tc.push_back(t);
				targetRMSbuffer.AddBlock(tc);
				t_gist.processAudioFrame(b_targ[i]);
				vector<vector<double>> t_mfcc;
				t_mfcc.push_back(t_gist.getMelFrequencyCepstralCoefficients());
				t_mfcc = Tools::Transpose(t_mfcc);
				targetBuffer[i].AddBlock(t_mfcc);
				this->b_targ[i].erase(this->b_targ[i].begin(), this->b_targ[i].begin() + 160);

				vector<double> m;
				m.push_back(Tools::RMS(b_sc[i]));
				vector<vector<double>> mc;
				mc.push_back(m);
				maskerRMSbuffer.AddBlock(mc);
				this->b_sc[i].erase(this->b_sc[i].begin(), this->b_sc[i].begin() + 160);

				m_gist.processAudioFrame(b_mask[i]);
				vector<vector<double>> m_mfcc;
				m_mfcc.push_back(m_gist.getMelFrequencyCepstralCoefficients());
				m_mfcc = Tools::Transpose(m_mfcc);
				maskerBuffer[i].AddBlock(m_mfcc);
				this->b_mask[i].erase(this->b_mask[i].begin(), this->b_mask[i].begin() + 160);
			}
		}

	}
	return;
}

void S3A_SiMeter::UpdateBuffersBinaural(vector<vector<double>> targ, vector<vector<double>> mask)
{
	if (targ.size() < 2) {
		UpdateBuffers(targ, mask);
		return;
	}

	int windowSize = 320;
	int hopSize = 160;
	Gist<double> t_gistL(windowSize, 16000);
	Gist<double> m_gistL(windowSize, 16000);
	Gist<double> t_gistR(windowSize, 16000);
	Gist<double> m_gistR(windowSize, 16000);

	Gist<double>* t_gist[2] = { &t_gistL, &t_gistR };

	Gist<double>* m_gist[2] = { &m_gistL, &m_gistR };
	//identify num channels in target
	int nTarg = targ.size();

	//identify num channels in masker
	int nMask = mask.size();
	//antialias inputs
	for (int i = 0; i < nTarg; i++) {
		for (int j = 0; j < targ[i].size(); j++) {
			int n = 0 * nTarg;
			targ[i][j] = aaT[n].Filter(aaT[n + 1].Filter(targ[i][j]));
		}
	}
	//printf("AntiAlias target OK\n");
	for (int i = 0; i < nMask; i++) {
		for (int j = 0; j < mask[i].size(); j++) {
			int n = 0 * nMask;
			mask[i][j] = aaM[n].Filter(aaM[n + 1].Filter(mask[i][j]));
		}
	}
	
	//resample input vectors to 16k
	vector<vector<double>> r_t;
	vector<vector<double>> r_m;

	for (int i = 0; i < nTarg; i++) {
		r_t.push_back(resampler.Resample(targ[i], 16000, fs));

		r_m.push_back(resampler.Resample(mask[i], 16000, fs));

	}


	for (int i = 0; i < 2; i++) {
		for (int j = 0; j < r_m[i].size(); j++) {
			if (isnan(r_t[i][j]))r_t[i][j] = 0;
			if (isnan(r_m[i][j]))r_m[i][j] = 0;
			this->b_targ[i].push_back(r_t[i][j]);
			this->b_sc[i].push_back(r_m[i][j]);
			this->b_mask[i].push_back(r_m[i][j] + r_t[i][j]);

			if (b_targ[i].size() >= 320) {


				vector<double> t;
				t.push_back(Tools::RMS(b_targ[i]));
				vector<vector<double>> tc;
				tc.push_back(t);
				targetRMSbuffer.AddBlock(tc);

				t_gist[i]->processAudioFrame(b_targ[i]);
				
				if (hrtfLoaded) {
					for (int k = 0; k < 320; k++) {
						complex<double> c{ t_gist[i]->fftReal[k] , t_gist[i]->fftImag[k] };
						complex<double> h[2];


						h[0] = c * hrtfs[1 * (i)+0][k];
						h[1] = c * hrtfs[1 * (i)+2][k];

						t_gist[i]->fftReal[k] = real(h[0]) + real(h[1]);
						t_gist[i]->fftImag[k] = imag(h[0]) + imag(h[1]);
					}
					vector<double> x = t_gist[i]->getMagnitudeSpectrum();
				}
				vector<vector<double>> t_mfcc;
				t_mfcc.push_back(t_gist[i]->getMelFrequencyCepstralCoefficients());
				t_mfcc = Tools::Transpose(t_mfcc);
				if (i == 0) {
					targetBuffer[i].AddBlock(t_mfcc);
				}
				else if (i == 1) {
					targetBuffer[i].AddBlock(t_mfcc);
				}
				this->b_targ[i].erase(this->b_targ[i].begin(), this->b_targ[i].begin() + 160);


				vector<double> m;
				m.push_back(Tools::RMS(b_sc[i]));
				vector<vector<double>> mc;
				mc.push_back(m);
				maskerRMSbuffer.AddBlock(mc);
				this->b_sc[i].erase(this->b_sc[i].begin(), this->b_sc[i].begin() + 160);

				m_gist[i]->processAudioFrame(b_mask[i]);

				
				if (hrtfLoaded) {
					for (int k = 0; k < 320; k++) {
						complex<double> c{ m_gist[i % 2]->fftReal[k] , m_gist[i % 2]->fftImag[k] };
						complex<double> h[2];

						h[0] = c * hrtfs[1 * (i) + 0][k];
						h[1] = c * hrtfs[1 * (i) + 2][k];

						m_gist[i]->fftReal[k] = real(h[0]) + real(h[1]);
						m_gist[i]->fftImag[k] = imag(h[0]) + imag(h[1]);
					}
					vector<double> x = m_gist[i]->getMagnitudeSpectrum();
				}
				vector<vector<double>> m_mfcc;
				m_mfcc.push_back(m_gist[i]->getMelFrequencyCepstralCoefficients());
				m_mfcc = Tools::Transpose(m_mfcc);
				if (i == 0) {
					maskerBuffer[i].AddBlock(m_mfcc);
				}
				else if (i == 1){
					maskerBuffer[i].AddBlock(m_mfcc);
				}
				this->b_mask[i].erase(this->b_mask[i].begin(), this->b_mask[i].begin() + 160);
			}
		}
	}

}
