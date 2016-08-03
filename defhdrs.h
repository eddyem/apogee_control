/*
 * defhdrs.h
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
#pragma once
#ifndef __DEFHDRS_H__
#define __DEFHDRS_H__

extern double crval1, crval2, crpix1, crpix2, CD[2][2]; // global coefficients for WCS

typedef struct klist_{
	char *record;
	int immutable; // not modify record if old value of `immutable` > new value
	struct klist_ *next;
	struct klist_ *last;
} KeyList;

void list_free(KeyList **list);
void free_fits_list();
KeyList *list_add_record(KeyList **list, char *rec, int immutable);
KeyList *list_find_key(KeyList *list, char *key);
KeyList *list_modify_key(KeyList *list, char *key, char *newval, int immutable);
KeyList *list_get_end(KeyList *list);
/*
void list_print(KeyList *list);
void show_list();
*/
void add_fits_header(int datatype, char *keyname, void *value, char *comment);

void write_list(fitsfile *fp);
void get_defhdrs(char *fname);
void add_morehdrs(int argc, char **argv);

void check_wcs();

#endif // __DEFHDRS_H__
