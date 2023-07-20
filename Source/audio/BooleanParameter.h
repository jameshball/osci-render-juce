#pragma once
#include "../shape/Vector2.h"
#include <JuceHeader.h>

class BooleanParameter : public juce::AudioProcessorParameter {
public:
	juce::String name;
	juce::String id;

	BooleanParameter(juce::String name, juce::String id, bool value) : name(name), id(id), value(value) {}

	// COPY CONSTRUCTOR SHOULD ONLY BE USED BEFORE
	// THE OBJECT IS USED IN MULTIPLE THREADS
	BooleanParameter(const BooleanParameter& other) {
        name = other.name;
        id = other.id;
        value.store(other.value.load());
    }

	juce::String getName(int maximumStringLength) const override {
		return name.substring(0, maximumStringLength);
	}

	juce::String getLabel() const override {
        return juce::String();
    }

	float getValue() const override {
		return value.load();
	}

	void setValue(float newValue) override {
		value.store(newValue >= 0.5f);
    }

	float getDefaultValue() const override {
        return false;
    }

	int getNumSteps() const override {
		return 2;
    }

	bool isDiscrete() const override {
        return true;
    }

	bool isBoolean() const override {
        return true;
    }

	bool isOrientationInverted() const override {
        return false;
    }

	juce::String getText(float value, int maximumStringLength) const override {
		juce::String string = value ? "true" : "false";
		return string.substring(0, maximumStringLength);
	}

	float getValueForText(const juce::String& text) const override {
		return text.length() > 0 && text.toLowerCase()[0] == 't';
    }

	bool isAutomatable() const override {
        return true;
    }

	bool isMetaParameter() const override {
        return false;
    }

	juce::AudioProcessorParameter::Category getCategory() const override {
        return juce::AudioProcessorParameter::genericParameter;
    }

private:
	std::atomic<bool> value = false;
};