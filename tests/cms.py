''' Test the functionality of the CountMinSketch class '''
from __future__ import (print_function)
import sys
sys.dont_write_bytecode = True
sys.path.append("./python")
from countminsketch import (CountMinSketch)

def test():
    ''' basic testing method '''
    print('build in memory check')
    cms = CountMinSketch(width=100000, depth=7)
    # add elements
    for i in range(100):
        tmp = 100 * (i + 1)
        cms.add(str(i), tmp)

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

    try:
        print('\n\nTest invalid initialization')
        cms_ex = CountMinSketch()
    except SyntaxError as ex:
        print(ex)


if __name__ == '__main__':
    test()
