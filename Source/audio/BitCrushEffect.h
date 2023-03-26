#pragma once
#include "Effect.h"
#include "../shape/Vector2.h"

class BitCrushEffect : public Effect {
public:
	BitCrushEffect();
	~BitCrushEffect();

	Vector2 apply(int index, Vector2 input) override;
	void setValue(double value) override;
	void setFrequency(double frequency) override;
	int getPrecedence() override;
	void setPrecedence(int precedence) override;
private:

	double value = 0.0;
	double frequency = 1.0;
	int precedence = -1;

	double bitCrush(double value, double places);
};