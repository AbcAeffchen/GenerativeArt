#!/usr/bin/env bash

# Reads all png file names from a folder, creates a sub folder named after the
# resolution and generates every image using the resolution.
#
# inputs:
# 1. path to image folder
# 2. target resolution

#mkdir $1/$2 && (ls -1 $1 | grep .png) | xargs -i ./build/GenerativeArt -o $1/$2 -f {} -r $2
if [ ! -d $1/$2 ]; then
    mkdir $1/$2
fi

(ls -1 $1 | grep .png) | xargs -i ./build/GenerativeArt -o $1/$2/ --color-permutations -r $2 -f {}