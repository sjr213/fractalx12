#include "pch.h"
#include "QuaternionGeneralRayTracer.h"

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
#include "Vector4Double.h"
#include "VectorUtils.h"


using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace DXF::VectorUtils;

namespace DXF
{
	class QuaternionGeneralRayTracer : public IRayTracer
	{
	private:
		TraceParams m_traceParams;
		ModelData m_modelData;
		std::function<void(const Vector4Double&, Vector4Double&, Vector4Double&, const Vector4Double&)> m_calculateNextCycle;

	protected:

		static Vector4Double QuadSquareIngles(const Vector4Double& q)
		{
			return Vector4Double(q.x * q.x - q.YZW().Dot(), 2.0 * q.x * q.YZW());
		}

		// return a * ( 4.0*a.x*a.x - dot(a,a)*vec4(3.0,1.0,1.0,1.0) );
		static Vector4Double QuadCubeIngles(const Vector4Double& q)
		{
			return q * ( (4.0 * q.x * q.x) - (Dot(q) * Vector4Double(3.0, 1.0, 1.0, 1.0)));
		}

		static Vector4Double QuadSquare(const Vector4Double& q)
		{
			double x = q.x * q.x - q.y * q.y - q.z * q.z - q.w * q.w;
			double y = 2 * q.x * q.y;
			double z = 2 * q.x * q.z;
			double w = 2 * q.x * q.w;

			return Vector4Double(x, y, z, w);
		}

		static Vector4Double QuadCube(const Vector4Double& q)
		{
			double r = -1 * (q.x * q.x) - q.y * q.y - q.z * q.z - q.w * q.w;
			double x = q.x * q.x - 3* q.y * q.y - 3* q.z * q.z - 3* q.w * q.w;
			return Vector4Double(x, q.y*r, q.z*r, q.w*r);
		}

		void CalculateNextCycleInglesCubed(const Vector4Double& Q, Vector4Double& Q1, Vector4Double& dQ1, const Vector4Double& C)
		{
			Q1 = QuadCubeIngles(Q) + C;
			dQ1 = QuadSquareIngles(Q);
		}

		void CalculateNextCycleSquared(const Vector4Double& Q, Vector4Double& Q1, Vector4Double& dQ1, const Vector4Double& C)
		{
			dQ1 = Vector4Double(2, 2, 2, 2);	// 2*dQ where dq = (1,1,1,1) I think
			Q1 = QuadSquare(Q) + C;
		}

		void CalculateNextCycleCubed(const Vector4Double& Q, Vector4Double& Q1, Vector4Double& dQ1, const Vector4Double& C)
		{
			dQ1 = 3 * QuadSquare(Q);
			Q1 = QuadCube(Q) + C;
		}

		double EstimateDistance(const Vector3Double& pos)
		{
			Vector4Double z0(pos, 0.2);
			Vector4Double z = z0;
			double dr = 1.0;
			double r = 0.0;
			Vector4Double z1;
			Vector4Double dz1;
			DXF::Vector4Double C4 = (m_traceParams.Fractal.Mandlelbrot) ? 
				DXF::Vector4Double(0.0, 0.0, 0.0, 0.0) : Vector4Double(m_traceParams.Fractal.ConstantC, m_traceParams.Fractal.ConstantCW);


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

				m_calculateNextCycle(z, z1, dz1, C4);

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
				z += z0;
			}

			if (r == 0.0 || dr == 0.0)
				return 0.0;

			return (log(r) * r) / dr;
		}

		// looks like we might have been calculating dQ wrong by using the previous value of dQ
		void SetNextCycle()
		{
			switch (m_traceParams.Fractal.QuaternionEquation)
			{
			case QuaternionEquationType::Q_Squared:
			{
				m_calculateNextCycle = [&](const Vector4Double& Q, Vector4Double& Q1, Vector4Double& dQ1, const Vector4Double& C)
				{
					this->CalculateNextCycleSquared(Q, Q1, dQ1, C);
				};
				break;
			}
			case QuaternionEquationType::Q_Cubed:
			{
				m_calculateNextCycle = [&](const Vector4Double& Q, Vector4Double& Q1, Vector4Double& dQ1, const Vector4Double& C)
				{
					this->CalculateNextCycleCubed(Q, Q1, dQ1, C);
				};
				break;
			}
			case QuaternionEquationType::Q_Ingles:
			{
				m_calculateNextCycle = [&](const Vector4Double& Q, Vector4Double& Q1, Vector4Double& dQ1, const Vector4Double& C)
				{
					this->CalculateNextCycleInglesCubed(Q, Q1, dQ1, C);
				};
				break;
			}
			default:
			{
				throw DxCore::CNotSupportedException("Unknown QuaternionEquationType");
			}
			}
		}

	public:

		QuaternionGeneralRayTracer(const TraceParams& traceParams, const ModelData& modelData)
			: m_traceParams(traceParams)
			, m_modelData(modelData)
		{
			SetNextCycle();
		}

		virtual ~QuaternionGeneralRayTracer()
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

	std::unique_ptr<IRayTracer> CreateQuaternionGeneralRayTracer(const TraceParams& traceParams, const ModelData& modelData)
	{
		return std::make_unique<QuaternionGeneralRayTracer>(traceParams, modelData);
	}
}