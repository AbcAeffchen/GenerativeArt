# GenerativeArt

This is a just for fun project aiming to generate some nice images.

<!-- todo add examples here -->

## Compiling
```bash
mkdir build && cd build && mkdir images
cmake ..
cmake --build . --target GenerativeArt -j
```

## How to use it
<!-- todo  -->

## Options
```text
Usage: GenerativeArt [OPTIONS]

Image Options:
  -r,--resolution UINT=200  The resolution the image(s) are generated in.
  -x,--x-domain FLOAT=0 FLOAT=1
                            Min and max values of the domain of the x dimension.
  -y,--y-domain FLOAT=0 FLOAT=1
                            Min and max values of the domain of the y dimension.
  -n,--no-normalization     Stops normalizing the color transitions, that reduces the flickering.
  --projection-type {Cap=0, Periodic=1, Smooth Periodic=2}=0
                            The way the values of the color polynomials get projected into the
                            [0,255] range.
                            - Cap: x -> 255 for x > 255, x -> 0 for x < 0, x -> x else.
                            - Periodic: x -> x mod 256
                            - Smooth Periodic: x -> x mod 2 -> 255 * x^2 * (x-2)^2

Options:
  -h,--help                 Print this help message and exit

Program Options:
  -s,--num-samples UINT=100 The number of samples generated.
  -v,--verbose              Shows details about what the program is doing.
  -w,--write-ini TEXT       Writes the current settings into the given ini file.
  --config TEXT             Read an ini file
  -o,--out TEXT=images/     The directory where the images are stored.

Randomness Options:
  -f,--file-name TEXT Excludes: -D,--function-depth -P,--function-params -d,--color-poly-deg
                                -p,--color-poly-params -F,--function-seed -C,--color-seed
                                --num_unary_functions --num_binary_functions
                            The file name of a sample image. This can be used to
                            regenerate the image on a higher resolution. If used you
                            cannot use any of the randomness options.
  -D,--function-depth UINT=4 UINT=10 Excludes: -f,--file-name
                            The domain the depth of the random function is drawn from.
  -P,--function-params FLOAT=-16 FLOAT=16 Excludes: -f,--file-name
                            The domain the parameters of the random function is drawn from.
  -d,--color-poly-deg UINT=2 UINT=3 Excludes: -f,--file-name
                            The domain the degree of the random color polynomials is drawn from.
  -p,--color-poly-params FLOAT=-128 FLOAT=128 Excludes: -f,--file-name
                            The domain the parameters of the random color polynomials is drawn from.
  -F,--function-seed UINT Excludes: -f,--file-name
                            The seed for the random function. If provided only one image is generated.
  -C,--color-seed UINT Excludes: -f,--file-name
                            The seed for the color functions. If provided only one image is generated.
  --num_unary_functions UINT=13 Excludes: -f,--file-name
                            Number of unary functions available. This is just to support more
                            functions without breaking generation of old images.
  --num_binary_functions UINT=4 Excludes: -f,--file-name
                            Number of binary functions available. This is just to support more
                            functions without breaking generation of old images.

```

## Development Notices
New functions can be added to the function pool without
risking that the old results cannot be reproduced.
But the order an number of `rand()` calls must not
be changed to ensure this! If the order or number of the
`rand()` calls changes the whole random function changes.

## How it works

1. A random function is drawn as a call tree of functions from
a function pool.
2. For each color (red, green, blue) is a random polynomial drawn.
3. Generate for each pixel a value using the random function.
4. Generate a color value for each pixel using the random color functions
and the value from the first function as input.
5. The color values for each pixel get normalized to smooth
color changes. (can be turned off)
6. Write the image to disk using a file name that contains all settings
so you can easily reproduce the image.

## TODO
- Improve the seed handling. Color map seeds turned out mostly useless.
- try other ways to project the result of the random function into [0,255].
- Maybe the performance can be improved by using SIMD.
- check for all while / all black images and don't store them.

## Dependencies
- Commandline Parser [cxxopts](https://github.com/jarro2783/cxxopts) is
used as a submodule.
- [png++ 0.2.9](http://savannah.nongnu.org/projects/pngpp/) is used to generate
png files.
Therefore libpng and libzip needs to be installed.
- OpenMP is used to generate an image in parallel.

## Licence
GPL v3.0