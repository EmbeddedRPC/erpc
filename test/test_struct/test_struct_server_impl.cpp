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

/*void test_struct_allDirection(const AllTypes *a, const AllTypes *b, AllTypes *c, AllTypes **d, AllTypes *e)
{
    char text2[] = "World";

    c->number = a->number;
    (*d)->number = b->number;
    e->number = 2 * e->number;

    int lenText = strlen(a->text);
    c->text = (char *)malloc(lenText + 1);
    strcpy(c->text, a->text);
    lenText = strlen(b->text);
    (*d)->text = (char *)malloc(lenText + 1);
    strcpy((*d)->text, b->text);
    strcpy(e->text, text2);
    free(a->text);
    free(b->text);

    c->color = a->color;
    (*d)->color = b->color;
    e->color = green;

    c->c.m = a->c.m;
    c->c.n = a->c.n;
    (*d)->c.m = b->c.m;
    (*d)->c.n = b->c.n;
    e->c.m = 2 * e->c.m;
    e->c.n = 2 * e->c.n;

    uint32_t elementsCount = a->list_numbers.elementsCount;
    c->list_numbers.elementsCount = c->list_text.elementsCount = elementsCount;
    (*d)->list_numbers.elementsCount = (*d)->list_text.elementsCount = elementsCount;
    c->list_numbers.elements = (int32_t *)malloc(elementsCount * sizeof(int32_t));
    c->list_text.elements = (char **)malloc(elementsCount * sizeof(char *));
    (*d)->list_numbers.elements = (int32_t *)malloc(elementsCount * sizeof(int32_t));
    (*d)->list_text.elements = (char **)malloc(elementsCount * sizeof(char *));
    for (uint32_t i = 0; i < elementsCount; ++i)
    {
        c->list_numbers.elements[i] = a->list_numbers.elements[i];
        lenText = strlen(a->list_text.elements[i]);
        c->list_text.elements[i] = (char *)malloc(lenText + 1);
        strcpy(c->list_text.elements[i], a->list_text.elements[i]);

        (*d)->list_numbers.elements[i] = b->list_numbers.elements[i];
        lenText = strlen(b->list_text.elements[i]);
        (*d)->list_text.elements[i] = (char *)malloc(lenText + 1);
        strcpy((*d)->list_text.elements[i], b->list_text.elements[i]);

        e->list_numbers.elements[i] = 2 * e->list_numbers.elements[i];
        strcpy(e->list_text.elements[i], text2);

        free(a->list_text.elements[i]);
        free(b->list_text.elements[i]);
    }
    free(a->list_numbers.elements);
    free(b->list_numbers.elements);
    free(a->list_text.elements);
    free(b->list_text.elements);

    for (uint32_t i = 0; i < elementsCount; ++i)
    {
        c->array_numbers[i] = a->array_numbers[i];
        lenText = strlen(a->array_text[i]);
        c->array_text[i] = (char *)malloc(lenText + 1);
        strcpy(c->array_text[i], a->array_text[i]);

        (*d)->array_numbers[i] = b->array_numbers[i];
        lenText = strlen(b->array_text[i]);
        (*d)->array_text[i] = (char *)malloc(lenText + 1);
        strcpy((*d)->array_text[i], b->array_text[i]);

        e->array_numbers[i] = 2 * e->array_numbers[i];
        strcpy(e->array_text[i], text2);

        free(a->array_text[i]);
        free(b->array_text[i]);
    }

    c->binary_numbers.elementsCount = (*d)->binary_numbers.elementsCount = a->binary_numbers.elementsCount;
    c->binary_numbers.elements = (uint8_t *)malloc(c->binary_numbers.elementsCount * sizeof(uint8_t));
    (*d)->binary_numbers.elements = (uint8_t *)malloc((*d)->binary_numbers.elementsCount * sizeof(uint8_t));
    for (uint32_t i = 0; i < a->binary_numbers.elementsCount; ++i)
    {
        c->binary_numbers.elements[i] = a->binary_numbers.elements[i];
        (*d)->binary_numbers.elements[i] = b->binary_numbers.elements[i];
        e->binary_numbers.elements[i] = 2 * e->binary_numbers.elements[i];
    }
    free(a->binary_numbers.elements);
    free(b->binary_numbers.elements);

    free((void *)a);
    free((void *)b);
}*/

/*AnonymousStruct *getAnonymousStruct(const AnonymousStruct *a)
{
    AnonymousStruct *anonymousStruct = (AnonymousStruct *)malloc(sizeof(AnonymousStruct));
    anonymousStruct->a = a->a + 2;
    anonymousStruct->b = a->b + 2;
    free((void *)a);
    return anonymousStruct;
}*/

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
