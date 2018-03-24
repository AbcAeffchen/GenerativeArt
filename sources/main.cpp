//
// Created by Alex Schickedanz <alex@ae.cs.uni-frankfurt.de> on 15.03.18.
//

#include <CLI11.hpp>
#include <CLI11Domain.h>

#include "GenerativeArt.h"

constexpr int wid = 30;
constexpr int space = ' ';

void write_ini(const std::string& file_name, const std::string& settings)
{
    std::ofstream new_ini_file(file_name);
    if (new_ini_file.is_open())
    {
        new_ini_file << settings;
        new_ini_file.close();
    }
    else
    {
        throw CLI::FileError("File could not be opened");
    }
}

/**
 * Reads the setting from the command line or an ini file. If both is not given it returns a Settings object using the
 * default settings. If there is no ini file available an ini file is generated using the default settings or, if
 * requested by the settings, using the current command line settings
 * @return A settings object containing the settings from the commandline or the ini file.
 */
int read_settings(GenerativeArt::Settings& settings, int argc, char**& argv)
{
    CLI::App app("GenerativeArt is a generator for random images using random functions.\n"
                 "See https://github.com/abcaeffchen/GenerativeArt for more details.\n");

    // Program Options
    app.add_option("-s,--num-samples", settings.num_samples, "The number of samples generated.", true)
        ->group("Program Options");
    app.add_flag("-v,--verbose", settings.verbose, "Shows details about what the program is doing.")
        ->group("Program Options");
    std::string new_ini_file;
    app.add_option("-w,--write-ini", new_ini_file, "Writes the current settings into the given ini file.")
        ->configurable(false)
        ->group("Program Options");
    app.set_config("--config")
        ->group("Program Options");
    app.add_option("-o,--out", settings.directory,
               "The directory where the images are stored.", true)
        ->check(CLI::ExistingDirectory)
        ->configurable(true)
        ->group("Program Options");

    // Randomness Options
    std::string file_name;
    auto* f = app.add_option("-f,--file-name", file_name, "The file name of a sample image. "
        "This can be used to regenerate the image on a higher resolution. "
        "If used you cannot use any of the randomness options.")
        ->configurable(false)
        ->group("Randomness Options");
    auto* D = add_domain(app, "-D,--function-depth", settings.function_depth,
               "The domain the depth of the random function is drawn from.", true)
        ->configurable(true)
        ->excludes(f)
        ->group("Randomness Options");
    auto* P = add_domain(app, "-P,--function-params", settings.function_param,
               "The domain the parameters of the random function is drawn from.", true)
        ->configurable(true)
        ->excludes(f)
        ->group("Randomness Options");
    auto* d = add_domain(app, "-d,--color-poly-deg", settings.color_poly_deg,
               "The domain the degree of the random color polynomials is drawn from.", true)
        ->configurable(true)
        ->excludes(f)
        ->group("Randomness Options");
    auto* p = add_domain(app, "-p,--color-poly-params", settings.color_poly_param,
               "The domain the parameters of the random color polynomials is drawn from.", true)
        ->configurable(true)
        ->excludes(f)
        ->group("Randomness Options");
    auto* F = app.add_option("-F,--function-seed", settings.random_function_seed,
               "The seed for the random function. If provided only one image is generated.")
        ->configurable(true)
        ->excludes(f)
        ->group("Randomness Options");
    auto* C = app.add_option("-C,--color-seed", settings.color_map_seed,
               "The seed for the color functions. If provided only one image is generated.")
        ->configurable(true)
        ->excludes(f)
        ->group("Randomness Options");
    auto* u = app.add_option("--num_unary_functions", settings.unary_function_pool_size,
               "Number of unary functions available. This is just to support more functions "
               "without breaking generation of old images.", true)
        ->configurable(true)
        ->excludes(f)
        ->group("Randomness Options");
    auto* b = app.add_option("--num_binary_functions", settings.binary_function_pool_size,
               "Number of binary functions available. This is just to support more functions "
               "without breaking generation of old images.", true)
        ->configurable(true)
        ->excludes(f)
        ->group("Randomness Options");
    f->excludes(P, D, p, d, F, C, u, b);

    // Image Options
    app.add_option("-r,--resolution", settings.resolution,
                   "The resolution the image(s) are generated in.", true)
        ->configurable(true)
        ->group("Image Options");
    add_domain(app, "-x,--x-domain", settings.x,
               "Min and max values of the domain of the x dimension.", true)
        ->configurable(true)
        ->group("Image Options");
    add_domain(app, "-y,--y-domain", settings.y,
               "Min and max values of the domain of the y dimension.", true)
        ->configurable(true)
        ->group("Image Options");
    app.add_flag("-n,--no-normalization",
                 [&settings](int count){ settings.normalize = !count; },
                 "Stops normalizing the color transitions, that reduces the flickering.")
        ->configurable(true)
        ->group("Image Options");
    app.add_set("--projection-type", settings.pt, {ColorMap::projection_type::cap,
                                                   ColorMap::projection_type::periodic,
                                                   ColorMap::projection_type::smooth_periodic},
                "The way the values of the color polynomials get projected into the [0,255] range.\n"
                + std::string(wid, space) + "- Cap: x -> 255 for x > 255, x -> 0 for x < 0, x -> x else.\n"
                + std::string(wid, space) + "- Periodic: x -> x mod 256\n"
                + std::string(wid, space) + "- Smooth Periodic: x -> x mod 2 -> 255 * x^2 * (x-2)^2", true)
        ->configurable(true)
        ->group("Image Options")
        ->set_type_name("{Cap=0, Periodic=1, Smooth Periodic=2}");

    CLI11_PARSE(app, argc, argv);

    if(app.count("--file-name") > 0)
    {
        settings.read_file_name(file_name, app.count("--projection-type") > 0,
                                app.count("-x") > 0, app.count("-y") > 0);
    }

    // if both seeds are set only one image is generated
    if(settings.random_function_seed != 0 && settings.color_map_seed != 0)
    {
        settings.num_samples = 1;
        verbose(settings.verbose, "Both seeds are set, so only one image is generated.");
        if(app.count("--resolution") == 0)
        {
            settings.resolution = 4000;
            verbose(settings.verbose, "The seed was set without a resolution. The resolution was set to 4000px.");
        }
    }

    // if the seed is set without a resolution, the resolution will be set to 3840px.

    // print settings used
    verbose(settings.verbose, "Settings:\n" + app.config_to_str(true));

    // show if a config file is loaded
    if(app.count("--config"))
        verbose(settings.verbose, "ini file loaded: " + app.get_config_ptr()->results()[0]);

    // write current settings to ini file if needed
    if(new_ini_file.length() > 0)
    {
        try
        {
            write_ini(new_ini_file, app.config_to_str(true, "", true));
            verbose(settings.verbose, "Current settings written to " + new_ini_file);
        }
        catch(const CLI::FileError& e)
        {
            app.exit(e);
        }
    }

    return 0;
}

int main(int argc, char** argv)
{
    GenerativeArt::Settings settings;

    // parse the CLI
    int cli_response = read_settings(settings, argc, argv);

    // if the CLI parser failed for some reason the program must exit here.
    if(cli_response != 0)
        return cli_response;

    GenerativeArt ga(settings);

    unsigned int num_empty_images = 0;

    for(unsigned int i = 0; i < settings.num_samples; /* i is set by the generator */)
    {
        if(ga.generate())
        {
            i++;
            verbose(settings.verbose, "Progress: " + std::to_string(i) + " / " + std::to_string(settings.num_samples));
        }
        else
        {
            num_empty_images++;
            if(num_empty_images > 20 * (i + 1))
            {
                verbose(settings.verbose, "There where more than 20 empty images per valid image.");
                break;
            }
        }
    }

    return 0;
}