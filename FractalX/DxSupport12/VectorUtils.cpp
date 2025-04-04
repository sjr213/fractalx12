#include "pch.h"
#include "VectorUtils.h"

namespace DXF::VectorUtils
{
	double Dot(const Vector3Double& v)
	{
		return v.x * v.x + v.y * v.y + v.z * v.z;
	}

	double Dot(const Vector4Double& v)
	{
		return v.x * v.x + v.y * v.y + v.z * v.z + v.w * v.w;
	}

	Vector4Double operator-(double s, const Vector4Double& q)
	{
		return Vector4Double(s - q.x, s - q.y, s - q.z, s - q.w);
	}
}