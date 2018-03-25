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
class ColorMap
{
public:
    enum projection_type : uint8_t {cap, periodic, smooth_periodic};

private:
    const projection_type pt = cap;

public:
    explicit ColorMap(const projection_type pt) : pt(pt) {}

    virtual void get_color(argument_type z, uint8_t& r, uint8_t& g, uint8_t& b) const = 0;

protected:
    /**
     * Smooth projection from R -> [0,255] casted to an integer x -> (x fmod 2)^2 * ((x fmod 2)-2)^2 * 255.
     * @param val
     * @return
     */
    uint8_t get_color_byte(const argument_type val) const
    {
        switch(pt)
        {
            case periodic:
                return static_cast<uint8_t>(fmod(val, 256.0));
            case smooth_periodic:
            {
                const argument_type x = static_cast<argument_type>(fmod(val, 2.0));
                return static_cast<uint8_t>(x * x * (x - 2) * (x - 2) * 255.0);
            }
            case cap:
            default:
                return static_cast<uint8_t>(fmax(0.0, fmin(val, 255.0)));
        }
    }
};

class PolynomialColorMap : public ColorMap
{
    const RandomPolynomial r_poly;
    const RandomPolynomial g_poly;
    const RandomPolynomial b_poly;

public:
    /**
     *
     * @param prng A seeded random number generator. It should be seeded directly before handing it over, so
     * one can have the random function and the colormap generate independently.
     */
    explicit PolynomialColorMap(std::default_random_engine& prng, const projection_type pt, const Domain<unsigned int>& degree_domain,
                                const Domain<float>& param_domain)
        : ColorMap(pt),
          r_poly(RandomPolynomial::get(degree_domain, param_domain, prng)),
          g_poly(RandomPolynomial::get(degree_domain, param_domain, prng)),
          b_poly(RandomPolynomial::get(degree_domain, param_domain, prng))
    {}

    // todo for vectorisation split the color generation up?
    void get_color(const argument_type z, uint8_t& r, uint8_t& g, uint8_t& b) const
    {
        r = get_color_byte(r_poly.eval(z));
        g = get_color_byte(g_poly.eval(z));
        b = get_color_byte(b_poly.eval(z));
    }

    std::string print() const
    {
        return "r = " + r_poly.print() + "\n"
             + "g = " + g_poly.print() + "\n"
             + "b = " + b_poly.print();
    }
};


#endif //GENERATIVEART_RANDOM_COLOR_MAP_H
