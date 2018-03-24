//
// Created by Alex Schickedanz <alex@ae.cs.uni-frankfurt.de> on 25.05.17.
//

#ifndef GENERATIVEART_FUNCTION_POOL_H
#define GENERATIVEART_FUNCTION_POOL_H

#include <vector>
#include <functional>
#include <cmath>

using argument_type = float;

struct FunctionPool
{
    using unary_function = std::function<argument_type(const argument_type)>;
    using binary_function = std::function<argument_type(const argument_type, const argument_type)>;

    const static std::vector<unary_function> unary;

    const static std::vector<binary_function> binary;
};

#endif //GENERATIVEART_FUNCTION_POOL_H
