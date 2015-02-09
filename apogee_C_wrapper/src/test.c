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

#define ERREND()  do{printf("Error!"); if(*errmsg) printf(" %s", errmsg); printf("\n"); goto theend;}while(0)

int main(_U_ int argc, _U_ char **argv){
	char errmsg[256];
	*errmsg = 0;
	int W, H;
	int res = ApnGlueOpen(0);
	if(res == ALTA_NO_SUCH_DEVICE || altaerr != ALTA_OK){
		printf("Can't open! No such device.\n");
		return 1;
	}
	char *sen, *cam;
	ApnGlueGetName(&sen, &cam);
	printf("Camera: %s with sensor %s\n", cam, sen);
	printf("Speed: %d\n", ApnGlueGetSpeed());
	int fanSpeed = ApnGlueGetFan();
	if(ALTA_OK != altaerr) ERREND();
	printf("Current fan speed: %d\n", fanSpeed);
	ApnGlueSetFan(0);
	if(ALTA_OK != altaerr) ERREND();
	double pixH, pixW;
	ApnGlueGetGeom(&pixW, &pixH);
	if(ALTA_OK != altaerr) ERREND();
	printf("Pixel size: %gx%g mkm\n", pixW, pixH);
	double exp[2];
	int roiw, roih, osw, binw, binh;
	ApnGlueGetMaxValues(exp, &roiw, &roih, &osw, NULL, &binw, &binh, NULL, NULL);
	if(ALTA_OK != altaerr) ERREND();
	printf("Extremal expositions (seconds): min = %g, max = %g\n", exp[0], exp[1]);
	printf("ROI maximum size: %dx%d\n", roiw, roih);
	printf("Number of overscan columns: %d\n", osw);
	printf("Maximum binning: %dx%d\n", binw, binh);
	double currtemp;
	int status = ApnGlueGetTemp(&currtemp);
	if(ALTA_OK != altaerr) ERREND();
	char *msg;
	switch(status){
		case CoolerStatus_Off:
			msg = "Off";
		break;
		case CoolerStatus_RampingToSetPoint:
			msg = "Ramping to setpoint";
		break;
		case CoolerStatus_AtSetPoint:
			msg = "At setpoint";
		break;
		case CoolerStatus_Revision:
			msg = "Revision";
		break;
		case CoolerStatus_Suspended:
			msg = "Suspended";
		break;
		default:
			msg = "Undefined";
	}
	printf("Cooler status: %s, CCD temperature: %g, Heatsing temperature: %g\n",
		msg, currtemp, ApnGlueGetHotTemp());
	ApnGlueReadSetPoint(&currtemp, NULL);
	if(ALTA_OK != altaerr) ERREND();
	printf("Temperature setpoint: %g\n", currtemp);
//	ApnGlueSetTemp(30.);
	if(ALTA_OK != altaerr) ERREND();
//	ApnGlueOpenShutter(0);
	printf("Shutter: %s\n", ApnGlueReadShutter() ? "opened" : "closed");

	*errmsg = 0;
	printf("Current speed: %u\n", ApnGlueGetSpeed());
	if(ALTA_OK != ApnGlueSetExpGeom(5000, 5000, 1, 0, 8, 8, 0, 0, &W, &H, errmsg))
		ERREND();
	printf("Image parameters: W = %d, H = %d\n", W, H);

	double exptime = 1.;
	if(ALTA_OK != ApnGlueStartExp(&exptime, 0)) ERREND();
	printf("start exposition for %g s\n", exptime);
	while(!ApnGlueExpDone());
	printf("Exposition done.\n");
	unsigned short *buf = (unsigned short *) calloc(W*H, sizeof(unsigned short));
	if(ALTA_OK != ApnGlueReadPixels(buf, W*H, NULL)) ERREND();
	printf("\nimage value in pixel (W/2, H/2): %u\n", buf[(W+1)*H/2]);
theend:
	ApnGlueClose();
	return 0;
}
