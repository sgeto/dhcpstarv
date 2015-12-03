/*
 * dhcp.c: create and manipulate DHCP packets.
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
#include <string.h>
#include <netinet/ip.h>
#include <netinet/udp.h>

#include "dhcp.h"
#include "leases.h"
#include "log.h"


/* DHCP magic cookie */
static const uint8_t dhcp_magic[] = { 0x63, 0x82, 0x53, 0x63 };


/*
 * Add option to DHCP packet. Return 0 if successful.
 */
int dhcp_add_option(struct dhcp_packet* msg,
		uint8_t optcode,
		const void* value,
		uint8_t len)
{
	int i;

	assert(msg);
	assert(value);
	assert(len > 0);

	for (i = 4; i < MAX_DHCP_OPTIONS_SIZE; ) {
		if (msg->options[i] == 0 || msg->options[i] == 0xff)
			break;
		i++;
		i += msg->options[i] + 1;
	}

	if (MAX_DHCP_OPTIONS_SIZE <= (i + len + 3))
		return -1;

	msg->options[i++] = optcode;
	msg->options[i++] = len;
	memcpy(msg->options + i, value, len);
	msg->options[i + len] = 0xff;
	
	return 0;
}

/*
 * Return actual DHCP packet size (without padded nulls in options) or -1 on
 * error.
 */
size_t get_dhcp_packet_size(const struct dhcp_packet* msg)
{
	int i;

	assert(msg);

	for (i = 4; i < MAX_DHCP_OPTIONS_SIZE; ) {
		if (msg->options[i] == 0 || msg->options[i] == 0xff)
			break;
		i++;
		i += msg->options[i] + 1;
	}

	if (i >= MAX_DHCP_OPTIONS_SIZE)
		return -1;
	
	return sizeof(struct dhcp_packet) - MAX_DHCP_OPTIONS_SIZE + i + 1;
}

/*
 * Return 0 if buffer contains valid DHCP packet and set `dhcp' to pointer to
 * respective header in buffer (if it isn't  NULL).
 */
int dhcp_msg(void* buffer,
		size_t len,
		struct dhcp_packet** dhcp)
{
	struct iphdr* iph;
	struct udphdr* udph;
	struct dhcp_packet* dhcph;

	assert(buffer);
	assert(len > 0);

	if (len < sizeof(struct iphdr) + sizeof(struct udphdr) +
			sizeof(struct dhcp_packet) - MAX_DHCP_OPTIONS_SIZE + 4)
		return -1;

	iph = (struct iphdr*) buffer;

	if (iph->protocol != IPPROTO_UDP)
		return -1;
	udph = (struct udphdr*) ((uint8_t*) buffer + iph->ihl * 4);

	if (67 != htons(udph->dest) && 68 != htons(udph->dest))
		return -1;
	dhcph = (struct dhcp_packet*)((uint8_t*) udph + sizeof(struct udphdr));

	if (0 != memcmp(dhcph->options, dhcp_magic, sizeof(dhcp_magic)))
		return -1;

	if (dhcp != NULL)
		*dhcp = dhcph;

	return 0;
}

/*
 * Get optcode, value and size for next option in DHCP packet. For first call 0
 * as index should be supplied. For next calls index must be set to returned
 * value.
 *
 * Before call `len' must contain `value' buffer size. After return it contains
 * actual option value size even if buffer was not large enough and data was
 * cut.
 *
 * Return index of next option or -1 if no more options.
 */
int dhcp_next_option(const struct dhcp_packet* msg,
		int index,
		int* code,
		void* value,
		size_t* len)
{
	size_t optlen;

	assert(msg);
	assert(code);
	assert(value);
	assert(len);
	assert(*len > 0);

	/* first 4 bytes are magic cookie */
	if (index < 4)
		index = 4;

	if (msg->options[index] == 0 || msg->options[index] == 0xff)
		return -1;

	*code = msg->options[index++];
	optlen = msg->options[index++];
	memcpy(value, msg->options + index, (optlen < *len) ? optlen : *len);
	*len = optlen;

	index += optlen;
	if (msg->options[index] == 0 || msg->options[index] == 0xff)
		return -1;
	else
		return index;
}

/*
 * Create DHCPDISCOVER. Return DHCP packet size in bytes or -1 on error. If
 * `broadcast' is not 0 broadcast DHCP flag is set.
 */
size_t dhcp_make_discover(struct dhcp_packet* dhcp,
		const struct dhcp_lease* lease,
		int broadcast)
{
	uint8_t type = DHCP_MSGTYPE_DISCOVER;

	assert(dhcp);
	assert(lease);

	memset(dhcp, 0, sizeof(struct dhcp_packet));

	dhcp->op = DHCP_OP_BOOTREQUEST;
	dhcp->htype = DHCP_HTYPE_ETHER;
	dhcp->hlen = DHCP_HLEN_ETHER;
	dhcp->xid = lease->xid;
	if (broadcast != 0)
		dhcp->flags |= DHCP_FLAGS_BROADCAST;
	memcpy(&dhcp->chaddr, lease->mac, sizeof(lease->mac));
	memcpy(dhcp->options, dhcp_magic, sizeof(dhcp_magic));

	if (0 != dhcp_add_option(dhcp, DHCP_OPT_MSGTYPE, &type, sizeof(type)))
		return -1;

	return get_dhcp_packet_size(dhcp);
}

/*
 * Create DHCPREQUEST. Return DHCP packet size or -1 on error. If `broadcast' is
 * not 0 then broadcast flag will be set.
 */
size_t dhcp_make_request(struct dhcp_packet* msg,
		const struct dhcp_lease* lease,
		int broadcast)
{
	uint8_t type = DHCP_MSGTYPE_REQUEST;

	assert(msg);
	assert(lease);

	memset(msg, 0, sizeof(struct dhcp_packet));

	msg->op = DHCP_OP_BOOTREQUEST;
	msg->htype = DHCP_HTYPE_ETHER;
	msg->hlen = DHCP_HLEN_ETHER;
	msg->xid = lease->xid;
	if (broadcast != 0)
		msg->flags |= DHCP_FLAGS_BROADCAST;
	memcpy(&msg->chaddr, lease->mac, sizeof(lease->mac));
	memcpy(msg->options, dhcp_magic, sizeof(dhcp_magic));

	if (0 > dhcp_add_option(msg, DHCP_OPT_MSGTYPE, &type, sizeof(type)))
		return -1;
	if (0 > dhcp_add_option(msg, DHCP_OPT_REQUESTEDIP, &lease->client_addr,
				sizeof(lease->client_addr)))
		return -1;
	if (0 > dhcp_add_option(msg, DHCP_OPT_LEASETIME, &lease->lease_time,
				sizeof(lease->lease_time)))
		return -1;
	if (0 > dhcp_add_option(msg, DHCP_OPT_SERVERID, &lease->server_id,
				sizeof(lease->server_id)))
		return -1;

	return get_dhcp_packet_size(msg);
}

/*
 * Return 0 if option value size conforms to RFC 1533 or -1 if option is
 * unknown.
 */
int dhcp_check_option_size(unsigned optcode, size_t size)
{
	int ret;

	/*
	 * TODO: here we check only options we use including those we receive
	 * from server. This can lead to 'unknown option'. Add all DHCP options
	 * to this code.
	 */
	switch (optcode) {
	case DHCP_OPT_MSGTYPE:
		ret = (size == 1) ? 0 : -1;
		break;
	case DHCP_OPT_DOMAINNAME:
		ret = (size > 0) ? 0 : -1;
		break;
	case DHCP_OPT_SUBNETMASK:
	case DHCP_OPT_BROADCAST:
	case DHCP_OPT_LEASETIME:
	case DHCP_OPT_SERVERID:
	case DHCP_OPT_RENEWALTIME:
	case DHCP_OPT_REBINDINGTIME:
		ret = (size == 4) ? 0 : -1;
		break;
	case DHCP_OPT_ROUTER:
	case DHCP_OPT_DNS:
		if (size == 0 || (size % 4) != 0)
			ret = -1;
		else
			ret = 0;
		break;
	default:
		/* unknown option */
		ret = -1;
	}

	return ret;
}

/*
 * Find option in DHCP packet and get it's value and size. Return 0 if option
 * is found and set `optlen' to actual value size. `optlen' can be NULL.
 */
int dhcp_get_option(const struct dhcp_packet* dhcp,
		uint8_t optcode,
		void* buffer,
		size_t bufsize,
		size_t* optlen)
{
	int i;
	size_t len;

	assert(dhcp);
	assert(buffer);
	assert(bufsize > 0);

	for (i = 4; i < MAX_DHCP_OPTIONS_SIZE; ) {
		if (dhcp->options[i] == 0 || dhcp->options[i] == 0xff)
			return -1;
		if (dhcp->options[i] == optcode)
			break;
		i++;
		i += dhcp->options[i] + 1;
	}

	len = (size_t)(dhcp->options[i + 1]);

	if (MAX_DHCP_OPTIONS_SIZE <= (i + len + 2))
		return -1;

	if (0 != dhcp_check_option_size(optcode, len)) {
		log_err("bad option size: optcode=%u, size=%u", optcode, len);
		return -1;
	}

	memcpy(buffer, &dhcp->options[i + 2], (len > bufsize) ? bufsize : len);
	if (optlen != NULL)
		*optlen = len;

	return 0;
}

/*
 * Create DHCPREQUEST to renew lease. Return DHCP packet size or -1 on error. If
 * `broadcast' is not 0 then broadcast flag will be set.
 */
size_t dhcp_make_renew(struct dhcp_packet* msg,
		const struct dhcp_lease* lease,
		int broadcast)
{
	uint8_t type = DHCP_MSGTYPE_REQUEST;

	assert(msg);
	assert(lease);

	memset(msg, 0, sizeof(struct dhcp_packet));

	msg->op = DHCP_OP_BOOTREQUEST;
	msg->htype = DHCP_HTYPE_ETHER;
	msg->hlen = DHCP_HLEN_ETHER;
	msg->xid = lease->xid;
	if (broadcast != 0)
		msg->flags |= DHCP_FLAGS_BROADCAST;
	msg->ciaddr = lease->client_addr;
	memcpy(&msg->chaddr, lease->mac, sizeof(lease->mac));
	memcpy(msg->options, dhcp_magic, sizeof(dhcp_magic));

	if (0 > dhcp_add_option(msg, DHCP_OPT_MSGTYPE, &type, sizeof(type)))
		return -1;

	return get_dhcp_packet_size(msg);
}

