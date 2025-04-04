#include "pch.h"
#include "Vector3Double.h"


namespace DXF
{
	Vector3Double::Vector3Double()
		: x(0)
		, y(0)
		, z(0)
	{}

	Vector3Double::Vector3Double(double x, double y, double z)
		: x(x)
		, y(y)
		, z(z)
	{}

	Vector3Double::Vector3Double(const Vector3Double& v)
		: x(v.x)
		, y(v.y)
		, z(v.z)
	{}

	Vector3Double::Vector3Double(const DirectX::SimpleMath::Vector3 v)
		: x(v.x)
		, y(v.y)
		, z(v.z)
	{}

	Vector3Double::Vector3Double(const DXF::Vertex<double>& v)
		: x(v.X)
		, y(v.Y)
		, z(v.Z)
	{}

	Vector3Double::Vector3Double(const DirectX::XMFLOAT3& v)
		: x(v.x)
		, y(v.y)
		, z(v.z)
	{}

	Vector3Double::~Vector3Double()
	{}

	Vector3Double& Vector3Double::operator=(const Vector3Double& v)
	{
		if (*this == v)
			return *this;

		x = v.x;
		y = v.y;
		z = v.z;

		return *this;
	}

	Vector3Double& Vector3Double::operator=(const DirectX::SimpleMath::Vector3& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;

		return *this;
	}

	DirectX::SimpleMath::Vector3 Vector3Double::ToVector3()
	{
		return DirectX::SimpleMath::Vector3(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z));
	}

	bool Vector3Double::operator==(const Vector3Double& v) const
	{
		return v.x == x && v.y == y && v.z == z;
	}

	bool Vector3Double::operator!=(const Vector3Double& v) const
	{
		return !(*this == v);
	}

	double Vector3Double::Length() const
	{
		return sqrt(x * x + y * y + z * z);
	}

	double Vector3Double::Length(double root) const
	{
		return pow(x * x + y * y + z * z, root);
	}

	Vector3Double Vector3Double::Normalize() const
	{
		auto len = Length();
		if (len == 0.0)
			return Vector3Double();

		return Vector3Double(x / len, y / len, z / len);
	}

	Vector3Double& Vector3Double::operator+=(const Vector3Double& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;

		return *this;
	}

	Vector3Double& Vector3Double::operator-=(const Vector3Double& v)
	{
		x -= v.x;
		y -= v.y;
		z -= v.z;

		return *this;
	}

	Vector3Double& Vector3Double::operator*=(const Vector3Double& v)
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;

		return *this;
	}

	Vector3Double& Vector3Double::operator+=(double d)
	{
		x += d;
		y += d;
		z += d;

		return *this;
	}

	Vector3Double& Vector3Double::operator-=(double d)
	{
		x -= d;
		y -= d;
		z -= d;

		return *this;
	}

	Vector3Double& Vector3Double::operator*=(double d)
	{
		x *= d;
		y *= d;
		z *= d;

		return *this;
	}

	double Vector3Double::Dot() const
	{
		return x * x + y * y + z * z;
	}

	Vector3Double Vector3Double::XYY() const
	{
		return Vector3Double(x, y, y);
	}

	Vector3Double Vector3Double::YXY() const
	{
		return Vector3Double(y, x, y);
	}

	Vector3Double Vector3Double::YYX() const
	{
		return Vector3Double(y, y, x);
	}

	// free functions
	///////////////////////////////////////////////////////////////////////////

	Vector3Double operator+(const Vector3Double& lf, const Vector3Double& rt)
	{
		Vector3Double temp = lf;
		temp += rt;
		return temp;
	}

	Vector3Double operator-(const Vector3Double& lf, const Vector3Double& rt)
	{
		Vector3Double temp = lf;
		temp -= rt;
		return temp;
	}

	Vector3Double operator*(const Vector3Double& lf, const Vector3Double& rt)
	{
		Vector3Double temp = lf;
		temp *= rt;
		return temp;
	}

	Vector3Double operator*(const Vector3Double& lf, double d)
	{
		Vector3Double temp = lf;
		temp *= d;
		return temp;
	}

	Vector3Double operator*(double d, const Vector3Double& rt)
	{
		Vector3Double temp = rt;
		temp *= d;
		return temp;
	}

	Vector3Double operator+(const Vector3Double& lf, const Vertex<double>& rt)
	{
		Vector3Double temp = lf;
		temp.x += rt.X;
		temp.y += rt.Y;
		temp.z += rt.Z;

		return temp;
	}

}