/**
 * @file Model.hpp
 * @author Gonzalo Perez Chamarro
 * @brief Clase que representa cualquier modelo obj de la escena
 * @version 0.1
 * @date 2019-03-10
 * 
 * @copyright Copyright (c) 2019
 * 
 */

#pragma once


#include "Color_Buffer_Rgba8888.hpp"
#include "Rasterizer.hpp"
#include <Projection.hpp> 
#include <string>
#include <vector>
#include <map>


#include <Translation.hpp> 
#include <Rotation.hpp>
#include <Scaling.hpp>
#include "Light.hpp"

namespace example
{
	using namespace toolkit;
	using std::string;
	using std::vector;
	using std::map;

	using toolkit::Point4i;


	class Model
	{
	private:
		typedef Color_Buffer_Rgba8888 Color_Buffer;
		typedef Color_Buffer_Rgba8888::Color Color;

		typedef Point4f               Vertex;
		typedef vector< Vertex >      Vertex_Buffer;
		typedef vector< int    >      Index_Buffer;
		typedef vector< Color  >      Vertex_Colors;


		typedef Transformation3f Transform;

		typedef Translation3f Position;
		typedef Rotation3f Rotation;
		typedef Scaling3f Scale;

	private:
	/**
	 * @brief Matriz de transformacion
	 * 
	 */
		Transform transform;

		/**
		 * @brief Transform del padre
		 * 
		 */
		Transform global_tr;
		/**
		 * @brief Transform de las normales
		 * 
		 */
		Transform normals_tr;

/**
 * @brief Posicion de la luz
 * 
 */
		Position position;
		/**
		 * @brief Rotacion en x
		 * 
		 */
		Rotation rotation_x;
		/**
		 * @brief Rotacion en y
		 * 
		 */
		Rotation rotation_y;
		/**
		 * @brief Rotacion en z
		 * 
		 */
		Rotation rotation_z;
		/**
		 * @brief Escala
		 * 
		 */
		Scale    scale;

		float rotation[3];

/**
 * @brief Mensaje de error de la carga del modelo
 * 
 */
		string error_message;
		size_t n_Vertex;

/**
 * @brief Buffer de indices del obj
 * 
 */
		Index_Buffer      original_indices;

/**
 * @brief Buffer de vertices ordenados
 * 
 */
		Vertex_Buffer     copy_vertices;

		/**
		 * @brief Buffer de normales ordenadas
		 * 
		 */
		Vertex_Buffer	  copy_normals;

/**
 * @brief Array de colores originales de los vertices
 * 
 */
		Vertex_Colors     original_colors;
/**
 * @brief Array de colores ya transformados con la luz
 * 
 */
		Vertex_Colors	transformed_colors;

/**
 * @brief Buffer de vertices transformados
 * 
 */
		Vertex_Buffer   transformed_vertices;

/**
 * @brief Buffer de normales transformadas
 * 
 */
		Vertex_Buffer	transformed_normals;

/**
 * @brief Vector de vertices a pintar
 * 
 */
		vector< Point4i > display_vertices;
/**
 * @brief indice en orden de vertices
 * 
 */
		vector<int> index_order;
/**
 * @brief Numero de vertices totales
 * 
 */
		int number_of_vertices;
/**
 * @brief Mapa de hijos localizados por nombre
 * 
 */
		map<string, std::shared_ptr<Model>> children;
/**
 * @brief Nombre del modelo
 * 
 */
		string name;
/**
 * @brief Velocidad de rotacion del modelo
 * 
 */
		float rotation_speed;


	public:

/**
 * @brief Constructor del modelo
 * 
 * @param name Nombre
 * @param path Ruta del obj
 * @param position Posicion local
 * @param scale Escala del modelo
 * @param rx Rotacion en x
 * @param ry Rotacion en y
 * @param rz Rotacion en z
 * @param c Color del modelo
 */
		Model(const string & name, const string & path, Translation3f position, Scaling3f scale, float rx, float ry, float rz, Color c);

/**
 * @brief Metodo de actualizacion del modelo en cuanto a posicion y luz
 * 
 * @param projection Proyeccion
 * @param light Luz de la escena
 * @param ambiental_intensity Intensidad ambiental de la escena
 */
		void update(Projection3f * projection, std::shared_ptr<Light> light, float ambiental_intensity);
/**
 * @brief Metodo de pintado del modelo
 * 
 * @param rasterizer 
 */
		void paint(Rasterizer<Color_Buffer> * rasterizer);

/**
 * @brief (Antiguo) Metodo que comprueba si el poligono mira a camara
 * 
 * @param projected_vertices 
 * @param indices 
 * @return true 
 * @return false 
 */
		bool is_frontface(const Vertex * const projected_vertices, const int * const indices);
/**
 * @brief Metodo que comprueba si el poligono mira a camara
 * 
 * @param projected_vertices 
 * @param indices 
 * @return true 
 * @return false 
 */
		bool is_frontface(const Vertex * const projected_vertices, const int index);
/**
 * @brief Añade un nuevo hijo a la lista de hijos
 * 
 * @param name Nombre del hijo
 * @param child Puntero al hijo
 */
		void add_child(const string name, std::shared_ptr<Model> child)
		{
			children[name] = child;
		}
/**
 * @brief Devuelve el nombre del modelo
 * 
 * @return const string& 
 */
		const string & get_name() const { return name; }
/**
 * @brief Ajusta el transform del padre
 * 
 * @param tr 
 */
		void set_parent_transform(Transform tr)
		{
			global_tr = tr;
		}
/**
 * @brief Devuelve el transform del modelo
 * 
 * @return Transform 
 */
		Transform get_transform() const
		{
			return transform;
		}
/**
 * @brief Ajusta la velocidad de rotacion en y
 * 
 * @param speed 
 */
		void set_rotation_speed_y(float speed) { rotation_speed = speed; }

	private:
/**
 * @brief Envia el transform del modelo a todos sus hijos
 * 
 */
		void refresh_children_transform();
/**
 * @brief Devuelve el modulo de un vector
 * 
 * @param vector 
 * @return float 
 */
		float get_vector_module(const Vector3f & vector)const
		{
			return sqrt((vector[0] * vector[0]) + (vector[1] * vector[1]) + (vector[2] * vector[2]));
		}
/**
 * @brief Normaliza el vector recibido
 * 
 * @param vector 
 * @return Vector3f 
 */
		Vector3f normalize_vector(Vector3f vector)
		{
			float module = get_vector_module(vector);
			return Vector3f({ vector[0] / module, vector[1] / module, vector[2] / module});
		}
/**
 * @brief Multiplicacion escalar de dos vectores
 * 
 * @param m 
 * @param n 
 * @return float 
 */
		float dot(Vector3f m, Vector3f n)
		{
			return m[0] * n[0] + m[1] * n[1] + m[2] * n[2];
		}

		int clip_with_viewport_2d(const Point4f * vertices, const int * first_index, const int * last_index, Point4f * clipped_vertices);

		int clip_with_line_2d
		(
			const Point4f * vertices,
			const int      * first_index,
			const int      * last_index,
			Point4f * clipped_vertices,
			float      a,
			float      b,
			float      c
		);

	};
}