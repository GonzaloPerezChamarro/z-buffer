#pragma once



#include <Translation.hpp>
#include <Projection.hpp>
#include <Rotation.hpp> 
#include <Scaling.hpp> 

namespace example
{
	using namespace toolkit;

	class Light
	{
	private:

		float intensity;

	private:
		typedef Translation3f Position;
		typedef Rotation3f Rotation;
		typedef Scaling3f Scale;

	private:
		Position position;
		Rotation rotation_x;
		Rotation rotation_y;
		Rotation rotation_z;
		float rotation[3];
		Scale scale;

	public:
		Light(Position position, Scale scale, float rx, float ry, float rz, float intensity)
			:position(position), scale(scale), intensity(intensity) 
		{
			rotation[0] = rx;
			rotation[1] = ry;
			rotation[2] = rz;

			rotation_x.set< Rotation3f::AROUND_THE_X_AXIS >(rx);
			rotation_y.set< Rotation3f::AROUND_THE_Y_AXIS >(ry);
			rotation_z.set< Rotation3f::AROUND_THE_Z_AXIS >(rz);
		}

		float get_intensity()const { return intensity; }
		Position get_position() const { return position; }


	};
}