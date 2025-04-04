#pragma once

#include "VertexData.h"

namespace DXF
{
	struct DxBackgroundVertexData
	{
		DxBackgroundVertexData()
			: VertexData(new DxVertexData)
		{}

		DxBackgroundVertexData(const DxVertexData& vData)
			: VertexData(std::make_shared<DxVertexData>(vData))
		{}

		DxBackgroundVertexData(const DxBackgroundVertexData& data)
		{
			*VertexData = *data.VertexData;
		}

		void operator=(const DxBackgroundVertexData& data)
		{
			*VertexData = *data.VertexData;
		}

		static const int BkgndVertexVersion = 1;
		std::shared_ptr<DxVertexData> VertexData;
	};
}
