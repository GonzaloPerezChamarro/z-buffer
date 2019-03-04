
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 *                                                                             *
 *  Started by Ángel on december of 2013                                       *
 *                                                                             *
 *  This is free software released into the public domain.                     *
 *                                                                             *
 *  angel.rodriguez@esne.edu                                                   *
 *                                                                             *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

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

        size_t width;
        size_t height;

        Color_Buffer               Color_buffer;
        Rasterizer< Color_Buffer > rasterizer;
		Color color;

        Vertex_Buffer     original_vertices;
        Index_Buffer      original_indices;
        Vertex_Colors     original_colors;
        Vertex_Buffer     transformed_vertices;
        vector< Point4i > display_vertices;

		std::list<std::shared_ptr<Model>> models;
		std::list<std::shared_ptr<Light>> lights;

		const float AMBIENTAL_INTENSITY = 0.1f;


    public:

        View(const std::string & path, size_t width, size_t height);

        void update ();
        void paint  ();

    private:

		bool load_scene(const std::string & path);
		bool parse_mesh(xml_Node * mesh_data);
		bool parse_light(xml_Node * light_data);

        bool is_frontface (const Vertex * const projected_vertices, const int * const indices);

    };

}

#endif
