
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 *                                                                             *
 *  Started by Ángel on december of 2013                                       *
 *                                                                             *
 *  This is free software released into the public domain.                     *
 *                                                                             *
 *  angel.rodriguez@esne.edu                                                   *
 *                                                                             *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <cmath>
#include <cassert>
#include "View.hpp"
#include <Vector.hpp>
#include <Scaling.hpp>
#include <Rotation.hpp>
#include <Projection.hpp>
#include <Translation.hpp>

#include <iostream>

using namespace rapidxml;
using namespace toolkit;

namespace example
{


    View::View(const std::string & path, size_t width, size_t height)
    :
        width       (width ),
        height      (height),
        Color_buffer(width, height),
        rasterizer  (Color_buffer )
    {
       // Carga de escena desde xml
		load_scene(path);

    }

    void View::update ()
    {
		Projection3f projection(0.3f, 1000.f, 90, (width /height));
		for (auto & m : models)
		{
			m->update(&projection, lights.front(),AMBIENTAL_INTENSITY);
		}
    }

    void View::paint ()
    {


		rasterizer.clear();

		for (auto & m : models)
		{
			m->paint(&rasterizer);
		}
		rasterizer.get_color_buffer().gl_draw_pixels(0, 0);

		glClearColor(0, 0, 1, 1);
    }

    bool View::is_frontface (const Vertex * const projected_vertices, const int * const indices)
    {
        const Vertex & v0 = projected_vertices[indices[0]];
        const Vertex & v1 = projected_vertices[indices[1]];
        const Vertex & v2 = projected_vertices[indices[2]];

        // Se asumen coordenadas proyectadas y polígonos definidos en sentido horario.
        // Se comprueba a qué lado de la línea que pasa por v0 y v1 queda el punto v2:

        return ((v1[0] - v0[0]) * (v2[1] - v0[1]) - (v2[0] - v0[0]) * (v1[1] - v0[1]) > 0.f);
    }

	bool View::load_scene(const string & path)
	{
		ifstream xml_file(path);

		xml_document<> document;
		vector<char> buffer((std::istreambuf_iterator<char>(xml_file)), std::istreambuf_iterator<char>());
		buffer.push_back('\0');

		document.parse<0>(&buffer[0]);
		xml_node<> * root = document.first_node();

		if (root && string(root->name()) == "scene")
		{
			for (xml_Node * elem = root->first_node(); elem; elem = elem->next_sibling()) 
			{
				if (elem->type() == node_element && std::string(elem->name()) == "mesh") 
				{
					if (!parse_mesh(elem)) return false;
				}
				else if (elem->type() == node_element && std::string(elem->name()) == "light") 
				{
					if (!parse_light(elem)) return false;
				}
			}

			return true;
		}

		return false;

	}

	bool View::parse_mesh(xml_Node * mesh_data)
	{
		string path = "..\\..\\resources\\";

		Translation3f position;
		float rot_x, rot_y, rot_z;
		Scaling3f scale;

		string attributes;

		for (xml_Node * tag = mesh_data->first_node(); tag; tag = tag->next_sibling()) {
			attributes = tag->value();
			if (tag->type() == node_element) {
				if (string(tag->name()) == "model") 
				{
					path += attributes;
				}
				else if (string(tag->name()) == "position") 
				{
					vector<float> values;
					size_t comas_count = std::count(attributes.begin(), attributes.end(), ',');
					
					for (size_t i = 0; i < comas_count + 1; ++i) 
					{
						size_t position;
						position = attributes.find(',');
						values.push_back(std::stof(attributes.substr(0, position)));
						attributes.erase(0, position + 1);
					}
					position.set(values[0], values[1], values[2]);
				}
				else if (string(tag->name()) == "rotation")
				{
					std::vector<float> values;
					size_t comas_count = std::count(attributes.begin(), attributes.end(), ',');
					
					for (size_t i = 0; i < comas_count + 1; ++i) {
						size_t position;
						position = attributes.find(',');
						values.push_back(std::stof(attributes.substr(0, position)));
						attributes.erase(0, position + 1);
					}
					rot_x = values[0];
					rot_y = values[1];
					rot_z = values[2];
				}
				else if (string(tag->name()) == "scale") 
				{
					scale.set(std::stof(tag->value()));
				}
				else if (string(tag->name()) == "color")
				{
					std::vector<uint8_t> values;
					size_t n_Coma = std::count(attributes.begin(), attributes.end(), ',');
					
					for (size_t i = 0; i < n_Coma + 1; ++i)
					{
						size_t position;
						position = attributes.find(',');
						values.push_back((uint8_t)std::stof(attributes.substr(0, position)));
						attributes.erase(0, position + 1);
					}
					color.set(values[0], values[1], values[2]);
				}
			}
		}

		std::shared_ptr<Model> new_model(new Model(path, position, scale, rot_x, rot_y, rot_z, color));
		std::cout << "Isla" << std::endl;
		models.push_back(new_model);

		return true;
	}

	bool View::parse_light(xml_Node * light_data)
	{
		float intensity;

		Translation3f position;
		float rot_x, rot_y, rot_z;
		Scaling3f scale;


		Point3i color;
		string attributes;
		for (xml_Node * tag = light_data->first_node(); tag; tag = tag->next_sibling()) 
		{
			attributes = tag->value();
			if (tag->type() == node_element) {
				if (string(tag->name()) == "intensity")
				{
					intensity = std::stof(attributes);
				}
				else if (string(tag->name()) == "position") 
				{
					size_t n_Coma = std::count(attributes.begin(), attributes.end(), ',');
					std::vector<float> values;
					for (size_t i = 0; i < n_Coma + 1; ++i) {
						size_t position;
						position = attributes.find(',');
						values.push_back(std::stof(attributes.substr(0, position)));
						attributes.erase(0, position + 1);
					}
					position.set(values[0], values[1], values[2]);
				}
				else if (std::string(tag->name()) == "rotation") 
				{
					size_t n_Coma = std::count(attributes.begin(), attributes.end(), ',');
					std::vector<float> values;
					for (size_t i = 0; i < n_Coma + 1; ++i) {
						size_t position;
						position = attributes.find(',');
						values.push_back(std::stof(attributes.substr(0, position)));
						attributes.erase(0, position + 1);
					}
					rot_x = values[0];
					rot_y = values[1];
					rot_z = values[2];
				}
				else if (std::string(tag->name()) == "scale") 
				{
					scale.set(std::stof(attributes));
				}
				else if (std::string(tag->name()) == "color") 
				{
					size_t n_Coma = std::count(attributes.begin(), attributes.end(), ',');
					std::vector<uint8_t> values;
					for (size_t i = 0; i < n_Coma + 1; ++i) {
						size_t position;
						position = attributes.find(',');
						values.push_back((uint8_t)std::stof(attributes.substr(0, position)));
						attributes.erase(0, position + 1);
					}
					color[0] = attributes[0];
					color[1] = attributes[1];
					color[2] = attributes[2];
				}
				else
				{
					return false;
				}
			}


		}

		std::shared_ptr<Light> new_light(new Light(position, scale, rot_x, rot_y, rot_z, intensity));
		lights.push_back(new_light);
		std::cout << "Luz" << std::endl;
		return true;
	}

}
