/*
 * ether.h: link level.
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

#ifndef ETHER_ROUTINES
#define ETHER_ROUTINES

int init_ether_header(struct ethhdr* eth,
		const unsigned char* srcmac,
		const unsigned char* dstmac);
int get_iface_index(int sock, const char* ifname);
int get_iface_hwaddr(int sock, const char* ifname, void* buffer, size_t len);
int set_promisc_mode(int sock, const char* ifname, int promisc_on);

#endif /* ETHER_ROUTINES */

