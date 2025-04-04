#pragma once

#include "IRayTracer.h"

namespace DXF
{
	struct ModelData;

	std::unique_ptr<IRayTracer> CreateQuaternionExperimentalRayTracer(const TraceParams& traceParams, const ModelData& modelData);
}
