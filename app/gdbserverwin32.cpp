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

#include <memory>

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

#include <winsock2.h>
#include <WS2tcpip.h>

#include "gdb/gdb.h"

//! Interface implementation for server socket wrapper on Win32 systems
struct GdbServerSocket::Impl {
	SOCKET sock;       //!< socket for listening for a new client
	SOCKET conn;       //!< the TCP connection from gdb client
	sockaddr_in address;
};

//////////////////////////////////////////////////////////////////////////////

GdbServerSocket::GdbServerSocket(int port) : pImpl (new Impl) {
	pImpl->conn = -1;        //no connection opened

	pImpl->sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (pImpl->sock == INVALID_SOCKET) {
		WSACleanup();		
		avr_error("socket failed with error: %ld", WSAGetLastError());
	}

	/* Let the kernel reuse the socket address. This lets us run
	twice in a row, without waiting for the (ip, port) tuple
	to time out. */
	int i = 1;
	setsockopt(pImpl->sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&i, sizeof(i));
	u_long iMode = 1;
	ioctlsocket(pImpl->sock, FIONBIO, &iMode); // non-blocking mode

	pImpl->address.sin_family = AF_INET;
	pImpl->address.sin_port = htons(port);
	inet_pton(AF_INET, "127.0.0.1", &(pImpl->address.sin_addr));

	if(bind(pImpl->sock, (SOCKADDR *)&(pImpl->address), sizeof(pImpl->address))==SOCKET_ERROR) {
		int iError = WSAGetLastError();
		closesocket(pImpl->sock);
		WSACleanup();
		avr_error("Can not bind socket: %ld", iError);
	}

	if(listen(pImpl->sock, 1) == SOCKET_ERROR) {
		int iError = WSAGetLastError();
		closesocket(pImpl->sock);
		WSACleanup();
		avr_error("Can not listen on socket: %ld", iError);
	}
}

//////////////////////////////////////////////////////////////////////////////

GdbServerSocket::~GdbServerSocket() {}

//////////////////////////////////////////////////////////////////////////////

void GdbServerSocket::SetBlockingMode(int mode) {
	if(mode) {
		/* turn non-blocking mode off */
		u_long iMode = 0;
		int iResult = ioctlsocket(pImpl->sock, FIONBIO, &iMode); // non-blocking mode
		if(iResult != NO_ERROR)
			avr_warning("fcntl failed: %ld\n", WSAGetLastError());
	} else {
		/* turn non-blocking mode on */
		u_long iMode = 1;
		int iResult = ioctlsocket(pImpl->sock, FIONBIO, &iMode); // non-blocking mode
		if(iResult != NO_ERROR)
			avr_warning("fcntl failed: %ld\n", WSAGetLastError());
	}
}

//////////////////////////////////////////////////////////////////////////////

bool GdbServerSocket::Connect(void) {
	/* accept() needs this set, or it fails (sometimes) */
	int addrLength = sizeof(struct sockaddr_in);

	/* We only want to accept a single connection, thus don't need a loop. */
	/* Wait until we have a connection */
	pImpl->conn = accept(pImpl->sock, (struct sockaddr *)&(pImpl->address), &addrLength);
	if(pImpl->conn != INVALID_SOCKET) {
		/* Tell TCP not to delay small packets.  This greatly speeds up
		interactive response. WARNING: If TCP_NODELAY is set on, then gdb
		may timeout in mid-packet if the (gdb)packet is not sent within a
		single (tcp)packet, thus all outgoing (gdb)packets _must_ be sent
		with a single call to write. (see Stevens "Unix Network
		Programming", Vol 1, 2nd Ed, page 202 for more info) */
		int i = 1;
		setsockopt (pImpl->conn, IPPROTO_TCP, TCP_NODELAY, (const char *)&i, sizeof (i));

		/* If we got this far, we now have a client connected and can start 
		processing. */
		fprintf(stderr, "Connection opened by host %s, port %hu.\n",
				inet_ntoa(pImpl->address.sin_addr), ntohs(pImpl->address.sin_port));

		return true;
	} else
		return false;
}

//////////////////////////////////////////////////////////////////////////////

int GdbServerSocket::ReadByte(void) {
	char c;
	int res;

	while(1) {
		res = recv(pImpl->conn, &c, 1, 0);
		if(res < 0) {
			if (errno == EAGAIN)
				/* fd was set to non-blocking and no data was available */
				return -1;

			int iError = WSAGetLastError();
			if (iError == WSAEWOULDBLOCK)
			{
				usleep(1000);
				continue;
			}
			avr_warning("read failed: %d", iError);
		}

		if (res == 0) {
			usleep(1000);
			avr_warning("incomplete read\n");
			continue;
		}
		return c;
	}
	avr_error("Maximum read retries reached");

	return 0; /* make compiler happy */
}

//////////////////////////////////////////////////////////////////////////////

void GdbServerSocket::Write(const void* buf, size_t count) {
	int res;

	res = send(pImpl->conn, (const char*)buf, count, 0);

	/* FIXME: should we try and catch interrupted system calls here? */
	if(res < 0)
		avr_error("write failed: %s", strerror(errno));

	/* FIXME: if this happens a lot, we could try to resend the
	unsent bytes. */
	if((unsigned int)res != count)
		avr_error("write only wrote %d of %lu bytes", res, count);
}

//////////////////////////////////////////////////////////////////////////////

void GdbServerSocket::Close(void) {
	CloseConnection();
	closesocket(pImpl->sock);
}

//////////////////////////////////////////////////////////////////////////////

void GdbServerSocket::CloseConnection(void) {
	closesocket(pImpl->conn);
	pImpl->conn = -1;
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
