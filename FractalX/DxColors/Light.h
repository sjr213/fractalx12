#pragma once

#include <tuple>
#include "ColorArgb.h"

namespace DxColor
{
	struct Light
	{
		Light()
		{
			SetDefaultLight(*this);
		}

		Light(bool enable, const ColorArgb& diffuse, const ColorArgb& spectacular, const std::tuple<float, float, float>& direction)
			: Enable(enable)
			, Diffuse(diffuse)
			, Spectacular(spectacular)
			, Direction(direction)
		{}

		Light& operator=(const Light& light)
		{
			Enable = light.Enable;
			Diffuse = light.Diffuse;
			Spectacular = light.Spectacular;
			Direction = light.Direction;

			return *this;
		}

		static void SetDefaultLight(Light& light)
		{
			light.Enable = false;
			light.Diffuse = ColorArgb((bite)255, (bite)192, (bite)192, (bite)192);
			light.Spectacular = ColorArgb((bite)255, (bite)192, (bite)192, (bite)192);
			light.Direction = std::make_tuple(0.0f, 0.0f, -1.0f);
		}

		static const int LightVersion = 1;
		bool Enable;
		ColorArgb Diffuse;
		ColorArgb Spectacular;
		std::tuple<float, float, float> Direction;
	};

	struct Lights
	{
		Lights()
		{
			SetDefaultLights(*this);
		}

		static void SetDefaultLights(Lights& lights)
		{
			lights.DefaultLights = false;
			lights.PerPixelLighting = false;
			lights.Alpha = 1.0f;

			ColorArgb d1 = ColorArgb((bite)255, (bite)192, (bite)192, (bite)192);
			ColorArgb s1 = ColorArgb((bite)255, (bite)192, (bite)192, (bite)192);
			std::tuple<float, float, float> direction1 = std::make_tuple(-0.5265408f, -0.5735765f, -0.6275069f);
			lights.Light1 = Light(true, d1, s1, direction1);

			std::tuple<float, float, float> direction2 = std::make_tuple(0.7198464f, 0.3420201f, 0.6040227f);
			lights.Light2 = Light(true, d1, s1, direction2);

			std::tuple<float, float, float> direction3 = std::make_tuple(0.4545195f, -0.7660444f, 0.4545195f);
			lights.Light3 = Light(true, d1, s1, direction3);
		}

		static const int LightsVersion = 1;

		bool DefaultLights;
		bool PerPixelLighting;
		float Alpha;

		Light Light1;
		Light Light2;
		Light Light3;
	};
}