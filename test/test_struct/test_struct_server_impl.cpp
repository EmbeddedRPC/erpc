/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "erpc_server_setup.h"
#include "test_ArithmeticService_server.h"
#include "unit_test.h"
#include "unit_test_wrapped.h"
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
// Implementation of function code
////////////////////////////////////////////////////////////////////////////////

int32_t getMember(const C *c)
{
    int32_t returnValue = c->m;
    return returnValue;
}

int32_t sendNestedStruct(const D *d)
{
    int dInt = d->c.m;
    return dInt;
}

B *returnStruct(float x, float y)
{
    B *b = (B *)erpc_malloc(sizeof(B));
    b->x = x;
    b->y = y;
    return b;
}

B *getMemberTest2(const A *a)
{
    B *result = (B *)erpc_malloc(sizeof(B));
    result->x = a->c.m;
    result->y = a->b.x;
    return result;
}

int32_t sendManyInts(const F *f)
{
    if (f->a == -1 && f->b == -1 && f->c == -1 && f->d == -1)
    {
        return 0;
    }
    return 1;
}

int32_t sendManyUInts(const G *g)
{
    if (g->a == 0xFF && g->b == 0xFFFF && g->c == 0xFFFFFFFF && g->d == 0xFFFFFFFFFFFFFFFF)
    {
        return 0;
    }
    return 1;
}

int32_t checkString(const primate *p)
{
    int b = strcmp(p->species, "Ape");
    erpc_free(p->species);
    erpc_free((void *)p);
    return b;
}

stringStruct *returnStrings()
{
    stringStruct *str_str = (stringStruct *)erpc_malloc(sizeof(stringStruct));
    char *str1 = (char *)erpc_malloc(10 * sizeof(char));
    strcpy(str1, "One");
    char *str2 = (char *)erpc_malloc(10 * sizeof(char));
    strcpy(str2, "Two");
    char *str3 = (char *)erpc_malloc(10 * sizeof(char));
    strcpy(str3, "Three");
    str_str->one = str1;
    str_str->two = str2;
    str_str->three = str3;
    return str_str;
}

char *getStudentName(const student *stud)
{
    int studNameLen = strlen(stud->name);
    char *studName = (char *)erpc_malloc(studNameLen * sizeof(char) + 1);
    strcpy(studName, stud->name);
    return studName;
}

float getStudentTestAverage(const student *stud)
{
    int16_t i = 0;
    int16_t array_size = 3;
    float total = 0;
    for (; i < array_size; ++i)
    {
        total += stud->test_grades[i];
    }
    return float(total / array_size);
}

int32_t getStudentYear(const student *stud)
{
    int32_t year = stud->school_year;
    return year;
}

int32_t getStudentAge(const student *stud)
{
    int studAge = stud->age;
    return studAge;
}

student *createStudent(const char *name, float test_scores[3], school_year_t year, int32_t age)
{
    // Log::info("returning from createStudent()\n");
    student *newStudent = (student *)erpc_malloc(sizeof(student));
    newStudent->name = (char *)erpc_malloc((strlen(name) + 1) * sizeof(char));
    strcpy(newStudent->name, name);
    for (uint8_t i = 0; i < 3; ++i)
        newStudent->test_grades[i] = test_scores[i];
    newStudent->school_year = year;
    newStudent->age = age;
    // Log::info("returning from createStudent()\n");
    return newStudent;
}

void test_struct_allDirection(const AllTypes *a, const AllTypes *b, AllTypes *e)
{
    char text2[] = "World";

    e->number = a->number * b->number;

    strcpy(e->text, text2);

    e->color = green;

    e->c.m = a->c.m * b->c.m;
    e->c.n = a->c.n * b->c.n;

    uint32_t elementsCount = a->list_numbers.elementsCount;
    for (uint32_t i = 0; i < elementsCount; ++i)
    {
        e->list_numbers.elements[i] = a->list_numbers.elements[i] * b->list_numbers.elements[i];
        strcpy(e->list_text.elements[i], text2);
    }

    for (uint32_t i = 0; i < elementsCount; ++i)
    {
        e->array_numbers[i] = a->array_numbers[i] * b->array_numbers[i];
        strcpy(e->array_text[i], text2);
    }

    for (uint32_t i = 0; i < a->binary_numbers.dataLength; ++i)
    {
        e->binary_numbers.data[i] = a->binary_numbers.data[i] * b->binary_numbers.data[i];
    }
}

bool testSendingByrefMembers(const StructWithByrefMembers *s)
{
    if (s->a == (A*)0xED && *s->b == 4)
    {
        return true;
    }
    return false;
}

////////////////////////////////////////////////////////////////////////////////
// Add service to server code
////////////////////////////////////////////////////////////////////////////////

void add_services(erpc::SimpleServer *server)
{
    // define services to add on heap
    // allocate on heap so service doesn't go out of scope at end of method
    // NOTE: possible memory leak? not ever deleting
    ArithmeticService1_service *svc1 = new ArithmeticService1_service();
    ArithmeticService2_service *svc2 = new ArithmeticService2_service();

    // add services
    server->addService(svc1);
    server->addService(svc2);
}

#ifdef __cplusplus
extern "C" {
#endif
void add_services_to_server()
{
    erpc_add_service_to_server(create_ArithmeticService1_service());
    erpc_add_service_to_server(create_ArithmeticService2_service());
}
#ifdef __cplusplus
}
#endif
