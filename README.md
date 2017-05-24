# count-min-sketch

A Count-Min Sketch implementation in C.

Count-Min Sketch is a probabilistic data-structure that takes sub linear space
to store the probable count of occurrences of elements added into the
data-structure. Due to the structure and strategy of storing elements, it is
possible that elements are over counted.

## License:
MIT 2017

## Main Features:
* Ability to add and lookup probable insertions into the Count-Min Sketch.

## Future Enhancements
* add remove functionality
* add inline functionality
* add other lookup strategies b/c of remove
* add export/import (?)
* add width and depth calculators (?)


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
