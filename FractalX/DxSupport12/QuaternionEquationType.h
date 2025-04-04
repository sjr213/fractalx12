#pragma once

namespace DXF
{
	enum class QuaternionEquationType
	{
		Q_Squared = 1,
		Q_Cubed = 2,
		Q_Ingles = 3
	};

	int QuaternionEquationTypeToInt(QuaternionEquationType quatType);

	QuaternionEquationType QuaternionEquationTypeFromInt(int type);

	CString QuaternionEquationTypeString(QuaternionEquationType quatType);
}