#pragma once

#include "Core/Math/HorizonMathCommon.h"

namespace HE
{
struct Rect
{
	Vector2 minimum;
	Vector2 maximum;

	Rect() : minimum(0.0f, 0.0f), maximum(0.0f, 0.0f) {}
	Rect(float x0, float y0, float x1, float y1) : minimum(x0, y0), maximum(x1, y1) {}
	Rect(Vector2 min, Vector2 max) : minimum(min), maximum(max) {}

	float GetWidth() const { return maximum.x - minimum.x; }
	float GetHeight() const { return maximum.y - minimum.y; }
	Vector2 GetSize() const { return Vector2(maximum.x - minimum.x, maximum.y - minimum.y); }
};
}