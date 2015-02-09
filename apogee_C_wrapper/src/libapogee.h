/*
 * libapogee.h - header for libapogee C wrapper
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
#pragma once
#ifndef __LIBAPOGEE_H__
#define __LIBAPOGEE_H__

#ifndef _U_
	#define _U_ __attribute__((unused))
#endif

// errors:
typedef enum{
	 ALTA_OK = 0					// all good
	,ALTA_UNDEF						// undefined error
	,ALTA_NO_SUCH_DEVICE			// can't open: no such device
	,ALTA_CANT_CLOSE				// error when closing device
	,ALTA_BAD_FANSPD				// bad fan speed (should be 0..3)
	,ALTA_BAD_GEOMETRY				// bad geometry parameter
	,ALTA_COOLER					// something wrong with cooler
	,ALTA_SHTR						// something wrong with shutter
	,ALTA_BADSPD					// error setting speed
	,ALTA_POWER						// error when working with power modes
	,ALTA_EXPOSURE					// error when exposing image
	,ALTA_GETIMAGE					// error getting image
	,ALTA_NOTWHEEL					// device isn't a filter wheel
	,ALTA_BADWHEELPOS				// bad wheel position
}CCDerr;

extern CCDerr altaerr;

// some definitions from CameraInfo.h ==================>
typedef enum{
	CameraMode_Normal = 0,
	CameraMode_TDI,
	CameraMode_Test,
	CameraMode_ExternalTrigger,
	CameraMode_ExternalShutter,
	CameraMode_Kinetics,
	CameraMode_Unknown
}Apn_CameraMode;
typedef enum{
	Resolution_SixteenBit = 0,
	Resolution_TwelveBit
}Apn_Resolution;
typedef enum{
	Status_ConnectionError = -3,
	Status_DataError = -2,
	Status_PatternError = -1,
	Status_Idle = 0,
	Status_Exposing = 1,
	Status_ImagingActive = 2,
	Status_ImageReady = 3,
	Status_Flushing = 4,
	Status_WaitingOnTrigger = 5
}Apn_Status;
typedef enum{
	UNKNOWN_TYPE = 0,
	FW50_9R   = 1,
	FW50_7S   = 2,
	AFW25_4R  = 3,
	AFW30_7R  = 4,
	AFW50_5R  = 5,
	AFW50_10S = 6,
	AFW31_17R = 9
}Apn_Filter;
typedef enum{
	CoolerStatus_Off = 0,
	CoolerStatus_RampingToSetPoint = 1,
	CoolerStatus_AtSetPoint = 2,
	CoolerStatus_Revision = 3,
	CoolerStatus_Suspended = 4
}Cooler_Status;
typedef enum{
	AdcSpeed_Unknown,
	AdcSpeed_Normal,
	AdcSpeed_Fast,
	AdcSpeed_Video
}Adc_Speed;

// For back-compatible
#define	Apn_CameraMode_Normal CameraMode_Normal
#define	Apn_CameraMode_TDI CameraMode_TDI
#define	Apn_CameraMode_Test CameraMode_Test
#define	Apn_CameraMode_ExternalTrigger CameraMode_ExternalTrigger
#define	Apn_CameraMode_ExternalShutter CameraMode_ExternalShutter
#define	Apn_CameraMode_Kinetics CameraMode_Kinetics
#define	Apn_Resolution_SixteenBit Resolution_SixteenBit
#define	Apn_Resolution_TwelveBit Resolution_TwelveBit
#define Apn_Filter_Unknown   UNKNOWN_TYPE
#define Apn_Filter_FW50_9R   FW50_9R
#define Apn_Filter_FW50_7S   FW50_7S
#define Apn_Filter_AFW25_4R  AFW25_4R
#define Apn_Filter_AFW30_7R  AFW30_7R
#define Apn_Filter_AFW50_5R  AFW50_5R
#define Apn_Filter_AFW50_10S AFW50_10S
#define Apn_Filter_AFW31_17R AFW31_17R
#define Apn_FilterStatus_NotConnected 0
#define Apn_FilterStatus_Ready 1
#define Apn_FilterStatus_Active 2
// <================== some definitions from CameraInfo.h

#ifdef __cplusplus
extern "C" {
#endif

// Camera
int ApnGlueOpen(unsigned int id);
void ApnGlueClose();
void ApnGlueGetName(char **sensor, char **camera);
void ApnGlueSetFan(int speed);
int ApnGlueGetFan();
int ApnGlueSetExpGeom (int roiw, int roih, int osw, int osh, int binw,
		int binh, int roix, int roiy, int *impixw, int *impixh, char whynot[]);
void ApnGlueGetGeom(double *pixX, double *pixY);
void ApnGlueGetMaxValues (double *exptime, int *roiw, int *roih, int *osw,
		int *osh, int *binw, int *binh, int *shutter, double *mintemp);
int ApnGlueGetTemp(double *Cp);
double ApnGlueGetHotTemp();
void ApnGlueSetTemp(double C);
int ApnGlueReadSetPoint(double *temp, int *stat);
int ApnGlueReadShutter();
void ApnGlueOpenShutter(int flag);
unsigned short ApnGlueGetSpeed();
void ApnGlueSetSpeed(unsigned short Sp);
void ApnGlueReset();
int ApnGluePowerDown();
void ApnGluePowerResume();
void ApnGlueDisablePostExpFlushing(int flag);
void ApnGluePreFlash(int flag);
void ApnGlueSetDatabits(Apn_Resolution BitResolution);
void ApnGlueWriteCamMode(Apn_CameraMode CameraMode);
int ApnGlueStartExp (double *exptime, int shutter);
void ApnGluePauseTimer(int flag);
int ApnGlueExpDone();
void ApnGlueExpAbort(void);
int ApnGlueStopExposure();
int ApnGlueReadPixels(unsigned short *buf, int nbuf, char whynot[]);

// Wheel
int ApnGlueWheelOpen(unsigned int id, Apn_Filter type);
void ApnGlueWheelClose();
int ApnGlueWheelGetStatus();
int ApnGlueWheelGetMaxPos();
int ApnGlueWheelSetPos(int pos);
int ApnGlueWheelGetPos();


#ifdef __cplusplus
}
#endif

#endif // __LIBAPOGEE_H__
