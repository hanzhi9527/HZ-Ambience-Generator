/*
  ==============================================================================

    Noise_Generator.h
    Created: 4 Dec 2023 8:41:46pm
    Author:  Hanzhi Zhang

  ==============================================================================
*/

#pragma once
#include <JuceHeader.h>

enum noise_type {WHITE_NOISE, BINARY1, BINARY2};

class WhiteNoise
{
public:
    WhiteNoise();
    ~WhiteNoise();
    
    void processSample (float& sample); // write random noise to output buffer
    double gain = 0.2; //gain for whitenoise
    
private:
    int counter = 0; //counter used in generateNoiseSample()
    double generateNoiseSample(); //generate random noise
};

class DisplayAudioWaveForm : public juce::Component
{
public:
    DisplayAudioWaveForm();
    ~DisplayAudioWaveForm() override;
    void addAudioData(const juce::AudioBuffer<float>& buffer, int startSample, int numSamples);
    void paint(juce::Graphics& g) override;
    void resized() override;
    void paintColor (); //I add a new paintColor() function to draw color to the window
    
private:
    juce::AudioVisualiserComponent audioVisualiser;
};
