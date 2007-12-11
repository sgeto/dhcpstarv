/*
 * udp.h: UDP headers and functions.
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

#ifndef UDP_PROTO
#define UDP_PROTO

size_t init_udp_packet(unsigned char* buffer,
		size_t bufflen,
		const void* data,
		size_t datalen,
		const unsigned char* srcmac,
		unsigned int srchost,
		unsigned short srcport,
		const unsigned char* dstmac,
		unsigned int dsthost,
		unsigned short dstport);

#endif /* UDP_PROTO */

