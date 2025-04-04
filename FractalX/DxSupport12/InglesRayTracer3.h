#pragma once

#include "IRayTracer.h"

namespace DXF
{
	struct ModelData;

	std::unique_ptr<IRayTracer> CreateInglesRayTracer3(const TraceParams& traceParams, const ModelData& modelData);
}
