/*
 * main.h: main definitions.
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

#ifndef MAIN_MODULE
#define MAIN_MODULE

/* Program name - used to print help. */
#define PROGNAME		"dhcpstarv"

/* Uncomment to print debug messages (e.g. DHCP packet field values). */
/* #define VERBOSE_DHCP_DEBUG_OUTPUT */


struct app_options {
	unsigned int exclude_server;
	char ifname[50];
	int help;
	int no_promisc;
	int verbose;
	unsigned char* dstmac;
	int debug;
};

extern struct app_options opts;
extern unsigned char ifmac[6];
extern int ifindex;

#endif /* MAIN_MODULE */

