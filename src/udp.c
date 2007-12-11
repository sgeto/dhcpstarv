/*
 * udp.c: UDP headers and functions.
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

#include <stddef.h>
#include <stdint.h>
#include <assert.h>
#include <string.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#include "udp.h"
#include "ip.h"
#include "ether.h"


/*
 * Return UDP checksum (header and payload). `words' contains number of 2-byte
 * words stored in `data'. `srchost' and `dsthost' are source and destination IP
 * address respectively (network byte order). `udplen' is UDP header and data
 * length in bytes without padding byte (host byte order).
 */
unsigned short udp_csum(unsigned short* data,
		int words,
		unsigned int srchost,
		unsigned int dsthost,
		unsigned short udplen)
{
	unsigned int sum, i;

	/* UDP pseudo-header for checksum computation */
	struct udp_pseudo_hdr {
		uint32_t srchost;
		uint32_t dsthost;
		uint8_t zero;
		uint8_t proto;
		uint16_t length;
	} pseudohdr;
	unsigned short* pseudoptr = (unsigned short*) &pseudohdr;

	assert(data);
	assert(words > 0);

	memset(&pseudohdr, 0, sizeof(pseudohdr));
	pseudohdr.srchost = srchost;
	pseudohdr.dsthost = dsthost;
	pseudohdr.proto = IPPROTO_UDP;
	pseudohdr.length = htons(udplen);

	/*
	 * This code is the same as in ip_csum except that UDP pseudoheader is
	 * calculated also.
	 */
	sum = 0;
	for (i = 0; i < sizeof(pseudohdr) / 2; i++)
		sum += pseudoptr[i];
	for (i = 0; i < words; i++)
		sum += data[i];
	sum += sum >> 16;

	return ~sum;
}

/*
 * Initialize UDP packet and store data for sending in `buffer'. Return size of
 * data in `buffer' of -1 on error.
 *
 * Before call `data' contains actual data for sending (UDP payload) and
 * `datalen' its length. `srcmac' and `dstmac' are source and destination
 * hardware addresses (at least ETH_ALEN bytes long). `srchost' and `dsthost'
 * are source and destination IP addresses (network byte order). `srcport' and
 * `dstport' are source and destination ports (network byte order).
 */
size_t init_udp_packet(unsigned char* buffer,
		size_t bufflen,
		const void* data,
		size_t datalen,
		const unsigned char* srcmac,
		unsigned int srchost,
		unsigned short srcport,
		const unsigned char* dstmac,
		unsigned int dsthost,
		unsigned short dstport)
{
	size_t packet_len;
	unsigned short udplen;
	int udppadded = 0;

	/* header and data pointers */
	struct ethhdr* eth = (struct ethhdr*) buffer;
	struct iphdr* ip = (struct iphdr*)(buffer + sizeof(struct ethhdr));
	struct udphdr* udp = (struct udphdr*)(buffer + sizeof(struct ethhdr) +
			sizeof(struct iphdr));
	unsigned char* udpdata = (unsigned char*) udp + sizeof(struct udphdr);

	assert(buffer);
	assert(bufflen > 0);
	assert(data);
	assert(datalen > 0);
	assert(srcmac);
	assert(dstmac);

	/* whole packet length */
	packet_len = sizeof(struct ethhdr) + sizeof(struct iphdr) +
		sizeof(struct udphdr) + datalen;
	/* UDP packet and data length */
	udplen = sizeof(struct udphdr) + datalen;

	/* if data length is odd add padding byte to compute UDP checksum */
	if (datalen % 2 != 0) {
		packet_len ++;
		udplen ++;
		udppadded = 1;
	}

	if (packet_len > bufflen)
		return -1;

	memset(buffer, 0, packet_len);
	memcpy(udpdata, data, datalen);

	udp->source = srcport;
	udp->dest = dstport;
	udp->len = htons(udplen);
	udp->check = udp_csum((unsigned short*) udp,
			udplen / 2,
			srchost,
			dsthost,
			udppadded ? udplen - 1 : udplen);

	init_ip_header(ip, sizeof(struct udphdr) + datalen, IPPROTO_UDP,
			srchost, dsthost);
	init_ether_header(eth, srcmac, dstmac);

	return packet_len;
}

