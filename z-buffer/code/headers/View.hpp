
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

    namespace example
    {

        using std::vector;
        using toolkit::Point4i;
        using toolkit::Point4f;

        class View
        {
        private:

            typedef Color_Buffer_Rgba8888 Color_Buffer;
            typedef Color_Buffer::Color   Color;
            typedef Point4f               Vertex;
            typedef vector< Vertex >      Vertex_Buffer;
            typedef vector< int    >      Index_Buffer;
            typedef vector< Color  >      Vertex_Colors;

        private:

            size_t width;
            size_t height;

            Color_Buffer               Color_buffer;
            Rasterizer< Color_Buffer > rasterizer;

            Vertex_Buffer     original_vertices;
            Index_Buffer      original_indices;
            Vertex_Colors     original_colors;
            Vertex_Buffer     transformed_vertices;
            vector< Point4i > display_vertices;

        public:

            View(size_t width, size_t height);

            void update ();
            void paint  ();

        private:

            bool is_frontface (const Vertex * const projected_vertices, const int * const indices);

        };

    }

#endif
