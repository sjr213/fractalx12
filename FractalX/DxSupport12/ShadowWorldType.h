#pragma once

namespace DXF
{
	enum class ShadowWorldType
	{
		MainModel = 1,
		Background = 2
	};

	int ShadowWorldTypeToInt(ShadowWorldType worldType);

	ShadowWorldType ShadowWorldTypeFromInt(int type);

	CString ShadowWorldTypeString(ShadowWorldType worldType);
}