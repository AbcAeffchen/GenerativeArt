//
// Created by Alex Schickedanz <alex@ae.cs.uni-frankfurt.de> on 15.03.18.
//

#ifndef GENERATIVEART_CLI11DOMAIN_H
#define GENERATIVEART_CLI11DOMAIN_H

#include <CLI11.hpp>
#include <RandomFunction.h>
#include <type_traits>

template<typename T>
constexpr std::string TypeName() {
    if(std::is_same<T, int>::value)
        return "INT";
    else if(std::is_same<T, unsigned int>::value)
        return "UINT";
    else if(std::is_same<T, float>::value)
        return "FLOAT";
    else
        return "";
}

template<typename T>
CLI::Option* add_domain(CLI::App& app,
                        std::string name,
                        Domain<T>& variable,
                        std::string description = "",
                        const bool defaulted = false)
{
    CLI::callback_t reader = [&variable, name](CLI::results_t res)
    {
        T x, y;
        bool worked = CLI::detail::lexical_cast(res[0], x) && CLI::detail::lexical_cast(res[1], y);

        if(x > y)
            throw CLI::ValidationError("The min value of \"" + name + " min max\" need to be smaller or equal to the max value, but " + std::to_string(x)
                   + " is bigger than " + std::to_string(y) + ".");

        if(worked)
            variable = Domain<T>(x, y);

        return worked;
    };

    CLI::Option* opt = app.add_option(std::move(name), reader, std::move(description), defaulted);
    opt->set_custom_option(TypeName<T>() + " " + TypeName<T>(), 2);

    if(defaulted)
    {
        std::stringstream out;
        out << variable;
        opt->set_default_str(out.str());
    }

    return opt;
}

#endif //GENERATIVEART_CLI11DOMAIN_H
