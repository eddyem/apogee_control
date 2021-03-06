/*
 * bta_print.c
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

/* Print some BTA NewACS data (or write  to file)
 * Usage:
 *         bta_print [time_step] [file_name]
 * Where:
 *         time_step - writing period in sec., >=1.0
 *                      <1.0 - once and exit (default)
 *         file_name - name of file to write to,
 *                      "-" - stdout (default)
 */
#include "bta_print.h"
#include "bta_shdata.h"
#include "camtools.h"
#include "defhdrs.h"
#include "macros.h"
#include "takepic.h"
#include "usage.h" // command line parameters
#include <sofa.h>

//#include <slamac.h>  // SLA macros

#ifndef DR2S
#define DR2S 1.3750987083139757010431557155385240879777313391975e4
#endif
/*
extern void sla_amp(double*, double*, double*, double*, double*, double*);

void slaamp(double ra, double da, double date, double eq, double *rm, double *dm ){
    double r = ra, d = da, mjd = date, equi = eq;
    sla_amp(&r, &d, &mjd, &equi, rm, dm);
}
const double jd0 = 2400000.5; // JD for MJD==0
*/
/**
 * convert apparent coordinates (nowadays) to mean (JD2000)
 * appRA, appDecl in seconds
 * r, d in seconds
 */
void calc_mean(double appRA, double appDecl, double *r, double *dc){
    double ra=0., dec=0., utc1, utc2, tai1, tai2, tt1, tt2, fd, eo, ri;
    int y, m, d, H, M;
    DBG("appRa: %g'', appDecl'': %g", appRA, appDecl);
    appRA *= DS2R;
    appDecl *= DAS2R;
/*    double mjd = JDate - jd0;
    slaamp(appRA, appDecl, mjd, 2000.0, &ra, &dec);
    ra *= DR2S;
    dec *= DR2AS;
    DBG("SLALIB: r=%g'', d=%g''", ra, dec);
    ra = 0.; dec = 0.;*/
// SOFA
#define SOFA(f, ...) do{if(f(__VA_ARGS__)){WARNX("Error in " #f); goto rtn;}}while(0)
    // 1. convert system JDate to UTC
    SOFA(iauJd2cal, JDate, 0., &y, &m, &d, &fd);
    fd *= 24.;
    H = (int)fd;
    fd = (fd - H)*60.;
    M = (int)fd;
    fd = (fd - M)*60.;
    SOFA(iauDtf2d, "UTC", y, m, d, H, M, fd, &utc1, &utc2);
    SOFA(iauUtctai, utc1, utc2, &tai1, &tai2);
    SOFA(iauTaitt, tai1, tai2, &tt1, &tt2);
    iauAtic13(appRA, appDecl, tt1, tt2, &ri, &dec, &eo);
    ra = iauAnp(ri + eo);
    ra *= DR2S;
    dec *= DR2AS;
    DBG("SOFA: r=%g'', d=%g''", ra, dec);
#undef SOFA
rtn:
    if(r) *r = ra;
    if(dc) *dc = dec;
}

char comment[FLEN_CARD];
#define CMNT(...) snprintf(comment, FLEN_CARD, __VA_ARGS__)
#define FTKEY(...) WRITEKEY(__VA_ARGS__, comment)
#define WRITEHIST(fp)							\
do{ if(test_headers){printf("HISTORY: %s\n", comment);}else{	\
    int status = 0;								\
    fits_write_history(fp, comment, &status);		\
    if(status) fits_report_error(stderr, status);\
}}while(0)

int shm_ready = FALSE; // BTA shm_get

// calculate airmass
extern void calc_airmass(
    // in
    double daynum,	// Day number from beginning of year, 0 = midnight Jan 1
    double relhumid,// Relative humidity in percent
    double p0,		// local pressure in mmHg
    double t0,		// temperature in kelvins
    double z,		// zenith distance in degr
    // out
    double *am,		// AIRMASS
    double *acd,	// column density
    double *wcd,	// water vapor column density
    double *wam		// water vapor airmass
);

static char buf[1024];
char *time_asc(double t){
    int h, m;
    double s;
    char *str = "";
    h   = (int)(t/3600.);
    if(t < 0.){ t = -t; str = "-";}
    m = (int)((t - (double)h*3600.)/60.);
    s = t - (double)h*3600. - (double)m*60.;
    h %= 24;
    if(s>59) s=59;
    sprintf(buf, "%s%dh:%02dm:%04.1fs", str, h,m,s);
    return buf;
}

char *angle_asc(double a){
    char s;
    int d, min;
    double sec;
    if (a >= 0.)
        s = '+';
    else{
        s = '-'; a = -a;
    }
    d   = (int)(a/3600.);
    min = (int)((a - (double)d*3600.)/60.);
    sec = a - (double)d*3600. - (double)min*60.;
    d %= 360;
    if(sec>59.9) sec=59.9;
    sprintf(buf, "%c%d:%02d':%04.1f''", s,d,min,sec);
    return buf;
}

typedef struct{
    double JD;			// JDate
    double Azimuth;		// val_A
    double Zenith;		// val_Z
    double P2;			// val_P
    double Wind;		// val_Wnd
    //double Tmir;		//
} BTA_PARAMS;

typedef struct _BTA_Que{
    BTA_PARAMS *data;
    struct _BTA_Que *next;
    struct _BTA_Que *last;
} BTA_Queue;

void write_bta_queue(fitsfile *fp);
BTA_Queue *bta_queue = NULL;

void write_bta_data(fitsfile *fp){
    char *val;
    double dtmp;
    //char buf[FLEN_CARD];
    time_t t_now = time(NULL);
    struct tm *tm_ut, *tm_loc;
    tm_ut = gmtime(&t_now);
    tm_loc = localtime(&t_now);
    if(!shm_ready){
        if(!get_shm_block(&sdat, ClientSide)) return;
        else shm_ready = TRUE;
    }
    if(!check_shm_block(&sdat)) return;
    /*
     * Observatory parameters
     */
    // TELESCOP / Telescope name
    CMNT("Telescope name");
    FTKEY(TSTRING, "TELESCOP", "BTA 6m telescope");
    // ORIGIN / organization responsible for the data
    CMNT("organization responsible for the data");
    FTKEY(TSTRING, "ORIGIN", "SAO RAS");
    // OBSERVAT / Observatory name
    CMNT("Observatory name");
    FTKEY(TSTRING, "OBSERVAT", "Special Astrophysical Observatory, Russia");
    // placement
    CMNT("Observatory altitude, m");
    dtmp = TELALT; FTKEY(TDOUBLE, "ALT_OBS", &dtmp);
    CMNT("Observatory longitude, degr");
    dtmp = TELLONG; FTKEY(TDOUBLE, "LONG_OBS", &dtmp);
    CMNT("Observatory lattitude, degr");
    dtmp = TELLAT; FTKEY(TDOUBLE, "LAT_OBS", &dtmp);
    /*
     * Times
     */
    dtmp = S_time-EE_time;
    // ST / sidereal time (hh:mm:ss.ms)
    CMNT("Sidereal time: %s", time_asc(dtmp));
    FTKEY(TDOUBLE, "ST", &dtmp);
    // UT / universal time (hh:mm:ss.ms)
    CMNT("Universal time: %s", time_asc(M_time));
    FTKEY(TDOUBLE, "UT", &M_time);
    CMNT("Julian date");
    FTKEY(TDOUBLE, "JD", &JDate);
    /*
     * Telescope parameters
     */
    switch(Tel_Focus){
        case Nasmyth1 :  val = "Nasmyth1";  break;
        case Nasmyth2 :  val = "Nasmyth2";  break;
        default       :  val = "Prime";     break;
    }
    // FOCUS / Focus of telescope (mm)
    CMNT("Observation focus");
    FTKEY(TSTRING, "FOCUS", val);
    // VAL_F / focus value
    CMNT("Focus value (mm)");
    FTKEY(TDOUBLE, "VAL_F", &val_F);
    // EQUINOX / Epoch of RA & DEC
    dtmp = 1900 + tm_ut->tm_year + tm_ut->tm_yday / 365.2422;
    CMNT("Epoch of RA & DEC");
    FTKEY(TDOUBLE, "EQUINOX", &dtmp);
    CMNT("Current object R.A.: %s", time_asc(CurAlpha));
    // RA / Right ascention (H.H)
    dtmp = CurAlpha / 3600.; FTKEY(TDOUBLE, "RA", &dtmp);
    // DEC / Declination (D.D)
    CMNT("Current object Decl.: %s", angle_asc(CurDelta));
    dtmp = CurDelta / 3600.; FTKEY(TDOUBLE, "DEC", &dtmp);
    CMNT("Source R.A.: %s", time_asc(SrcAlpha));
    dtmp = SrcAlpha / 3600.; FTKEY(TDOUBLE, "S_RA", &dtmp);
    CMNT("Source Decl.: %s", angle_asc(SrcDelta));
    dtmp = SrcDelta / 3600.; FTKEY(TDOUBLE, "S_DEC", &dtmp);
    CMNT("Telescope R.A: %s", time_asc(val_Alp));
    dtmp = val_Alp / 3600.; FTKEY(TDOUBLE, "T_RA", &dtmp);
    CMNT("Telescope Decl.: %s", angle_asc(val_Del));
    dtmp = val_Del / 3600.; FTKEY(TDOUBLE, "T_DEC", &dtmp);
    double a2000, d2000;
    calc_mean(InpAlpha, InpDelta, &a2000, &d2000);
    CMNT("R.A. given by user (for J2000): %s", time_asc(a2000));
    dtmp = a2000 / 3600.;
    FTKEY(TDOUBLE, "INPRA0", &dtmp);
    CMNT("Decl. given by user (for J2000): %s", angle_asc(d2000));
    dtmp = d2000 / 3600;
    FTKEY(TDOUBLE, "INPDEC0", &dtmp);
    calc_mean(CurAlpha, CurDelta, &a2000, &d2000);
    CMNT("Current R.A. (for J2000): %s", time_asc(a2000));
    dtmp = a2000 / 3600.;
    FTKEY(TDOUBLE, "CURRA0", &dtmp);
    CMNT("Current Decl. (for J2000): %s", angle_asc(d2000));
    dtmp = d2000 / 3600;
    FTKEY(TDOUBLE, "CURDEC0", &dtmp);
    // A / Azimuth
    CMNT("Current object Azimuth: %s", angle_asc(tag_A));
    dtmp = tag_A / 3600.; FTKEY(TDOUBLE, "A", &dtmp);
    // Z / Zenith distance
    CMNT("Current object Zenith: %s", angle_asc(tag_Z));
    dtmp = tag_Z / 3600.; FTKEY(TDOUBLE, "Z", &dtmp);
    // PARANGLE / Parallactic angle
    CMNT("Parallactic  angle: %s", angle_asc(tag_P));
    dtmp = tag_P / 3600.;FTKEY(TDOUBLE, "PARANGLE", &dtmp);

    CMNT("Telescope A: %s", angle_asc(val_A));
    dtmp = val_A / 3600.; FTKEY(TDOUBLE, "VAL_A", &dtmp);
    CMNT("Telescope Z: %s", angle_asc(val_Z));
    dtmp = val_Z / 3600.; FTKEY(TDOUBLE, "VAL_Z", &dtmp);
    CMNT("Current P2 value: %s", angle_asc(val_P));
    dtmp = val_P / 3600.; FTKEY(TDOUBLE, "VAL_P", &dtmp);

    CMNT("Dome A: %s", angle_asc(val_D));
    dtmp = val_D / 3600.; FTKEY(TDOUBLE, "VAL_D", &dtmp);
    // OUTTEMP / Out temperature (C)
    CMNT("Outern temperature, degC");
    FTKEY(TDOUBLE, "OUTTEMP", &val_T1);
    // DOMETEMP / Dome temperature (C)
    CMNT("In-dome temperature, degC");
    FTKEY(TDOUBLE, "DOMETEMP", &val_T2);
    // MIRRTEMP / Mirror temperature (C)
    CMNT("Mirror temperature, degC");
    FTKEY(TDOUBLE, "MIRRTEMP", &val_T3);
    // PRESSURE / Pressure (mmHg)
    CMNT("Pressure, mmHg");
    FTKEY(TDOUBLE, "PRESSURE", &val_B);
    // WIND / Wind speed (m/s)
    CMNT("Wind speed, m/s");
    FTKEY(TDOUBLE, "WIND", &val_Wnd);
    CMNT("Humidity, %%");
    FTKEY(TDOUBLE, "HUM", &val_Hmd);
    /*
     * Airmass calculation
     * by Reed D. Meyer
     */
    double am, acd, wcd, wam;
    dtmp = tm_loc->tm_yday; // current day of year
    calc_airmass(dtmp, val_Hmd, val_B, val_T1+273.15, val_Z/3600., &am, &acd, &wcd, &wam);
    //printf("am=%g, acd=%g, wcd=%g, wam=%g\n", am,acd,wcd,wam);
    CMNT("Air mass by Reed D. Meyer");
    FTKEY(TDOUBLE, "AIRMASS", &am);
    CMNT("Water vapour air mass by Reed D. Meyer");
    FTKEY(TDOUBLE, "WVAM", &wam);
    CMNT("Atm. column density by Reed D. Meyer, g/cm^2");
    FTKEY(TDOUBLE, "ATMDENS", &acd);
    CMNT("WV column density by Reed D. Meyer, g/cm^2");
    FTKEY(TDOUBLE, "WVDENS", &wcd);
    // if bta_queue nonempty write its data to HISTORY section
    if(bta_queue) write_bta_queue(fp);
}

// get parameters
int get_params(BTA_PARAMS *P){
    if(!shm_ready){
        if(!get_shm_block(&sdat, ClientSide)) return -1;
        else shm_ready = TRUE;
    }
    if(!check_shm_block(&sdat)) return -1;
    P->JD = JDate;
    P->Azimuth = val_A;
    P->Zenith = val_Z;
    P->P2 = val_P;
    P->Wind = val_Wnd;
    return 0;
}

int push_param(){
    DBG("Try to push parameter");
    if(!shm_ready){
        if(!get_shm_block(&sdat, ClientSide)) return -4;
        else shm_ready = TRUE;
    }
    if(!check_shm_block(&sdat)) return -3;
    BTA_PARAMS *Par = calloc(1, sizeof(BTA_PARAMS));
    BTA_Queue *qcur = calloc(1, sizeof(BTA_Queue));
    if(!Par || !qcur){
        free(Par);
        free(qcur);
        return -2; // malloc error
    }
    if(get_params(Par)){
        free(Par);
        free(qcur);
        return -1; // error getting parameters
    }
    qcur->data = Par;
    qcur->next = NULL;
    qcur->last = qcur;
    if(!bta_queue){ // initialisation of que
        bta_queue = qcur;
    }else{ // variable que initialized previously
        bta_queue->last->next = qcur;
        bta_queue->last = qcur;
    }
    return 0;
}

void write_bta_queue(fitsfile *fp){
#define HISTRY(...) do{CMNT(__VA_ARGS__); WRITEHIST(fp);}while(0)
    if(!bta_queue || !fp) return;
    BTA_Queue *cur = bta_queue, *ptr;
    BTA_PARAMS *P;
    int i = 0;
    do{
        P = cur->data;
        ptr = cur;
        HISTRY("Data record # %d", i);
        HISTRY("JD      = %.8f / Julian date", P->JD);
        HISTRY("T_AZ    = %.8f / Telescope Az: %s", P->Azimuth / 3600., angle_asc(P->Azimuth));
        HISTRY("T_ZD    = %.8f / Telescope ZD: %s", P->Zenith / 3600., angle_asc(P->Zenith));
        HISTRY("T_P2    = %.8f / Current P: %s", P->P2 / 3600., angle_asc(P->P2));
        HISTRY("WIND    = %.2f / Wind speed, m/s", P->Wind);
        i++;
        cur = cur->next;
        free(P);
        free(ptr);
        bta_queue = cur;
    }while(cur);
}
