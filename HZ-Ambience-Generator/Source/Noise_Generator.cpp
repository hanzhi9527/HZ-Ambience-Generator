/*
  ==============================================================================

    Noise_Generator.cpp
    Created: 4 Dec 2023 8:41:46pm
    Author:  Hanzhi Zhang

  ==============================================================================
*/

#include "Noise_Generator.h"
#include <cmath>
#include <vector>
#include <JuceHeader.h>

WhiteNoise::WhiteNoise()
{
    
}

WhiteNoise::~WhiteNoise()
{
    
}

void WhiteNoise::processSample(float &sample)
{
    sample = 0.1*gain*generateNoiseSample();
}

double WhiteNoise::generateNoiseSample()
{
    counter++;
    //generate random numbers
    double sample = juce::Random::getSystemRandom().nextFloat() *2.0f - 1.0f;
    //limit sample to 5 decimal points
    //sample = floor(sample*100000 +0.5) / 100000;
    
    return sample;
}


DisplayAudioWaveForm::DisplayAudioWaveForm():audioVisualiser(1)
{
    
    
    audioVisualiser.setBufferSize(1024);
    
    audioVisualiser.setSamplesPerBlock(256);

    audioVisualiser.setNumChannels(1);

    audioVisualiser.setColours(juce::Colour(0xBF000000), juce::Colours::white);

    addAndMakeVisible(audioVisualiser);
}

DisplayAudioWaveForm::~DisplayAudioWaveForm()
{
    
}

void DisplayAudioWaveForm::addAudioData(const juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    buffer.getArrayOfReadPointers();
    
    juce::AudioBuffer<float> tempBuffer = buffer;
    tempBuffer.applyGain(6.0);
    
    audioVisualiser.pushBuffer(tempBuffer);
}

void DisplayAudioWaveForm::paint(juce::Graphics& g)
{
//    g.fillAll(juce::Colour(0xFF000000));
//    g.fillAll(juce::Colour(0xBF000000));
//    g.fillAll(juce::Colour(0x7F000000));
}

void DisplayAudioWaveForm::resized()
{
    audioVisualiser.setBounds(getLocalBounds());
}

void DisplayAudioWaveForm::paintColor ()
{
    audioVisualiser.setColours(juce::Colour(0xFF000000), juce::Colours::white);
}
