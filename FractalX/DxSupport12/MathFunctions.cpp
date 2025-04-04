#include "pch.h"
#include "MathFunctions.h"

namespace DXF::MathFunctions
{
	double sec(double v)
	{
		auto c = cos(v);
		if (c == 0)
			return std::numeric_limits<double>::max();

		return 1 / c;
	}
}
