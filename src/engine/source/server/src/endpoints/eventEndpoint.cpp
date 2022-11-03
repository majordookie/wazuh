/* Copyright (C) 2015-2022, Wazuh Inc.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include "eventEndpoint.hpp"

#include <cstring>
#include <fcntl.h>
#include <iostream>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>

#include <logging/logging.hpp>

using uvw::ErrorEvent;
using uvw::Loop;

namespace engineserver::endpoints
{

constexpr int BIND_SOCK_ERROR {-1};
constexpr unsigned int MAX_MSG_SIZE {65536 + 512};
constexpr unsigned int TRY_ENQUEUE_WAIT_MS {100};

/**
 * @brief This function opens, binds and configures a unix datagram socket.
 * @todo The code was extracted from the Wazuh source, so it must be adapted when the
 * engine is integrated to the rest of the Wazuh code to avoid code duplicity.
 * @param path (const char *) Contains the absolute path to the unix datagram socket
 * @return (int) Returns either the file descriptor value or -1
 */
static inline int bindUnixDatagramSocket(const char* path)
{
    struct sockaddr_un n_us;

    /* TODO: Check the unlink's parameter before unlinking it (to be sure that it is a
     * socket and not a regular file) */
    unlink(path);

    memset(&n_us, 0, sizeof(n_us));
    n_us.sun_family = AF_UNIX;
    strncpy(n_us.sun_path, path, sizeof(n_us.sun_path) - 1);

    const int socketFd {socket(PF_UNIX, SOCK_DGRAM, 0)};
    if (0 > socketFd)
    {
        return BIND_SOCK_ERROR;
    }

    if (bind(socketFd, (struct sockaddr*)&n_us, SUN_LEN(&n_us)) < 0)
    {
        close(socketFd);
        return BIND_SOCK_ERROR;
    }

    /* Change permissions */
    if (chmod(path, 0660) < 0)
    {
        close(socketFd);
        return BIND_SOCK_ERROR;
    }

    int len;
    socklen_t optlen {sizeof(len)};

    /* Get current maximum size */
    if (-1 == getsockopt(socketFd, SOL_SOCKET, SO_RCVBUF, (void*)&len, &optlen))
    {
        len = 0;
    }

    /* Set maximum message size */
    if (MAX_MSG_SIZE > len)
    {
        len = MAX_MSG_SIZE;
        if (setsockopt(socketFd, SOL_SOCKET, SO_RCVBUF, (const void*)&len, optlen) < 0)
        {
            close(socketFd);
            return BIND_SOCK_ERROR;
        }
    }

    // Set close-on-exec
    if (-1 == fcntl(socketFd, F_SETFD, FD_CLOEXEC))
    {
        WAZUH_LOG_ERROR("Engine event endpoints: The flag `close-on-exec` cannot be set "
                        "on datagram socket ({}): {} ({}).",
                        path,
                        strerror(errno),
                        errno);
    }

    return (socketFd);
}

EventEndpoint::EventEndpoint(
    const std::string& path,
    std::shared_ptr<moodycamel::BlockingConcurrentQueue<std::string>> eventQueue)
    : BaseEndpoint {path}
    , m_loop {Loop::getDefault()}
    , m_handle {m_loop->resource<DatagramSocketHandle>()}
    , m_eventQueue {eventQueue}
{

    m_handle->on<ErrorEvent>(
        [this](const ErrorEvent& event, DatagramSocketHandle& datagramSocketHandle)
        {
            WAZUH_LOG_ERROR("Engine event endpoints: Event error on datagram socket "
                            "({}): code=[{}]; name=[{}]; message=[{}].",
                            m_path,
                            event.code(),
                            event.name(),
                            event.what());
        });

    m_handle->on<DatagramSocketEvent>(
        [this](const DatagramSocketEvent& event, DatagramSocketHandle& handle)
        {
            auto client = handle.loop().resource<DatagramSocketHandle>();

            client->on<ErrorEvent>(
                [this](const ErrorEvent& event, DatagramSocketHandle& client)
                {
                    WAZUH_LOG_ERROR(
                        "Engine event endpoints: Event error on datagram socket "
                        "({}): code=[{}]; name=[{}]; message=[{}].",
                        m_path,
                        event.code(),
                        event.name(),
                        event.what());
                });

            const std::string strRequest {event.data.get(), event.length};
            while (!m_eventQueue->try_enqueue(strRequest))
            {
                std::this_thread::sleep_for(
                    std::chrono::milliseconds(TRY_ENQUEUE_WAIT_MS));
            }
        });

    m_socketFd = bindUnixDatagramSocket(m_path.c_str());

    if (0 >= m_socketFd)
    {
        WAZUH_LOG_ERROR("Engine event endpoints: It was not possible to open the "
                        "datagram socket ({}): {} ({}).",
                        m_path,
                        strerror(errno),
                        errno);
    }
}

void EventEndpoint::configure(void)
{
    if (0 < m_socketFd)
    {
        m_handle->open(m_socketFd);
        m_handle->recv();
    }
    else
    {
        WAZUH_LOG_ERROR("Engine event endpoints: The file descriptor of the datagram "
                        "socket ({}) is invalid ({}).",
                        m_path,
                        m_socketFd);
    }
}

void EventEndpoint::run(void)
{
    m_loop->run<Loop::Mode::DEFAULT>();
}

void EventEndpoint::close(void)
{
    if (m_loop->alive())
    {
        // The loop is stoped
        m_loop->stop();
        // Every handle's closing callback is triggered
        m_loop->walk([](uvw::BaseHandle& handle) { handle.close(); });
        // The loop is run again, so every handle is able to receive its close callback
        m_loop->run();
        m_loop->clear();
        m_loop->close();
        WAZUH_LOG_INFO("Engine event endpoints: All the endpoints were closed.");
    }
    else
    {
        WAZUH_LOG_INFO("Engine event endpoints: Loop is already closed.");
    }
}

EventEndpoint::~EventEndpoint(void)
{
    close();
}

std::shared_ptr<moodycamel::BlockingConcurrentQueue<std::string>>
EventEndpoint::getEventQueue(void) const
{
    return m_eventQueue;
}

} // namespace engineserver::endpoints
