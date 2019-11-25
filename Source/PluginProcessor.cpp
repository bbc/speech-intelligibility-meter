/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <algorithm>
//==============================================================================
S3a_siMeter_pluginAudioProcessor::S3a_siMeter_pluginAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
	: AudioProcessor(BusesProperties()
		.withInput("Input", AudioChannelSet::stereo(), true)
		.withInput("Sidechain", AudioChannelSet::stereo(), true)
		.withOutput("Output", AudioChannelSet::stereo(), true))
#endif
{
	binaural = false;
	setupDone = false;
	nnLoaded = false;
	addParameter(intelligibility = new AudioParameterFloat(intelligibilityParamName,
		"Intelligibility",
		NormalisableRange<float>(0.0f, 1.0f),
		0.0f));
	addParameter(targetRMS = new AudioParameterFloat(targetRMSParamName,
		"Target RMS",
		NormalisableRange<float>(-120.0f, 10.0f),
		0.0f));
	addParameter(maskRMS = new AudioParameterFloat(maskRMSParamName,
		"Sidechain RMS",
		NormalisableRange<float>(-120.0f, 10.0f),
		0.0f));
	detectorThreshold = -80;
	measurePeriodInSeconds = 1;// default value
	overlap = 0.2;
	measurementTimer = 0.0;
	refSignalDetected = false;
	maskSignalDetected = false;
	emptyBufferCount = 0;
}

S3a_siMeter_pluginAudioProcessor::~S3a_siMeter_pluginAudioProcessor()
{
}

//==============================================================================
const String S3a_siMeter_pluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool S3a_siMeter_pluginAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool S3a_siMeter_pluginAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool S3a_siMeter_pluginAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double S3a_siMeter_pluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int S3a_siMeter_pluginAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int S3a_siMeter_pluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void S3a_siMeter_pluginAudioProcessor::setCurrentProgram (int index)
{
}

const String S3a_siMeter_pluginAudioProcessor::getProgramName (int index)
{
    return {};
}

void S3a_siMeter_pluginAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void S3a_siMeter_pluginAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
	meter.Config(1,sampleRate);
}

void S3a_siMeter_pluginAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
//	meter.Config(1, getSampleRate(), true);
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool S3a_siMeter_pluginAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
	return true;
	//Throws faults if DAW has wrong inputs
	// Currently requires stereo inputs for both ref and mix
	//return layouts.getMainInputChannelSet() == AudioChannelSet::stereo()
	//	&& layouts.getChannelSet(true, 1) == AudioChannelSet::stereo()
	//	&& layouts.getMainInputChannelSet() == layouts.getMainOutputChannelSet()
	//	&& !layouts.getMainInputChannelSet().isDisabled();
}
#endif

void S3a_siMeter_pluginAudioProcessor::processBlock (AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
	meter.SetDetectThreshold(detectorThreshold);
	if (measurePeriodInSeconds < 1)
		measurePeriodInSeconds = 1;
	auto mixInput = getBusBuffer(buffer, true, 0);
	auto refInput = getBusBuffer(buffer, true, 1);
	

	
    ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();
	AudioSampleBuffer mixBuffer = AudioSampleBuffer(2, static_cast<int>(buffer.getNumSamples()));
	mixBuffer.clear();
	AudioSampleBuffer refBuffer = AudioSampleBuffer(2, static_cast<int>(buffer.getNumSamples()));
	refBuffer.clear();

	if (refInput.getNumChannels() > 0) {
		if (refInput.getReadPointer(0) != NULL) {
			refBuffer.copyFrom(0, 0, refInput, 0, 0, refInput.getNumSamples());
			refBuffer.copyFrom(1, 0, refInput, 1, 0, refInput.getNumSamples());
		}
	}
	else {
		// create empty buffer of n = mixSample length
		refBuffer.setSize(buffer.getNumChannels(), buffer.getNumSamples());
		refBuffer.clear();
	}
	if (mixInput.getNumChannels() >0) {
		if (mixInput.getReadPointer(0) != NULL) {
			mixBuffer.copyFrom(0, 0, mixInput, 0, 0, mixInput.getNumSamples());
			mixBuffer.copyFrom(1, 0, mixInput, 1, 0, mixInput.getNumSamples());
		}
	}
	else {
		// create empty buffer of n = mixSample length
		mixBuffer.setSize(buffer.getNumChannels(), buffer.getNumSamples());
		mixBuffer.clear();
	}

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

	
	vector<vector<double>> masker;
	vector<vector<double>> target;
	for (int i = 0; i < 2; i++) {
		const float* start_m = refBuffer.getReadPointer(i);
		std::vector<double> m(start_m, start_m + refBuffer.getNumSamples());
		masker.push_back(m);
		const float* start_t = mixBuffer.getReadPointer(i);
		int size = mixInput.getNumSamples();
		std::vector<double> t(start_t, start_t + mixBuffer.getNumSamples());
		target.push_back(t);
	}
	
	
	double _intel = 0;
	if (!binaural) {
		for (int i = 0; i < target.size(); i++) {
			for (int j = 0; j < target[i].size(); j++)
			{
				target[0][j] += target[i][j]/target.size();
				masker[0][j] += masker[i][j]/ target.size();
			}
		}
		meter.UpdateBuffers(target, masker);
		_intel = meter.GetIntelligibility(measurePeriodInSeconds);
	}
	else {
		if (getChannelCountOfBus(true, 0) < 2) {
			for (int i = 1; i < target.size(); i++) {
				for (int j = 0; j < target[i].size(); j++)
				{
					target[1][j] += target[i][j];
					masker[1][j] += masker[i][j];
				}
			}
		}
		meter.UpdateBuffersBinaural(target, masker);
		_intel = meter.GetIntelligibilityBinaural(measurePeriodInSeconds);
	}
	measurementTimer += (float(getBlockSize())/ float(getSampleRate()));
	
	
	if (_intel > 1.0) {
		_intel = 1.0;
	}
	else if (_intel < 0) {
		_intel = 0;
	}

	if (isnan(_intel))
		_intel = 0;

	intel.push_back(_intel);
	t_rms.push_back(meter.GetTargetRMS());
	m_rms.push_back(meter.GetMaskerRMS());
	double result = 0;
	double t_result = 0;
	double m_result = 0;
	for (int i = 0; i < t_rms.size(); i++)
		t_result += t_rms[i];
	t_result /= t_rms.size();
	t_rms.clear();
	for (int k = 0; k < m_rms.size(); k++)
		m_result += m_rms[k];
	m_result /= m_rms.size();
	m_rms.clear();
	*targetRMS = t_result;
	*maskRMS = m_result;
	refSignalDetected = (t_result > detectorThreshold);
	maskSignalDetected = (m_result > detectorThreshold);
	if (measurementTimer > measurePeriodInSeconds) {
		for (int i = 0; i < intel.size(); i++)
			result += intel[i];
		result /= intel.size();
		intel.clear();
		
		measurementTimer = 0;
		*intelligibility = static_cast<float>(result);
		
	}
	//*targetRMS = meter.GetTargetRMS();
	//*maskRMS = meter.GetMaskerRMS();
	
}
vector<double> S3a_siMeter_pluginAudioProcessor::GetIntelBuffer() {
	return intel;
}
//==============================================================================
bool S3a_siMeter_pluginAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* S3a_siMeter_pluginAudioProcessor::createEditor()
{
    return new S3a_siMeter_pluginAudioProcessorEditor (*this);
}

//==============================================================================
void S3a_siMeter_pluginAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void S3a_siMeter_pluginAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new S3a_siMeter_pluginAudioProcessor();
}

void S3a_siMeter_pluginAudioProcessor::SetBinaural(bool b) {
	binaural = b; 
}
