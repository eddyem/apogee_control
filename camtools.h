/*
 * camtools.h
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
#ifndef __CAMTOOLS_H__
#define __CAMTOOLS_H__

#define TRYFITS(f, ...)						\
do{if(!test_headers){ int status = 0;		\
	f(__VA_ARGS__, &status);				\
	if (status){							\
		fits_report_error(stderr, status);	\
		return -1;}							\
}}while(0)
#define WRITEKEY(...)	do{add_fits_header(__VA_ARGS__);}while(0)

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

#ifdef IMAGEVIEW
#include <GL/glut.h>
#include <GL/glext.h>
#include <GL/freeglut.h>
#include <math.h>

// functions for converting grayscale value into colour
typedef enum{
	COLORFN_LINEAR, // linear
	COLORFN_LOG,    // ln
	COLORFN_SQRT    // sqrt
} colorfn_type;

void change_colorfun(colorfn_type f);
void convert_grayimage(unsigned short *src, GLubyte *dst, int w, int h);
#endif // IMAGEVIEW

#endif // __CAMTOOLS_H__

