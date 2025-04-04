#pragma once

namespace DXF
{
	enum class FractalType
	{
		StandardBulb = 1,
		DoubleBulb = 2,
		InglesFractal = 3,
		InglesFractal2 = 4,
		InglesFractal3 = 5,
		Quaternion = 6,
		QuaternionGeneral = 7
	};

	int FractalTypeToInt(FractalType fractalType);

	FractalType FractalTypeFromInt(int type);

	CString FractalTypeString(FractalType fractalType);
}
