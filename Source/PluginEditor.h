/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "PluginProcessor.h"

//==============================================================================
/**
*/
class S3a_siMeter_pluginAudioProcessorEditor  : public AudioProcessorEditor, public juce::Timer, public Slider::Listener, public Button::Listener
{
public:
    S3a_siMeter_pluginAudioProcessorEditor (S3a_siMeter_pluginAudioProcessor&);
    ~S3a_siMeter_pluginAudioProcessorEditor();

    //==============================================================================
    void paint (Graphics&) override;
    void resized() override;

private:
	struct graphPoint {
		int xPos;
		int yPos;
		bool isActive;
	};

	enum trafficLight {
		red,
		//amber,
		green
	};
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    S3a_siMeter_pluginAudioProcessor& processor;
	juce::ScopedPointer<juce::Timer> timer;
	void timerCallback() override;
	void sliderValueChanged(Slider* slider) override;
	void buttonClicked(Button* button) override;
	
    juce::Label debugMessage;
    juce::Label signalLabel;
	juce::Label maskLabel;
    int testCounter;
    float holdValue;
    float smoothingValue;
    int refreshRate;
    int tickCounter;
    float pointSeparation;

	juce::Label outputMessage;

    std::vector<int> points;
    std::vector<int> pointsX;
    std::vector<graphPoint> graphPoints;
	
    int canvasHeight = 250;
    int canvasWidth = 420;
    int canvasTopBorder = 40;
    int canvasBottomEdge = canvasHeight + canvasTopBorder;
    int graphWidth = 400;
    
    juce::Slider intelligibilityOutSlider;
    juce::Slider measurePeriodSlider;
	juce::Label measurePeriodSliderLabel;
	juce::Slider detectorThresholdSlider;
	juce::Label detectorThresholdSliderLabel;
    juce::Slider SiThresholdSlider;
    juce::Label SiThresholdSliderLabel;
	juce::ImageButton warningLight;
	juce::TextButton binauralOn;
	
    trafficLight activeLight;
	float t_rms;
	float m_rms;

	bool warning;
	bool binaural;
	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(S3a_siMeter_pluginAudioProcessorEditor)
};
