
#define TINYOBJLOADER_IMPLEMENTATION
#include "Model.hpp"
#include <iostream>

#include <math.h>
#include <vector>
#include <tiny_obj_loader.h>


namespace example
{
	using namespace tinyobj;
	using std::max;

	Model::Model(const std::string & path, Translation3f position, Scaling3f scale, float rx, float ry, float rz, Color c)
		:position(position), scale(scale)
	{
		rotation[0] = rx;
		rotation[1] = ry;
		rotation[2] = rz;

		
		vector<shape_t> shapes;
		vector<material_t> materials;

		attrib_t attributes;

		if (!LoadObj(&attributes, &shapes, &materials, &error_message, path.c_str()) || !error_message.empty())
		{
			return;
		}

		if (shapes.size() == 0) { error_message = string("There're no shapes in ") + path; return; }
		if (attributes.vertices.size() == 0) { error_message = string("There're no vertices in ") + path; return; }
		if (attributes.normals.size() == 0) { error_message = string("There're no normals in ") + path; return; }

		n_Vertex = attributes.vertices.size();
		for (size_t v = 0; v < n_Vertex; v += 3)
		{
			Point4f temp_Vertices({
				attributes.vertices[v + 0],
				attributes.vertices[v + 1],
				attributes.vertices[v + 2],
				1.f });

			original_vertices.push_back(temp_Vertices);
		}

		for (size_t v = 0; v < attributes.normals.size(); v += 3)
		{
			Point4f temp_Normals({
				attributes.normals[v + 0],
				attributes.normals[v + 1],
				attributes.normals[v + 2],
				1.f });
			original_normals.push_back(temp_Normals);
		}

		for (auto const & index : shapes[0].mesh.indices)
		{
			original_indices.push_back(index.vertex_index);

			copy_vertices.push_back(original_vertices[index.vertex_index]);
			copy_normals.push_back(original_normals[index.normal_index]);
		}

		original_colors.resize(copy_vertices.size());
		transformed_vertices.resize(copy_vertices.size());
		display_vertices.resize(original_colors.size());
		transformed_colors.resize(original_colors.size());
		transformed_normals.resize(copy_normals.size());
		

		for (auto & color : original_colors)
		{
			color = c;
		}
	}

	void Model::update(Projection3f * projection)
	{

		rotation_x.set< Rotation3f::AROUND_THE_X_AXIS >(rotation[0]);
		rotation_y.set< Rotation3f::AROUND_THE_Y_AXIS >(rotation[1]);
		rotation_z.set< Rotation3f::AROUND_THE_Z_AXIS >(rotation[2]);

		transform = (*projection) * position * rotation_x * rotation_y * rotation_z * scale;

		for
			(
				size_t index = 0, number_of_vertices = copy_vertices.size();
				index < number_of_vertices;
				++index
				)
		{

			Vertex & vertex = transformed_vertices[index] = Matrix44f(transform) * Matrix41f(copy_vertices[index]);
			Vertex & normal_vertex = transformed_normals[index] = Matrix44f(transform) * Matrix41f(copy_normals[index]);

			float divisor = 1.f / vertex[3];

			vertex[0] *= divisor;
			vertex[1] *= divisor;
			vertex[2] *= divisor;
			vertex[3] = 1.f;

			normal_vertex[0] *= divisor;
			normal_vertex[1] *= divisor;
			normal_vertex[2] *= divisor;
			normal_vertex[3] = 1.f;
		}
	}

	void Model::paint(Rasterizer<Color_Buffer> * rasterizer, std::shared_ptr<Light> light)
	{
		Scaling3f scaling = Scaling3f(float(rasterizer->get_color_buffer().get_width() / 2), float(rasterizer->get_color_buffer().get_height() / 2), 100000000.f);
		Translation3f translation = Translation3f(float(rasterizer->get_color_buffer().get_width() / 2), float(rasterizer->get_color_buffer().get_height() / 2), 0.f);
		Transformation3f transformation = translation * scaling;

		for (size_t index = 0, n_Vertices = transformed_vertices.size(); index < n_Vertices; index++) {
			display_vertices[index] = Point4i(Matrix44f(transformation) * Matrix41f(transformed_vertices[index]));
		}
		
		//MALLA CON COLOR - LAMBERT
		for (size_t index = 0, number_of_vertices = copy_vertices.size();
			index < number_of_vertices;
			++index) {

			Translation3f pos = light->get_position();
			
			//Sobrecargado el operador de indexacion de Translation3f
			float x = pos[0] - transformed_vertices[index][0];
			float y = pos[1] - transformed_vertices[index][1];
			float z = pos[2] - transformed_vertices[index][2];


			Point4f vector({ x,y,z,1 });
			float length = get_vector_module(vector);
			Vertex light_Vertex = Point4f({ vector[0] / length, vector[1] / length, vector[2] / length, 1 });

			float distance = get_distance(Point4f({ x, y, z, 1 }));


			float value = max(dot(transformed_normals[index], Point4f({ x, y, z, 1 })), -1.0f);
			value += distance;
			value += light->get_intensity();
			Color & color = transformed_colors[index] = original_colors[index];

			color.data.component.r *= value;
			color.data.component.g *= value;
			color.data.component.b *= value;
		}


		for(size_t index = 0, number_of_vertices = copy_vertices.size();
			index < number_of_vertices;
			++index)
		{

			Vertex & vertex = transformed_vertices[index] = Matrix44f(transform) * Matrix41f(copy_vertices[index]);

			float divisor = 1.f / vertex[3];

			vertex[0] *= divisor;
			vertex[1] *= divisor;
			vertex[2] *= divisor;
			vertex[3] = 1.f;
		}

		Point4i clipped_vertices[20];
		static const int clipped_indices[20] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19 };

		/*
		for (int * indices = original_indices.data(), *end = indices + original_indices.size();
			indices < end;
			indices += 3)
		{
			if (is_frontface(transformed_vertices.data(), indices))
			{
				int clipped_vertex_count = clip_with_viewport_2d(&copy_vertices[0], indices, indices + 3, clipped_vertices);
				if (clipped_vertex_count >= 3)
				{
					rasterizer->set_color(transformed_colors[*indices]);
					rasterizer->fill_convex_polygon_z_buffer(display_vertices.data(), indices, indices+3);

				//}
			}
		}*/
		vector<int> indd;
		for (int i = 0; i < original_indices.size(); ++i)
		{
			indd.push_back(i);
		}
		for (int * indices = indd.data(), *end = indices + indd.size(); indices < end; indices +=3)
		{
			if (is_frontface(transformed_vertices.data(), indices))
			{
				rasterizer->set_color(transformed_colors[*indices]);
				rasterizer->fill_convex_polygon_z_buffer(display_vertices.data(),indices, indices+3);
			}
		}
	}

	bool Model::is_frontface(const Vertex * const projected_vertices, const int * const indices)
	{
		const Vertex & v0 = projected_vertices[indices[0]];
		const Vertex & v1 = projected_vertices[indices[1]];
		const Vertex & v2 = projected_vertices[indices[2]];

		return ((v1[0] - v0[0]) * (v2[1] - v0[1]) - (v2[0] - v0[0]) * (v1[1] - v0[1]) > 0.f);
	}

	bool Model::is_frontface(const Vertex * const projected_vertices, const int index)
	{
		const Vertex & v0 = projected_vertices[index];
		const Vertex & v1 = projected_vertices[index+1];
		const Vertex & v2 = projected_vertices[index+2];

		return ((v1[0] - v0[0]) * (v2[1] - v0[1]) - (v2[0] - v0[0]) * (v1[1] - v0[1]) > 0.f);
	}
	
	
	int clip_with_viewport_2d(const Point4f * vertices, const int * first_index, const int * last_index, Point4f * clipped_vertices)
	{
		Point4f        aux_vertices[20];
		static const int aux_indices[20] = { 0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19 };

		int count = clip_with_line_2d
		(
			vertices,
			first_index,
			last_index,
			aux_vertices,
			-1,
			0,
			0
		);

		if (count < 3) return count;

		count = clip_with_line_2d
		(
			aux_vertices,
			aux_indices,
			aux_indices + count,
			clipped_vertices,
			? ,
			? ,
			?
		);

		if (count < 3) return count;

		count = clip_with_line_2d
		(
			clipped_vertices,
			aux_indices,
			aux_indices + count,
			aux_vertices,
			? ,
			? ,
			?
		);

		if (count < 3) return count;

		return clip_with_line_2d
		(
			aux_vertices,
			aux_indices,
			aux_indices + count,
			clipped_vertices,
			? ,
			? ,
			?
		);
	}

	int clip_with_line_2d(const Point4f * vertices, const int * first_index, const int * last_index, Point4f * clipped_vertices,
		float a, float b, float c)
	{

	}

}


