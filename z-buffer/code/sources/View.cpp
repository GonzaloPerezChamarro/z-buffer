/**
 * @file View.cpp
 * @author Gonzalo Perez Chamarro
 * @brief Clase de código fuente de View.hpp
 * @version 0.1
 * @date 2019-03-10
 * 
 * @copyright Copyright (c) 2019
 * 
 */

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
		Projection3f projection(0.3f, 1000.f, 90.f, (float)(width /height));
		for (auto & m : models)
		{
			m->update(&projection, lights.front(),AMBIENTAL_INTENSITY);
		}
    }

    void View::paint ()
    {

		rasterizer.clear(0,100,255);

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
				//"Parseo" de un modelo
				if (elem->type() == node_element && std::string(elem->name()) == "mesh") 
				{
					if (!parse_mesh(elem)) return false;
				}
				//"Parseo" de una luz
				else if (elem->type() == node_element && std::string(elem->name()) == "light") 
				{
					if (!parse_light(elem)) return false;
				}
			}

			return true;
		}

		return false;

	}

	std::shared_ptr<Model> View::parse_mesh(xml_Node * mesh_data)
	{
		//Ruta relativa a los .obj
		string path = "..\\resources\\";

		Translation3f position;
		float rot_x, rot_y, rot_z;
		Scaling3f scale;
		float rot_speed;

		string attributes;
		string name;

		map<string, std::shared_ptr<Model>> children;

		for (xml_Node * tag = mesh_data->first_node(); tag; tag = tag->next_sibling()) {
			attributes = tag->value();
			if (tag->type() == node_element) {
				//Nombre del modelo
				if (string(tag->name()) == "model") 
				{
					name = attributes;
					path += attributes;
				}
				//Posicion del modelo
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
				//Rotacion del modelo
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
				//Escala del modelo
				else if (string(tag->name()) == "scale") 
				{
					scale.set(std::stof(tag->value()));
				}
				//Color del modelo
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
				//Rotacion en el eje Y del modelo
				else if (string(tag->name()) == "rot_speed")
				{
					rot_speed = std::stof(tag->value());
				}
				//Modelos hijos
				else if (string(tag->name()) == "children")
				{
					for (xml_Node * child = tag->first_node(); child;child = child->next_sibling())
					{

						std::shared_ptr<Model> c = parse_mesh(child);
						children[c->get_name()] = c;
						
					}
				}
			}
		}

		//El modelo padre es el ultimo en crearse, por lo que se realiza un push_front
		// (y no push_back) para que en el momento de actualizarse, tenga preferencia el padre
		std::shared_ptr<Model> new_model(new Model(name, path, position, scale, rot_x, rot_y, rot_z, color));
		models.push_front(new_model);

		for (std::map<string,std::shared_ptr<Model>>::iterator it = children.begin(); it != children.end(); ++it)
		{
			new_model->add_child(it->first, it->second);
		}

		new_model->set_rotation_speed_y(rot_speed);

		return new_model;
	}

	bool View::parse_light(xml_Node * light_data)
	{

		Translation3f position;
		float rot_x, rot_y, rot_z;
		Scaling3f scale;


		Point3i color;
		string attributes;
		for (xml_Node * tag = light_data->first_node(); tag; tag = tag->next_sibling()) 
		{
			attributes = tag->value();
			if (tag->type() == node_element) {
				if (string(tag->name()) == "position") 
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
				//Actualmente no hay implementado color
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

		std::shared_ptr<Light> new_light(new Light(position, scale, rot_x, rot_y, rot_z));
		lights.push_back(new_light);
		return true;
	}

}
