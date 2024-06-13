import sys
import os

# pathToErpc = os.path.abspath(os.path.join(
#     '..', 'erpc_python'))
# print(pathToErpc)
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
from Linux.tcp.gcc.test_annotations_server.Debug import test, test_unit_test_common  # nopep8
from erpc_python import erpc  # nopep8
from common import unit_test_tcp_client  # nopep8

clientManager = unit_test_tcp_client.getClientManager(erpc)

annotateTestClient = test.client.AnnotateTestClient(clientManager)
commonClient = test_unit_test_common.client.CommonClient(clientManager)


def AnnotationServiceID():
    assert (5 == test.interface.IAnnotateTest.SERVICE_ID)


def testIfMyIntAndConstExist():
    assert (5 == annotateTestClient.testIfMyIntAndConstExist(5))


def quit():
    commonClient.quit()
