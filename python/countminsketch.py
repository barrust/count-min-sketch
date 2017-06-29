''' Count-Min Sketch python implementation
    License: MIT
    Author: Tyler Barrus (barrust@gmail.com)
    URL: https://github.com/barrust/count-min-sketch
'''
from __future__ import (print_function)
import os
from struct import (pack, unpack, calcsize)
import math


class CountMinSketch(object):
    ''' Count-Min Sketch class '''

    def __init__(self, width=None, depth=None, confidence=None,
                 error_rate=None, filepath=None, hash_function=None):
        ''' default initilization function '''
        # default values
        self.__width = 0
        self.__depth = 0
        self.__confidence = 0.0
        self.__error_rate = 0.0
        self.__elements_added = 0
        # for python2 and python3 support
        self.__int32_t_min = -2147483648
        self.__int32_t_max = 2147483647
        self.__int64_t_min = -9223372036854775808
        self.__int64_t_max = 9223372036854775807
        self.__uint64_t_max = 2 ** 64

        if width is not None and depth is not None:
            self.__width = width
            self.__depth = depth
            self.__confidence = 1 - (1 / math.pow(2, depth))
            self.__error_rate = 2 / width
            self._bins = [0] * (self.__width * self.__depth)
        elif confidence is not None and error_rate is not None:
            self.__confidence = confidence
            self.__error_rate = error_rate
            self.__width = math.ceil(2 / error_rate)
            numerator = (-1 * math.log(1 - confidence))
            self.__depth = math.ceil(numerator / 0.6931471805599453)
            self._bins = [0] * (self.__width * self.__depth)
        elif filepath is not None:
            self.load(filepath, hash_function)

        if hash_function is None:
            self._hash_function = self.__default_hash
        else:
            self._hash_function = hash_function

    @property
    def width(self):
        ''' get width '''
        return self.__width

    @property
    def depth(self):
        ''' get depth '''
        return self.__depth

    @property
    def confidence(self):
        ''' get confidence '''
        return self.__confidence

    @property
    def error_rate(self):
        ''' get error rate '''
        return self.__error_rate

    @property
    def elements_added(self):
        ''' get elements added '''
        return self.__elements_added

    def clear(self):
        ''' reset the count-min sketch to empty '''
        self.__elements_added = 0
        for i, _ in enumerate(self._bins):
            self._bins[i] = 0

    def hashes(self, key, depth=None):
        ''' return the hashes for the passed in key '''
        t_depth = self.__depth if depth is None else depth
        return self._hash_function(key, t_depth)

    def add(self, key, num_els=1):
        ''' add element 'key' to the count-min sketch 'x' times '''
        hashes = self.hashes(key)
        return self.add_alt(hashes, num_els)

    def add_alt(self, hashes, num_els=1):
        ''' add an element by using the hashes '''
        res = self.__int32_t_max
        for i, val in enumerate(hashes):
            t_bin = (val % self.__width) + (i * self.__width)
            self._bins[t_bin] += num_els
            if self._bins[t_bin] > self.__int32_t_max:
                self._bins[t_bin] = self.__int32_t_max
            if self._bins[t_bin] < res:
                res = self._bins[t_bin]

        self.__elements_added += num_els
        if self.__elements_added > self.__int32_t_max:
            self.__elements_added = self.__int32_t_max

        return res

    def remove(self, key, num_els=1):
        ''' remove element 'key' from the count-min sketch 'x' times '''
        hashes = self.hashes(key)
        return self.remove_alt(hashes, num_els)

    def remove_alt(self, hashes, num_els=1):
        ''' remove an element by using the hashes '''
        res = self.__int32_t_max
        for i, val in enumerate(hashes):
            t_bin = (val % self.__width) + (i * self.__width)
            self._bins[t_bin] -= num_els
            if self._bins[t_bin] < self.__int32_t_min:
                self._bins[t_bin] = self.__int32_t_min
            if self._bins[t_bin] < res:
                res = self._bins[t_bin]
        self.__elements_added -= num_els
        if self.__elements_added < self.__int32_t_min:
            self.__elements_added = self.__int32_t_max

        return res

    def check(self, key, query='min'):
        ''' check number of times element 'key' is in the count-min sketch '''
        hashes = self.hashes(key)
        return self.check_alt(hashes, query)

    def check_alt(self, hashes, query='min'):
        ''' check the count-min sketch for an element by using the hashes '''
        qry = query.lower()
        bins = self.__get_values_sorted(hashes)
        if qry == 'min' or qry is 'min':
            res = bins[0]
        elif qry == 'mean' or qry is 'mean':
            res = sum(bins) // self.__depth
        elif qry == 'mean-min' or qry is 'mean-min':
            meanmin = list()
            for t_bin in bins:
                diff = self.__elements_added - t_bin
                calc = t_bin - diff // (self.__width - 1)
                meanmin.append(calc)
            meanmin.sort()
            if self.__depth % 2 == 0:
                calc = meanmin[self.__depth//2] + meanmin[self.__depth//2 - 1]
                res = calc / 2  # TODO: should this be // ?
            else:
                res = meanmin[self.__depth//2]
        else:
            print('invalid query type')
        return res

    def export(self, filepath):
        ''' export the count-min sketch to file '''
        with open(filepath, 'wb') as filepointer:
            # write out the bins
            rep = 'i' * len(self._bins)
            filepointer.write(pack(rep, *self._bins))
            filepointer.write(pack('IIq', self.__width, self.__depth,
                                   self.__elements_added))

    def load(self, filepath, hash_function=None):
        ''' load the count-min sketch from file '''
        with open(filepath, 'rb') as filepointer:
            offset = calcsize('IIq')
            filepointer.seek(offset * -1, os.SEEK_END)
            mybytes = unpack('IIq', filepointer.read(offset))
            self.__width = mybytes[0]
            self.__depth = mybytes[1]
            self.__elements_added = mybytes[2]
            self.__confidence = 1 - (1 / math.pow(2, self.__depth))
            self.__error_rate = 2 / self.__width

            filepointer.seek(0, os.SEEK_SET)
            length = self.__width * self.__depth
            rep = 'i' * length
            offset = calcsize(rep)
            self._bins = list(unpack(rep, filepointer.read(offset)))

        if hash_function is None:
            self._hash_function = self.__default_hash
        else:
            self._hash_function = hash_function

    def __default_hash(self, key, depth):
        ''' the default fnv-1a hashing routine '''
        res = list()
        tmp = key
        for _ in range(0, depth):
            if tmp != key:
                tmp = self.__fnv_1a("{0:x}".format(tmp))
            else:
                tmp = self.__fnv_1a(key)
            res.append(tmp)
        return res

    def __fnv_1a(self, key):
        ''' 64 bit fnv-1a hash '''
        hval = 14695981039346656073
        fnv_64_prime = 1099511628211
        for t_str in key:
            hval = hval ^ ord(t_str)
            hval = (hval * fnv_64_prime) % self.__uint64_t_max
        return hval

    def __get_values_sorted(self, hashes):
        ''' get the values sorted '''
        bins = list()
        for i, val in enumerate(hashes):
            t_bin = (val % self.__width) + (i * self.__width)
            bins.append(self._bins[t_bin])
        bins.sort()
        return bins


if __name__ == '__main__':
    # pass
    # Test export output
    print('build in memory check')
    cms = CountMinSketch(width=100000, depth=7)
    # add elements
    for i in range(0, 100):
        t = 100 * (i + 1)
        cms.add(str(i), t)

    print(cms.check(str(0), 'min'))
    print(cms.check(str(0), 'mean'))
    print(cms.check(str(0), 'mean-min'))
    cms.export('./dist/py_test.cms')

    print('import from disk check')
    cmsf = CountMinSketch(filepath='./dist/py_test.cms')
    if cms.width != cmsf.width:
        print('width does not match!')
    if cms.depth != cmsf.depth:
        print('depth does not match!')

    print(cmsf.check(str(0), 'min'))
    print(cmsf.check(str(0), 'mean'))
    print(cmsf.check(str(0), 'mean-min'))
