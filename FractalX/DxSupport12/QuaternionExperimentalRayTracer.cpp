#include "pch.h"
#include "QuaternionExperimentalRayTracer.h"

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
#include "Vector2Double.h"
#include "Vector3Double.h"
#include "Vector4Double.h"
#include "VectorUtils.h"


using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace DXF::VectorUtils;

namespace DXF
{
	Vector4Double QuadSquare(const Vector4Double& q)
	{
		return Vector4Double(q.x * q.x - q.YZW().Dot(), 2.0 * q.x * q.YZW());
	}

	// return a * ( 4.0*a.x*a.x - dot(a,a)*vec4(3.0,1.0,1.0,1.0) );
	Vector4Double QuadCube(const Vector4Double& q)
	{
		return q * ((4.0 * q.x * q.x) - (Dot(q) * Vector4Double(3.0, 1.0, 1.0, 1.0)));
	}

	double LengthSquared(const Vector4Double& z)
	{
		return z.Dot();
	}

	class QuaternionExperimentalRayTracer : public IRayTracer
	{
	private:
		TraceParams m_traceParams;
		ModelData m_modelData;

	protected:

		XMFLOAT3 CalcNormal(const Vector3Double& pos, double e, Vector4Double C)
		{
			Vector3Double eps(e, 0.0, 0.0);
			double distance = 0.0;

			return Vector3Double(
				map(pos + eps.XYY(), C, distance).x - map(pos - eps.XYY(), C, distance).x,
				map(pos + eps.YXY(), C, distance).x - map(pos - eps.YXY(), C, distance).x,
				map(pos + eps.YYX(), C, distance).x - map(pos - eps.YYX(), C, distance).x).Normalize().ToVector3();
		}
/*
		// Change from vec map(vec3 p, vec4 c) to
		Vector3Double map(Vector3Double p, Vector4Double c)
		{
			Vector4Double z = Vector4Double(p, 0.2);

			double m2 = 0.0;
			Vector2Double  t = Vector2Double(1e10);

			double dz2 = 1.0;
			for (int i = 0; i < 10; i++)
			{
				// |dz|² = |3z²|²
				dz2 *= 9.0 * LengthSquared(QuadSquare(z));

				// z = z^3 + c		
				z = QuadCube(z) + c;

				// stop under divergence		
				m2 = z.Dot();
				if (m2 > 10000.0) break;

				// orbit trapping ( |z|² and z_x  )
				t = Min(t, Vector2Double(m2, abs(z.x)));

			}

			// distance estimator: d(z) = 0.5·log|z|·|z|/|dz|   (see https://iquilezles.org/articles/distancefractals)
			double d = 0.25 * log(m2) * sqrt(m2 / dz2);

			return Vector3Double(d, t.x, t.y);
		}

		// This is similar to Ingles raycast
		double RayMarch(const Vector3Double& start, const Vector3Double& direction, Vector3Double& p, const Vector4Double& C4)
		{
			double maxd = 8.0;
			double precis = 0.002;
			double h = 1.0;
			double t = 0.0;
			double d = 0.0;
			double m = 1.0;
			for (int i = 0; i < 150; i++)
			{
				if (h < precis || t > maxd) break;
				t += h;
				Vector3Double res = map(start + direction * t, C4);
				h = res.x;
				d = res.y;
				m = res.z;
			}

			if (t > maxd) m = -1.0;

			p.x = t;
			p.y = d;
			p.z = m;

			// This should be
			// return (log(r) * r) / dr; where r == z.length()
			return t;		// Not sure if t is correct
		}
*/
		// Change from vec map(vec3 p, vec4 c) to
		Vector3Double map(Vector3Double p, Vector4Double c, double& distance)
		{
			Vector4Double z = Vector4Double(p, 0.2);

			double m2 = 0.0;
			Vector2Double  t = Vector2Double(1e10);

			double dz2 = 1.0;
			for (int i = 0; i < 10; i++)
			{
				// |dz|² = |3z²|²
				dz2 *= 9.0 * LengthSquared(QuadSquare(z));

				// z = z^3 + c		
				z = QuadCube(z) + c;

				// stop under divergence		
				m2 = z.Dot();
				if (m2 > 10000.0) break;

				// orbit trapping ( |z|² and z_x  )
				t = Min(t, Vector2Double(m2, abs(z.x)));

			}

			// distance estimator: d(z) = 0.5·log|z|·|z|/|dz|   (see https://iquilezles.org/articles/distancefractals)
			distance = 0.25 * log(m2) * sqrt(m2 / dz2);

			return z.XYZ();
		}

		// This is similar to Ingles raycast
		double RayMarch(const Vector3Double& start, const Vector3Double& direction, Vector3Double& p, const Vector4Double& C4)
		{
			double maxd = 8.0;
			double precis = 0.002;
			double h = 1.0;
			double t = 0.0;
			double d = 0.0;
			double m = 1.0;
			Vector3Double res;
			double distance = 0.0;

			for (int i = 0; i < 150; i++)
			{
				if (h < precis || t > maxd) break;
				t += h;
			
				res = map(start + direction * t, C4, distance);
				h = res.x;
				d = res.y;
				m = res.z;
			}

			if (t > maxd) //m = -1.0;
				res.z = -1.0;

			p = res;

			// This should be
			// return (log(r) * r) / dr; where r == z.length()
			return distance;		// try changing to distance
		}

		double March(const Vector3Double& start, const Vector3Double& direction, Vector3Double& p, const Vector4Double& C4)
		{
			Vector3Double startD(start);
			Vector3Double directionD(direction);
			Vector3Double pD(p);
			double distance = RayMarch(startD, directionD, pD, C4);
			p = pD.ToVector3();

			return distance;
		}

	public:

		QuaternionExperimentalRayTracer(const TraceParams& traceParams, const ModelData& modelData)
			: m_traceParams(traceParams)
			, m_modelData(modelData)
		{

		}

		virtual ~QuaternionExperimentalRayTracer()
		{}

		std::shared_ptr<DxVertexData> RayTrace(const TriangleData& data, const std::function<void(double)>& setProgress) override
		{
			std::shared_ptr<DxVertexData> vData = std::make_shared<DxVertexData>();
			vData->StretchParams = m_traceParams.Stretch;

			double total = data.Vertices.size() + 2.0;
			int progress = 1;

			Vector4Double C4(m_traceParams.Fractal.ConstantC, -0.3);

			for (const XMFLOAT3& v : data.Vertices)
			{
				auto origin = m_traceParams.Bulb.Origin;
				if (m_modelData.OriginTargetType == OriginTarget::Parallel)
				{
					origin = v;
					origin.z = 0.0f;
				}

				XMFLOAT3 pt;
				if (m_traceParams.Fractal.Mandlelbrot)
					pt = MakeStartingPoint(m_traceParams.Bulb.Distance, origin, v, m_traceParams.Fractal.ConstantC);
				else
					pt = MakeStartingPoint(m_traceParams.Bulb.Distance, origin, v);

				Vector3Double direction = -1.0 * Vector3Double(v - origin);
				Vector3Double p;
				double distance = March(pt, direction, p, C4);

				auto normal = CalcNormal(p, 0.001, C4);

				vData->Vertices.emplace_back(p.ToVector3(), normal, Vector2(static_cast<float>(distance), 0.0f));

				++progress;
				if (progress % 20 == 0)
					setProgress(progress / total);
			}

			for (const Triangle& t : data.Triangles)
			{
				vData->Indices.push_back(t.one);
				vData->Indices.push_back(t.two);
				vData->Indices.push_back(t.three);
			}

			CalculateNormals(*vData);

			setProgress(1.0);

			return vData;
		}

		std::shared_ptr<DxVertexData> RayTraceStretch(const TriangleData& data, const std::function<void(double)>& setProgress) override
		{
			return RayTrace(data, setProgress);
		}

	};

	std::unique_ptr<IRayTracer> CreateQuaternionExperimentalRayTracer(const TraceParams& traceParams, const ModelData& modelData)
	{
		return std::make_unique<QuaternionExperimentalRayTracer>(traceParams, modelData);
	}
}