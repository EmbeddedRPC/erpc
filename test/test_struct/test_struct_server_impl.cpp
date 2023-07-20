/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_server_setup.h"

#include "c_test_ArithmeticService_server.h"
#include "c_test_unit_test_common_server.h"
#include "test_ArithmeticService_server.hpp"
#include "unit_test.h"
#include "unit_test_wrapped.h"

#include <stdlib.h>
#include <string.h>

using namespace erpc;
using namespace erpcshim;

ArithmeticService1_service *svc1;
ArithmeticService2_service *svc2;

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

student *createStudent(const char *name, const float test_scores[3], school_year_t year, int32_t age)
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
    if (s->a == (A *)0xED && *s->b == 4)
    {
        return true;
    }
    return false;
}

class ArithmeticService1_server : public ArithmeticService1_interface
{
public:
    int32_t getMember(const C *c)
    {
        int32_t result;
        result = ::getMember(c);

        return result;
    }

    B *returnStruct(float a, float b)
    {
        B *result = NULL;
        result = ::returnStruct(a, b);

        return result;
    }

    B *getMemberTest2(const A *a)
    {
        B *result = NULL;
        result = ::getMemberTest2(a);

        return result;
    }

    int32_t sendNestedStruct(const D *d)
    {
        int32_t result;
        result = ::sendNestedStruct(d);

        return result;
    }

    int32_t checkString(const primate *p)
    {
        int32_t result;
        result = ::checkString(p);

        return result;
    }

    stringStruct *returnStrings(void)
    {
        stringStruct *result = NULL;
        result = ::returnStrings();

        return result;
    }
};

class ArithmeticService2_server : public ArithmeticService2_interface
{
public:
    int32_t sendManyInts(const F *f)
    {
        int32_t result;
        result = ::sendManyInts(f);

        return result;
    }

    int32_t sendManyUInts(const G *g)
    {
        int32_t result;
        result = ::sendManyUInts(g);

        return result;
    }

    char *getStudentName(const student *stud)
    {
        char *result = NULL;
        result = ::getStudentName(stud);

        return result;
    }

    float getStudentTestAverage(const student *stud)
    {
        float result;
        result = ::getStudentTestAverage(stud);

        return result;
    }

    int32_t getStudentYear(const student *stud)
    {
        int32_t result;
        result = ::getStudentYear(stud);

        return result;
    }

    int32_t getStudentAge(const student *stud)
    {
        int32_t result;
        result = ::getStudentAge(stud);

        return result;
    }

    student *createStudent(const char *name, const float test_scores[3], school_year_t year, int32_t age)
    {
        student *result = NULL;
        result = ::createStudent(name, test_scores, year, age);

        return result;
    }

    void test_struct_allDirection(const AllTypes *a, const AllTypes *b, AllTypes *e)
    {
        ::test_struct_allDirection(a, b, e);
    }

    bool testSendingByrefMembers(const StructWithByrefMembers *s)
    {
        bool result;
        result = ::testSendingByrefMembers(s);

        return result;
    }
};

////////////////////////////////////////////////////////////////////////////////
// Add service to server code
////////////////////////////////////////////////////////////////////////////////

void add_services(erpc::SimpleServer *server)
{
    // define services to add on heap
    // allocate on heap so service doesn't go out of scope at end of method
    svc1 = new ArithmeticService1_service(new ArithmeticService1_server());
    svc2 = new ArithmeticService2_service(new ArithmeticService2_server());

    // add services
    server->addService(svc1);
    server->addService(svc2);
}

////////////////////////////////////////////////////////////////////////////////
// Remove service from server code
////////////////////////////////////////////////////////////////////////////////

void remove_services(erpc::SimpleServer *server)
{
    /* Remove services
     * Example: server->removeService (svc);
     */
    server->removeService(svc1);
    server->removeService(svc2);
    /* Delete unused service
     */
    delete svc1->getHandler();
    delete svc2->getHandler();
    delete svc1;
    delete svc2;
}

#ifdef __cplusplus
extern "C" {
#endif
erpc_service_t service1 = NULL;
erpc_service_t service2 = NULL;
void add_services_to_server(erpc_server_t server)
{
    service1 = create_ArithmeticService1_service();
    service2 = create_ArithmeticService2_service();
    erpc_add_service_to_server(server, service1);
    erpc_add_service_to_server(server, service2);
}

void remove_services_from_server(erpc_server_t server)
{
    erpc_remove_service_from_server(server, service1);
    erpc_remove_service_from_server(server, service2);
    destroy_ArithmeticService1_service(service1);
    destroy_ArithmeticService2_service(service2);
}

#ifdef __cplusplus
}
#endif
