/*
 * camtools.c
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

#include "takepic.h"
#include "camtools.h"
#include "usage.h"
#include "macros.h"

#ifdef USE_BTA
#include "bta_print.h"
#endif

unsigned short max=0, min=65535; // extremums of current image
double avr, std; // average value and standard deviation

/*
 * Fake util to show FITS keys in stdout
 * when test_headers == 1
 */
void print_fits_header(fitsfile *fptr __attribute((unused)), int datatype,
						char *keyname, void *value, char *comment){
	void _ub(char* r, void* p){snprintf(r, 80, "%hhu", *(unsigned char*)p);}
	void _b(char* r, void* p){snprintf(r, 80, "%hhd", *(char*)p);}
	void _us(char* r, void* p){snprintf(r, 80, "%hu", *(unsigned short*)p);}
	void _ui(char* r, void* p){snprintf(r, 80, "%u", *(unsigned int*)p);}
	void _ul(char* r, void* p){snprintf(r, 80, "%lu", *(unsigned long*)p);}
	void _s(char* r, void* p){snprintf(r, 80, "%hd", *(short*)p);}
	void _i(char* r, void* p){snprintf(r, 80, "%d", *(int*)p);}
	void _l(char* r, void* p){snprintf(r, 80, "%ld", *(long*)p);}
	void _ll(char* r, void* p){snprintf(r, 80, "%lld", *(long long*)p);}
	void _f(char* r, void* p){snprintf(r, 80, "%g", *(float*)p);}
	void _d(char* r, void* p){snprintf(r, 80, "%g", *(double*)p);}
	void _fc(char* r, void* p){snprintf(r, 80, "(%.4g, %.4g)",
								((float*)p)[0], ((float*)p)[1]);}
	void _dc(char* r, void* p){snprintf(r, 80, "(%.4g, %.4g)",
								((double*)p)[0], ((double*)p)[1]);}
	void _log(char* r, void* p){snprintf(r, 80, "'%s'", (int*)p ? "true" : "false");}
	void _str(char* r, void* p){snprintf(r, 80, "'%s'", (char*)p);}
	void _unk(char* r, void* p __attribute((unused))){sprintf(r, "unknown datatype");}
	char tmp[81], res[81];
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
	if(strlen(res) < 20)
		snprintf(tmp, 80, "%-8s = %-20s", keyname, res);
	else
		snprintf(tmp, 80, "%-8s = %s", keyname, res);
	snprintf(res, 80, "%s / %s", tmp, comment);
	printf("%s\n", res);
}

/*
 * set fun speed
 * if user set option fan-speed=F, then speed would be set to F
 * if onoff == FALSE, try to set speed according to hot Peltier temperature
 * if onoff == TRUE, set speed=3, if cooler is on, else set speed=0
 */
void AutoadjustFanSpeed(bool onoff){
	int Mode = 3, curMode, stat;
	double temp, HotTemp;
	stat = ApnGlueGetTemp(&temp);
	curMode = ApnGlueGetFan();
	HotTemp = ApnGlueGetHotTemp();
	if(fanspd != -1){
		ApnGlueSetFan(fanspd);
	}
	if(HotTemp < -30.){ // -255. - there's no thermometer on hot side
		// "В вашем светоприемнике нет \"горячего\" термометра"
		info(_("Your camera have no hot temperature sensor\n"));
		return;
	}
	if(onoff){ // quit, set speed according cooler status
		if(!stat && HotTemp < 30.) Mode = 0; // cooler is off -> turn fans off
	}else{
		if(HotTemp < 20.) Mode = 0;
		else if(HotTemp < 25.) Mode = 1;
		else if(HotTemp < 30.) Mode = 2;
	}
	if(Mode == curMode) return;
	ApnGlueSetFan(Mode);
	if(ApnGlueGetFan() == Mode)
		// "Устанавливаю скорость вращения вентиляторов в %d\n"
		info(_("Set fan speed %d\n"), Mode);
	else
		// "Не могу сменить скорость вращения вентиляторов!\n"
		ERR("Can't set fan speed\n");
}

double printCoolerStat(double *t_ext){
	double temp, extemp, settemp;
	char *s;
	int stat = ApnGlueGetTemp(&temp);
	extemp = ApnGlueGetHotTemp();
	if(ApnGlueReadSetPoint(&settemp, NULL)){
		// "Целевая температура: %g\n"
		info(_("Cooler setpoint: %g\n"), settemp);
	}
	switch(stat){
		case 0:
			// "Холодильник отключен"
			s = _("Cooler is off"); break;
		case 1:
			// "Установка заданной температуры"
			s = _("Go to setpoint"); break;
		case 2:
			// "Поддержание заданной температуры"
			s = _("Keeping setpoint"); break;
		default:
			// "Статус неизвестен"
			s = _("Unknown status");
	}
	info("%s, T=%g, Thot=%g\n", s, temp, extemp);
	if(t_ext) *t_ext = extemp;
	return temp;
}

void print_stat(unsigned short *img, long size, FILE *f){
	long i, Noverld = 0L, N = 0L;
	double pv, sum=0., sum2=0., sz = (double)size, tres;
	unsigned short *ptr = img, val, valoverld;
	max = 0; min = 65535;
	if(twelveBit) min = 4095;
	valoverld = min - 5;
	for(i = 0; i < size; i++, ptr++){
		val = *ptr;
		pv = (double) val;
		sum += pv;
		sum2 += (pv * pv);
		if(max < val) max = val;
		if(min > val) min = val;
		if(val >= valoverld) Noverld++;
	}
	// "Статистика по изображению:\n"
	printf(_("Image stat:\n"));
	avr = sum/sz;
	printf("avr = %.1f, std = %.1f, Noverload = %ld\n", avr,
		std = sqrt(fabs(sum2/sz - avr*avr)), Noverld);
	printf("max = %u, min = %u, size = %ld\n", max, min, size);
	if(!f) return;
	// full statistics
	fprintf(f, "%d\t%d\t%.3f\t%.3f\t%ld\t", max, min, avr, std, Noverld);
	Noverld = 0L;
	ptr = img; sum = 0.; sum2 = 0.;
	tres = avr + 3. * std; // max treshold == 3sigma
	for(i = 0; i < size; i++, ptr++){
		val = *ptr;
		pv = (double) val;
		if(pv > tres){
			Noverld++; // now this is an amount of overload pixels
			continue;
		}
		sum += pv;
		sum2 += (pv * pv);
		N++;
	}
	if(N == 0L){
		fprintf(f, "err\n");
		return;
	}
	sz = (double)N;
	avr = sum/sz; std = sqrt(fabs(sum2/sz - avr*avr));
	fprintf(f, "%ld\t%.3f\t%.3f\n", Noverld, avr, std);
	fflush(f);
	printf("Novr3 = %ld\n", Noverld);
}





/*
 * header must include:
 * RATE / Readout rate (KPix/sec)
 * GAIN / gain, e/ADU
 * SEEING / Seeing ('')
 * IMSCALE / Image scale (''/pix x ''/pix)
 * CLOUDS / Clouds (%)
 */


int writefits(char *filename, int width, int height, void *data){
	long naxes[2] = {width, height};//, startTime;
	double tmp = 0.0;
	unsigned short ustmp;
	struct tm *tm_starttime;
	char buf[80];
	time_t savetime = time(NULL);
	fitsfile *fp;
	TRYFITS(fits_create_file, &fp, filename);
	TRYFITS(fits_create_img, fp, USHORT_IMG, 2, naxes);
	// FILE / Input file original name
	WRITEKEY(fp, TSTRING, "FILE", filename, "Input file original name");
	/*
	 * Detector parameters
	 */
	// DETECTOR / detector
	if(camera){
		WRITEKEY(fp, TSTRING, "DETECTOR", camera, "Detector model");
	}
	// SENSOR / sensor
	if(sensor){
		WRITEKEY(fp, TSTRING, "SENSOR", sensor, "Camera sensor model");
	}
	// INSTRUME / Instrument
	if(instrument){
		WRITEKEY(fp, TSTRING, "INSTRUME", instrument, "Instrument");
	}else
		WRITEKEY(fp, TSTRING, "INSTRUME", "direct imaging", "Instrument");
	snprintf(buf, 79, "%.g x %.g", pixX, pixY);
	// PXSIZE / pixel size
	WRITEKEY(fp, TSTRING, "PXSIZE", buf, "Pixel size in mkm");
	// XPIXELSZ, YPIXELSZ -- the same
	WRITEKEY(fp, TDOUBLE, "XPIXELSZ", &pixX, "X pixel size in mkm");
	WRITEKEY(fp, TDOUBLE, "YPIXELSZ", &pixY, "Y pixel size in mkm");
	WRITEKEY(fp, TSTRING, "VIEW_FIELD", viewfield, "Camera field of view");
	// CRVAL1, CRVAL2 / Offset in X, Y
	if(X0) WRITEKEY(fp, TINT, "CRVAL1", &X0, "Offset in X");
	if(Y0) WRITEKEY(fp, TINT, "CRVAL2", &Y0, "Offset in Y");
	if(exptime == 0) sprintf(buf, "bias");
	else if(shutter == 0) sprintf(buf, "dark");
	else if(objtype) strncpy(buf, objtype, 79);
	else sprintf(buf, "object");
	// IMAGETYP / object, flat, dark, bias, scan, eta, neon, push
	WRITEKEY(fp, TSTRING, "IMAGETYP", buf, "Image type");
	// DATAMAX, DATAMIN / Max,min pixel value
	ustmp = ((twelveBit) ? 4095 : 65535);
	WRITEKEY(fp, TUSHORT, "DATAMAX", &ustmp, "Max pixel value");
	ustmp = 0;
	WRITEKEY(fp, TUSHORT, "DATAMIN", &ustmp, "Min pixel value");
	// Some Statistics
	WRITEKEY(fp, TUSHORT, "STATMAX", &max, "Max data value");
	WRITEKEY(fp, TUSHORT, "STATMIN", &min, "Min data value");
	WRITEKEY(fp, TDOUBLE, "STATAVR", &avr, "Average data value");
	WRITEKEY(fp, TDOUBLE, "STATSTD", &std, "Standart deviation of data value");
	// Temperatures
	WRITEKEY(fp, TDOUBLE, "TEMP0", &temperature, "Camera temperature at exp. start (degr C)");
	WRITEKEY(fp, TDOUBLE, "TEMP1", &t_int, "Camera temperature at exp. end (degr C)");
	if(t_ext > -30.) // there's a hot temp sensor
		WRITEKEY(fp, TDOUBLE, "TEMPBODY", &t_ext, "Camera body temperature at exp. end (degr C)");
	tmp = (temperature + t_int) / 2. + 273.15;
	// CAMTEMP / Camera temperature (K)
	WRITEKEY(fp, TDOUBLE, "CAMTEMP", &tmp, "Camera temperature (K)");
	tmp = (double)exptime / 1000.;
	// EXPTIME / actual exposition time (sec)
	WRITEKEY(fp, TDOUBLE, "EXPTIME", &tmp, "actual exposition time (sec)");
	// DATE / Creation date (YYYY-MM-DDThh:mm:ss, UTC)
	strftime(buf, 79, "%Y-%m-%dT%H:%M:%S", gmtime(&savetime));
	WRITEKEY(fp, TSTRING, "DATE", buf, "Creation date (YYYY-MM-DDThh:mm:ss, UTC)");
	tm_starttime = localtime(&expStartsAt);
	/*
	 * startTime = (long)expStartsAt;
	 * strftime(buf, 79, "exposition starts at %d/%m/%Y, %H:%M:%S (local)", tm_starttime);
	 * WRITEKEY(fp, TLONG, "UNIXTIME", &startTime, buf);
	 */
	strftime(buf, 79, "%Y-%m-%dT%H:%M:%S", tm_starttime);
	// DATE-OBS / DATE OF OBS.
	WRITEKEY(fp, TSTRING, "DATE-OBS", buf, "DATE OF OBS. (YYYY-MM-DDThh:mm:ss, local)");
	/*
	 * // START / Measurement start time (local) (hh:mm:ss)
	 * strftime(buf, 79, "%H:%M:%S", tm_starttime);
	 * WRITEKEY(fp, TSTRING, "START", buf, "Measurement start time (hh:mm:ss, local)");
	 */
	// OBJECT  / Object name
	if(objname){
		WRITEKEY(fp, TSTRING, "OBJECT", objname, "Object name");
	}
	// BINNING / Binning
	if(hbin != 1 || vbin != 1){
		snprintf(buf, 79, "%d x %d", hbin, vbin);
		WRITEKEY(fp, TSTRING, "BINNING", buf, "Binning (hbin x vbin)");
	}
	WRITEKEY(fp, TINT, "XBIN", &hbin, "Horizontal binning");
	WRITEKEY(fp, TINT, "YBIN", &vbin, "Vertical binning");
	// OBSERVER / Observers
	if(observers){
		WRITEKEY(fp, TSTRING, "OBSERVER", observers, "Observers");
	}
	// PROG-ID / Observation program identifier
	if(prog_id){
		WRITEKEY(fp, TSTRING, "PROG-ID", prog_id, "Observation program identifier");
	}
	// AUTHOR / Author of the program
	if(author){
		WRITEKEY(fp, TSTRING, "AUTHOR", author, "Author of the program");
	}
#ifdef USE_BTA
	write_bta_data(fp);
#endif
	TRYFITS(fits_write_img, fp, TUSHORT, 1, width * height, data);
	TRYFITS(fits_close_file, fp);
	return 0;
}

#ifdef USEPNG
int writepng(char *filename, int width, int height, void *data){
	int err;
	FILE *fp = NULL;
	png_structp pngptr = NULL;
	png_infop infoptr = NULL;
	void *row;
	if ((fp = fopen(filename, "wb")) == NULL){
		err = -errno;
		goto done;
	}
	if ((pngptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
							NULL, NULL, NULL)) == NULL){
		err = -ENOMEM;
		goto done;
	}
	if ((infoptr = png_create_info_struct(pngptr)) == NULL){
		err = -ENOMEM;
		goto done;
	}
	png_init_io(pngptr, fp);
	png_set_compression_level(pngptr, Z_BEST_COMPRESSION);
	png_set_IHDR(pngptr, infoptr, width, height, 16, PNG_COLOR_TYPE_GRAY,
				PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
				PNG_FILTER_TYPE_DEFAULT);
	png_write_info(pngptr, infoptr);
	png_set_swap(pngptr);
	for(row = data; height > 0; row += width * sizeof(u_int16_t), height--)
		png_write_row(pngptr, row);
	png_write_end(pngptr, infoptr);
	err = 0;
	done:
	if(fp) fclose(fp);
	if(pngptr) png_destroy_write_struct(&pngptr, &infoptr);
	return err;
}
#endif /* USEPNG */

#ifdef USERAW
int writeraw(char *filename, int width, int height, void *data){
	int fd, size, err;
	if ((fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC,
		S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH )) == -1){
		warn("open(%s) failed", filename);
		return -errno;
	}
	size = width * height * sizeof(unsigned short);
	if ((err = write(fd, data, size)) != size){
		warn("write() failed");
		err = -errno;
	}
	else err = 0;
	close(fd);
	return err;
}
#endif // USERAW

#ifdef IMAGEVIEW
// here are the functions for converting gray (GLubyte) image into colour buffer

/**
 * Convert gray (unsigned short) into RGB components (GLubyte)
 * @argument L   - gray level
 * @argument rgb - rgb array (GLubyte [3])
 */
void gray2rgb(double gray, GLubyte *rgb){
	int i = gray * 4.;
	double x = (gray - (double)i * .25) * 4.;
	GLubyte r = 0, g = 0, b = 0;
	//r = g = b = (gray < 1) ? gray * 256 : 255;
	switch(i){
		case 0:
			g = (GLubyte)(255. * x);
			b = 255;
		break;
		case 1:
			g = 255;
			b = (GLubyte)(255. * (1. - x));
		break;
		case 2:
			r = (GLubyte)(255. * x);
			g = 255;
		break;
		case 3:
			r = 255;
			g = (GLubyte)(255. * (1. - x));
		break;
		default:
			r = 255;
	}
	*rgb++ = r;
	*rgb++ = g;
	*rgb   = b;
}


double linfun(double arg){ return arg; } // bung for PREVIEW_LINEAR
double logfun(double arg){ return log(1.+arg); } // for PREVIEW_LOG
double (*colorfun)(double) = linfun; // default function to convert color

void change_colorfun(colorfn_type f){
	switch (f){
		case COLORFN_LINEAR:
			colorfun = linfun;
		break;
		case COLORFN_LOG:
			colorfun = logfun;
		break;
		default: // sqrt
			colorfun = sqrt;
	}
}

/**
 * Convert given FITS image src into RGB array dst
 * for OpenGL texture
 * @param w, h - image size (in pixels)
 * array dst should have size w*h*3
 */
void convert_grayimage(unsigned short *src, GLubyte *dst, int w, int h){
	FNAME();
	int  x, y, S = w*h;
	unsigned short *ptr = src;
	double avr, wd, max, min;
	avr = max = min = (double)*src;
	for(x = 1; x < S; x++, ptr++){
		double pix = (double) *ptr;
		if(pix > max) max = pix;
		if(pix < min) min = pix;
		avr += pix;
	}
	avr /= (double)S;
	wd = max - min;
	avr = (avr - min) / wd;	// normal average by preview
	DBG("stat: avr=%f wd=%f max=%f min=%f", avr, wd, max, min);
//	avr = -log(avr);		// scale factor
//	if(avr > 1.) wd /= avr;
	if(avr < 0.6) wd *= avr + 0.2;
	DBG("now wd is %f", wd);
	for(y = 0; y < h; y++){
		for(x = 0; x < w; x++, dst += 3, src++){
			gray2rgb(colorfun((*src - min) / wd), dst);
		}
	}
}

#endif // IMAGEVIEW
