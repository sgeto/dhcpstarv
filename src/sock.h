/*
 * sock.h: network functions.
 *
 * Copyright (C) 2007 Dmitry Davletbaev <ddomgn@gmail.com>
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

#ifndef DHCP_SOCKETS
#define DHCP_SOCKETS

struct dhcp_packet;

int create_send_socket();
int create_recv_socket();
int read_dhcp_from_socket(int sock,
		unsigned int xid,
		struct dhcp_packet* dhcp,
		long timeout);

#endif /* DHCP_SOCKETS */

