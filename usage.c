/*
 * usage.c
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

#include "usage.h"
#include "macros.h"
#include "defhdrs.h"

Apn_Filter Tturret = Apn_Filter_FW50_7S; // turrer type
int
     TurretPos = 0		// turret setposition
    ,wheelPos = 0		// turret position
    ,Nwheel = 1			// turret number
;

unsigned short
     ROspeed = 0		// readout speed
;

char
     *objname = NULL	// object name
    ,*outfile = NULL	// filename prefix
    ,*objtype = NULL	// frame type (default - object, when -d - dark)
    ,*instrument = NULL	// instrument name (default - "direct imaging")
    ,*observers = NULL	// observers name
    ,*prog_id = NULL	// program identificator
    ,*author = NULL		// author of program
    ,*subnet = NULL		// subnet for ethernet camera discovery
    ,*cammsgid = NULL	// MSG-ID of camera
    ,*defhdr_filename = NULL // name of file with default headers
;
int
     exptime = -1	// exposition time (in ms), -1 means no exposition
    ,fanspd = -1	// set fan speed (default - auto)
    ,time_interval = 10 // interval between sequential logging
    ,pics = 1		// number of frames per run
    ,Ncam = 1		// camera number (if many)
    ,hbin = 1		// horizontal binning
    ,vbin = 1		// vertical binning
    ,X0=-1,Y0=-1	// top left corner coordinates
    ,X1=-1,Y1=-1	// right bottom corner coordinates
        // if ==-1 - all, including overscan
    ,pause_len = 0	// pause length (in s) between frames
    ,pre_exp = 0	// pre-expose
    ,StopRes = 0	// reset (1), sleep (2), awake (3)
    ,Shtr = -1		// shutter: -1 -- no action or SHUTTER_OPEN/SHUTTER_CLOSE
    ,noclean = 0	// don't flush camera after exp
    ,twelveBit = 0	// 12-bit ADC
    ,fake = 0		// fake image
    ,flipX = 0		// flip image around X axe (vertical flip)
    ,flipY = 0		// flip image around Y axe (horizontal flip)
    ,histry = 0		// write history at expositions
;
double
     temperature = -25.	// setpoint of temperature
    ,imscale = -1.		// image scale (''/pix) given by user
;


int shutter = 1;	// object frame == 1, dark frame == 0

bool
     only_T			= FALSE	// only show temperature
    ,noflash		= FALSE // don't flash before exposition
    ,set_T			= FALSE	// set temperature
    ,cooler_off		= FALSE // set cooler off
    ,save_Tlog		= FALSE	// save temperature log
    ,save_image		= TRUE	// save image into file
    ,stat_logging	= FALSE	// full stat log
    ,only_turret	= TRUE  // only move turret
    ,open_turret	= FALSE // work with turret
#ifdef IMAGEVIEW
    ,show_image		= FALSE // show image on webgl window
#endif
;

int myatoi(int *num, const char *str){ // "careful" atoi
    if(!num || !str) return -1;
    long tmp;
    char *endptr;
    tmp = strtol(str, &endptr, 0);
    if (*str == '\0' || *endptr != '\0' || tmp < INT_MIN || tmp > INT_MAX){
        return -1;
    }
    *num = (int)tmp;
    return 0;
}

void getrange(int *X0, int *X1, char *arg){
    char *a = NULL, *pair;
    pair = strdup(arg);
    if((a = strchr(pair, ','))){
        *a = 0;
        a++;
    }
    if(myatoi(X0, pair) || *X0 < 0){
        // "Неверная нижняя граница: %s"
        usage(_("Wrong lower border: %s"), pair);
    }
    if(a){
        if(myatoi(X1, a) || *X1 < 0 || *X1 <= *X0){
            // "Неверная верхняя граница: %s"
            usage(_("Wrong upper border: %s"), pair);
        }
    }
    free(pair);
}

void usage(char *fmt, ...){
    va_list ap;
    va_start(ap, fmt);
    printf("\n");
    if (fmt != NULL){
        vprintf(fmt, ap);
        printf("\n\n");
    }
    va_end(ap);
    // "Использование:\t%s [опции]\n"
    printf(_("Usage:\t%s [options]\n"),
        __progname);
    // "\tОпции:\n"
    printf(_("\tOptions:\n"));
    printf("\t-A,\t--author=author\t\t%s\n",
        // "автор программы"
        _("program author"));
    printf("\t-b,\t--defhdr=filename\t%s\n",
        // "имя файла с заголовками по умолчанию"
        _("file with default headers"));
    printf("\t-B,\t--observer=obs\t\t%s\n",
        // "имена наблюдателей"
        _("observers' names"));
    printf("\t-c,\t--cooler-off\t\t%s\n",
        // "отключить холодильник"
        _("set cooler off"));
    printf("\t-C,\t--imscale\t\t%s\n",
        // "масштаб изображения без биннинга"
        _("image scale without binning"));
    printf("\t-d,\t--dark\t\t\t%s\n",
        // "не открывать затвор при экспозиции (\"темновые\")"
        _("not open shutter when exposing (\"dark frames\")"));
    printf("\t-D,\t--display-image\t\t%s\n",
        // "Отобразить на экране полученное изображение"
        _("display last image"));
    printf("\t-E,\t--ether-subnet\t\t%s\n",
        // "Подсеть для поиска ethernet-камеры"
        _("subnet fot ethernet camera discovery"));
    printf("\t-f,\t--no-flash\t\t%s\n",
        // "не засвечивать матрицу перед экспозицией"
        _("don't flash CCD chip before expose"));
    printf("\t-F,\t--fan-speed=F\t\t%s\n",
        // "Установить скорость вентиляторов в F (0..3)"
        _("set fan speed to F (0..3)"));
    printf("\t-g,\t--wheel-get\t\t%s\n",
        // получить сведения о турели
        _("get turret's parameters"));
    printf("\t-G,\t--wheel-go=N\t\t%s\n",
        // переместить турель в N-ю позицию
        _("set turret to Nth position"));
    printf("\t-h,\t--hbin=N\t\t%s\n",
        // "биннинг N пикселей по горизонтали"
        _("horizontal binning to N pixels"));
    printf("\t-H,\t--time-interval=T\t%s\n",
        // "интервал времени между последовательными записями в лог и HISTORY (в секундах)"
        _("time interval between sequential writings to log & HISTORY (in seconds)"));
    printf("\t-i,\t--instrument=instr\t%s\n",
        // "название прибора"
        _("instrument name"));
    printf("\t-I,\t--image-type=type\t%s\n",
        // "тип изображения"
        _("image type"));
    printf("\t-l,\t--tlog\t\t\t%s\n",
        // "вести запись рабочих температур в файл temp_log"
        _("make temperatures logging to file temp_log"));
    printf("\t-L,\t--log-only\t\t%s\n",
        // "не сохранять изображения, лишь вести запись статистки"
        _("don't save images, only make all-stat log"));
    printf("\t-M,\t--msg-id\t\t%s\n",
        // "открыть камеру по MSG-ID"
        _("open camera by its MSG-ID"));
    printf("\t-n,\t--nframes=N\t\t%s\n",
        // "N кадров в серии"
        _("make series of N frames"));
    printf("\t-N,\t--ncam=N\t\t%s\n",
        // "работать с N-й камерой"
        _("work with Nth camera"));
    printf("\t-o,\t--outfile=prefix\t%s\n",
        // "префикс имени выходного файла"
        _("output filename prefix"));
    printf("\t-O,\t--object=obj\t\t%s\n",
        // "название объекта"
        _("object name"));
    printf("\t-p,\t--pause-len=ptime\t%s\n",
        // "выдержать ptime секунд между экспозициями"
        _("make pause for ptime seconds between expositions"));
    printf("\t-P,\t--prog-id=prname\t%s\n",
        // "название программы наблюдений"
        _("observing program name"));
    printf("\t-r,\t--speed-set=N\t\t%s\n",
        // "установить скорость считывания в N"
        _("set readout speed to N"));
    printf("\t-R,\t--reset\t\t\t%s\n",
        // "Полный сброс"
        _("full reset"));
    printf("\t-s,\t--only-stat\t\t%s\n",
        // "не сохранять изображение, а только отобразить статистику"
        _("not save image, just show statistics"));
    printf("\t-S,\t--sleep\t\t\t%s\n",
        // "перейти в спящий режим"
        _("go to sleeping mode"));
    printf("\t-t,\t--set-temp=degr\t\t%s\n",
        // "задать рабочую температуру degr градусов"
        _("set work temperature to degr C"));
    printf("\t-T,\t--only-temp\t\t%s\n",
        // "только задать/получить температуру"
        _("only set/get temperature"));
    printf("\t-v,\t--vbin=N\t\t%s\n",
        // "биннинг N пикселей по вертикали"
        _("vertical binning to N pixels"));
    printf("\t-w,\t--wheel-num=N\t\t%s\n",
        // установить номер турели в N
        _("set turret's number to N"));
    printf("\t-W,\t--wakeup\t\t%s\n",
        // "возобновить питание"
        _("resume system"));
    printf("\t-x,\t--exp=exptime\t\t%s\n",
        // "время экспозиции exptime мс"
        _("set exposure time to exptime ms"));
    printf("\t-X,\t--xclip=X0[,X1]\t\t%s [X0:X1]\n",
        // "выбрать диапазон для считывания"
        _("select clip region"));
    printf("\t-Y,\t--xclip=Y0[,Y1]\t\t%s [Y0:Y1]\n",
        // "выбрать диапазон для считывания"
        _("select clip region"));
    // ONLY LONG
    printf("\t\t--fakeimg\t\t%s\n",
        // "Тест окна OpenGL без получения изображения"
        _("Test OpenGL window (without image exposure)"));
    printf("\t\t--flipX\t\t\t%s\n",
        // "отразить изображение вертикально (относительно оси X)"
        _("flip image vertically (around X axe)"));
    printf("\t\t--flipY\t\t\t%s\n",
        // "отразить изображение горизонтально (относительно оси Y)"
        _("flip image horizontally (around Y axe)"));
    printf("\t\t--noclean\t\t%s\n",
        // "не очищать матрицу после считывания"
        _("don't flush ccd after expose"));
    printf("\t\t--pre-exp\t\t%s\n",
        // "выполнить предварительную нулевую экспозицию для очистки матрицы"
        _("make a preliminary zero exposition for cleaning CCD"));
    printf("\t\t--shutter-open\t\t%s\n",
        // "открыть затвор"
        _("open shutter"));
    printf("\t\t--shutter-close\t\t%s\n",
        // "заткрыть затвор"
        _("close shutter"));
    printf("\t\t--test-headers\t\t%s\n",
        // "не открывать устройство, лишь отобразить шапку FITS"
        _("don't open camera device, just show FITS header"));
    printf("\t\t--twelve-bit\t\t%s\n",
        // "работать в 12-битном режиме"
        _("work in a 12-bit ADC mode"));
    printf("\t\t--write-history\t\t%s\n",
        // "записывать в историю параметры телескопа"
        _("write telescope parameters into history"));
    exit(0);
}

void parse_args(int argc, char **argv){
    FNAME();
    int i;
    char short_options[] = "A:b:B:cC:dDE:fF:gG:H:h:I:i:LlM:N:n:O:o:P:p:Rr:SsTt:v:Ww:x:X:Y:";
    struct option long_options[] = {
/*		{ name, has_arg, flag, val }, где:
 * name - name of long parameter
 * has_arg = 0 - no argument, 1 - need argument, 2 - unnesessary argument
 * flag = NULL to return val, pointer to int - to set it
 * 		value of val (function returns 0)
 * val -  getopt_long return value or value, flag setting to
 * !!! last string - for zeros !!!
 */
        {"author",		1,	0,	'A'},
        {"defhdr",		1,	0,	'b'},
        {"observers",	1,	0,	'B'},
        {"cooler-off",	0,	0,	'c'},
        {"imscale",		1,	0,	'C'},
        {"dark",		0,	0,	'd'},
        {"display-image",0,	0,	'D'},
        {"ether-subnet",1,	0,	'E'},
        {"no-flash",	0,	0,	'f'},
        {"fan-speed",	1,	0,	'F'},
        {"wheel-get",	0,	0,	'g'},
        {"wheel-go",	1,	0,	'G'},
        {"time-interval",1, 0,	'H'},
        {"hbin",		1,	0,	'h'},
        {"image-type",	1,	0,	'I'},
        {"instrument",	1,	0,	'i'},
        {"log-only",	0,	0,	'L'},
        {"tlog",		0,	0,	'l'},
        {"msg-id",		1,	0,	'M'},
        {"ncam",		1,	0,	'N'},
        {"nframes",		1,	0,	'n'},
        {"outfile",		1,	0,	'o'},
        {"object",		1,	0,	'O'},
        {"prog-id",		1,	0,	'P'},
        {"pause-len",	1,	0,	'p'},
        {"reset",		0,	0,	'R'},
        {"speed-set",	1,	0,	'r'},
        {"sleep",		0,	0,	'S'},
        {"only-stat",	0,	0,	's'},
        {"only-temp",	0,	0,	'T'},
        {"set-temp",	1,	0,	't'},
        {"vbin",		1,	0,	'v'},
        {"wheel-num",	1,	0,	'w'},
        {"wakeup",		0,	0,	'W'},
        {"exp",			1,	0,	'x'},
        {"xclip",		1,	0,	'X'},
        {"yclip",		1,	0,	'Y'},
        // options, that have no short analogs:
        {"fakeimg",			0,	&fake,		1},
        {"flipX",			0,	&flipX,		1},
        {"flipY",			0,	&flipY,		1},
        {"noclean",			0,	&noclean,	1},
        {"shutter-open",	0,	&Shtr,		1},
        {"shutter-close",	0,	&Shtr,		0},
        {"test-headers",	0,	&test_headers, 1},
        {"twelve-bit",		0,	&twelveBit, 1},
        {"pre-exp",			0,	&pre_exp, 	1},
        {"write-history",	0,	&histry,	1},
        { 0, 0, 0, 0 }
    };
    if(argc == 1){
        // "Не введено никаких параметров"
        usage(_("Any parameters are absent"));
    }
    while (1){
        int opt;
        if((opt = getopt_long(argc, argv, short_options,
            long_options, NULL)) == -1) break;
        switch(opt){
        case 0: // only long option
        break;
        case 'A':
            author = strdup(optarg);
            // "Автор программы: %s"
            info(_("Program author: %s"), author);
            break;
        case 'b':
            defhdr_filename = strdup(optarg);
            break;
        case 'B':
            observers = strdup(optarg);
            // "Наблюдатели: %s"
            info(_("Observers: %s"), observers);
            break;
        case 'c':
            only_turret = FALSE;
            set_T = TRUE;
            // "отключить холодильник"
            info(_("Set cooler off"));
            cooler_off = TRUE;
            break;
        case 'C':
            imscale = atof(optarg);
            if(imscale < 0.){
                // "IMSCALE должно быть больше нуля"
                usage(_("IMSCALE should be greater than zero"));
            }
            break;
        case 'd':
            shutter = 0;
            // "Съемка темновых"
            info(_("Dark frames"));
            break;
        case 'D':
#ifdef IMAGEVIEW
            show_image = TRUE;
            // "Отображение снятых кадров"
            info(_("Will show images"));
#else
            ERR("%s was compiled without OpenGL support!", __progname);
#endif
            break;
        case 'E':
            subnet = strdup(optarg);
            // "Подсеть: %s"
            info(_("Subnet: %s"), subnet);
            break;
        case 'f':
            noflash = TRUE;
            // "Не засвечивать камеру до экспозиции"
            info(_("No pre-expose flash"));
            break;
        case 'F':
            if(myatoi(&fanspd, optarg) || fanspd > 3 || fanspd < 0)
                // "Скорость вентилятора должна быть в пределе 0..3"
                usage(_("Fan speed should be in interval 0..3"));
            // "Установить скорость вращения вентиляторов в %d"
            info(_("Fan speed would be set to %d"), fanspd);
            only_turret = FALSE;
            break;
        case 'g':
            open_turret = TRUE;
            // "Получить информацию о турели"
            info(_("Get turret's info"));
            break;
        case 'G':
            open_turret = TRUE;
            if(myatoi(&wheelPos, optarg) || wheelPos < 1)
                // "Номер позиции должен быть больше 0"
                usage(_("Position number should be positive value"));
            // "Переместить турель в позицию %d"
            info(_("Move turret into %d"), wheelPos);
            break;
        case 'H':
            if(myatoi(&time_interval, optarg) || time_interval < 1)
                // "Значение time-interval должно быть натуральным числом"
                usage(_("Value of time-interval must be non-zero positive"));
            // "Установить интервал логгирования в %d секунд"
            info(_("Set logging interval to %d seconds"), time_interval);
            break;
        case 'h':
            if (myatoi(&hbin, optarg) || hbin < 1){
                // "Неверный"
                usage("%s hbin: %s", _("Wrong"), optarg);
            }
            // "Гор. биннинг: %d"
            info(_("Horisontal binning: %d"), hbin);
            break;
        case 'I':
            objtype = strdup(optarg);
            // "Тип изображения - %s"
            info(_("Image type - %s"), objtype);
            break;
        case 'i':
            instrument = strdup(optarg);
            // "Название прибора - %s"
            info(_("Instrument name - %s"), instrument);
            break;
        case 'L':
            stat_logging = TRUE;
            // Полное журналирование статистики
            info(_("Full statistics logging"));
            break;
        case 'l':
            save_Tlog = TRUE;
            only_turret = FALSE;
            // "Сохранение журнала температур"
            info(_("Save temperature log"));
            break;
        case 'M':
                cammsgid = strdup(optarg);
            info("CAMMSGID: %s", cammsgid);
            break;
        case 'N':
            only_turret = FALSE;
            if (myatoi(&Ncam, optarg) || Ncam < 1){
                // "Неверный номер камеры: %s"
                usage(_("Wrong camera number: %s"), optarg);
            }
            // "Работать с камерой номер %d"
            info(_("Open camera number %d"), Ncam);
            break;
        case 'n':
            if (myatoi(&pics, optarg) || pics <= 0){
                // "Неверное кол-во кадров: %s"
                usage(_("Wrong frames number in series: %s"), optarg);
            }
            // "Серия из %d кадров"
            info(_("Series of %d frames"), pics);
            break;
        case 'o':
            outfile = strdup(optarg);
            // "Префикс выходного файла"
            info(_("Output filename prefix: %s"), outfile);
            break;
        case 'O':
            objname = strdup(optarg);
            // "Имя объекта - %s"
            info(_("Object name - %s"), objname);
            break;
        case 'P':
            prog_id = strdup(optarg);
            // "Название программы: %s"
            info(_("Program name: %s"), prog_id);
            break;
        case 'p':
            if (myatoi(&pause_len, optarg) || pause_len < 0){
                // "Неверная пауза: %s"
                usage(_("Wrong pause length: %s"), optarg);
            }
            // "Пауза: %dс"
            info(_("Pause: %ds"), pause_len);
            break;
        case 'r':
            if (myatoi(&i, optarg) || i < 1 || i > USHRT_MAX){
                // "Скорость считывания - беззнаковое целое!"
                usage(_("Readout speed should be unsigned int!"));
            }
            ROspeed = (unsigned short) i;
        break;
        case 'R':
            only_turret = FALSE;
            StopRes = Reset;
            // "Полный сброс"
            info(_("Reset"));
            break;
        case 'S':
            only_turret = FALSE;
            StopRes = Sleep;
            // "Спящий режим"
            info(_("Go to sleep"));
            break;
        case 's':
            save_image = FALSE;
            break;
        case 'T':
            only_T = TRUE;
            only_turret = FALSE;
            save_image = FALSE;
            // "только отобразить/задать температуру"
            info(_("only set/get temperature"));
            break;
        case 't':
            only_turret = FALSE;
            temperature = atof(optarg);
            if(temperature < -35. || temperature > 30.){
                // "Неверное значение температуры: %s (должно быть от -35 до 30)"
                usage(_("Wrong temperature: %s (must be from -35 to 30)"), optarg);
            }
            set_T = TRUE;
            // "Установить температуру: %.3f"
            info(_("Set temperature: %.3f"), temperature);
            break;
        case 'v':
            if (myatoi(&vbin, optarg) || vbin < 1){
                // "Неверный"
                usage("%s vbin: %s", _("Wrong"), optarg);
            }
            // "Верт. биннинг: %d"
            info(_("Vertical binning: %d"), vbin);
            break;
        case 'w':
            open_turret = TRUE;
            if(myatoi(&Nwheel, optarg) || Nwheel < 1)
                // "Номер турели должен быть больше 0"
                usage(_("Turret number should be positive value"));
            // "Работать с турелью %d"
            info(_("Work with turret %d"), Nwheel);
            break;
        case 'W':
            only_turret = FALSE;
            StopRes = Wake;
            // "Активировать камеру"
            info(_("Wakeup camera"));
            break;
        case 'x':
            only_turret = FALSE;
            if (myatoi(&exptime, optarg) || exptime < 0){
                // "Неверное время экспозиции: %s"
                usage(_("Wrong exposure time: %s"), optarg);
            }
            // "Время экспозиции: %dмс"
            info(_("Exposure time: %dms"), exptime);
            break;
        case 'X':
            getrange(&X0, &X1, optarg);
            // "Диапазон по X: [%d, %d]"
            info(_("X range: [%d, %d]"), X0, X1);
            break;
        case 'Y':
            getrange(&Y0, &Y1, optarg);
            // "Диапазон по Y: [%d, %d]"
            info(_("Y range: [%d, %d]"), Y0, Y1);
            break;
        default:
        usage(NULL);
        }
    }
    if(fake){
        only_turret = FALSE;
        save_image = FALSE;
    }
    argc -= optind;
    argv += optind;
    if(outfile == NULL){
        save_image = FALSE;
    }
    get_defhdrs(defhdr_filename);
    if(argc > 0){ // additional headers
        // "Дополнительные заголовки: "
        info(_("Additional headers: "));
        for (i = 0; i < argc; i++)
            info("%s ", argv[i]);
        add_morehdrs(argc, argv);
    }
    if(Shtr != -1) only_turret = FALSE;
}
