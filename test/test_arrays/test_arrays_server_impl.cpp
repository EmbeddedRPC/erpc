/*
 * Copyright (c) 2014-2016, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "erpc_server_setup.h"

#include "test_server.h"
#include "test_unit_test_common_server.h"
#include "unit_test.h"
#include "unit_test_wrapped.h"

#include <stdlib.h>
#include <string.h>

PointersService_service *svc;

////////////////////////////////////////////////////////////////////////////////
// Implementation of function code
////////////////////////////////////////////////////////////////////////////////

int32_t (*sendReceivedInt32(const int32_t arrayNumbers[12]))[12]
{
    int32_t(*sendArrays)[12] = (int32_t(*)[12])erpc_malloc(sizeof(int32_t[12]));

    for (int32_t i = 0; i < 12; ++i)
    {
        (*sendArrays)[i] = arrayNumbers[i];
    }
    return sendArrays;
}

int32_t (*sendReceived2Int32(int32_t arrayNumbers[12][10]))[12][10]
{
    int32_t(*sendArrays)[12][10] = (int32_t(*)[12][10])erpc_malloc(sizeof(int32_t[12][10]));
    for (int32_t i = 0; i < 12; ++i)
    {
        for (int32_t j = 0; j < 10; ++j)
        {
            (*sendArrays)[i][j] = arrayNumbers[i][j];
        }
    }
    return sendArrays;
}

char *(*sendReceivedString(char *arrayStrings[12]))[12]
{
    char *(*sendArrays)[12] = (char *(*)[12])erpc_malloc(sizeof(char *[12]));
    for (int32_t i = 0; i < 12; ++i)
    {
        uint32_t textLen = strlen(arrayStrings[i]);
        (*sendArrays)[i] = (char *)erpc_malloc(textLen + 1);
        strcpy((*sendArrays)[i], arrayStrings[i]);
    }
    return sendArrays;
}

char *(*sendReceived2String(char *arrayStrings[3][5]))[3][5]
{
    char *(*sendArrays)[3][5] = (char *(*)[3][5])erpc_malloc(sizeof(char *[3][5]));
    for (int32_t i = 0; i < 3; ++i)
    {
        for (int32_t j = 0; j < 5; ++j)
        {
            uint32_t textLen = strlen(arrayStrings[i][j]);
            (*sendArrays)[i][j] = (char *)erpc_malloc(textLen + 1);
            strcpy((*sendArrays)[i][j], arrayStrings[i][j]);
        }
    }
    return sendArrays;
}

enumColor (*sendReceivedEnum(const enumColor arrayEnums[3]))[3]
{
    enumColor(*sendArrays)[3] = (enumColor(*)[3])erpc_malloc(sizeof(enumColor[3]));

    for (int32_t i = 0; i < 3; ++i)
    {
        (*sendArrays)[i] = arrayEnums[i];
    }
    return sendArrays;
}

enumColor (*sendReceived2Enum(enumColor arrayEnums[3][3]))[3][3]
{
    enumColor(*sendArrays)[3][3] = (enumColor(*)[3][3])erpc_malloc(sizeof(enumColor[3][3]));
    for (int32_t i = 0; i < 3; ++i)
    {
        for (int32_t j = 0; j < 3; ++j)
        {
            (*sendArrays)[i][j] = arrayEnums[i][j];
        }
    }
    return sendArrays;
}

list_int32_1_t (*sendReceivedList(const list_int32_1_t arrayLists[2]))[2]
{
    uint32_t array_count = 2;

    list_int32_1_t(*sendArrays)[2] = (list_int32_1_t(*)[2])erpc_malloc(sizeof(list_int32_1_t[2]));

    for (uint32_t i = 0; i < array_count; ++i)
    {
        uint32_t elements_count = arrayLists[i].elementsCount;
        (*sendArrays)[i].elementsCount = elements_count;
        (*sendArrays)[i].elements = (int32_t *)erpc_malloc(elements_count * sizeof(int32_t));
        for (uint32_t j = 0; j < elements_count; ++j)
        {
            (*sendArrays)[i].elements[j] = arrayLists[i].elements[j];
        }
    }
    return sendArrays;
}

list_int32_1_t (*sendReceived2List(list_int32_1_t arrayLists[2][2]))[2][2]
{
    list_int32_1_t(*sendArrays)[2][2] = (list_int32_1_t(*)[2][2])erpc_malloc(sizeof(list_int32_1_t[2][2]));

    for (uint32_t k = 0; k < 2; ++k)
    {
        for (int32_t i = 0; i < 2; ++i)
        {
            uint32_t elements_count = arrayLists[k][i].elementsCount;
            (*sendArrays)[k][i].elementsCount = elements_count;
            (*sendArrays)[k][i].elements = (int32_t *)erpc_malloc(elements_count * sizeof(int32_t));
            for (uint32_t j = 0; j < elements_count; ++j)
            {
                (*sendArrays)[k][i].elements[j] = arrayLists[k][i].elements[j];
            }
        }
    }
    return sendArrays;
}

ArrayIntType *sendReceivedInt32Type(const ArrayIntType arrayNumbers)
{
    ArrayIntType *send = (ArrayIntType *)erpc_malloc(sizeof(ArrayIntType));

    for (int32_t i = 0; i < 12; ++i)
    {
        (*send)[i] = arrayNumbers[i];
    }

    return send;
}

Array2IntType *sendReceived2Int32Type(Array2IntType arrayNumbers)
{
    Array2IntType *sendArrays = (Array2IntType *)erpc_malloc(sizeof(Array2IntType));
    for (int32_t i = 0; i < 12; ++i)
    {
        for (int32_t j = 0; j < 10; ++j)
        {
            (*sendArrays)[i][j] = arrayNumbers[i][j];
        }
    }
    return sendArrays;
}

ArrayStringType *sendReceivedStringType(ArrayStringType arrayStrings)
{
    ArrayStringType *sendArrays = (ArrayStringType *)erpc_malloc(sizeof(ArrayStringType));
    for (int32_t i = 0; i < 12; ++i)
    {
        uint32_t textLen = strlen(arrayStrings[i]);
        (*sendArrays)[i] = (char *)erpc_malloc(textLen + 1);
        strcpy((*sendArrays)[i], arrayStrings[i]);
    }
    return sendArrays;
}

Array2StringType *sendReceived2StringType(Array2StringType arrayStrings)
{
    Array2StringType *sendArrays = (Array2StringType *)erpc_malloc(sizeof(Array2StringType));

    for (int32_t i = 0; i < 3; ++i)
    {
        for (int32_t j = 0; j < 5; ++j)
        {
            uint32_t textLen = strlen(arrayStrings[i][j]);
            (*sendArrays)[i][j] = (char *)erpc_malloc(textLen + 1);
            strcpy((*sendArrays)[i][j], arrayStrings[i][j]);
        }
    }
    return sendArrays;
}

ArrayEnumType *sendReceivedEnumType(const ArrayEnumType arrayEnums)
{
    ArrayEnumType *sendArrays = (ArrayEnumType *)erpc_malloc(sizeof(ArrayEnumType));

    for (int32_t i = 0; i < 3; ++i)
    {
        (*sendArrays)[i] = arrayEnums[i];
    }
    return sendArrays;
}

Array2EnumType *sendReceived2EnumType(Array2EnumType arrayEnums)
{
    Array2EnumType *sendArrays = (Array2EnumType *)erpc_malloc(sizeof(Array2EnumType));
    for (int32_t i = 0; i < 3; ++i)
    {
        for (int32_t j = 0; j < 3; ++j)
        {
            (*sendArrays)[i][j] = arrayEnums[i][j];
        }
    }
    return sendArrays;
}

ArrayStructType *sendReceivedStructType(const ArrayStructType arrayStructs)
{
    ArrayStructType *sendArrays = (ArrayStructType *)erpc_malloc(sizeof(ArrayStructType));

    for (int32_t i = 0; i < 3; ++i)
    {
        (*sendArrays)[i].m = arrayStructs[i].m;
        (*sendArrays)[i].n = arrayStructs[i].n;
    }
    return sendArrays;
}

Array2StructType *sendReceived2StructType(Array2StructType arrayStructs)
{
    Array2StructType *sendArrays = (Array2StructType *)erpc_malloc(sizeof(Array2StructType));
    for (int32_t i = 0; i < 3; ++i)
    {
        for (int32_t j = 0; j < 3; ++j)
        {
            (*sendArrays)[i][j].m = arrayStructs[i][j].m;
            (*sendArrays)[i][j].n = arrayStructs[i][j].n;
        }
    }
    return sendArrays;
}

ArrayListType *sendReceivedListType(const ArrayListType arrayLists)
{
    uint32_t array_count = 2;
    ArrayListType *sendArrays = (ArrayListType *)erpc_malloc(sizeof(ArrayListType));

    for (uint32_t i = 0; i < array_count; ++i)
    {
        uint32_t elements_count = arrayLists[i].elementsCount;
        (*sendArrays)[i].elementsCount = elements_count;
        (*sendArrays)[i].elements = (int32_t *)erpc_malloc(elements_count * sizeof(int32_t));
        for (uint32_t j = 0; j < elements_count; ++j)
        {
            (*sendArrays)[i].elements[j] = arrayLists[i].elements[j];
        }
    }
    return sendArrays;
}

Array2ListType *sendReceived2ListType(Array2ListType arrayLists)
{
    Array2ListType *sendArrays = (Array2ListType *)erpc_malloc(sizeof(Array2ListType));

    for (uint32_t k = 0; k < 2; ++k)
    {
        for (int32_t i = 0; i < 2; ++i)
        {
            uint32_t elements_count = arrayLists[k][i].elementsCount;
            (*sendArrays)[k][i].elementsCount = elements_count;
            (*sendArrays)[k][i].elements = (int32_t *)erpc_malloc(elements_count * sizeof(int32_t));
            for (uint32_t j = 0; j < elements_count; ++j)
            {
                (*sendArrays)[k][i].elements[j] = arrayLists[k][i].elements[j];
            }
        }
    }
    return sendArrays;
}

AllTypes (*sendReceiveStruct(const AllTypes all_types[2]))[2]
{
    const AllTypes **received_struct = &all_types;
    AllTypes(*send_struct)[2] = (AllTypes(*)[2])erpc_malloc(sizeof(AllTypes[2]));

    for (uint32_t k = 0; k < 2; ++k)
    {
        (*send_struct)[k].number = (*received_struct)[k].number;

        uint32_t lenText = strlen((*received_struct)[k].text);
        (*send_struct)[k].text = (char *)erpc_malloc(lenText + 1);
        strcpy((*send_struct)[k].text, (*received_struct)[k].text);

        (*send_struct)[k].color = (*received_struct)[k].color;

        (*send_struct)[k].c.m = (*received_struct)[k].c.m;
        (*send_struct)[k].c.n = (*received_struct)[k].c.n;

        uint32_t elementsCount = (*received_struct)[k].list_numbers.elementsCount;
        (*send_struct)[k].list_text.elementsCount = (*send_struct)[k].list_numbers.elementsCount = elementsCount;
        (*send_struct)[k].list_numbers.elements = (int32_t *)erpc_malloc(elementsCount * sizeof(int32_t));
        (*send_struct)[k].list_text.elements = (char **)erpc_malloc(elementsCount * sizeof(char *));
        for (uint32_t i = 0; i < elementsCount; ++i)
        {
            (*send_struct)[k].list_numbers.elements[i] = (*received_struct)[k].list_numbers.elements[i];
            lenText = strlen((*received_struct)[k].list_text.elements[i]);
            (*send_struct)[k].list_text.elements[i] = (char *)erpc_malloc(lenText + 1);
            strcpy((*send_struct)[k].list_text.elements[i], (*received_struct)[k].list_text.elements[i]);
        }

        for (uint32_t i = 0; i < 5; ++i)
        {
            (*send_struct)[k].array_numbers[i] = (*received_struct)[k].array_numbers[i];
            lenText = strlen((*received_struct)[k].array_text[i]);
            (*send_struct)[k].array_text[i] = (char *)erpc_malloc(lenText + 1);
            strcpy((*send_struct)[k].array_text[i], (*received_struct)[k].array_text[i]);
        }
    }
    return send_struct;
}

AllTypes (*sendReceive2Struct(AllTypes all_types[1][1]))[1][1]
{
    AllTypes(**received_struct)[1] = &all_types;
    AllTypes(*send_struct)[1][1] = (AllTypes(*)[1][1])erpc_malloc(sizeof(AllTypes[1][1]));

    for (uint32_t k = 0; k < 1; ++k)
    {
        for (uint32_t l = 0; l < 1; ++l)
        {
            (*send_struct)[k][l].number = (*received_struct)[k][l].number;

            uint32_t lenText = strlen((*received_struct)[k][l].text);
            (*send_struct)[k][l].text = (char *)erpc_malloc(lenText + 1);
            strcpy((*send_struct)[k][l].text, (*received_struct)[k][l].text);

            (*send_struct)[k][l].color = (*received_struct)[k][l].color;

            (*send_struct)[k][l].c.m = (*received_struct)[k][l].c.m;
            (*send_struct)[k][l].c.n = (*received_struct)[k][l].c.n;

            uint32_t elementsCount = (*received_struct)[k][l].list_numbers.elementsCount;
            (*send_struct)[k][l].list_text.elementsCount = (*send_struct)[k][l].list_numbers.elementsCount =
                elementsCount;
            (*send_struct)[k][l].list_numbers.elements = (int32_t *)erpc_malloc(elementsCount * sizeof(int32_t));
            (*send_struct)[k][l].list_text.elements = (char **)erpc_malloc(elementsCount * sizeof(char *));
            for (uint32_t i = 0; i < elementsCount; ++i)
            {
                (*send_struct)[k][l].list_numbers.elements[i] = (*received_struct)[k][l].list_numbers.elements[i];
                lenText = strlen((*received_struct)[k][l].list_text.elements[i]);
                (*send_struct)[k][l].list_text.elements[i] = (char *)erpc_malloc(lenText + 1);
                strcpy((*send_struct)[k][l].list_text.elements[i], (*received_struct)[k][l].list_text.elements[i]);
            }

            for (uint32_t i = 0; i < 5; ++i)
            {
                (*send_struct)[k][l].array_numbers[i] = (*received_struct)[k][l].array_numbers[i];
                lenText = strlen((*received_struct)[k][l].array_text[i]);
                (*send_struct)[k][l].array_text[i] = (char *)erpc_malloc(lenText + 1);
                strcpy((*send_struct)[k][l].array_text[i], (*received_struct)[k][l].array_text[i]);
            }
        }
    }
    return send_struct;
}

void test_array_allDirection(const int32_t a[5], const int32_t b[5], int32_t c[5], int32_t d[5])
{
    for (uint32_t i = 0; i < 5; ++i)
    {
        c[i] = i + 1;
        d[i] = i * 8;
    }
}

void testFunction(){};

////////////////////////////////////////////////////////////////////////////////
// Add service to server code
////////////////////////////////////////////////////////////////////////////////

void add_services(erpc::SimpleServer *server)
{
    // define services to add on heap
    // allocate on heap so service doesn't go out of scope at end of method
    svc = new PointersService_service();

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
    delete svc;
}

#ifdef __cplusplus
extern "C" {
#endif
erpc_service_t service_test = NULL;
void add_services_to_server(erpc_server_t server)
{
    service_test = create_PointersService_service();
    erpc_add_service_to_server(server, service_test);
}

void remove_services_from_server(erpc_server_t server)
{
    erpc_remove_service_from_server(server, service_test);
    destroy_PointersService_service(service_test);
}

void remove_common_services_from_server(erpc_server_t server, erpc_service_t service)
{
    erpc_remove_service_from_server(server, service);
    destroy_Common_service(service);
}
#ifdef __cplusplus
}
#endif
