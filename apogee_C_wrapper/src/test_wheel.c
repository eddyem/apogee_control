/*
 * test.c - file for wrapper testing
 *
 * Copyright 2013 Edward V. Emelianoff <eddy@sao.ru>
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
#include <stdio.h>
#include <stdlib.h>
#include "libapogee.h"

#define ERREND()  do{printf("Error!"); if(altaerr != ALTA_OK) printf(" errno: %d", altaerr); printf("\n"); goto theend;}while(0)

int main(_U_ int argc, _U_ char **argv){
	char errmsg[256];
	*errmsg = 0;
	int res = ApnGlueWheelOpen(0, FW50_7S);
	if(res || altaerr != ALTA_OK){
		printf("Can't open! No such device.\n");
		return 1;
	}
	int maxpos, curpos;
	maxpos = ApnGlueWheelGetMaxPos();
	if(maxpos < 1) ERREND();
	curpos = ApnGlueWheelGetPos();
	if(curpos < 0) ERREND();
	printf("Found a wheel, which have %d positions, current position is %d\n", maxpos, curpos);
	if(curpos == maxpos) curpos = 1;
	else curpos++;
	printf("Go to position %d\n", curpos);
	if(ApnGlueWheelSetPos(curpos)) ERREND();
	while(ApnGlueWheelGetStatus()); // wait while moving
	curpos = ApnGlueWheelGetPos();
	if(curpos < 0) ERREND();
	printf("Now wheel is on position %d\n", curpos);
theend:
	ApnGlueWheelClose();
	return 0;
}
