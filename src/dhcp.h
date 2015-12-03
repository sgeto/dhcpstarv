/*
 * dhcp.h: create and manipulate DHCP packets.
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

#ifndef DHCP_ROUTINES
#define DHCP_ROUTINES

/* Message op code / message type (op). */
#define DHCP_OP_BOOTREQUEST		1
#define DHCP_OP_BOOTREPLY		2

/* Hardware address type (htype). */
#define DHCP_HTYPE_ETHER		1

/* Hardware address length (hlen). */
#define DHCP_HLEN_ETHER			6

/* Broadcast flag (flags field). */
#define DHCP_FLAGS_BROADCAST		0x0800

/* DHCP options maximum size. */
#define MAX_DHCP_OPTIONS_SIZE		312

/* Message types. */
#define DHCP_MSGTYPE_DISCOVER		1
#define DHCP_MSGTYPE_OFFER		2
#define DHCP_MSGTYPE_REQUEST		3
#define DHCP_MSGTYPE_DECLINE		4
#define DHCP_MSGTYPE_ACK		5
#define DHCP_MSGTYPE_NACK		6
#define DHCP_MSGTYPE_RELEASE		7

/* DHCP option codes. */
#define DHCP_OPT_SUBNETMASK		1
#define DHCP_OPT_ROUTER			3
#define DHCP_OPT_DNS			6
#define DHCP_OPT_DOMAINNAME		15
#define DHCP_OPT_BROADCAST		28
#define DHCP_OPT_REQUESTEDIP		50
#define DHCP_OPT_LEASETIME		51
#define DHCP_OPT_MSGTYPE		53
#define DHCP_OPT_SERVERID		54
#define DHCP_OPT_RENEWALTIME		58
#define DHCP_OPT_REBINDINGTIME		59

struct ip_header;
struct udp_header;
struct dhcp_lease;

/* DHCP packet. See RFC 2131. */
struct dhcp_packet {
	uint8_t op;
	uint8_t htype;
	uint8_t hlen;
	uint8_t hops;
	uint32_t xid;
	uint16_t secs;
	uint16_t flags;
	uint32_t ciaddr;
	uint32_t yiaddr;
	uint32_t siaddr;
	uint32_t giaddr;
	uint8_t chaddr[16];
	uint8_t sname[64];
	uint8_t file[128];
	uint8_t options[MAX_DHCP_OPTIONS_SIZE];
};


int dhcp_add_option(struct dhcp_packet* msg, uint8_t optcode,
		const void* value, uint8_t len);
int dhcp_msg(void* buffer,
		size_t len,
		struct dhcp_packet** dhcp);
int dhcp_next_option(const struct dhcp_packet* msg,
		int index,
		int* code,
		void* value,
		size_t* len);
size_t dhcp_make_discover(struct dhcp_packet* dhcp,
		const struct dhcp_lease* lease,
		int broadcast);
size_t dhcp_make_request(struct dhcp_packet* msg,
		const struct dhcp_lease* lease,
		int broadcast);
size_t dhcp_make_renew(struct dhcp_packet* msg,
		const struct dhcp_lease* lease,
		int broadcast);
int dhcp_get_option(const struct dhcp_packet* dhcp,
		uint8_t optcode,
		void* buffer,
		size_t bufsize,
		size_t* optlen);

#endif /* DHCP_ROUTINES */

