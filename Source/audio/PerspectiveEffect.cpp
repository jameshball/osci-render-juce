#include "PerspectiveEffect.h"
#include <numbers>
#include "../MathUtil.h"

const juce::String PerspectiveEffect::FILE_NAME = "6a3580b0-c5fc-4b28-a33e-e26a487f052f";

PerspectiveEffect::PerspectiveEffect(int versionHint, std::function<void(int, juce::String, juce::String)> errorCallback) : versionHint(versionHint), errorCallback(errorCallback) {
    fixedRotateX = new BooleanParameter("Perspective Fixed Rotate X", "perspectiveFixedRotateX", versionHint, false);
    fixedRotateY = new BooleanParameter("Perspective Fixed Rotate Y", "perspectiveFixedRotateY", versionHint, false);
    fixedRotateZ = new BooleanParameter("Perspective Fixed Rotate Z", "perspectiveFixedRotateZ", versionHint, false);
}

PerspectiveEffect::~PerspectiveEffect() {
	parser->close(L);
}

Vector2 PerspectiveEffect::apply(int index, Vector2 input, const std::vector<double>& values, double sampleRate) {
	auto effectScale = values[0];
	auto depth = 1.0 + (values[1] - 0.1) * 3;
	auto rotateSpeed = linearSpeedToActualSpeed(values[2]);
	double baseRotateX, baseRotateY, baseRotateZ;
	if (fixedRotateX->getBoolValue()) {
		baseRotateX = 0;
		currentRotateX = values[3] * std::numbers::pi;
	} else {
        baseRotateX = values[3] * std::numbers::pi;
    }
	if (fixedRotateY->getBoolValue()) {
		baseRotateY = 0;
        currentRotateY = values[4] * std::numbers::pi;
	} else {
        baseRotateY = values[4] * std::numbers::pi;
	}
	if (fixedRotateZ->getBoolValue()) {
        baseRotateZ = 0;
        currentRotateZ = values[5] * std::numbers::pi;
	} else {
        baseRotateZ = values[5] * std::numbers::pi;
    }

	currentRotateX = MathUtil::wrapAngle(currentRotateX + baseRotateX * rotateSpeed);
	currentRotateY = MathUtil::wrapAngle(currentRotateY + baseRotateY * rotateSpeed);
	currentRotateZ = MathUtil::wrapAngle(currentRotateZ + baseRotateZ * rotateSpeed);

	auto x = input.x;
	auto y = input.y;
	auto z = 0.0;

	{
		juce::SpinLock::ScopedLockType lock(codeLock);
		if (!defaultScript) {
			parser->setVariable("x", x);
			parser->setVariable("y", y);
			parser->setVariable("z", z);

			auto result = parser->run(L, LuaVariables{sampleRate, 0}, step, phase);
			if (result.size() >= 3) {
				x = result[0];
				y = result[1];
				z = result[2];
			}
		} else {
			parser->resetErrors();
		}
	}

	auto rotateX = baseRotateX + currentRotateX;
	auto rotateY = baseRotateY + currentRotateY;
	auto rotateZ = baseRotateZ + currentRotateZ;

	// rotate around x-axis
	double cosValue = std::cos(rotateX);
	double sinValue = std::sin(rotateX);
	double y2 = cosValue * y - sinValue * z;
	double z2 = sinValue * y + cosValue * z;

	// rotate around y-axis
	cosValue = std::cos(rotateY);
	sinValue = std::sin(rotateY);
	double x2 = cosValue * x + sinValue * z2;
	double z3 = -sinValue * x + cosValue * z2;

	// rotate around z-axis
	cosValue = cos(rotateZ);
	sinValue = sin(rotateZ);
	double x3 = cosValue * x2 - sinValue * y2;
	double y3 = sinValue * x2 + cosValue * y2;

	// perspective projection
	auto focalLength = 1.0;
	return Vector2(
		(1 - effectScale) * input.x + effectScale * (x3 * focalLength / (z3 - depth)),
		(1 - effectScale) * input.y + effectScale * (y3 * focalLength / (z3 - depth))
	);
}

void PerspectiveEffect::updateCode(const juce::String& newCode) {
	juce::SpinLock::ScopedLockType lock(codeLock);
	defaultScript = newCode == DEFAULT_SCRIPT;
    code = newCode;
	parser = std::make_unique<LuaParser>(FILE_NAME, code, errorCallback);
}

void PerspectiveEffect::setVariable(juce::String variableName, double value) {
	juce::SpinLock::ScopedLockType lock(codeLock);
	if (!defaultScript) {
        parser->setVariable(variableName, value);
    }
}

juce::String PerspectiveEffect::getCode() {
	juce::SpinLock::ScopedLockType lock(codeLock);
    return code;
}
