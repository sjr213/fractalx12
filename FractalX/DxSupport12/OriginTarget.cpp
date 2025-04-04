#include "pch.h"
#include "OriginTarget.h"

#include <assert.h>

namespace DXF
{
	int OriginTargetToInt(OriginTarget originTarget)
	{
		switch (originTarget)
		{
		case OriginTarget::Point:
			return 1;
		case OriginTarget::Parallel:
			return 2;
		default:
			assert(false);
			return 1;
		}
	}

	OriginTarget OriginTargetFromInt(int type)
	{
		switch (type)
		{
		case 1:
			return OriginTarget::Point;
		case 2:
			return OriginTarget::Parallel;
		default:
			assert(false);
			return OriginTarget::Point;
		}
	}

	CString OriginTargetString(OriginTarget originTarget)
	{
		switch (originTarget)
		{
		case OriginTarget::Point:
			return _T("Point");
		case OriginTarget::Parallel:
			return _T("Parallel");
		default:
			assert(false);
			return _T("Unknown");
		}
	}

	int GetIndexForOriginTarget(OriginTarget originTarget)
	{
		switch (originTarget)
		{
		case OriginTarget::Point:
			return 0;
		case OriginTarget::Parallel:
			return 1;
		default:
			assert(false);
			return 0;
		}
	}

	OriginTarget GetOriginTargetForIndex(int index)
	{
		switch (index)
		{
		case 0:
			return OriginTarget::Point;
		case 1:
			return OriginTarget::Parallel;
		default:
			assert(false);
			return OriginTarget::Point;
		}
	}
}