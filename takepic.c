/*
 * takepic.c
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

#include <dirent.h>
#include <float.h>
#include <linux/usbdevice_fs.h>
#include <locale.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <usb.h>

#ifdef USE_BTA
#include "bta_print.h"
#endif
#include "camtools.h"
#include "defhdrs.h"
#ifdef IMAGEVIEW
#include "imageview.h"
#endif
#include "macros.h"
#include "takepic.h"
#include "usage.h"


#define BUFF_SIZ 4096
#define PROC_BASE "/proc"

#define TMBUFSIZ 40 // time string buffer length

static char *pidfilename = "/tmp/apogee_control.pid"; // pidfile

static char tm_buf[TMBUFSIZ];	// time string buffer

char *camera = NULL, *sensor = NULL, viewfield[80];
double pixX, pixY; // pixel size

int test_headers = 0; // don't even try to open camera device, just show FITS keys

double t_ext, t_int;	// hot & cold side temperatures
time_t expStartsAt;		// time of exposition start (time_t)

static int itime(){ // time in seconds to count intervals
    struct timeval ct;
    gettimeofday(&ct, NULL);
    return (ct.tv_sec);
}
static int time0;
static int ltime(){ // time since last time0 reinitialising
    return(itime()-time0);
}

// return string with current date/time
static size_t curtime(char *s_time){
    time_t tm = time(NULL);
    size_t s = strftime(s_time, TMBUFSIZ, "%d/%m/%Y,%H:%M:%S", localtime(&tm));
    if(!s) info("curtime() error");
    return s;
}

// find the first non-exists filename & put it into buff
static int check_filename(char *buff, char *outfile, char *ext){
    struct stat filestat;
    int num;
    for(num = 1; num < 10000; num++){
        if(snprintf(buff, BUFF_SIZ, "%s_%04d.%s", outfile, num, ext) < 1)
            return 0;
        if(stat(buff, &filestat)) // OK, file not exists
            return 1;
    }
    return 0;
}

static void reset_usb_port(int pid, int vid){
    int fd, rc;
    char buf[FILENAME_MAX*3], *d = NULL, *f = NULL;
    struct usb_bus *bus;
    struct usb_device *dev;
    int found = 0;
    usb_init();
    usb_find_busses();
    usb_find_devices();
    for(bus = usb_busses; bus && !found; bus = bus->next) {
        for(dev = bus->devices; dev && !found; dev = dev->next) {
            if (dev->descriptor.idVendor == vid && dev->descriptor.idProduct == pid){
                found = 1;
                d = bus->dirname;
                f = dev->filename;
            }
        }
    }
    if(!found){
        // "���������� �� �������"
        ERR(_("Device not found"));
        return;
    }
    DBG("found camera device, reseting");
    snprintf(buf, sizeof(buf), "/dev/bus/usb/%s/%s", d,f);
    fd = open(buf, O_WRONLY);
    if (fd < 0){
        // "�� ���� ������� ���� ���������� %s: %s"
        ERR(_("Can't open device file %s: %s"), buf, strerror(errno));
        return;
    }
    info("Resetting USB device %s", buf);
    rc = ioctl(fd, USBDEVFS_RESET, 0);
    if (rc < 0) {
        // "�� ���� �������� ioctl"
        perror(_("Error in ioctl"));
        return;
    }
    close(fd);
}

// quit by signal
static void signals(int sig){
    int u;
    // "������� ������ %d, ����������.\n"
    if(sig){
        signal(sig, signals);
        printf(_("Get signal %d, quit.\n"), sig);
    }
    u = unlink(pidfilename);
    // "�� ���� ������� PID-����"
    if(u == -1) perror(_("Can't delete PIDfile"));
    ApnGlueWheelClose();
    DBG("wheel closed");
    ApnGlueExpAbort(); // this function stubs all!
    DBG("exp aborted");
    ApnGlueClose();
    DBG("device closed");
    #ifdef IMAGEVIEW
    clear_GL_context();
    #endif
    exit(sig);
}

static int sigcounter = 3;
static void cnt_signals(int sig){
    --sigcounter;
    if(sigcounter){
        // "������� Ctrl+C ��� %d ���[�], ����� �������� ����������\n"
        printf(_("Press Ctrl+C %d time[s] more to interrupt reading\n"), sigcounter);
        signal(SIGINT, cnt_signals);
    }else{
        signals(sig);
    }
}

// Try to ignore all signals exept SIGINT, which is counted
static void ignore_signals(){
    sigcounter = 3;
    for(int _=0; _<256; _++)
        signal(_, SIG_IGN);
    signal(SIGINT, cnt_signals);
}
// We must take care about signals to avoid ctrl+C problems
static void catch_signals(){
    for(int _=0; _<256; _++)
        signal(_, SIG_IGN);
    signal(SIGTERM, signals); // kill (-15) - quit
    signal(SIGHUP, signals);  // hup - quit
    signal(SIGINT, signals);  // ctrl+C - quit
    signal(SIGQUIT, signals); // ctrl+\ - quit
    //signal(SIGTSTP, SIG_IGN); // ignore ctrl+Z
}
/*
void restore_signals(){
    sigcounter = 3;
    for(int _=0; _<256; _++)
        signal(_, SIG_DFL);
    unlink(pidfilename);
}*/

// check for running process
static void check4running(){
    // get process name from /proc
    int readname(char *name, pid_t pid){
        char *pp = name, byte, path[256];
        int cntr = 0, file;
        snprintf (path, 255, PROC_BASE "/%d/cmdline", pid);
        file = open(path, O_RDONLY);
        if(file == -1) return 0;
        do{
            if(read(file, &byte, 1) != 1) break;
            if (byte != '/') *pp++ = byte;
            else pp = name;
        }
        while(byte != EOF && byte != 0 && cntr++ < 255);
        name[255] = 0;
        close(file);
        return 1;
    }
    DIR *dir;
    FILE* pidfile;
    struct dirent *de;
    struct stat s_buf;
    pid_t pid, self, run = 0;
    char name[256], myname[256];
    if (!(dir = opendir(PROC_BASE))){ // open /proc dir
        perror(PROC_BASE);
        exit(1);
    }
    self = getpid(); // self PID
    if(stat(pidfilename, &s_buf) == 0){ // PID file exists
        pidfile = fopen(pidfilename, "r");
        if(1 == fscanf(pidfile, "%d", &run)){ // get PID of (possibly) running process
            if(readname(name, run) && strncmp(name, myname, 255) == 0){
            // "��������� ���������� ������� (pid=%d), �����.\n"
            ERR(_("\nFound running process (pid=%d), exit.\n"), run);
            exit(7);
            }
        }
        fclose(pidfile);
    }
    // there's no PID file or it's old
    readname(myname, self); // self name
    while ((de = readdir (dir)) != NULL){ // scan /proc
        if (!(pid = (pid_t) atoi (de->d_name)) || pid == self)
            continue;
        readname(name, pid); // get process name
        if(strncmp(name, myname, 255) == 0){
            // "��������� ���������� ������� (pid=%d), �����.\n"
            ERR(_("\nFound running process (pid=%d), exit.\n"), pid);
            exit(7);
        }
    }
    closedir(dir);
    unlink(pidfilename); // try to remove pidfilename
    DBG("my PID: %d\n", self);
    pidfile = fopen(pidfilename, "w");
    fprintf(pidfile, "%d\n", self); // write into pidfilename the pid
    fclose(pidfile);
}

// Work with turret
static void parse_turret_args(){
    int maxPos, curPos;
    if(ApnGlueWheelOpen(Nwheel, Apn_Filter_FW50_7S)){
        // "�� ���� ������� ������"
        ERR(_("Can't open turret"));
        return;
    }
    DBG("get max pos");
    maxPos = ApnGlueWheelGetMaxPos();
    curPos = ApnGlueWheelGetPos();
    // "������������ (MAX) � ������� (CUR) ������� ������:\n"
    printf(_("Turret MAXimum position and CURrent position:\n"));
    printf("MAX=%d, CUR=%d\n", maxPos, curPos);
    if(wheelPos == 0) goto wheelret;
    if(wheelPos > maxPos){
        // "��������� ������� ������ ������������"
        ERR(_("Required position greater then max"));
        goto wheelret;
    }
    if(ApnGlueWheelSetPos(wheelPos)){
        // "�� ���� ����������� ������"
        ERR(_("Can't move turret"));
        goto wheelret;
    }
    if(ApnGlueWheelGetStatus()){
        // "��������� ���������� ����������� ������ "
        printf(_("Wait, while turret is moving "));
        while(ApnGlueWheelGetStatus()){
            printf("."); fflush(NULL);
            sleep(1);
        }
    }
    printf("\n");
wheelret:
    ApnGlueWheelClose();
}

#ifdef IMAGEVIEW
static void change_displayed_image(unsigned short *buf, windowData *win){
    FNAME();
    if(!win) return;
    pthread_mutex_lock(&win->mutex);
    int w = win->image->w, h = win->image->h;
    static GLubyte i = 0;
    GLubyte *raw = win->image->rawdata;
    DBG("image size: %dx%d",w,h);
    convert_grayimage(buf, raw, w, h);
    win->image->changed = 1;
    pthread_mutex_unlock(&win->mutex);
    i++;
}
#endif

int main(int argc, char **argv){
    int i;	//cycles
    int pid = -1, vid = -1; // device pid/vid
    FILE *f_tlog = NULL; // temperature logging file
    FILE *f_statlog = NULL; // stat file
    int roih=0, roiw=0, osh=0, osw=0, binh=0, binw=0, shtr=0; // camera parameters
    char  whynot[BUFF_SIZ]; // temporary buffer for error texts
    int imW=1024, imH=1024; // real (with binning) image size
    unsigned short *buf = NULL; // image buffer
    double mintemp=0.;
#ifdef IMAGEVIEW
    windowData *mainwin = NULL;
    rawimage im;
#endif

    initial_setup(); // setup for macros.c
    parse_args(argc, argv);

    catch_signals();
    // Check whether there's no concurrents
    check4running();

    if(test_headers){
        writefits(NULL, imW, imH, NULL);
        return(0);
    }
    if(fake) test_headers = 1;
// Begin of non-fake block ------->
    if(!fake){
    // Turret block
    if(open_turret) parse_turret_args();
    if(only_turret){
        // "�� ������ ��������� ����������, ����������"
        info(_("Expose parameters aren't specified, exit"));
        goto returning;
    }

    // And camera block
    // First - open camera devise
    if(subnet) ApnGlueSetSubnet(subnet); // set subnet name if there's an ethernet camera
    if(cammsgid) ApnGlueSetMsgId(cammsgid); // set msgid given by user
    if(ApnGlueOpen(Ncam)){
        // "�� ���� ������� ������, ��������"
        ERR(_("Can't open camera device, exit"));
        if(ApnGlueIsEthernet()){
            // "����������� ������������� ������ ����� ���-���������"
            info(_("Try to reboot camera from web-interface"));
        }
        exit(9);
    }
DBG("open %d", Ncam);
   // ApnGlueGetName(&sensor, &camera);
   // camera = strdup(camera); sensor = strdup(sensor); - bug. it shows a shit!
    // "���������� ������ '%s' � �������� '%s'"

    char *msg = NULL;
    char *findnm(const char *nm){
        char *f = strstr(msg, nm);
        if(!f) return NULL;
        f += strlen(nm);
        char *e = strchr(f, '\n');
        size_t l = (e) ? (size_t)(e-f) : strlen(f);
        char *r = MALLOC(char, l+1);
        snprintf(r, l, f);
        return r;
    }
    msg = ApnGlueGetInfo(&pid, &vid);
    if(msg){
        //printf("\n Camera info:\n%s\n", msg);
        camera = findnm("Model: ");
        sensor = findnm("Sensor: ");
        info(_("Camera '%s' with sensor '%s' detected"), camera, sensor);
        free(msg);
    }

    // Second - check whether we want do a simple power operations
    if(StopRes){
        switch(StopRes){
            case Reset:
                ApnGlueReset();
                if(pid > 0 && vid > 0)
                    reset_usb_port(pid, vid);
            break;
            case Sleep:
                if(ApnGluePowerDown())
                // "������ �������� � ������ �����!"
                ERR(_("Error: sleepless night!"));
            break;
            default:
                ApnGluePowerResume();
        }
        goto returning;
    }

    // And then we can do other work
    if(save_Tlog){
        struct stat s;
        char print_hdr = 1;
        if(stat("temp_log", &s) == 0) print_hdr = 0;
        f_tlog = fopen("temp_log", "a");
        // "�� ���� ������� ���� ������� ����������"
        if(!f_tlog) err(1, _("Can't open temperature log file"));
        if(print_hdr)
            fprintf(f_tlog, "Time\t\t\tUnix time\tTint\tText\tStatus (eXp, Idle)\n");
        //fprintf(f_tlog, "\n\n\n");
    }
    // Shutter
    if(Shtr != -1)
        ApnGlueOpenShutter(Shtr);
    if(stat_logging){
        struct stat s;
        char print_hdr = 1;
        if(stat("stat_log", &s) == 0) print_hdr = 0;
        f_statlog = fopen("stat_log", "a");
        // "�� ���� ������� ���� ������� ����������"
        if(!f_statlog) err(1, _("Can't open statistics log file"));
        if(print_hdr)
            fprintf(f_statlog, "Time\t\t\tUnix time\tTexp\tTint\tText\tImax\tImin\tIavr\tIstd\tNover\tN>3std\tIavr3\t\tIstd3\n");
    }
    ApnGlueGetMaxValues(NULL, &roiw, &roih, &osw, &osh, &binw, &binh,
        &shtr, &mintemp);
    info(_("Max binning: %dx%d\n"), binw, binh);
    DBG("Shtr = %d, mintemp = %g", shtr, mintemp);
    ApnGlueGetGeom(&pixX, &pixY);
    // "\n������ "
    printf(_("\nThe shutter is "));
    if(ApnGlueReadShutter())
        // "������\n"
        printf(_("open\n"));
    else
        // "������\n"
        printf(_("closed\n"));
    printCoolerStat(NULL);

    if(set_T){
        if(cooler_off){
            ApnGlueSetTemp(0);
            // "���������� ������������"
            info(_("Switch cooler off"));
        }else{
            if(temperature >= mintemp){
                if(temperature == 0.) temperature += 2.*DBL_EPSILON; // remove problem of zero temperature
                ApnGlueSetTemp(temperature);
                // "��������� ����������� ������������"
                info(_("Changing of cooler setpoint"));
            }else
                // "����������� ���� ����������"
                info(_("The temperature setpoint is too low"));
        }
    }

    if(fanspd > -1){
        ApnGlueSetFan(fanspd);
        if(ApnGlueGetFan() == fanspd)
            // "������������ �������� �������� ������������ � %d\n"
            info(_("Set fan speed %d\n"), fanspd);
        else
            // "�� ���� ������� �������� �������� ������������!\n"
            ERR("Can't set fan speed\n");
    }

    if(only_T || exptime < 0) goto returning;

    // ������ �������: %g x %g
    //info(_("Pixel size: %g x %g"), pixX, pixY);
    --roiw;
    --roih;
    snprintf(viewfield, 79, "(0, %d)(0, %d)", roiw-osw, roih-osh);
    // "������� ����: %s"
    info(_("Field of view: %s"), viewfield);
    // "���� �����������: (0, %d)(0, %d)"
    info(_("Array field: (0, %d)(0, %d)"), roiw, roih);

    if(X0 == -1) X0 = 0; // default parameters
    if(Y0 == -1) Y0 = 0;
    if(X1 == -1 || X1 > roiw) X1 = roiw;
    if(Y1 == -1 || Y1 > roih) Y1 = roih;
    // check overskan
    if(X0 != 0 || X1 != roiw){
        if(X1 == roiw) X1 -= osw;
        osw = 0;
    }
    if(Y0 != 0 || Y1 != roih){
        if(Y1 == roih) Y1 -= osh;
        osh = 0;
    }
    if(hbin > binw) hbin = binw;
    if(vbin > binh) vbin = binh;

    //AutoadjustFanSpeed(FALSE);
    if(noflash) ApnGluePreFlash(0);
    if(noclean) ApnGlueDisablePostExpFlushing(1);
    if(twelveBit)
        ApnGlueSetDatabits(Apn_Resolution_TwelveBit);
    else
        ApnGlueSetDatabits(Apn_Resolution_SixteenBit);

    if(ROspeed) ApnGlueSetSpeed(ROspeed);

    if(pre_exp){// pre-expose
        // "��������������� ����������"
        info(_("Pre-expose"));
        double E = 0.;
        if(!twelveBit)
        ApnGlueSetDatabits(Apn_Resolution_TwelveBit);
        if(ApnGlueSetExpGeom(8, 8, 0, 0, 8, 8, 0, 0, &imW, &imH, whynot)){
            // "�� ���� ���������� ��������� ����������: %s"
            ERR("Can't set readout parameters: %s", whynot);
            goto returning;
        }
        int L = imW*imH;
        buf = (unsigned short*) calloc(L, sizeof(unsigned short));
        if(!buf){
            // "������ ��������� ������!"
            ERR(_("malloc() failed!"));
        }
        if(ApnGlueStartExp(&E, 0)){
            if(pid > 0 && vid > 0)
                reset_usb_port(pid, vid);
            // "������ ����������!"
            if(ApnGlueStartExp(&E, 0)) ERR("Error exposing pre-exp frame!");
        }
        ignore_signals();
        if(ApnGlueReadPixels(buf, L, whynot)){
            // "������ ����������: %s\n"
            ERR(_("Readout error: %s\n"), whynot);
        }
        free(buf);
        // restore signals
        catch_signals();
        if(!twelveBit)
            ApnGlueSetDatabits(Apn_Resolution_SixteenBit);
    }
    if(ApnGlueSetExpGeom(X1-X0-1, Y1-Y0+1, osw, osh, hbin, vbin,
                    X0, Y0, &imW, &imH, whynot)){
        DBG("ApnGlueSetExpGeom(%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %s)", X1-X0+1, Y1-Y0+1, osw, osh, hbin, vbin,
            X0, Y0, imW, imH, whynot);
        // "�� ���� ���������� ��������� ����������: %s"
        ERR("Can't set readout parameters: %s", whynot);
        goto returning;
    }
    DBG("ApnGlueSetExpGeom(%d, %d, %d, %d, %d, %d, %d, %d, %d, %d, %s)", X1-X0+1, Y1-Y0+1, osw, osh, hbin, vbin,
        X0, Y0, imW, imH, whynot);
    } // <------ end of non-fake block
    DBG("geomery: %dx%d", imW, imH);
    int L = imW*imH;
    buf = (unsigned short*) calloc(L, sizeof(unsigned short));
    if(!buf){
        // "������ ��������� ������!"
        ERR(_("malloc() failed!"));
    }
#ifdef IMAGEVIEW
    // start image view module if not need to save image or manually defined displaying
    if(fake || !save_image || show_image){
        imageview_init();
        im.protected = 1;
        im.rawdata = MALLOC(GLubyte, imW*imH*3);
        if(!im.rawdata) ERR("Can't allocate memory");
        im.w = imW; im.h = imH;
    }
#endif
    if(fake) pics = 0;
    DBG("start %d expositions", pics);
    for (i = 0; i < pics; i++){
        /*DBG("spd");
        AutoadjustFanSpeed(FALSE);
        DBG("cooler");*/
        temperature = printCoolerStat(NULL);
        double E;
        int I;
        if(exptime > -1){ // do expositions only if there is -x key
            E = (double) exptime / 1000.;
            I = (int) E;
            ignore_signals();
            DBG("start exp");
            if(ApnGlueStartExp(&E, shutter)){
                DBG("pid=%d, vid=%d", pid, vid);
                if(pid > 0 && vid > 0)
                    reset_usb_port(pid, vid);
                // "������ ����������!"
                if(ApnGlueStartExp(&E, shutter)) ERR("Error exposing frame!");
            }
            DBG("Exposing");
#ifdef USE_BTA
            if(histry) push_param();
#endif
            //exptime = (int)(E*1000.);
            printf("\n\n");
            // "������ ����� %d, ����� ���������� - %g ������\n"
            printf(_("Capture frame %d, exp time - %g sec\n"), i, E);
            expStartsAt = time(NULL); // ����� ������ ����������
            /*if(save_Tlog) if(curtime(tm_buf))
                // "������� ���������� %d-�� �����; ����������: %g�, ����� ������: %s, ����: %s\n"
                fprintf(f_tlog, _("Begin exposition of %dth frame; exp length: %gs, start time: %s, filename: %s\n"),
                    i, E, tm_buf, outfile);*/
            time0 = itime();
            do{
                // %.3f ������ �� ��������� ����������\n
                //printf(_("%.3f seconds till exposition ends\n"), ((float)ltmp) / 1000.);
                t_int = printCoolerStat(&t_ext);
                if(save_Tlog && curtime(tm_buf)){
                    fprintf(f_tlog, "%s\t%ld\t%.2f\t%.2f\tX\n", tm_buf, time(NULL), t_int, t_ext);
                }
                //AutoadjustFanSpeed(FALSE);
                int tt = ltime();
                printf("%d sec\n", tt);
                if(I - tt > time_interval){
                    sleep(time_interval);
#ifdef USE_BTA
                    if(histry) push_param();
#endif
                }else while(!ApnGlueExpDone()) usleep(100000); // make 100ms error
            }while(!ApnGlueExpDone());
            DBG("exp done");
#ifdef USE_BTA
            if(histry) push_param();
#endif
            // "���������� �����������:"
            printf(_("Read image: "));
            fflush(stdout);
            // try to ignore signals for this long operation
            ignore_signals();
            if(ApnGlueReadPixels(buf, L, whynot)){
                // "������ ����������: %s\n"
                ERR(_("Readout error: %s\n"), whynot);
                catch_signals();
                continue;
            }
            // mirror image if needed
            if(flipX){ // mirror vertically (around X axe)
                unsigned short *newbuf = MALLOC(unsigned short, imW*imH);
                OMP_FOR()
                for(int _row = 0; _row < imH; _row++){
                    unsigned short *optr = &newbuf[_row*imW], *iptr = &buf[(imH-1-_row)*imW];
                    for(int _pix = 0; _pix < imW; _pix++)
                        *optr++ = *iptr++;
                }
                FREE(buf);
                buf = newbuf;
            }
            if(flipY){ // mirror horizontally (around Y axe)
                unsigned short *newbuf = MALLOC(unsigned short, imW*imH);
                OMP_FOR()
                for(int _row = 0; _row < imH; _row++){
                    unsigned short *optr = &newbuf[_row*imW], *iptr = &buf[(_row+1)*imW-1];
                    for(int _pix = 0; _pix < imW; _pix++)
                        *optr++ = *iptr--;
                }
                FREE(buf);
                buf = newbuf;
            }
            // restore signals
            catch_signals();
            t_int = printCoolerStat(&t_ext);
            if(f_statlog && curtime(tm_buf))
                fprintf(f_statlog, "%s\t%ld\t%g\t%.2f\t%.2f\t", tm_buf,
                    time(NULL), E, t_int, t_ext);
            print_stat(buf, L, f_statlog);
#ifdef IMAGEVIEW
            if(!save_image || show_image){
                // build  FITS headers tree for viewer
                #ifdef USE_BTA
                write_bta_data(NULL);
                #endif
                check_wcs();
                if(!get_windows_amount() || !mainwin){
                    mainwin = createGLwin("Sample window", 400, 400, &im);
                    if(!mainwin){
                        // "�� ���� ������� ���� OpenGL, �������� ����� ����������!"
                        info(_("Can't open OpenGL window, image preview will be inaccessible"));
                    }else{
                        mainwin->killthread = 1;
                    }
                }
            }
            if(get_windows_amount() && mainwin){
                DBG("change image");
                change_displayed_image(buf, mainwin);
            }
#endif
            inline void WRITEIMG(int (*writefn)(char*,int,int,void*), char *ext){
                if(!check_filename(whynot, outfile, ext))
                    // "�� ���� ��������� ����"
                    err(1, _("Can't save file"));
                else{
                    int r = writefn(whynot, imW, imH, buf);
                    // "���� ������� � '%s'"
                    if (r == 0) info(_("File saved as '%s'"), whynot);
                }
            }
            if(save_image){
                DBG("save image");
                #ifdef USERAW
                WRITEIMG(writeraw, "raw");
                #endif // USERAW
                WRITEIMG(writefits, "fit");
                #ifdef USEPNG
                WRITEIMG(writepng, "png");
                #endif /* USEPNG */
            }
        }
        if(pause_len){
            int delta;
            time0 = itime();
            while((delta = pause_len - ltime()) > 0){
            // "%d ������ �� ��������� �����\n"
            printf(_("%d seconds till pause ends\n"), delta);
            temperature = printCoolerStat(&t_ext);
            if(save_Tlog && curtime(tm_buf)){
                fprintf(f_tlog, "%s\t%ld\t%.2f\t%.2f\tI\n", tm_buf, time(NULL), temperature, t_ext);
            }
            //AutoadjustFanSpeed(FALSE);
            if(delta > 10) sleep(10);
            else sleep(delta);
            }
        }
        fflush(NULL);
    }
#ifdef IMAGEVIEW
    if(fake){
        writefits(NULL, imW, imH, NULL);
        if(!get_windows_amount() || !mainwin){
            mainwin = createGLwin("Sample window", 400, 400, &im);
            if(!mainwin){
                // "�� ���� ������� ���� OpenGL, �������� ����� ����������!"
                info(_("Can't open OpenGL window, image preview will be inaccessible"));
            }else{
                mainwin->killthread = 1;
            }
        }
        if(get_windows_amount() && mainwin){
            DBG("change image");
            change_displayed_image(buf, mainwin);
        }
    }
#endif
returning:
    if(!only_turret){
#ifdef IMAGEVIEW
        DBG("test for GL window");
        if(mainwin){ //window was created - wait for manual close
            DBG("wait for window closing");
            while(get_windows_amount()){
                usleep(50000);
            }
            FREE(im.rawdata);
        }
#endif
        if(!fake){
            if(f_tlog) fclose(f_tlog);
            if(f_statlog) fclose(f_statlog);
            signals(0);
        }
    }
    return 0;
}
