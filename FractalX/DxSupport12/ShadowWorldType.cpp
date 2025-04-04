#include "pch.h"
#include "ShadowWorldType.h"

#include <assert.h>

namespace DXF
{
	int ShadowWorldTypeToInt(ShadowWorldType worldType)
	{
		switch (worldType)
		{
		case ShadowWorldType::MainModel:
			return 1;
		case ShadowWorldType::Background:
			return 2;
		default:
			assert(false);
			return 1;
		}
	}

	ShadowWorldType ShadowWorldTypeFromInt(int type)
	{
		switch (type)
		{
		case 1:
			return ShadowWorldType::MainModel;
		case 2:
			return ShadowWorldType::Background;
		default:
			assert(false);
			return ShadowWorldType::MainModel;
		}
	}

	CString ShadowWorldTypeString(ShadowWorldType worldType)
	{
		switch (worldType)
		{
		case ShadowWorldType::MainModel:
			return _T("Main Model");
		case ShadowWorldType::Background:
			return _T("Background Model");
		default:
			assert(false);
			return _T("Unknown");
		}
	}
}