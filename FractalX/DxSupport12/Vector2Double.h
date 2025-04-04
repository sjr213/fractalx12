#pragma once
#include <SimpleMath.h>
#include "Vertex.h"

namespace DXF
{
	struct Vector2Double
	{
		double x;
		double y;

		Vector2Double();

		Vector2Double(double q);

		Vector2Double(double x, double y);

		Vector2Double(const Vector2Double& v);

		virtual ~Vector2Double() = default;

		bool operator==(const Vector2Double& v) const;
		bool operator!=(const Vector2Double& v) const;

		Vector2Double& operator=(const Vector2Double& v);

		Vector2Double& operator+=(const Vector2Double& v);

		Vector2Double& operator*=(const Vector2Double& v);

		Vector2Double& operator+=(double d);

		Vector2Double& operator*=(double d);

		double Length() const;

		double Length(double root) const;

		Vector2Double Normalize() const;

		double Dot() const;

		friend Vector2Double operator+(const Vector2Double& lf, const Vector2Double& rt);

		friend Vector2Double operator*(const Vector2Double& lf, const Vector2Double& rt);

		friend Vector2Double operator*(const Vector2Double& lf, double d);

		friend Vector2Double operator*(double d, const Vector2Double& rt);

		friend Vector2Double Min(const Vector2Double& lf, const Vector2Double& rt);
	};

}
