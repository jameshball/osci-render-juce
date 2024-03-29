#pragma once
#include "../shape/Vector2.h"
#include <JuceHeader.h>

class EffectApplication {
public:
	EffectApplication() {};

	virtual Vector2 apply(int index, Vector2 input, const std::vector<double>& values, double sampleRate) = 0;
	
	void resetPhase();
	double nextPhase(double frequency, double sampleRate);
private:
	double phase = 0.0;
};