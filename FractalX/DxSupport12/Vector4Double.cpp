#include "pch.h"
#include "Vector4Double.h"

namespace DXF
{
	Vector4Double::Vector4Double()
		: x(0)
		, y(0)
		, z(0)
		, w(0)
	{}

	Vector4Double::Vector4Double(double x, double y, double z, double w)
		: x(x)
		, y(y)
		, z(z)
		, w(w)
	{}

	Vector4Double::Vector4Double(const Vector4Double& v)
		: x(v.x)
		, y(v.y)
		, z(v.z)
		, w(v.w)
	{}

	Vector4Double::Vector4Double(const DirectX::SimpleMath::Vector3 v)
		: x(v.x)
		, y(v.y)
		, z(v.z)
		, w(0)
	{}

	Vector4Double::Vector4Double(const DirectX::XMFLOAT3& v)
		: x(v.x)
		, y(v.y)
		, z(v.z)
		, w(0)
	{}

	Vector4Double::Vector4Double(double x, const Vector3Double& yzw)
		: x(x)
		, y(yzw.x)
		, z(yzw.y)
		, w(yzw.z)
	{}

	Vector4Double::Vector4Double(const Vector3Double& xyz, double w)
		: x(xyz.x)
		, y(xyz.y)
		, z(xyz.z)
		, w(w)
	{}

	Vector4Double::~Vector4Double()
	{}

	Vector4Double& Vector4Double::operator=(const Vector4Double& v)
	{
		if (*this == v)
			return *this;

		x = v.x;
		y = v.y;
		z = v.z;
		w = v.w;

		return *this;
	}

	Vector4Double& Vector4Double::operator=(const DirectX::SimpleMath::Vector3& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
		w = 0;

		return *this;
	}

	DirectX::SimpleMath::Vector4 Vector4Double::ToVector4()
	{
		return DirectX::SimpleMath::Vector4(static_cast<float>(x), static_cast<float>(y), static_cast<float>(z), static_cast<float>(w));
	}

	bool Vector4Double::operator==(const Vector4Double& v) const
	{
		return v.x == x && v.y == y && v.z == z && v.w == w;
	}

	bool Vector4Double::operator!=(const Vector4Double& v) const
	{
		return !(*this == v);
	}

	double Vector4Double::Length() const
	{
		return sqrt(x * x + y * y + z * z + w * w);
	}

	double Vector4Double::Length(double root) const
	{
		return pow(x * x + y * y + z * z + w * w, root);
	}

	Vector4Double Vector4Double::Normalize() const
	{
		auto len = Length();
		if (len == 0.0)
			return Vector4Double();

		return Vector4Double(x / len, y / len, z / len, w/len );
	}

	Vector4Double& Vector4Double::operator+=(const Vector4Double& v)
	{
		x += v.x;
		y += v.y;
		z += v.z;
		w += v.w;

		return *this;
	}

	Vector4Double& Vector4Double::operator*=(const Vector4Double& v)
	{
		x *= v.x;
		y *= v.y;
		z *= v.z;
		w *= v.w;

		return *this;
	}

	Vector4Double& Vector4Double::operator+=(double d)
	{
		x += d;
		y += d;
		z += d;
		w += d;

		return *this;
	}

	Vector4Double& Vector4Double::operator*=(double d)
	{
		x *= d;
		y *= d;
		z *= d;
		w *= d;

		return *this;
	}

	Vector3Double Vector4Double::YZW() const
	{
		return Vector3Double(y, z, w);
	}

	Vector3Double Vector4Double::XYZ() const
	{
		return Vector3Double(x, y, z);
	}

	double Vector4Double::Dot() const
	{
		return x * x + y * y + z * z + w * w;
	}

	// free functions
	///////////////////////////////////////////////////////////////////////////

	Vector4Double operator+(const Vector4Double& lf, const Vector4Double& rt)
	{
		Vector4Double temp = lf;
		temp += rt;
		return temp;
	}

	Vector4Double operator*(const Vector4Double& lf, const Vector4Double& rt)
	{
		Vector4Double temp = lf;
		temp *= rt;
		return temp;
	}

	Vector4Double operator*(const Vector4Double& lf, double d)
	{
		Vector4Double temp = lf;
		temp *= d;
		return temp;
	}

	Vector4Double operator*(double d, const Vector4Double& rt)
	{
		Vector4Double temp = rt;
		temp *= d;
		return temp;
	}

	Vector4Double operator+(const Vector4Double& lf, const Vertex<double>& rt)
	{
		Vector4Double temp = lf;
		temp.x += rt.X;
		temp.y += rt.Y;
		temp.z += rt.Z;

		return temp;
	}

}