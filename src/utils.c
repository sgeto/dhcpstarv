/*
 * utils.c: utility functions.
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

#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "utils.h"
#include "dhcp.h"


static char tmp_mac[50];


/*
 * Initialise sockaddr_in structure with IP specified in `ip' and port specified
 * in `port'. This function is not for strings supplied by user so no error
 * checking performed.
 */
void init_addr(struct sockaddr_in* addr, const char* ip, unsigned short port)
{
	assert(addr);

	memset(addr, 0, sizeof(struct sockaddr_in));
	addr->sin_family = AF_INET;
	addr->sin_port = htons(port);
	if (ip != NULL)
		inet_aton(ip, &addr->sin_addr);
}

/*
 * Convert IP specified as string to integer (in network order). This function
 * is not for strings supplied by user so no error checking performed. Return IP
 * address in network byte order or 0 on error.
 */
unsigned int strip_to_int(const char* ip)
{
	struct sockaddr_in addr;

	assert(ip);

	if (0 == inet_aton(ip, &addr.sin_addr))
		return 0;
	else
		return addr.sin_addr.s_addr;
}

/*
 * Return hardware address as string.
 */
const char* mac_to_str(const unsigned char* mac)
{
	int i;
	char tmp[50];

	assert(mac);

	memset(tmp, 0, sizeof(tmp));
	memset(tmp_mac, 0, sizeof(tmp_mac));
	for (i = 0; i < DHCP_HLEN_ETHER; i++) {
		sprintf(tmp, (i == 0) ? "%02x" : ":%02x", mac[i]);
		strcat(tmp_mac, tmp);
	}

	return tmp_mac;
}

/*
 * Convert string with MAC (for example `00:14:78:04:de:e0' to byte array.
 * Return 0 if successful or -1 if bad string format or not enough room in
 * `mac'.
 */
int str_to_mac(const char* str, unsigned char* mac, size_t macsize)
{
	unsigned char tmpmac[6];
	int enough = (macsize >= sizeof(tmpmac));
	int converted;

	assert(str);
	assert(mac);

	converted = sscanf(str, "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
			&tmpmac[0],
			&tmpmac[1],
			&tmpmac[2],
			&tmpmac[3],
			&tmpmac[4],
			&tmpmac[5]);
	if (converted != sizeof(tmpmac))
		return -1;

	memset(mac, 0, macsize);
	memcpy(mac, tmpmac, enough ? sizeof(tmpmac) : macsize);

	return !enough;
}

/*
 * Return IP address as string.
 */
const char* get_ip_str(unsigned int ip)
{
	struct sockaddr_in addr;

	addr.sin_addr.s_addr = ip;
	return inet_ntoa(addr.sin_addr);
}

