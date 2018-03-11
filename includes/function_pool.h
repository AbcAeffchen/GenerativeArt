//
// Created by Alex Schickedanz <alex@ae.cs.uni-frankfurt.de> on 25.05.17.
//

#ifndef GENERATIVEART_FUNCTION_POOL_H
#define GENERATIVEART_FUNCTION_POOL_H

#include <utility>
#include <vector>
#include <functional>
#include <iostream>
#include <cmath>

using argument_type = float;

class polynomial
{
    const std::vector<argument_type> poly;

public:
    explicit polynomial(std::vector<argument_type> poly)
        : poly(std::move(poly))
    {}

    /**
     * Evaluates the polynomial in x using Horner's scheme
     * @param arg
     * @return
     */
    argument_type eval(argument_type x) const
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
};

struct function_pool
{
    using unary_function = std::function<argument_type(const argument_type)>;
    using binary_function = std::function<argument_type(const argument_type, const argument_type)>;

    const std::vector<unary_function> unary_pool{sinf, cosf, expf, logf, sinhf, coshf, tanhf, fabsf, sqrt,
                                                 [](const argument_type a){ return -a; },
                                                 [](const argument_type a){ return a*a; },
                                                 [](const argument_type a){ return a*a*a; }
                                                 // add more functions here
    };

    const std::vector<binary_function> binary_pool{[](const argument_type a, const argument_type b){ return (a + b) / 2.0; },
                                                   [](const argument_type a, const argument_type b){ return a - b; },
                                                   [](const argument_type a, const argument_type b){ return a * b; },
                                                   [](const argument_type a, const argument_type b){ return sin(a * b); }
                                                   // add more functions here
    };

};

#endif //GENERATIVEART_FUNCTION_POOL_H
