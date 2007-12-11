/*
 * sock.c: network functions.
 *
 * Copyright (C) 2007 Dmitry Davletbaev <ddo_@users.sourceforge.net>
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <assert.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <linux/if_ether.h>

#include "sock.h"
#include "log.h"
#include "dhcp.h"
#include "main.h"
#include "utils.h"


/*
 * Create socket for sending requests. Return socket descriptor or -1 on error.
 */
int create_send_socket()
{
	int sock, sockopt;

	if (-1 == (sock = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL)))) {
		log_err("Error: can not create send socket (%s)",
				strerror(errno));
		return -1;
	}

	sockopt = 1;
	if (0 != setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &sockopt,
				sizeof(sockopt))) {
		log_err("Error: can not set send socket options (%s)",
				strerror(errno));
		return -1;
	}

	return sock;
}

/*
 * Create socket for DHCP server replies. Return socket descriptor or -1 on
 * error.
 */
int create_recv_socket()
{
	int sock = -1;
	int sockopt, flags;

	if (-1 == (sock = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_ALL)))) {
		log_err("Error: can not create recv socket (%s)",
				strerror(errno));
		return -1;
	}

	sockopt = 1;
	if (0 != setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &sockopt,
				sizeof(sockopt))) {
		log_err("Error: can not set recv socket options (%s)",
				strerror(errno));
		close(sock);
		return -1;
	}

	/* FIXME: do we realy need nonblocking socket? */
	if (-1 == (flags = fcntl(sock, F_GETFL))) {
		log_err("can not get socket flags (%s)", strerror(errno));
		close(sock);
		return -1;
	} else {
		if (-1 == fcntl(sock, F_SETFL, flags | O_NONBLOCK)) {
			log_err("can not set socket flags (%s)",
					strerror(errno));
			close(sock);
			return -1;
		}
	}

	return sock;
}

/*
 * Block until DHCP reply is read from socket. `xid' identifies replies we're
 * waiting. Return 0 if successful.
 */
int read_dhcp_from_socket(int sock,
		unsigned int xid,
		struct dhcp_packet* dhcp,
		long timeout)
{
	int ret = -1;
	fd_set rfds;
	struct timeval read_timeout;
	int select_ret;
	int read_bytes = 0;
	unsigned char buffer[8196];
	struct dhcp_packet* recv_dhcp;
	time_t start_time = time(NULL);
	uint32_t server_id;

	assert(sock != -1);
	assert(dhcp);
	assert(timeout > 0);

	FD_ZERO(&rfds);
	while (1) {
		FD_SET(sock, &rfds);
		memset(&read_timeout, 0, sizeof(read_timeout));
		read_timeout.tv_sec = timeout;
		select_ret = select(sock + 1, &rfds, NULL, NULL, &read_timeout);
		if (select_ret == -1) {
			ret = -2;
			goto Out;
		} else if (select_ret == 0) {
			goto Out;
		} else {
			read_bytes = read(sock, buffer, sizeof(buffer));
			if (0 == dhcp_msg(buffer, sizeof(buffer), &recv_dhcp) &&
						recv_dhcp->op == DHCP_OP_BOOTREPLY &&
						recv_dhcp->xid == xid) {
				/* checking if reply must be ignored */
				if (0 != dhcp_get_option(recv_dhcp,
							DHCP_OPT_SERVERID,
							&server_id,
							sizeof(server_id),
							NULL)) {
					log_err("no server ID in reply");
					return -1;
				}
				if (server_id == opts.exclude_server) {
					log_verbose("ignoring server %s",
							get_ip_str(server_id));
					continue;
				}

				memcpy(dhcp, recv_dhcp, sizeof(struct dhcp_packet));
				break;
			} else {
				if (start_time < (time(NULL) - (time_t) timeout)) {
					goto Out;
				}
			}
		}
	}
	ret = 0;

Out:
	if (ret == -2) {
		log_err("socket error while waiting for incoming data (%s)",
				strerror(errno));
	} else if (ret == -1) {
		log_verbose("timeout while waiting for incoming data "
				"(%u seconds)", timeout);
	}
	return ret;
}

