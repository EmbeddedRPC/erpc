#!/usr/bin/python

# Copyright 2021-2024 NXP
#
# SPDX-License-Identifier: BSD-3-Clause

import erpc
import pytest

from .service.erpc_outputs.test_ArithmeticService import common
from .service.erpc_outputs.test_ArithmeticService.client import ArithmeticService1Client
from .service.erpc_outputs.test_ArithmeticService.client import ArithmeticService2Client
from .service.erpc_outputs.test_ArithmeticService.server import ArithmeticService1Service
from .service.erpc_outputs.test_ArithmeticService.server import ArithmeticService2Service
from .service.erpc_outputs.test_ArithmeticService.interface import IArithmeticService1
from .service.erpc_outputs.test_ArithmeticService.interface import IArithmeticService2


###############################################################################
# Variables
###############################################################################


###############################################################################
# Fixtures
###############################################################################
@pytest.fixture()
def client1(client_manager):
    if client_manager is None:
        pytest.skip()

    return ArithmeticService1Client(client_manager)


@pytest.fixture()
def client2(client_manager):
    if client_manager is None:
        pytest.skip()

    return ArithmeticService2Client(client_manager)


@pytest.fixture(scope="module")
def services():
    return [
        ArithmeticService1Service(StructTestServiceHandler1()),
        ArithmeticService2Service(StructTestServiceHandler2())
    ]


###############################################################################
# Handlers
###############################################################################
class StructTestServiceHandler1(IArithmeticService1):
    def getMember(self, c):
        return c.m

    def sendNestedStruct(self, d):
        return d.c.m

    def returnStruct(self, x, y):
        b = common.B()
        b.x = x
        b.y = y
        return b

    def getMemberTest2(self, a):
        result = common.B()
        result.x = a.c.m
        result.y = a.b.x
        return result

    def checkString(self, p):
        return not (p.species == "Ape")

    def returnStrings(self):
        str_str = common.stringStruct(one="One", two="Two", three="Three")
        return str_str


class StructTestServiceHandler2(IArithmeticService2):
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
        newStudent = common.student(name=name, test_grades=test_scores, school_year=year, age=age)
        return newStudent

    def test_struct_allDirection(self, a, b, e):
        e.value.number = a.number * b.number
        e.value.text = "World"
        e.value.color = common.enumColor.green
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
# Tests
###############################################################################
def test_GetMember1(client1):
    c = common.C(m=4, n=5)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    r = client1.getMember(c)
    assert r == 4


def test_TestArithmeticServiceID():
    assert 3 == IArithmeticService1.SERVICE_ID


def test_NestedStruct1(client1):
    c = common.C(m=4, n=5)
    d = common.D(c)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    r = client1.sendNestedStruct(d)
    assert r == 4


def test_ReturnStruct1(client1):
    b = common.B()
    # send request to the server
    print('\r\neRPC request is sent to the server')
    b = client1.returnStruct(float(3.14), float(2.71828))
    print(b)
    assert round(b.x, 5) == float(3.14)
    assert round(b.y, 5) == float(2.71828)


def test_SendIntSizes(client2):
    f = common.F(-1, -1, -1, -1)
    # print(f)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    assert 0 == client2.sendManyInts(f)


def test_SendUnsignedIntSizes(client2):
    g = common.G(0xFF, 0xFFFF, 0xFFFFFFFF, 0xFFFFFFFFFFFFFFFF)
    # print(g)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    assert 0 == client2.sendManyUInts(g)


def test_GetMember2(client1):
    c = common.C(m=4, n=5)
    b = common.B(x=float(3.14), y=float(2.71828))
    a = common.A(b=b, c=c)
    bb = common.B()
    # send request to the server
    print('\r\neRPC request is sent to the server')
    bb = client1.getMemberTest2(a)
    # print(bb)
    assert round(float(bb.x), 2) == float(4.0)
    assert round(float(bb.y), 2) == float(3.14)


def test_TestString1(client1):
    prim8 = common.primate(species="Ape", is_ape=False)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    assert 0 == client1.checkString(prim8)


def test_TestReturnString1(client1):
    result = common.stringStruct()
    # send request to the server
    print('\r\neRPC request is sent to the server')
    result = client1.returnStrings()
    assert result.one == "One"
    assert result.two == "Two"
    assert result.three == "Three"


def test_TestStudent1(client2):
    stud = common.student(name="Donnie Darko",
                          test_grades=[float(85.75), float(93.29), float(92.46)],
                          school_year=common.school_year_t.SENIOR, age=19)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    assert "Donnie Darko" == client2.getStudentName(stud)
    assert float(
        (stud.test_grades[0] + stud.test_grades[1] + stud.test_grades[2]) / 3) == client2.getStudentTestAverage(
        stud)
    assert 19 == client2.getStudentAge(stud)
    assert 12 == common.school_year_t.SENIOR
    assert common.school_year_t.SENIOR == client2.getStudentYear(stud)


def test_TestStudent2(client2):
    stud = common.student()
    # send request to the server
    print('\r\neRPC request is sent to the server')
    stud = client2.createStudent("George Bush", [float(65.32), float(69.68), float(0)],
                                 common.school_year_t.FRESHMAN, 68)
    assert "George Bush" == client2.getStudentName(stud)
    assert float((65.32 + 69.68 + 0) / 3) == client2.getStudentTestAverage(stud)
    assert 68 == client2.getStudentAge(stud)
    assert common.school_year_t.FRESHMAN == client2.getStudentYear(stud)


def test_struct_allDirection(client2):
    a_binary = bytearray(b'')
    b_binary = bytearray(b'')
    e_binary = bytearray(b'')
    for x in range(2):
        a_binary.append(x)
        b_binary.append(x * 2)
        e_binary.append(x)
    a = common.AllTypes(number=5, text="Hello",
                        color=common.enumColor.red,
                        c=common.C(m=5, n=20), list_numbers=[0, 1],
                        list_text=["Hello", "Hello"], array_numbers=[0, 1],
                        array_text=["Hello", "Hello"], binary_numbers=a_binary)
    b = common.AllTypes(number=10, text="World",
                        color=common.enumColor.green,
                        c=common.C(m=7, n=17), list_numbers=[1, 2],
                        list_text=["World", "World"], array_numbers=[1, 2],
                        array_text=["World", "World"], binary_numbers=b_binary)
    e = erpc.Reference(common.AllTypes(number=15, text="Hello",
                                       color=common.enumColor.blue,
                                       c=common.C(m=4, n=14),
                                       list_numbers=[2, 3], list_text=["Hello", "Hello"],
                                       array_numbers=[2, 3], array_text=["Hello", "Hello"],
                                       binary_numbers=e_binary))
    # send request to the server
    print('\r\neRPC request is sent to the server')
    client2.test_struct_allDirection(a, b, e)
    assert e.value.number == a.number * b.number
    assert e.value.text == "World"
    assert e.value.color == common.enumColor.green
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


@pytest.mark.skip("Not implemented")
def test_TestSendingByrefMembers(client2):
    s = common.StructWithByrefMembers()
    b = 4
    s.a = 0xED
    s.b = id(b)
    # send request to the server
    print('\r\neRPC request is sent to the server')
    assert True == client2.testSendingByrefMembers(s)


def test_Quit(common_client):
    # send request to the server
    print("eRPC quit request send to the server.")
    return common_client.quit()
