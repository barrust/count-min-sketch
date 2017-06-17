## Current Version

### Version 0.1.4
* Fix mean-min query calculation
* Add and remove same element `x` times at once
* ***python*** implementation
    * Add and remove
    * Export (Binary compatible with the C implementation)
    * Load saved count-min sketch
* Minor code cleanup
* Added additional tests

### Version 0.1.3
* Updated tests
* Added import / export functionality
* Added mean-min lookup strategy
    * NOTE: This means this can also be used as a **Count-Mean-Min Sketch**

### Version 0.1.2
* Added ability to add, remove, and lookup elements based only on the hash array
* Changed bin layout to be a single array of ints for easier import export
* Fixed logical error with calculating maximum and mean insertion values

### Version 0.1.1
* Ability to remove elements
* Mean based lookup
* Optimal width / depth initialization based on error rate and certainty
* Track elements inserted
* Calculate hashes for an element and return to caller

### Version 0.1.0
* Initial version of a basic count-min sketch data-structure
* Ability to:
    * Add element to the count-min sketch
    * Check count of element in the count-min sketch
    * Clear count-min sketch
    * Set user defined hashing function
