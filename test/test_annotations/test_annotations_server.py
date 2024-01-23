import sys
import os

# To access folders outside of current folder
# pathToErpc = os.path.abspath(os.path.join(
#     '..', 'erpc_python'))
# print(pathToErpc)
# sys.path.insert(0, pathToErpc)
pathToErpc = os.path.abspath('erpc_python')
print(pathToErpc)
sys.path.insert(0, pathToErpc)
# pathToErpc = os.path.abspath(os.path.join('..'))
# sys.path.insert(0, pathToErpc)
# print(pathToErpc)
pathToErpc = os.path.abspath('.')
print(pathToErpc)
sys.path.insert(0, pathToErpc)
pathToErpc = os.path.abspath('test')
print(pathToErpc)
sys.path.insert(0, pathToErpc)
from erpc_python import erpc  # nopep8
from Linux.tcp.gcc.test_annotations_server.Debug import test, test_unit_test_common  # nopep8
from common import unit_test_tcp_server  # nopep8


class AnnotateTestServer(test.interface.IAnnotateTest):
    def add(self, a, b):
        return a+b

    def testIfFooStructExist(self, a):
        pass

    def testIfMyEnumExist(self, a):
        pass

    def testIfMyIntAndConstExist(self, a):
        return a

server,commonServer = unit_test_tcp_server.getServer(erpc, test_unit_test_common.interface.ICommon)
server.add_service(test.server.AnnotateTestService(AnnotateTestServer(
)))
server.add_service(test_unit_test_common.server.CommonService(commonServer))
server.run()
