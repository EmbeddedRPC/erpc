/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2021 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_server_setup.h"

#include "c_test_server.h"
#include "test_server.hpp"
#include "unit_test.h"
#include "unit_test_wrapped.h"

#include <stdlib.h>
#include <string.h>

using namespace erpc;
using namespace erpcshim;

BuiltinServices_service *svc;

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

int32_t test_int32_allDirection(int32_t a, int32_t b, int32_t *c, int32_t *e)
{
    *c = a;
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

char *test_string_allDirection(const char *a, const char *b, char *c, char *e)
{
    char *r = (char *)erpc_malloc(13 * sizeof(char));
    strcpy(c, a);
    strcat(e, b);
    strcpy(r, e);
    return r;
}

char *test_string_empty(const char *a, const char *b, char *c, char *e)
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

class BuiltinServices_server : public BuiltinServices_interface
{
public:
    void test_int32_in(int32_t a) { ::test_int32_in(a); }

    void test_int32_in2(int32_t b) { ::test_int32_in2(b); }

    void test_int32_out(int32_t *c) { ::test_int32_out(c); }

    void test_int32_inout(int32_t *e) { ::test_int32_inout(e); }

    int32_t test_int32_return(void)
    {
        int32_t result;
        result = ::test_int32_return();

        return result;
    }

    int32_t test_int32_allDirection(int32_t a, int32_t b, int32_t *c, int32_t *e)
    {
        int32_t result;
        result = ::test_int32_allDirection(a, b, c, e);

        return result;
    }

    void test_float_inout(float a, float *b) { ::test_float_inout(a, b); }

    void test_double_inout(double a, double *b) { ::test_double_inout(a, b); }

    void test_string_in(const char *a) { ::test_string_in(a); }

    void test_string_in2(const char *b) { ::test_string_in2(b); }

    void test_string_out(char *c) { ::test_string_out(c); }

    void test_string_inout(char *e) { ::test_string_inout(e); }

    char *test_string_return(void)
    {
        char *result = NULL;
        result = ::test_string_return();

        return result;
    }

    char *test_string_allDirection(const char *a, const char *b, char *c, char *e)
    {
        char *result = NULL;
        result = ::test_string_allDirection(a, b, c, e);

        return result;
    }

    char *test_string_empty(const char *a, const char *b, char *c, char *e)
    {
        char *result = NULL;
        result = ::test_string_empty(a, b, c, e);

        return result;
    }

    int32_t sendHello(const char *str)
    {
        int32_t result;
        result = ::sendHello(str);

        return result;
    }

    int32_t sendTwoStrings(const char *myStr1, const char *myStr2)
    {
        int32_t result;
        result = ::sendTwoStrings(myStr1, myStr2);

        return result;
    }

    char *returnHello(void)
    {
        char *result = NULL;
        result = ::returnHello();

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
    svc = new BuiltinServices_service(new BuiltinServices_server());

    // add services
    server->addService(svc);
}

////////////////////////////////////////////////////////////////////////////////
// Remove service from server code
////////////////////////////////////////////////////////////////////////////////

void remove_services(erpc::SimpleServer *server)
{
    /* Remove services
     * Example: server->removeService (svc);
     */
    server->removeService(svc);
    /* Delete unused service
     */
    delete svc->getHandler();
    delete svc;
}

#ifdef __cplusplus
extern "C" {
#endif
erpc_service_t service_test = NULL;
void add_services_to_server(erpc_server_t server)
{
    service_test = create_BuiltinServices_service();
    erpc_add_service_to_server(server, service_test);
}

void remove_services_from_server(erpc_server_t server)
{
    erpc_remove_service_from_server(server, service_test);
    destroy_BuiltinServices_service(service_test);
}
#ifdef __cplusplus
}
#endif
