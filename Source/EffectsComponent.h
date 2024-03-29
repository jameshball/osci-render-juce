#pragma once

#include <JuceHeader.h>
#include "audio/BitCrushEffect.h"
#include "PluginProcessor.h"
#include "components/DraggableListBox.h"
#include "components/EffectsListComponent.h"

class OscirenderAudioProcessorEditor;
class EffectsComponent : public juce::GroupComponent, public juce::ChangeListener {
public:
	EffectsComponent(OscirenderAudioProcessor&, OscirenderAudioProcessorEditor&);
	~EffectsComponent() override;

	void resized() override;
	void changeListenerCallback(juce::ChangeBroadcaster* source) override;

private:
	OscirenderAudioProcessor& audioProcessor;

	// juce::TextButton addBtn;

	AudioEffectListBoxItemData itemData;
	EffectsListBoxModel listBoxModel;
	DraggableListBox listBox;
	
	EffectComponent frequency = EffectComponent(audioProcessor, *audioProcessor.frequencyEffect, false);

	JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(EffectsComponent)
};