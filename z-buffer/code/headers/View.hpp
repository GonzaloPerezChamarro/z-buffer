/**
 * @file View.hpp
 * @author Gonzalo Perez Chamarro
 * @brief Clase que representa una escena y gestiona los elementos de pantalla.
 * @version 0.1
 * @date 2019-03-10
 * 
 * @copyright Copyright (c) 2019
 * 
 */


#ifndef VIEW_HEADER
#define VIEW_HEADER

#include <vector>
#include <Point.hpp>
#include "Rasterizer.hpp"
#include "Color_Buffer_Rgb565.hpp"
#include "Color_Buffer_Rgba8888.hpp"

#include "Model.hpp"

#include <rapidxml.hpp>
#include <list>
#include <iostream>
#include <fstream>

namespace example
{

    using std::vector;
	using std::string;
    using toolkit::Point4i;
    using toolkit::Point4f;
	using std::ifstream;

    class View
    {
    private:

        typedef Color_Buffer_Rgba8888 Color_Buffer;
        typedef Color_Buffer::Color   Color;
        typedef Point4f               Vertex;
        typedef vector< Vertex >      Vertex_Buffer;
        typedef vector< int    >      Index_Buffer;
        typedef vector< Color  >      Vertex_Colors;

		typedef rapidxml::xml_node<> xml_Node;
		typedef rapidxml::xml_attribute<> xml_Attrib;

    private:
/**
 * @brief Ancho de la ventana
 * 
 */
        size_t width;
/**
 * @brief Alto de la ventana
 * 
 */
        size_t height;
/**
 * @brief Buffer de colores
 * 
 */
        Color_Buffer               Color_buffer;
/**
 * @brief Rasterizador de la escena
 * 
 */
        Rasterizer< Color_Buffer > rasterizer;
		Color color;

        Vertex_Buffer     original_vertices;
        Index_Buffer      original_indices;
        Vertex_Colors     original_colors;
        Vertex_Buffer     transformed_vertices;
        vector< Point4i > display_vertices;

/**
 * @brief Lista de modelos
 * 
 */
		std::list<std::shared_ptr<Model>> models;

/**
 * @brief Lista de luces
 * 
 */
		std::list<std::shared_ptr<Light>> lights;

/**
 * @brief Intensidad ambiental de la escena
 * 
 */
		const float AMBIENTAL_INTENSITY = 0.1f;


    public:
/**
 * @brief Constructor de escena
 * 
 * @param path Ruta del xml de la escena
 * @param width Ancho de la ventana
 * @param height Alto de la ventana
 */
        View(const std::string & path, size_t width, size_t height);
/**
 * @brief Actualizacion de la escena
 * 
 */
        void update ();

/**
 * @brief Pintado de la escena
 * 
 */
        void paint  ();

    private:
/**
 * @brief Carga de la escena desde el xml
 * 
 * @param path Ruta al xml
 * @return true Carga correcta
 * @return false Carga incorrecta
 */
		bool load_scene(const std::string & path);
/**
 * @brief Convierte los datos del xml a un modelo
 * 
 * @param mesh_data 
 * @return std::shared_ptr<Model> 
 */
		std::shared_ptr<Model> parse_mesh(xml_Node * mesh_data);
/**
 * @brief Convierte los datos del xml a una luz
 * 
 * @param light_data 
 * @return true 
 * @return false 
 */
		bool parse_light(xml_Node * light_data);

        bool is_frontface (const Vertex * const projected_vertices, const int * const indices);

    };

}

#endif
