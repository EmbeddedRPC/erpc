/*
 * Copyright (c) 2014-2015, Freescale Semiconductor, Inc.
 * Copyright 2016 - 2022 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "gtest.h"
#include "c_test_client.h"
#include "unit_test_wrapped.h"

#include <string.h>

////////////////////////////////////////////////////////////////////////////////
// Unit test Implementation code
////////////////////////////////////////////////////////////////////////////////

void initInterfaces(erpc_client_t client)
{
    initPointersService_client(client);
}

TEST(test_arrays, sendReceivedInt32)
{
    uint32_t array_count = 12;
    int32_t send_array[12];
    int32_t(*received_array)[12];

    for (uint32_t i = 0; i < array_count; ++i)
    {
        send_array[i] = (int32_t)i;
    }

    received_array = sendReceivedInt32(send_array);

    for (uint32_t i = 0; i < array_count; ++i)
    {
        EXPECT_EQ(send_array[i], (*received_array)[i]);
    }
    erpc_free(received_array);
}

TEST(test_arrays, sendReceived2Int32)
{
    int32_t send_array[12][10];
    int32_t(*received_array)[12][10];

    for (uint32_t i = 0; i < 12; ++i)
    {
        for (uint32_t j = 0; j < 10; ++j)
        {
            send_array[i][j] = (int32_t)(i * j);
        }
    }

    received_array = sendReceived2Int32(send_array);
    for (uint32_t i = 0; i < 12; ++i)
    {
        for (uint32_t j = 0; j < 10; ++j)
        {
            EXPECT_EQ(send_array[i][j], (*received_array)[i][j]);
        }
    }
    erpc_free(received_array);
}

TEST(test_arrays, sendReceivedString)
{
    char *send_array[12];
    char *(*received_array)[12];

    char text[] = "Hello";
    uint32_t lenText = strlen(text);

    for (uint32_t i = 0; i < 12; ++i)
    {
        send_array[i] = (char *)erpc_malloc(lenText + 1);
        strcpy(send_array[i], text);
    }

    received_array = sendReceivedString(send_array);

    for (uint32_t i = 0; i < 12; ++i)
    {
        EXPECT_STREQ(send_array[i], (*received_array)[i]);
        erpc_free((*received_array)[i]);
        erpc_free(send_array[i]);
    }
    erpc_free(received_array);
}

TEST(test_arrays, sendReceived2String)
{
    char *send_array[3][5];
    char *(*received_array)[3][5];

    char text[] = "Hello";
    uint32_t lenText = strlen(text);

    for (uint32_t i = 0; i < 3; ++i)
    {
        for (uint32_t j = 0; j < 5; ++j)
        {
            send_array[i][j] = (char *)erpc_malloc(lenText + 1);
            strcpy(send_array[i][j], text);
        }
    }

    received_array = sendReceived2String(send_array);

    for (uint32_t i = 0; i < 3; ++i)
    {
        for (uint32_t j = 0; j < 5; ++j)
        {
            EXPECT_STREQ(send_array[i][j], (*received_array)[i][j]);
            erpc_free((*received_array)[i][j]);
            erpc_free(send_array[i][j]);
        }
    }
    erpc_free(received_array);
}

TEST(test_arrays, sendReceivedEnum)
{
    uint32_t array_count = 3;
    enumColor send_array[3];
    enumColor(*received_array)[3];

    for (uint32_t i = 0; i < array_count; ++i)
    {
        send_array[i] = (enumColor)i;
    }

    received_array = sendReceivedEnum(send_array);

    for (uint32_t i = 0; i < array_count; ++i)
    {
        EXPECT_EQ(send_array[i], (*received_array)[i]);
    }
    erpc_free(received_array);
}

TEST(test_arrays, sendReceived2Enum)
{
    enumColor send_array[3][3];
    enumColor(*received_array)[3][3];

    for (uint32_t i = 0; i < 3; ++i)
    {
        for (uint32_t j = 0; j < 3; ++j)
        {
            send_array[i][j] = (enumColor)j;
        }
    }

    received_array = sendReceived2Enum(send_array);

    for (uint32_t i = 0; i < 3; ++i)
    {
        for (uint32_t j = 0; j < 3; ++j)
        {
            EXPECT_EQ(send_array[i][j], (*received_array)[i][j]);
        }
    }
    erpc_free(received_array);
}

TEST(test_arrays, sendReceivedList)
{
    uint32_t array_count = 2;
    uint32_t elements_count = 2;
    list_int32_1_t send_array[2];
    list_int32_1_t(*received_array)[2];

    for (uint32_t i = 0; i < array_count; ++i)
    {
        send_array[i].elementsCount = elements_count;
        send_array[i].elements = (int32_t *)erpc_malloc(elements_count * sizeof(int32_t));
        for (uint32_t j = 0; j < elements_count; ++j)
        {
            send_array[i].elements[j] = (int32_t)j;
        }
    }

    received_array = sendReceivedList(send_array);

    for (uint32_t i = 0; i < array_count; ++i)
    {
        int32_t *list_s = send_array[i].elements;
        int32_t *list_r = (*received_array)[i].elements;
        for (uint32_t j = 0; j < elements_count; ++j)
        {
            EXPECT_EQ(*list_r, *list_s);
            list_r++;
            list_s++;
        }
        erpc_free((*received_array)[i].elements);
        erpc_free(send_array[i].elements);
    }
    erpc_free(received_array);
}

TEST(test_arrays, sendReceived2List)
{
    uint32_t elements_count = 2;
    list_int32_1_t send_array[2][2];
    list_int32_1_t(*received_array)[2][2];

    for (uint32_t k = 0; k < 2; ++k)
    {
        for (uint32_t i = 0; i < 2; ++i)
        {
            send_array[k][i].elementsCount = elements_count;
            send_array[k][i].elements = (int32_t *)erpc_malloc(elements_count * sizeof(int32_t));
            for (uint32_t j = 0; j < elements_count; ++j)
            {
                send_array[k][i].elements[j] = (int32_t)j;
            }
        }
    }

    received_array = sendReceived2List(send_array);

    for (uint32_t k = 0; k < 2; ++k)
    {
        for (uint32_t i = 0; i < 2; ++i)
        {
            int32_t *list_s = send_array[k][i].elements;
            int32_t *list_r = (*received_array)[k][i].elements;
            for (uint32_t j = 0; j < elements_count; ++j)
            {
                EXPECT_EQ(*list_r, *list_s);
                list_r++;
                list_s++;
            }
            erpc_free((*received_array)[k][i].elements);
            erpc_free(send_array[k][i].elements);
        }
    }
    erpc_free(received_array);
}

TEST(test_arrays, sendReceivedInt32Type)
{
    uint32_t array_count = 12;
    ArrayIntType send_array;
    ArrayIntType *received_array;

    for (uint32_t i = 0; i < array_count; ++i)
    {
        send_array[i] = (int32_t)i;
    }

    received_array = sendReceivedInt32Type(send_array);

    for (uint32_t i = 0; i < array_count; ++i)
    {
        EXPECT_EQ(send_array[i], (*received_array)[i]);
    }
    erpc_free(received_array);
}

TEST(test_arrays, sendReceived2Int32Type)
{
    Array2IntType send_array;
    Array2IntType *received_array;

    for (uint32_t i = 0; i < 12; ++i)
    {
        for (uint32_t j = 0; j < 10; ++j)
        {
            send_array[i][j] = (int32_t)(i * j);
        }
    }

    received_array = sendReceived2Int32Type(send_array);

    for (uint32_t i = 0; i < 12; ++i)
    {
        for (uint32_t j = 0; j < 10; ++j)
        {
            EXPECT_EQ(send_array[i][j], (*received_array)[i][j]);
        }
    }
    erpc_free(received_array);
}

TEST(test_arrays, sendReceivedStringType)
{
    ArrayStringType send_array;
    ArrayStringType *received_array;

    char text[] = "Hello";
    uint32_t lenText = strlen(text);

    for (uint32_t i = 0; i < 12; ++i)
    {
        send_array[i] = (char *)erpc_malloc(lenText + 1);
        strcpy(send_array[i], text);
    }

    received_array = sendReceivedStringType(send_array);

    for (uint32_t i = 0; i < 12; ++i)
    {
        EXPECT_STREQ(send_array[i], (*received_array)[i]);
        erpc_free((*received_array)[i]);
        erpc_free(send_array[i]);
    }
    erpc_free(received_array);
}

TEST(test_arrays, sendReceived2StringType)
{
    Array2StringType send_array;
    Array2StringType *received_array;

    char text[] = "Hello";
    uint32_t lenText = strlen(text);

    for (uint32_t i = 0; i < 3; ++i)
    {
        for (uint32_t j = 0; j < 5; ++j)
        {
            send_array[i][j] = (char *)erpc_malloc(lenText + 1);
            strcpy(send_array[i][j], text);
        }
    }

    received_array = sendReceived2StringType(send_array);

    for (uint32_t i = 0; i < 3; ++i)
    {
        for (uint32_t j = 0; j < 5; ++j)
        {
            EXPECT_STREQ(send_array[i][j], (*received_array)[i][j]);
            erpc_free((*received_array)[i][j]);
            erpc_free(send_array[i][j]);
        }
    }
    erpc_free(received_array);
}

TEST(test_arrays, sendReceivedEnumType)
{
    uint32_t array_count = 3;
    ArrayEnumType send_array;
    ArrayEnumType *received_array;

    for (uint32_t i = 0; i < array_count; ++i)
    {
        send_array[i] = (enumColor)i;
    }

    received_array = sendReceivedEnumType(send_array);

    for (uint32_t i = 0; i < array_count; ++i)
    {
        EXPECT_EQ(send_array[i], (*received_array)[i]);
    }
    erpc_free(received_array);
}

TEST(test_arrays, sendReceived2EnumType)
{
    Array2EnumType send_array;
    Array2EnumType *received_array;

    for (uint32_t i = 0; i < 3; ++i)
    {
        for (uint32_t j = 0; j < 3; ++j)
        {
            send_array[i][j] = (enumColor)j;
        }
    }

    received_array = sendReceived2EnumType(send_array);

    for (uint32_t i = 0; i < 3; ++i)
    {
        for (uint32_t j = 0; j < 3; ++j)
        {
            EXPECT_EQ(send_array[i][j], (*received_array)[i][j]);
        }
    }
    erpc_free(received_array);
}

TEST(test_arrays, sendReceivedStructType)
{
    uint32_t array_count = 3;
    ArrayStructType send_array;
    ArrayStructType *received_array;

    for (uint32_t i = 0; i < array_count; ++i)
    {
        send_array[i].m = i;
        send_array[i].n = i + 2;
    }

    received_array = sendReceivedStructType(send_array);

    for (uint32_t i = 0; i < array_count; ++i)
    {
        EXPECT_EQ(send_array[i].m, (*received_array)[i].m);
        EXPECT_EQ(send_array[i].n, (*received_array)[i].n);
    }
    erpc_free(received_array);
}

TEST(test_arrays, sendReceived2StructType)
{
    Array2StructType send_array;
    Array2StructType *received_array;

    for (uint32_t i = 0; i < 3; ++i)
    {
        for (uint32_t j = 0; j < 3; ++j)
        {
            send_array[i][j].m = i;
            send_array[i][j].n = j;
        }
    }

    received_array = sendReceived2StructType(send_array);

    for (uint32_t i = 0; i < 3; ++i)
    {
        for (uint32_t j = 0; j < 3; ++j)
        {
            EXPECT_EQ(send_array[i][j].m, (*received_array)[i][j].m);
            EXPECT_EQ(send_array[i][j].n, (*received_array)[i][j].n);
        }
    }
    erpc_free(received_array);
}

TEST(test_arrays, sendReceivedListType)
{
    uint32_t array_count = 2;
    uint32_t elements_count = 2;
    ArrayListType send_array;
    ArrayListType *received_array;

    for (uint32_t i = 0; i < array_count; ++i)
    {
        send_array[i].elementsCount = elements_count;
        send_array[i].elements = (int32_t *)erpc_malloc(elements_count * sizeof(int32_t));
        for (uint32_t j = 0; j < elements_count; ++j)
        {
            send_array[i].elements[j] = (int32_t)j;
        }
    }

    received_array = sendReceivedListType(send_array);

    for (uint32_t i = 0; i < array_count; ++i)
    {
        int32_t *list_s = send_array[i].elements;
        int32_t *list_r = (*received_array)[i].elements;
        for (uint32_t j = 0; j < elements_count; ++j)
        {
            EXPECT_EQ(*list_r, *list_s);
            list_r++;
            list_s++;
        }
        erpc_free(send_array[i].elements);
        erpc_free((*received_array)[i].elements);
    }
    erpc_free(received_array);
}

TEST(test_arrays, sendReceived2ListType)
{
    uint32_t elements_count = 2;
    Array2ListType send_array;
    Array2ListType *received_array;

    for (uint32_t k = 0; k < 2; ++k)
    {
        for (uint32_t i = 0; i < 2; ++i)
        {
            send_array[k][i].elementsCount = elements_count;
            send_array[k][i].elements = (int32_t *)erpc_malloc(elements_count * sizeof(int32_t));
            for (uint32_t j = 0; j < elements_count; ++j)
            {
                send_array[k][i].elements[j] = (int32_t)j;
            }
        }
    }

    received_array = sendReceived2ListType(send_array);

    for (uint32_t k = 0; k < 2; ++k)
    {
        for (uint32_t i = 0; i < 2; ++i)
        {
            int32_t *list_s = send_array[k][i].elements;
            int32_t *list_r = (*received_array)[k][i].elements;
            for (uint32_t j = 0; j < elements_count; ++j)
            {
                EXPECT_EQ(*list_r, *list_s);
                list_r++;
                list_s++;
            }
            erpc_free(send_array[k][i].elements);
            erpc_free((*received_array)[k][i].elements);
        }
    }
    erpc_free(received_array);
}

TEST(test_arrays, sendReceiveStruct)
{
    char text[] = "Hello";
    uint32_t lenText = strlen(text);

    AllTypes send_struct[2];
    AllTypes(*received_struct)[2];

    for (uint32_t k = 0; k < 2; ++k)
    {
        send_struct[k].number = 5;
        send_struct[k].text = (char *)erpc_malloc(lenText + 1);
        strcpy(send_struct[k].text, text);
        send_struct[k].color = red;

        send_struct[k].c.m = 5;
        send_struct[k].c.n = 20;

        send_struct[k].list_text.elementsCount = send_struct[k].list_numbers.elementsCount = 5;
        send_struct[k].list_numbers.elements = (int32_t *)erpc_malloc(5 * sizeof(int32_t));
        send_struct[k].list_text.elements = (char **)erpc_malloc(5 * sizeof(char *));
        for (uint32_t i = 0; i < 5; ++i)
        {
            send_struct[k].list_numbers.elements[i] = i;
            send_struct[k].list_text.elements[i] = (char *)erpc_malloc(lenText + 1);
            strcpy(send_struct[k].list_text.elements[i], text);
        }

        for (uint32_t i = 0; i < 5; ++i)
        {
            send_struct[k].array_numbers[i] = i;
            send_struct[k].array_text[i] = (char *)erpc_malloc(lenText + 1);
            strcpy(send_struct[k].array_text[i], text);
        }
    }

    received_struct = sendReceiveStruct(send_struct);

    for (uint32_t k = 0; k < 2; ++k)
    {
        EXPECT_EQ(send_struct[k].number, (*received_struct)[k].number);
        EXPECT_STREQ(send_struct[k].text, (*received_struct)[k].text);
        erpc_free(send_struct[k].text);
        erpc_free((*received_struct)[k].text);
        EXPECT_EQ(send_struct[k].color, (*received_struct)[k].color);
        EXPECT_EQ(send_struct[k].c.m, (*received_struct)[k].c.m);
        EXPECT_EQ(send_struct[k].c.n, (*received_struct)[k].c.n);

        EXPECT_EQ(send_struct[k].list_numbers.elementsCount, (*received_struct)[k].list_numbers.elementsCount);
        for (uint32_t i = 0; i < 5; ++i)
        {
            EXPECT_EQ(send_struct[k].list_numbers.elements[i], (*received_struct)[k].list_numbers.elements[i]);
            EXPECT_STREQ(send_struct[k].list_text.elements[i], (*received_struct)[k].list_text.elements[i]);
            erpc_free(send_struct[k].list_text.elements[i]);
            erpc_free((*received_struct)[k].list_text.elements[i]);
        }
        erpc_free(send_struct[k].list_numbers.elements);
        erpc_free((*received_struct)[k].list_numbers.elements);
        erpc_free(send_struct[k].list_text.elements);
        erpc_free((*received_struct)[k].list_text.elements);

        for (uint32_t i = 0; i < 5; ++i)
        {
            EXPECT_EQ(send_struct[k].array_numbers[i], (*received_struct)[k].array_numbers[i]);
            EXPECT_STREQ(send_struct[k].array_text[i], (*received_struct)[k].array_text[i]);
            erpc_free(send_struct[k].array_text[i]);
            erpc_free((*received_struct)[k].array_text[i]);
        }
    }
    erpc_free(received_struct);
}

TEST(test_arrays, sendReceive2Struct)
{
    char text[] = "Hello";
    uint32_t lenText = strlen(text);

    AllTypes send_struct[1][1];
    AllTypes(*received_struct)[1][1];

    for (uint32_t k = 0; k < 1; ++k)
    {
        for (uint32_t l = 0; l < 1; ++l)
        {
            send_struct[k][l].number = 5;
            send_struct[k][l].text = (char *)erpc_malloc(lenText + 1);
            strcpy(send_struct[k][l].text, text);
            send_struct[k][l].color = red;

            send_struct[k][l].c.m = 5;
            send_struct[k][l].c.n = 20;

            send_struct[k][l].list_text.elementsCount = send_struct[k][l].list_numbers.elementsCount = 5;
            send_struct[k][l].list_numbers.elements = (int32_t *)erpc_malloc(5 * sizeof(int32_t));
            send_struct[k][l].list_text.elements = (char **)erpc_malloc(5 * sizeof(char *));
            for (uint32_t i = 0; i < 5; ++i)
            {
                send_struct[k][l].list_numbers.elements[i] = i;
                send_struct[k][l].list_text.elements[i] = (char *)erpc_malloc(lenText + 1);
                strcpy(send_struct[k][l].list_text.elements[i], text);
            }

            for (uint32_t i = 0; i < 5; ++i)
            {
                send_struct[k][l].array_numbers[i] = i;
                send_struct[k][l].array_text[i] = (char *)erpc_malloc(lenText + 1);
                strcpy(send_struct[k][l].array_text[i], text);
            }
        }
    }

    received_struct = sendReceive2Struct(send_struct);

    for (uint32_t k = 0; k < 1; ++k)
    {
        for (uint32_t l = 0; l < 1; ++l)
        {
            EXPECT_EQ(send_struct[k][l].number, (*received_struct)[k][l].number);
            EXPECT_STREQ(send_struct[k][l].text, (*received_struct)[k][l].text);
            erpc_free(send_struct[k][l].text);
            erpc_free((*received_struct)[k][l].text);
            EXPECT_EQ(send_struct[k][l].color, (*received_struct)[k][l].color);
            EXPECT_EQ(send_struct[k][l].c.m, (*received_struct)[k][l].c.m);
            EXPECT_EQ(send_struct[k][l].c.n, (*received_struct)[k][l].c.n);

            EXPECT_EQ(send_struct[k][l].list_numbers.elementsCount,
                      (*received_struct)[k][l].list_numbers.elementsCount);
            for (uint32_t i = 0; i < 5; ++i)
            {
                EXPECT_EQ(send_struct[k][l].list_numbers.elements[i],
                          (*received_struct)[k][l].list_numbers.elements[i]);
                EXPECT_STREQ(send_struct[k][l].list_text.elements[i], (*received_struct)[k][l].list_text.elements[i]);
                erpc_free(send_struct[k][l].list_text.elements[i]);
                erpc_free((*received_struct)[k][l].list_text.elements[i]);
            }
            erpc_free(send_struct[k][l].list_numbers.elements);
            erpc_free((*received_struct)[k][l].list_numbers.elements);
            erpc_free(send_struct[k][l].list_text.elements);
            erpc_free((*received_struct)[k][l].list_text.elements);

            for (uint32_t i = 0; i < 5; ++i)
            {
                EXPECT_EQ(send_struct[k][l].array_numbers[i], (*received_struct)[k][l].array_numbers[i]);
                EXPECT_STREQ(send_struct[k][l].array_text[i], (*received_struct)[k][l].array_text[i]);
                erpc_free(send_struct[k][l].array_text[i]);
                erpc_free((*received_struct)[k][l].array_text[i]);
            }
        }
    }
    erpc_free(received_struct);
}

TEST(test_arrays, test_array_allDirection)
{
    int32_t a[5];
    int32_t b[5];
    int32_t c[5];
    int32_t d[5];
    int32_t pA[5];
    int32_t pB[5];
    int32_t pC[5];
    int32_t pD[5];

    for (uint32_t i = 0; i < 5; ++i)
    {
        a[i] = i;
        b[i] = i * 2;
        d[i] = i * 4;
        pA[i] = a[i];
        pB[i] = b[i];
        pC[i] = i + 1;
        pD[i] = 2 * d[i];
    }

    test_array_allDirection(a, b, c, d);

    for (uint32_t i = 0; i < 5; ++i)
    {
        EXPECT_EQ(a[i], pA[i]);
        EXPECT_EQ(b[i], pB[i]);
        EXPECT_EQ(c[i], pC[i]);
        EXPECT_EQ(d[i], pD[i]);
    }
}
