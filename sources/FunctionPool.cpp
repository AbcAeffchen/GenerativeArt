//
// Created by Alex Schickedanz <alex@ae.cs.uni-frankfurt.de> on 23.03.18.
//

#include <FunctionPool.h>

const std::vector<FunctionPool::unary_function> FunctionPool::unary = {
    sinf, cosf, expf, logf, sinhf, coshf, tanhf, fabsf, sqrt,
    [](const argument_type a){ return a; },    // identity to reduce randomly the depth
    [](const argument_type a){ return -a; },
    [](const argument_type a){ return a*a; },
    [](const argument_type a){ return a*a*a; }
    // add more functions here, else older images cannot be reproduced
};

const std::vector<FunctionPool::unary_function_discription> FunctionPool::unary_string = {
    {"sin(", ")"},
    {"cos(", ")"},
    {"exp(", ")"},
    {"log(", ")"},
    {"sinh(", ")"},
    {"cosh(", ")"},
    {"tanh(", ")"},
    {"|", "|"},
    {"sqrt(", ")"},
    {"", ""},
    {"-", ""},
    {"(", ")^2"},
    {"(", ")^3"}
};

const std::vector<FunctionPool::binary_function> FunctionPool::binary = {
    [](const argument_type a, const argument_type b){ return a + b; },
    [](const argument_type a, const argument_type b){ return a - b; },
    [](const argument_type a, const argument_type b){ return a * b; },
    [](const argument_type a, const argument_type b){ return sin(a * b); }
    // add more functions here, else older images cannot be reproduced
};

const std::vector<FunctionPool::binary_function_discription> FunctionPool::binary_string = {
    std::make_tuple("(", " + ", ")"),
    std::make_tuple("(", " - ", ")"),
    std::make_tuple("(", " * ", ")"),
    std::make_tuple("sin(", " * ", ")")
};






