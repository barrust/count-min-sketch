# count-min-sketch

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![GitHub release](https://img.shields.io/github/v/release/barrust/count-min-sketch.svg)](https://github.com/barrust/count-min-sketch/releases)
[![Build Status](https://travis-ci.com/barrust/count-min-sketch.svg?branch=master)](https://travis-ci.com/barrust/count-min-sketch)
[![codecov](https://codecov.io/gh/barrust/count-min-sketch/branch/master/graph/badge.svg)](https://codecov.io/gh/barrust/count-min-sketch)


A Count-Min Sketch implementation in **C**.

Count-Min Sketch is a probabilistic data-structure that takes sub linear space
to store the probable count, or frequency, of occurrences of elements added
into the data-structure. Due to the structure and strategy of storing elements,
it is possible that elements are over counted but not under counted.

To use the library, copy the `src/count_min_sketch.h` and
`src/count_min_sketch.c` files into your project and include it where needed.

## License:
MIT 2017

# Point Query Strategies
To generic method to query the count-min sketch for the number of times an
element was inserted is to return the minimum value from each row in the
data-structure. This is the maximum number of times that it may have been
inserted, but there is a defined bias. This number is always greater than or
equal to the actual value but ***never*** lower.

To help account for this bias, there are two other methods of querying the
data. One is to use the mean of the results. This will result in larger answers,
but is useful when elements can be removed from the count-min sketch.

The other option is to use the count-mean-min query strategy. This strategy
attempts to remove the bias by taking the median value from the results of the
following calculation of each row (where `i` is the bin result of the hash):
`bin[i] - ((number-elements - bin[i]) / (width - 1))`

For a good description of different uses and methods of the count-min sketch,
read [this link](https://highlyscalable.wordpress.com/2012/05/01/probabilistic-structures-web-analytics-data-mining/).

For a **python version**, please check out [pyprobables](https://github.com/barrust/pyprobables)
which has a binary compatible output.


## Main Features
* Ability to add and remove elements from the Count-Min Sketch
    * Increment or add `x` elements at once
    * Decrement or remove `x` elements at once
* Ability to lookup elements in the data-structure
* Add, remove, or lookup elements based on pre-calculated hashes
* Ability to set depth & width or have the library calculate them based on
error and confidence
* Multiple lookup types:
    * ***Minimum:*** largest possible number of insertions by taking the
    maximum result
    * ***Mean:*** good for when removes and negatives are possible, but
    increases the false count
    * ***Mean-Min*** attempts to take bias into account; results are less
    skewed upwards compared to the mean lookup
* Export and Import count-min sketch to file
* Ability to merge multiple count-min sketches together

## Future Enhancements
* add method to calculate the possible bias (?)
* add do everything directly on disk (?)
* add import / export to hex (?)

## Usage:
``` c
#include <stdio.h>
#include "count_min_sketch.h"

CountMinSketch cms;
cms_init(&cms, 10000, 7);

int i, res;
for (i = 0; i < 10; i++) {
    res = cms_add(&cms, "this is a test");
}

res = cms_check(&cms, "this is a test");
if (res != 10) {
    printf("Error with lookup: %d\n", res);
}
cms_destroy(&cms);
```


## Required Compile Flags
-lm
