#include "takepic.h"
#include <locale.h>
#include <signal.h>
#include <dirent.h>
#include <float.h>

#include <usb.h>
#include <sys/ioctl.h>
#include <linux/usbdevice_fs.h>

#include "usage.h"
#include "camtools.h"
#include "bta_print.h"

#define BUFF_SIZ 4096
#define PROC_BASE "/proc"

#define TMBUFSIZ 40 // time string buffer length

char *pidfilename = "/tmp/takepic.pid"; // pidfile

char tm_buf[TMBUFSIZ];	// time string buffer

char *camera = NULL, *sensor = NULL, viewfield[80];
double pixX, pixY; // pixel size

int test_headers = 0; // don't even try to open camera device, just show FITS keys

double t_ext, t_int;	// hot & cold side temperatures
time_t expStartsAt;		// time of exposition start (time_t)

int itime(){ // time in seconds to count intervals
	struct timeval ct;
	gettimeofday(&ct, NULL);
	return (ct.tv_sec);
}
int time0;
int ltime(){ // time since last time0 reinitialising
	return(itime()-time0);
}

// return string with current date/time
size_t curtime(char *s_time){
	time_t tm = time(NULL);
	size_t s = strftime(s_time, TMBUFSIZ, "%d/%m/%Y,%H:%M:%S", localtime(&tm));
	if(!s) info("curtime() error");
	return s;
}

// find the first non-exists filename & put it into buff
int check_filename(char *buff, char *outfile, char *ext){
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

void reset_usb_port(int pid, int vid){
	int fd, rc;
	char buf[256], *d = NULL, *f = NULL;
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
		// "Устройство не найдено"
		ERR(_("Device not found"));
		return;
	}
	DBG("found camera device, reseting");
	snprintf(buf, 255, "/dev/bus/usb/%s/%s", d,f);
	fd = open(buf, O_WRONLY);
	if (fd < 0) {
		// "Не могу открыть файл устройства %s: %s"
		ERR(_("Can't open device file %s: %s"), buf, strerror(errno));
		return;
	}
	info("Resetting USB device %s", buf);
	rc = ioctl(fd, USBDEVFS_RESET, 0);
	if (rc < 0) {
		// "Не могу вызывать ioctl"
		perror(_("Error in ioctl"));
		return;
	}
	close(fd);
}

// quit by signal
static void signals(int sig){
	int u;
	// "Получен сигнал %d, отключаюсь.\n"
	printf(_("Get signal %d, quit.\n"), sig);
	ApnGlueWheelClose();
	DBG("wheel closed");
//	ApnGlueExpAbort(); // this function stubs all!
//	DBG("exp aborted");
	ApnGlueClose();
	DBG("device closed");
	u = unlink(pidfilename);
	// "Не могу удалить PID-файл"
	if(u == -1) perror(_("Can't delete PIDfile"));
	exit(sig);
}

int sigcounter = 3;
static void cnt_signals(int sig){
	--sigcounter;
	if(sigcounter != 0){
		// "Нажмите Ctrl+C еще %d раз[а], чтобы прервать считывание\n"
		printf(_("Press Ctrl+C %d time[s] more to interrupt reading\n"), sigcounter);
		signal(SIGINT, cnt_signals);
	}else{
		signals(sig);
	}
}

// Try to ignore all signals exept SIGINT, which is counted
void ignore_signals(){
	sigcounter = 3;
	for(int _=0; _<256; _++)
		signal(_, SIG_IGN);
	signal(SIGINT, cnt_signals);
}
// We must take care about signals to avoid ctrl+C problems
void catch_signals(){
	for(int _=0; _<256; _++)
		signal(_, SIG_IGN);
	signal(SIGTERM, signals); // kill (-15) - quit
	signal(SIGHUP, signals);  // hup - quit
	signal(SIGINT, signals);  // ctrl+C - quit
	signal(SIGQUIT, signals); // ctrl+\ - quit
	//signal(SIGTSTP, SIG_IGN); // ignore ctrl+Z
}

// check for running process
void check4running(){
	// get process name from /proc
	int readname(char *name, pid_t pid){
		char *pp = name, byte, path[256];
		int cntr = 0, file;
		snprintf (path, 255, PROC_BASE "/%d/cmdline", pid);
		file = open(path, O_RDONLY);
		if(file == -1) return 0;
		do{
			read(file, &byte, 1);
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
		fscanf(pidfile, "%d", &run); // get PID of (possibly) running process
		fclose(pidfile);
		if(readname(name, run) && strncmp(name, myname, 255) == 0){
			// "Обнаружен работающий процесс (pid=%d), выход.\n"
			ERR(_("\nFound running process (pid=%d), exit.\n"), run);
			exit(0);
		}
	}
	// there's no PID file or it's old
	readname(myname, self); // self name
	while ((de = readdir (dir)) != NULL){ // scan /proc
		if (!(pid = (pid_t) atoi (de->d_name)) || pid == self)
			continue;
		readname(name, pid); // get process name
		if(strncmp(name, myname, 255) == 0){
			// "Обнаружен работающий процесс (pid=%d), выход.\n"
			ERR(_("\nFound running process (pid=%d), exit.\n"), pid);
			exit(0);
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
void parse_turret_args(){
	int maxPos, curPos;
	if(ApnGlueWheelOpen(Nwheel, Apn_Filter_FW50_7S)){
		// "Не могу открыть турель"
		ERR(_("Can't open turret"));
		return;
	}
	DBG("get max pos");
	maxPos = ApnGlueWheelGetMaxPos();
	curPos = ApnGlueWheelGetPos();
	// "Максимальная (MAX) и текущая (CUR) позиции турели:\n"
	printf(_("Turret MAXimum position and CURrent position:\n"));
	printf("MAX=%d, CUR=%d\n", maxPos, curPos);
	if(wheelPos == 0) goto wheelret;
	if(wheelPos > maxPos){
		// "Требуемая позиция больше максимальной"
		ERR(_("Required position greater then max"));
		goto wheelret;
	}
	if(ApnGlueWheelSetPos(wheelPos)){
		// "Не могу переместить турель"
		ERR(_("Can't move turret"));
		goto wheelret;
	}
	if(ApnGlueWheelGetStatus()){
		// "Подождите завершения перемещения турели "
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

int main(int argc, char **argv){
	int i;	//cycles
	int pid = -1, vid = -1; // device pid/vid
	FILE *f_tlog = NULL; // temperature logging file
	FILE *f_statlog = NULL; // stat file
	int roih=0, roiw=0, osh=0, osw=0, binh=0, binw=0, shtr=0; // camera parameters
	char  whynot[BUFF_SIZ]; // temporary buffer for error texts
	int imW=0, imH=0; // real (with binning) image size
	unsigned short *buf = NULL; // image buffer
	double mintemp=0.;

	//setlocale(LC_ALL, getenv("LC_ALL"));
	setlocale(LC_ALL, "");
	setlocale(LC_NUMERIC, "C");
	bindtextdomain(GETTEXT_PACKAGE, LOCALEDIR);
	//bind_textdomain_codeset(GETTEXT_PACKAGE, "UTF-8");
	textdomain(GETTEXT_PACKAGE);

	parse_args(argc, argv);

	catch_signals();
	// Check whether there's no concurrents
	check4running();

	if(test_headers){
		writefits(NULL, imW, imH, NULL);
		exit(0);
	}

	// Turret block
	if(open_turret) parse_turret_args();
	if(only_turret) goto returning;

	// And camera block
	// First - open camera devise
	if(ApnGlueOpen(Ncam)){
		// "Не могу открыть камеру, завершаю"
		ERR(_("Can't open camera device, exit"));
		return 1;
	}
	ApnGlueGetName(&sensor, &camera);
	camera = strdup(camera); sensor = strdup(sensor);
	// "Обнаружена камера '%s' с датчиком '%s'"
	info(_("Find camera '%s' with sensor '%s'"), camera, sensor);

	// "Адрес USB: "
	char *msg = NULL;
	msg = ApnGlueGetInfo(&pid, &vid);
	printf("\n Camera info:\n%s\n", msg);
	free(msg);

	// Second - check whether we want do a simple power operations
	if(StopRes){
		switch(StopRes){
			case Reset:
				ApnGlueReset();
				reset_usb_port(pid, vid);
			break;
			case Sleep:
				if(ApnGluePowerDown())
				// "Ошибка перехода в спящий режим!"
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
		// "Не могу открыть файл журнала температур"
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
		// "Не могу открыть файл журнала статистики"
		if(!f_statlog) err(1, _("Can't open statistics log file"));
		if(print_hdr)
			fprintf(f_statlog, "Time\t\t\tUnix time\tTexp\tTint\tText\tImax\tImin\tIavr\tIstd\tNover\tN>3std\tIavr3\t\tIstd3\n");
	}
	//ApnGlueSetFan(3);
	ApnGlueGetMaxValues (NULL, &roiw, &roih, &osw, &osh, &binw, &binh,
		&shtr, &mintemp);
	ApnGlueGetGeom(&pixX, &pixY);
	// "\nЗатвор "
	printf(_("\nThe shutter is "));
	if(ApnGlueReadShutter())
		// "открыт\n"
		printf(_("open\n"));
	else
		// "закрыт\n"
		printf(_("closed\n"));
	printCoolerStat(NULL);

	if(set_T){
		if(cooler_off){
			ApnGlueSetTemp(0);
			// "Отключение холодильника"
			info(_("Switch cooler off"));
		}else{
			if(temperature >= mintemp){
				if(temperature == 0) temperature += DBL_EPSILON; // remove problem of zero temperature
				ApnGlueSetTemp(temperature);
				// "Изменение температуры холодильника"
				info(_("Changing of cooler setpoint"));
			}else
				// "Температура ниже допустимой"
				info(_("The temperature setpoint is too low"));
		}
	}

	// Размер пикселя: %g x %g
	//info(_("Pixel size: %g x %g"), pixX, pixY);

	snprintf(viewfield, 79, "(0, %d)(0, %d)", roiw, roih);
	// "Видимое поле: %s"
	info(_("Field of view: %s"), viewfield);
	// "Поле изображения: (0, %d)(0, %d)"
	info(_("Array field: (0, %d)(0, %d)"), roiw+osw, roih+osh);

	if(X0 == -1) X0 = 0; // default parameters
	if(Y0 == -1) Y0 = 0;
	if(X1 == -1 || X1 > roiw) X1 = roiw;
	if(Y1 == -1 || Y1 > roih) Y1 = roih;
	if(X0 != 0 || X1 != roiw) osw = 0; // check overskan
	if(Y0 != 0 || Y1 != roih) osh = 0;
	if(hbin > binw) hbin = binw;
	if(vbin > binh) vbin = binh;

	if(only_T) goto returning;
	AutoadjustFanSpeed(FALSE);
	if(noflash) ApnGluePreFlash(0);
	if(noclean) ApnGlueDisablePostExpFlushing(1);
	if(twelveBit)
		ApnGlueSetDatabits(Apn_Resolution_TwelveBit);
	else
		ApnGlueSetDatabits(Apn_Resolution_SixteenBit);

	if(ROspeed) ApnGlueSetSpeed(ROspeed);
	DBG("here");
	if(pre_exp){// pre-expose
		// "Предварительная экспозиция"
		info(_("Pre-expose"));
		double E = 0.;
		if(!twelveBit)
		ApnGlueSetDatabits(Apn_Resolution_TwelveBit);
		if(ApnGlueSetExpGeom(roiw, roih, 0, 0, 8, 8,
						0, 0, &imW, &imH, whynot)){
			// "Не могу установить параметры считывания: %s"
			ERR("Can't set readout parameters: %s", whynot);
			goto returning;
		}
		int L = imW*imH;
		buf = (unsigned short*) calloc(L, sizeof(unsigned short));
		if(!buf){
			// "Ошибка выделения памяти!"
			err(1, "malloc() failed");
		}
		if(ApnGlueStartExp(&E, 0)){
			// "Ошибка экспозиции!"
			ERR("Error exposing frame!");
			exit(-2);
		}
		ignore_signals();
		if(ApnGlueReadPixels(buf, L, whynot)){
			// "Ошибка считывания: %s\n"
			ERR(_("Readout error: %s\n"), whynot);
			exit(-2);
		}
		free(buf);
		// restore signals
		catch_signals();
		if(!twelveBit)
			ApnGlueSetDatabits(Apn_Resolution_SixteenBit);
	}
	if(ApnGlueSetExpGeom(X1-X0, Y1-Y0, osw, osh, hbin, vbin,
					X0, Y0, &imW, &imH, whynot)){
		// "Не могу установить параметры считывания: %s"
		ERR("Can't set readout parameters: %s", whynot);
		goto returning;
	}
	DBG("geomery: %dx%d", imW, imH);
	int L = imW*imH;
	buf = (unsigned short*) calloc(L, sizeof(unsigned short));
	if(!buf){
		// "Ошибка выделения памяти!"
		err(1, "malloc() failed");
	}
	for (i = 0; i < pics; i++){
		AutoadjustFanSpeed(FALSE);
		temperature = printCoolerStat(NULL);
		double E;
		int I;
		if(exptime > -1){ // do expositions only if there is -x key
			E = (double) exptime / 1000.;
			I = (int) E;
			ignore_signals();
			if(ApnGlueStartExp(&E, shutter)){
				// "Ошибка экспозиции!"
				ERR("Error exposing frame!");
				continue;
			}
			DBG("Exposing");
#ifdef USE_BTA
			push_param();
#endif
			//exptime = (int)(E*1000.);
			printf("\n\n");
			// "Захват кадра %d, время экспозиции - %g секунд\n"
			printf(_("Capture frame %d, exp time - %g sec\n"), i, E);
			expStartsAt = time(NULL); // время начала экспозиции
			/*if(save_Tlog) if(curtime(tm_buf))
				// "Начинаю экспозицию %d-го файла; экспозиция: %gс, время начала: %s, файл: %s\n"
				fprintf(f_tlog, _("Begin exposition of %dth frame; exp length: %gs, start time: %s, filename: %s\n"),
					i, E, tm_buf, outfile);*/
			time0 = itime();
			do{
				// %.3f секунд до окончания экспозиции\n
				//printf(_("%.3f seconds till exposition ends\n"), ((float)ltmp) / 1000.);
				t_int = printCoolerStat(&t_ext);
				if(save_Tlog && curtime(tm_buf)){
					fprintf(f_tlog, "%s\t%ld\t%.2f\t%.2f\tX\n", tm_buf, time(NULL), t_int, t_ext);
				}
				AutoadjustFanSpeed(FALSE);
				int tt = ltime();
				printf("%d sec\n", tt);
				if(I - tt > time_interval){
					sleep(time_interval);
#ifdef USE_BTA
					push_param();
#endif
				}else while(!ApnGlueExpDone()) usleep(100000); // make 100ms error
			}while(!ApnGlueExpDone());
			DBG("exp done");
#ifdef USE_BTA
			push_param();
#endif
			// "Считывание изображения:"
			printf(_("Read image: "));
			fflush(stdout);
			// try to ignore signals for this long operation
			ignore_signals();
			if(ApnGlueReadPixels(buf, L, whynot)){
				// "Ошибка считывания: %s\n"
				ERR(_("Readout error: %s\n"), whynot);
				catch_signals();
				continue;
			}
			// restore signals
			catch_signals();
			t_int = printCoolerStat(&t_ext);
			if(f_statlog && curtime(tm_buf))
				fprintf(f_statlog, "%s\t%ld\t%g\t%.2f\t%.2f\t", tm_buf,
					time(NULL), E, t_int, t_ext);
			print_stat(buf, L, f_statlog);
			inline void WRITEIMG(int (*writefn)(char*,int,int,void*), char *ext){
				if(!check_filename(whynot, outfile, ext))
					// "Не могу сохранить файл"
					err(1, _("Can't save file"));
				else{
					int r = writefn(whynot, imW, imH, buf);
					// "Файл записан в '%s'"
					if (r == 0) info(_("File saved as '%s'"), whynot);
				}
			}
			if(save_image){
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
			// "%d секунд до окончания паузы\n"
			printf(_("%d seconds till pause ends\n"), delta);
			temperature = printCoolerStat(&t_ext);
			if(save_Tlog && curtime(tm_buf)){
				fprintf(f_tlog, "%s\t%ld\t%.2f\t%.2f\tI\n", tm_buf, time(NULL), temperature, t_ext);
			}
			AutoadjustFanSpeed(FALSE);
			if(delta > 10) sleep(10);
			else sleep(delta);
			}
		}
		fflush(NULL);
	}
returning:
	// set fan speed to 0 or 3 according cooler status
	if(!only_turret) AutoadjustFanSpeed(TRUE);
	DBG("abort exp");
	ApnGlueExpAbort();
	DBG("close");
	ApnGlueClose();
	DBG("free buffers & close files");
	free(buf);
	if(f_tlog) fclose(f_tlog);
	if(f_statlog) fclose(f_statlog);
	return 0;
}
