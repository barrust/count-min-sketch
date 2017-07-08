''' Test the functionality of the HeavyHitters class '''
from __future__ import (print_function)
import sys
sys.dont_write_bytecode = True
sys.path.append("./python")
from countminsketch import (HeavyHitters)

def test():
    ''' basic testing method '''
    print('Test Heavy Hitters!')
    hh1 = HeavyHitters(num_hitters=3, width=1000, depth=7)
    print(hh1.heavyhitters)
    hh1.add('test', 100)
    print(hh1.heavyhitters)
    hh1.add('test1', 99)
    print(hh1.heavyhitters)
    hh1.add('test2', 75)
    print(hh1.heavyhitters)
    hh1.add('test3', 76)
    print(hh1.heavyhitters)
    hh1.add('test2', 2)
    print(hh1.heavyhitters)
    hh1.add('test7', 112)
    print(hh1.heavyhitters)
    hh1.export('./dist/heavyhitters.cms')

    print('\n\n')
    hh2 = HeavyHitters(filepath='./dist/heavyhitters.cms')
    print('Test loading an exported Heavy Hitter!')
    print(hh2.heavyhitters)
    hh2.add('test', 1)
    print(hh2.heavyhitters)


if __name__ == '__main__':
    test()
