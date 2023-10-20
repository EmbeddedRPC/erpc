#!/usr/bin/python

# Copyright 2021 NXP
# All rights reserved.
#
# SPDX-License-Identifier: BSD-3-Clause

import inspect, os, sys
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(inspect.getfile(inspect.currentframe())))))
from server.runner import Runner
from service.test_struct.erpc_outputs import test_ArithmeticService, test_unit_test_common
#from service import test_ArithmeticService2Service
from common.common import CommonTestServiceHandler, runTesttestQuit
try:
    import erpc
except ImportError:
    print("Could not import erpc.\r\nPlease install it first by running \"python setup.py install\" in folder \"erpc/erpc_python/\".")
    sys.exit()

###############################################################################
# Variables
###############################################################################

###############################################################################
# Functions
###############################################################################


###############################################################################
# Server
###############################################################################

## Add service handler
class StructTestServiceHandler1(test_ArithmeticService.interface.IArithmeticService1):
    def getMember(self, c):
        return c.m

    def sendNestedStruct(self, d):
        return d.c.m

    def returnStruct(self, x, y):
        b = test_ArithmeticService.common.B()
        b.x = x
        b.y = y
        return b

    def getMemberTest2(self, a):
        result = test_ArithmeticService.common.B()
        result.x = a.c.m
        result.y = a.b.x
        return result

    def checkString(self, p):
        return not(p.species == "Ape")

    def returnStrings(self):
        str_str = test_ArithmeticService.common.stringStruct(one="One", two="Two", three="Three")
        return str_str

class StructTestServiceHandler2(test_ArithmeticService.interface.IArithmeticService2):
    def sendManyInts(self, f):
        if (f.a == -1) and (f.b == -1) and (f.c == -1) and (f.d == -1):
            return 0
        return 1

    def sendManyUInts(self, g):
        if (g.a == 0xff) and (g.b == 0xffff) and (g.c == 0xffffffff) and (g.d == 0xffffffffffffffff):
            return 0
        return 1

    def getStudentName(self, stud):
        return stud.name

    def getStudentTestAverage(self, stud):
        total = float(0.0)
        for x in range(3):
            total += stud.test_grades[x]
        return float(total / 3)

    def getStudentYear(self, stud):
        return stud.school_year

    def getStudentAge(self, stud):
        return stud.age

    def createStudent(self, name, test_scores, year, age):
        newStudent = test_ArithmeticService.common.student(name=name, test_grades=test_scores, school_year=year, age=age)
        return newStudent

    def test_struct_allDirection(self, a, b, e):
        e.value.number = a.number * b.number
        e.value.text = "World"
        e.value.color = test_ArithmeticService.common.enumColor.green
        e.value.c.m = a.c.m * b.c.m
        e.value.c.n = a.c.n * b.c.n
        for i in range(len(a.list_numbers)):
            e.value.list_numbers[i] = a.list_numbers[i] * b.list_numbers[i]
            e.value.list_text[i] = "World"
        for i in range(len(a.array_numbers)):
            e.value.array_numbers[i] = a.array_numbers[i] * b.array_numbers[i]
            e.value.array_text[i] = "World"
        for i in range(len(a.binary_numbers)):
            e.value.binary_numbers[i] = a.binary_numbers[i] * b.binary_numbers[i]
        return

    def testSendingByrefMembers(self, s):
        if (s.a == 0xED) and (s.b == 4):
            return True
        return False

###############################################################################
# Client
###############################################################################

## Individual test cases / eRPC calls
def runGetMember1(client):
    c = test_ArithmeticService.common.C(m=4, n=5)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    r = client.getMember(c)
    assert r == 4

def runTestArithmeticServiceID(client):
    assert 3 == test_ArithmeticService.interface.IArithmeticService1.SERVICE_ID 

def runNestedStruct1(client):
    c = test_ArithmeticService.common.C(m=4, n=5)
    d = test_ArithmeticService.common.D(c)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    r = client.sendNestedStruct(d)
    assert r == 4

def runReturnStruct1(client):
    b = test_ArithmeticService.common.B()
    # send request to the server
    print('\r\neRPC request is sent to the server')
    b = client.returnStruct(float(3.14), float(2.71828))
    print(b)
    assert b.x == float(3.14)
    assert b.y == float(2.71828)

def runSendIntSizes(client):
    f = test_ArithmeticService.common.F(-1, -1, -1, -1)
    #print(f)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    assert 0 == client.sendManyInts(f)

def runSendUnsignedIntSizes(client):
    g = test_ArithmeticService.common.G(0xFF, 0xFFFF, 0xFFFFFFFF, 0xFFFFFFFFFFFFFFFF)
    #print(g)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    assert 0 == client.sendManyUInts(g)

def runGetMember2(client):
    c = test_ArithmeticService.common.C(m=4, n=5)
    b = test_ArithmeticService.common.B(x=float(3.14), y=float(2.71828))
    a = test_ArithmeticService.common.A(b=b, c=c)
    bb = test_ArithmeticService.common.B()
    # send request to the server
    print('\r\neRPC request is sent to the server')
    bb = client.getMemberTest2(a)
    #print(bb)
    assert float(bb.x) == float(4.0)
    assert float(bb.y) == float(3.14)

def runTestString1(client):
    prim8 = test_ArithmeticService.common.primate(species="Ape", is_ape=False)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    assert 0 == client.checkString(prim8)

def runTestReturnString1(client):
    result = test_ArithmeticService.common.stringStruct()
    # send request to the server
    print('\r\neRPC request is sent to the server')
    result = client.returnStrings()
    assert result.one == "One"
    assert result.two == "Two"
    assert result.three == "Three"

def runTestStudent1(client):
    stud = test_ArithmeticService.common.student(name="Donnie Darko", test_grades=[float(85.75), float(93.29), float(92.46)], school_year=test_ArithmeticService.common.school_year_t.SENIOR, age=19)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    assert "Donnie Darko" == client.getStudentName(stud)
    assert float((stud.test_grades[0] + stud.test_grades[1] + stud.test_grades[2]) / 3) == client.getStudentTestAverage(stud)
    assert 19 == client.getStudentAge(stud)
    assert 12 == test_ArithmeticService.common.school_year_t.SENIOR
    assert test_ArithmeticService.common.school_year_t.SENIOR == client.getStudentYear(stud)

def runTestStudent2(client):
    stud = test_ArithmeticService.common.student()
    # send request to the server
    print('\r\neRPC request is sent to the server')
    stud = client.createStudent("George Bush", [float(65.32), float(69.68), float(0)], test_ArithmeticService.common.school_year_t.FRESHMAN, 68)
    assert "George Bush" == client.getStudentName(stud)
    assert float((65.32 + 69.68 + 0) / 3) == client.getStudentTestAverage(stud)
    assert 68 == client.getStudentAge(stud)
    assert test_ArithmeticService.common.school_year_t.FRESHMAN == client.getStudentYear(stud)

def runtest_struct_allDirection(client):
    a_binary = bytearray(b'')
    b_binary = bytearray(b'')
    e_binary = bytearray(b'')
    for x in range(2):
        a_binary.append(x)
        b_binary.append(x * 2)
        e_binary.append(x)
    a = test_ArithmeticService.common.AllTypes(number=5, text="Hello", color=test_ArithmeticService.common.enumColor.red, c=test_ArithmeticService.common.C(m=5, n=20), list_numbers=[0, 1], list_text=["Hello", "Hello"], array_numbers=[0, 1], array_text=["Hello", "Hello"], binary_numbers=a_binary)
    b = test_ArithmeticService.common.AllTypes(number=10, text="World", color=test_ArithmeticService.common.enumColor.green, c=test_ArithmeticService.common.C(m=7, n=17), list_numbers=[1, 2], list_text=["World", "World"], array_numbers=[1, 2], array_text=["World", "World"], binary_numbers=b_binary)
    e = erpc.Reference(test_ArithmeticService.common.AllTypes(number=15, text="Hello", color=test_ArithmeticService.common.enumColor.blue, c=test_ArithmeticService.common.C(m=4, n=14), list_numbers=[2, 3], list_text=["Hello", "Hello"], array_numbers=[2, 3], array_text=["Hello", "Hello"], binary_numbers=e_binary))
    # send request to the server
    print('\r\neRPC request is sent to the server')
    client.test_struct_allDirection(a, b, e)
    assert e.value.number == a.number * b.number
    assert e.value.text == "World"
    assert e.value.color == test_ArithmeticService.common.enumColor.green
    assert e.value.c.m == a.c.m * b.c.m
    assert e.value.c.n == a.c.n * b.c.n
    assert len(e.value.list_numbers) == 2 
    for x in range(len(a.list_numbers)):
        assert e.value.list_numbers[x] == a.list_numbers[x] * b.list_numbers[x]
        assert "World" == e.value.list_text[x]
    for x in range(2):
        assert e.value.array_numbers[x] == a.array_numbers[x] * b.array_numbers[x]
        assert "World" == e.value.array_text[x]
    for x in range(len(a.binary_numbers)):
        assert e.value.binary_numbers[x] == a.binary_numbers[x] * b.binary_numbers[x]

def runTestSendingByrefMembers(client):
    s = test_ArithmeticService.common.StructWithByrefMembers()
    b = 4
    s.a = 0xED 
    s.b = id(b) 
    # send request to the server
    print('\r\neRPC request is sent to the server')
    assert True == client.testSendingByrefMembers(s)

###############################################################################
# PYTESTS
###############################################################################
class TestStruct:
    server_cmd = [(os.path.abspath(__file__)).replace(os.sep, '/'), '-s']
    services = [test_ArithmeticService.server.ArithmeticService1Service(StructTestServiceHandler1()),test_ArithmeticService.server.ArithmeticService2Service(StructTestServiceHandler2()), test_unit_test_common.server.CommonService(CommonTestServiceHandler())]
    clientsConstructors = [test_ArithmeticService.client.ArithmeticService1Client, test_ArithmeticService.client.ArithmeticService2Client, test_unit_test_common.client.CommonClient]
    runner = None

    def test_runTesttestPrepareEnv(self):
        TestStruct.runner = Runner('test_struct', TestStruct.server_cmd, TestStruct.services, TestStruct.clientsConstructors)

    def test_runTestArithmeticServiceID(self):
        self.runner.runTest(test_ArithmeticService.interface.IArithmeticService1.SERVICE_ID, runTestArithmeticServiceID)
    
    def test_runGetMember1(self):
        self.runner.runTest(test_ArithmeticService.interface.IArithmeticService1.SERVICE_ID, runGetMember1)

    def test_runNestedStruct1(self):
        self.runner.runTest(test_ArithmeticService.interface.IArithmeticService1.SERVICE_ID, runNestedStruct1)

#    def test_runGetMember2(self):
#        self.runner.runTest(test_ArithmeticService.interface.IArithmeticService1.SERVICE_ID, runGetMember2)

    def test_runTestString1(self):
        self.runner.runTest(test_ArithmeticService.interface.IArithmeticService1.SERVICE_ID, runTestString1)

    def test_runTestReturnString1(self):
        self.runner.runTest(test_ArithmeticService.interface.IArithmeticService1.SERVICE_ID, runTestReturnString1)

    def test_runSendIntSizes(self):
        self.runner.runTest(test_ArithmeticService.interface.IArithmeticService2.SERVICE_ID, runSendIntSizes)

    def test_runSendUnsignedIntSizes(self):
        self.runner.runTest(test_ArithmeticService.interface.IArithmeticService2.SERVICE_ID, runSendUnsignedIntSizes)

    def test_runTestStudent1(self):
        self.runner.runTest(test_ArithmeticService.interface.IArithmeticService2.SERVICE_ID, runTestStudent1)

    def test_runTestStudent2(self):
        self.runner.runTest(test_ArithmeticService.interface.IArithmeticService2.SERVICE_ID, runTestStudent2)

    def test_runtest_struct_allDirection(self):
        self.runner.runTest(test_ArithmeticService.interface.IArithmeticService2.SERVICE_ID,
                             runtest_struct_allDirection)

    def test_runTesttestQuit(self):
        self.runner.runTest(test_unit_test_common.interface.ICommon.SERVICE_ID, runTesttestQuit)

    def runTests(self):
        self.test_runTesttestPrepareEnv()
        self.test_runTestArithmeticServiceID()
        self.test_runGetMember1()
        self.test_runNestedStruct1()
#        self.test_runGetMember2()
        self.test_runTestString1()
        self.test_runTestReturnString1()
        self.test_runSendIntSizes()
        self.test_runSendUnsignedIntSizes()  
        self.test_runTestStudent1()
        self.test_runTestStudent2()
        self.test_runtest_struct_allDirection()
        self.test_runTesttestQuit()

#runReturnStruct1(client1)
#runTestSendingByrefMembers(client2)

###############################################################################
# Main
###############################################################################

if __name__ == "__main__":
    tester = TestStruct()
    tester.runTests()
