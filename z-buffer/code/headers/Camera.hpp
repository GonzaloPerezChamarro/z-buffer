#pragma once

#include <Projection.hpp> 
#include <Translation.hpp> 
#include <Rotation.hpp>
#include <Scaling.hpp>

namespace example
{
	using namespace toolkit;

	class Camera
	{
	private:

		typedef Transformation3f Transform;

		typedef Translation3f Position;
		typedef Rotation3f Rotation;
		typedef Scaling3f Scale;

		typedef Projection3f Projection;

	private:
		Transform transform;

		Position position;
		Rotation rotation_x;
		Rotation rotation_y;
		Rotation rotation_z;
		Scale    scale;

		Projection projection_matrix;


	public:
		Camera();
		~Camera();
	};
}
