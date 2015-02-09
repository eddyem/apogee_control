#pragma once
#ifndef __CAMTOOLS_H__
#define __CAMTOOLS_H__

#define TRYFITS(f, ...)						\
do{if(!test_headers){ int status = 0;		\
	f(__VA_ARGS__, &status);				\
	if (status){							\
		fits_report_error(stderr, status);	\
		return -1;}							\
}}while(0)
#define WRITEKEY(...)							\
do{ if(test_headers){							\
	print_fits_header(__VA_ARGS__);				\
	}else{ int status = 0;						\
	fits_write_key(__VA_ARGS__, &status);		\
	if(status) fits_report_error(stderr, status);\
}}while(0)

void print_fits_header(fitsfile *fptr, int datatype, char *keyname,
						void *value, char *comment);

extern char *camera, *sensor, viewfield[];
extern double pixX, pixY, t_ext, t_int;
extern unsigned short max, min;
extern double avr, std;
extern time_t expStartsAt;


void AutoadjustFanSpeed(bool onoff);
double printCoolerStat(double *t_ext);

void print_stat(unsigned short *img, long size, FILE* f);


int writefits(char *filename, int width, int height, void *data);
#ifdef USEPNG
int writepng(char *filename, int width, int height, void *data);
#endif /* USEPNG */
#ifdef USERAW
int writeraw(char *filename, int width, int height, void *data);
#endif // USERAW

#endif // __CAMTOOLS_H__

