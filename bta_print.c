/* Print some BTA NewACS data (or write  to file)
 * Usage:
 *         bta_print [time_step] [file_name]
 * Where:
 *         time_step - writing period in sec., >=1.0
 *                      <1.0 - once and exit (default)
 *         file_name - name of file to write to,
 *                      "-" - stdout (default)
 */
#include "bta_shdata.h"
#include "takepic.h"
#include "camtools.h"
#include "bta_print.h"

#define CMNTSZ 79
char comment[CMNTSZ + 1];
#define CMNT(...) snprintf(comment, CMNTSZ, __VA_ARGS__)
#define FTKEY(...) WRITEKEY(fp, __VA_ARGS__, comment)
#define WRITEHIST(fp)							\
do{ if(test_headers){printf("HISTORY: %s\n", comment);}else{	\
	int status = 0;								\
	fits_write_history(fp, comment, &status);		\
	if(status) fits_report_error(stderr, status);\
}}while(0)

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
	h   = (int)(t/3600.);
	m = (int)((t - (double)h*3600.)/60.);
	s = t - (double)h*3600. - (double)m*60.;
	h %= 24;
	if(s>59) s=59;
	sprintf(buf, "%dh:%02dm:%04.1fs", h,m,s);
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
	double Sid_time;		// S_time-EE_time
	double JD;				// JDate
	double Alpha;		// val_Alp
	double Delta;		// val_Del
	double Azimuth;		// val_A
	double Zenith;		// val_Z
	double P2;			// val_P
	double Wind;			// val_Wnd
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
	time_t t_now = time(NULL);
	struct tm *tm_ut, *tm_loc;
	tm_ut = gmtime(&t_now);
	tm_loc = localtime(&t_now);
	if(!get_shm_block(&sdat, ClientSide)) return;
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
	// A / Azimuth
	CMNT("Current object Azimuth: %s", angle_asc(tag_A));
	dtmp = tag_A / 3600.; FTKEY(TDOUBLE, "A", &dtmp);
	// Z / Zenith distance
	CMNT("Current object Zenith: %s", angle_asc(tag_Z));
	dtmp = tag_Z / 3600.; FTKEY(TDOUBLE, "Z", &dtmp);
	// ROTANGLE / Field rotation angle
	CMNT("Field rotation angle: %s", angle_asc(tag_P));
	dtmp = tag_P / 3600.;FTKEY(TDOUBLE, "ROTANGLE", &dtmp);

	CMNT("Telescope A: %s", angle_asc(val_A));
	dtmp = val_A / 3600.; FTKEY(TDOUBLE, "VAL_A", &dtmp);
	CMNT("Telescope Z: %s", angle_asc(val_Z));
	dtmp = val_Z / 3600.; FTKEY(TDOUBLE, "VAL_Z", &dtmp);
	CMNT("Current P: %s", angle_asc(val_P));
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
	if(!get_shm_block(&sdat, ClientSide)) return -1;
	if(!check_shm_block(&sdat)) return -1;
	P->Sid_time = S_time-EE_time;
	P->JD = JDate;
	P->Alpha = val_Alp;
	P->Delta = val_Del;
	P->Azimuth = val_A;
	P->Zenith = val_Z;
	P->P2 = val_P;
	P->Wind = val_Wnd;
	return 0;
}

int push_param(){
	if(!check_shm_block(&sdat)) return -3;
	BTA_PARAMS *Par = calloc(1, sizeof(BTA_PARAMS));
	BTA_Queue *qcur = calloc(1, sizeof(BTA_Queue));
	if(!Par || !qcur){
		free(Par);
		free(qcur);
		return -1; // malloc error
	}
	if(get_params(Par)){
		free(Par);
		free(qcur);
		return -2; // error getting parameters
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
	if(!bta_queue) return;
	BTA_Queue *cur = bta_queue;
	BTA_PARAMS *P;
	int i = 0;
	do{
		P = cur->data;
		HISTRY("Data record # %d", i);
		HISTRY("ST      = %.3f / Sidereal time: %s", P->Sid_time, time_asc(P->Sid_time));
		HISTRY("JD      = %.8g / Julian date", P->JD);
		HISTRY("T_RA    = %.8g / Telescope R.A: %s", P->Alpha / 3600., time_asc(P->Alpha));
		HISTRY("T_DEC   = %.8g / Telescope Decl.: %s", P->Delta / 3600., angle_asc(P->Delta));
		HISTRY("T_AZ    = %.8g / Telescope Az: %s", P->Azimuth / 3600., angle_asc(P->Azimuth));
		HISTRY("T_ZD    = %.8g / Telescope ZD: %s", P->Zenith / 3600., angle_asc(P->Zenith));
		HISTRY("T_P2    = %.8g / Current P: %s", P->P2 / 3600., angle_asc(P->P2));
		HISTRY("WIND    = %.2g / Wind speed, m/s", P->Wind);
		i++;
	}while((cur = cur->next));
}
