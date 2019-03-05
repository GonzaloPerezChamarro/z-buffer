

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
		Transform transform;
		Transform global_tr;
		Transform normals_tr;

		Position position;
		Rotation rotation_x;
		Rotation rotation_y;
		Rotation rotation_z;
		Scale    scale;

		float rotation[3];

		string error_message;
		size_t n_Vertex;


		Index_Buffer      original_indices;

		Vertex_Buffer     copy_vertices;
		Vertex_Buffer	  copy_normals;


		Vertex_Colors     original_colors;
		Vertex_Colors	transformed_colors;

		Vertex_Buffer   transformed_vertices;
		Vertex_Buffer	transformed_normals;
		vector< Point4i > display_vertices;

		vector<int> index_order;
		int number_of_vertices;

		map<string, std::shared_ptr<Model>> children;
		string name;

		float rotation_speed;


	public:


		Model(const string & name, const string & path, Translation3f position, Scaling3f scale, float rx, float ry, float rz, Color c);

		void update(Projection3f * projection, std::shared_ptr<Light> light, float ambiental_intensity);
		void paint(Rasterizer<Color_Buffer> * rasterizer);

		bool is_frontface(const Vertex * const projected_vertices, const int * const indices);
		bool is_frontface(const Vertex * const projected_vertices, const int index);

		void add_child(const string name, std::shared_ptr<Model> child)
		{
			children[name] = child;
		}

		const string & get_name() const { return name; }

		void set_parent_transform(Transform tr)
		{
			global_tr = tr;
		}

		Transform get_transform() const
		{
			return transform;
		}

		void set_rotation_speed_y(float speed) { rotation_speed = speed; }

	private:

		void refresh_children_transform();

		float get_vector_module(const Vector3f & vector)const
		{
			return sqrt((vector[0] * vector[0]) + (vector[1] * vector[1]) + (vector[2] * vector[2]));
		}

		Vector3f normalize_vector(Vector3f vector)
		{
			float module = get_vector_module(vector);
			return Vector3f({ vector[0] / module, vector[1] / module, vector[2] / module});
		}

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