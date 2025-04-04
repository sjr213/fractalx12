#pragma once
#include <SimpleMath.h>
#include "Vertex.h"

namespace DXF
{
	struct Vector3Double
	{
		double x;
		double y;
		double z;

		Vector3Double();

		Vector3Double(double x, double y, double z);

		Vector3Double(const Vector3Double& v);

		Vector3Double(const DirectX::SimpleMath::Vector3 v);

		Vector3Double(const DXF::Vertex<double>& v);

		Vector3Double(const DirectX::XMFLOAT3& v);

		// make one with vector3

		virtual ~Vector3Double();

		bool operator==(const Vector3Double& v) const;
		bool operator!=(const Vector3Double& v) const;

		Vector3Double& operator=(const Vector3Double& v);

		Vector3Double& operator=(const DirectX::SimpleMath::Vector3& v);

		Vector3Double& operator+=(const Vector3Double& v);

		Vector3Double& operator-=(const Vector3Double& v);

		Vector3Double& operator*=(const Vector3Double& v);

		Vector3Double& operator+=(double d);

		Vector3Double& operator-=(double d);

		Vector3Double& operator*=(double d);

		DirectX::SimpleMath::Vector3 ToVector3();

		double Length() const;

		double Length(double root) const;

		Vector3Double Normalize() const;

		double Dot() const;

		Vector3Double XYY() const;

		Vector3Double YXY() const;

		Vector3Double YYX() const;

		friend Vector3Double operator+(const Vector3Double& lf, const Vector3Double& rt);

		friend Vector3Double operator-(const Vector3Double& lf, const Vector3Double& rt);

		friend Vector3Double operator*(const Vector3Double& lf, const Vector3Double& rt);

		friend Vector3Double operator*(const Vector3Double& lf, double d);

		friend Vector3Double operator*(double d, const Vector3Double& rt);

		friend Vector3Double operator+(const Vector3Double& lf, const Vertex<double>& rt);
	};

	inline bool IsNan(Vector3Double p)
	{
		if (isnan(p.x) || isnan(p.y) || isnan(p.z))
			return true;

		return false;
	}

}