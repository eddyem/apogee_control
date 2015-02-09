#include "takepic.h"
//#define __USE_XOPEN
//#include <math.h>
static int main(int argc, char *argv[]) __attribute((unused));
static void usage(void) __attribute((unused));
#define inline
#include "airmass.c"
#undef inline

void calc_airmass(
			// in
			double daynum,	// Day number from beginning of year, 0 = midnight Jan 1
			double h0,// Relative humidity in percent
			double p0,		// local pressure in mmHg
			double t0,		// temperature in kelvins
			double z,		// zenith distance in degr
			// out
			double *am,		// AIRMASS
			double *acd,	// column density
			double *wcd,	// water vapor column density
			double *wam		// water vapor airmass
	){
	int i;
	double colz, col0, cosyearfrac, sinz, rmsl, bigrmsl,
		rjunk, coslatsq, lambda=5500., alt=TELALT,
		lat=TELLAT, betapoly[5]={-0.0065107, -4.5403e-06,
		3.6599e-07, -2.2174e-09, 7.9392e-12}, r1poly[11]={17.204, 8.9155e-03,
		-3.6420e-03, 2.5617e-05, 2.4796e-07, -1.2774e-08, 1.3017e-10,
		2.0151e-12, -2.6985e-14, -1.0397e-16, 1.4849e-18}, hvec[(NTLEVELS+1)]=
		{0., 11000., 20000., 32000., 47000., 52000., 61000., 79000., 88743.};
	p0 *= 101325. / 760.;
	relhumid = h0 / 100.;
	sinz=sin(z*DEG2RAD);
	cee=CONSTC*(2.87566E-4+134.12/(lambda*lambda)+3.777E8*pow(lambda, -4.));
	cosyearfrac=cos((daynum-202.)*(360.*DEG2RAD/365.));
	coslatsq=cos(lat*DEG2RAD);
	coslatsq*=coslatsq;
	rmsl=sqrt(((POLAR*POLAR*POLAR*POLAR)+(EQUATOR*EQUATOR*EQUATOR*EQUATOR-
		POLAR*POLAR*POLAR*POLAR)*coslatsq)/((POLAR*POLAR)+(EQUATOR*EQUATOR-
		POLAR*POLAR)*coslatsq));
	bigrmsl=(-STANDARDR02)/rmsl;
   /* Calculate bigrvec, the bigr at the bottom of each temperature layer */
	*bigrvec=(-STANDARDR02)/(rmsl+alt);
	rjunk=r1poly[10];
	for (i=9;i>=0;i--) rjunk=rjunk*lat+((i%2) ? r1poly[i]*cosyearfrac :
		r1poly[i]);
	bigrvec[1]=(-STANDARDR02)/(rmsl+rjunk*1000.);
	for (i=2;i<(NTLEVELS+1);i++) bigrvec[i]=hvec[i]+bigrmsl;

   /* Set up our temperature profile for the troposphere/lower stratosphere */
	*betavec=betapoly[4];
	for (i=3;i>=0;i--) *betavec=*betavec*lat+((i%2) ? betapoly[i]*cosyearfrac :
		betapoly[i]);
	*betavec*=(-rmsl/bigrvec[1]);
	*tvec=t0;
	tvec[1]=t0+*betavec*(bigrvec[1]-*bigrvec);
	betavec[1]=(tvec[2]-tvec[1])/(bigrvec[2]-bigrvec[1]);

   /* Compute the density at the bottom of each temperature layer */
	*rhovec=p0/(AIRCONSTANT*t0);
	for (i=0;i<(NTLEVELS-1);i++) rhovec[i+1]=frho(bigrvec[i+1]-bigrvec[i],
		rhovec[i], tvec[i], betavec[i]);

	const6=musquare(*rhovec)*sinz*sinz/(*bigrvec*(*bigrvec));    /* for z */
	colz=qromb(columndensityint, *bigrvec, bigrvec[NTLEVELS], 1.E-8);
	const6=0.;  /* equivalent to setting z = 0. */
	col0=qromb(columndensityint, *bigrvec, bigrvec[NTLEVELS], 1.E-8);
	*am = colz/col0; *acd = 0.1*colz;
	if(h0 < 0.1){ *wcd = 0.; *wam = 0.; return;}
	const7=relhumid/(bigrvec[2]-bigrvec[1]);
	const6=musquare(*rhovec)*sinz*sinz/(*bigrvec*(*bigrvec));    /* for z */
	colz=qromb(vaporcolumndensityint, *bigrvec, bigrvec[2], 1.E-8);
	const6=0.;  /* equivalent to setting z = 0. */
	col0=qromb(vaporcolumndensityint, *bigrvec, bigrvec[2], 1.E-8);
	*wcd = 0.1*colz; *wam = colz/col0;
}
