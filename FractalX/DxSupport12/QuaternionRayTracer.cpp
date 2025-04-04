#include "pch.h"
#include "QuaternionRayTracer.h"

#include <algorithm>
#include <limits>
#include "MathFunctions.h"
#include "ModelData.h"
#include "NotSupportedException.h"
#include "RayTracerCommon.h"
#include "RayTracerDoubleCommon.h"
#include <SimpleMath.h>
#include "SphereApproximator.h"
#include "TraceParams.h"
#include "VertexData.h"
#include "Vector3Double.h"


using namespace DirectX;
using namespace DirectX::SimpleMath;

namespace DXF
{
	class QuaternionRayTracer : public IRayTracer
	{
	private:
		TraceParams m_traceParams;
		ModelData m_modelData;
		std::function<void(const Vector3Double&, const Vector3Double&, Vector3Double&, Vector3Double&, const DXF::Vertex<double>&)> m_calculateNextCycle;

	protected:

		void CalculateNextCycleQuaternion(const Vector3Double& Q, const Vector3Double& dQ, Vector3Double& Q1, Vector3Double& dQ1, const DXF::Vertex<double>& C)
		{		
			Q1.x = Q.x * Q.x - Q.y * Q.y - Q.z * Q.z + C.X;
			Q1.y = 2.0 * Q.x * Q.y + C.Y;
			Q1.z = 2.0 * Q.x * Q.z + C.Z;

			dQ1.x = 2.0 * (Q.x + Q.y + Q.z) * dQ.x;
			dQ1.y = 2.0 * (Q.x - Q.y) * dQ.y;
			dQ1.z = 2.0 * (Q.x - Q.z) * dQ.z;
		}

		double EstimateDistance(Vector3Double& pos)
		{
			Vector3Double z = pos;
			Vector3Double dz(1.0, 1.0, 1.0);
			double dr = 1.0;
			double r = 0.0;
			Vector3Double z1;
			Vector3Double dz1;
			DXF::Vertex<double> C = (m_traceParams.Fractal.Mandlelbrot) ? DXF::Vertex<double>(0.0, 0.0, 0.0) : m_traceParams.Fractal.ConstantC;

			if (m_traceParams.Fractal.NormalizationType == BulbNormalizeType::AltRoots)
			{
				r = z.Length(m_traceParams.Fractal.NormalizationRoot);
			}
			else
			{
				r = z.Length();
			}

			int i = 0;
			for (; i < m_traceParams.Bulb.Iterations; ++i)
			{
				if (r > m_traceParams.Fractal.Bailout)
					break;

				m_calculateNextCycle(z, dz, z1, dz1, C);

				if (IsNan(z1))
					break;

				if (m_traceParams.Fractal.NormalizationType == BulbNormalizeType::AltRoots)
				{
					r = z1.Length(m_traceParams.Fractal.NormalizationRoot);
					dr = dz1.Length(m_traceParams.Fractal.NormalizationRoot);
				}
				else
				{
					r = z1.Length();
					dr = dz1.Length();
				}

				z = z1;
				dz = dz1;

				z += pos;
			}

			if (r == 0.0 || dr == 0.0)
				return 0.0;

			return (log(r) * r) / dr;
		}

		void SetNextCycle()
		{
			m_calculateNextCycle = [&](const Vector3Double& Q, const Vector3Double& dQ, Vector3Double& Q1, Vector3Double& dQ1, const Vertex<double>& C)
			{
				this->CalculateNextCycleQuaternion(Q, dQ, Q1, dQ1, C);
			};
		}

	public:

		QuaternionRayTracer(const TraceParams& traceParams, const ModelData& modelData)
			: m_traceParams(traceParams)
			, m_modelData(modelData)
		{
			SetNextCycle();
		}

		virtual ~QuaternionRayTracer()
		{}

		std::shared_ptr<DxVertexData> RayTrace(const TriangleData& data, const std::function<void(double)>& setProgress) override
		{
			auto tracer = CreateInternalDoubleRayTracer(m_traceParams, m_modelData, 
				[this](Vector3Double& pos)
				{
					return EstimateDistance(pos);
				});

			return tracer->RayTrace(data, setProgress);
		}

		std::shared_ptr<DxVertexData> RayTraceStretch(const TriangleData& data, const std::function<void(double)>& setProgress) override
		{
			auto tracer = CreateInternalDoubleRayTracer(m_traceParams, m_modelData, 
				[this](Vector3Double& pos)
				{
					return EstimateDistance(pos);
				});

			return tracer->RayTraceStretch(data, setProgress);
		}

	};

	std::unique_ptr<IRayTracer> CreateQuaternionRayTracer(const TraceParams& traceParams, const ModelData& modelData)
	{
		return std::make_unique<QuaternionRayTracer>(traceParams, modelData);
	}
}