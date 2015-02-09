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
;
extern double temperature;

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
;
extern char
	 *objname
	,*outfile
	,*objtype
	,*instrument
	,*observers
	,*prog_id
	,*author
;
void usage(char *fmt, ...);
void parse_args(int argc, char **argv);

#endif // __USAGE_H__
