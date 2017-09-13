import _typycal
#class _typycal:
#    @staticmethod
#    def hook(s):...
s = _typycal.hook('')
from mypy.main import main
import time
start = time.time()
main(None, args=['C:\\Users\\ethanhs\\Documents\\mypy\\mypy'])
print(f'took {time.time() - start}s')