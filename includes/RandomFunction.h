//
// Created by Alex Schickedanz <alex@ae.cs.uni-frankfurt.de> on 25.05.17.
//

#ifndef GENERATIVEART_RANDOM_FUNCTION_TREE_H
#define GENERATIVEART_RANDOM_FUNCTION_TREE_H

#include "FunctionPool.h"
#include <random>
#include <limits>
#include <algorithm>
#include <cassert>

template<typename T>
struct Domain
{
    T min;
    T max;

    explicit Domain(T a) : min(std::min(-a, a)), max(std::max(-a, a)) {}

    Domain(T a, T b) : min(std::min(a, b)), max(std::max(a, b)) {}

    explicit Domain(const std::vector<T>& a)
        : min(std::min(a[0], a[1])), max(std::max(a[0], a[1]))
    {}

    friend std::ostream& operator<< (std::ostream& out, const Domain<T>& a)
    {
        return out << a.min << " " << a.max;
    }
};

class RandomPolynomial
{
    const std::vector<argument_type> poly;

public:
    explicit RandomPolynomial(std::vector<argument_type> poly)
        : poly(std::move(poly))
    {}

    static RandomPolynomial get(const Domain<unsigned int>& degree_domain, const Domain<float>& param_domain, std::default_random_engine& prng)
    {
        std::vector<argument_type> poly;
        std::uniform_int_distribution<uint8_t> degree_dist(static_cast<uint8_t>(degree_domain.min),
                                                           static_cast<uint8_t>(degree_domain.max));
        std::uniform_real_distribution<argument_type> param_dist(param_domain.min, param_domain.max);
        uint8_t degree = degree_dist(prng);
        poly.resize(degree);
        for(unsigned int i = 0; i <= degree; i++)
            poly[i] = param_dist(prng);

        return RandomPolynomial(poly);
    }

    /**
     * Evaluates the polynomial in x using Horner's scheme
     * @param arg
     * @return
     */
    argument_type eval(const argument_type x) const
    {
        argument_type result = poly[0];

        // Horner's scheme
        for(unsigned int i = 1; i < poly.size(); i++)
        {
            result *= x;
            result += poly[i];
        }

        return result;
    }

    std::string print() const
    {
        std::string description = std::to_string(poly[0]);

        for(unsigned int i = 1; i < poly.size(); i++)
        {
            description += " x^" + std::to_string(poly.size() - i) + " + " + std::to_string(poly[i]);
        }

        return description;
    }
};

class RandomFunction
{
    enum function_type {unary, binary, terminal_index};

    const unsigned int depth;

    function_type type;
    unsigned int function_index;
    argument_type param = 1.0;

    RandomFunction* child_function_1 = nullptr;
    RandomFunction* child_function_2 = nullptr;

public:
    // todo fix all doc comments (add the missing ones)
    /**
     *
     * @param fp The function pool to choose from
     * @param prng The seeded pseudo random number generator
     * @param depth The depth of the function call tree. If zero or not provided, a depth gets randomly picked between 10 and 50.
     * @param num_unary_functions The max number of random unary functions that should be used (For reproducability)
     * @param num_binary_functions The max number of random binary functions that should be used (For reproducability)
     * @param num_ternary_functions The max number of random ternary functions that should be used (For reproducability)
     */
    RandomFunction(std::default_random_engine& prng, unsigned int depth,
                   const Domain<float>& param_domain,
                   const size_t num_unary_functions = FunctionPool::unary.size(),
                   const size_t num_binary_functions = FunctionPool::binary.size())
        : depth(depth)
    {
        assert(depth > 0);

        std::uniform_int_distribution<unsigned int> unary_dist(0, static_cast<unsigned int>(std::min(FunctionPool::unary.size(), num_unary_functions))-1);
        std::uniform_int_distribution<unsigned int> binary_dist(0, static_cast<unsigned int>(std::min(FunctionPool::binary.size(), num_unary_functions))-1);
        std::uniform_int_distribution<unsigned int> int_dist(0, 99);
        std::uniform_real_distribution<argument_type> param_dist(param_domain.min, param_domain.max);

        const unsigned int r = int_dist(prng);

        param = param_dist(prng);

        if(depth > 1)
        {
            if(r < 70)
            {
                type = binary;
                function_index = binary_dist(prng);
                child_function_1 = new RandomFunction(prng, depth - 1, param_domain, num_unary_functions, num_binary_functions);
                child_function_2 = new RandomFunction(prng, depth - 1, param_domain, num_unary_functions, num_binary_functions);
            }
            else
            {
                type = unary;
                function_index = unary_dist(prng);
                child_function_1 = new RandomFunction(prng, depth - 1, param_domain, num_unary_functions, num_binary_functions);
            }
        }
        else
        {
            type = terminal_index;
            function_index = r % 2;
        }
    }

    ~RandomFunction()
    {
        delete child_function_1;
        delete child_function_2;
    }

    argument_type eval(const argument_type x, const argument_type y) const
    {
        switch(type)
        {
            case unary:  return param * FunctionPool::unary[function_index]( child_function_1->eval(x, y));
            case binary: return param * FunctionPool::binary[function_index](child_function_1->eval(x, y),
                                                                     child_function_2->eval(x, y));
            default: return param * (function_index ? x : y);
        }
    }

    std::string print() const
    {
        std::string description = std::to_string(param) + " * ";
        switch(type)
        {
            case unary: return description
                    + FunctionPool::unary_string[function_index].first
                    + child_function_1->print()
                    + FunctionPool::unary_string[function_index].second;
            case binary: return description
                    + std::get<0>(FunctionPool::binary_string[function_index])
                    + child_function_1->print()
                    + std::get<1>(FunctionPool::binary_string[function_index])
                    + child_function_2->print()
                    + std::get<2>(FunctionPool::binary_string[function_index]);
            default: return description + (function_index ? "x" : "y");
        }
    }

    unsigned int get_depth() const
    {
        return depth;
    }
};

#endif //GENERATIVEART_RANDOM_FUNCTION_TREE_H
