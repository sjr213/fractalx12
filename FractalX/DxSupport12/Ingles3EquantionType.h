#pragma once

namespace DXF
{
	enum class Ingles3EquationType
	{
		I_Squared = 1,
		I_Cubed = 2,
		I_SinX = 3,
		I_Alt1 = 4,
		I_Alt2 = 5,
		I_Alt3 = 6,
		I_Alt4 = 7,
		I_Alt5 = 8,
		I_CubedCorrect = 9,
		I_Test1 = 10,
		I_Test2 = 11,
		I_Squared_Split_Derivative = 12
	};

	int Ingles3EquationTypeToInt(Ingles3EquationType ingles3Type);

	Ingles3EquationType Ingles3EquationTypeFromInt(int type);

	CString Ingles3EquationTypeString(Ingles3EquationType ingles3Type);
}