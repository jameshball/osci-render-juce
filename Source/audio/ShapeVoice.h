#pragma once
#include <JuceHeader.h>
#include "ShapeSound.h"

class OscirenderAudioProcessor;
class ShapeVoice : public juce::SynthesiserVoice {
public:
	ShapeVoice(OscirenderAudioProcessor& p);

	bool canPlaySound(juce::SynthesiserSound* sound) override;
	void startNote(int midiNoteNumber, float velocity, juce::SynthesiserSound* sound, int currentPitchWheelPosition) override;
    void updateSound(juce::SynthesiserSound* sound);
	void renderNextBlock(juce::AudioSampleBuffer& outputBuffer, int startSample, int numSamples) override;
	void stopNote(float velocity, bool allowTailOff) override;
	void pitchWheelMoved(int newPitchWheelValue) override;
	void controllerMoved(int controllerNumber, int newControllerValue) override;

	void incrementShapeDrawing();

private:
	const double MIN_TRACE = 0.005;
	const double MIN_LENGTH_INCREMENT = 0.000001;

	OscirenderAudioProcessor& audioProcessor;
	std::vector<std::unique_ptr<Shape>> frame;
	std::atomic<ShapeSound*> sound = nullptr;
	double actualTraceMin;
	double actualTraceMax;

	double frameLength = 0.0;
	int currentShape = 0;
	double shapeDrawn = 0.0;
	double frameDrawn = 0.0;
	double lengthIncrement = 0.0;

    bool currentlyPlaying = false;
	double tailOff = 0.0;
	double frequency = 1.0;
};