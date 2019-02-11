
/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *\
 *                                                                             *
 *  Started by Ángel on november of 2013                                       *
 *                                                                             *
 *  This is free software released into the public domain.                     *
 *                                                                             *
 *  angel.rodriguez@esne.edu                                                   *
 *                                                                             *
\* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#ifndef COLOR_BUFFER_RGB565_HEADER
#define COLOR_BUFFER_RGB565_HEADER

    #include "Color_Buffer.hpp"
    #include <SFML/OpenGL.hpp>
    #include <GL/glext.h>
    #include <stdint.h>             // This header is <csdint> on C++11...
    #include <vector>

    namespace example
    {

        class Color_Buffer_Rgb565 : public Color_Buffer
        {
        public:

            struct Color
            {
                uint16_t value;

                void set (int r, int g, int b)
                {
                    value = (r >> 3 << 11) | (g >> 2 << 5) | (b >> 3);
                }

                Color & operator = (const uint16_t & new_value)
                {
                    value = new_value;
                    return (*this);
                }
            };

            typedef std::vector< Color > Buffer;

        private:

            Buffer buffer;
            Color  color;

        public:

            Color_Buffer_Rgb565(size_t width, size_t height)
            :
                Color_Buffer(width,  height),
                buffer      (width * height)
            {
            }

        public:

            Color * colors ()
            {
                return (&buffer.front ());
            }

            const Color * colors () const
            {
                return (&buffer.front ());
            }

            int bits_per_color () const
            {
                return (sizeof(Color) * 8);
            }

            size_t size () const
            {
                return (buffer.size ());
            }

        public:

            void set_color (const Color & new_color)
            {
                color = new_color;
            }

            void set_color (int r, int g, int b)
            {
                color.set (r, g, b);
            }

            void set_pixel (int x, int y)
            {
                buffer[y * width + x] = color;
            }

            void set_pixel (size_t offset)
            {
                buffer[offset] = color;
            }

            void gl_draw_pixels (int raster_x, int raster_y) const
            {
                // glDrawPixels() is efficient when the driver has proper support. Otherwise it will be slow.
                // Color buffer objects could be more appropriate, but glDrawPixels() is more simple and compatible
                // within the context of this example.

                glRasterPos2i (raster_x, raster_y);
                glDrawPixels  (width, height, GL_RGB, GL_UNSIGNED_SHORT_5_6_5, colors ());
            }

        };

    }

#endif
