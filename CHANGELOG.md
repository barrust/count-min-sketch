# Count-Min Sketch

### Version 0.2.1 (unreleased)
* Fix: `width * depth` overflowed an `unsigned int`, allocating too few bins so adds wrote out of bounds; `cms_init()` now fails when it is larger than `UINT32_MAX`
* Fix: `cms_init_optimal()` accepted an error rate of 0 and a confidence of 0 or 1, converting infinity to an `unsigned int` or giving a depth of 0; it now fails for those
* Fix: `cms_check_mean()` summed the bins in an `int32_t`, which overflowed (4 saturated bins gave 1073741823)
* Fix: `cms_check_mean_min()` compared values by truncating their `int64_t` difference to an `int`, which could sort them in the wrong order, and wrapped a median outside the `int32_t` range around; it now compares correctly and clamps the median
* Fix: `cms_check_mean_min()` divided by `width - 1`, which is 0 for a width of 1; with a single column it uses the counts
* Fix: merging lost saturation when the other sketch's bin was saturated (a count at `INT32_MAX` became `INT32_MAX - 10`)
* Fix: importing read the footer at the wrong offset on Windows (`sizeof(long)` is 4 there), did not check the values it read or the file size (so a crafted footer could make it allocate far more than the file holds, and `malloc(length * sizeof(int32_t))` could overflow with a 32-bit `size_t`), and called `exit(1)` on a short read; it now checks the file is exactly the size its footer describes before allocating, and returns `CMS_ERROR` for an invalid file
* Fix: a failed hash allocation is now returned as `CMS_ERROR` instead of dereferencing `NULL`; `cms_get_hashes()` returns `NULL` if the memory cannot be allocated
* Fix: the overflow check used without `__builtin_sub_overflow` returned the wrong limit, and `va_end` was not called after validating a merge
* CI: test with gcc 13, 14, and 15 (Ubuntu 26.04), and remove the cppcheck suppression that no longer matches

### Version 0.2.0
* ***BACKWARD INCOMPATIBLE CHANGES***
    * **NOTE:** Breaks backwards compatibility with previously exported blooms using the default hash!
    * Update to the FNV_1a hash function
    * Simplified hashing at depth by using a seed value

### Version 0.1.8
* Added in-depth testing
* Fixed potential overflow issues

### Version 0.1.7
* Added count-min sketch merge functionality; thanks to [jonahharris](https://github.com/jonahharris)

### Version 0.1.6
* Remove the python version with the recommendation of using `pyprobables` instead
* Ensure appropriate `const` usage for `char*`
* Improve the hashing to be more efficient

### Version 0.1.5
* Fix for non gcc overflow checks; thank to [meixsh](https://github.com/meixsh)
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
