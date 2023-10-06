/*
 * Copyright 2017 NXP
 * All rights reserved.
 *
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "rpmsg_linux_endpoint.h"
#include <cassert>
#include <fstream>
#include <fcntl.h> /* For O_RDWR */
#include <iostream>
#include <sstream>
#include <string.h>
#include <unistd.h>

using namespace std;

////////////////////////////////////////////////////////////////////////////////
// Code
////////////////////////////////////////////////////////////////////////////////
RPMsgEndpoint::RPMsgEndpoint(int16_t new_addr = kLocalDefaultAddress, int8_t type = kDatagram, int16_t remote_addr = kRemoteDefaultAddress)
: m_id(1)
, m_type(type)
, m_addr(new_addr)
, m_remote(remote_addr)
, m_currentDST(kCurrentDSTDefaultAddress)
, m_fd(-1)
, m_rpmsg_root("/sys/class/rpmsg")
{
}

int8_t RPMsgEndpoint::init()
{
    fstream fd;
    stringstream path;

    //Get the channel specific default endpoint address
    if (m_addr == kLocalDefaultAddress)
    {
        path.str("");
        path << m_rpmsg_root << "/channel_" << m_id << "/local_addr";
        fd.open(path.str().c_str(), fstream::in);
        if (!fd.is_open())
        {
            perror("failed to get channel default local endpoint: ");
            return -1;
        }

        fd >> m_addr;
        fd.close();
    }

    //Create the new endpoint
    path.str("");
    path << m_rpmsg_root << "/channel_" << m_id << "/ept_new";
    fd.open(path.str().c_str(), fstream::out);
    if (!fd.is_open())
    {
        perror("failed to create new endpoint: ");
        return -2;
    }

    if (m_type == kStream)
    {
        fd << m_addr << ",S";
    }
    else if (m_type == kDatagram)
    {
        fd << m_addr << ",D";
    }
    else
    {
        perror("uknown endpoint type: ");
        return -3;
    }

    fd.close();

    if (m_type == kStream)
    {
        //Get the channel specific default endpoint address
        if (m_remote == kRemoteDefaultAddress)
        {
            path.str("");
            path << m_rpmsg_root << "/channel_" << m_id << "/remote_addr";
            fd.open(path.str().c_str(), fstream::in);
            if (!fd.is_open())
            {
                perror("failed to get channel default remote endpoint: ");
                return -4;
            }

            fd >> m_remote;
            fd.close();
        }

        //src
        path.str("");
        path << m_rpmsg_root << "/channel_" << m_id << "/rpmsg_ept" << m_addr << "." << m_id << "/src";
        fd.open(path.str().c_str(), fstream::out);
        if (!fd.is_open())
        {
            perror("failed to set source target: ");
            return -5;
        }

        fd << m_remote;
        fd.close();

        //dst
        path.str("");
        path << m_rpmsg_root << "/channel_" << m_id << "/rpmsg_ept" << m_addr << "." << m_id << "/dst";
        fd.open(path.str().c_str(), fstream::out);
        if (!fd.is_open())
        {
            perror("failed to set destination target: ");
            return -6;
        }

        fd << m_remote;
        fd.close();
    }
    if ((m_type==kStream) || (m_type==kDatagram))
    {
        path.str("");
        path << "/dev/rpmsg_ept" << m_addr << "." << m_id;
        m_fd = open(path.str().c_str(), O_RDWR);
        if (m_fd == -1)
        {
            perror("failed to open device: ");
            return -7;
        }
    }
    else
    {
        perror("uknown endpoint type: ");
        return -3;
    }
    return 0;
}

RPMsgEndpoint::~RPMsgEndpoint()
{
    fstream fd;
    stringstream path;

    //Close the opened endpoint
    close(m_fd);

    //Deinit the endpoint in the rpmsg internals
    path.str("");
    path << m_rpmsg_root << "/channel_" << m_id << "/ept_delete";
    fd.open(path.str().c_str(), fstream::out);
    if (!fd.is_open())
    {
        perror("destructor Error 1: ");
    }
    else
    {
        fd << m_addr;
    }
}

int32_t RPMsgEndpoint::send(uint8_t *buffer, int16_t dst, uint16_t len)
{
    assert(buffer != NULL);
    fstream fd;
    stringstream path;
    int16_t _dst;

    if (m_type == kStream)
    {
        return write(m_fd, buffer, len);
    }
    else if (m_type == kDatagram)
    {
        if (m_currentDST != dst)
        {
            _dst = dst;
            if (dst == kRemoteDefaultAddress)
            {
                path.str("");
                path << m_rpmsg_root << "/channel_" << m_id << "/remote_addr";
                fd.open(path.str().c_str(), fstream::in);
                if (!fd.is_open())
                {
                    perror("failed to get channel default remote endpoint during send: ");
                    return -1;
                }

                fd >> dst;
                fd.close();
            }

            path.str("");
            path << m_rpmsg_root << "/channel_" << m_id << "/rpmsg_ept" << m_addr << "." << m_id << "/dst";
            fd.open(path.str().c_str(), fstream::out);
            if (!fd.is_open())
            {
                perror("failed to set destination target during send: ");
                return -1;
            }

            fd << dst;
            fd.close();
            m_currentDST = _dst;
        }
        return write(m_fd, buffer, len);
    }
}

int32_t RPMsgEndpoint::receive(uint8_t *buffer, uint32_t maxlen)
{
    assert(buffer != NULL);

    if (maxlen > 0)
    {
        return read(m_fd, buffer, maxlen);
    }
    else
    {
        //nothing to read
        return 0;
    }
}
