/* sim_sock.c: OS-dependent socket routines

   Copyright (c) 2001-2010, Robert M Supnik

   Permission is hereby granted, free of charge, to any person obtaining a
   copy of this software and associated documentation files (the "Software"),
   to deal in the Software without restriction, including without limitation
   the rights to use, copy, modify, merge, publish, distribute, sublicense,
   and/or sell copies of the Software, and to permit persons to whom the
   Software is furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included in
   all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
   IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
   ROBERT M SUPNIK BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
   IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

   Except as contained in this notice, the name of Robert M Supnik shall not be
   used in advertising or otherwise to promote the sale, use or other dealings
   in this Software without prior written authorization from Robert M Supnik.

   22-Jun-10    RMS     Fixed types in sim_accept_conn (from Mark Pizzolato)
   19-Nov-05    RMS     Added conditional for OpenBSD (from Federico G. Schwindt)
   16-Aug-05    RMS     Fixed spurious SIGPIPE signal error in Unix
   14-Apr-05    RMS     Added WSAEINPROGRESS test (from Tim Riker)
   09-Jan-04    RMS     Fixed typing problem in Alpha Unix (found by Tim Chapman)
   17-Apr-03    RMS     Fixed non-implemented version of sim_close_sock
                        (found by Mark Pizzolato)
   17-Dec-02    RMS     Added sim_connect_socket, sim_create_socket
   08-Oct-02    RMS     Revised for .NET compatibility
   22-Aug-02    RMS     Changed calling sequence for sim_accept_conn
   22-May-02    RMS     Added OS2 EMX support from Holger Veit
   06-Feb-02    RMS     Added VMS support from Robert Alan Byer
   16-Sep-01    RMS     Added Macintosh support from Peter Schorn
   02-Sep-01    RMS     Fixed UNIX bugs found by Mirian Lennox and Tom Markson
*/

#include "sim_defs.h"
#include "sim_sock.h"
#if 0
// XXX pic32
#include <signal.h>
#include <sys/types.h>          /* for fcntl, getpid */
#include <sys/socket.h>         /* for sockets */
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>         /* for sockaddr_in */
#include <netdb.h>
#include <sys/time.h>           /* for EMX */
#endif

/* OS dependent routines

   sim_master_sock      create master socket
   sim_accept_conn      accept connection
   sim_read_sock        read from socket
   sim_write_sock       write from socket
   sim_close_sock       close socket
   sim_setnonblock      set socket non-blocking
   sim_msg_sock         send message to socket
*/

int32 sim_sock_cnt = 0;

static int sim_setnonblock(int sock);

/* First, all the non-implemented versions */

int
sim_err_sock(int s, char *emsg, int32 flg)
{
	int32 err = errno;

	printf("Sockets: %s error %d\n", emsg, err);
	sim_close_sock(s, flg);
	return INVALID_SOCKET;
}

int
sim_create_sock(void)
{
	int newsock;
#if 0
	// XXX pic32
	int32 err;

	signal(SIGPIPE, SIG_IGN);	/* no pipe signals */

	newsock = socket(AF_INET, SOCK_STREAM, 0);	/* create socket */
	if (newsock == INVALID_SOCKET) {	/* socket error? */
		err = errno;
		printf("Sockets: socket error %d\n", err);
		return INVALID_SOCKET;
	}
#endif
	return newsock;
}

int
sim_master_sock(int32 port)
{
	int newsock;
#if 0
	// XXX pic32
	struct sockaddr_in name;
	int32 sta;

	newsock = sim_create_sock();	/* create socket */
	if (newsock == INVALID_SOCKET)	/* socket error? */
		return newsock;

	name.sin_family = AF_INET;	/* name socket */
	name.sin_port = htons((unsigned short)port);	/* insert port */
	name.sin_addr.s_addr = htonl(INADDR_ANY);	/* insert addr */

	sta = bind(newsock, (struct sockaddr *)&name, sizeof(name));
	if (sta == SOCKET_ERROR)	/* bind error? */
		return sim_err_sock(newsock, "bind", 1);
	sta = sim_setnonblock(newsock);	/* set nonblocking */
	if (sta == SOCKET_ERROR)	/* fcntl error? */
		return sim_err_sock(newsock, "fcntl", 1);
	sta = listen(newsock, 1);	/* listen on socket */
	if (sta == SOCKET_ERROR)	/* listen error? */
		return sim_err_sock(newsock, "listen", 1);
#endif
	return newsock;		/* got it! */
}

int
sim_accept_conn(int master, uint32 * ipaddr)
{
	int newsock;
#if 0
	// XXX pic32
	int32 sta, err;
	socklen_t size;
	struct sockaddr_in clientname;

	if (master == 0)	/* not attached? */
		return INVALID_SOCKET;
	size = sizeof(clientname);
	newsock = accept(master, (struct sockaddr *)&clientname, &size);
	if (newsock == INVALID_SOCKET) {	/* error? */
		err = errno;
		if (err != EWOULDBLOCK)
			printf("Sockets: accept error %d\n", err);
		return INVALID_SOCKET;
	}
	if (ipaddr != NULL)
		*ipaddr = ntohl(clientname.sin_addr.s_addr);

	sta = sim_setnonblock(newsock);	/* set nonblocking */
	if (sta == SOCKET_ERROR)	/* fcntl error? */
		return sim_err_sock(newsock, "fcntl", 0);
#endif
	return newsock;
}

int32
sim_read_sock(int sock, char *buf, int32 nbytes)
{
	int32 rbytes, err;

#if 0
	// XXX pic32
	rbytes = recv(sock, buf, nbytes, 0);
#endif
	rbytes = 0;
	if (rbytes == 0)	/* disconnect */
		return -1;
	if (rbytes == SOCKET_ERROR) {
		err = errno;
		if (err == EWOULDBLOCK)	/* no data */
			return 0;
		printf("Sockets: read error %d\n", err);
		return -1;
	}
	return rbytes;
}

int32
sim_write_sock(int sock, char *msg, int32 nbytes)
{
#if 0
	// XXX pic32
	return send(sock, msg, nbytes, 0);
#endif
	return 0;
}

void
sim_close_sock(int sock, t_bool master)
{
#if 0
	// XXX pic32
	close(sock);
#endif
}

int32
sim_setnonblock(int sock)
{
	int32 fl, sta;

#if 0
	// XXX pic32
	fl = fcntl(sock, F_GETFL, 0);	/* get flags */
	if (fl == -1)
		return SOCKET_ERROR;
	sta = fcntl(sock, F_SETFL, fl | O_NONBLOCK);	/* set nonblock */
	if (sta == -1)
		return SOCKET_ERROR;
	sta = fcntl(sock, F_SETOWN, getpid());	/* set ownership */
	if (sta == -1)
		return SOCKET_ERROR;
#endif
	return 0;
}
