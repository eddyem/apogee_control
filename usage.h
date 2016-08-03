/*
 * usage.h
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
#ifndef __USAGE_H__
#define __USAGE_H__

#include "takepic.h"
#include <getopt.h>
#include <stdarg.h>

extern Apn_Filter Tturret;
extern int TurretPos;

enum{
	Reset = 1,
	Sleep,
	Wake
};

unsigned short
	 ROspeed
;

extern int
	 exptime
	,fanspd
	,time_interval
	,pics
	,Ncam
	,hbin
	,vbin
	,X0, X1, Y0, Y1
	,pause_len
	,pre_exp
	,shutter
	,StopRes
	,Nwheel
	,wheelPos
	,Shtr
	,noclean
	,twelveBit
	,flipX
	,flipY
	,histry
;
extern double
	 temperature
	,imscale
;

extern bool
	 only_T
	,noflash
	,set_T
	,cooler_off
	,save_Tlog
	,save_image
	,stat_logging
	,only_turret
	,open_turret
#ifdef IMAGEVIEW
	,show_image
#endif
;
extern char
	 *objname
	,*outfile
	,*objtype
	,*instrument
	,*observers
	,*prog_id
	,*author
	,*subnet
	,*cammsgid
	,*defhdr_filename
;
void usage(char *fmt, ...);
void parse_args(int argc, char **argv);

#endif // __USAGE_H__
