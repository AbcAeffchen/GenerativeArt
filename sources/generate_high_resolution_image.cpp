//
// Created by Alex Schickedanz <alex@ae.cs.uni-frankfurt.de> on 25.05.17.
//

#include "random_function.h"
#include "color_map.h"
#include "image_generator.h"

#include <iostream>
#include <random>
#include <string>
#include <sstream>
#include <vector>
#include <iterator>
#include <omp.h>

template<typename Out>
void split(const std::string& s, char delim, Out result) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}

std::vector<unsigned int> get_params(const std::string& s) {
    std::vector<std::string> elems;
    split(s, '-', std::back_inserter(elems));

    std::vector<unsigned int> params(4);
    for(unsigned int i = 0; i < 4; i++)
    {
        params[i] = static_cast<unsigned int>(std::stoul(elems[i]));
    }

    return params;
}

int main(int argc, char* args[])
{
    std::random_device rd;
    const function_pool fp = function_pool();

    std::string dir = "images/the-not-so-bad-ones/";

    unsigned int num_unary_functions = static_cast<unsigned int>(fp.unary_pool.size());
    unsigned int num_binary_functions = static_cast<unsigned int>(fp.binary_pool.size());
    auto function_seed = rd();
    auto color_seed = rd();
    unsigned int resolution = 3840;

    if(argc < 5)
    {
        auto params = get_params(static_cast<std::string>(args[1]));
        num_unary_functions = params[0];
        num_binary_functions = params[1];
        function_seed = params[2];
        color_seed = params[3];

        if(argc > 2)
            resolution = static_cast<unsigned int>(atoi(args[2]));
    }
    else
    {
        if(argc > 1)
            num_unary_functions = static_cast<unsigned int>(atoi(args[1]));

        if(argc > 2)
            num_binary_functions = static_cast<unsigned int>(atoi(args[2]));

        if(argc > 3)
            function_seed = static_cast<unsigned int>(atoi(args[3]));

        if(argc > 4)
            color_seed = static_cast<unsigned int>(atoi(args[4]));

        if(argc > 5)
            resolution = static_cast<unsigned int>(atoi(args[5]));
    }

    // todo support the domain

    auto start = omp_get_wtime();

    generate_image(fp, function_seed, color_seed, dir, num_unary_functions, num_binary_functions, resolution);

    std::cout <<  omp_get_wtime() - start << std::endl;

    return 0;
}