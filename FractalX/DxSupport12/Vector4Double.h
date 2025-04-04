#pragma once
#include <SimpleMath.h>
#include "Vertex.h"
#include "Vector3Double.h"

namespace DXF
{
	struct Vector4Double
	{
		double x;
		double y;
		double z;
		double w;

		Vector4Double();

		Vector4Double(double x, double y, double z, double w);

		Vector4Double(const Vector4Double& v);

		Vector4Double(const DirectX::SimpleMath::Vector3 v);

		Vector4Double(const DirectX::XMFLOAT3& v);

		Vector4Double(double x, const Vector3Double& yzw);

		Vector4Double(const Vector3Double& xyz, double w);

		// make one with vector3

		virtual ~Vector4Double();

		bool operator==(const Vector4Double& v) const;
		bool operator!=(const Vector4Double& v) const;

		Vector4Double& operator=(const Vector4Double& v);

		Vector4Double& operator=(const DirectX::SimpleMath::Vector3& v);

		Vector4Double& operator+=(const Vector4Double& v);

		Vector4Double& operator*=(const Vector4Double& v);

		Vector4Double& operator+=(double d);

		Vector4Double& operator*=(double d);

		DirectX::SimpleMath::Vector4 ToVector4();

		double Length() const;

		double Length(double root) const;

		Vector4Double Normalize() const;

		Vector3Double YZW() const;

		Vector3Double XYZ() const;

		double Dot() const;

		friend Vector4Double operator+(const Vector4Double& lf, const Vector4Double& rt);

		friend Vector4Double operator*(const Vector4Double& lf, const Vector4Double& rt);

		friend Vector4Double operator*(const Vector4Double& lf, double d);

		friend Vector4Double operator*(double d, const Vector4Double& rt);

		friend Vector4Double operator+(const Vector4Double& lf, const Vertex<double>& rt);
	};

	inline bool IsNan(Vector4Double p)
	{
		if (isnan(p.x) || isnan(p.y) || isnan(p.z) || isnan(p.w))
			return true;

		return false;
	}
}