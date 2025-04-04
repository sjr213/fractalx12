#include "stdafx.h"
#include "DxColorsSerialization.h"

#include <afx.h>
#include <assert.h>
#include "ColorArgb.h"
#include "ColorContrast.h"
#include "../DxCore/SerializationException.h"
#include "EffectColors.h"
#include "Light.h"


namespace DxColor
{
	void SerializeTuple3(CArchive& ar, std::tuple<float, float, float>& f3)
	{
		if (ar.IsStoring())
		{
			ar << std::get<0>(f3) << std::get<1>(f3) << std::get<2>(f3);
		}
		else
		{
			ar >> std::get<0>(f3) >> std::get<1>(f3) >> std::get<2>(f3);
		}
	}

	void Serialize(CArchive& ar, ColorArgb& color)
	{
		if (ar.IsStoring())
			ar << color.A << color.R << color.G << color.B;
		else
		{
			// ar >> color.A >> color.R << color.G << color.B;
			BYTE a, r, g, b;
			ar >> a >> r >> g >> b;

			color.A = a;
			color.R = r;
			color.G = g;
			color.B = b;
		}
	}

	void Serialize(CArchive& ar, Light& light)
	{
		if (ar.IsStoring())
		{
			ar << Light::LightVersion;
			ar << light.Enable;
			Serialize(ar, light.Diffuse);
			Serialize(ar, light.Spectacular);
			SerializeTuple3(ar, light.Direction);
		}
		else
		{
			int version = 0;
			ar >> version;

			if (version != Light::LightVersion)
			{
				assert(false);
				throw DxCore::CSerializationException("Light version not supported");
			}

			ar >> light.Enable;
			Serialize(ar, light.Diffuse);
			Serialize(ar, light.Spectacular);
			SerializeTuple3(ar, light.Direction);
		}
	}

	void Serialize(CArchive& ar, ColorContrast& contrast)
	{
		if (ar.IsStoring())
		{
			ar << ColorContrast::ContrastVersion;
			ar << static_cast<int>(contrast.Mode);
			ar << contrast.MinContrast[0] << contrast.MinContrast[1] << contrast.MinContrast[2];
			ar << contrast.MaxContrast[0] << contrast.MaxContrast[1] << contrast.MaxContrast[2];

			ar << contrast.MinHue << contrast.MaxHue;
			ar << contrast.MinSaturation << contrast.MaxSaturation;
			ar << contrast.MinLightness << contrast.MaxLightness;
		}
		else
		{
			int version = ColorContrast::ContrastVersion;
			ar >> version;

			if (version != ColorContrast::ContrastVersion)
				throw DxCore::CSerializationException("Contrast version not supported");

			int mode = 1;
			ar >> mode;
			ar >> contrast.MinContrast[0] >> contrast.MinContrast[1] >> contrast.MinContrast[2];
			ar >> contrast.MaxContrast[0] >> contrast.MaxContrast[1] >> contrast.MaxContrast[2];

			ar >> contrast.MinHue >> contrast.MaxHue;
			ar >> contrast.MinSaturation >> contrast.MaxSaturation;
			ar >> contrast.MinLightness >> contrast.MaxLightness;

			contrast.Mode = static_cast<ContrastType>(mode);
		}
	}

	void Serialize(CArchive& ar, EffectColors& colors)
	{		
		int version = EffectColors::EffectColorsVersion;

		if (ar.IsStoring())
			ar << version;
		else
			ar >> version;

		if(version != EffectColors::EffectColorsVersion)
			throw DxCore::CSerializationException("Effect color version not supported");

		Serialize(ar, colors.Ambient);
		Serialize(ar, colors.Diffuse);
		Serialize(ar, colors.Specular);

		if (ar.IsStoring())
			ar << colors.Roughness;
		else
			ar >> colors.Roughness;

	}

	void Serialize(CArchive& ar, Lights& lights)
	{
		if (ar.IsStoring())
		{
			ar << Lights::LightsVersion;
			ar << lights.DefaultLights;
			ar << lights.PerPixelLighting;
			ar << lights.Alpha;
			Serialize(ar, lights.Light1);
			Serialize(ar, lights.Light2);
			Serialize(ar, lights.Light3);
		}
		else
		{
			int version = 0;
			ar >> version;

			if (version < Lights::LightsVersion)
				throw DxCore::CSerializationException("Lights version not supported");

			ar >> lights.DefaultLights;
			ar >> lights.PerPixelLighting;
			ar >> lights.Alpha;
			Serialize(ar, lights.Light1);
			Serialize(ar, lights.Light2);
			Serialize(ar, lights.Light3);
		}
	}
}