# Generative Art

This is a just for fun project aiming to generate some
nice images.

## Compiling
```
mkdir build && cd build && mkdir images
cmake ..
cmake --build . --target all
```

## How to use it
Run `generate_samples NUM_SAMPLES` to generate some samples.

If you found a nice image, run `generate_high_res_image FILE_NAME RESOLUTION`
to get the same image in a better quality. `FILE_NAME` is the name of the
low resolution file.

## Development Notices
New functions can be added to the function pool without
risking that the old results cannot be reproduced.
But the order an number of `rand()` calls must not
be changed to ensure this! If the order or number of the
`rand()` calls changes the whole random function changes.

## TODO
- Improve the seed handling. Color map seeds turned out mostly useless.
- try other ways to project the result of the random function into [0,255].
- Maybe the performance can be improved by using SIMD.

## Dependencies
- [png++ 2.9.0](http://www.nongnu.org/pngpp/doc/0.2.9/) is used to generate
png files.
Therefor libpng and libzip needs to be installed.
- OpenMP is used to generate an image in parallel.

## Licence
GPL 3.0