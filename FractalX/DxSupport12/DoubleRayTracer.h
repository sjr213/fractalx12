#pragma once

#include "IRayTracer.h"

namespace DXF
{
	struct ModelData;

	std::unique_ptr<IRayTracer> CreateDoubleRayTracer(const TraceParams& traceParams, const ModelData& modelData);
}