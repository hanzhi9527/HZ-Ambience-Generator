#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (800, 600);

    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (0, 2); // 0 input, mono signal for 2 output
    }
    
    //set up the play Button and the stop button
    addAndMakeVisible(playButton);
    playButton.setButtonText("Play");
    playButton.setEnabled(true);
    playButton.addListener(this);
    
    addAndMakeVisible(stopButton);
    stopButton.setButtonText("Stop");
    stopButton.setEnabled(true);
    stopButton.addListener(this);
    
    formatManager.registerBasicFormats(); //register basic formats including .wav
    
    //set up the comboBox
    addAndMakeVisible(comboBox);
    comboBox.addItem("White_Noise", 1);
    comboBox.addItem("Ambience_Forest", 2);
    comboBox.addItem("Ambience_Seashore", 3);
    comboBox.setSelectedId(2); //default item
    comboBox.setJustificationType(juce::Justification::centred);
    comboBox.addListener(this);
    
    //set up the enum
    type = BINARY1;
    
    //initialize the buffer size
    buffer1.setSize(1, BinaryData::Binary_File_1_mono_wavSize/2);
    buffer2.setSize(1, BinaryData::Binary_File_2_mono_wavSize/2);
//    DBG("wavsize" << BinaryData::Binary_File_1_mono_wavSize);
    
    //set up the lagrange interpolator
    resampler1.reset();
    resampler2.reset();
    
    //get the image from binary data
    image1 = juce::ImageCache::getFromMemory(BinaryData::Binary_Image_1_png, BinaryData::Binary_Image_1_pngSize);
    image2 = juce::ImageCache::getFromMemory(BinaryData::Binary_Image_2_png, BinaryData::Binary_Image_2_pngSize);
    imagewn = juce::ImageCache::getFromMemory(BinaryData::WhiteNoise_Image_png, BinaryData::WhiteNoise_Image_pngSize);
    
    //set up reader for the BinaryData
    reader1 = std::unique_ptr<juce::AudioFormatReader>(formatManager.createReaderFor(std::make_unique<juce::MemoryInputStream>(BinaryData::Binary_File_1_mono_wav, BinaryData::Binary_File_1_mono_wavSize, false)));
    reader2 = std::unique_ptr<juce::AudioFormatReader>(formatManager.createReaderFor(std::make_unique<juce::MemoryInputStream>(BinaryData::Binary_File_2_mono_wav, BinaryData::Binary_File_2_mono_wavSize, false)));
    
    //read the binary data to audio buffer
    bool result1 = reader1->read(&buffer1, 0, BinaryData::Binary_File_1_mono_wavSize/2, 0, true, true);
    bool result2 = reader2->read(&buffer2, 0, BinaryData::Binary_File_2_mono_wavSize/2, 0, true, true);
    
    //match sampling rate of different device
    auto *device = deviceManager.getCurrentAudioDevice();
    auto system_sr = device->getCurrentSampleRate();
    ratio_sr = 48000/system_sr;
    
    buffer1b.setSize(1, BinaryData::Binary_File_1_mono_wavSize/2*(system_sr/48000));
    buffer2b.setSize(1, BinaryData::Binary_File_2_mono_wavSize/2*(system_sr/48000));
    
    
    const float *inputdata1 = buffer1.getReadPointer(0, counter);
    const float *inputdata2 = buffer2.getReadPointer(0, counter);
    
    float *outputdata1 = buffer1b.getWritePointer(0, counter);
    float *outputdata2 = buffer2b.getWritePointer(0, counter);
    
    //resampling
    resampler1.processAdding(48000/system_sr, inputdata1, outputdata1, BinaryData::Binary_File_1_mono_wavSize/2*(system_sr/48000), 1);
    resampler2.processAdding(48000/system_sr, inputdata2, outputdata2, BinaryData::Binary_File_2_mono_wavSize/2*(system_sr/48000), 1);
    
    //reset the reverb
    reverb1.reset();
    
    //initialize sliders and waveform display
    addAndMakeVisible(gainSlider);
    gainSlider.setValue(0.5);
    gainSlider.setSliderStyle(juce::Slider::SliderStyle::LinearHorizontal);
    gainSlider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxLeft, 1, 50, 20);
    gainSlider.setRange(0, 1);
//    DBG("maxvalueslider: " << gainSlider.getMaximum());
    gainSlider.setNumDecimalPlacesToDisplay(2);
    gainSlider.addListener(this);
    
    addAndMakeVisible(r_roomSize_Slider);
    r_roomSize_Slider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    r_roomSize_Slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, 0, 50, 20);
    r_roomSize_Slider.setRange(0, 1);
    r_roomSize_Slider.setValue(0.5);
    r_roomSize_Slider.setNumDecimalPlacesToDisplay(2);
    r_roomSize_Slider.addListener(this);
    
    addAndMakeVisible(r_damping_Slider);
    r_damping_Slider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    r_damping_Slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, 0, 50, 20);
    r_damping_Slider.setRange(0, 1);
    r_damping_Slider.setValue(0);
    r_damping_Slider.setNumDecimalPlacesToDisplay(2);
    r_damping_Slider.addListener(this);
    
    addAndMakeVisible(r_wetLevel_Slider);
    r_wetLevel_Slider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    r_wetLevel_Slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, 0, 50, 20);
    r_wetLevel_Slider.setRange(0, 1);
    r_wetLevel_Slider.setValue(0);
    r_wetLevel_Slider.setNumDecimalPlacesToDisplay(2);
    r_wetLevel_Slider.addListener(this);
    
    addAndMakeVisible(r_dryLevel_Slider);
    r_dryLevel_Slider.setSliderStyle(juce::Slider::SliderStyle::Rotary);
    r_dryLevel_Slider.setTextBoxStyle(juce::Slider::TextEntryBoxPosition::TextBoxBelow, 0, 50, 20);
    r_dryLevel_Slider.setRange(0, 1);
    r_dryLevel_Slider.setValue(0.6);
    r_dryLevel_Slider.setNumDecimalPlacesToDisplay(2);
    r_dryLevel_Slider.addListener(this);

    addAndMakeVisible(waveForm);
    
    //initialize labels
    r_roomSize_Label.setText("Room Size", juce::dontSendNotification);
    r_roomSize_Label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(r_roomSize_Label);

    r_damping_Label.setText("Damping", juce::dontSendNotification);
    r_damping_Label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(r_damping_Label);

    r_wetLevel_Label.setText("Wet Level", juce::dontSendNotification);
    r_wetLevel_Label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(r_wetLevel_Label);

    r_dryLevel_Label.setText("Dry Level", juce::dontSendNotification);
    r_dryLevel_Label.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(r_dryLevel_Label);
    
    gainLabel.setText("Gain", juce::dontSendNotification);
    gainLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(gainLabel);
    
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
    
    //clear the buffer when closed
    buffer1.clear();
    buffer1b.clear();
    buffer2.clear();
    buffer2b.clear();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    // This function will be called when the audio device is started, or when
    // its settings (i.e. sample rate, block size, etc) are changed.

    // You can use this function to initialise any resources you might need,
    // but be careful - it will be called on the audio thread, not the GUI thread.

    // For more details, see the help for AudioProcessor::prepareToPlay()
    
    //transportSource.prepareToPlay(samplesPerBlockExpected, sampleRate);
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    // Your audio-processing code goes here!

    // For more details, see the help for AudioProcessor::getNextAudioBlock()

    // Right now we are not producing any data, in which case we need to clear the buffer
    // (to prevent the output of random noise)
    bufferToFill.clearActiveBufferRegion();
    
    //my code
    float *channeldata = bufferToFill.buffer->getWritePointer(0);
    float *channeldata_copy = bufferToFill.buffer->getWritePointer(1);
    
    // a barrier that only pass when the play button is clicked
    if (!currentlyPlaying)
        return;
    
    
    reverb1.setParameters(reverbParam); //refresh the reverb parameters every block
    
    if (type == WHITE_NOISE)
    {
        //write data to the output buffer
        renderBuffer(*bufferToFill.buffer);
    }
    
    else if (type == BINARY1)
    {
        for (int i = 0; i < bufferToFill.buffer->getNumSamples(); i++)
        {
            //DBG("cannot return sample");
            //write the data to the output buffer
            float wetSignal = buffer1b.getSample(0, counter)*gainOfSlider;
            reverb1.processMono(&wetSignal, 1);
            channeldata[i] = wetSignal;
            channeldata_copy[i] = channeldata[i];
            counter++;
            if (counter == BinaryData::Binary_File_1_mono_wavSize/2*(1/ratio_sr))
                counter = 0;
        }
  
    }
        
    else if (type == BINARY2)
    {
        for (int i = 0; i < bufferToFill.buffer->getNumSamples(); i++)
        {
            //write the data to the output buffer
            float wetSignal = buffer2b.getSample(0, counter)*gainOfSlider;
            reverb1.processMono(&wetSignal, 1);
            channeldata[i] = wetSignal;
            channeldata_copy[i] = channeldata[i];
            counter++;
            if (counter >= BinaryData::Binary_File_2_mono_wavSize)
                counter = 0;
        }
    }
    
    waveForm.addAudioData(*bufferToFill.buffer, 0, bufferToFill.numSamples); //display the waveform every block
}

void MainComponent::releaseResources()
{
    // This will be called when the audio device stops, or when it is being
    // restarted due to a setting change.

    // For more details, see the help for AudioProcessor::releaseResources()
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

    // You can add your drawing code here!
    //g.drawImageAt(resizedImage, 0, 0);
    
    //draw the image to output.
    
    //draw binary images
    if (type == BINARY1)
        g.drawImage(image1, getLocalBounds().toFloat());
    else if (type == BINARY2)
        g.drawImage(image2, getLocalBounds().toFloat());
    else if (type == WHITE_NOISE)
        g.drawImage(imagewn, getLocalBounds().toFloat());
    
    //set color of the GUI components
    comboBox.setColour(juce::ComboBox::ColourIds::backgroundColourId, juce::Colour(0xBF000000));
    playButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(0xBF000000));
    stopButton.setColour(juce::TextButton::ColourIds::buttonColourId, juce::Colour(0xBF000000));
    gainSlider.setColour(juce::Slider::ColourIds::backgroundColourId, juce::Colours::white);
    gainSlider.setColour(juce::Slider::ColourIds::textBoxBackgroundColourId, juce::Colour(0xBF000000));
    gainSlider.setColour(juce::Slider::ColourIds::thumbColourId, juce::Colours::blue);
    
    r_roomSize_Slider.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, juce::Colours::whitesmoke);
    r_roomSize_Slider.setColour(juce::Slider::ColourIds::textBoxBackgroundColourId, juce::Colour(0xBF000000));
    r_damping_Slider.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, juce::Colours::whitesmoke);
    r_damping_Slider.setColour(juce::Slider::ColourIds::textBoxBackgroundColourId, juce::Colour(0xBF000000));
    r_wetLevel_Slider.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, juce::Colours::whitesmoke);
    r_wetLevel_Slider.setColour(juce::Slider::ColourIds::textBoxBackgroundColourId, juce::Colour(0xBF000000));
    r_dryLevel_Slider.setColour(juce::Slider::ColourIds::rotarySliderOutlineColourId, juce::Colours::whitesmoke);
    r_dryLevel_Slider.setColour(juce::Slider::ColourIds::textBoxBackgroundColourId, juce::Colour(0xBF000000));
    
    r_roomSize_Label.setColour(juce::Label::ColourIds::backgroundColourId, juce::Colour(0xBF000000));
    r_damping_Label.setColour(juce::Label::ColourIds::backgroundColourId, juce::Colour(0xBF000000));
    r_wetLevel_Label.setColour(juce::Label::ColourIds::backgroundColourId, juce::Colour(0xBF000000));
    r_dryLevel_Label.setColour(juce::Label::ColourIds::backgroundColourId, juce::Colour(0xBF000000));
    gainLabel.setColour(juce::Label::ColourIds::backgroundColourId, juce::Colour(0xBF000000));
    
    waveForm.paintColor();
    
}

void MainComponent::resized()
{
    // This is called when the MainContentComponent is resized.
    // If you add any child components, this is where you should
    // update their positions.
    
    comboBox.setBounds(getRight()/10, getBottom()/5-getHeight()/30, getWidth()/5, getHeight()/15);
    playButton.setBounds(getRight()/10, 2*getBottom()/5-getHeight()/30, getWidth()/5, getHeight()/15);
    stopButton.setBounds(getRight()/10, 3*getBottom()/5-getHeight()/30, getWidth()/5, getHeight()/15);
    gainSlider.setBounds(getRight()/10, 4*getBottom()/5-getHeight()/30, getWidth()/5, getHeight()/15);
    
    r_roomSize_Slider.setBounds(getRight()/2, getBottom()/5-getHeight()/30 , getWidth()/6, getHeight()/6);
    r_damping_Slider.setBounds(getRight()*3/4, getBottom()/5-getHeight()/30 , getWidth()/6, getHeight()/6);
    r_wetLevel_Slider.setBounds(getRight()/2, 2*getBottom()/5-getHeight()/30 +getBottom()/25, getWidth()/6, getHeight()/6);
    r_dryLevel_Slider.setBounds(getRight()*3/4,2*getBottom()/5-getHeight()/30 +getBottom()/25, getWidth()/6, getHeight()/6);
    waveForm.setBounds(getRight()/2+getRight()/20, 3.5*getBottom()/5-getHeight()/30 +getBottom()/25, getWidth()/4+getWidth()/9, getHeight()/10);
    
    r_roomSize_Label.setBounds(r_roomSize_Slider.getX(), r_roomSize_Slider.getY() - 25, r_roomSize_Slider.getWidth(), 30);
    r_damping_Label.setBounds(r_damping_Slider.getX(), r_damping_Slider.getY() - 25, r_damping_Slider.getWidth(), 30);
    r_wetLevel_Label.setBounds(r_wetLevel_Slider.getX(), r_wetLevel_Slider.getY() - 25, r_wetLevel_Slider.getWidth(), 30);
    r_dryLevel_Label.setBounds(r_dryLevel_Slider.getX(), r_dryLevel_Slider.getY() - 25, r_dryLevel_Slider.getWidth(), 30);
    gainLabel.setBounds(gainSlider.getX(), gainSlider.getY()-25, gainSlider.getWidth()/4, 30);
}


//-----------------
// my own functions

void MainComponent::renderBuffer (juce::AudioBuffer<float>& buffer)
{

    float *samples = buffer.getWritePointer(0);
    float *samples_copy = buffer.getWritePointer(1);
    for (int j=0; j < buffer.getNumSamples(); j++)
    {
        // write the white noise to the output render buffer
        whiteNoise.processSample(samples[j]);
        whiteNoise.processSample(samples_copy[j]);
        
        samples[j] *= gainOfSlider;
        samples_copy[j] *= gainOfSlider;
        
        reverb1.processMono(&samples[j], 1);
        reverb1.processMono(&samples_copy[j], 1);
    }

}

void MainComponent::comboBoxChanged(juce::ComboBox* box)
{
    if (box == &comboBox)
    {
        int selectedID = box->getSelectedId();
        if (selectedID == 1)
            type = WHITE_NOISE;
        else if (selectedID == 2)
            type = BINARY1;
        else if (selectedID == 3)
            type = BINARY2;
        else ;
        
        repaint(); //call the paint() again, otherwise the image will only change when the window is resized.
    }
        
}

void MainComponent::buttonClicked(juce::Button *button)
{
    if (button == &playButton)
    {
        currentlyPlaying = true;
    }
    else if (button == &stopButton)
    {
        currentlyPlaying = false;
    }
}

void MainComponent::sliderValueChanged(juce::Slider *slider)
{
    if (slider == &gainSlider)
        gainOfSlider = slider->getValue();
    else if (slider == &r_roomSize_Slider)
        reverbParam.roomSize = slider->getValue();
    else if (slider == &r_damping_Slider)
        reverbParam.damping = slider->getValue();
    else if (slider == &r_wetLevel_Slider)
        reverbParam.wetLevel = slider->getValue();
    else if (slider == &r_dryLevel_Slider)
        reverbParam.dryLevel = slider->getValue();
}
