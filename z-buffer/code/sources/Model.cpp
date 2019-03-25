
/**
 * @file Model.cpp
 * @author Gonzalo Perez Chamarro
 * @brief Clase de código fuente de Model.hpp
 * @version 0.1
 * @date 2019-03-10
 * 
 * @copyright Copyright (c) 2019
 * 
 */

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

	Model::Model(const string & name, const std::string & path, Translation3f position, Scaling3f scale, float rx, float ry, float rz, Color c)
		:position(position), scale(scale), name(name)
	{
		//Recibe la rotacion inicial local
		rotation[0] = rx;
		rotation[1] = ry;
		rotation[2] = rz;

		//Crea un buffer de vertices y normales para albergar los del archivo .obj
		Vertex_Buffer     original_vertices;
		Vertex_Buffer	  original_normals;
		vector<shape_t> shapes;
		vector<material_t> materials;

		attrib_t attributes;

		//Comprueba la correcta carga del obj
		if (!LoadObj(&attributes, &shapes, &materials, &error_message, path.c_str()) || !error_message.empty())
		{
			return;
		}

		if (shapes.size() == 0) { error_message = string("There're no shapes in ") + path; return; }
		if (attributes.vertices.size() == 0) { error_message = string("There're no vertices in ") + path; return; }
		if (attributes.normals.size() == 0) { error_message = string("There're no normals in ") + path; return; }

		//Registra el numero de vertices
		n_Vertex = attributes.vertices.size();
		//Recoge los vertices del obj
		for (size_t v = 0; v < n_Vertex; v += 3)
		{
			Point4f temp_Vertices({
				attributes.vertices[v + 0],
				attributes.vertices[v + 1],
				attributes.vertices[v + 2],
				1.f });

			original_vertices.push_back(temp_Vertices);
		}
		//Recoge las normales del obj
		for (size_t v = 0; v < attributes.normals.size(); v += 3)
		{
			Point4f temp_Normals({
				attributes.normals[v + 0],
				attributes.normals[v + 1],
				attributes.normals[v + 2],
				1.f });
			original_normals.push_back(temp_Normals);
		}
		//Recoge los indices del obj, y a su vez, copia los vertices y las normales en orden
		// en dos buffer del mismo tamaño que los indices
		for (auto const & index : shapes[0].mesh.indices)
		{
			original_indices.push_back(index.vertex_index);

			copy_vertices.push_back(original_vertices[index.vertex_index]);
			copy_normals.push_back(original_normals[index.normal_index]);
		}

		//Reescala el resto de buffers a utilizar posteriormente
		original_colors.resize(copy_vertices.size());
		transformed_vertices.resize(copy_vertices.size());
		display_vertices.resize(original_colors.size());
		transformed_colors.resize(original_colors.size());
		transformed_normals.resize(copy_normals.size());
		number_of_vertices = copy_vertices.size();

		//Se crea el buffer de indices del orden de los vértices.
		//Al colocar los vertices y normales en su orden, este buffer es una simple enumeracion
		//*Ha sido necesario su utilización para  no modificar demasiado código del rasterizador*
		for (int i = 0; i < original_indices.size(); ++i)
		{
			index_order.push_back(i);
		}
		
		//Se recogen los colores del modelo
		// (En este ejemplo solo hay un color por modelo, pero se podrian añadir mas)
		for (auto & color : original_colors)
		{
			color = c;
		}
	}

	void Model::update(Projection3f * projection, std::shared_ptr<Light> light, float ambiental_intensity)
	{
		//Se ejerce la rotacion en Y
		rotation[1] += rotation_speed;
		rotation_x.set< Rotation3f::AROUND_THE_X_AXIS >(rotation[0]);
		rotation_y.set< Rotation3f::AROUND_THE_Y_AXIS >(rotation[1]);
		rotation_z.set< Rotation3f::AROUND_THE_Z_AXIS >(rotation[2]);

		//Se realizan las transformaciones adecuadas
		normals_tr = position * rotation_x * rotation_y * rotation_z * scale *  global_tr;
		transform = (*projection) * normals_tr;
		
		//Se actualizan los hijos
		refresh_children_transform();

		for(size_t index = 0; index < number_of_vertices; ++index)
		{
			//LIGHTNING
			Vertex & vertex = transformed_vertices[index] = Matrix44f(transform) * Matrix41f(copy_vertices[index]);
			Vertex & normal_vertex = transformed_normals[index] = Matrix44f(normals_tr) * Matrix41f(copy_normals[index]);

			//Sobrecargado el operador de indexacion en Translation3f
			Vector3f light_position = Vector3f({ light->get_position()[0], light->get_position()[2], light->get_position()[2] });
			Vector3f norm_light_pos = normalize_vector(light_position);

			Vector3f normal = Vector3f({ normal_vertex[0],normal_vertex[1] ,normal_vertex[2] });
			Vector3f norm_normal = normalize_vector(normal);

			//Se realiza el producto escalar entre las normales y el vector de la luz
			// Y se maximiza para que no de un valor negativo.
			//Se comprueba el mínimo, para que la suma de luz ambiental no supere 1
			float light_intensity = std::max(dot(norm_normal, norm_light_pos), 0.f) + ambiental_intensity;
			light_intensity = std::min(light_intensity, 1.0f);


			transformed_colors[index] = original_colors[index];
			transformed_colors[index].data.component.r *= light_intensity;
			transformed_colors[index].data.component.g *= light_intensity;
			transformed_colors[index].data.component.b *= light_intensity;

			//End Lightning


			float divisor = 1.f / vertex[3];

			vertex[0] *= divisor;
			vertex[1] *= divisor;
			vertex[2] *= divisor;
			vertex[3] = 1.f;

			divisor = 1.f / normal_vertex[3];

			normal_vertex[0] *= divisor;
			normal_vertex[1] *= divisor;
			normal_vertex[2] *= divisor;
			normal_vertex[3] = 1.f;
		}
	}

	void Model::paint(Rasterizer<Color_Buffer> * rasterizer)
	{
		Scaling3f scaling = Scaling3f(float(rasterizer->get_color_buffer().get_width() / 2), float(rasterizer->get_color_buffer().get_height() / 2), 100000000.f);
		Translation3f translation = Translation3f(float(rasterizer->get_color_buffer().get_width() / 2), float(rasterizer->get_color_buffer().get_height() / 2), 0.f);
		Transformation3f transformation = translation * scaling;

		for (size_t index = 0, n_Vertices = transformed_vertices.size(); index < n_Vertices; index++) {
			display_vertices[index] = Point4i(Matrix44f(transformation) * Matrix41f(transformed_vertices[index]));
		}
	

		for (int * indices = index_order.data(), *end = indices + index_order.size(); indices < end; indices +=3)
		{
			if (is_frontface(transformed_vertices.data(), indices))
			{
				//En esta parte se debería añadir el recorte(Se podría realizar antes)
				//De esta manera solo se recortarían las caras que miran a camara
				rasterizer->set_color(transformed_colors[*indices]);
				rasterizer->fill_convex_polygon_z_buffer(display_vertices.data(),indices, indices+3);
			}
		}
	}

	void Model::refresh_children_transform()
	{
		for (map<string, std::shared_ptr<Model>>::iterator it =children.begin(); it != children.end(); ++it)
		{
			it->second->set_parent_transform(normals_tr);
		}
	}

	bool Model::is_frontface(const Vertex * const projected_vertices, const int * const indices)
	{
		const Vertex & v0 = projected_vertices[indices[0]];
		const Vertex & v1 = projected_vertices[indices[1]];
		const Vertex & v2 = projected_vertices[indices[2]];

		return ((v1[0] - v0[0]) * (v2[1] - v0[1]) - (v2[0] - v0[0]) * (v1[1] - v0[1]) > 0.f);
	}

	/**
	 * @brief Metodo sobrecargado para esta disposición de buffer de vertices ordenados
	 * 
	 * @param projected_vertices 
	 * @param index 
	 * @return true 
	 * @return false 
	 */
	bool Model::is_frontface(const Vertex * const projected_vertices, const int index)
	{
		const Vertex & v0 = projected_vertices[index];
		const Vertex & v1 = projected_vertices[index+1];
		const Vertex & v2 = projected_vertices[index+2];

		return ((v1[0] - v0[0]) * (v2[1] - v0[1]) - (v2[0] - v0[0]) * (v1[1] - v0[1]) > 0.f);
	}
	
}


