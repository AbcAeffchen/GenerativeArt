//
// Created by Alex Schickedanz <alex@ae.cs.uni-frankfurt.de> on 25.05.17.
//

#ifndef GENERATIVEART_IMAGE_GENERATOR_H
#define GENERATIVEART_IMAGE_GENERATOR_H

#include "random_function.h"
#include "color_map.h"
#include <png.hpp>
#include <omp.h>

class image_generator
{
    const random_function& rf;
    const color_map& cm;

public:
    image_generator(const random_function& rf, const color_map& cm)
        : rf(rf), cm(cm)
    {}

    png::image<png::rgb_pixel> compute_image(const std::pair<int, int> domain_x, const std::pair<int, int> domain_y, const unsigned int resolution) const
    {
        png::image<png::rgb_pixel> image((domain_x.second - domain_x.first) * resolution,
                                         (domain_y.second - domain_y.first) * resolution);
        // todo try to vectorize this
        const int x_offset = domain_x.first * resolution;
        const int y_offset = domain_y.first * resolution;

#pragma omp parallel for
        for(png::uint_32 y = 0; y < image.get_height(); y++)
        {
            for(png::uint_32 x = 0; x < image.get_width(); x++)
            {
                argument_type z = rf.eval((static_cast<argument_type>(x) + static_cast<argument_type>(x_offset)) / static_cast<argument_type>(resolution),
                                   (static_cast<argument_type>(y) + static_cast<argument_type>(y_offset)) / static_cast<argument_type>(resolution));
                uint8_t r, g, b;
                cm.get_color(z, r, g, b);

                image[y][x] = png::rgb_pixel(r, g, b);
            }
        }

        return image;
    }

};

/**
 * Wrapper to ensure identical random number generator calls.
 */
void generate_image(const function_pool& fp, const unsigned int function_seed,
                    const unsigned color_seed, const std::string& dir, unsigned int num_unary_functions,
                    unsigned int num_binary_function, unsigned int resolution)
{
    std::default_random_engine function_prng(function_seed);
    random_function rf(fp, function_prng, num_unary_functions, num_binary_function);

    std::default_random_engine color_prng(color_seed);
    polynomial_color_map cm(color_prng);

    image_generator gen(rf, cm);
    auto image = gen.compute_image({0, 1}, {0, 1}, resolution);

    std::string filename = dir + std::to_string(fp.unary_pool.size()) + "-"
                           + std::to_string(fp.binary_pool.size()) + "-"
                           + std::to_string(function_seed) + "-"
                           + std::to_string(color_seed) + "-"
                           + std::to_string(resolution) + ".png";

    image.write(filename);
}

void generate_image(const function_pool& fp, const unsigned int function_seed,
                    const unsigned color_seed, const std::string& dir, unsigned int resolution = 200)
{
    generate_image(fp, function_seed, color_seed, dir, static_cast<unsigned int>(fp.unary_pool.size()),
                   static_cast<unsigned int>(fp.binary_pool.size()), resolution);
}


#endif //GENERATIVEART_IMAGE_GENERATOR_H
