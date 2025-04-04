#include "pch.h"
#include "QuaternionEquationType.h"

#include <assert.h>

namespace DXF
{
	int QuaternionEquationTypeToInt(QuaternionEquationType quatType)
	{
		switch (quatType)
		{
		case QuaternionEquationType::Q_Squared:
			return 1;
		case QuaternionEquationType::Q_Cubed:
			return 2;
		case QuaternionEquationType::Q_Ingles:
			return 3;
		default:
			assert(false);
			return 1;
		}
	}

	QuaternionEquationType QuaternionEquationTypeFromInt(int type)
	{
		switch (type)
		{
		case 1:
			return QuaternionEquationType::Q_Squared;
		case 2:
			return QuaternionEquationType::Q_Cubed;
		case 3:
			return QuaternionEquationType::Q_Ingles;
		default:
			assert(false);
			return QuaternionEquationType::Q_Squared;
		}
	}

	CString QuaternionEquationTypeString(QuaternionEquationType quatType)
	{
		switch (quatType)
		{
		case QuaternionEquationType::Q_Squared:
			return _T("Quaternion Squared");
		case QuaternionEquationType::Q_Cubed:
			return _T("Quaternion Cubed");
		case QuaternionEquationType::Q_Ingles:
			return _T("Quaternion Ingles");
		default:
			assert(false);
			return _T("Unknown");
		}
	}
}