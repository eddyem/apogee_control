#ifndef SLALIBHDEF
#define SLALIBHDEF
/*
**  Author:
**    Patrick Wallace  (ptw@tpsoft.demon.co.uk)
**
**  License:
**    This program is free software; you can redistribute it and/or modify
**    it under the terms of the GNU General Public License as published by
**    the Free Software Foundation; either version 2 of the License, or
**    (at your option) any later version.
**
**    This program is distributed in the hope that it will be useful,
**    but WITHOUT ANY WARRANTY; without even the implied warranty of
**    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**    GNU General Public License for more details.
**
**    You should have received a copy of the GNU General Public License
**    along with this program; if not, write to the Free Software 
**    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  
**    USA.
**
**  Last revision:   10 December 2002
**
*/

#ifdef __cplusplus
extern "C" {
#endif

#include <math.h>

void sla_addet ( double rm, double dm, double eq, double *rc, double *dc );

void sla_afin ( char *string, int *iptr, float *a, int *j );

double sla_airmas ( double zd );

void sla_altaz ( double ha, double dec, double phi,
                double *az, double *azd, double *azdd,
                double *el, double *eld, double *eldd,
                double *pa, double *pad, double *padd );

void sla_amp ( double ra, double da, double date, double eq,
              double *rm, double *dm );

void sla_ampqk ( double ra, double da, double amprms[21],
                double *rm, double *dm );

void sla_aop ( double rap, double dap, double date, double dut,
              double elongm, double phim, double hm, double xp,
              double yp, double tdk, double pmb, double rh,
              double wl, double tlr,
              double *aob, double *zob, double *hob,
              double *dob, double *rob );

void sla_aoppa ( double date, double dut, double elongm, double phim,
                double hm, double xp, double yp, double tdk, double pmb,
                double rh, double wl, double tlr, double aoprms[14] );

void sla_aoppat ( double date, double aoprms[14] );

void sla_aopqk ( double rap, double dap, double aoprms[14],
                double *aob, double *zob, double *hob,
                double *dob, double *rob );

void sla_atmdsp ( double tdk, double pmb, double rh, double wl1,
                 double a1, double b1, double wl2, double *a2, double *b2 );

void sla_av2m ( float axvec[3], float rmat[3][3] );

float sla_bear ( float a1, float b1, float a2, float b2 );

void sla_caf2r ( int ideg, int iamin, float asec, float *rad, int *j );

void sla_caldj ( int iy, int im, int id, double *djm, int *j );

void sla_calyd ( int iy, int im, int id, int *ny, int *nd, int *j );

void sla_cc2s ( float v[3], float *a, float *b );

void sla_cc62s ( float v[6], float *a, float *b, float *r,
                float *ad, float *bd, float *rd );

void sla_cd2tf ( int ndp, float days, char *sign, int ihmsf[4] );

void sla_cldj ( int iy, int im, int id, double *djm, int *j );

void sla_clyd ( int iy, int im, int id, int *ny, int *nd, int *jstat );

void sla_combn ( int nsel, int ncand, int list[], int *j );

void sla_cr2af ( int ndp, float angle, char *sign, int idmsf[4] );

void sla_cr2tf ( int ndp, float angle, char *sign, int ihmsf[4] );

void sla_cs2c ( float a, float b, float v[3] );

void sla_cs2c6 ( float a, float b, float r, float ad,
                float bd, float rd, float v[6] );

void sla_ctf2d ( int ihour, int imin, float sec, float *days, int *j );

void sla_ctf2r ( int ihour, int imin, float sec, float *rad, int *j );

void sla_daf2r ( int ideg, int iamin, double asec, double *rad, int *j );

void sla_dafin ( char *string, int *iptr, double *a, int *j );

double sla_dat ( double dju );

void sla_dav2m ( double axvec[3], double rmat[3][3] );

double sla_dbear ( double a1, double b1, double a2, double b2 );

void sla_dbjin ( char *string, int *nstrt,
                double *dreslt, int *jf1, int *jf2 );

void sla_dc62s ( double v[6], double *a, double *b, double *r,
                double *ad, double *bd, double *rd );

void sla_dcc2s ( double v[3], double *a, double *b );

void sla_dcmpf ( double coeffs[6], double *xz, double *yz, double *xs,
                double *ys, double *perp, double *orient );

void sla_dcs2c ( double a, double b, double v[3] );

void sla_dd2tf ( int ndp, double days, char *sign, int ihmsf[4] );

void sla_de2h ( double ha, double dec, double phi,
               double *az, double *el );

void sla_deuler ( char *order, double phi, double theta, double psi,
                 double rmat[3][3] );

void sla_dfltin ( char *string, int *nstrt, double *dreslt, int *jflag );

void sla_dh2e ( double az, double el, double phi, double *ha, double *dec);

void sla_dimxv ( double dm[3][3], double va[3], double vb[3] );

void sla_djcal ( int ndp, double djm, int iymdf[4], int *j );

void sla_djcl ( double djm, int *iy, int *im, int *id, double *fd, int *j );

void sla_dm2av ( double rmat[3][3], double axvec[3] );

void sla_dmat ( int n, double *a, double *y, double *d, int *jf, int *iw );

void sla_dmoon ( double date, double pv[6] );

void sla_dmxm ( double a[3][3], double b[3][3], double c[3][3] );

void sla_dmxv ( double dm[3][3], double va[3], double vb[3] );

double sla_dpav ( double v1[3], double v2[3] );

void sla_dr2af ( int ndp, double angle, char *sign, int idmsf[4] );

void sla_dr2tf ( int ndp, double angle, char *sign, int ihmsf[4] );

double sla_drange ( double angle );

double sla_dranrm ( double angle );

void sla_ds2c6 ( double a, double b, double r, double ad, double bd,
                double rd, double v[6] );

void sla_ds2tp ( double ra, double dec, double raz, double decz,
                double *xi, double *eta, int *j );

double sla_dsep ( double a1, double b1, double a2, double b2 );

double sla_dsepv ( double v1[3], double v2[3] );

double sla_dt ( double epoch );

void sla_dtf2d ( int ihour, int imin, double sec, double *days, int *j );

void sla_dtf2r ( int ihour, int imin, double sec, double *rad, int *j );

void sla_dtp2s ( double xi, double eta, double raz, double decz,
                double *ra, double *dec );

void sla_dtp2v ( double xi, double eta, double v0[3], double v[3] );

void sla_dtps2c ( double xi, double eta, double ra, double dec,
                 double *raz1, double *decz1,
                 double *raz2, double *decz2, int *n );

void sla_dtpv2c ( double xi, double eta, double v[3],
                 double v01[3], double v02[3], int *n );

double sla_dtt ( double dju );

void sla_dv2tp ( double v[3], double v0[3], double *xi, double *eta, int *j );

double sla_dvdv ( double va[3], double vb[3] );

void sla_dvn ( double v[3], double uv[3], double *vm );

void sla_dvxv ( double va[3], double vb[3], double vc[3] );

void sla_e2h ( float ha, float dec, float phi, float *az, float *el );

void sla_earth ( int iy, int id, float fd, float posvel[6] );

void sla_ecleq ( double dl, double db, double date, double *dr, double *dd );

void sla_ecmat ( double date, double rmat[3][3] );

void sla_ecor ( float rm, float dm, int iy, int id, float fd,
               float *rv, float *tl );

void sla_eg50 ( double dr, double dd, double *dl, double *db );

void sla_el2ue ( double date, int jform, double epoch, double orbinc,
                double anode, double perih, double aorq, double e,
                double aorl, double dm, double u[], int *jstat );

double sla_epb ( double date );

double sla_epb2d ( double epb );

double sla_epco ( char k0, char k, double e );

double sla_epj ( double date );

double sla_epj2d ( double epj );

void sla_eqecl ( double dr, double dd, double date, double *dl, double *db );

double sla_eqeqx ( double date );

void sla_eqgal ( double dr, double dd, double *dl, double *db );

void sla_etrms ( double ep, double ev[3] );

void sla_euler ( char *order, float phi, float theta, float psi,
                float rmat[3][3] );

void sla_evp ( double date, double deqx,
              double dvb[3], double dpb[3],
              double dvh[3], double dph[3] );

void sla_fitxy ( int itype, int np, double xye[][2], double xym[][2],
                double coeffs[6], int *j );

void sla_fk425 ( double r1950, double d1950, double dr1950,
                double dd1950, double p1950, double v1950,
                double *r2000, double *d2000, double *dr2000,
                double *dd2000, double *p2000, double *v2000 );

void sla_fk45z ( double r1950, double d1950, double bepoch,
                double *r2000, double *d2000 );

void sla_fk524 ( double r2000, double d2000, double dr2000,
                double dd2000, double p2000, double v2000,
                double *r1950, double *d1950, double *dr1950,
                double *dd1950, double *p1950, double *v1950 );

void sla_fk52h ( double r5, double d5, double dr5, double dd5,
                double *dr, double *dh, double *drh, double *ddh );

void sla_fk54z ( double r2000, double d2000, double bepoch,
                double *r1950, double *d1950,
                double *dr1950, double *dd1950 );

void sla_fk5hz ( double r5, double d5, double epoch,
                double *rh, double *dh );

void sla_flotin ( char *string, int *nstrt, float *reslt, int *jflag );

void sla_galeq ( double dl, double db, double *dr, double *dd );

void sla_galsup ( double dl, double db, double *dsl, double *dsb );

void sla_ge50 ( double dl, double db, double *dr, double *dd );

void sla_geoc ( double p, double h, double *r, double *z );

double sla_gmst ( double ut1 );

double sla_gmsta ( double date, double ut1 );

void sla_h2e ( float az, float el, float phi, float *ha, float *dec );

void sla_h2fk5 ( double dr, double dh, double drh, double ddh,
                double *r5, double *d5, double *dr5, double *dd5 );

void sla_hfk5z ( double rh, double dh, double epoch,
                double *r5, double *d5, double *dr5, double *dd5 );

void sla_imxv ( float rm[3][3], float va[3], float vb[3] );

void sla_int2in ( char *string, int *nstrt, int *ireslt, int *jflag );

void sla_intin ( char *string, int *nstrt, long *ireslt, int *jflag );

void sla_invf ( double fwds[6], double bkwds[6], int *j );

void sla_kbj ( int jb, double e, char *k, int *j );

void sla_m2av ( float rmat[3][3], float axvec[3] );

void sla_map ( double rm, double dm, double pr, double pd,
              double px, double rv, double eq, double date,
              double *ra, double *da );

void sla_mappa ( double eq, double date, double amprms[21] );

void sla_mapqk ( double rm, double dm, double pr, double pd,
                double px, double rv, double amprms[21],
                double *ra, double *da );

void sla_mapqkz ( double rm, double dm, double amprms[21],
                 double *ra, double *da );

void sla_moon ( int iy, int id, float fd, float posvel[6] );

void sla_mxm ( float a[3][3], float b[3][3], float c[3][3] );

void sla_mxv ( float rm[3][3], float va[3], float vb[3] );

void sla_nut ( double date, double rmatn[3][3] );

void sla_nutc ( double date, double *dpsi, double *deps, double *eps0 );

void sla_nutc80 ( double date, double *dpsi, double *deps, double *eps0 );

void sla_oap ( char *type, double ob1, double ob2, double date,
              double dut, double elongm, double phim, double hm,
              double xp, double yp, double tdk, double pmb,
              double rh, double wl, double tlr,
              double *rap, double *dap );

void sla_oapqk ( char *type, double ob1, double ob2, double aoprms[14],
                double *rap, double *dap );

void sla_obs ( int n, char *c, char *name, double *w, double *p, double *h );

double sla_pa ( double ha, double dec, double phi );

double sla_pav ( float v1[3], float v2[3] );

void sla_pcd ( double disco, double *x, double *y );

void sla_pda2h ( double p, double d, double a,
                double *h1, int *j1, double *h2, int *j2 );

void sla_pdq2h ( double p, double d, double q,
                double *h1, int *j1, double *h2, int *j2 );

void sla_permut ( int n, int istate[], int iorder[], int *j );

void sla_pertel (int jform, double date0, double date1,
                double epoch0, double orbi0, double anode0,
                double perih0, double aorq0, double e0, double am0,
                double *epoch1, double *orbi1, double *anode1,
                double *perih1, double *aorq1, double *e1, double *am1,
                int *jstat );

void sla_pertue ( double date, double u[], int *jstat );

void sla_planel ( double date, int jform, double epoch, double orbinc,
                 double anode, double perih, double aorq,  double e,
                 double aorl, double dm, double pv[6], int *jstat );

void sla_planet ( double date, int np, double pv[6], int *j );

void sla_plante ( double date, double elong, double phi, int jform,
                 double epoch, double orbinc, double anode, double perih,
                 double aorq, double e, double aorl, double dm,
                 double *ra, double *dec, double *r, int *jstat );

void sla_plantu ( double date, double elong, double phi, double u[],
                 double *ra, double *dec, double *r, int *jstat );

void sla_pm ( double r0, double d0, double pr, double pd,
             double px, double rv, double ep0, double ep1,
             double *r1, double *d1 );

void sla_polmo ( double elongm, double phim, double xp, double yp,
                double *elong, double *phi, double *daz );

void sla_prebn ( double bep0, double bep1, double rmatp[3][3] );

void sla_prec ( double ep0, double ep1, double rmatp[3][3] );

void sla_precl ( double ep0, double ep1, double rmatp[3][3] );

void sla_preces ( char sys[3], double ep0, double ep1,
                 double *ra, double *dc );

void sla_prenut ( double epoch, double date, double rmatpn[3][3] );

void sla_pv2el ( double pv[], double date, double pmass, int jformr,
                int *jform, double *epoch, double *orbinc,
                double *anode, double *perih, double *aorq, double *e,
                double *aorl, double *dm, int *jstat );

void sla_pv2ue ( double pv[], double date, double pmass,
                double u[], int *jstat );

void sla_pvobs ( double p, double h, double stl, double pv[6] );

void sla_pxy ( int np, double xye[][2], double xym[][2],
              double coeffs[6],
              double xyp[][2], double *xrms, double *yrms, double *rrms );

float sla_range ( float angle );

float sla_ranorm ( float angle );

double sla_rcc ( double tdb, double ut1, double wl, double u, double v );

void sla_rdplan ( double date, int np, double elong, double phi,
                 double *ra, double *dec, double *diam );

void sla_refco ( double hm, double tdk, double pmb, double rh,
                double wl, double phi, double tlr, double eps,
                double *refa, double *refb );

void sla_refcoq ( double tdk, double pmb, double rh, double wl,
                double *refa, double *refb );

void sla_refro ( double zobs, double hm, double tdk, double pmb,
                double rh, double wl, double phi, double tlr, double eps,
                double *ref );

void sla_refv ( double vu[3], double refa, double refb, double vr[3] );

void sla_refz ( double zu, double refa, double refb, double *zr );

float sla_rverot ( float phi, float ra, float da, float st );

float sla_rvgalc ( float r2000, float d2000 );

float sla_rvlg ( float r2000, float d2000 );

float sla_rvlsrd ( float r2000, float d2000 );

float sla_rvlsrk ( float r2000, float d2000 );

void sla_s2tp ( float ra, float dec, float raz, float decz,
               float *xi, float *eta, int *j );

float sla_sep ( float a1, float b1, float a2, float b2 );

float sla_sepv ( float v1[3], float v2[3] );

void sla_smat ( int n, float *a, float *y, float *d, int *jf, int *iw );

void sla_subet ( double rc, double dc, double eq,
                double *rm, double *dm );

void sla_supgal ( double dsl, double dsb, double *dl, double *db );

void sla_svd ( int m, int n, int mp, int np,
              double *a, double *w, double *v, double *work,
              int *jstat );

void sla_svdcov ( int n, int np, int nc,
                 double *w, double *v, double *work, double *cvm );

void sla_svdsol ( int m, int n, int mp, int np,
                 double *b, double *u, double *w, double *v,
                 double *work, double *x );

void sla_tp2s ( float xi, float eta, float raz, float decz,
               float *ra, float *dec );

void sla_tp2v ( float xi, float eta, float v0[3], float v[3] );

void sla_tps2c ( float xi, float eta, float ra, float dec,
                float *raz1, float *decz1,
                float *raz2, float *decz2, int *n );

void sla_tpv2c ( float xi, float eta, float v[3],
                float v01[3], float v02[3], int *n );

void sla_ue2el ( double u[], int jformr,
                int *jform, double *epoch, double *orbinc,
                double *anode, double *perih, double *aorq, double *e,
                double *aorl, double *dm, int *jstat );

void sla_ue2pv ( double date, double u[], double pv[], int *jstat );

void sla_unpcd ( double disco, double *x, double *y );

void sla_v2tp ( float v[3], float v0[3], float *xi, float *eta, int *j );

float sla_vdv ( float va[3], float vb[3] );

void sla_vn ( float v[3], float uv[3], float *vm );

void sla_vxv ( float va[3], float vb[3], float vc[3] );

void sla_xy2xy ( double x1, double y1, double coeffs[6],
                double *x2, double *y2 );

double sla_zd ( double ha, double dec, double phi );

#ifdef __cplusplus
}
#endif

#endif
