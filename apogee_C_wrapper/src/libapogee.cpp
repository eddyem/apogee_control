/*
 * libapogee.cpp - libapogee C wrapper
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

#include <iostream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include "libapogee.h"

#include <Alta.h>
#include <AltaF.h>
#include <Ascent.h>
#include <Aspen.h>
#include <Quad.h>
#include <CameraInfo.h>
#include <FindDeviceUsb.h>
#include <FindDeviceEthernet.h>
#include <ApogeeFilterWheel.h>
#include <ApogeeCam.h>
/*
#if defined APOGEE_ASCENT
	#define CCD Ascent
#elif defined APOGEE_ALTA
	#define CCD Alta
#elif defined APOGEE_ALTAF
	#define CCD AltaF
#elif defined APOGEE_ASPEN
	#define CCD Aspen
#elif defined APOGEE_QUAD
	#define CCD Quad
#else
	#error "You must define camera type: APOGEE_ASCENT, APOGEE_ALTA, \
APOGEE_ALTAF, APOGEE_ASPEN or APOGEE_QUAD"
#endif
*/

// static class for CCD device
//static CCD *alta = NULL;
static ApogeeCam *alta = NULL;
static bool isethernet = false;
int ApnGlueIsEthernet(){
	if(isethernet) return 1;
	else return 0;
}
static std::string cam_msg_id = "";
void ApnGlueSetMsgId(char *str){
	cam_msg_id = str;
}
// static variable with last error
CCDerr altaerr = ALTA_OK;

typedef struct{
	std::string address;
	uint16_t FirmwareRev;
	uint16_t Id;
	std::string deviceType;
	std::string model;
} CamParams;
// last camera parameters
static CamParams par;

#define TRY altaerr=ALTA_OK; try
#ifdef EBUG
	#define SHOWEX() do{std::cout << "ERROR! " << err.what() << std::endl;}while(0)
	#define DBG(msg) do{std::cout << msg << std::endl;}while(0)
#else
	#define SHOWEX()
	#define DBG(msg)
#endif
#define CATCH(ERR) catch(std::exception & err){altaerr=ERR;SHOWEX();}catch(...){altaerr=ALTA_UNDEF;}

#define RETERR(ERR) do{altaerr=ERR; return altaerr;}while(0)

static void checkalta(){
	if(!alta){
		fprintf(stderr, "Bug! Alta used before open\n");
		exit(1);
	}
}

// Functions for getting camera parameters from discovery string
// first two functions are from apogee's library
std::vector<std::string> MakeTokens(const std::string &str, const std::string &separator){
	std::vector<std::string> returnVector;
	std::string::size_type start = 0;
	std::string::size_type end = 0;
	while((end = str.find(separator, start)) != std::string::npos){
		returnVector.push_back (str.substr(start, end-start));
		start = end + separator.size();
	}
	returnVector.push_back( str.substr(start) );
	return returnVector;
}
std::string GetItemFromFindStr( const std::string & msg, const std::string & item ){
    std::vector<std::string> params =  MakeTokens(msg, "," );
	std::vector<std::string>::iterator iter;
	for(iter = params.begin(); iter != params.end(); ++iter){
		if( std::string::npos != (*iter).find( item )){
		 std::string result = MakeTokens((*iter), "=" ).at(1);
		 return result;
		}
	}
//	fprintf(stderr, "Bug! Can't find parameter in description string!\n");
//	exit(1);
	std::string noOp;
	return noOp;
}
uint16_t readUI(const std::string & msg, const std::string & key){
	std::string str = GetItemFromFindStr(msg, key);
	unsigned int x;
	if(sscanf(str.c_str(), "%x", &x) < 1){
		fprintf(stderr, "Bug! Can't convert parameter from hex!\n");
		exit(1);
	}
	return (uint16_t) x;
}
CamParams *getCamParams(std::string & msg){
	std::string port = GetItemFromFindStr(msg, "port=");
	par.address = GetItemFromFindStr(msg, "address=");
	if(port.size()){ // there's an network device
		par.address.append(":");
		par.address.append(port);
	}
	par.FirmwareRev = readUI(msg, "firmwareRev=");
	par.Id = readUI(msg, "id=");
	par.deviceType = GetItemFromFindStr(msg, "deviceType=");
	par.model = GetItemFromFindStr(msg, "model=");
	return &par;
}
/*
static bool IsProperDevice(const std::string & msg){
	std::string model = GetItemFromFindStr(msg, "model=");
	std::string
#if defined APOGEE_ASCENT
	cam("Ascent");
#elif defined APOGEE_ALTA
	cam("Alta");
#elif defined APOGEE_ALTAF
	cam("AltaF");
#elif defined APOGEE_ASPEN
	cam("Aspen");
#elif defined APOGEE_QUAD
	cam("Quad");
#else
	return false;
#endif
    return(0 == model.compare(0, cam.size(), cam) ? true : false );
}*/

static bool assignAlta(const std::string & msg){
	std::string model = GetItemFromFindStr(msg, "model=");
	if(model.compare(0,6,"Ascent")==0) alta = (Ascent*) new Ascent();
	else if(model.compare(0,5,"AltaF")==0) alta = (AltaF*) new AltaF();
	else if(model.compare(0,4,"Alta")==0) alta = (Alta*) new Alta();
	else if(model.compare(0,5,"Aspen")==0) alta = (Aspen*) new Aspen();
	else if(model.compare(0,4,"Quad")==0) alta = (Quad*) new Quad();
	else return false;
	return true;
}

/**
 * return camera info
 * Don't forget to call FREE!!!
 * @param pid, vid - device PID and VID
 */
char *ApnGlueGetInfo(int *pid, int *vid){
	if(!alta || isethernet) return NULL;
	if(pid || vid){
		uint16_t v = 1, p = 2, d = 3;
		alta->GetUsbVendorInfo(v, p, d);
		if(pid) *pid = p;
		if(vid) *vid = v;
	}
	std::string str = alta->GetInfo();
	char *writable = new char[str.size() + 1];
	std::copy(str.begin(), str.end(), writable);
	writable[str.size()] = '\0';
	return writable;
}

std::string subnet = "";
/**
 * Set subnet name to use network camera
 */
void ApnGlueSetSubnet(char *val){
	subnet = std::string(val);
}

/**
 * try to find USB device
 * return pointer to CamParams of found device or NULL
 */
static CamParams *findUSB(){
	FindDeviceUsb look4cam;
	std::string msg = look4cam.Find();
//	if(!IsProperDevice(msg)) // device not found
//		return NULL;
	std::cout << "Camera MSG_ID=" << msg << std::endl;
	if(!assignAlta(msg)){
		std::cerr << "Unknown camera!" << std::endl;
		return NULL;
	}
	isethernet = false;
	return getCamParams(msg);
}
/**
 * try to find ethernet device
 * return pointer to CamParams of found device or NULL
 */
static CamParams *findNET(){
	if(subnet.size() == 0) return NULL; // subnet wasn't defined
	FindDeviceEthernet look4cam;
	DBG(subnet);
	//clearenv(); // clear all proxy & other data
	setenv("http_proxy", "", 1);
	setenv("HTTP_PROXY", "", 1);
	std::string msg = look4cam.Find(subnet);
//	if(!IsProperDevice(msg)) // device not found
//		return NULL;
	std::cout << "Camera MSG_ID=" << msg << std::endl;
	if(!assignAlta(msg)){
		std::cerr << "Unknown camera!" << std::endl;
		return NULL;
	}
	isethernet = true;
	return getCamParams(msg);
}

/**
 * Open camera device and assign it to variable <alta>
 * IT DON'T WORK WITH MULTIPLE CAMERAS SIMULTANEOUSLY!
 * @param id - camera identificator (number)
 * @return 0 in case of success
 */
int ApnGlueOpen(_U_ unsigned int id){
	bool found = false;
	CamParams *campar = NULL;
	std::string ioInterface;
	TRY{
		if(cam_msg_id.size()){ // user had set MSG param, try it
			DBG("Try to find camera by given id");
			//if(IsProperDevice(cam_msg_id) && (campar = getCamParams(cam_msg_id))){
			if(assignAlta(cam_msg_id) && (campar = getCamParams(cam_msg_id))){
				found = true;
				if(GetItemFromFindStr(cam_msg_id, "interface=") == "ethernet"){
					clearenv(); // clear all proxy & other data
					isethernet = true;
				}else
					isethernet = false;
			}else std::cerr << "Can't find camera with given ID" << std::endl;
		}
		if(!found && subnet.size()){ // there's an ability of network camera presence
			DBG("Try to find network camera");
			if((campar = findNET())){
				found = true;
			}else std::cerr << "Network camera not found, try USB" << std::endl;
		}
		if(!found){
			DBG("Try to find USB camera");
			if(!(campar = findUSB()))
				RETERR(ALTA_NO_SUCH_DEVICE);
		}
		if(isethernet)
			ioInterface = "ethernet";
		else
			ioInterface = "usb";
		//alta = (CCD*) new CCD();
		alta->OpenConnection(ioInterface, campar->address, campar->FirmwareRev, campar->Id);
		alta->Init();
	}CATCH(ALTA_NO_SUCH_DEVICE);
	return(altaerr);
}


/**
 * Close connection and destroy camera object
 */
void ApnGlueClose(){
	if(!alta) return;
	TRY{
		alta->CloseConnection();
		delete alta;
	}CATCH(ALTA_CANT_CLOSE);
}

/**
 * Camera sensor and model
 * @param sensor, model (o) - dinamically allocated strings or NULL
 */
void ApnGlueGetName(char **sensor, char **camera){
	checkalta();
	TRY{
		if(camera) *camera = (char *)alta->GetModel().c_str();
		if(sensor) *sensor = (char *)alta->GetSensor().c_str();
	}CATCH(ALTA_UNDEF);
}

/**
 * Set fan speed
 * @param speed: 0 - turn fan off; 1 - slowest, 3 - fastest
 */
void ApnGlueSetFan(int speed){
	checkalta();
	TRY{
		Apg::FanMode mode = (Apg::FanMode)(speed % 4);
		alta->SetFanMode(mode);
	}CATCH(ALTA_BAD_FANSPD);
}

/**
 * Get fan speed
 * @return fan speed
 */
int ApnGlueGetFan(){
	int mode = -1;
	checkalta();
	TRY{
		mode = alta->GetFanMode();
	}CATCH(ALTA_BAD_FANSPD);
	return mode;
}


/**
 * Set parameters for subsequent exposures.
 * In case of error return -1 or altaerr with explanation in whynot[], else 0.
 * Also return final image size in net binned pixels, allowing for overscan,
 * binning etc.
 * @param roiw, roih - image width & height in unbinned pixels
 * @param osw, osh   - (USED only their sign) width & height of saved overscan part (should be less than max OS)
 * @param binw, binh - binning by X and Y
 * @param roix, roiy - coordinate of image angle on CCD
 * @param impixw, impixh (o) - size of output image (binned) or NULL
 * @param whynot[]   - error explanation or NULL
 * @return In case of error return -1 or altaerr with explanation in whynot[], else 0.
 */
int ApnGlueSetExpGeom(int roiw, int roih, int osw, int osh, int binw,
		int binh, int roix, int roiy, int *impixw, int *impixh, char whynot[]){
	uint16_t maxw, maxh, maxtotw;
	//uint16_t maxtoth;
	checkalta();
	TRY{
		maxw = alta->GetMaxImgCols();
		maxh = alta->GetMaxImgRows();
		maxtotw = alta->GetTotalRows();
		//maxtoth = alta->GetTotalCols();
		if(binw < 1) binw = 1;
		if(binh < 1) binh = 1;
		if(roiw < 1 || roiw > (int)maxw) roiw = maxw;
		if(roih < 1 || roih > (int)maxh) roih = maxh;
		if(osw > 0 || osh > 0){
			int maxosw = alta->GetNumOverscanCols();
			//int maxosh = ApogeeCam::m_CamCfgData->m_MetaData.OverscanRows;
			if (osw > maxosw){
				if(whynot) sprintf (whynot, "Max overscan columns is %d", maxosw);
				RETERR(ALTA_BAD_GEOMETRY);
			}
			/*if (osh > maxosh) {
			sprintf (whynot, "Max overscan rows is %d", maxosh);
			return (-1);
			}*/
			if (roix > 0 || roiw < maxw || roiy > 0 || roih < maxh) {
				if(whynot) sprintf (whynot, "Can not overscan with windowing");
				RETERR(ALTA_BAD_GEOMETRY);
			}
			roiw += osw;
			//roih += osh;
			if(roiw > (int)maxtotw) roiw = maxtotw;
			//if(roih > (int)maxtoth) roih = maxtoth;
			roiw = maxtotw;
			//roih = maxtoth;
			alta->SetDigitizeOverscan(true);
		}else alta->SetDigitizeOverscan(false);
		alta->SetRoiBinCol(binw);
		alta->SetRoiBinRow(binh);
		if(roix > 0 || roiy > 0){
			if((roix + roiw > maxw) || (roiy + roih > maxh)){
				if(whynot) sprintf (whynot, "Windowed image too large");
				RETERR(ALTA_BAD_GEOMETRY);
			}
			alta->SetRoiStartCol(roix);
			alta->SetRoiStartRow(roiy);
		}else{
			alta->SetRoiStartCol(0);
			alta->SetRoiStartRow(0);
		}
		alta->SetRoiNumCols(roiw / binw);
		alta->SetRoiNumRows(roih / binh);
		if(impixw) *impixw = roiw / binw;
		if(impixh) *impixh = roih / binh;
	}CATCH(ALTA_BAD_GEOMETRY);
	return altaerr;
}

/**
 * Get pixel size
 * @param pixX, pixY (o) - pixel size in mkm or NULL
 */
void ApnGlueGetGeom(double *pixX, double *pixY){
	checkalta();
	TRY{
		if(pixX) *pixX = alta->GetPixelWidth();
		if(pixY) *pixY = alta->GetPixelHeight();
	}CATCH(ALTA_UNDEF);
}

/**
 * Get extremal values
 * @param exptime    - exposition time (array: [min, max]) or NULL
 * @param roiw, roih - ROI image size or NULL
 * @param osw, osh   - overscan size (used only osw) or NULL
 * @param binw, binh - binning size or NULL
 * @param shutter    - shutter presence or NULL
 * @param mintemp    - minimal temperature or NULL
 */
void ApnGlueGetMaxValues(double *exptime, int *roiw, int *roih, int *osw,
		_U_ int *osh, int *binw, int *binh, int *shutter, double *mintemp){
	checkalta();
	TRY{
		if(exptime){
			exptime[0] = alta->GetMinExposureTime();
			exptime[1] = alta->GetMaxExposureTime();
		}
		if(roiw) *roiw = alta->GetMaxImgCols();
		if(roih) *roih = alta->GetMaxImgRows();
		if(osw) *osw = alta->GetNumOverscanCols();
		if(binw) *binw = alta->GetMaxBinCols();
		if(binh) *binh = alta->GetMaxBinRows();
		if(shutter) *shutter = 1; // !!!TODO!!!
		if(mintemp) *mintemp = -30.; // !!!TODO!!!
	}CATCH(ALTA_UNDEF);
}

/**
 * Get sensor temperature and cooler status
 * @param Cp - current sensor temperature
 * @return Cooler_Status value
 */
int ApnGlueGetTemp(double *Cp){
	int status = -1;
	checkalta();
	TRY{
		*Cp = alta->GetTempCcd();
		status = alta->GetCoolerStatus();
	}CATCH(ALTA_COOLER);
	return status;
}

/**
 * Get heatsink temperature (in degC)
 * @return hot side of Peltier temperature value
 */
double ApnGlueGetHotTemp(){
	checkalta();
	double temp = 1000.;
	TRY{
		temp = alta->GetTempHeatsink();
	}CATCH(ALTA_COOLER);
	return temp;
}

/**
 * Set CCD temperature (in degC)
 * @param C - new temperature, !!! value C == 0, disables cooler !!!
 * @return
 */
void ApnGlueSetTemp(double C){
	checkalta();
	TRY{
		if (C == 0) {
			alta->SetCooler(0);
		} else {
			alta->SetCooler(1);
			alta->SetCoolerSetPoint(C);
		}
	}CATCH(ALTA_COOLER);
}

/* Get setpoint temperature; return 1 if cooler is on, 0 if cooler is off;
	stat == 0 - clamp to setpoint, 1 - at setpoint*/
/**
 * Get temperature setpoint
 * @param temp (o) - temperature setpoint or NULL
 * @param stat (o) - ==1 if cooler is at setpoint or NULL
 * @return cooler status
 */
int ApnGlueReadSetPoint(double *temp, int *stat){
	checkalta();
	Apg::CoolerStatus S = Apg::CoolerStatus_Suspended;
	TRY{
		S = alta->GetCoolerStatus();
		if(S == Apg::CoolerStatus_Off) return S;
		if(stat) *stat = (S == Apg::CoolerStatus_AtSetPoint) ? 1:0;
		if(temp) *temp = alta->GetCoolerSetPoint();
	}CATCH(ALTA_COOLER);
	return S;
}

/**
 * Read shutter state
 * @return 1 if shutter opened, 0 if closed
 */
int ApnGlueReadShutter(){
	checkalta();
	int state = 0;
	TRY{
		Apg::ShutterState shtr = alta->GetShutterState();
		if(shtr == Apg::ShutterState_ForceOpen)
			state = 1;
	}CATCH(ALTA_SHTR);
	return state;
}
/**
 * Force open/close shutter
 * @param flag - == 0 to close, != 0 to open
 */
void ApnGlueOpenShutter(int flag){
	checkalta();
	TRY{
		Apg::ShutterState shtr = (flag) ? Apg::ShutterState_ForceOpen : Apg::ShutterState_ForceClosed;
		alta->SetShutterState(shtr);
	}CATCH(ALTA_SHTR);
}

/**
 * Get ADC speed
 * @return Adc_Speed value
 */
unsigned short ApnGlueGetSpeed(){
	checkalta();
	Apg::AdcSpeed spd = Apg::AdcSpeed_Unknown;
	TRY{
		spd = alta->GetCcdAdcSpeed();
	}CATCH(ALTA_BADSPD);
	return (unsigned short)spd;
}

/**
 * Set ADC speed
 * @param Sp - speed (Adc_Speed)
 */
void ApnGlueSetSpeed(unsigned short Sp){
	checkalta();
	TRY{
		if(Sp != AdcSpeed_Normal && Sp != AdcSpeed_Fast && Sp != AdcSpeed_Video){
			altaerr = ALTA_BADSPD;
			return;
		}
		alta->SetCcdAdcSpeed((Apg::AdcSpeed)Sp);
	}CATCH(ALTA_BADSPD);
}

/**
 * Reset camera
 */
void ApnGlueReset(){
	checkalta();
	TRY{
		alta->Reset();
	}CATCH(ALTA_POWER);
}
/**
 * Power down/resume - just a stub for backward compability
 */
int ApnGluePowerDown(){return -1;}
void ApnGluePowerResume(){}

/**
 * Disable (flag!=1) or enable post expose flushing
 * @param flag == 1 to disable post-flushing
 */
void ApnGlueDisablePostExpFlushing(int flag){
	checkalta();
	TRY{
		alta->SetPostExposeFlushing(flag);
	}CATCH(ALTA_UNDEF);
}

/**
 * Set (flag!=0) or reset preexposure IR-flashing
 * @param flag != 1 to enable pre-flash
 * @return
 */
void ApnGluePreFlash(int flag){
	checkalta();
	TRY{
		alta->SetPreFlash(flag);
	}CATCH(ALTA_UNDEF);
}

/**
 * Set ADC databits (16/12)
 * !!!DEPRECATED!!! Use ApnGlueSetSpeed() instead!
 * @param BitResolution - ADC resolution
 */
void ApnGlueSetDatabits(Apn_Resolution BitResolution){
	checkalta();
	TRY{
		Adc_Speed sp = AdcSpeed_Normal;
		if(BitResolution != Resolution_SixteenBit)
			sp = AdcSpeed_Fast;
		ApnGlueSetSpeed(sp);
	}CATCH(ALTA_UNDEF);
}

/**
 * Set camera mode
 * @param CameraMode - one of camera mode
 */
void ApnGlueWriteCamMode(Apn_CameraMode CameraMode){
	checkalta();
	TRY{
		alta->SetCameraMode((Apg::CameraMode)CameraMode);
	}CATCH(ALTA_UNDEF);
}

/* start an exposure with the given duration (secs) and whether to open shutter.
 * update *exptime with the actual exposure time (may have been bumped to min).
 * return 0 if ok, else -1
 */
/**
 * Start an exposure with the given duration (secs)
 * Update *exptime with the actual exposure time
 * @param exptime (i/o) - exposition time in secs
 * @param shutter       - shutter state (1 - object, 0 - dark)
 * @return ALTA_OK if ok
 */
int ApnGlueStartExp(double *exptime, int shutter){
	checkalta();
	if(!exptime) return ALTA_EXPOSURE;
	TRY{
		double maxtime; // ,mintime
		//mintime = alta->GetMinExposureTime();
		maxtime = alta->GetMaxExposureTime();
		if(*exptime > maxtime) *exptime = maxtime;
		else if(*exptime < 0.) *exptime = 0.;
		alta->SetImageCount(1);
		alta->StartExposure(*exptime, shutter);
	}CATCH(ALTA_EXPOSURE);
	return altaerr;
}

/**
 * Pause or resume timer
 * @param flag - !=0 to pause, ==0 to resume
 * @return
 */
void ApnGluePauseTimer(int flag){
	checkalta();
	TRY{
		alta->PauseTimer(flag);
	}CATCH(ALTA_EXPOSURE);
}

/**
 * These two functions makes similar work: aborts exposition
 * ApnGlueExpAbort() fully aborts it, whith sensor cleaning
 * ApnGlueStopExposure() allows to read stored image
 * @return ApnGlueStopExposure() returns value of altaerr
 */
void ApnGlueExpAbort(){
	checkalta();
	TRY{
		alta->StopExposure(false);
	}CATCH(ALTA_EXPOSURE);
}
int ApnGlueStopExposure(){
	checkalta();
	TRY{
		alta->StopExposure(true);
	}CATCH(ALTA_EXPOSURE);
	return altaerr;
}

/**
 * Check whether exposition is done
 * @return 1 if exposition is over
 */
int ApnGlueExpDone(){
	checkalta();
	TRY{
		Apg::Status st = alta->GetImagingStatus();
		if(st == Apg::Status_ImageReady) return 1;
	}CATCH(ALTA_EXPOSURE);
	return 0;
}

/**
 * read pixels from the camera into the given buffer of length nbuf.
 * @param buf    - image buffer
 * @param nbuf   - buf size
 * @param whynot - error explanation or NULL
 * @return ALTA_OK if OK
 */
int ApnGlueReadPixels(unsigned short *buf, int nbuf, char whynot[]){
	checkalta();
	if(!buf || nbuf < 1) RETERR(ALTA_GETIMAGE);
	printf("READ\n");
	TRY{
		std::vector<uint16_t> v;
		alta->GetImage(v);
		if(nbuf < alta->GetRoiNumCols() * (int)alta->GetRoiNumRows() || nbuf < (int)v.size()){
			if(whynot) sprintf(whynot, "Buffer size less than image size");
			RETERR(ALTA_GETIMAGE);
		}
		std::copy(v.begin(), v.end(), buf);
	}CATCH(ALTA_GETIMAGE);
	printf("DONE\n");
	return altaerr;
}



/*
 ******************************** FILTER WHEEL ********************************
 */

static ApogeeFilterWheel *wheel;
static void checkwheel();

/**
 * Open and initialise filter wheel device
 * @param id   - number of wheel
 * @param type - wheel type
 * @return 0 if init OK, else return !0 and set altaerr
 */
int ApnGlueWheelOpen(_U_ unsigned int id, Apn_Filter type){
	TRY{
		FindDeviceUsb look4FilterWheel;
		std::string msg = look4FilterWheel.Find();
		DBG(msg);
		if(msg == "<d></d>")
			RETERR(ALTA_NO_SUCH_DEVICE);
		std::string str = GetItemFromFindStr(msg, "deviceType=");
		if(str.compare("filterWheel"))
			RETERR(ALTA_NOTWHEEL);
		wheel = (ApogeeFilterWheel*) new ApogeeFilterWheel();
		std::string addr = GetItemFromFindStr(msg, "address=");
		wheel->Init((ApogeeFilterWheel::Type)type, addr);
	}CATCH(ALTA_NO_SUCH_DEVICE);
	return 0;
}

/**
 * Close filter device
 */
void ApnGlueWheelClose(){
	TRY{
		if(wheel) wheel->Close();
		delete wheel;
	}CATCH(ALTA_CANT_CLOSE);
}

/**
 * Get current filter wheel status
 * @param
 * @return 0 if ready, 1 if moving or absent
 */
int ApnGlueWheelGetStatus(){
	checkwheel();
	TRY{
		ApogeeFilterWheel::Status st = wheel->GetStatus();
		if(st != ApogeeFilterWheel::READY) return 1;
	}CATCH(ALTA_NOTWHEEL);
	if(altaerr != ALTA_OK) return 1;
	return 0;
}

/**
 * Get maximum position number
 * @return amount of positions or -1 if err
 */
int ApnGlueWheelGetMaxPos(){
	uint16_t m = 0;
	checkwheel();
	TRY{
		m = wheel->GetMaxPositions();
	}CATCH(ALTA_NOTWHEEL);
	if(altaerr != ALTA_OK) return -1;
	return (int) m;
}

/**
 * Set position
 * @param pos - new position number (1..max)
 * @return 0 on success or !0 in case of error
 */
int ApnGlueWheelSetPos(int pos){
	checkwheel();
	TRY{
		if(pos < 1 || wheel->GetMaxPositions() < pos) RETERR(ALTA_BADWHEELPOS);
		wheel->SetPosition((uint16_t) pos);
	}CATCH(ALTA_BADWHEELPOS);
	if(altaerr != ALTA_OK) return ALTA_BADWHEELPOS;
	return 0;
}

/**
 * Get current position (1..max)
 * @return current position or -1 on error
 */
int ApnGlueWheelGetPos(){
	uint16_t m = 0;
	checkwheel();
	TRY{
		m = wheel->GetPosition();
	}CATCH(ALTA_BADWHEELPOS);
	if(altaerr != ALTA_OK) return -1;
	return (int)m;
}

/**
 * Check wheel
 */
static void checkwheel(){
	if(!wheel){
		fprintf(stderr, "Bug! Wheel used before open\n");
		exit(1);
	}
}
