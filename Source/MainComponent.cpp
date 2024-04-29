#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent()
{
    addAndMakeVisible(levelSlider);
    levelSlider.setRange(0.0, 0.125);
    levelSlider.setValue ((double) currentLevel, juce::dontSendNotification);
    levelSlider.onValueChange = [this]
    {
        targetLevel = (float) levelSlider.getValue();
    };
    
    addAndMakeVisible(frequencySlider);
    frequencySlider.setRange(50.0, 5000.0);
    frequencySlider.setSkewFactorFromMidPoint(500.0);
    frequencySlider.setValue (currentFrequency, juce::dontSendNotification);
    frequencySlider.onValueChange = [this]
    {
        targetFrequency = frequencySlider.getValue();
    };
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (600, 100);

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
        setAudioChannels (0, 2);
    }
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    currentSampleRate = sampleRate;
    updateAngleDelta();
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    auto* leftBuffer = bufferToFill.buffer->getWritePointer (0, bufferToFill.startSample);
    auto* rightBuffer = bufferToFill.buffer->getWritePointer(1, bufferToFill.startSample);
    auto localTargetFrequency = targetFrequency;
    
    if (localTargetFrequency != currentFrequency)
    {
        auto frequencyIncrement = (localTargetFrequency - currentFrequency) / bufferToFill.numSamples;
        for (auto sample = 0; sample < bufferToFill.numSamples; ++sample)
        {
            
            auto currentSample = (float) std::sin (currentAngle);
            currentFrequency += frequencyIncrement;
            updateAngleDelta();
            
            currentAngle += angleDelta;
            leftBuffer[sample] = currentSample * currentLevel;
            rightBuffer[sample] = currentSample * currentLevel;
        }
        currentFrequency = localTargetFrequency;
    }
    else
    {
        for(auto sample = 0; sample < bufferToFill.numSamples; ++sample){
            auto currentSample = (float) std::sin(currentAngle);
            currentAngle += angleDelta;
            leftBuffer[sample] = currentSample *currentLevel;
            rightBuffer[sample] = currentSample *currentLevel;
        }
    }
    
    auto localTargetLevel = targetLevel;
            bufferToFill.buffer->applyGainRamp (bufferToFill.startSample, bufferToFill.numSamples, currentLevel, localTargetLevel);
    currentLevel = localTargetLevel;
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
}

void MainComponent::resized()
{
    frequencySlider.setBounds(10,10, getWidth() - 20, 20);
    levelSlider.setBounds(10,40, getWidth() - 20, 20);
}

void MainComponent::updateAngleDelta()
{
    auto cyclesPerSample = frequencySlider.getValue() / currentSampleRate;
    angleDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;
}
