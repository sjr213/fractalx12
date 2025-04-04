#pragma once

#include "Vector3Double.h"
#include "Vector4Double.h"

namespace DXF::VectorUtils
{
	double Dot(const Vector3Double& v);

	double Dot(const Vector4Double& v);

	Vector4Double operator-(double s, const Vector4Double& q);
}