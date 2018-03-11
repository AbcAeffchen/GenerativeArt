//
// Created by Alex Schickedanz <alex@ae.cs.uni-frankfurt.de> on 25.05.17.
//

#ifndef GENERATIVEART_RANDOM_FUNCTION_TREE_H
#define GENERATIVEART_RANDOM_FUNCTION_TREE_H

#include "function_pool.h"
#include <random>
#include <limits>
#include <algorithm>

polynomial get_random_polynomial(const unsigned int degree, std::default_random_engine& prng)
{
    std::uniform_real_distribution<argument_type> param_dist(-1.0f, 1.0f);
    std::vector<argument_type> poly;
    poly.resize(degree);
    for(unsigned int i = 0; i <= degree; i++)
        poly[i] = param_dist(prng);

    return polynomial(poly);
}

class random_function
{
    enum function_type {unary, binary, terminal_value, terminal_index};

    const function_pool& fp;

    function_type type;
    unsigned int function_index;
    argument_type opt_val = 0.0;

    random_function* child_function_1 = nullptr;
    random_function* child_function_2 = nullptr;

public:
    /**
     *
     * @param fp The function pool to choose from
     * @param prng The seeded pseudo random number generator
     * @param depth The depth of the function call tree. If zero or not provided, a depth gets randomly picked between 10 and 50.
     * @param num_unary_functions The max number of random unary functions that should be used (For reproducability)
     * @param num_binary_functions The max number of random binary functions that should be used (For reproducability)
     * @param num_ternary_functions The max number of random ternary functions that should be used (For reproducability)
     */
    random_function(const function_pool& fp, std::default_random_engine& prng,
                    const unsigned int num_unary_functions = std::numeric_limits<unsigned int>::max(),
                    const unsigned int num_binary_functions = std::numeric_limits<unsigned int>::max(),
                    unsigned int depth = 0)
        : fp(fp)
    {
        if(depth < 1)
        {
            std::uniform_int_distribution<unsigned int> depth_dist(4, 8);  // todo try some values
            depth = depth_dist(prng);
        }

        std::uniform_int_distribution<unsigned int> unary_dist(0, std::min(static_cast<unsigned int>(fp.unary_pool.size()), num_unary_functions)-1);
        std::uniform_int_distribution<unsigned int> binary_dist(0, std::min(static_cast<unsigned int>(fp.binary_pool.size()), num_unary_functions)-1);
        std::uniform_int_distribution<unsigned int> int_dist(0, 99);
        std::uniform_real_distribution<argument_type> real_dist(0.0, 1.0);

        unsigned int r = int_dist(prng);
        unsigned int d = int_dist(prng);

        if(depth == 1)
        {
            if(r < 10)
            {
                type = terminal_value;
                opt_val = real_dist(prng);
            }
            else
            {
                type = terminal_index;
                function_index = r % 2;
            }
        }
        else
        {
            if(r < 70)
            {
                type = binary;
                function_index = binary_dist(prng);
                child_function_1 = new random_function(fp, prng, num_unary_functions, num_binary_functions, depth - 1);
                child_function_2 = new random_function(fp, prng, num_unary_functions, num_binary_functions, depth - 1);
            }
            else
            {
                type = unary;
                function_index = unary_dist(prng);
                child_function_1 = new random_function(fp, prng, num_unary_functions, num_binary_functions, depth - 1);
            }

        }
    }

    ~random_function()
    {
        delete child_function_1;
        delete child_function_2;
    }

    argument_type eval(const argument_type x, const argument_type y) const
    {
        switch(type)
        {
            case unary: return fp.unary_pool[function_index](child_function_1->eval(x, y));
            case binary: return fp.binary_pool[function_index](child_function_1->eval(x, y),
                                                               child_function_2->eval(x, y));
            case terminal_value: return opt_val;
            default: return function_index ? x : y;
        }
    }
};


#endif //GENERATIVEART_RANDOM_FUNCTION_TREE_H
