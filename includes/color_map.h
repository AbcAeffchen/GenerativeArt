//
// Created by Alex Schickedanz <alex@ae.cs.uni-frankfurt.de> on 25.05.17.
//

#ifndef GENERATIVEART_RANDOM_COLOR_MAP_H
#define GENERATIVEART_RANDOM_COLOR_MAP_H

#include <vector>
#include <random>
#include <cmath>

/**
 * Color map interface
 */
class color_map
{
public:
    virtual void get_color(const argument_type z, uint8_t& r, uint8_t& g, uint8_t& b) const = 0;

protected:
    /**
     * Smooth projection from R -> [0,255] casted to an integer x -> (x fmod 2)^2 * ((x fmod 2))-2)^2 * 255
     * @param val
     * @return
     */
    uint8_t get_color_byte(argument_type val) const
    {
//        return static_cast<uint8_t>(fmod(val, 255.0));
//        return static_cast<uint8_t>(fmax(0.0, fmin(val, 255.0)));
        argument_type x = static_cast<argument_type>(fmod(val, 2.0));
        return static_cast<uint8_t>(x * x * (x - 2) * (x - 2) * 255.0);
    }
};

class polynomial_color_map : public color_map
{
    std::uniform_int_distribution<unsigned int> degree_dist;

    const polynomial r_poly;
    const polynomial g_poly;
    const polynomial b_poly;

public:
    /**
     *
     * @param prng A seeded random number generator. It should be seeded directly before handing it over, so
     * one can have the random function and the colormap generate independently.
     */
    polynomial_color_map(std::default_random_engine& prng)
        : degree_dist(std::uniform_int_distribution<unsigned int>(3, 3)),
          r_poly(get_random_polynomial(polynomial_color_map::degree_dist(prng), prng)),
          g_poly(get_random_polynomial(polynomial_color_map::degree_dist(prng), prng)),
          b_poly(get_random_polynomial(polynomial_color_map::degree_dist(prng), prng))
    {}

    void get_color(const argument_type z, uint8_t& r, uint8_t& g, uint8_t& b) const
    {
        r = get_color_byte(r_poly.eval(z));
        g = get_color_byte(g_poly.eval(z));
        b = get_color_byte(b_poly.eval(z));
    }
};


#endif //GENERATIVEART_RANDOM_COLOR_MAP_H
