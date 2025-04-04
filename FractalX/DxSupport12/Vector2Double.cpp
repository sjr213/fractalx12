#include "pch.h"
#include "Vector2Double.h"


namespace DXF
{
	Vector2Double::Vector2Double()
		: x(0)
		, y(0)
	{}

	Vector2Double::Vector2Double(double q)
		: x(q)
		, y(q)
	{}

	Vector2Double::Vector2Double(double x, double y)
		: x(x)
		, y(y)
	{}

	Vector2Double::Vector2Double(const Vector2Double& v)
		: x(v.x)
		, y(v.y)
	{}

	Vector2Double& Vector2Double::operator=(const Vector2Double& v)
	{
		if (*this == v)
			return *this;

		x = v.x;
		y = v.y;

		return *this;
	}

	bool Vector2Double::operator==(const Vector2Double& v) const
	{
		return v.x == x && v.y == y;
	}

	bool Vector2Double::operator!=(const Vector2Double& v) const
	{
		return !(*this == v);
	}

	double Vector2Double::Length() const
	{
		return sqrt(x * x + y * y);
	}

	double Vector2Double::Length(double root) const
	{
		return pow(x * x + y * y, root);
	}

	Vector2Double Vector2Double::Normalize() const
	{
		auto len = Length();
		if (len == 0.0)
			return Vector2Double();

		return Vector2Double(x / len, y / len);
	}

	Vector2Double& Vector2Double::operator+=(const Vector2Double& v)
	{
		x += v.x;
		y += v.y;

		return *this;
	}

	Vector2Double& Vector2Double::operator*=(const Vector2Double& v)
	{
		x *= v.x;
		y *= v.y;

		return *this;
	}

	Vector2Double& Vector2Double::operator+=(double d)
	{
		x += d;
		y += d;

		return *this;
	}

	Vector2Double& Vector2Double::operator*=(double d)
	{
		x *= d;
		y *= d;

		return *this;
	}

	double Vector2Double::Dot() const
	{
		return x * x + y * y;
	}

	// free functions
	///////////////////////////////////////////////////////////////////////////

	Vector2Double operator+(const Vector2Double& lf, const Vector2Double& rt)
	{
		Vector2Double temp = lf;
		temp += rt;
		return temp;
	}

	Vector2Double operator*(const Vector2Double& lf, const Vector2Double& rt)
	{
		Vector2Double temp = lf;
		temp *= rt;
		return temp;
	}

	Vector2Double operator*(const Vector2Double& lf, double d)
	{
		Vector2Double temp = lf;
		temp *= d;
		return temp;
	}

	Vector2Double operator*(double d, const Vector2Double& rt)
	{
		Vector2Double temp = rt;
		temp *= d;
		return temp;
	}

	Vector2Double Min(const Vector2Double& lf, const Vector2Double& rt)
	{
		if (lf.x < rt.x)
			return lf;

		if (lf.x > rt.x)
			return rt;

		if (lf.y > rt.y)
			return rt;

		return lf;
	}
}