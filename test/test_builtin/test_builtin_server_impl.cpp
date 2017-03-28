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
#include "test_builtin_server.h"
#include "unit_test.h"
#include "unit_test_wrapped.h"
#include <stdlib.h>
#include <string.h>

////////////////////////////////////////////////////////////////////////////////
// Implementation of function code
////////////////////////////////////////////////////////////////////////////////

struct saveVariables
{
    int32_t int32A;
    int32_t int32B;
    char *stringA;
    char *stringB;
};

struct saveVariables saveVariables1;

void test_int32_in(int32_t a)
{
    saveVariables1.int32A = a;
}

void test_int32_in2(int32_t b)
{
    saveVariables1.int32B = b;
}

void test_int32_out(int32_t *c)
{
    *c = saveVariables1.int32A;
}

void test_int32_outbyref(int32_t *d)
{
    *d = saveVariables1.int32B;
}

void test_int32_inout(int32_t *e)
{
    *e = (*e) + 1;
}

int32_t test_int32_return()
{
    return saveVariables1.int32A * saveVariables1.int32B;
}

int32_t test_int32_allDirection(int32_t a, int32_t b, int32_t *c, int32_t *d, int32_t *e)
{
    *c = a;
    *d = b;
    *e = 2 * (*e);
    return a * b;
}

void test_float_inout(float a, float *b)
{
    *b = a;
}

void test_double_inout(double a, double *b)
{
    *b = a;
}

void test_string_in(const char *a)
{
    saveVariables1.stringA = (char *)erpc_malloc(strlen(a) + 1);
    memcpy(saveVariables1.stringA, a, strlen(a));
    saveVariables1.stringA[strlen(a)] = '\0';
}

void test_string_in2(const char *b)
{
    saveVariables1.stringB = (char *)erpc_malloc(strlen(b) + 1);
    memcpy(saveVariables1.stringB, b, strlen(b));
    saveVariables1.stringB[strlen(b)] = '\0';
}

void test_string_out(char *c)
{
    strcpy(c, saveVariables1.stringA);
    // allocated via test_string_in()
    erpc_free(saveVariables1.stringA);
}

void test_string_outbyref(char **d)
{
    // allocated via test_string_in2()
    *d = saveVariables1.stringB;
}

void test_string_inout(char *e)
{
    strcat(e, saveVariables1.stringB);
    // allocated via test_string_in2()
    erpc_free(saveVariables1.stringB);
}

char *test_string_return()
{
    char *r = (char *)erpc_malloc(13 * sizeof(char));
    strcpy(r, "Hello World!");
    return r;
}

char *test_string_allDirection(const char *a, const char *b, char *c, char **d, char *e)
{
    char *r = (char *)erpc_malloc(13 * sizeof(char));
    strcpy(c, a);
    *d = (char *)erpc_malloc(strlen(b));
    strcpy(*d, b);
    strcat(e, b);
    strcpy(r, e);
    return r;
}

char *test_string_empty(const char *a, const char *b, char *c, char **d, char *e)
{
    return NULL;
}

int32_t sendHello(const char *myStr)
{
    const char *str = "Hello World!";
    int result = strcmp(myStr, str);
    return result;
}

int32_t sendTwoStrings(const char *myStr1, const char *myStr2)
{
    const char *myStr1_check = "String one.";
    const char *myStr2_check = "String two.";
    int result = (strcmp(myStr1, myStr1_check) | strcmp(myStr2, myStr2_check));
    return result;
}

char *returnHello()
{
    char *hello = (char *)erpc_malloc(10 * sizeof(char));
    memcpy(hello, "Hello", strlen("Hello") + 1);
    return hello;
}

////////////////////////////////////////////////////////////////////////////////
// Add service to server code
////////////////////////////////////////////////////////////////////////////////

void add_services(erpc::SimpleServer *server)
{
    // define services to add on heap
    // allocate on heap so service doesn't go out of scope at end of method
    // NOTE: possible memory leak? not ever deleting
    BuiltinServices_service *svc = new BuiltinServices_service();

    // add services
    server->addService(svc);
}

#ifdef __cplusplus
extern "C" {
#endif
void add_services_to_server()
{
    erpc_add_service_to_server(create_BuiltinServices_service());
}
#ifdef __cplusplus
}
#endif
