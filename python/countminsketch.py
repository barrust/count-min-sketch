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
            self._depth = math.ceil((-1 * math.log(1 - confidence)) / 0.6931471805599453);
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

    def add(self, key, x=1):
        ''' add element 'key' to the count-min sketch 'x' times '''
        res = sys.maxint
        hashes = self._hash_function(key, self._depth)
        for i, val in enumerate(hashes):
            t_bin = (val % self._width) + (i * self._width)
            self._bins[t_bin] += x
            if self._bins[t_bin] < res:
                res = self._bins[t_bin]
        self._elements_added += x
        return res

    def remove(self, key, x=1):
        ''' remove element 'key' from the count-min sketch 'x' times '''
        res = sys.maxint
        hashes = self._hash_function(key, self._depth)
        for i, val in enumerate(hashes):
            t_bin = (val % self._width) + (i * self._width)
            self._bins[t_bin] -= x
            if self._bins[t_bin] < res:
                res = self._bins[t_bin]
        self._elements_added -= x
        return res

    def check(self, key, query='min'):
        ''' check number of times element 'key' is in the count-min sketch '''
        qry = query.lower()
        hashes = self._hash_function(key, self._depth)
        bins = self.__get_values_sorted(hashes)
        if qry == 'min' or qry is 'min':
            res = bins[0]
        elif qry == 'mean' or qry is 'mean':
            res = sum(bins) / self._depth
        elif qry == 'mean-min' or qry is 'mean-min':
            meanmin = list()
            for b in bins:
                meanmin.append(b - ((self._elements_added - b) / (self._width - 1)))
                meanmin.sort()
            if self._depth % 2 == 0:
                res = (meanmin[self._depth/2] + meanmin[self._depth/2 - 1]) / 2;
            else:
                res = meanmin[self._depth/2]
        else:
            print 'invalid query type'
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
            fp.write(struct.pack('l', self._elements_added))

    def load(self, filepath, hash_function=None):
        ''' load the count-min sketch from file '''
        with open(filepath, 'rb') as fp:
            offset = struct.calcsize('IIl')
            fp.seek(offset * -1, os.SEEK_END)
            mybytes = struct.unpack('IIl', fp.read(offset))
            # print mybytes
            self._width = mybytes[0]
            self._depth = mybytes[1]
            self._elements_added = mybytes[2]
            self._confidence = 1 - (1 / math.pow(2, self._depth))
            self._error_rate = 2 / self._width

            fp.seek(0, os.SEEK_SET)
            length = self._width * self._depth
            self._bins = [0] * length
            j = 0
            q = 0
            for i in range(0, length):
                val = struct.unpack('i', fp.read(4))[0]
                if int(val) != 0:
                    self._bins[i] = int(val)

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
    # cms = CountMinSketch(width=100000, depth=7)
    cmsf = CountMinSketch(filepath='./dist/test_export.cms')
    # if cms._width != cmsf._width:
    #     print 'width does not match!'
    # if cms._depth != cmsf._depth:
    #     print 'depth does not match!'
    # length = cmsf._width * cmsf._depth
    # print cmsf._bins
    # for bn in cmsf._bins:
    #     # print bn
    #     # print cms._bins[i]
    #     if bn != 0:
    #         print bn
    # for i in range(0, 100):
    #     t = 100 * (i + 1);
    #     cms.add(str(i), t)
    print cmsf.check(str(0), 'min')
    # print cms.check(str(0), 'mean')
    # print cms.check(str(0), 'mean-min')
    # print cms._elements_added, cms._width, cms._depth, cms._confidence
    # cms.export('./dist/py_test.cms')
    # cms.load('./dist/py_test.cms')
