#pragma once

#include <JuceHeader.h>
#include "Noise_Generator.h"

//==============================================================================
/*
    This component lives inside our window, and this is where you should put all
    your controls and content.
*/


class MainComponent  : public juce::AudioAppComponent, /*public juce::ChangeListener,*/ public juce::Button::Listener, public juce::Slider::Listener, /*public juce::Timer,*/ public juce::ComboBox::Listener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;
    
    //==============================================================================
    //my own variables and functions
    void renderBuffer (juce::AudioBuffer<float>& buffer);
    
    juce::Slider gainSlider;
    double gainOfSlider;
    void sliderValueChanged(juce::Slider *slider) override;
    
    //for reverb
    juce::Slider r_roomSize_Slider, r_damping_Slider, r_wetLevel_Slider, r_dryLevel_Slider;
    juce::Reverb::Parameters reverbParam;

private:
    //==============================================================================
    // Your private member variables go here...
    
    //==============================================================================
    // my own variables and functions
    // set Buttons
    juce::TextButton playButton;
    juce::TextButton stopButton;
    void buttonClicked(juce::Button *button) override;
    
    noise_type type;
    
    // set up required object to handle audio
    juce::AudioTransportSource transportSource;
    
    WhiteNoise whiteNoise;
    
    int counter = 0;
    
    //used to read the binary data
    juce::AudioFormatManager formatManager;
    
    std::unique_ptr<juce::AudioFormatReader> reader1, reader2;
    
    juce::Image resizedImage, image1, image2, imagewn;
    juce::Reverb reverb;
    
    
    double ratio_sr;
    
    juce::AudioBuffer<float> buffer1, buffer2, buffer1b, buffer2b;
    
    juce::LagrangeInterpolator resampler1, resampler2;
    
    //comboBox used to trigger different enums
    juce::ComboBox comboBox;
    void comboBoxChanged (juce::ComboBox* box) override; //override the function in juce::ComboBox::Listener

    bool currentlyPlaying = false; //the flag used to check if the audio is currently playing
    
    juce::Reverb reverb1;
    
    DisplayAudioWaveForm waveForm;
    
    juce::Label r_roomSize_Label, r_damping_Label, r_wetLevel_Label, r_dryLevel_Label, gainLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
