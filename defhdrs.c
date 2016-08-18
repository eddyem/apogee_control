/*
 * defhdrs.c - read default headers from user file
 *
 * Copyright 2016 Edward V. Emelianov <eddy@sao.ru, edward.emelianoff@gmail.com>
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
/*
 * Only these parameters are recognized:
SUBNET (instead of -E)
CAMMSGID (instead of -M)
INSTRUME
OBSERVER
PROG-ID
AUTHOR

XPIXELSZ
YPIXELSZ
IMSCALE
 */
#define _GNU_SOURCE
#include <math.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <fitsio.h>


#include "defhdrs.h"
#include "usage.h"
#include "macros.h"
#include "camtools.h"

// global keylist
static KeyList *FITS_keys = NULL;
double crval1 = -5e5, crval2 = -5e5, crpix1 = -5e5, crpix2 = -5e5;
double CD[2][2] = {{0.,0.}, {0.,0.}};

void get_defhdrs(char *fname){
	mmapbuf *mmb = NULL;
	if(!fname){ // find in default file: ~/$DEFCONF
		const char *homedir;
		if ((homedir = getenv("HOME")) == NULL){
			homedir = getpwuid(getuid())->pw_dir;
		}
		size_t L = strlen(homedir) + strlen(DEFCONF) + 2;
		fname = malloc(L);
		snprintf(fname, L, "%s/%s", homedir, DEFCONF);
		mmb = My_mmap(fname);
		FREE(fname);
	}else mmb = My_mmap(fname);
	if(!mmb) return;
	char *hdrs = strdup(mmb->data);
	My_munmap(mmb);
	char *nl = NULL;
	while((nl = strchr(hdrs, '\n'))){
		*nl = 0;
		list_add_record(&FITS_keys, hdrs, 1);
		hdrs = nl + 1;
	}
	if(*hdrs){ // last line in file didn't have newline
		list_add_record(&FITS_keys, hdrs, 1);
	}
}

/**
 * Add headers from command line parameters
 * something like `apogee_control <parameters> filename "KEY1 = VAL1 / comment" "KEYn = VALn / comment"
 */
void add_morehdrs(int argc, char **argv){
	int i;
	char buf[FLEN_CARD];
	for(i = 0; i < argc; ++i){ // we should override parameters from default configuration file
		char *dup = strdup(argv[i]);
		char *eq = strchr(dup, '=');
		if(eq){
			*(eq++) = 0;
			KeyList *found = list_find_key(FITS_keys, dup);
			if(found) list_modify_key(FITS_keys, dup, eq, 2); // comments (if exists in new header) will be override too
			else{
				snprintf(buf, FLEN_CARD, "%-8s= %s", dup, eq);
				list_add_record(&FITS_keys, buf, 2);
			}
		}else{ // comment or something else
			list_add_record(&FITS_keys, dup, 2);
		}
		FREE(dup);
	}
}

KeyList *list_get_end(KeyList *list){
	if(!list) return NULL;
	KeyList *first = list;
	if(list->last) return list->last;
	while(list->next) list = list->next;
	first->last = list;
	return list;
}

/**
 * add record to keylist
 * @param list (io) - pointer to root of list or NULL
 * 						if *root == NULL, just created node will be placed there
 * @param rec       - data inserted
 * @return pointer to created node
 */
KeyList *list_add_record(KeyList **list, char *rec, int immutable){
	KeyList *node, *last;
	if((node = (KeyList*) MALLOC(KeyList, 1)) == 0)  return NULL; // allocation error
	node->record = strdup(rec); // insert data
	node->immutable = immutable;
	//DBG("add record %s", rec);
	if(!node->record){
		/// "Не могу скопировать данные"
		WARNX(_("Can't copy data"));
		return NULL;
	}
	if(list){
		if(*list){ // there was root node - search last
			last = list_get_end(*list);
			last->next = node; // insert pointer to new node into last element in list
			(*list)->last = node;
		//	DBG("last node %s", (*list)->last->record);
		}else *list = node;
	}
	return node;
}

// compare keywords from `list` and `keyname`
int compare_keyw(KeyList *list, char *keyname){
	if(!list || !keyname || !list->record) return 0;
	size_t L = strlen(keyname);
	if(strncmp(list->record, keyname, L) == 0){ // key found
		char *ltr = list->record + L;
		while(*ltr == ' ') ++ltr; // omit spaces
		if(*ltr == '=') return 1; // only if there's equal sign after keyname!
	}
	return 0;
}

/**
 * return record with given key or NULL
 */
KeyList *list_find_key(KeyList *list, char *keyname){
	if(!list || !keyname) return NULL;
	do{
		if(compare_keyw(list, keyname)) return list;
		list = list->next;
	}while(list);
	return NULL;
}

/**
 * find value of key
 * @return NULL if not found or strdup`ed value
 */
char *list_find_keyval(KeyList *l, char *key){
	KeyList *list = list_find_key(l, key);
	if(!list) return NULL;
	char *rec = strdup(list->record);
	char *val = strchr(rec, '=');
	*val++ = 0;
	char *com = strchr(val, '/');
	if(com) *com = 0;
	char *retval = strdup(val);
	FREE(rec);
	return retval;
}

int getdoubleval(double *val, KeyList *list, char *key){
	char *v = list_find_keyval(list, key);
	if(!v) return 0;
	*val = strtod(v, NULL);
	FREE(v);
	return 1;
}

/**
 * modify key value
 * return NULL if given key is absent
 */
KeyList *list_modify_key(KeyList *list, char *key, char *newval, int immutable){
	char buf[FLEN_CARD];
	KeyList *rec = list_find_key(list, key);
	if(!rec) return NULL;
	if(rec->immutable > immutable) return NULL; // not modify immutable records
	rec->immutable = immutable;
	newval = strdup(newval);
	char *comnt = strchr(newval, '/');
	if(comnt){
		*(comnt++) = 0;
	}else{
		comnt = strchr(rec->record, '/');
		if(comnt) ++comnt;
	}
	if(comnt){
		snprintf(buf, FLEN_CARD, "%-8s= %-21s/%s", key, newval, comnt);
	}else{
		snprintf(buf, FLEN_CARD, "%-8s= %s", key, newval);
	}
	FREE(rec->record);
	FREE(newval);
	DBG("modify record %s", buf);
	rec->record = strdup(buf);
	return rec;
}

void add_fits_header(int datatype, char *keyname, void *value, char *comment){
	void _ub(char* r, void* p){snprintf(r, FLEN_CARD, "%20hhu", *(unsigned char*)p);}
	void _b(char* r, void* p){snprintf(r, FLEN_CARD, "%20hhd", *(char*)p);}
	void _us(char* r, void* p){snprintf(r, FLEN_CARD, "%20hu", *(unsigned short*)p);}
	void _ui(char* r, void* p){snprintf(r, FLEN_CARD, "%20u", *(unsigned int*)p);}
	void _ul(char* r, void* p){snprintf(r, FLEN_CARD, "%20lu", *(unsigned long*)p);}
	void _s(char* r, void* p){snprintf(r, FLEN_CARD, "%20hd", *(short*)p);}
	void _i(char* r, void* p){snprintf(r, FLEN_CARD, "%20d", *(int*)p);}
	void _l(char* r, void* p){snprintf(r, FLEN_CARD, "%20ld", *(long*)p);}
	void _ll(char* r, void* p){snprintf(r, FLEN_CARD, "%20lld", *(long long*)p);}
	void _f(char* r, void* p){snprintf(r, FLEN_CARD, "%20.8f", *(float*)p);}
	void _d(char* r, void* p){snprintf(r, FLEN_CARD, "%20.8f", *(double*)p);}
	void _fc(char* r, void* p){snprintf(r, FLEN_CARD, "(%.8f, %.8f)",
								((float*)p)[0], ((float*)p)[1]);}
	void _dc(char* r, void* p){snprintf(r, FLEN_CARD, "(%.8f, %.8f)",
								((double*)p)[0], ((double*)p)[1]);}
	void _log(char* r, void* p){snprintf(r, FLEN_CARD, "'%s'", (int*)p ? "true" : "false");}
	void _str(char* r, void* p){snprintf(r, FLEN_CARD, "'%s'", (char*)p);}
	void _unk(char* r, void* p __attribute((unused))){sprintf(r, "unknown datatype");}
	char tmp[FLEN_CARD], res[FLEN_CARD];
	void (*__)(char*, void*);
	switch(datatype){
		case TBIT:
		case TBYTE: __ = _ub; break;
		case TSBYTE: __ = _b; break;
		case TUSHORT: __ = _us; break;
		case TUINT: __ = _ui; break;
		case TULONG: __ = _ul; break;
		case TSHORT: __ = _s; break;
		case TINT: __ = _i; break;
		case TLONG: __ = _l; break;
		case TLONGLONG: __ = _ll; break;
		case TFLOAT: __ = _f; break;
		case TDOUBLE: __ = _d; break;
		case TCOMPLEX: __ = _fc; break;
		case TDBLCOMPLEX: __ = _dc; break;
		case TLOGICAL: __ = _log; break;
		case TSTRING: __ = _str; break;
		default: __ = _unk;
	}
	__(res, value);
	KeyList *rec = list_find_key(FITS_keys, keyname);
	if(rec){
		if(comment){
			if(strlen(res) < 21)
				snprintf(tmp, FLEN_CARD, "%-21s / %s", res, comment);
			else
				snprintf(tmp, FLEN_CARD, "%s / %s", res, comment);
		}else snprintf(tmp, FLEN_CARD, "%s", res);
		list_modify_key(FITS_keys, keyname, tmp, 0);
	}else{
		if(strlen(res) < 21)
			snprintf(tmp, FLEN_CARD, "%-8s= %-20s", keyname, res);
		else
			snprintf(tmp, FLEN_CARD, "%-8s=%s", keyname, res);
		snprintf(res, FLEN_CARD, "%s / %s", tmp, comment);
		list_add_record(&FITS_keys, res, 0);
	}
}

/**
 * free list memory & set it to NULL
 */
void list_free(KeyList **list){
	KeyList *node = *list, *next;
	if(!list || !*list) return;
	do{
		next = node->next;
		FREE(node->record);
		free(node);
		node = next;
	}while(node);
	*list = NULL;
}

void free_fits_list(){ list_free(&FITS_keys); }
/*
void list_print(KeyList *list){
	while(list){
		printf("%s\n", list->record);
		list = list->next;
	}
}
void show_list(){
	list_print(FITS_keys);
}
*/

void write_list(fitsfile *fp){
	if(FITS_keys){ // there's keys
		KeyList *records = FITS_keys;
		while(records){
			char *rec = records->record;
			records = records->next;
			if(strncmp(rec, "SIMPLE", 6) == 0 || strncmp(rec, "EXTEND", 6) == 0) // key "file does conform ..."
				continue;
				// comment of obligatory key in FITS head
			else if(strncmp(rec, "COMMENT   FITS", 14) == 0 || strncmp(rec, "COMMENT   and Astrophysics", 26) == 0)
				continue;
			else if(strncmp(rec, "NAXIS", 5) == 0 || strncmp(rec, "BITPIX", 6) == 0) // NAXIS, NAXISxxx, BITPIX
				continue;
			if(!test_headers){
				int status = 0;
				fits_write_record(fp, rec, &status);
				if(status) fits_report_error(stderr, status);
			}else
				printf("%s\n", rec);
		}
	}
}

static int wcs_rdy = 0;
/**
 * Check if user tell some information about WCS and add it into headers
 */
void check_wcs(){
	double cd = -5e5, crot = -5e5, rot0 = -5e5;
	char buf[FLEN_CARD];
	// first correct scale: user value SCALE will fix parameter imscale
	if(imscale < 0.) getdoubleval(&imscale, FITS_keys, "SCALE");
	if(imscale < 0.){
		imscale = 180.*3600./M_PI / TELFOCUS / 1000000. * sqrt(pixX*pixY); // default system value
	}
	snprintf(buf, FLEN_CARD, "%.4f x %.4f", imscale * hbin, imscale * vbin);
	WRITEKEY(TSTRING, "IMSCALE", buf, "image scale (''/Pix x ''/Pix)");

	int cnt = getdoubleval(&crval1, FITS_keys, "CRVAL1");
	cnt += getdoubleval(&crval2, FITS_keys, "CRVAL1");
	cnt += getdoubleval(&crpix1, FITS_keys, "CRPIX1");
	cnt += getdoubleval(&crpix2, FITS_keys, "CRPIX2");
	cnt += getdoubleval(&cd, FITS_keys, "CD1_1");
	cnt += getdoubleval(&crot, FITS_keys, "CROTA2");
	cnt += getdoubleval(&rot0, FITS_keys, "ROT0");
	if(!cnt) return;
	wcs_rdy = 1;
	int wcs = 2;
	WRITEKEY(TINT, "WCSAXIS", &wcs, "Number of WCS axes");
	WRITEKEY(TSTRING, "CTYPE1", "RA---TAN", "RA-Gnomic projection");
	WRITEKEY(TSTRING, "CUNIT1", "deg", "RA units -  degrees");
	WRITEKEY(TSTRING, "CTYPE2", "DEC---TAN", "Decl-Gnomic projection");
	WRITEKEY(TSTRING, "CUNIT2", "deg", "Decl units -  degrees");
	// CRVAL1  =  / RA of reference pixel
	if(crval1 > -4e-5)
		WRITEKEY(TDOUBLE, "CRVAL1", &crval1, "RA of reference pixel");
	else crval1 = 0;
	// CRVAL2  =  / Decl of reference pixel
	if(crval2 > -4e-5)
		WRITEKEY(TDOUBLE, "CRVAL2", &crval2, "Decl of reference pixel");
	else crval2 = 0;
	//CRPIX1  = / X reference pixel
	if(crpix1 > -4e-5)
		WRITEKEY(TDOUBLE, "CRPIX1", &crpix1, "X reference pixel");
	else crpix1 = 0;
	//CRPIX2  = / Y reference pixel
	if(crpix2 > -4e-5)
		WRITEKEY(TDOUBLE, "CRPIX2", &crpix2, "Y reference pixel");
	else crpix2 = 0;
	cnt = 0;
	if(cd > -4e5){
		++cnt;
		WRITEKEY(TDOUBLE, "CD1_1", &cd, "rotation matrix coefficient [1,1]");
		CD[0][0] = cd;
	}
	if(getdoubleval(&cd, FITS_keys, "CD1_2")){
		++cnt;
		WRITEKEY(TDOUBLE, "CD1_2", &cd, "rotation matrix coefficient [1,2]");
		CD[0][1] = cd;
	}
	if(getdoubleval(&cd, FITS_keys, "CD2_1")){
		++cnt;
		WRITEKEY(TDOUBLE, "CD2_1", &cd, "rotation matrix coefficient [2,1]");
		CD[1][0] = cd;
	}
	if(getdoubleval(&cd, FITS_keys, "CD2_2")){
		++cnt;
		WRITEKEY(TDOUBLE, "CD2_2", &cd, "rotation matrix coefficient [2,2]");
		CD[1][1] = cd;
	}
	if(cnt == 4) return;
	// no coefficients - use CROTA & CDELT
	cnt = 0;
	if(crot > -4e5){
		++cnt;
		DBG("crot: %g", crot);
		WRITEKEY(TDOUBLE, "CROTA2", &crot, "North rotation angle");
	}
	if(getdoubleval(&crot, FITS_keys, "CDELT1")){
		++cnt;
		WRITEKEY(TDOUBLE, "CDELT1", &crot, "X axis scale");
		getdoubleval(&crot, FITS_keys, "CDELT2");
		WRITEKEY(TDOUBLE, "CDELT2", &crot, "Y axis scale"); // use CDELT1 if user omits CDELT2
	}
	if(cnt == 2) return;
	// still no coefficients - try to calculate CDx_x by user data
	double parangle, rotangle;
	if(rot0 < -4e5) return; // no values
	if(!getdoubleval(&parangle, FITS_keys, "PARANGLE")) return;
	if(!getdoubleval(&rotangle, FITS_keys, "VAL_P")) return;
	double s, c, scx = imscale / 3600. * hbin, scy = imscale / 3600. * vbin;
	if(rot0 < 0){ // left-handed
		crot = (-rot0 + parangle - rotangle)*M_PI/180;
		sincos(crot, &s, &c);
		CD[0][0] = -scx * c; CD[0][1] = scy * s;
		CD[1][0] = scx * s; CD[1][1] = scy * c;
	}else{ // right-handed
		crot = (rot0 - parangle + rotangle)*M_PI/180;
		sincos(crot, &s, &c);
		CD[0][0] = scx * c; CD[0][1] = -scy * s;
		CD[1][0] = scx * s; CD[1][1] = scy * c;
	}
	WRITEKEY(TDOUBLE, "CD1_1", &CD[0][0], "rotation matrix coefficient [1,1]");
	WRITEKEY(TDOUBLE, "CD1_2", &CD[0][1], "rotation matrix coefficient [1,2]");
	WRITEKEY(TDOUBLE, "CD2_1", &CD[1][0], "rotation matrix coefficient [2,1]");
	WRITEKEY(TDOUBLE, "CD2_2", &CD[1][1], "rotation matrix coefficient [2,2]");
}

// function for events.c - recalculate coordinates on image into WCS
void calc_coords(float x, float y, double *alpha, double *delta){
	double ra, dec, dx = x - crpix1, dy = y - crpix2;
	if(!wcs_rdy){*alpha = 5e6; *delta = 5e6; return; }
	ra = crval1 + dx * CD[0][0] + dy * CD[0][1];
	dec = crval2 + dx * CD[1][0] + dy * CD[1][1];
	DBG("ra=%g, dec=%g", ra,dec);
	if(alpha) *alpha = ra * 3600.; // hrsec
	if(delta) *delta = dec * 3600.; // arcsec
	DBG("ra=%g, dec=%g", *alpha, *delta);
}
