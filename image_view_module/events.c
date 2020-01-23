/*
 * events.c
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
#include "events.h"
#include "imageview.h"
#include "macros.h"

static char buf[1024];
static char *time_asc(double t){
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

static char *angle_asc(double a){
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


/**
 * manage pressed keys & menu items
 */
void processKeybrd(unsigned char key, int mod, int win_GL_ID, _U_ int x, _U_ int y){
	windowData *win = searchWindow_byGLID(win_GL_ID);
	if(!win) return;
	if((mod == GLUT_ACTIVE_CTRL) && (key == 'q' || key == 17)) exit(0); // ctrl + q
	switch(key){
		case '1': // return zoom to 1 & image to 0
			win->zoom = 1;
			win->x = 0; win->y = 0;
		break;
		case 27:
			destroyWindow_async(win_GL_ID);
		break;
/*		case 'x':
		break;*/
		case 'Z':
			win->zoom *= 1.1;
			calc_win_props(win, NULL, NULL);
			DBG("zoom: %f", win->zoom);
		break;
		case 'z':
			win->zoom /= 1.1;
			calc_win_props(win, NULL, NULL);
		break;
	}
}

/*
 *	Process keyboard
 */
void keyPressed(unsigned char key,
				_U_ int x, _U_ int y){
	int mod = glutGetModifiers();
	int window = glutGetWindow();
	//mod: GLUT_ACTIVE_SHIFT, GLUT_ACTIVE_CTRL, GLUT_ACTIVE_ALT; result is their sum
	//DBG("Key pressed. mod=%d, keycode=%d, point=(%d,%d)\n", mod, key, x,y);
	if((mod == GLUT_ACTIVE_CTRL) && (key == 'q' || key == 17)) exit(0); // ctrl + q
	processKeybrd(key, mod, window, x, y);
}

void keySpPressed(_U_ int key, _U_ int x, _U_ int y){
//	int mod = glutGetModifiers();
	DBG("Sp. key pressed. mod=%d, keycode=%d, point=(%d,%d)\n", glutGetModifiers(), key, x,y);
}

// in defhdrs.c
extern void calc_coords(float x, float y, double *alpha, double *delta);

int oldx, oldy;         // coordinates when mouse was pressed
int movingwin = 0; // ==1 when user moves image by middle button
void mousePressed(_U_ int key, _U_ int state, _U_ int x, _U_ int y){
// key: GLUT_LEFT_BUTTON, GLUT_MIDDLE_BUTTON, GLUT_RIGHT_BUTTON
// state: GLUT_UP, GLUT_DOWN
	int window = glutGetWindow(), mod = glutGetModifiers();
	windowData *win = searchWindow_byGLID(window);
	if(!win) return;
	if(state == GLUT_DOWN){
		oldx = x; oldy = y;
		float X,Y;
		double RA, D;
		conv_mouse_to_image_coords(x,y,&X,&Y,win);
		DBG("press in (%d, %d) == (%f, %f) on image; mod == %d", x,y,X,Y, mod);
		calc_coords(X, Y, &RA, &D);
		printf("x = %f, y= %f", X, Y);
		/// "Задайте WCS параметры, по крайней мере ROT0, CRPIX1 и CRPIX2"
		if(RA > 4e6){
			printf("\n");
			WARNX(_("Give WCS parameters, at least ROT0, CRPIX1 and CRPIX2"));
		}else{
			printf(", ra = %s", angle_asc(RA));
			printf(", dec=%s\n", time_asc(D));
		}
		if(key == GLUT_MIDDLE_BUTTON) movingwin = 1;
		if(key == 3){ // wheel UP
			if(mod == 0) win->y += 10.*win->zoom; // nothing pressed - scroll up
			else if(mod == GLUT_ACTIVE_SHIFT) win->x -= 10.*win->zoom; // shift pressed - scroll left
			else if(mod == GLUT_ACTIVE_CTRL) win->zoom *= 1.1; // ctrl+wheel up == zoom+
		}else if(key == 4){ // wheel DOWN
			if(mod == 0) win->y -= 10.*win->zoom; // nothing pressed - scroll down
			else if(mod == GLUT_ACTIVE_SHIFT) win->x += 10.*win->zoom; // shift pressed - scroll right
			else if(mod == GLUT_ACTIVE_CTRL) win->zoom /= 1.1; // ctrl+wheel down == zoom-
		}
		calc_win_props(win, NULL, NULL);
	}else{
		movingwin = 0;
	}
}

void mouseMove(_U_ int x, _U_ int y){
	int window = glutGetWindow();
	windowData *win = searchWindow_byGLID(window);
	if(!win) return;
	if(movingwin){
		float X, Y, nx, ny;
		float a = win->Daspect;
		X = (x - oldx) * a; Y = (y - oldy) * a;
		nx = win->x + X;
		ny = win->y - Y;
		win->x = nx;
		win->y = ny;
		oldx = x;
		oldy = y;
		calc_win_props(win, NULL, NULL);
	}
}

void menuEvents(int opt){
	FNAME();
	int window = glutGetWindow();
	if(opt == 'q') exit(0);
	processKeybrd((unsigned char)opt, 0, window, 0, 0);
}

/**
 * winID - inner !!!
 */
void createMenu(int GL_ID){
	FNAME();
	windowData *win;
	win = searchWindow_byGLID(GL_ID);
	glutSetWindow(GL_ID);
	if(win->menu) glutDestroyMenu(win->menu);
	win->menu = glutCreateMenu(menuEvents);
	DBG("created menu %d\n", win->menu);
	glutAddMenuEntry("Quit (ctrl+q)", 'q');
	glutAddMenuEntry("Close this window (ESC)", 27);
	glutAddMenuEntry("Restore zoom (1)", '1');
//	glutAddMenuEntry("Make exposition (x)", 'x');
	glutAttachMenu(GLUT_RIGHT_BUTTON);
}

