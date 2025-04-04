#include "pch.h"
#include "TraceParamsSerialize.h"

#include "TraceParams.h"
#include "DxSerialize.h"
#include "SerializationException.h"
#include "VertexSerialization.h"
#include "DxColorsSerialization.h"

namespace DXF
{
	void Serialize(CArchive& ar, CartesianConverterGroup& gr)
	{
		if (ar.IsStoring())
		{
			ar << CartesianConverterGroup::CCGVersion;
			ar << gr.MultiplerX << TrigOptionToInt(gr.ThetaOptionX) <<TrigOptionToInt(gr.PhiOptionX);
			ar << gr.MultiplerY << TrigOptionToInt(gr.ThetaOptionY) << TrigOptionToInt(gr.PhiOptionY);
			ar << gr.MultiplerZ << TrigOptionToInt(gr.ThetaOptionZ) << TrigOptionToInt(gr.PhiOptionZ);
		}
		else
		{
			int version = 0;
			ar >> version;

			assert(version == CartesianConverterGroup::CCGVersion);
			if (version != CartesianConverterGroup::CCGVersion)
				throw DxCore::CSerializationException("Unsupported version of CartesianConverterGroup");

			int thetaOpt = 0, phiOpt = 0;

			ar >> gr.MultiplerX >> thetaOpt >> phiOpt;
			gr.ThetaOptionX = TrigOptionFromInt(thetaOpt);
			gr.PhiOptionX = TrigOptionFromInt(phiOpt);

			ar >> gr.MultiplerY >> thetaOpt >> phiOpt;
			gr.ThetaOptionY = TrigOptionFromInt(thetaOpt);
			gr.PhiOptionY = TrigOptionFromInt(phiOpt);

			ar >> gr.MultiplerZ >> thetaOpt >> phiOpt;
			gr.ThetaOptionZ = TrigOptionFromInt(thetaOpt);
			gr.PhiOptionZ = TrigOptionFromInt(phiOpt);
		}
	}

	void Serialize(CArchive& ar, StretchDistanceParams& stretchParams)
	{
		if (ar.IsStoring())
		{
			ar << StretchDistanceParams::StretchVersion;
			ar << stretchParams.StretchDistance << stretchParams.EstimateMinMax <<
				stretchParams.MinDistance << stretchParams.MaxDistance << stretchParams.Stretch;
		}
		else
		{
			int version = StretchDistanceParams::StretchVersion;
			ar >> version;

			if (version < 1)
				throw DxCore::CSerializationException("Stretch distance params version less than 1");

			if(version > StretchDistanceParams::StretchVersion)
				throw DxCore::CSerializationException("Stretch distance params version not supported");

			ar >> stretchParams.StretchDistance >> stretchParams.EstimateMinMax >>
				stretchParams.MinDistance >> stretchParams.MaxDistance >> stretchParams.Stretch;
		}
	}

	void Serialize(CArchive& ar, FractalParams& fractalParams)
	{
		if (ar.IsStoring())
		{
			ar << FractalParams::FractalParamVersion;
			ar << fractalParams.Derivative << fractalParams.Power << fractalParams.Bailout;
			ar << FractalTypeToInt(fractalParams.FractalModelType);
			ar << CartesianConversionTypeToInt(fractalParams.CartesianType);
			ar << BulbNormalizeTypeToInt(fractalParams.NormalizationType);
			ar << fractalParams.NormalizationRoot;
			Serialize(ar, fractalParams.ConversionGroup);
			Serialize(ar, fractalParams.ConstantC);
			ar << Ingles3EquationTypeToInt(fractalParams.InglesEquation);
			ar << fractalParams.Mandlelbrot;
			ar << fractalParams.ConstantCW;
			ar << QuaternionEquationTypeToInt(fractalParams.QuaternionEquation);
		}
		else
		{
			int version = FractalParams::FractalParamVersion;
			ar >> version;

			if (version < 1 || version > FractalParams::FractalParamVersion)
				throw DxCore::CSerializationException("Fractal params version less than 1 or greater than FractalParamVersion");
				
			ar >> fractalParams.Derivative >> fractalParams.Power >> fractalParams.Bailout;
	
			int type = 1;
			ar >> type;

			fractalParams.FractalModelType = FractalTypeFromInt(type);

			int cartesianType = 1;
			ar >> cartesianType;
			fractalParams.CartesianType = CartesianConversionTypeFromInt(cartesianType);

			int normalizationType = 1;
			ar >> normalizationType;
			fractalParams.NormalizationType = BulbNormalizeTypeFromInt(normalizationType);

			ar >> fractalParams.NormalizationRoot;

			Serialize(ar, fractalParams.ConversionGroup);

			Serialize(ar, fractalParams.ConstantC);

			int inglesType = 1;
			ar >> inglesType;
			fractalParams.InglesEquation = Ingles3EquationTypeFromInt(inglesType);

			if (version > 1)
				ar >> fractalParams.Mandlelbrot;

			if (version > 2)
			{
				ar >> fractalParams.ConstantCW;
				int quatType = 1;
				ar >> quatType;
				fractalParams.QuaternionEquation = QuaternionEquationTypeFromInt(quatType);
			}
		}
	}

	void Serialize(CArchive& ar, BulbParams& bulbParams)
	{
		float normalDeltaObsolete = 0.0;

		if (ar.IsStoring())
		{
			ar << BulbParams::BulbParamVersion;
			Serialize(ar, bulbParams.Origin);

			ar << bulbParams.Distance << bulbParams.MaxRaySteps << bulbParams.MinRayDistance;
			ar << bulbParams.StepDivisor << bulbParams.Iterations << normalDeltaObsolete << bulbParams.Fractional;
			ar << bulbParams.MaxDistance;
			ar << bulbParams.BlockDistanceIncrease;
		}
		else
		{
			int version = 0;
			ar >> version;

			if (version > BulbParams::BulbParamVersion || version < 1)
				throw DxCore::CSerializationException("Bulb params version not supported");

			Serialize(ar, bulbParams.Origin);

			ar >> bulbParams.Distance >> bulbParams.MaxRaySteps >> bulbParams.MinRayDistance;
			ar >> bulbParams.StepDivisor >> bulbParams.Iterations >> normalDeltaObsolete >> bulbParams.Fractional;

			if (version > 1)
				ar >> bulbParams.MaxDistance;

			if (version > 2)
				ar >> bulbParams.BlockDistanceIncrease;
		}
	}

	void Serialize(CArchive& ar, BackgroundImageParams& bkgrdParams)
	{
		if (ar.IsStoring())
		{
			ar << BackgroundImageParams::BackgroundParamVersion;
			CString tempFilename = bkgrdParams.ImageFilename.c_str();
			ar << tempFilename;
			ar << bkgrdParams.ShowBackgroundModel;
			Serialize(ar, bkgrdParams.BackgroundModel);
			ar << bkgrdParams.ShowShadow;
			ar << ShadowWorldTypeToInt(bkgrdParams.WorldType);
			DxColor::Serialize(ar, bkgrdParams.ShadowColor);
		}
		else
		{
			int version = 0;
			ar >> version;

			assert(version == BackgroundImageParams::BackgroundParamVersion);
			if (version != BackgroundImageParams::BackgroundParamVersion)
				throw DxCore::CSerializationException("Background params version not supported");

			CString tempFilename;
			ar >> tempFilename;
			bkgrdParams.ImageFilename = tempFilename.GetString();
			ar >> bkgrdParams.ShowBackgroundModel;
			Serialize(ar, bkgrdParams.BackgroundModel);
			ar >> bkgrdParams.ShowShadow;
			int worldType = 0;
			ar >> worldType;
			bkgrdParams.WorldType = ShadowWorldTypeFromInt(worldType);
			DxColor::Serialize(ar, bkgrdParams.ShadowColor);
		}
	}

	void Serialize(CArchive& ar, TraceParams& traceParams)
	{
		int currentVersion = TraceParams::TraceParamVersion;

		if (ar.IsStoring())
		{
			ar << currentVersion;
		}
		else
		{
			ar >> currentVersion;

			if (currentVersion < 1)
				throw DxCore::CSerializationException("Trace params version less than 1");

			if(currentVersion > TraceParams::TraceParamVersion)
				throw DxCore::CSerializationException("Trace params version greater than TraceParamVersion");
		}

		Serialize(ar, traceParams.Fractal);
		Serialize(ar, traceParams.Bulb);
		Serialize(ar, traceParams.Stretch);
		Serialize(ar, traceParams.Background);	
	}

}