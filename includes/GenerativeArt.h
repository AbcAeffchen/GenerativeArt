//
// Created by Alex Schickedanz <alex@ae.cs.uni-frankfurt.de> on 25.05.17.
//

// todo replace all include guards by pragma once
#ifndef GENERATIVEART_IMAGE_GENERATOR_H
#define GENERATIVEART_IMAGE_GENERATOR_H

#include <CLI11.hpp>

#include <png.hpp>
#include <omp.h>
#include <algorithm>

#include "RandomFunction.h"
#include "ColorMap.h"

#include <unordered_map>

// Helper to make verbose easier.
void verbose(bool on, const std::string& msg, bool new_line = true)
{
    if(on)
    {
        std::cout << msg;
        if(new_line)
            std::cout << std::endl;
    }
}

constexpr char delimiter = '.';

constexpr uint32_t pos_to_index(uint32_t x, uint32_t y, uint32_t max_x, uint32_t /*max_y*/)
{
    return y * max_x + x;
//    return x * max_y + y;
}

constexpr uint32_t close_to_white(uint8_t r, uint8_t g, uint8_t b)
{
    return static_cast<uint32_t>(r > 240 && g > 240 && b > 240);
}

constexpr uint32_t close_to_black(uint8_t r, uint8_t g, uint8_t b)
{
    return static_cast<uint32_t>(r < 15 && g < 15 && b < 15);
}

constexpr uint32_t colors_to_int(uint8_t r, uint8_t g, uint8_t b)
{
    return static_cast<uint32_t>(r) << 16
           | static_cast<uint32_t>(g) << 8
           | static_cast<uint32_t>(b);
}

constexpr float accuracy = 100.f;

constexpr int float_to_int(float a)
{
    return static_cast<int>(a * accuracy);
}

template<typename T>
std::tuple<double, double, double> get_color_variances(const std::vector<T>& colors,
                                                       double mean_r, double mean_g, double mean_b)
{
    const size_t num_pixels =  colors.size() / 3;

    double var_r = 0.0,
           var_g = 0.0,
           var_b = 0.0;

#pragma omp parallel for reduction(+:var_r,var_g,var_b)
    for(size_t i = 0; i < num_pixels; i++)
    {
        var_r += pow(colors[3 * i] - mean_r, 2.0);
        var_g += pow(colors[3 * i + 1] - mean_g, 2.0);
        var_b += pow(colors[3 * i + 2] - mean_b, 2.0);
    }

    var_r /= num_pixels;
    var_g /= num_pixels;
    var_b /= num_pixels;

    return std::tie(var_r, var_g, var_b);
}

class GenerativeArt
{
public:
    struct Settings
    {
        // ------------------------------------------------------
        // Settings unrelated to the image
        // ------------------------------------------------------

        bool verbose = false;

        std::string directory = "images/";

        bool generate_all_color_permutations = false;

        unsigned int num_samples = 100;

        unsigned int random_function_seed = 0;
        unsigned int color_map_seed = 0;

        // ------------------------------------------------------
        // Settings for the image generator
        // ------------------------------------------------------

        // depth of the random function
        Domain<unsigned int> function_depth = {4, 7};
        Domain<argument_type> function_param = {1.f, 1.9f};

        // color map settings
        Domain<unsigned int> color_poly_deg = {2, 3};
        Domain<argument_type> color_poly_param = {-96.f, 96.f};

        ColorMap::projection_type pt = ColorMap::projection_type::cap;

        size_t unary_function_pool_size = FunctionPool::unary.size();
        size_t binary_function_pool_size = FunctionPool::binary.size();

        // normalize the colors
        bool normalize = true;

        // image settings. Images dimensions are max_x * resolution x max_y * resolution.
        Domain<argument_type> x = {0.f, 1.f};
        Domain<argument_type> y = {0.f, 1.f};
        unsigned int resolution = 200;

        Settings() = default;

        void read_file_name(const std::string& file_name, bool include_pt, bool include_x, bool include_y)
        {
            // split string on delimiter into a vector
            const size_t num_options = 17;
            std::vector<std::string> settings(num_options);

            const auto end_of_file_name = file_name.find("png");

            for(size_t i = 0, last = 0, next = 0; i < num_options; i++)
            {
                next = file_name.find(delimiter, last);
                if(next >= end_of_file_name)
                {
                    // the last item and ".png" is not part of the file name
                    if(i == num_options - 1 && end_of_file_name == std::string::npos)
                    {
                        settings[i] = file_name.substr(last); // use the rest of the file name for the last option
                        break;
                    }
                    // else either more than one option is missing or the file extension is included.
                    throw CLI::ValidationError("The file name is not valid.");
                }
                settings[i] = file_name.substr(last, next - last);

                last = next + 1;
            }

            // see get_file_name() for the order of the settings.
            random_function_seed = static_cast<unsigned int>(stoul(settings[0]));
            color_map_seed = static_cast<unsigned int>(stoul(settings[1]));
            unary_function_pool_size = stoul(settings[2]);
            binary_function_pool_size = stoul(settings[3]);
            function_depth = {
                static_cast<unsigned int>(stoul(settings[4])),
                static_cast<unsigned int>(stoul(settings[5]))
            };
            function_param = {
                static_cast<argument_type>(stoi(settings[6])) / accuracy,
                static_cast<argument_type>(stoi(settings[7])) / accuracy
            };
            color_poly_deg = {
                static_cast<unsigned int>(stoul(settings[8])),
                static_cast<unsigned int>(stoul(settings[9]))
            };
            color_poly_param = {
                static_cast<argument_type>(stoi(settings[10])) / accuracy,
                static_cast<argument_type>(stoi(settings[11])) / accuracy
            };

            if(include_pt)
                pt = static_cast<ColorMap::projection_type>(stoul(settings[12]));

            if(include_x)
                x = {
                    static_cast<argument_type>(stoi(settings[13])) / accuracy,
                    static_cast<argument_type>(stoi(settings[14])) / accuracy
                };

            if(include_y)
                y = {
                    static_cast<argument_type>(stoi(settings[15])) / accuracy,
                    static_cast<argument_type>(stoi(settings[16])) / accuracy
                };
        }

        /**
         * Return all data necessary to regenerate the image.
         * @return An file name without extension
         */
        std::string get_file_name(unsigned int function_seed, unsigned int color_seed) const
        {
            // File name gets long but should not be to long. The limit is 255 on most OS.
            // The file name should be about 2 * 10 + 15 * 3 + 4 characters long.
            return std::to_string(function_seed) + delimiter +
                   std::to_string(color_seed) + delimiter +
                   std::to_string(unary_function_pool_size) + delimiter +
                   std::to_string(binary_function_pool_size) + delimiter +
                   std::to_string(function_depth.min) + delimiter +
                   std::to_string(function_depth.max) + delimiter +
                   std::to_string(float_to_int(function_param.min)) + delimiter +
                   std::to_string(float_to_int(function_param.max)) + delimiter +
                   std::to_string(color_poly_deg.min) + delimiter +
                   std::to_string(color_poly_deg.max) + delimiter +
                   std::to_string(float_to_int(color_poly_param.min)) + delimiter +
                   std::to_string(float_to_int(color_poly_param.max)) + delimiter +
                   std::to_string(pt) + delimiter +
                   std::to_string(float_to_int(x.min)) + delimiter +
                   std::to_string(float_to_int(x.max)) + delimiter +
                   std::to_string(float_to_int(y.min)) + delimiter +
                   std::to_string(float_to_int(y.max));
        }
    };

private:
    const Settings& settings;

public:
    explicit GenerativeArt(const Settings& settings)
        : settings(settings)
    {}

    bool generate() const
    {
        // todo random device is used to seed the random number generators. This does maybe not work on some systems...
        std::random_device rd;

        unsigned int function_seed = settings.random_function_seed ? settings.random_function_seed : rd();
        unsigned int color_seed = settings.color_map_seed ? settings.color_map_seed : rd();

        verbose(settings.verbose, "Function Seed: " + std::to_string(function_seed));
        verbose(settings.verbose, "Color Seed:    " + std::to_string(color_seed));

        // draw the random function
        std::default_random_engine function_prng(function_seed);
        std::uniform_int_distribution<unsigned int> depth_dist(settings.function_depth.min,
                                                               settings.function_depth.max);

        const RandomFunction rf(function_prng, depth_dist(function_prng),
                                settings.function_param,
                                settings.unary_function_pool_size,
                                settings.binary_function_pool_size);

        verbose(settings.verbose, "Function:\nf = " + rf.print());
        verbose(settings.verbose, "depth: " + std::to_string(rf.get_depth()));

        // draw the color map
        std::default_random_engine color_prng(color_seed);

        const PolynomialColorMap cm(color_prng,
                                    settings.pt,
                                    settings.color_poly_deg,
                                    settings.color_poly_param);

        verbose(settings.verbose, "Color:\n" + cm.print());

        // prepare for generation
        const auto dim_x = static_cast<uint32_t>((settings.x.max - settings.x.min) * settings.resolution);
        const auto dim_y = static_cast<uint32_t>((settings.y.max - settings.y.min) * settings.resolution);
        const auto num_pixels = dim_x * dim_y;
        const auto step_size = 1.f / static_cast<argument_type>(settings.resolution);

        std::vector<argument_type> values(num_pixels);

#pragma omp parallel for
        for(uint32_t y_px = 0; y_px < dim_y; y_px++)
        {
            const argument_type y = static_cast<argument_type>(y_px) * step_size + settings.y.min;
            for(uint32_t x_px = 0; x_px < dim_x; x_px++)
            {
                const argument_type x = static_cast<argument_type>(x_px) * step_size + settings.x.min;
                values[pos_to_index(x_px, y_px, dim_x, dim_y)] = rf.eval(x, y);
            }
        }

        std::vector<uint8_t> colors(num_pixels * 3);

        const auto normalize = settings.normalize;

        // statistics
        uint32_t acc_r = 0, acc_g = 0, acc_b = 0;
        uint8_t  min_r = 255, min_g = 255, min_b = 255;
        uint8_t  max_r = 0, max_g = 0, max_b = 0;

        uint32_t white = 0,
                 black = 0;

#pragma omp parallel for reduction(+:acc_r,acc_g,acc_b,white,black) reduction(min:min_r,min_g,min_b) reduction(max:max_r,max_g,max_b)
        for(uint32_t y_px = 0; y_px < dim_y; y_px++)
        {
            for(uint32_t x_px = 0; x_px < dim_x; x_px++)
            {
                const auto i = pos_to_index(x_px, y_px, dim_x, dim_y);

                uint8_t r, g, b;
                cm.get_color(values[i], r, g, b);

                colors[3 * i] = r;
                colors[3 * i + 1] = g;
                colors[3 * i + 2] = b;

                // prepare statistics
                white += close_to_white(r, g, b);
                black += close_to_black(r, g, b);
                acc_r += r;
                acc_g += g;
                acc_b += b;
                min_r = std::min(min_r, r);
                min_g = std::min(min_g, g);
                min_b = std::min(min_b, b);
                max_r = std::max(max_r, r);
                max_g = std::max(max_g, g);
                max_b = std::max(max_b, b);
            }
        }

        verbose(settings.verbose, "white pixels: " + std::to_string(static_cast<double>(white) / num_pixels));
        verbose(settings.verbose, "black pixels: " + std::to_string(static_cast<double>(black) / num_pixels));

        double mean_r = static_cast<double>(acc_r) / num_pixels;
        double mean_g = static_cast<double>(acc_g) / num_pixels;
        double mean_b = static_cast<double>(acc_b) / num_pixels;

        // find single color images
        if((max_r - min_r < 5 && max_g - min_g < 5 && max_b - min_b < 5)
            || white > num_pixels * 0.85 || black > num_pixels * 0.85)
        {
            verbose(settings.verbose, "Single color image -> trying again");
            return false;
        }

        double var_r, var_g, var_b;

        std::tie(var_r, var_g, var_b) = get_color_variances(colors, mean_r, mean_g, mean_b);

        if(var_r < 0.01 && var_g < 0.01 && var_b < 0.01)
        {
            verbose(settings.verbose, "Single color image -> trying again");
            return false;
        }

        if(normalize)
        {
            verbose(settings.verbose, "Normalizing:\n"
                "min:  " + std::to_string(min_r) + ", " + std::to_string(min_g) + ", " + std::to_string(min_b) + "\n" +
                "max:  " + std::to_string(max_r) + ", " + std::to_string(max_g) + ", " + std::to_string(max_b) + "\n" +
                "mean: " + std::to_string(mean_r) + ", " + std::to_string(mean_g) + ", " + std::to_string(mean_b) + "\n" +
                "var:  " + std::to_string(var_r) + ", " + std::to_string(var_g) + ", " + std::to_string(var_b));

#pragma omp parallel for
            for (size_t i = 0; i < num_pixels; i++)
            {
                if(var_r < 0.001)
                    colors[i * 3 + 0] = static_cast<uint8_t>((static_cast<double>(colors[i * 3 + 0]) - mean_r) / (var_r / 3));
                if(var_g < 0.001)
                    colors[i * 3 + 1] = static_cast<uint8_t>((static_cast<double>(colors[i * 3 + 1]) - mean_g) / (var_g / 3));
                if(var_b < 0.001)
                    colors[i * 3 + 2] = static_cast<uint8_t>((static_cast<double>(colors[i * 3 + 2]) - mean_b) / (var_b / 3));
            }
        }

        // calculate the image from the color values.
        if(settings.generate_all_color_permutations)
            store_image_color_permutaions(dim_x, dim_y, function_seed, color_seed, colors);
        else
            store_image(dim_x, dim_y, function_seed, color_seed, colors);

        return true;
    }

private:
    void store_image(const uint32_t dim_x, const uint32_t dim_y,
                     const unsigned int function_seed, const unsigned int color_seed,
                     const std::vector<uint8_t>& colors) const
    {
        png::image<png::rgb_pixel> image(dim_x, dim_y);

#pragma omp parallel for
        for(png::uint_32 y_px = 0; y_px < image.get_height(); y_px++)
        {
            for(png::uint_32 x_px = 0; x_px < image.get_width(); x_px++)
            {
                const auto i = 3 * pos_to_index(static_cast<uint32_t>(x_px), static_cast<uint32_t>(y_px), dim_x, dim_y);
                image[y_px][x_px] = png::rgb_pixel(colors[i], colors[i + 1], colors[i + 2]);
            }
        }

        image.write(settings.directory + settings.get_file_name(function_seed, color_seed) + ".png");
    }

    void store_image_color_permutaions(const uint32_t dim_x, const uint32_t dim_y,
                                       const unsigned int function_seed, const unsigned int color_seed,
                                       const std::vector<uint8_t>& colors) const
    {
        png::image<png::rgb_pixel> image1(dim_x, dim_y);
        png::image<png::rgb_pixel> image2(dim_x, dim_y);
        png::image<png::rgb_pixel> image3(dim_x, dim_y);
        png::image<png::rgb_pixel> image4(dim_x, dim_y);
        png::image<png::rgb_pixel> image5(dim_x, dim_y);
        png::image<png::rgb_pixel> image6(dim_x, dim_y);

#pragma omp parallel for
        for(png::uint_32 y_px = 0; y_px < image1.get_height(); y_px++)
        {
            for(png::uint_32 x_px = 0; x_px < image1.get_width(); x_px++)
            {
                const auto i = 3 * pos_to_index(static_cast<uint32_t>(x_px), static_cast<uint32_t>(y_px), dim_x, dim_y);
                image1[y_px][x_px] = png::rgb_pixel(colors[i], colors[i + 1], colors[i + 2]);
                image2[y_px][x_px] = png::rgb_pixel(colors[i], colors[i + 2], colors[i + 1]);
                image3[y_px][x_px] = png::rgb_pixel(colors[i + 1], colors[i + 0], colors[i + 2]);
                image4[y_px][x_px] = png::rgb_pixel(colors[i + 1], colors[i + 2], colors[i + 0]);
                image5[y_px][x_px] = png::rgb_pixel(colors[i + 2], colors[i + 1], colors[i + 0]);
                image6[y_px][x_px] = png::rgb_pixel(colors[i + 2], colors[i + 0], colors[i + 1]);
            }
        }

        image1.write(settings.directory + settings.get_file_name(function_seed, color_seed) + ".1.png");
        image2.write(settings.directory + settings.get_file_name(function_seed, color_seed) + ".2.png");
        image3.write(settings.directory + settings.get_file_name(function_seed, color_seed) + ".3.png");
        image4.write(settings.directory + settings.get_file_name(function_seed, color_seed) + ".4.png");
        image5.write(settings.directory + settings.get_file_name(function_seed, color_seed) + ".5.png");
        image6.write(settings.directory + settings.get_file_name(function_seed, color_seed) + ".6.png");
    }
};

#endif //GENERATIVEART_IMAGE_GENERATOR_H
