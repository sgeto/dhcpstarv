/*
 * ip.c: IP headers and functions.
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

#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <netinet/ip.h>

#include "ip.h"


/*
 * Return checksum of IP header. `words' is number of 2-byte words stored in
 * `data'.
 */
unsigned short ip_csum(unsigned short* data, int words)
{
	unsigned int sum, i;

	assert(data);
	assert(words > 0);

	sum = 0;
	for (i = 0; i < words; i++)
		sum += data[i];
	sum += sum >> 16;

	return ~sum;
}

/*
 * Initialize IP header. `len' is data size (UDP header and payload for UDP
 * packet). `srchost' and `dsthost' are in network byte order.
 *
 * Always return IP header size.
 */
int init_ip_header(struct iphdr* ip,
		size_t len,
		unsigned short proto,
		uint32_t srchost,
		uint32_t dsthost)
{
	assert(ip);

	ip->ihl = sizeof(struct iphdr) / 4;
	ip->version = 4;
	ip->tot_len = htons(sizeof(struct iphdr) + len);
	ip->frag_off |= htons(IP_DF);	/* do not fragment */
	ip->ttl = 64;
	ip->protocol = proto;
	ip->saddr = srchost;
	ip->daddr = dsthost;
	ip->check = 0; /* set to 0 before checksum computation */
	ip->check = ip_csum((unsigned short*) ip, sizeof(struct iphdr) >> 1);

	return sizeof(struct iphdr);
}

