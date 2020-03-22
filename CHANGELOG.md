## Current Version

### Version 0.1.6
* Remove the python version with the recommendation of using `pyprobables` instead
* Ensure appropriate `const` usage for `char*`
* Improve the hashing to be more efficient

### Version 0.1.5
* Fix for non gcc overflow checks
* Added python sub classes:
    * StreamThreshold
        * Those elements that meet the threshold are maintained in a dictionary
        with the element as the key and the number of times reported
    * HeavyHitters
        * Those elements, up to the set number or elements, that are most
        frequent are kept in a dictionary with the element as the key and
        the number of times reported

### Version 0.1.4
* Fix mean-min query calculation
* Add and remove same element `x` times at once
* Minor code cleanup
* Added additional tests
* ***python*** implementation
    * Add, remove and check functions
        * Either by key or hashes
        * Possible to insert more than 1 count at a time
    * Export and Import (Binary compatible with the C implementation)

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
