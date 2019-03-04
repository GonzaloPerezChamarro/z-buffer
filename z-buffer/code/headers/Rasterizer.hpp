
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 *                                                                             *
 *  Started by Ángel on december of 2013                                       *
 *                                                                             *
 *  This is free software released into the public domain.                     *
 *                                                                             *
 *  angel.rodriguez@esne.edu                                                   *
 *                                                                             *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef RASTERIZER_HEADER
#define RASTERIZER_HEADER

    #include <limits>
    #include <stdint.h>
    #include <Point.hpp>
    #include <algorithm>

    namespace example
    {

        using toolkit::Point4i;
        using toolkit::Point4f;

        template< class COLOR_BUFFER_TYPE >
        class Rasterizer
        {
        public:

            typedef COLOR_BUFFER_TYPE            Color_Buffer;
            typedef typename Color_Buffer::Color Color;

        private:

            Color_Buffer & color_buffer;

            int offset_cache0[1082];
            int offset_cache1[1082];

            int z_cache0[1082];
            int z_cache1[1082];

            std::vector< int > z_buffer;

        public:

            Rasterizer(Color_Buffer & target)
            :
                color_buffer(target),
                z_buffer(target.get_width () * target.get_height ())
            {
            }

            const Color_Buffer & get_color_buffer () const
            {
                return (color_buffer);
            }

        public:

            void set_color (const Color & new_color)
            {
                color_buffer.set_color (new_color);
            }

            void set_color (int r, int  g, int b)
            {
                color_buffer.set_color (r, g, b);
            }

            void clear ()
            {
                for (Color * c = color_buffer.colors (), * end = c + color_buffer.size (); c < end; c++)
                {
                    *c = 0;
                }

                for (int * z = z_buffer.data (), * end = z + z_buffer.size (); z != end; z++)
                {
                    *z = std::numeric_limits< int >::max ();
                }
            }

            void fill_convex_polygon
            (
                const Point4i * const vertices, 
                const int     * const indices_begin, 
                const int     * const indices_end
            );

            void fill_convex_polygon_z_buffer
            (
                const Point4i * const vertices, 
                const int     * const indices_begin, 
                const int     * const indices_end
            );

        private:

            template< typename VALUE_TYPE, size_t SHIFT >
            void interpolate (int   * cache, int   v0, int   v1, int y_min, int y_max);

        };

        template< class  COLOR_BUFFER_TYPE >
        void Rasterizer< COLOR_BUFFER_TYPE >::fill_convex_polygon
        (
            const Point4i * const vertices, 
            const int     * const indices_begin, 
            const int     * const indices_end
        )
        {
            // Se cachean algunos valores de interés:

                  int   pitch         = color_buffer.get_width ();
                  int * offset_cache0 = this->offset_cache0;
                  int * offset_cache1 = this->offset_cache1;
            const int * indices_back  = indices_end - 1;

            // Se busca el vértice de inicio (el que tiene menor Y) y el de terminación (el que tiene mayor Y):

            const int * start_index   = indices_begin;
                  int   start_y       = vertices[*start_index].coordinates ()[1];
            const int * end_index     = indices_begin;
                  int   end_y         = start_y;

            for (const int * index_iterator = start_index; ++index_iterator < indices_end; )
            {
                int current_y = vertices[*index_iterator].coordinates ()[1];

                if (current_y < start_y)
                {
                    start_y     = current_y; 
                    start_index = index_iterator;
                }
                else
                if (current_y > end_y)
                {
                    end_y       = current_y;
                    end_index   = index_iterator;
                }
            }

            // Se cachean las coordenadas X de los lados que van desde el vértice con Y menor al
            // vértice con Y mayor en sentido antihorario:

            const int * current_index = start_index;
            const int *    next_index = start_index > indices_begin ? start_index - 1 : indices_back;

            int y0 = vertices[*current_index].coordinates ()[1];
            int y1 = vertices[*   next_index].coordinates ()[1];
            int o0 = vertices[*current_index].coordinates ()[0] + y0 * pitch;
            int o1 = vertices[*   next_index].coordinates ()[0] + y1 * pitch;

            while (true)
            {
                interpolate< int64_t, 32 > (offset_cache0, o0, o1, y0, y1);

                if (current_index == indices_begin) current_index = indices_back; else current_index--;
                if (current_index == end_index    ) break;
                if (   next_index == indices_begin) next_index    = indices_back; else    next_index--;

                y0 = y1;
                y1 = vertices[*next_index].coordinates ()[1];
                o0 = o1;
                o1 = vertices[*next_index].coordinates ()[0] + y1 * pitch;
            }

            int end_offset = o1;

            // Se cachean las coordenadas X de los lados que van desde el vértice con Y menor al
            // vértice con Y mayor en sentido horario:

            current_index = start_index;
               next_index = start_index < indices_back ? start_index + 1 : indices_begin;

            y0 = vertices[*current_index].coordinates ()[1];
            y1 = vertices[*   next_index].coordinates ()[1];
            o0 = vertices[*current_index].coordinates ()[0] + y0 * pitch;
            o1 = vertices[*   next_index].coordinates ()[0] + y1 * pitch;

            while (true)
            {
                interpolate< int64_t, 32 > (offset_cache1, o0, o1, y0, y1);

                if (current_index == indices_back) current_index = indices_begin; else current_index++;
                if (current_index == end_index   ) break;
                if (   next_index == indices_back) next_index    = indices_begin; else next_index++;

                y0 = y1;
                y1 = vertices[*next_index].coordinates ()[1];
                o0 = o1;
                o1 = vertices[*next_index].coordinates ()[0] + y1 * pitch;
            }

            if (o1 > end_offset) end_offset = o1;

            // Se rellenan las scanlines desde la que tiene menor Y hasta la que tiene mayor Y:

            offset_cache0 += start_y;
            offset_cache1 += start_y;

            for (int y = start_y; y < end_y; y++)
            {
                o0 = *offset_cache0++;
                o1 = *offset_cache1++;

                if (o0 < o1)
                {
                    while (o0 < o1) color_buffer.set_pixel (o0++);

                    if (o0 > end_offset) break;
                }
                else
                {
                    while (o1 < o0) color_buffer.set_pixel (o1++);

                    if (o1 > end_offset) break;
                }
            }
        }

        template< class  COLOR_BUFFER_TYPE >
        void Rasterizer< COLOR_BUFFER_TYPE >::fill_convex_polygon_z_buffer
        (
            const Point4i * const vertices, 
            const int     * const indices_begin, 
            const int     * const indices_end
        )
        {
            // Se cachean algunos valores de interés:

                  int   pitch         = color_buffer.get_width ();
                  int * offset_cache0 = this->offset_cache0;
                  int * offset_cache1 = this->offset_cache1;
                  int * z_cache0      = this->z_cache0;
                  int * z_cache1      = this->z_cache1;
            const int * indices_back  = indices_end - 1;

            // Se busca el vértice de inicio (el que tiene menor Y) y el de terminación (el que tiene mayor Y):

            const int * start_index   = indices_begin;
                  int   start_y       = vertices[*start_index].coordinates ()[1];
            const int * end_index     = indices_begin;
                  int   end_y         = start_y;

            for (const int * index_iterator = start_index; ++index_iterator < indices_end; )
            {
                int current_y = vertices[*index_iterator].coordinates ()[1];

                if (current_y < start_y)
                {
                    start_y     = current_y; 
                    start_index = index_iterator;
                }
                else
                if (current_y > end_y)
                {
                    end_y       = current_y;
                    end_index   = index_iterator;
                }
            }

            // Se cachean las coordenadas X de los lados que van desde el vértice con Y menor al
            // vértice con Y mayor en sentido antihorario:

            const int * current_index = start_index;
            const int *    next_index = start_index > indices_begin ? start_index - 1 : indices_back;

            int y0 = vertices[*current_index].coordinates ()[1];
            int y1 = vertices[*   next_index].coordinates ()[1];
            int z0 = vertices[*current_index].coordinates ()[2];
            int z1 = vertices[*   next_index].coordinates ()[2];
            int o0 = vertices[*current_index].coordinates ()[0] + y0 * pitch;
            int o1 = vertices[*   next_index].coordinates ()[0] + y1 * pitch;

            while (true)
            {
                interpolate< int64_t, 32 > (offset_cache0, o0, o1, y0, y1);
                interpolate< int32_t,  0 > (     z_cache0, z0, z1, y0, y1);

                if (current_index == indices_begin) current_index = indices_back; else current_index--;
                if (current_index == end_index    ) break;
                if (   next_index == indices_begin) next_index    = indices_back; else    next_index--;

                y0 = y1;
                y1 = vertices[*next_index].coordinates ()[1];
                z0 = z1;
                z1 = vertices[*next_index].coordinates ()[2];
                o0 = o1;
                o1 = vertices[*next_index].coordinates ()[0] + y1 * pitch;
            }

            int end_offset = o1;

            // Se cachean las coordenadas X de los lados que van desde el vértice con Y menor al
            // vértice con Y mayor en sentido horario:

            current_index = start_index;
               next_index = start_index < indices_back ? start_index + 1 : indices_begin;

            y0 = vertices[*current_index].coordinates ()[1];
            y1 = vertices[*   next_index].coordinates ()[1];
            z0 = vertices[*current_index].coordinates ()[2];
            z1 = vertices[*   next_index].coordinates ()[2];
            o0 = vertices[*current_index].coordinates ()[0] + y0 * pitch;
            o1 = vertices[*   next_index].coordinates ()[0] + y1 * pitch;

            while (true)
            {
                interpolate< int64_t, 32 > (offset_cache1, o0, o1, y0, y1);
                interpolate< int32_t,  0 > (     z_cache1, z0, z1, y0, y1);

                if (current_index == indices_back) current_index = indices_begin; else current_index++;
                if (current_index == end_index   ) break;
                if (   next_index == indices_back) next_index    = indices_begin; else next_index++;

                y0 = y1;
                y1 = vertices[*next_index].coordinates ()[1];
                z0 = z1;
                z1 = vertices[*next_index].coordinates ()[2];
                o0 = o1;
                o1 = vertices[*next_index].coordinates ()[0] + y1 * pitch;
            }

            if (o1 > end_offset) end_offset = o1;

            // Se rellenan las scanlines desde la que tiene menor Y hasta la que tiene mayor Y:

            offset_cache0 += start_y;
            offset_cache1 += start_y;
            z_cache0 += start_y;
            z_cache1 += start_y;

            for (int y = start_y; y < end_y; y++)
            {
                o0 = *offset_cache0++;
                o1 = *offset_cache1++;
                z0 = *z_cache0++;
                z1 = *z_cache1++;

                if (o0 < o1)
                {
                    int z_step = (z1 - z0) / (o1 - o0);

                    while (o0 < o1)
                    {
                        if (z0 < z_buffer[o0])
                        {
                            color_buffer.set_pixel (o0);
                            z_buffer[o0] = z0;
                        }

                        z0 += z_step;
                        o0++;
                    }

                    if (o0 > end_offset) break;
                }
                else
                if (o1 < o0)
                {
                    int z_step = (z0 - z1) / (o0 - o1);

                    while (o1 < o0)
                    {
                        if (z1 < z_buffer[o1])
                        {
                            color_buffer.set_pixel (o1);
                            z_buffer[o1] = z1;
                        }

                        z1 += z_step;
                        o1++;
                    }

                    if (o1 > end_offset) break;
                }
            }
        }

        template< class  COLOR_BUFFER_TYPE >
        template< typename VALUE_TYPE, size_t SHIFT >
        void Rasterizer< COLOR_BUFFER_TYPE >::interpolate (int * cache, int v0, int v1, int y_min, int y_max)
        {
            if (y_max > y_min)
            {
                VALUE_TYPE value = (VALUE_TYPE(     v0) << SHIFT);
                VALUE_TYPE step  = (VALUE_TYPE(v1 - v0) << SHIFT) / (y_max - y_min);

                for (int * iterator = cache + y_min, * end = cache + y_max; iterator <= end; )
                {
                   *iterator++ = int(value >> SHIFT);
                    value += step;
                   *iterator++ = int(value >> SHIFT);
                    value += step;
                }
            }
        }

    }

#endif
