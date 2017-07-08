''' Test the functionality of the StreamThreshold class '''
from __future__ import (print_function)
import sys
sys.dont_write_bytecode = True
sys.path.append("./python")
from countminsketch import (StreamThreshold)

def test():
    ''' basic testing method '''
    strm = StreamThreshold(threshold=75, width=1000, depth=7)
    print(strm.meets_threshold)
    strm.add('test', 78)
    print(strm.meets_threshold)
    strm.add('test1', 74)
    print(strm.meets_threshold)
    strm.add('test1', 2)
    print(strm.meets_threshold)
    strm.remove('test', 7)
    print(strm.meets_threshold)
    strm.add('test', 78)
    print(strm.meets_threshold)
    strm.add('testing', 15)
    print(strm.meets_threshold)
    strm.add('testing', 60)
    print(strm.meets_threshold)


if __name__ == '__main__':
    test()
