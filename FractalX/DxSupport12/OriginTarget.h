#pragma once

namespace DXF
{
	enum class OriginTarget
	{
		Point = 1,
		Parallel = 2,
	};

	int OriginTargetToInt(OriginTarget originTarget);

	OriginTarget OriginTargetFromInt(int type);

	CString OriginTargetString(OriginTarget originTarget);

	int GetIndexForOriginTarget(OriginTarget originTarget);

	OriginTarget GetOriginTargetForIndex(int index);
}