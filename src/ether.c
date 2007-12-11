/*
 * ether.c: link level.
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

#include <string.h>
#include <errno.h>
#include <assert.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <sys/ioctl.h>
#include <net/if.h>

#include "ether.h"
#include "log.h"


/*
 * Initialize ethernet header structure for sending data via packet socket.
 * Always return ethernet structure size.
 */
int init_ether_header(struct ethhdr* eth,
		const unsigned char* srcmac,
		const unsigned char* dstmac)
{
	assert(eth);
	assert(srcmac);
	assert(dstmac);

	memcpy(&eth->h_source, srcmac, sizeof(eth->h_source));
	memcpy(&eth->h_dest, dstmac, sizeof(eth->h_dest));
	eth->h_proto = htons(ETH_P_IP);

	return sizeof(struct ethhdr);
}

/*
 * Return network interface index which name is stored in `ifname', or -1 on
 * error. `sock' must contain packet socket descriptor.
 */
int get_iface_index(int sock, const char* ifname)
{
	struct ifreq ifr;

	assert(sock != -1);
	assert(ifname);

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	if (-1 == ioctl(sock, SIOCGIFINDEX, &ifr)) {
		log_err("ioctl error: %s", strerror(errno));
		return -1;
	}

	return ifr.ifr_ifindex;
}

/*
 * Get hardware address for network interface which name is stored in `ifname'.
 * Before call `sock' must contain valid packet socket descriptor. On success
 * return 0 and hardware address in `buffer'.
 */
int get_iface_hwaddr(int sock, const char* ifname, void* buffer, size_t len)
{
	struct ifreq ifr;

	assert(sock != -1);
	assert(ifname);
	assert(buffer);
	assert(len >= ETH_ALEN);

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	if (-1 == ioctl(sock, SIOCGIFHWADDR, &ifr)) {
		log_err("ioctl error: %s", strerror(errno));
		return -1;
	}
	memcpy(buffer, ifr.ifr_hwaddr.sa_data,
			(len > ETH_ALEN) ? ETH_ALEN : len);

	return 0;
}

/*
 * If `promisc_on' is not 0 set network interface to promiscuous mode, else set
 * to non-promiscuous mode. Return 1 if interface was set to promiscuous mode, 0
 * if not or -1 if error occured.
 */
int set_promisc_mode(int sock, const char* ifname, int promisc_on)
{
	int ret = 0;
	struct ifreq ifr;

	assert(sock != -1);
	assert(ifname);

	memset(&ifr, 0, sizeof(ifr));
	strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name));
	if (-1 == ioctl(sock, SIOCGIFFLAGS, &ifr)) {
		log_err("can not get flags for %s: %s", ifname,
				strerror(errno));
		return -1;
	}

	/* do not set to promiscuous mode if already in one */
	if ((ifr.ifr_flags & IFF_PROMISC) && promisc_on)
		return 0;

	if (promisc_on)
		ifr.ifr_flags |= IFF_PROMISC;
	else
		ifr.ifr_flags ^= IFF_PROMISC;

	log_verbose("setting %s to %s mode", ifname,
			promisc_on ? "promiscuous" : "non-promiscuous");

	if (-1 == ioctl(sock, SIOCSIFFLAGS, &ifr)) {
		log_err("can not set %s to %s mode: %s", ifname,
			promisc_on ? "promiscuous" : "non-promiscuous",
			strerror(errno));
		return -1;
	}
	ret = 1;

	return ret;
}

