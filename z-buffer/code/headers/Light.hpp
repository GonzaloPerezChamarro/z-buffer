/**
 * @file Light.hpp
 * @author Gonzalo Perez Chamarro
 * @brief Clase que representa un objeto luz básico
 * @version 0.1
 * @date 2019-03-10
 * 
 * @copyright Copyright (c) 2019
 * 
 */

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
		typedef Translation3f Position;
		typedef Rotation3f Rotation;
		typedef Scaling3f Scale;

	private:
	/**
	 * @brief Vector de la luz
	 * 
	 */
		Position position;
		Rotation rotation_x;
		Rotation rotation_y;
		Rotation rotation_z;
		float rotation[3];
		Scale scale;

	public:
	/**
	 * @brief Constructor de la luz
	 * 
	 * @param position 
	 * @param scale 
	 * @param rx 
	 * @param ry 
	 * @param rz 
	 */
		Light(Position position, Scale scale, float rx, float ry, float rz)
			:position(position), scale(scale)
		{
			rotation[0] = rx;
			rotation[1] = ry;
			rotation[2] = rz;

			rotation_x.set< Rotation3f::AROUND_THE_X_AXIS >(rx);
			rotation_y.set< Rotation3f::AROUND_THE_Y_AXIS >(ry);
			rotation_z.set< Rotation3f::AROUND_THE_Z_AXIS >(rz);
		}

		/**
		 * @brief Devuelve el vector de la luz
		 * 
		 * @return Position 
		 */
		Position get_position() const { return position; }


	};
}