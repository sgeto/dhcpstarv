/*
 * request.h: requests to DHCP server.
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

#ifndef DHCP_REQUEST_ROUTINE
#define DHCP_REQUEST_ROUTINE

struct dhcp_lease;

int request_lease(int sock_send,
		int sock_recv,
		const unsigned char* mac,
		const unsigned char* dstmac,
		long timeout,
		int retries);
int renew_lease(int sock_send,
		int sock_recv,
		struct dhcp_lease* lease,
		const unsigned char* dstmac,
		long timeout,
		int retries);

#endif /* DHCP_REQUEST_ROUTINE */

