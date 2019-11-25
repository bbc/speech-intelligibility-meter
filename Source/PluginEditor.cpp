/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
S3a_siMeter_pluginAudioProcessorEditor::S3a_siMeter_pluginAudioProcessorEditor (S3a_siMeter_pluginAudioProcessor& p)
    : AudioProcessorEditor (&p), processor (p){
    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
  setSize (800, 450);
    refreshRate = 25;
    startTimer(1000/refreshRate);
    
    testCounter = 0;
    holdValue = 0;
    tickCounter = 0;
    
    addAndMakeVisible(debugMessage);
    
    outputMessage.setColour(Label::textColourId, Colours::red);
    addAndMakeVisible(outputMessage);
    
    intelligibilityOutSlider.setSliderStyle(Slider::LinearBarVertical);
    intelligibilityOutSlider.setRange(0.0, 1.0, 0.001);
    intelligibilityOutSlider.setTextBoxStyle(Slider::NoTextBox, false, 90, 0);
    
    measurePeriodSlider.setSliderStyle(Slider::LinearHorizontal);
    measurePeriodSlider.setRange(1, 30, 1);
    measurePeriodSlider.setValue(1);
    measurePeriodSlider.setTextValueSuffix("s");
    measurePeriodSlider.addListener(this);
    
    measurePeriodSliderLabel.setText("Measurement Period", dontSendNotification);
    measurePeriodSliderLabel.attachToComponent(&measurePeriodSlider, false);
    
	detectorThresholdSlider.setSliderStyle(Slider::LinearHorizontal);
	detectorThresholdSlider.setRange(-120, 0, 1);
	detectorThresholdSlider.setValue(-80);
	detectorThresholdSlider.setTextValueSuffix("dB");
	detectorThresholdSlider.addListener(this);

	detectorThresholdSliderLabel.setText("Signal detector threshold", dontSendNotification);
	detectorThresholdSliderLabel.attachToComponent(&detectorThresholdSlider, false);

	SiThresholdSlider.setSliderStyle(Slider::LinearHorizontal);
	SiThresholdSlider.setRange(0, 0.95, 0.05);
	SiThresholdSlider.setValue(0.4);
	SiThresholdSlider.setTextValueSuffix("");
	SiThresholdSlider.addListener(this);

	SiThresholdSliderLabel.setText("Speech intelligibility threshold", dontSendNotification);
	SiThresholdSliderLabel.attachToComponent(&SiThresholdSlider, false);

	warningLight.addListener(this);
	addAndMakeVisible(warningLight);

	/*
	//Currently only binaural network is used. Mono signals are sent as two identical stereo channels
	binauralOn.setName("Stereo");
	binauralOn.setButtonText("Stereo");
	binauralOn.setTooltip("Toggles stereo estimation of speech intelligibility,\n simulating stereo signals as emitted from two loudspeakers");
	binauralOn.setClickingTogglesState(true);
	
	binauralOn.addListener(this);
	
	addAndMakeVisible(binauralOn);
	binaural = binauralOn.getToggleState();
	*/
	//comment out this line if the above is implemented for a dedicated mono trained network
	binaural = true;
    addAndMakeVisible(intelligibilityOutSlider);
    
	addAndMakeVisible(measurePeriodSlider);
    addAndMakeVisible(measurePeriodSliderLabel);
	
	addAndMakeVisible(detectorThresholdSlider);
	addAndMakeVisible(detectorThresholdSliderLabel);
	
	addAndMakeVisible(SiThresholdSlider);
	addAndMakeVisible(SiThresholdSliderLabel);
    
    signalLabel.setText("Speech signal detected :", dontSendNotification);
    addAndMakeVisible(signalLabel);
	
	maskLabel.setText("Sidechain signal detected : " , dontSendNotification);
	addAndMakeVisible(maskLabel);
	warning = false;
	pointSeparation = 1;
    resized();
}

S3a_siMeter_pluginAudioProcessorEditor::~S3a_siMeter_pluginAudioProcessorEditor()
{
}

//==============================================================================

void S3a_siMeter_pluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
	// Set positions of GUI components - all just hard coded for now
	outputMessage.setBounds(30, 390, 700, 30);

	intelligibilityOutSlider.setBounds(190, 40, 60, getHeight() / 2);
	debugMessage.setBounds(180, 180, 80, getHeight() - 240);

	measurePeriodSlider.setBounds(30, getHeight() - 115, 250, 50);
	SiThresholdSlider.setBounds(360, getHeight() - 45, 250, 50);

	detectorThresholdSlider.setBounds(30, getHeight() - 45, 250, 50);
	
	signalLabel.setBounds(360, getHeight() -130, 100, 80);
	maskLabel.setBounds(560, getHeight() -130, 100, 80);
	warningLight.setBounds(310, canvasTopBorder, canvasWidth, canvasHeight + canvasTopBorder / 2);
	
	binauralOn.setBounds(60, 200, 70, 50);
	
}

void S3a_siMeter_pluginAudioProcessorEditor::paint(Graphics& g)
{
	// (Our component is opaque, so we must completely fill the background with a solid colour)
	g.fillAll(getLookAndFeel().findColour(ResizableWindow::backgroundColourId));

	// Draw empty traffic light circles
	g.setColour(Colours::green);
	g.drawEllipse(70, 50, 50, 50, 3);

	g.setColour(Colours::red);
	g.drawEllipse(70, 125, 50, 50, 3);

	
	if (processor.refSignalDetected) {
		signalLabel.setText("Speech signal detected " + to_string(*processor.targetRMS), dontSendNotification);
		// Fill the appropriate traffic light
		switch (activeLight) {
		case red:
			g.setColour(Colours::red);
			g.fillEllipse(70, 125, 50, 50);
			break;
		case green:
			g.setColour(Colours::green);
			g.fillEllipse(70, 50, 50, 50);
			break;
		default:
			break;
		}

		//Set the colour for the reference signal detected circle
		g.setColour(Colours::green);
		
		intelligibilityOutSlider.setAlpha(1.0);
	}
	else {
		signalLabel.setText("Speech signal not detected", dontSendNotification);
		g.setColour(Colours::red);
		intelligibilityOutSlider.setAlpha(0.4);
	}
	// Draw the target signal detected circle
	g.fillEllipse(330, getHeight() - 100, 20, 20);

	if (processor.maskSignalDetected) {
		
		g.setColour(Colours::green);
		maskLabel.setText("Sidechain signal detected", dontSendNotification);
	}
	else {
		g.setColour(Colours::red);
		maskLabel.setText("Sidechain signal not detected", dontSendNotification);
	}

	// Draw the mask signal detected circle
	g.fillEllipse(530, getHeight() - 100, 20, 20);

	
	// Draw graph
	if(warning)
	g.setColour(Colours::darkred);
	else g.setColour(Colours::black);

	g.fillRect(320, canvasTopBorder, canvasWidth, canvasHeight + canvasTopBorder / 2);

	// Draw lines between consecutive graph points
	if (graphPoints.size() > 1) {
		for (int i = 0; i < graphPoints.size() - 1; i++) {
			if (graphPoints[i].isActive) {
				g.setColour(Colours::white);
				g.drawLine(graphPoints[i].xPos, graphPoints[i].yPos, graphPoints[i + 1].xPos, graphPoints[i + 1].yPos);
			}
			else {
				g.setColour(Colours::blue);
				int length = (graphPoints[i + 1].xPos - graphPoints[i].xPos) / 2;
				g.drawLine(graphPoints[i].xPos, graphPoints[i].yPos, graphPoints[i].xPos + length, graphPoints[i + 1].yPos);
			}
		}
	}
	// Paint over the start and end of the graph to create the gradual appearance/disappearance of lines
	if (warning)
		g.setColour(Colours::darkred);
	else g.setColour(Colours::black);
	int blockWidth = ceil(pointSeparation);
	if (blockWidth > 20) { blockWidth = 20; }
	g.fillRect(680 - blockWidth, canvasTopBorder, blockWidth + 5, canvasHeight + canvasTopBorder / 2);
	g.fillRect(320, canvasTopBorder, blockWidth, canvasHeight + canvasTopBorder / 2);

	// Add a green dashed line to indicate the threshold of "acceptable" intelligibility values
	int acceptableLine = canvasBottomEdge - (SiThresholdSlider.getValue() * (canvasHeight - 20));
	g.setColour(Colours::green);
	for (int i = 0; i < ((canvasWidth / 40) + 1); i++) {
		g.drawLine(320 + (i * 40), acceptableLine, 320 + (i * 40) + 20, acceptableLine);
	}
	g.setFont(14.0f);
	g.setColour(Colours::green);
	g.drawText("Good", 320 + 10, acceptableLine - 30, 80, 30, Justification::centred);
	g.setColour(Colours::red);
	g.drawText("Bad", 320 + 10, acceptableLine, 80, 30, Justification::centred);
}

void S3a_siMeter_pluginAudioProcessorEditor::timerCallback() {
	
	//repeat last value if speech isn't detected in the measurement period
	float currentValue = 0;
	float threshold = SiThresholdSlider.getValue();
	processor.SetBinaural(binauralOn.getToggleState());
	if(graphPoints.size() !=0)
		 currentValue = holdValue;
	if(processor.refSignalDetected)
		currentValue = *processor.intelligibility;
	
	t_rms = *processor.targetRMS;
	m_rms = *processor.maskRMS;
	intelligibilityOutSlider.setValue(currentValue);

	   if (currentValue != holdValue) {
	        smoothingValue = float(currentValue - holdValue) / float(refreshRate * measurePeriodSlider.getValue());
	        holdValue = currentValue;

	    }
	
	    float sliderValue = intelligibilityOutSlider.getValue();
	    if ((smoothingValue > 0 && sliderValue < holdValue) || (smoothingValue < 0 && sliderValue > holdValue)) {
	        float newSliderValue = intelligibilityOutSlider.getValue() + smoothingValue;
	        intelligibilityOutSlider.setValue(newSliderValue);
	    }
	std::string intelligibilityAsString = std::to_string(intelligibilityOutSlider.getValue()).substr(0,4);
	debugMessage.setText("Intelligibility:    " + intelligibilityAsString, dontSendNotification);

	float maxPointsToDisplay = 60;
	pointSeparation = graphWidth / maxPointsToDisplay;
													
	int interpolationRequired = 20;

	// Create a graph point for the current intelligibility value and add it to the vector of points
	// Currently only adds a point if it is new (as this callback occurs far more frequently than the calculation of an intelligibility value)
	// this would cause a bug if two consecutive intelligibility values are similar enough to create the same graph point value...
	graphPoint thisGraphPoint;

	thisGraphPoint.yPos = canvasBottomEdge - (currentValue * (canvasHeight - 20));
	thisGraphPoint.xPos = 680;
	if (processor.refSignalDetected) {
		thisGraphPoint.isActive = true;
	}
	else {
		thisGraphPoint.isActive = false;
	}

	if (graphPoints.size() == 0) {
		graphPoints.push_back(thisGraphPoint);
	}
	else if (graphPoints[graphPoints.size() - 1].yPos != thisGraphPoint.yPos
		|| graphPoints[graphPoints.size() - 1].xPos < 680 - interpolationRequired) {
		graphPoints.push_back(thisGraphPoint);
	}

	// Move all graph points to the left to create a scrolling effect
	// Remove graph point if it passes the left edge of the canvas
	tickCounter++;
	int totalTicks = 60 * refreshRate; // ticks required for one minute of data on graph
	int ticksToWait = totalTicks / graphWidth * measurePeriodSlider.getValue();

	if (tickCounter >= ticksToWait) {
		for (int i = 0; i < graphPoints.size(); i++) {
			graphPoints[i].xPos--;
		}
		if (graphPoints[0].xPos <= 320) {
			graphPoints.erase(graphPoints.begin());
		}

		tickCounter = 0;
	}

	// Calculate average of last few seconds of intelligibility values to determine
	// which traffic light to set
	double sum = 0;
	
	if (*processor.intelligibility < threshold && processor.refSignalDetected) {
		activeLight = red;
		warning = true;
	}
	else {
		activeLight = green;
	}


	S3a_siMeter_pluginAudioProcessorEditor::repaint();
}

void S3a_siMeter_pluginAudioProcessorEditor::sliderValueChanged(Slider* slider) {
	if (slider == &measurePeriodSlider) {
		processor.measurePeriodInSeconds = measurePeriodSlider.getValue();
		processor.setupDone = false;
	}
	
	else if (slider == &detectorThresholdSlider) {
		processor.detectorThreshold = detectorThresholdSlider.getValue();
		processor.setupDone = false;
	}
}

void S3a_siMeter_pluginAudioProcessorEditor::buttonClicked(Button* button) 
{
		if (button == &warningLight)
		{
			warning = false;
		}
		
	
}



