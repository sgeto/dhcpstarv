/*
 * debug.c: functions used to debug program.
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
#include <stdio.h>
#include <stdint.h>
#include <netinet/in.h>

#include "debug.h"
#include "dhcp.h"
#include "utils.h"


/*
 * Print DHCP option description and value.
 */
void print_opt_description(int code, unsigned char* optvalue, size_t len)
{
	int i;

	switch (code) {
	case DHCP_OPT_SUBNETMASK:
		printf("\t\tSubnet Mask: %s\n",
				get_ip_str(*((uint32_t*) optvalue)));
		break;
	case DHCP_OPT_ROUTER:
		printf("\t\tRouter Option: %s\n",
				get_ip_str(*((uint32_t*) optvalue)));
		break;
	case DHCP_OPT_DNS:
		printf("\t\tDomain Name Server Option:");
		for (i = 0; i < len / 4; i++) {
			printf(" %s", get_ip_str(*((uint32_t*) optvalue + i)));
		}
		printf("\n");
		break;
	case DHCP_OPT_BROADCAST:
		printf("\t\tBroadcast Address Option: %s\n",
				get_ip_str(*((uint32_t*) optvalue)));
		break;
	case DHCP_OPT_REQUESTEDIP:
		printf("\t\tRequested IP Address: %u\n",
				htonl(*((uint32_t*) optvalue)));
		break;
	case DHCP_OPT_LEASETIME:
		printf("\t\tIP Address Lease Time: %u\n",
				htonl(*((uint32_t*) optvalue)));
		break;
	case DHCP_OPT_MSGTYPE:
		printf("\t\tDHCP Message Type: ");
		if (optvalue[0] == DHCP_MSGTYPE_DISCOVER)
			printf("DHCPDISCOVER\n");
		else if (optvalue[0] == DHCP_MSGTYPE_OFFER)
			printf("DHCPOFFER\n");
		else if (optvalue[0] == DHCP_MSGTYPE_REQUEST)
			printf("DHCPREQUEST\n");
		else if (optvalue[0] == DHCP_MSGTYPE_DECLINE)
			printf("DHCPDECLINE\n");
		else if (optvalue[0] == DHCP_MSGTYPE_ACK)
			printf("DHCPACK\n");
		else if (optvalue[0] == DHCP_MSGTYPE_NACK)
			printf("DHCPNAK\n");
		else if (optvalue[0] == DHCP_MSGTYPE_RELEASE)
			printf("DHCPRELEASE\n");
		else
			printf("unknown (0x%02u)\n", optvalue[0]);
		break;
	case DHCP_OPT_SERVERID:
		printf("\t\tServer Identifier: %s\n",
				get_ip_str(*((uint32_t*) optvalue)));
		break;
	case DHCP_OPT_RENEWALTIME:
		printf("\t\tRenewal (T1) Time Value: %u\n",
				htonl(*((uint32_t*) optvalue)));
		break;
	case DHCP_OPT_REBINDINGTIME:
		printf("\t\tRebinding (T2) Time Value: %u\n",
				htonl(*((uint32_t*) optvalue)));
		break;
	default:
		printf("\t\tcode: %u, size: %u, value:", code, len);
		for (i = 0; i < len; i++)
			printf(" 0x%02x", optvalue[i]);
		printf("\n");
		break;
	}
}

/*
 * Print DHCP packet contents.
 */
void print_dhcp_contents(const struct dhcp_packet* dhcp)
{
	int optind = 0;
	unsigned char optvalue[255];
	int code;
	size_t len;
	int i;

	printf("\tDHCP op: %u\n", dhcp->op);
	printf("\tDHCP hops: %u\n", dhcp->hops);
	printf("\tDHCP xid: %u\n", dhcp->xid);
	printf("\tDHCP flags: 0x%x\n", dhcp->flags);
	printf("\tDHCP ciaddr: %s\n", get_ip_str(dhcp->ciaddr));
	printf("\tDHCP yiaddr: %s\n", get_ip_str(dhcp->yiaddr));
	printf("\tDHCP siaddr: %s\n", get_ip_str(dhcp->siaddr));
	printf("\tDHCP giaddr: %s\n", get_ip_str(dhcp->giaddr));

	printf("\tDHCP client MAC: ");
	for (i = 0; i < 6; i++)
		printf(" %02x", dhcp->chaddr[i]);
	printf("\n");

	printf("\tOptions:\n");
	len = sizeof(optvalue);
	while (0 <= (optind = dhcp_next_option(dhcp, optind, &code,
					optvalue, &len))) {
		print_opt_description(code, optvalue, len);
		len = sizeof(optvalue);
	}
}

