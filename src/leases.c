/*
 * leases.c: lease storage and manipulation.
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
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <time.h>

#include "leases.h"
#include "dhcp.h"
#include "log.h"


/* Client leases. */
static struct dhcp_lease* lease_list = NULL;


/*
 * Return address of first lease.
 */
struct dhcp_lease* ls_get_first_lease()
{
	return lease_list;
}

/*
 * Return address of last lease.
 */
struct dhcp_lease* ls_get_last_lease()
{
	struct dhcp_lease* last;

	if (lease_list == NULL)
		return NULL;

	last = lease_list;
	while (NULL != last->next)
		last = last->next;

	return last;
}

/*
 * Create new lease and append it to list. The `xid' field of new lease will be
 * random generated.
 */
struct dhcp_lease* ls_create_lease(const void* mac)
{
	struct dhcp_lease* lease;
	struct dhcp_lease* last;

	assert(mac);

	lease = (struct dhcp_lease*) malloc(sizeof(struct dhcp_lease));
	memset(lease, 0, sizeof(struct dhcp_lease));
	lease->xid = rand();
	memcpy(lease->mac, mac, sizeof(lease->mac));

	last = ls_get_last_lease();
	if (last == NULL)
		lease_list = lease;
	else
		last->next = lease;

	return lease;
}

/*
 * Delete all leases and free memory.
 */
void ls_free_all_leases()
{
	struct dhcp_lease* lease = lease_list;
	struct dhcp_lease* tmp;

	while (lease != NULL) {
		tmp = lease;
		lease = lease->next;
		free(tmp);
	}
	lease_list = NULL;
}

/*
 * Copy values from DHCP packet to lease.
 */
void ls_change_lease(struct dhcp_lease* lease, const struct dhcp_packet* dhcp)
{
	unsigned char msgtype;

	assert(lease);
	assert(dhcp);

	if (0 != dhcp_get_option(dhcp, DHCP_OPT_MSGTYPE, &msgtype,
				sizeof(msgtype), NULL)) {
		log_err("no DHCP message type in reply");
		return;
	}

	switch (msgtype) {
	case DHCP_MSGTYPE_DISCOVER:
		break;
	case DHCP_MSGTYPE_OFFER:
		/* mandatory */
		lease->client_addr = dhcp->yiaddr;
		if (0 != dhcp_get_option(dhcp, DHCP_OPT_SERVERID,
					&lease->server_id,
					sizeof(lease->server_id), NULL)) {
			log_err("no server id option in DHCPOFFER");
			return;
		}
		if (0 != dhcp_get_option(dhcp, DHCP_OPT_LEASETIME,
					&lease->lease_time,
					sizeof(lease->lease_time), NULL)) {
			log_err("no IP address lease option time in DHCPOFFER");
			return;
		}
		if (0 != dhcp_get_option(dhcp, DHCP_OPT_SUBNETMASK,
					&lease->netmask, sizeof(lease->netmask),
					NULL)) {
			log_err("no network mask option in DHCPOFFER");
			return;
		}
		if (0 != dhcp_get_option(dhcp, DHCP_OPT_RENEWALTIME,
					&lease->renewal_time,
					sizeof(lease->renewal_time), NULL)) {
			log_err("no renewal time option in DHCPOFFER");
			return;
		}
		if (0 != dhcp_get_option(dhcp, DHCP_OPT_REBINDINGTIME,
					&lease->rebinding_time,
					sizeof(lease->rebinding_time), NULL)) {
			log_err("no rebinding time option in DHCPOFFER");
			return;
		}

		/* optional */
		dhcp_get_option(dhcp, DHCP_OPT_DOMAINNAME, &lease->domain_name,
				sizeof(lease->domain_name), NULL);
		dhcp_get_option(dhcp, DHCP_OPT_ROUTER, &lease->router,
				sizeof(lease->router), NULL);
		dhcp_get_option(dhcp, DHCP_OPT_DNS, lease->dnss,
				sizeof(lease->dnss), NULL);
		break;
	case DHCP_MSGTYPE_REQUEST:
		break;
	case DHCP_MSGTYPE_DECLINE:
		break;
	case DHCP_MSGTYPE_ACK:
		/* mandatory */
		if (0 != dhcp_get_option(dhcp, DHCP_OPT_SERVERID,
					&lease->server_id,
					sizeof(lease->server_id), NULL)) {
			log_err("no server id option in DHCPOFFER");
			return;
		}
		/* optional */
		dhcp_get_option(dhcp, DHCP_OPT_LEASETIME, &lease->lease_time,
				sizeof(lease->lease_time), NULL);
		dhcp_get_option(dhcp, DHCP_OPT_RENEWALTIME,
				&lease->renewal_time,
				sizeof(lease->renewal_time), NULL);
		dhcp_get_option(dhcp, DHCP_OPT_REBINDINGTIME,
				&lease->rebinding_time,
				sizeof(lease->rebinding_time), NULL);
		dhcp_get_option(dhcp, DHCP_OPT_SUBNETMASK, &lease->netmask,
				sizeof(lease->netmask), NULL);
		dhcp_get_option(dhcp, DHCP_OPT_ROUTER, &lease->router,
				sizeof(lease->router), NULL);
		dhcp_get_option(dhcp, DHCP_OPT_DNS, lease->dnss,
				sizeof(lease->dnss), NULL);
		dhcp_get_option(dhcp, DHCP_OPT_DOMAINNAME, &lease->domain_name,
				sizeof(lease->domain_name), NULL);

		lease->last_updated = time(NULL);
		break;
	case DHCP_MSGTYPE_NACK:
		/* TODO: fill */
		break;
	case DHCP_MSGTYPE_RELEASE:
		break;
	default:
		log_err("unknown message type %u", (unsigned) msgtype);
	}
}

