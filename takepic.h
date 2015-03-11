/*
 * takepic.h
 *
 * Copyright 2015 Edward V. Emelianov <eddy@sao.ru, edward.emelianoff@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */
#pragma once
#ifndef __TAKEPIC_H__
#define __TAKEPIC_H__
#define _XOPEN_SOURCE 501
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <err.h>
#include <limits.h>
#include <errno.h>
#include <math.h>
#include <stdbool.h>
#include <fitsio.h>
#include <libintl.h>

#ifdef USEPNG
#include <png.h>
#endif /* USEPNG */

#include <libapogee.h>

#ifndef GETTEXT_PACKAGE
#define GETTEXT_PACKAGE "apogee_control"
#endif
#ifndef LOCALEDIR
#define LOCALEDIR "./locale"
#endif

/*
 * SAO longitude 41 26 29.175
 * SAO latitude 43 39 12.7
 * SAO altitude 2070
 */
#ifndef TELLAT
	#define TELLAT  43.6535278
#endif
#ifndef TELLONG
	#define TELLONG 41.44143375
#endif
#ifndef TELALT
	#define TELALT 2070.0
#endif

/*
#define _(String)				gettext(String)
#define gettext_noop(String)	String
#define N_(String)				gettext_noop(String)
// режим отладки, -DEBUG
#ifdef EBUG
	#define RED			"\033[1;32;41m"
	#define GREEN		"\033[5;30;42m"
	#define OLDCOLOR	"\033[0;0;0m"
	#define FNAME() fprintf(stderr, "\n%s (%s, line %d)\n", __func__, __FILE__, __LINE__)
	#define DBG(...) do{fprintf(stderr, "%s (%s, line %d): ", __func__, __FILE__, __LINE__); \
					fprintf(stderr, __VA_ARGS__);			\
					fprintf(stderr, "\n");} while(0)
	#define ERR(...) DBG(__VA_ARGS__)
#else
	#define FNAME()	 do{}while(0)
	#define DBG(...) do{}while(0)
	#define ERR(...)	do{fprintf(stderr, __VA_ARGS__);			\
						fprintf(stderr, "\n");} while(0)
#endif //EBUG

*/

extern int test_headers;

extern const char *__progname;
#define info(format, args...)	do{		\
	printf("%s: ", __progname);		\
	printf(format,  ## args);		\
	printf("\n");}while(0)

/*
#define warnc(c, format, args...)	\
	warnx(format ": %s", ## args, strerror(c))
long r;
#define TRYFUNC(f, ...)				\
do{	if((r = f(__VA_ARGS__)))		\
		warnc(-r, #f "() failed");	\
}while(0)
*/

#endif // __TAKEPIC_H__
