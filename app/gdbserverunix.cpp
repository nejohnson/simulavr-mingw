/*
 ****************************************************************************
 *
 * simulavr - A simulator for the Atmel AVR family of microcontrollers.
 * Copyright (C) 2001, 2002, 2003  Theodore A. Roth, Klaus Rudolph      
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 ****************************************************************************
 *
 *  $Id$
 */

#include <iostream>

#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <signal.h>


# include <sys/socket.h>
# include <sys/types.h>
# include <netinet/in.h>
# include <netinet/tcp.h>
# include <arpa/inet.h>
# include <vector




#include "avrmalloc.h"
#include "avrerror.h"
#include "systemclock.h"
#include "flash.h"
#include "hweeprom.h"
#include "hwsreg.h"
#include "hwstack.h"

#include "gdb/gdb.h"

//! Interface implementation for server socket wrapper on Win32 systems
struct GdbServerSocket::Impl {
	int sock;       //!< socket for listening for a new client
	int conn;       //!< the TCP connection from gdb client
	struct sockaddr_in address[1];
};

GdbServerSocket::GdbServerSocket(int port) : pImpl (new Impl) {
    pImpl->conn = -1;        //no connection opened
    
    if((pImpl->sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
        avr_error("Can't create socket: %s", strerror(errno));

    /* Let the kernel reuse the socket address. This lets us run
    twice in a row, without waiting for the (ip, port) tuple
    to time out. */
    int i = 1;  
    setsockopt(pImpl->sock, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i));
    fcntl(pImpl->sock, F_SETFL, fcntl(pImpl->sock, F_GETFL, 0) | O_NONBLOCK); //dont know 

    pImpl->address->sin_family = AF_INET;
    pImpl->address->sin_port = htons(port);
    memset(&(pImpl->address->sin_addr), 0, sizeof(pImpl->address->sin_addr));

    if(bind(pImpl->sock, (struct sockaddr *)pImpl->address, sizeof(pImpl->address)))
        avr_error("Can not bind socket: %s", strerror(errno));

    if(listen(pImpl->sock, 1) < 0)
        avr_error("Can not listen on socket: %s", strerror(errno));
}

void GdbServerSocket::Close(void) {
    CloseConnection();
    close(pImpl->sock);
}

int GdbServerSocket::ReadByte(void) {
    char c;
    int res;
    int cnt = MAX_READ_RETRY;

    while(cnt--) {
        res = read(pImpl->conn, &c, 1);
        if(res < 0) {
            if (errno == EAGAIN)
                /* fd was set to non-blocking and no data was available */
                return -1;

            avr_error("read failed: %s", strerror(errno));
        }

        if (res == 0) {
            usleep(1000);
            avr_warning("incomplete read\n");
            continue;
        }
        return c;
    }
    avr_error("Maximum read reties reached");

    return 0; /* make compiler happy */
}

void GdbServerSocket::Write(const void* buf, size_t count) {
    int res;

    res = write(pImpl->conn, buf, count);

    /* FIXME: should we try and catch interrupted system calls here? */
    if(res < 0)
        avr_error("write failed: %s", strerror(errno));

    /* FIXME: if this happens a lot, we could try to resend the
    unsent bytes. */
    if((unsigned int)res != count)
        avr_error("write only wrote %d of %lu bytes", res, count);
}

void GdbServerSocket::SetBlockingMode(int mode) {
    if(mode) {
        /* turn non-blocking mode off */
        if(fcntl(pImpl->conn, F_SETFL, fcntl(pImpl->conn, F_GETFL, 0) & ~O_NONBLOCK) < 0)
            avr_warning("fcntl failed: %s\n", strerror(errno));
    } else {
        /* turn non-blocking mode on */
        if(fcntl(pImpl->conn, F_SETFL, fcntl(pImpl->conn, F_GETFL, 0) | O_NONBLOCK) < 0)
            avr_warning("fcntl failed: %s\n", strerror(errno));
    }
}

bool GdbServerSocket::Connect(void) {
    /* accept() needs this set, or it fails (sometimes) */
    socklen_t addrLength = sizeof(struct sockaddr_in);

    /* We only want to accept a single connection, thus don't need a loop. */
    /* Wait until we have a connection */
    pImpl->conn = accept(pImpl->sock, (struct sockaddr *)pImpl->address, &addrLength);
    if(pImpl->conn > 0) {
        /* Tell TCP not to delay small packets.  This greatly speeds up
        interactive response. WARNING: If TCP_NODELAY is set on, then gdb
        may timeout in mid-packet if the (gdb)packet is not sent within a
        single (tcp)packet, thus all outgoing (gdb)packets _must_ be sent
        with a single call to write. (see Stevens "Unix Network
        Programming", Vol 1, 2nd Ed, page 202 for more info) */
        int i = 1;
        setsockopt (pImpl->conn, IPPROTO_TCP, TCP_NODELAY, &i, sizeof (i));

        /* If we got this far, we now have a client connected and can start 
        processing. */
        fprintf(stderr, "Connection opened by host %s, port %hu.\n",
                inet_ntoa(pImpl->address->sin_addr), ntohs(pImpl->address->sin_port));

        return true;
    } else
        return false;
}

void GdbServerSocket::CloseConnection(void) {
    close(pImpl->conn);
    pImpl->conn = -1;
}
