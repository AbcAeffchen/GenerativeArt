//
// Created by Alex Schickedanz <alex@ae.cs.uni-frankfurt.de> on 25.05.17.
//

#include "random_function.h"
#include "color_map.h"
#include "image_generator.h"

#include <iostream>
#include <random>
#include <string>
#include <omp.h>

int main(int argc, char* args[])
{
    std::string dir = "images/";

    unsigned int num_samples = 100;
    unsigned int resolution = 200;

    if(argc > 1)
        num_samples = static_cast<unsigned int>(atoi(args[1]));
    if(argc > 2)
        resolution = static_cast<unsigned int>(atoi(args[2]));

    std::random_device rd;
    const function_pool fp = function_pool();

    auto start = omp_get_wtime();

    for(unsigned int i = 0; i < num_samples; i++)
    {
        // todo make the domain arbitrary (not only [0,1])
        generate_image(fp, rd(), rd(), "images/", resolution);
    }

    std::cout <<  omp_get_wtime() - start << std::endl;

    return 0;
}