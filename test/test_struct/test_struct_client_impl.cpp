/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "gtest.h"
#include "test_ArithmeticService.h"

#include <string.h>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Unit test Implementation code
////////////////////////////////////////////////////////////////////////////////

TEST(test_struct, GetMember1)
{
    C c = { 4, 5 };
    EXPECT_EQ(4, getMember(&c));
}

TEST(test_struct, NestedStruct1)
{
    C c = { 4, 5 };
    D d = { c };
    EXPECT_EQ(4, sendNestedStruct(&d));
}

TEST(test_struct, ReturnStruct1)
{
    B *b;
    b = returnStruct(3.14, 2.71828);
    EXPECT_EQ((float)b->x, (float)3.14);
    EXPECT_EQ((float)b->y, (float)2.71828);
    erpc_free(b);
}

TEST(test_struct, SendIntSizes)
{
    F f = { (int8_t)0xFF, (int16_t)0xFFFF, (int32_t)0xFFFFFFFF, (int64_t)0xFFFFFFFFFFFFFFFF };
    EXPECT_EQ(0, sendManyInts(&f));
}

TEST(test_struct, SendUnsignedIntSizes)
{
    G g = { 0xFF, 0xFFFF, 0xFFFFFFFF, 0xFFFFFFFFFFFFFFFF };
    EXPECT_EQ(0, sendManyUInts(&g));
}

TEST(test_struct, GetMember2)
{
    C c = { 4, 5 };
    B b = { 3.14, 2.71828 };
    A a = { b, c };
    B *bb = getMemberTest2(&a);
    EXPECT_EQ((float)bb->x, (float)4.0);
    EXPECT_EQ((float)bb->y, (float)3.14);
    erpc_free(bb);
}

TEST(test_struct, TestString1)
{
    primate prim8;
    char str[] = "Ape";
    prim8.species = &str[0];
    prim8.is_ape = false;
    EXPECT_EQ(checkString(&prim8), 0);
}

TEST(test_struct, TestReturnString1)
{
    stringStruct *result = returnStrings();
    EXPECT_STREQ("One", result->one);
    EXPECT_STREQ("Two", result->two);
    EXPECT_STREQ("Three", result->three);
    erpc_free(result->one);
    erpc_free(result->two);
    erpc_free(result->three);
    erpc_free(result);
}

TEST(test_struct, TestStudent1)
{
    student *stud = (student *)erpc_malloc(sizeof(student));
    char name[] = "Donnie Darko";
    stud->name = &name[0];
    stud->test_grades[0] = 85.75;
    stud->test_grades[1] = 93.29;
    stud->test_grades[2] = 92.46;
    float test_avg = (stud->test_grades[0] + stud->test_grades[1] + stud->test_grades[2]) / 3;
    stud->school_year = SENIOR;
    stud->age = 19;
    char *resultStudent = getStudentName(stud);
    EXPECT_STREQ("Donnie Darko", resultStudent);
    erpc_free(resultStudent);
    EXPECT_EQ(getStudentTestAverage(stud), test_avg);
    EXPECT_EQ(getStudentAge(stud), 19);
    EXPECT_EQ(SENIOR, 12);
    EXPECT_EQ(getStudentYear(stud), SENIOR);
    erpc_free(stud);
}

TEST(test_struct, TestStudent2)
{
    student *stud;
    float scores[3] = { 65.32, 69.68, 0 };
    stud = createStudent("George Bush", scores, FRESHMAN, 68);
    char *resultStudent = getStudentName(stud);
    EXPECT_STREQ("George Bush", resultStudent);
    erpc_free(resultStudent);
    EXPECT_EQ(getStudentTestAverage(stud), 45.);
    EXPECT_EQ(getStudentAge(stud), 68);
    EXPECT_EQ(getStudentYear(stud), FRESHMAN);
    erpc_free(stud->name);
    erpc_free(stud);
}

TEST(test_struct, test_struct_allDirection)
{
    AllTypes a;
    AllTypes b;
    AllTypes e;

    char text1[] = "Hello";
    char text2[] = "World";
    uint32_t lenText1 = strlen(text1);
    uint32_t lenText2 = strlen(text2);
    a.number = 5;
    b.number = 10;
    e.number = 15;

    a.text = (char *)erpc_malloc(lenText1 + 1);
    strcpy(a.text, text1);
    b.text = (char *)erpc_malloc(lenText2 + 1);
    strcpy(b.text, text2);
    // c.text = (char*)erpc_malloc(lenText1+1);
    e.text = (char *)erpc_malloc(lenText1 + 1);
    strcpy(e.text, text1);

    a.color = red;
    b.color = green;
    e.color = blue;

    a.c.m = 5;
    a.c.n = 20;
    b.c.m = 7;
    b.c.n = 17;
    e.c.m = 4;
    e.c.n = 14;

    a.list_text.elementsCount = a.list_numbers.elementsCount = 2;
    b.list_text.elementsCount = b.list_numbers.elementsCount = 2;
    // c.list_text.elementsCount = c.list_numbers.elementsCount = 2;
    e.list_text.elementsCount = e.list_numbers.elementsCount = 2;

    a.list_numbers.elements = (int32_t *)erpc_malloc(2 * sizeof(int32_t));
    b.list_numbers.elements = (int32_t *)erpc_malloc(2 * sizeof(int32_t));
    // c.list_numbers.elements = (int32_t*)erpc_malloc(2*sizeof(int32_t));
    e.list_numbers.elements = (int32_t *)erpc_malloc(2 * sizeof(int32_t));

    a.list_text.elements = (char **)erpc_malloc(2 * sizeof(char *));
    b.list_text.elements = (char **)erpc_malloc(2 * sizeof(char *));
    // c.list_text.elements = (char**)erpc_malloc(2*sizeof(char*));
    e.list_text.elements = (char **)erpc_malloc(2 * sizeof(char *));

    for (uint32_t i = 0; i < a.list_text.elementsCount; ++i)
    {
        a.list_numbers.elements[i] = i;
        a.list_text.elements[i] = (char *)erpc_malloc(lenText1 + 1);
        strcpy(a.list_text.elements[i], text1);
        b.list_numbers.elements[i] = i + 1;
        b.list_text.elements[i] = (char *)erpc_malloc(lenText2 + 1);
        strcpy(b.list_text.elements[i], text2);
        // c.list_text.elements[i] = (char*)erpc_malloc(lenText1+1);
        e.list_numbers.elements[i] = i + 2;
        e.list_text.elements[i] = (char *)erpc_malloc(lenText1 + 1);
        strcpy(e.list_text.elements[i], text1);
    }

    for (uint32_t i = 0; i < 2; ++i)
    {
        a.array_numbers[i] = i;
        a.array_text[i] = (char *)erpc_malloc(lenText1 + 1);
        strcpy(a.array_text[i], text1);
        b.array_numbers[i] = i + 1;
        b.array_text[i] = (char *)erpc_malloc(lenText2 + 1);
        strcpy(b.array_text[i], text2);
        // c.array_text[i] = (char*)erpc_malloc(lenText1+1);
        e.array_numbers[i] = i + 2;
        e.array_text[i] = (char *)erpc_malloc(lenText1 + 1);
        strcpy(e.array_text[i], text1);
    }

    a.binary_numbers.dataLength = b.binary_numbers.dataLength = e.binary_numbers.dataLength = 2;
    a.binary_numbers.data = (uint8_t *)erpc_malloc(a.binary_numbers.dataLength * sizeof(uint8_t));
    b.binary_numbers.data = (uint8_t *)erpc_malloc(b.binary_numbers.dataLength * sizeof(uint8_t));
    e.binary_numbers.data = (uint8_t *)erpc_malloc(e.binary_numbers.dataLength * sizeof(uint8_t));

    for (uint8_t i = 0; i < a.binary_numbers.dataLength; ++i)
    {
        a.binary_numbers.data[i] = i;
        b.binary_numbers.data[i] = i * 2;
        e.binary_numbers.data[i] = i;
    }

    test_struct_allDirection(&a, &b, &e);

    EXPECT_EQ(e.number, a.number * b.number);

    EXPECT_STREQ(text2, e.text);
    erpc_free(a.text);
    erpc_free(b.text);
    erpc_free(e.text);

    EXPECT_EQ(e.color, green);

    EXPECT_EQ(e.c.m, a.c.m * b.c.m);
    EXPECT_EQ(e.c.n, a.c.n * b.c.n);

    EXPECT_EQ(e.list_numbers.elementsCount, 2);

    for (uint32_t i = 0; i < a.list_numbers.elementsCount; ++i)
    {
        EXPECT_EQ(e.list_numbers.elements[i], a.list_numbers.elements[i] * b.list_numbers.elements[i]);

        EXPECT_STREQ(text2, e.list_text.elements[i]);
        erpc_free(a.list_text.elements[i]);
        erpc_free(b.list_text.elements[i]);
        erpc_free(e.list_text.elements[i]);
    }
    erpc_free(a.list_text.elements);
    erpc_free(b.list_text.elements);
    erpc_free(e.list_text.elements);
    erpc_free(a.list_numbers.elements);
    erpc_free(b.list_numbers.elements);
    erpc_free(e.list_numbers.elements);

    for (uint32_t i = 0; i < 2; ++i)
    {
        EXPECT_EQ(e.array_numbers[i], a.array_numbers[i] * b.array_numbers[i]);

        EXPECT_STREQ(text2, e.array_text[i]);
        erpc_free(a.array_text[i]);
        erpc_free(b.array_text[i]);
        erpc_free(e.array_text[i]);
    }

    for (uint8_t i = 0; i < a.binary_numbers.dataLength; ++i)
    {
        EXPECT_EQ(e.binary_numbers.data[i], a.binary_numbers.data[i] * b.binary_numbers.data[i]);
    }
    erpc_free(a.binary_numbers.data);
    erpc_free(b.binary_numbers.data);
    erpc_free(e.binary_numbers.data);
}

TEST(test_struct, TestSendingByrefMembers)
{
    StructWithByrefMembers s;
    int32_t b = 4;
    s.a = (A *)0xED;
    s.b = &b;

    EXPECT_TRUE(testSendingByrefMembers(&s));
}
