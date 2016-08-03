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
#ifndef _XOPEN_SOURCE
#define _XOPEN_SOURCE 501
#endif
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
 * BTA focal ratio 24.024 m
 */
#ifndef TELLAT
	#define TELLAT  (43.6535278)
#endif
#ifndef TELLONG
	#define TELLONG (41.44143375)
#endif
#ifndef TELALT
	#define TELALT (2070.0)
#endif
#ifndef TELFOCUS
	#define TELFOCUS (24.024)
#endif
// filename for default headers (in ~)
#ifndef DEFCONF
#define DEFCONF "apogee_hdrs.fits"
#endif
extern int test_headers;

extern const char *__progname;
#define info(format, args...)	do{		\
	printf("%s: ", __progname);		\
	printf(format,  ## args);		\
	printf("\n");}while(0)

#endif // __TAKEPIC_H__
