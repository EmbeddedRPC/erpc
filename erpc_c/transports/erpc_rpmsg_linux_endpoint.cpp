/*
 * Copyright 2017 NXP
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
 * 
 * This file was copied and renamed from https://github.com/EmbeddedRPC/erpc-imx-demos/tree/master/middleware/rpmsg-cpp
 * Adding the prefix "erpc_" to the filename
 */

#include "erpc_rpmsg_linux_endpoint.h"
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
    // should never get here
    return -1;
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
