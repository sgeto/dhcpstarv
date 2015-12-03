/*
 * leases.h: lease storage and manipulation.
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

#ifndef DHCP_LEASES
#define DHCP_LEASES

/* Max. number of DNSs in DHCP option. */
#define MAX_DNS_COUNT			10

/* Max. domain name characters. */
#define MAX_DOMAIN_NAME			128

struct dhcp_packet;

/* Client lease. */
struct dhcp_lease {
	struct dhcp_lease* next;
	uint32_t xid;
	uint16_t secs;			/* no used */
	uint8_t mac[6];			/* client hardware address */
	uint32_t client_addr;		/* client IP address */
	uint32_t server_id;
	uint32_t lease_time;
	uint32_t netmask;
	uint32_t router;
	uint32_t dnss[MAX_DNS_COUNT];	/* DNS servers */
	uint32_t renewal_time;
	uint32_t rebinding_time;
	uint32_t last_updated;		/* last update time (host order) */
	char domain_name[MAX_DOMAIN_NAME];	/* domain name */
};

struct dhcp_lease* ls_get_first_lease();
struct dhcp_lease* ls_create_lease(const void* mac);
void ls_free_all_leases();
void ls_change_lease(struct dhcp_lease* lease, const struct dhcp_packet* dhcp);

#endif /* DHCP_LEASES */

