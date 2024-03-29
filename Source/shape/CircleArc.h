#pragma once

#include "Shape.h"
#include "Vector2.h"

class CircleArc : public Shape {
public:
	CircleArc(double x, double y, double radiusX, double radiusY, double startAngle, double endAngle);

	Vector2 nextVector(double drawingProgress) override;
	void rotate(double theta) override;
	void scale(double x, double y) override;
	void translate(double x, double y) override;
	double length() override;
	std::unique_ptr<Shape> clone() override;
	std::string type() override;

	double x, y, radiusX, radiusY, startAngle, endAngle;
	
};