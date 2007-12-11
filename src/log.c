/*
 * log.c: logging.
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
#include <stdarg.h>
#include <time.h>
#include <string.h>

#include "log.h"
#include "main.h"


/* Date and time format. */
static const char timefmt[] = "%T %D: ";

/*
 * Log information message.
 */
void log_info(const char* format, ...)
{
	va_list ap;
	char time_format[1024];
	time_t now = time(NULL);

	strftime(time_format, sizeof(time_format), timefmt, localtime(&now));
	strncat(time_format, format, sizeof(time_format));

	va_start(ap, format);
	vprintf(time_format, ap);
	printf("\n");
	va_end(ap);
}

/*
 * Log error message.
 */
void log_err(const char* format, ...)
{
	va_list ap;
	char time_format[1024];
	time_t now = time(NULL);

	strftime(time_format, sizeof(time_format), timefmt, localtime(&now));
	strncat(time_format, format, sizeof(time_format));

	va_start(ap, format);
	vfprintf(stderr, time_format, ap);
	fprintf(stderr, "\n");
	va_end(ap);
}

/*
 * Log verbose information message.
 */
void log_verbose(const char* format, ...)
{
	va_list ap;
	char time_format[1024];
	time_t now = time(NULL);

	if (opts.verbose) {
		strftime(time_format, sizeof(time_format), timefmt,
				localtime(&now));
		strncat(time_format, format, sizeof(time_format));

		va_start(ap, format);
		vprintf(time_format, ap);
		printf("\n");
		va_end(ap);
	}
}

