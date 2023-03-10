/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "parser/FileParser.h"
#include "parser/FrameProducer.h"

//==============================================================================
OscirenderAudioProcessor::OscirenderAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
    , producer(std::make_unique<FrameProducer>(*this, parser)) {
    producer->startThread();
}

OscirenderAudioProcessor::~OscirenderAudioProcessor()
{
}

//==============================================================================
const juce::String OscirenderAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool OscirenderAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool OscirenderAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool OscirenderAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double OscirenderAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int OscirenderAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int OscirenderAudioProcessor::getCurrentProgram()
{
    return 0;
}

void OscirenderAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String OscirenderAudioProcessor::getProgramName (int index)
{
    return {};
}

void OscirenderAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void OscirenderAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	currentSampleRate = sampleRate;
    updateAngleDelta();
}

void OscirenderAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool OscirenderAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void OscirenderAudioProcessor::updateAngleDelta() {
	auto cyclesPerSample = frequency / currentSampleRate;
	thetaDelta = cyclesPerSample * 2.0 * juce::MathConstants<double>::pi;
}

void OscirenderAudioProcessor::addFrame(std::vector<std::unique_ptr<Shape>> frame) {
    const auto scope = frameFifo.write(1);

    if (scope.blockSize1 > 0) {
        frameBuffer[scope.startIndex1].clear();
        for (auto& shape : frame) {
            frameBuffer[scope.startIndex1].push_back(std::move(shape));
        }
    }

    if (scope.blockSize2 > 0) {
        frameBuffer[scope.startIndex2].clear();
        for (auto& shape : frame) {
            frameBuffer[scope.startIndex2].push_back(std::move(shape));
        }
    }
}

void OscirenderAudioProcessor::updateFrame() {
    currentShape = 0;
    shapeDrawn = 0.0;
    frameDrawn = 0.0;
    
	if (frameFifo.getNumReady() > 0) {
        {
            const auto scope = frameFifo.read(1);

            if (scope.blockSize1 > 0) {
				frame.swap(frameBuffer[scope.startIndex1]);
            } else if (scope.blockSize2 > 0) {
                frame.swap(frameBuffer[scope.startIndex2]);
            }

            frameLength = Shape::totalLength(frame);
        }
	}
}

void OscirenderAudioProcessor::updateLengthIncrement() {
    lengthIncrement = std::max(frameLength / (currentSampleRate / frequency), MIN_LENGTH_INCREMENT);
}

void OscirenderAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    
    auto* channelData = buffer.getArrayOfWritePointers();
	auto numSamples = buffer.getNumSamples();
    
	for (auto sample = 0; sample < numSamples; ++sample) {
        updateLengthIncrement();

        double x = 0.0;
        double y = 0.0;
        double length = 0.0;

        if (currentShape < frame.size()) {
			
            auto& shape = frame[currentShape];
            length = shape->length();
            double drawingProgress = length == 0.0 ? 1 : shapeDrawn / length;
            Vector2 channels = shape->nextVector(drawingProgress);
			x = channels.x;
            y = channels.y;
        }

        if (totalNumOutputChannels >= 2) {
			channelData[0][sample] = x;
			channelData[1][sample] = y;
		} else if (totalNumOutputChannels == 1) {
            channelData[0][sample] = x;
        }
        
        // hard cap on how many times it can be over the length to
        // prevent audio stuttering
        frameDrawn += std::min(lengthIncrement, 20 * length);
		shapeDrawn += std::min(lengthIncrement, 20 * length);

        // Need to skip all shapes that the lengthIncrement draws over.
        // This is especially an issue when there are lots of small lines being
        // drawn.
		while (shapeDrawn > length) {
			shapeDrawn -= length;
			currentShape++;
			if (currentShape >= frame.size()) {
				currentShape = 0;
                break;
			}
            // POTENTIAL TODO: Think of a way to make this more efficient when iterating
            // this loop many times
            length = frame[currentShape]->len;
		}

        if (frameDrawn > frameLength) {
			updateFrame();
		}
	}

	juce::MidiBuffer processedMidi;
    
    for (const auto metadata : midiMessages) {
        auto message = metadata.getMessage();
        const auto time = metadata.samplePosition;
        
        if (message.isNoteOn()) {
            message = juce::MidiMessage::noteOn(message.getChannel(), message.getNoteNumber(), (juce::uint8)noteOnVel);
        }
        processedMidi.addEvent(message, time);
    }
	midiMessages.swapWith(processedMidi);
}

//==============================================================================
bool OscirenderAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* OscirenderAudioProcessor::createEditor()
{
    return new OscirenderAudioProcessorEditor (*this);
}

//==============================================================================
void OscirenderAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void OscirenderAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new OscirenderAudioProcessor();
}
