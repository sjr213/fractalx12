#pragma once

#include "IRayTracer.h"

namespace DXF
{
	struct ModelData;

	std::unique_ptr<IRayTracer> CreateQuaternionGeneralRayTracer(const TraceParams& traceParams, const ModelData& modelData);
}