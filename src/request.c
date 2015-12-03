/*
 * request.c: requests to DHCP server.
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

#include <string.h>
#include <stdint.h>
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netpacket/packet.h>
#include <linux/if_ether.h>

#include "request.h"
#include "main.h"
#include "dhcp.h"
#include "udp.h"
#include "log.h"
#include "sock.h"
#include "leases.h"
#include "utils.h"


static const unsigned char brd_mac[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

/*
 * Initialise sockaddr_ll structure.
 */
void init_ll_addr(struct sockaddr_ll* lladdr)
{
	memset(lladdr, 0, sizeof(struct sockaddr_ll));
	lladdr->sll_family = AF_PACKET;
	lladdr->sll_protocol = ETH_P_IP;
	lladdr->sll_ifindex = ifindex;
	lladdr->sll_hatype = 1;
	lladdr->sll_pkttype = PACKET_BROADCAST;
	lladdr->sll_halen = ETH_ALEN;
	memcpy(lladdr->sll_addr, brd_mac, sizeof(brd_mac));
}

/*
 * Send DHCP packet (broadcast) and receive response for it. Return 0 if
 * successul, > 0 if can not receive response, < 0 if another error occurred.
 */
int send_recv_dhcp(int sock_send,
		int sock_recv,
		struct dhcp_packet* dhcp,
		size_t dhcplen,
		const struct sockaddr_ll* dstaddr,
		const unsigned char* dstmac,
		struct dhcp_lease* lease,
		long timeout)
{
	unsigned char buffer[1024];
	size_t bufflen;
	int sent_bytes;

	assert(sock_send != -1);
	assert(sock_recv != -1);
	assert(dhcp);
	assert(dstaddr);
	assert(lease);

	bufflen = init_udp_packet(buffer,
			sizeof(buffer),
			dhcp,
			dhcplen,
			ifmac,
			0,
			htons(68),
			dstmac ? dstmac : brd_mac,
			0xffffffff,
			htons(67));

	if (bufflen == -1) {
		log_err("can not initialize packet to send");
		return -1;
	}

	sent_bytes = sendto(sock_send, buffer, bufflen, 0,
			(struct sockaddr*) dstaddr,
			sizeof(struct sockaddr_ll));
	if (sent_bytes <= 0) {
		log_err("can not send DHCP packet: %s\n", strerror(errno));
		return -1;
	}

	if (0 == read_dhcp_from_socket(sock_recv, lease->xid, dhcp, timeout))
		return 0;
	else
		return 1;
}


/*
 * Request lease from DHCP server. Return 0 if get DHCPACK, < 0 if uncorrectable
 * error orruced and caller should exit, > 0 otherwise.
 */
int request_lease(int sock_send,
		int sock_recv,
		const unsigned char* mac,
		const unsigned char* dstmac,
		long timeout,
		int retries)
{
	int i;
	struct dhcp_packet dhcp;
	size_t dhcplen;
	struct dhcp_lease* lease;
	unsigned char msgtype;
	char tmp[50];
	struct sockaddr_ll lladdr;
	int ret;

	assert(sock_send != -1);
	assert(sock_recv != -1);
	assert(mac);

	lease = ls_create_lease(mac);

	init_ll_addr(&lladdr);

	/* DHCPDISCOVER */
	for (i = 1; ; i++) {
		if (-1 == (dhcplen = dhcp_make_discover(&dhcp, lease, 1))) {
			log_err("can not create DHCPDISCOVER");
			return -1;
		}

		ret = send_recv_dhcp(sock_send, sock_recv, &dhcp, dhcplen,
				&lladdr, dstmac, lease, timeout);

		if (ret == 0) {
			ls_change_lease(lease, &dhcp);
			break;
		} else if (ret > 0 && i >= retries) {
			log_verbose("did not received DHCP reply for "
					"lease request (DHCPDISCOVER) "
					"in %d retries",
					retries);
			return 1;
		} else if (ret < 0) {
			return -1;
		}
	}
#ifdef VERBOSE_DHCP_DEBUG_OUTPUT
	print_dhcp_contents(&dhcp);
#endif

	/* DHCPREQUEST */
	for (i = 1; ; i++) {
		if (-1 == (dhcplen = dhcp_make_request(&dhcp, lease, 1))) {
			log_err("can not create DHCPREQUEST");
			return -1;
		}

		ret = send_recv_dhcp(sock_send, sock_recv, &dhcp, dhcplen,
				&lladdr, dstmac, lease, timeout);
		if (ret == 0) {
			break;
		} else if (ret > 0 && i >= retries) {
			log_verbose("did not received DHCP reply for "
					"lease request (DHCPREQUEST) "
					"in %d retries",
					retries);
			return 1;
		} else if (ret < 0) {
			return -1;
		}
	}
#ifdef VERBOSE_DHCP_DEBUG_OUTPUT
	print_dhcp_contents(&dhcp);
#endif

	if (0 != dhcp_get_option(&dhcp, DHCP_OPT_MSGTYPE, &msgtype,
				sizeof(msgtype), NULL)) {
		log_err("no message type option in DHCP reply");
		return 1;
	}

	if (msgtype == DHCP_MSGTYPE_ACK) {
		ls_change_lease(lease, &dhcp);
		strncpy(tmp, get_ip_str(lease->client_addr), sizeof(tmp));
		log_info("got address %s for %s from %s",
				tmp,
				mac_to_str(lease->mac),
				get_ip_str(lease->server_id));
		return 0;
	} else if (msgtype == DHCP_MSGTYPE_NACK) {
		log_info("got DHCPNACK reply when requesting address "
				"for %s from %s",
				mac_to_str(lease->mac),
				get_ip_str(lease->server_id));
		return 1;
	} else {
		log_info("got %d reply when requesting address "
				"for %s from %s",
				(int) msgtype,
				mac_to_str(lease->mac),
				get_ip_str(lease->server_id));
		return 1;
	}
}

/*
 * Renew lease. Return 0 if successful. Return 0 if get DHCPACK, < 0 if
 * uncorrectable error orruced and caller should exit, > 0 otherwise.
 */
int renew_lease(int sock_send,
		int sock_recv,
		struct dhcp_lease* lease,
		const unsigned char* dstmac,
		long timeout,
		int retries)
{
	int i;
	struct dhcp_packet dhcp;
	size_t dhcplen;
	unsigned char msgtype;
	struct sockaddr_ll lladdr;
	int ret;

	assert(sock_send != -1);
	assert(sock_recv != -1);
	assert(lease);

	init_ll_addr(&lladdr);

	for (i = 1; ; i++) {
		if (-1 == (dhcplen = dhcp_make_renew(&dhcp, lease, 1))) {
			log_err("can not create DHCPRENEW");
			return -1;
		}

		ret = send_recv_dhcp(sock_send, sock_recv, &dhcp, dhcplen,
					&lladdr, dstmac, lease, timeout);
		if (ret == 0) {
			break;
		} else if (ret > 0 && i >= retries) {
			log_verbose("did not received DHCP reply for "
					"lease renew (DHCPREQUEST) "
					"in %d retries",
					retries);
			return 1;
		} else if (ret < 0) {
			return -1;
		}
	}

#ifdef VERBOSE_DHCP_DEBUG_OUTPUT
	print_dhcp_contents(&dhcp);
#endif

	if (0 != dhcp_get_option(&dhcp, DHCP_OPT_MSGTYPE, &msgtype,
				sizeof(msgtype), NULL)) {
		log_err("no message type option in DHCP reply");
		return -1;
	}

	if (msgtype == DHCP_MSGTYPE_ACK) {
		ls_change_lease(lease, &dhcp);
		log_verbose("renewed lease with address %s for %s",
				get_ip_str(lease->client_addr),
				mac_to_str(lease->mac));
		return 0;
	} else if (msgtype == DHCP_MSGTYPE_NACK) {
		log_verbose("got DHCPNACK reply when renewing lease with "
				"address %s",
				get_ip_str(lease->client_addr));
		return 1;
	} else {
		log_verbose("got %d reply when renewing lease with "
				"address %s",
				(int) msgtype,
				get_ip_str(lease->client_addr));
		return 1;
	}
}

