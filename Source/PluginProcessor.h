/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "S3A_SiMeter/S3A_SiMeter.h"
//==============================================================================
/**
*/
class S3a_siMeter_pluginAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    S3a_siMeter_pluginAudioProcessor();
    ~S3a_siMeter_pluginAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;
	vector<double> GetIntelBuffer();
	void SetBinaural(bool b);
	//==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;
	juce::String intelligibilityParamName = "intelligibilityParam";
	AudioParameterFloat* intelligibility;
	juce::String targetRMSParamName = "targetRMSParam";
	juce::String maskRMSParamName = "maskRMSParam";

	AudioParameterFloat* targetRMS;
	AudioParameterFloat* maskRMS;


	std::string outputMessage;
	float measurePeriodInSeconds; // The length of time over which an intelligibility value is calculated
	float detectorThreshold;
	float overlap; // The overlap between windows
	bool setupDone;
	bool nnLoaded;
	std::vector<float> intelligibilityBuffer;
	bool refSignalDetected;
	bool maskSignalDetected;
	int emptyBufferCount;


private:
	bool binaural;
	S3A_SiMeter meter;
	float measurementTimer;
	vector<double> intel;
	vector<double> t_rms;
	vector<double> m_rms;
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (S3a_siMeter_pluginAudioProcessor)
};
