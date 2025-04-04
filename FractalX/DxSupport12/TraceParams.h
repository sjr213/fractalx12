#pragma once

#include <SimpleMath.h>

#include "BulbNormalizeType.h"
#include "CartesianConverterGroup.h"
#include "CartesianConversionType.h"
#include "DefaultFields.h"
#include "FractalType.h"
#include "Ingles3EquantionType.h"
#include "QuaternionEquationType.h"
#include "ShadowWorldType.h"
#include "Vertex.h"


namespace DXF
{
	struct StretchDistanceParams
	{
		static const int StretchVersion = 1;
		bool StretchDistance = false;
		bool EstimateMinMax = false;
		double MinDistance = 0.0;
		double MaxDistance = 0.03;
		bool Stretch = false;
	};

	struct FractalParams
	{
		static const int FractalParamVersion = 3;
		double Derivative = 1.0;
		double Power = 8.0;		
		double Bailout = 2.0;
		FractalType FractalModelType = FractalType::StandardBulb;
		CartesianConversionType CartesianType = CartesianConversionType::StandardConversion;
		BulbNormalizeType NormalizationType = BulbNormalizeType::StandardNormalization;
		double NormalizationRoot = 0.5;
		CartesianConverterGroup ConversionGroup;
		Vertex<double> ConstantC = Vertex<double>(0.0,0.0,0.0);
		Ingles3EquationType InglesEquation = Ingles3EquationType::I_Squared;
		bool Mandlelbrot = false;
		double ConstantCW = 0.0;	// For Quaternions
		QuaternionEquationType QuaternionEquation = QuaternionEquationType::Q_Squared;
	};

	struct BulbParams
	{
		static const int BulbParamVersion = 3;
		DirectX::XMFLOAT3 Origin = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
		float Distance = 10.0f;

		// Distance estimate params
		int MaxRaySteps = 1000;
		double MinRayDistance = 0.0001;
		double StepDivisor = 10.0;

		int Iterations = 256;

		bool Fractional = false;

		double MaxDistance = 10000.0;	// new 7/23/22 to remove spiked when < 1

		bool BlockDistanceIncrease = true;	// new 8/2/22 
	};

	struct BackgroundImageParams
	{
		static const int BackgroundParamVersion = 1;
		std::wstring ImageFilename;
		bool ShowBackgroundModel = false;
		std::vector<Vertex<float>> BackgroundModel = GetDefaultBackgroundModel();

		bool ShowShadow = false;
		ShadowWorldType WorldType = ShadowWorldType::MainModel;
		DxColor::ColorArgb ShadowColor = DXF::GetDefaultShadowColor();
	};

	struct TraceParams
	{
		static const int TraceParamVersion = 1;

		FractalParams Fractal;

		BulbParams Bulb;

		StretchDistanceParams Stretch;

		BackgroundImageParams Background;
	};

}