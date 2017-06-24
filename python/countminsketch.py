'''
Count-Min Sketch python implementation
'''
# MIT License
# Author: Tyler Barrus (barrust@gmail.com)
import sys, os
import struct
import math

class CountMinSketch(object):
    ''' Count-Min Sketch class '''
    def __init__(self, width=None, depth=None, confidence=None, error_rate=None, filepath=None, hash_function=None):
        ''' default initilization function '''
        # default values
        self._width = 0
        self._depth = 0
        self._confidence = 0.0
        self._error_rate = 0.0
        self._elements_added = 0
        # for python2 and python3 support
        self.__int32_t_min = -2147483648
        self.__int32_t_max =  2147483647
        self.__int64_t_min = -9223372036854775808
        self.__int64_t_max =  9223372036854775807

        if width is not None and depth is not None:
            self._width = width
            self._depth = depth
            self._confidence = 1 - (1 / math.pow(2, depth))
            self._error_rate = 2 / width
            self._bins = [0] * (self._width * self._depth)
        elif confidence is not None and error_rate is not None:
            self._confidence = confidence
            self._error_rate = error_rate
            self._width = math.ceil(2 / error_rate)
            self._depth = math.ceil((-1 * math.log(1 - confidence)) / 0.6931471805599453)
            self._bins = [0] * (self._width * self._depth)
        elif filepath is not None:
            self.load(filepath, hash_function)

        if hash_function is None:
            self._hash_function = self.__default_hash
        else:
            self._hash_function = hash_function

    def clear(self):
        ''' reset the count-min sketch to empty '''
        self._elements_added = 0
        for i, _ in enumerate(self._bins):
            self._bins[i] = 0

    def hashes(self, key, depth=None):
        return self._hash_function(key, self._depth if depth is None else depth)

    def add(self, key, x=1):
        ''' add element 'key' to the count-min sketch 'x' times '''
        hashes = self.hashes(key)
        return self.add_alt(hashes, x)

    def add_alt(self, hashes, x=1):
        res = self.__int32_t_max
        for i, val in enumerate(hashes):
            t_bin = (val % self._width) + (i * self._width)
            tmp = self._bins[t_bin] + x
            self._bins[t_bin] = tmp if tmp < self.__int32_t_max else self.__int32_t_max
            if self._bins[t_bin] < res:
                res = self._bins[t_bin]
        self._elements_added = self.__int32_t_max if self._elements_added + x > self.__int32_t_max else self._elements_added + x
        return res

    def remove(self, key, x=1):
        ''' remove element 'key' from the count-min sketch 'x' times '''
        hashes = self.hashes(key)
        return remove_alt(hashes, x)

    def remove_alt(self, hashes, x=1):
        res = self.__int32_t_max
        for i, val in enumerate(hashes):
            t_bin = (val % self._width) + (i * self._width)
            tmp = self._bins[t_bin] - x
            self._bins[t_bin] = tmp if tmp > self.__int32_t_min else self.__int32_t_min
            if self._bins[t_bin] < res:
                res = self._bins[t_bin]
        self._elements_added = self.__int32_t_max if self._elements_added - x < self.__int32_t_min else self._elements_added - x
        return res

    def check(self, key, query='min'):
        ''' check number of times element 'key' is in the count-min sketch '''
        hashes = self.hashes(key)
        return self.check_alt(hashes, query)

    def check_alt(self, hashes, query='min'):
        qry = query.lower()
        bins = self.__get_values_sorted(hashes)
        if qry == 'min' or qry is 'min':
            res = bins[0]
        elif qry == 'mean' or qry is 'mean':
            res = sum(bins) // self._depth
        elif qry == 'mean-min' or qry is 'mean-min':
            meanmin = list()
            for b in bins:
                meanmin.append(b - ((self._elements_added - b) // (self._width - 1)))
                meanmin.sort()
            if self._depth % 2 == 0:
                res = (meanmin[self._depth//2] + meanmin[self._depth//2 - 1]) / 2
            else:
                res = meanmin[self._depth//2]
        else:
            print('invalid query type')
        return res

    def export(self, filepath):
        ''' export the count-min sketch to file '''
        with open(filepath, 'wb') as fp:
            # write out the bins
            for bn in self._bins:
                fp.write(struct.pack('i', bn))
            # write the other pieces of information...
            fp.write(struct.pack('I', self._width))
            fp.write(struct.pack('I', self._depth))
            fp.write(struct.pack('q', self._elements_added))

    def load(self, filepath, hash_function=None):
        ''' load the count-min sketch from file '''
        with open(filepath, 'rb') as fp:
            offset = struct.calcsize('IIq')
            fp.seek(offset * -1, os.SEEK_END)
            mybytes = struct.unpack('IIl', fp.read(offset))
            self._width = mybytes[0]
            self._depth = mybytes[1]
            self._elements_added = mybytes[2]
            self._confidence = 1 - (1 / math.pow(2, self._depth))
            self._error_rate = 2 / self._width

            fp.seek(0, os.SEEK_SET)
            length = self._width * self._depth
            self._bins = [0] * length
            for i in range(0, length):
                val = struct.unpack('i', fp.read(4))[0]
                if val != 0:
                    self._bins[i] = val

        if hash_function is None:
            self._hash_function = self.__default_hash
        else:
            self._hash_function = hash_function

    def __default_hash(self, key, depth):
        ''' the default fnv-1a hashing routine '''
        res = list()
        tmp = key
        for i in range(0, depth):
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
        uint64_max = 2 ** 64
        for s in key:
            hval = hval ^ ord(s)
            hval = (hval * fnv_64_prime) % uint64_max
        return hval

    def __get_values_sorted(self, hashes):
        bins = list()
        for i, val in enumerate(hashes):
            t_bin = (val % self._width) + (i * self._width)
            bins.append(self._bins[t_bin])
        bins.sort()
        return bins


if __name__ == '__main__':

    ## Test export output
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
    if cms._width != cmsf._width:
        print('width does not match!')
    if cms._depth != cmsf._depth:
        print('depth does not match!')

    print(cmsf.check(str(0), 'min'))
    print(cmsf.check(str(0), 'mean'))
    print(cmsf.check(str(0), 'mean-min'))
