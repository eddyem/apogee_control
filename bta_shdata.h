#pragma once
#ifndef __BTA_SHDATA_H__
#define __BTA_SHDATA_H__

#define _XOPEN_SOURCE 501
/* �������� ����������� � ������� ��������� ��������������� ���������� */
/*   ��������� ������� �����������:         */
/*     BTA_MODULE   - ��� ���-� � ���. C-������� (�� � �����.���������) */
/*     SHM_OLD_SIZE - ��� ��������� ���������� ����� ��������� ���-������ */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <errno.h>

#pragma GCC diagnostic ignored "-Wunused-function"
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"
#pragma GCC diagnostic ignored "-Wsequence-point"
#pragma GCC diagnostic ignored "-Wpointer-to-int-cast"
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration"

//#define int __int32_t
#define uint __uint32_t

struct SHM_Block {  /* �������� ����� ����������� ������ */
    union {
      char  name[5];          /* ���� ������������� �������� ������ */
      key_t code;
    } key;
    int size;                 /* ������ ������������ ����� � ������ */
    int maxsize;              /* ������ ��� �������� ("� �������" ��� ������� ������) */
    int mode;                 /* ����� ������� (rwxrwxrwx) */
    int atflag;               /* ����� ������������� (SHM_RDONLY ��� 0) */
    void (*init)();           /* ��������� ������������� */
    int  (*check)();          /* ��������� �������� */
    void (*close)();          /* ��������� ������������ */
    int side;                 /* ��� �������������: ������/������ */
    int id;                   /* ���������� ������������� */
    unsigned char *addr;      /* ����� ������������� */
};

struct CMD_Queue {  /* �������� ������� (������) ������ */
    union {
      char  name[5];          /* ���� ������������� ������� */
      key_t code;
    } key;
    int mode;                 /* ����� ������� (rwxrwxrwx) */
    int side;                 /* ��� �������������: ������/������ (Sender/Receiver)*/
    int id;                   /* ���������� ������������� */
    uint acckey;      /* ���� ������� (��� �������� ������->������) */
};

#ifndef BTA_MODULE
/* ����� ������ �������� ������������� ���������� (Level5)*/
struct CMD_Queue mcmd = {{'M','c','m','d',0},0200,0,-1,0};
/* ����� �������� ������������ (�����������������) ������ (Level4)*/
struct CMD_Queue ocmd = {{'O','c','m','d',0},0200,0,-1,0};
/* ����� �������� ���������������� (�������������������) ������ (Level2/3)*/
struct CMD_Queue ucmd = {{'U','c','m','d',0},0200,0,-1,0};
#else
extern struct CMD_Queue mcmd;
extern struct CMD_Queue ocmd;
extern struct CMD_Queue ucmd;
#endif

static void send_cmd_noarg(int);
static void send_cmd_str(int, char *);
static void send_cmd_i1(int, int);
static void send_cmd_i2(int, int, int);
static void send_cmd_i3(int, int, int, int);
static void send_cmd_i4(int, int, int, int, int);
static void send_cmd_d1(int, double);
static void send_cmd_d2(int, double, double);
static void send_cmd_i1d1(int, int, double);
static void send_cmd_i2d1(int, int, int, double);
static void send_cmd_i3d1(int, int, int, int, double);

/* ������ ������ */
/*          ���                           ���    ���������           ���   */
#define StopTel                           1 /* ������� ��������� */
#define StopTeleskope()   send_cmd_noarg( 1 )                     /* ����. */
#define StartHS                           2 /* ����� ������� ��������� */
#define StartHightSpeed() send_cmd_noarg( 2 )                     /* ����/���*/
#define StartLS                           3 /* ����� ������� ������� */
#define StartLowSpeed()   send_cmd_noarg( 3 )                     /* ����/���*/
#define SetTmr                            4 /* ���. Ch7_15 ��� SysTimer */
#define SetTimerMode(T)   send_cmd_i1   ( 4, (int)(T))            /* �.����. */
#define SetModMod                         5 /* ���. ����� ������������� */
#define SetModelMode(M)   send_cmd_i1   ( 5, (int)(M))            /* �.����. */
#define SetCodA                           6 /* ��� �������� �� A */
#define SetPKN_A(iA,sA)   send_cmd_i2   ( 6, (int)(iA),(int)(sA)) /* ����/���*/
#define SetCodZ                           7 /* ��� �������� �� Z */
#define SetPKN_Z(iZ)      send_cmd_i1   ( 7, (int)(iZ))           /* ����/���*/
#define SetCodP                           8 /* ��� �������� �� P */
#define SetPKN_P(iP)      send_cmd_i1   ( 8, (int)(iP))           /* ����/���*/
#define SetVA                             9 /* ���. �������� �� A */
#define SetSpeedA(vA)     send_cmd_d1   ( 9, (double)(vA))        /* ����/���*/
#define SetVZ                            10 /* ���. �������� �� Z */
#define SetSpeedZ(vZ)     send_cmd_d1   (10, (double)(vZ))        /* ����/���*/
#define SetVP                            11 /* ���. �������� �� P */
#define SetSpeedP(vP)     send_cmd_d1   (11, (double)(vP))        /* ����/���*/
#define SetAD                            12 /* ���.����� ���������� R.A.� Decl */
#define SetRADec(Alp,Del) send_cmd_d2   (12, (double)(Alp),(double)(Del))/* �����.*/
#define SetAZ                            13 /* ���.���������� ������ � ���.�����.*/
#define SetAzimZ(A,Z)     send_cmd_d2   (13, (double)(A),(double)(Z))/* �����.*/
#define GoToAD                           14 /* ����� ��������� �� ������ (�� R.A.� Decl)*/
#define GoToObject()      send_cmd_noarg(14 )                     /*  ����. */
#define MoveToAD                         15 /* ������� �� ������ (�� R.A.� Decl)*/
#define MoveToObject()    send_cmd_noarg(15 )                     /*  �����.*/
#define GoToAZ                           16 /* ��������� �� ��������� (�� A Z)*/
#define GoToAzimZ()       send_cmd_noarg(16 )                     /*  ����. */
#define WriteAZ                          17 /* ��������� A � Z ��� FullModel*/
#define WriteModelAZ()    send_cmd_noarg(17 )                     /* ����. */
#define SetModP                          18 /* ���. ����� ������������� P2 */
#define SetPMode(pmod)    send_cmd_i1   (18, (int)(pmod))         /* �����.*/
#define P2Move                           19 /* ���./����. (+-1,0) �������� P2 */
#define MoveP2(dir)       send_cmd_i1   (19, (int)(dir))          /* �����.*/
#define FocMove                          20 /* ���./����. (+-2,+-1,0) �������� ������ */
#define MoveFocus(speed,time) send_cmd_i1d1(20,(int)(speed),(double)(time))  /* �����.*/
#define UsePCorr                         21 /* ����� ����� �������� ��������� (���) */
#define SwitchPosCorr(pc_flag) send_cmd_i1 (21, (int)(pc_flag))   /* ����. */
#define SetTrkFlags                      22 /* ���. ������  ������ �������� */
#define SetTrkOkMode(trk_flags) send_cmd_i1 (22, (int)(trk_flags))   /* ����.*/
#define SetTFoc                          23 /* ���.������: 0-��, 1-�1, 2-�2 */
#define SetTelFocus(N)    send_cmd_i1  ( 23, (int)(N))            /* ����. */
#define SetVAD                           24 /* ���.��.�����.��-� ������� �� R.A.� Decl */
#define SetVelAD(VAlp,VDel) send_cmd_d2 (24, (double)(VAlp),(double)(VDel))/* ����.*/
#define SetRevA                          25 /* ���. ����� "������" ������� */
#define SetAzRevers(amod) send_cmd_i1   (25, (int)(amod))         /* �����.*/
#define SetVP2                           26 /* ���.����..��-� P2 (��� ���) */
#define SetVelP2(vP2) send_cmd_d1       (26, (double)(vP2))       /* �����.*/
#define SetTarg                          27 /* ���. ���� ��������� */
#define SetSysTarg(Targ) send_cmd_i1    (27, (int)(Targ))         /* ����.*/
#define SendMsg                          28 /* ���������� ��������� (���� �������� � � ��������) */
#define SendMessage(Mesg) send_cmd_str  (28, (char *)(Mesg))      /* �����.*/
#define CorrAD                           29 /* ��������� ��������� R.A.� Decl */
#define DoADcorr(dAlp,dDel) send_cmd_d2 (29, (double)(dAlp),(double)(dDel))/* �����.*/
#define CorrAZ                           30 /* ��������� ��������� A � Z*/
#define DoAZcorr(dA,dZ)    send_cmd_d2  (30, (double)(dA),(double)(dZ))/* �����.*/
#define SetVCAZ                          31 /* ���.����.��������� �� A � Z*/
#define SetVCorr(vA,vZ)   send_cmd_d2   (31, (double)(vA),(double)(vZ))/* �����.*/
#define P2MoveTo                         32 /* ������� P2 �� ������� */
#define MoveP2To(vP2,time) send_cmd_d2  (32, (double)(vP2),(double)(time))/* �����.*/
#define GoToTD                           33 /* ����� ��������� �� ��������� (�� t � Decl)*/
#define GoToSat()        send_cmd_noarg (33 )                     /*  ����..*/
#define MoveToTD                         34 /* ������� �� ��������� (�� t � Decl)*/
#define MoveToSat()      send_cmd_noarg (34 )                     /*  �����.*/
#define NullCom                          35 /* ������ ������� (��� �������������?) */
#define SyncCom()        send_cmd_noarg (35 )                     /* ����. */
#define StartTel                         36 /* ������ "����" ��������� */
#define StartTeleskope()  send_cmd_noarg(36 )                     /* ����. */
#define SetTMod                          37 /* ���. ������ ������ ��������� */
#define SetTelMode(M)     send_cmd_i1  ( 37, (int)(M))            /* �.����. */
#define TelOn                            38 /* ������ ���. �����, ��� � �.�.*/
#define TeleskopeOn()     send_cmd_noarg(38 )                     /* �.����. */
#define SetModD                          39 /* ���. ����� ������������� ������ */
#define SetDomeMode(dmod) send_cmd_i1   (39, (int)(dmod))         /* �.����.*/
#define DomeMove                         40 /* ���./����. (+-1+-2,+-3,0) �������� ������ */
#define MoveDome(speed,time) send_cmd_i1d1(40,(int)(speed),(double)(time))  /* ����.*/
#define SetPass                          41 /* ���. ������ ������ �������  */
#define SetPasswd(LPass)   send_cmd_str (41, (char *)(LPass))     /* �.����.*/
#define SetLevC                          42 /* ���. ��� ������ �������     */
#define SetLevCode(Nlev,Cod) send_cmd_i2(42, (int)(Nlev),(int)(Cod)) /* �.����.*/
#define SetLevK                          43 /* ���. ���� ������ �������    */
#define SetLevKey(Nlev,Key)  send_cmd_i2(43, (int)(Nlev),(int)(Key)) /* �.����.*/
#define SetNet                           44 /* ���. ����� � ����� �������  */
#define SetNetAcc(Mask,Addr) send_cmd_i2(44, (int)(Mask),(int)(Addr)) /* �.����.*/
#define SetMet                           45 /* ���� ����� ������ */
#define SetMeteo(m_id,m_val) send_cmd_i1d1(45,(int)(m_id),(double)(m_val))  /* ����.*/
#define TurnMetOff                       46 /* ������ ���. ����� ������ */
#define TurnMeteoOff(m_id)   send_cmd_i1 (46, (int)(m_id))         /* ����.*/
#define SetDUT1                          47 /* ���.����.�������(IERS DUT1=UT1-UTC) */
#define SetDtime(dT)         send_cmd_d1 (47, (double)(dT))       /* �.����.*/
#define SetPM                            48 /* ���.�����.������(IERS polar motion)*/
#define SetPolMot(Xp,Yp)     send_cmd_d2 (48, (double)(Xp),(double)(Yp)) /* �.����.*/
#define GetSEW                           49 /* ��������� SEW �������� */
#define GetSEWparam(Ndrv,Indx,Cnt) send_cmd_i3(49,(int)(Ndrv),(int)(Indx),(int)(Cnt))  /* M.����.*/
#define PutSEW                           50 /* �������� SEW �������� */
#define PutSEWparam(Ndrv,Indx,Key,Val) send_cmd_i4(50,(int)(Ndrv),(int)(Indx),(int)(Key),(int)(Val))  /* M.����.*/
#define SetLocks                         51 /* ��������� ���������� ���������� ������ */
#define SetLockFlags(f)      send_cmd_i1 (SetLocks, (int)(f))     /* M.����.*/
#define ClearLocks                       52 /* ������ ���������� ���������� ������ */
#define ClearLockFlags(f)    send_cmd_i1 (ClearLocks, (int)(f))   /* M.����.*/
#define SetRKbits                        53 /* ��������� ���.����� PEP-RK */
#define AddRKbits(f)         send_cmd_i1 (SetRKbits, (int)(f))    /* M.����.*/
#define ClrRKbits                        54 /* ������� ���.����� PEP-RK */
#define ClearRKbits(f)       send_cmd_i1 (ClrRKbits, (int)(f))    /* M.����.*/
#define SetSEWnd                         55 /* ���.����� SEW-������ ������ (��� ���������)*/
#define SetDomeDrive(ND)     send_cmd_i1 (SetSEWnd, (int)(ND))    /* �.����.*/
#define SEWsDome                         56 /* ���./����. SEW-������� ������ */
#define DomeSEW(OnOff)       send_cmd_i1 (SEWsDome, (int)(OnOff)) /* �.����.*/


/* ��������� ������ ��������� ���������� (������������� "���������� �������") */
#define BTA_Data_Ver 2
#pragma pack(4)
//struct __attribute__((packed)) BTA_Data {
struct BTA_Data {
   int magic;    /* ��� ��������� ��������� */
   int version;  /* ����� ������ ��������� = BTA_Data_Ver*/
   int size;     /* ������ ��������� = sizeof(struct BTA_Data)*/
   int pid;
#define ServPID  (sdt->pid) /*����� �������� ��.���.��������� */

  /* ������ ������ */
   int model;
#define UseModel  (sdt->model) /* ������� ������������� ������������� */
#define NoModel    0  /* ��������� */
#define CheckModel 1  /* ������� �������������� �� ������ */
#define DriveModel 2  /* ������������� �������� � "������" ���������� ��� �������� �������� */
#define FullModel  3  /* ������ ������������� ��� ��������� */
   int timer;
#define ClockType (sdt->timer) /* ����� ���� ������������ */
#define Ch7_15     0  /* �����.���� ���.��������� � �������������� �� �7-15 */
#define SysTimer   1  /* ������ ������� (���������� ������������������ ��� ���) */
#define ExtSynchro 2  /* �������� ������������� ������� ������� ������� ���������� (bta_time ��� xntpd)*/
   int system;
#define Sys_Mode (sdt->system) /* ����� ������ ������� */
#define SysStop     0  /* ������� */
#define SysWait     1  /* �������� ������ (���������) */
#define SysPointAZ  2  /* ��������� �� ��������� (�� A Z)*/
#define SysPointAD  3  /* ��������� �� ������ (�� R.A.� Decl)*/
#define SysTrkStop  4  /* �������: �������� ������ */
#define SysTrkStart 5  /* �������: ������ �� �����.�������� (�������)*/
#define SysTrkMove  6  /* �������: ������� �� ������ */
#define SysTrkSeek  7  /* �������: �������� �� �������� */
#define SysTrkOk    8  /* �������: ��������������� � ������� */
#define SysTrkCorr  9  /* �������: ��������� ��������� */
#define SysTest     10 /* ������������ */
   int sys_target;
#define Sys_Target (sdt->sys_target) /* ���� ��������� */
#define TagPosition 0  /* ��������� A/Z        */
#define TagObject   1  /* ������    Alpha/Delta */
#define TagNest     2  /* ��������� "������"   */
#define TagZenith   3  /* ��������� "�����"    */
#define TagHorizon  4  /* ��������� "��������" */
#define TagStatObj  5  /* "���������"  t/Delta */

   int tel_focus;
#define Tel_Focus (sdt->tel_focus) /* ��� ������ ���������: 0-��, 1-�1, 2-�2 */
#define Prime     0
#define Nasmyth1  1
#define Nasmyth2  2
   double pc_coeff[8];
#define  PosCor_Coeff  (sdt->pc_coeff) /* ����-�� ��� ��� ���.������ */

  /* ��������� ��������� */
#define Stopping  0    /*  �������   */
#define Pointing  1    /*  ��������� */
#define Tracking  2    /*  �������   */
   int tel_state;
#define  Tel_State (sdt->tel_state) /* ������� �������������� */
   int req_state;
#define  Req_State (sdt->req_state) /* ������������� ���������� */
   int tel_hard_state;
#define  Tel_Hardware (sdt->tel_hard_state) /* ��������� ��� */
#define Hard_Off 0                          /* ������� ��������� */
#define Hard_On  1                          /* �������� */

  /* ������ ������ ��������� */
   int tel_mode;
#define  Tel_Mode (sdt->tel_mode)
#define  Automatic  0      /* "�������" - ���������� �����*/
#define  Manual     1      /* "�/���.���." - ��������� ����� � � ���:*/
#define  ZenHor     2      /*    "�����-��������" - ������ ��� Z<5 � Z>80*/
#define  A_Move     4      /*    ������ �������� A  */
#define  Z_Move     8      /*      --- "" ---    Z  */
#define  Balance 0x10      /*    ������������ ����� */

  /* ���./����. ����� "������" ������� */
   int az_mode;
#define Az_Mode (sdt->az_mode)
#define Rev_Off 0    /* ���������� ��������� �� ��������� ��������� �� ������� */
#define Rev_On  1    /* ��������� � ��������� �� 360����. */

  /* ������ P2 */
   int p2_state;
#define P2_State (sdt->p2_state) /* �������� ��������� ������� P2 */
#define P2_Off   0     /*  �����  */
#define P2_On    1     /*  �����   */
#define P2_Plus  2     /*  ������ ���� � +   */
#define P2_Minus -2    /*  ������ ���� � -   */
   int p2_req_mode;
#define  P2_Mode (sdt->p2_req_mode) /* ����� ������������� P2 (����: ���/����)*/

  /* ��������� ������� ������ */
   int focus_state;
#define Foc_State (sdt->focus_state)
#define Foc_Off    0   /*  �����  */
#define Foc_Lplus  1   /*  ����. ���� � +   */
#define Foc_Lminus -1  /*  ����. ���� � -   */
#define Foc_Hplus  2   /*  ������ ���� � +   */
#define Foc_Hminus -2  /*  ������ ���� � -   */

  /* ��������� ������� ������ */
   int dome_state;
#define Dome_State (sdt->dome_state)
#define D_On     7   /*  �������������� ������������ � ���������� */
#define D_Off    0   /*  �����  */
#define D_Lplus  1   /*  ����. ���� � +   */
#define D_Lminus -1  /*  ����. ���� � -   */
#define D_Mplus  2   /*  ����.����. � +   */
#define D_Mminus -2  /*  ����.����. � -   */
#define D_Hplus  3   /*  ������ ���� � +  */
#define D_Hminus -3  /*  ������ ���� � -  */

/* ���� �������� ��������� (���) */
   int pcor_mode;
#define Pos_Corr (sdt->pcor_mode) /* ��������� ��������� ������� �� A/Z: ���/����*/
#define PC_Off   0     /* ����. */
#define PC_On    1     /* ���.  */

/* ����� ���/����. ��������� ������ �������� */
   int trkok_mode;
#define TrkOk_Mode (sdt->trkok_mode)
#define UseDiffVel 1 /* �����������&���� �������� �������� �������� �������� (~��������)*/
#define UseDiffAZ  2 /* �������� �� ��������������� (����� ���.��������.���������) */
#define UseDFlt    4 /* ���. ��������� ������� ��������������� */

  /* ��������� �������� */
   double i_alpha, i_delta;
#define  InpAlpha  (sdt->i_alpha)  /* ��������� ���������� R.A. (sec) */
#define  InpDelta  (sdt->i_delta)  /*       -- " --        Decl. (") */
   double s_alpha, s_delta;
#define  SrcAlpha  (sdt->s_alpha)  /* �������� ���������� R.A. (sec) */
#define  SrcDelta  (sdt->s_delta)  /*       -- " --        Decl. (") */
   double v_alpha, v_delta;
#define  VelAlpha  (sdt->v_alpha)  /* ��.�����.��-� ������� �� R.A. (sec/���) */
#define  VelDelta  (sdt->v_delta)  /*       -- " --            Decl. ("/���) */
   double i_azim, i_zdist;
#define  InpAzim   (sdt->i_azim)   /* ��� ��������� �� ������� (") */
#define  InpZdist  (sdt->i_zdist)  /*       -- " --    ���.�����. (") */

  /* ���������� �������� */
   double c_alpha, c_delta;
#define  CurAlpha  (sdt->c_alpha)  /* ������� ���������� R.A. (sec) */
#define  CurDelta  (sdt->c_delta)  /*     -- " --        Decl. (") */
   double tag_a, tag_z, tag_p;
#define  tag_A  (sdt->tag_a)    /* �������  A (") ������� */
#define  tag_Z  (sdt->tag_z)    /*  - " -   Z (")  - " -  */
#define  tag_P  (sdt->tag_p)    /*  - " -   P (")  - " -  */
   double pcor_a, pcor_z, refr_z;
#define  pos_cor_A  (sdt->pcor_a) /* �������� ��������� ������� �� A (") */
#define  pos_cor_Z  (sdt->pcor_z) /*       - " -      - " -     �� Z (") */
#define  refract_Z  (sdt->refr_z) /* �������� �� ��������� ��� �������  (") */
   double tcor_a, tcor_z, tref_z;
#define  tel_cor_A  (sdt->tcor_a) /* �������� ���.��������� ��������� ��������� �� A (") */
#define  tel_cor_Z  (sdt->tcor_z) /*       - " -       - " -       - " -        �� Z (") */
#define  tel_ref_Z  (sdt->tref_z) /* �������� ���.��������� �� ���������  (") */
   double diff_a, diff_z, diff_p;
#define  Diff_A  (sdt->diff_a)    /* �������-�(�������� �����) �� A (") */
#define  Diff_Z  (sdt->diff_z)    /*     - " -      - " -         Z (") */
#define  Diff_P  (sdt->diff_p)    /*     - " -      - " -         P (") */
   double vbasea,vbasez,vbasep;
#define  vel_objA (sdt->vbasea)   /* ������� �������� ������� �� A ("/���) */
#define  vel_objZ (sdt->vbasez)   /*       - " -        - " -    Z  - " -  */
#define  vel_objP (sdt->vbasep)   /*       - " -        - " -    P  - " -  */
   double diffva,diffvz,diffvp;
#define diff_vA  (sdt->diffva)    /* �������� �������� �������� ������� �� ������� */
#define diff_vZ  (sdt->diffvz)    /*       --  ""  --     --  ""  --    �� Z */
#define diff_vP  (sdt->diffvp)    /*       --  ""  --     --  ""  --    �� P */
   double speeda,speedz,speedp;
#define  speedA  (sdt->speeda)   /* �������� �� A ("/���) ��� ���������� �������� */
#define  speedZ  (sdt->speedz)   /*    - " -    Z      - " -                   */
#define  speedP  (sdt->speedp)   /*    - " -    P      - " -                   */
   double m_time_precip;
#define  Precip_time (sdt->m_time_precip)/* ������ ������� ��������� ������� (precipitations)*/
   unsigned char reserve[16];
#define  Reserve (sdt->reserve)    /* ��������� ����� */
   double rspeeda, rspeedz, rspeedp;
#define  req_speedA (sdt->rspeeda) /* �������� ("/���) �������� �� ������ A */
#define  req_speedZ (sdt->rspeedz) /*           - " -                     Z */
#define  req_speedP (sdt->rspeedp) /*           - " -                     P */
   double simvela, simvelz, simvelp, simvelf, simveld;
#define  mod_vel_A  (sdt->simvela) /* �������� �� A ("/���) ��������� */
#define  mod_vel_Z  (sdt->simvelz) /*    - " -    Z      - " -        */
#define  mod_vel_P  (sdt->simvelp) /*    - " -    P      - " -        */
#define  mod_vel_F  (sdt->simvelf) /*    - " -    F      - " -        */
#define  mod_vel_D  (sdt->simvelf) /*    - " -    D      - " -        */


  /* ���������� ��������� �������� � ������������ �� ��� �������� */
   uint kost;
#define  code_KOST (sdt->kost) /* ����. ��������� � ������ ��������� */
			      /* 0x8000 - ������ ������������� */
			      /* 0x4000 - ��������� ���. */
			      /* 0x2000 - ����� ������� */
			      /* 0x1000 - ��������� P2 ���.*/
			      /* 0x01F0 - ��.����. 0.2 0.4 1.0 2.0 5.0("/���) */
			      /* 0x000F - ����.����. +Z -Z +A -A */
   double m_time, s_time, l_time;
#define  M_time  (sdt->m_time) /* ������� ���������� ����� (������ UTC!)*/
#define  S_time  (sdt->s_time) /* ������� �������� ����� */
#define  L_time  (sdt->l_time) /* ����� ������ ��������� */
   uint ppndd_a, ppndd_z, ppndd_p, ppndd_b;
#define  ppndd_A  (sdt->ppndd_a) /* ��� ������� ����� (������� �������) A */
#define  ppndd_Z  (sdt->ppndd_z) /*          -  ""  -                   Z */
#define  ppndd_P  (sdt->ppndd_p) /*          -  ""  -                   P */
#define  ppndd_B  (sdt->ppndd_b) /* ��� ������� ����� ��������  */
   uint dup_a, dup_z, dup_p, dup_f, dup_d;
#define  dup_A  (sdt->dup_a) /* ��� ���� ������� ��� (������� �������) A */
#define  dup_Z  (sdt->dup_z) /*          -  ""  -                      Z */
#define  dup_P  (sdt->dup_p) /*          -  ""  -                      P */
#define  dup_F  (sdt->dup_f) /* ��� ���� ������� ��� ������ ���������  */
#define  dup_D  (sdt->dup_d) /* ��� ���� ������� ��� ��������� ������  */
   uint low_a, low_z, low_p, low_f, low_d;
#define  low_A  (sdt->low_a) /* 14�-� ��.���� ������� ������� A */
#define  low_Z  (sdt->low_z) /*          -  ""  -             Z */
#define  low_P  (sdt->low_p) /*          -  ""  -             P */
#define  low_F  (sdt->low_f) /* ��� ������� ������ ���������  */
#define  low_D  (sdt->low_d) /* ��� ������� ��������� ������  */
   uint code_a, code_z, code_p, code_b, code_f, code_d;
#define  code_A  (sdt->code_a) /* 23�-� ��.����  ������� A */
#define  code_Z  (sdt->code_z) /*   -  ""  -             Z */
#define  code_P  (sdt->code_p) /*   -  ""  -             P */
#define  code_B  (sdt->code_b) /* ���  �������� */
#define  code_F  (sdt->code_f) /* ��� ������� ������ ���������  */
#define  code_D  (sdt->code_d) /* ��� ������� ��������� ������  */
   uint adc[8];
#define  ADC(N) (sdt->adc[(N)]) /* ���� 8-�� ������� ��� PCL818 */
#define  code_T1 ADC(0)        /* ��� ������� �����. �����������*/
#define  code_T2 ADC(1)        /* ��� ������� ����������� ��� ���.*/
#define  code_T3 ADC(2)        /* ��� ������� ����������� ������� */
#define  code_Wnd ADC(3)       /* ��� ������� ����� */
   double val_a, val_z, val_p, val_b, val_f, val_d;
   double val_t1, val_t2, val_t3, val_wnd;
#define  val_A  (sdt->val_a)    /* ������  A (") */
#define  val_Z  (sdt->val_z)    /*  - " -  Z (") */
#define  val_P  (sdt->val_p)    /*  - " -  P (") */
#define  val_B  (sdt->val_b)    /* �������e (��.��.��.)*/
#define  val_F  (sdt->val_f)    /* ����� ��������� (��) */
#define  val_D  (sdt->val_d)    /* ��������� ������ (") */
#define  val_T1 (sdt->val_t1)   /* �����. �����������  (��.)*/
#define  val_T2 (sdt->val_t2)   /* ����������� ��� ���.(��.)*/
#define  val_T3 (sdt->val_t3)   /* ����������� ������� (��.)*/
#define  val_Wnd (sdt->val_wnd) /* ����� (�/���)*/
   double val_alp, val_del;
#define  val_Alp  (sdt->val_alp)    /* �������� �������� R.A. (sec) */
#define  val_Del  (sdt->val_del)    /*     -- " --       Decl. (") */

   double vel_a, vel_z, vel_p, vel_f, vel_d;
#define  vel_A  (sdt->vel_a) /* �������� �� A ("/���) ���������� */
#define  vel_Z  (sdt->vel_z) /*    - " -    Z      - " -         */
#define  vel_P  (sdt->vel_p) /*    - " -    P      - " -         */
#define  vel_F  (sdt->vel_f) /*    - " -    F      - " -         */
#define  vel_D  (sdt->vel_d) /*    - " -    D      - " -         */

  /* ������� ��������� ��������� ��������� */
#define MesgNum 3
#define MesgLen 39
    //struct __attribute__((packed)) SysMesg {
    struct SysMesg {
	 int seq_num;
	 char type;
#define MesgEmpty 0
#define MesgInfor 1
#define MesgWarn  2
#define MesgFault 3
#define MesgLog   4
	 char text[MesgLen];
    } sys_msg_buf[MesgNum];
#define Sys_Mesg(N) (sdt->sys_msg_buf[N])

  /* ���������� �������� */
  /* ���� ��������� ������� ������� ��� �������� */
   uint code_lev1,code_lev2,code_lev3,code_lev4,code_lev5;
#define  code_Lev1  (sdt->code_lev1) /* "��������� �����������" - ������ ���������� */
#define  code_Lev2  (sdt->code_lev2) /* "������� �����������" -  + ���� ���������   */
#define  code_Lev3  (sdt->code_lev3) /* "������� �����������" -  + A/Z-����-�, ���.P2/F */
#define  code_Lev4  (sdt->code_lev4) /* "��������" -  + ����/���� ������., ������������ */
#define  code_Lev5  (sdt->code_lev5) /* "������� ��������" - ���  �������� */
  /* ����������� �������� ������� */
   uint netmask, netaddr, acsmask, acsaddr;
#define  NetMask    (sdt->netmask) /* ����� ������� (������: 255.255.255.0) */
#define  NetWork    (sdt->netaddr) /* ����� ������� (��������: 192.168.3.0) */
#define  ACSMask    (sdt->acsmask) /* ����� ���-���� (��������: 255.255.255.0) */
#define  ACSNet     (sdt->acsaddr) /* ����� ���-���� (��������: 192.168.13.0) */

  /* ���� �����-������ */
   int meteo_stat;
#define  MeteoMode (sdt->meteo_stat)   /* ����� �������� � ����� ������*/
#define  INPUT_B   1     /* �������� *//* ����� ������� ����� ����� ������ */
#define  INPUT_T1  2     /* T-�������� */
#define  INPUT_T2  4     /* T-������������ */
#define  INPUT_T3  8     /* T-������� */
#define  INPUT_WND 0x10  /* �����     */
#define  INPUT_HMD 0x20  /* ��������� */
#define  SENSOR_B   (INPUT_B  <<8)   /* ����� ������� �����-�������� (e.g.�� CAN-����)*/
#define  SENSOR_T1  (INPUT_T1 <<8)
#define  SENSOR_T2  (INPUT_T2 <<8)
#define  SENSOR_T3  (INPUT_T3 <<8)
#define  SENSOR_WND (INPUT_WND<<8)
#define  SENSOR_HMD (INPUT_HMD<<8)
#define  ADC_B      (INPUT_B  <<16)  /* ����� ���������� � ��� ���.��������� */
#define  ADC_T1     (INPUT_T1 <<16)
#define  ADC_T2     (INPUT_T2 <<16)
#define  ADC_T3     (INPUT_T3 <<16)
#define  ADC_WND    (INPUT_WND<<16)
#define  ADC_HMD    (INPUT_HMD<<16)
#define  NET_B      (INPUT_B  <<24)  /* ����� ��������� ������ � ������������ �� ���� */
#define  NET_T1     (INPUT_T1 <<24)
#define  NET_WND    (INPUT_WND<<24)
#define  NET_HMD    (INPUT_HMD<<24)
   double inp_b, inp_t1, inp_t2, inp_t3, inp_wnd;
#define  inp_B  (sdt->inp_b)    /* �������e (��.��.��.)*/
#define  inp_T1 (sdt->inp_t1)   /* �����. �����������  (��.)*/
#define  inp_T2 (sdt->inp_t2)   /* ����������� ��� ���.(��.)*/
#define  inp_T3 (sdt->inp_t3)   /* ����������� ������� (��.)*/
#define  inp_Wnd (sdt->inp_wnd) /* ����� (�/���)*/

   double temper, press;
#define  Temper      (sdt->temper)  /* ����������� ������������ ��� ��������� */
#define  Pressure    (sdt->press)   /* �������� ������������ ��� ��������� */
   double m_time10, m_time15;
#define  Wnd10_time  (sdt->m_time10) /* ������ ������� ������ >=10�/���*/
#define  Wnd15_time  (sdt->m_time15) /* - " -   - " -  - " -  >=15�/���*/

  /* IERS DUT1  (��������: ftp://maia.usno.navy.mil/ser7/ser7.dat) */
   double dut1;
#define  DUT1  (sdt->dut1) /* �������� ��.���������� �������: DUT1 = UT1-UTC */

   double a_time, z_time, p_time;
#define  A_time  (sdt->a_time)  /* ������ ���������� ������� A */
#define  Z_time  (sdt->z_time)  /*  - " -   - " -     - " -  Z */
#define  P_time  (sdt->p_time)  /*  - " -   - " -     - " -  P */

   double speedain, speedzin, speedpin;
#define  speedAin  (sdt->speedain)  /* ���������� �������� ���-� �� A */
#define  speedZin  (sdt->speedzin)  /* ���������� �������� ���-� �� Z */
#define  speedPin  (sdt->speedpin)  /* ���������� �������� ���-� �� P2*/

   double acc_a, acc_z, acc_p, acc_f, acc_d;
#define  acc_A  (sdt->acc_a)    /* ��������� �� A ("/���^2) */
#define  acc_Z  (sdt->acc_z)    /*    - " -     Z   - " -   */
#define  acc_P  (sdt->acc_p)    /*    - " -     P   - " -   */
#define  acc_F  (sdt->acc_f)    /*    - " -     F   - " -   */
#define  acc_D  (sdt->acc_d)    /*    - " -     D   - " -   */

   uint code_sew;
#define  code_SEW  (sdt->code_sew) /* ��� ����.������� � SEW-������������ */

/* ��������� SEW-������������ */
//struct __attribute__((packed)) SEWdata {
struct SEWdata {
    int status;
    double set_speed;
    double mes_speed;
    double current;
    int index;
    union {
      unsigned char b[4];
      __uint32_t l;
    } value;
} sewdrv[3];
#define  statusSEW(Drv) (sdt->sewdrv[(Drv)-1].status)   /*��������� �����������*/
#define  statusSEW1     (sdt->sewdrv[0].status)
#define  statusSEW2     (sdt->sewdrv[1].status)
#define  statusSEW3     (sdt->sewdrv[2].status)
#define  speedSEW(Drv) (sdt->sewdrv[(Drv)-1].set_speed) /*������������� ��������*/
#define  speedSEW1     (sdt->sewdrv[0].set_speed)       /* ��/��� (rpm)*/
#define  speedSEW2     (sdt->sewdrv[1].set_speed)
#define  speedSEW3     (sdt->sewdrv[2].set_speed)
#define  vel_SEW(Drv) (sdt->sewdrv[(Drv)-1].mes_speed)  /*���������� �������� */
#define  vel_SEW1     (sdt->sewdrv[0].mes_speed)        /* ��/��� (rpm)*/
#define  vel_SEW2     (sdt->sewdrv[1].mes_speed)
#define  vel_SEW3     (sdt->sewdrv[2].mes_speed)
#define  currentSEW(Drv) (sdt->sewdrv[(Drv)-1].current) /*��� (�)*/
#define  currentSEW1     (sdt->sewdrv[0].current)
#define  currentSEW2     (sdt->sewdrv[1].current)
#define  currentSEW3     (sdt->sewdrv[2].current)
#define  indexSEW(Drv) (sdt->sewdrv[(Drv)-1].index)     /*����� ���������*/
#define  indexSEW1     (sdt->sewdrv[0].index)
#define  indexSEW2     (sdt->sewdrv[1].index)
#define  indexSEW3     (sdt->sewdrv[2].index)
#define  valueSEW(Drv) (sdt->sewdrv[(Drv)-1].value.l) /*��� �������� ���������*/
#define  valueSEW1     (sdt->sewdrv[0].value.l)
#define  valueSEW2     (sdt->sewdrv[1].value.l)
#define  valueSEW3     (sdt->sewdrv[2].value.l)
#define  bvalSEW(Drv,Nb) (sdt->sewdrv[(Drv)-1].value.b[Nb]) /*���� ���� �������� ���������*/

/* ���������� �� PEP-������������ */
   uint pep_code_a, pep_code_z, pep_code_p;
#define  PEP_code_A  (sdt->pep_code_a) /* 23�-� ��.����  ������� A */
#define  PEP_code_Z  (sdt->pep_code_z) /*   -  ""  -             Z */
#define  PEP_code_P  (sdt->pep_code_p) /*   -  ""  -             P */
   uint pep_sw_a, pep_sw_z, pep_sw_p;
#define  switch_A  (sdt->pep_sw_a)     /* ��� ���������� ������� */
#define  Sw_minus_A    1               /* ������ ������������� (��. code_KOST&0x8000)*/
#define  Sw_plus240_A  2               /* �������� "+240����" */
#define  Sw_minus240_A 4               /* �������� "-240����" */
#define  Sw_minus45_A  8               /* ��������� "� ��������" (~-46����)*/
#define  switch_Z  (sdt->pep_sw_z)     /* ��� ���������� Z   */
#define  Sw_0_Z    0x01                /* �������� "0����" */
#define  Sw_5_Z    0x02                /* �������� "5����" */
#define  Sw_20_Z   0x04                /* �������� "20����" */
#define  Sw_60_Z   0x08                /* �������� "60����" */
#define  Sw_80_Z   0x10                /* �������� "80����" */
#define  Sw_90_Z   0x20                /* �������� "90����" */
#define  switch_P  (sdt->pep_sw_p)     /*    -  ""  -    ��� */
#define  Sw_No_P   0x00                /* "��� ����������"  */
#define  Sw_22_P   0x01                /* �������� "22����" */
#define  Sw_89_P   0x02                /* �������� "89����" */
#define  Sw_Sm_P   0x80                /* ������ ���� ���   */
   uint pep_code_f, pep_code_d, pep_code_ri, pep_code_ro;
#define  PEP_code_F  (sdt->pep_code_f) /* ��� ������� ������ ���������  */
#define  PEP_code_D  (sdt->pep_code_d) /* ��� ������� ��������� ������  */
#define  PEP_code_Rin  (sdt->pep_code_ri)/* ��� �������� �� ��  */
#define  PEP_code_Rout (sdt->pep_code_ro)/* ��� ���������� � ��  */
   unsigned char pep_on[10];           /* ����� ������ PEP-������������ */
#define PEP_A_On  (sdt->pep_on[0])
#define PEP_A_Off (PEP_A_On==0)
#define PEP_Z_On  (sdt->pep_on[1])
#define PEP_Z_Off (PEP_Z_On==0)
#define PEP_P_On  (sdt->pep_on[2])
#define PEP_P_Off (PEP_P_On==0)
#define PEP_F_On  (sdt->pep_on[3])
#define PEP_F_Off (PEP_F_On==0)
#define PEP_D_On  (sdt->pep_on[4])
#define PEP_D_Off (PEP_D_On==0)
#define PEP_R_On  (sdt->pep_on[5])
#define PEP_R_Off ((PEP_R_On&1)==0)
#define PEP_R_Inp ((PEP_R_On&2)!=0)
#define PEP_K_On  (sdt->pep_on[6])
#define PEP_K_Off ((PEP_K_On&1)==0)
#define PEP_K_Inp ((PEP_K_On&2)!=0)

  /* IERS polar motion (��������: ftp://maia.usno.navy.mil/ser7/ser7.dat) */
   double xpol, ypol;
#define  polarX (sdt->xpol)  /* X-�������a �����.������  */
#define  polarY (sdt->ypol)  /* Y-�������a �����.������  */

   double  jdate, eetime;
#define JDate   (sdt->jdate)  /* ������� ��������� ���� */
#define EE_time (sdt->eetime) /* ������.��.��. �� "Equation of the Equinoxes" */

  /* ��� ���� �����-������ */
   double val_hmd, inp_hmd;
#define  val_Hmd (sdt->val_hmd) /* �������� ��������� (%) */
#define  inp_Hmd (sdt->val_hmd) /* ������ ���� */

  /* ��������� ������� (��������) */
   double worm_a, worm_z;
#define  worm_A (sdt->worm_a)   /* ��������� �������� A (���) */
#define  worm_Z (sdt->worm_z)   /* ��������� �������� Z (���) */

  /* ����� ���������� ���������� ������ */
   __uint32_t lock_flags;
#define LockFlags  (sdt->lock_flags)
#define Lock_A     0x01
#define Lock_Z     0x02
#define Lock_P     0x04
#define Lock_F     0x08
#define Lock_D     0x10
#define A_Locked   (LockFlags&Lock_A)
#define Z_Locked   (LockFlags&Lock_Z)
#define P_Locked   (LockFlags&Lock_P)
#define F_Locked   (LockFlags&Lock_F)
#define D_Locked   (LockFlags&Lock_D)

  /* ��������� �������� ������� ������ (��� ���-� SEW-���������)*/
   int sew_dome_speed;                    /* ���� ���� ��� � Dome_State */
#define Dome_Speed (sdt->sew_dome_speed)  /* �.�. D_Lplus,D_Lminus,.... */

/* ����� SEW-������ ������ (��� ���������)*/
   int sew_dome_num;
#define DomeSEW_N  (sdt->sew_dome_num)

/* ��������� ����������(DomeSEW_N) SEW-�����������  ������*/
struct SEWdata  sewdomedrv;
#define  statusSEWD  (sdt->sewdomedrv.status)    /*��������� �����������*/
#define  speedSEWD   (sdt->sewdomedrv.set_speed) /*������������� �������� ��/��� (rpm)*/
#define  vel_SEWD    (sdt->sewdomedrv.mes_speed) /*���������� �������� ��/��� (rpm)*/
#define  currentSEWD (sdt->sewdomedrv.current)   /*��� (�)*/
#define  indexSEWD   (sdt->sewdomedrv.index)     /*����� ���������*/
#define  valueSEWD   (sdt->sewdomedrv.value.l)   /*��� �������� ���������*/

/* ���������� PEP-����������� ������ */
   uint pep_code_di, pep_code_do;
#define PEP_code_Din  (sdt->pep_code_di)    /* ��� �������� �� PEP-������  */
#define PEP_Dome_SEW_Ok   0x200
#define PEP_Dome_Cable_Ok 0x100
#define PEP_code_Dout (sdt->pep_code_do)    /* ��� ���������� � PEP-������ */
#define PEP_Dome_SEW_On  0x10
#define PEP_Dome_SEW_Off 0x20

};

#ifndef BTA_MODULE
struct BTA_Data *sdt;
#else
extern struct BTA_Data *sdt;
#endif

struct BTA_Local { /* ��������� ��������� ������ */
   unsigned char reserve[120]; /* ��������� ����� ��� ���������� ���������� ������� */
			       /*  (�� ���������� ������� ����������� 1500 ����) */
   double pr_oil_a,pr_oil_z,pr_oil_t;
#define PressOilA    (sdtl->pr_oil_a)  /* �������� � ������������ A (���) */
#define PressOilZ    (sdtl->pr_oil_z)  /* �������� � ������������ Z (���) */
#define PressOilTank (sdtl->pr_oil_t)  /* ������ ������ ����� � ����(���) */
   double t_oil_1,t_oil_2;
#define OilTemper1   (sdtl->t_oil_1)   /* ����������� ����� */
#define OilTemper2   (sdtl->t_oil_2)   /* ����������� ����������� ���� */
};

#ifndef BTA_MODULE
struct BTA_Local *sdtl; /* ����� ����������, ������ ��������� ������ */
#else
extern struct BTA_Local *sdtl;
#endif

#define ClientSide 0
#define ServerSide 1

#ifndef BTA_MODULE
static void bta_data_init();
static int  bta_data_check();
static void bta_data_close();

/* �������� ����� ������ ��������� ���������� ("���������� �������") */
struct SHM_Block sdat = {
{'S','d','a','t',0},sizeof(struct BTA_Data),2048,0444,SHM_RDONLY,bta_data_init,bta_data_check,bta_data_close,0,-1,NULL
};
#else
extern struct SHM_Block sdat;
#endif

#ifndef BTA_MODULE
/* ������������� ������ ��������� ���������� (��������� "���������� �������") */
static void bta_data_init() {
   int i;
   sdt = (struct BTA_Data *)sdat.addr;
   sdtl = (struct BTA_Local *)(sdat.addr+sizeof(struct BTA_Data));
   if(sdat.side == ClientSide) {
      if(sdt->magic != sdat.key.code) {
	 fprintf(stderr,"Wrong shared data (maybe server turned off)\n");
       /*exit(1);*/
      }
      if(sdt->version == 0) {
	 fprintf(stderr,"Null shared data version (maybe server turned off)\n");
       /*exit(1);*/
      }
      else if(sdt->version != BTA_Data_Ver) {
	 fprintf(stderr,"Wrong shared data version: I'am - %d, but server - %d ...\n",
			 BTA_Data_Ver, sdt->version );
       /*exit(1);*/
      }
      if(sdt->size != sdat.size) {
	 if(sdt->size > sdat.size) {
	 /* �� ������ ����� ����� ������������ ��������� ����� ������ */
	    fprintf(stderr,"Wrong shared area size: I needs - %d, but server - %d ...\n",
			    sdat.size, sdt->size );
	 } else {
	 /* "���������" � ������ ��������� ����� ������ ���� */
	 /*   ����� ��������� �� ������� ������ �������!     */
	    fprintf(stderr,"Attention! Too little shared data structure!\n");
	    sleep(1);
	    fprintf(stderr,"I needs - %d, but server gives only %d ...\n",
			    sdat.size, sdt->size );
	    sleep(1);
	    fprintf(stderr,"May be server's version too old!?\n");
	  /* exit(1); */

	 }
      }
      return;
   }
   /* ServerSide */
   if(sdt->magic != sdat.key.code  ||
      sdt->version != BTA_Data_Ver ||
      sdt->size != sdat.size) {

      for(i=0; i<sdat.maxsize; i++) sdat.addr[i]=0;
      sdt->magic = sdat.key.code;
      sdt->version = BTA_Data_Ver;
      sdt->size = sdat.size;
      ServPID = 0;
      UseModel = NoModel;
      ClockType = Ch7_15;
      Sys_Mode = SysStop;
      Sys_Target = TagPosition;
      Tel_Focus = Prime;
      Tel_Hardware = Hard_On;
      Tel_Mode = Automatic;
      Az_Mode = Rev_Off;
      P2_State = P2_Mode = P2_Off;
      Foc_State = Foc_Off;
      Dome_State = D_Off;
      Pos_Corr = PC_On;
      TrkOk_Mode = UseDiffVel | UseDiffAZ ;
      InpAlpha=InpDelta= 0.;
      SrcAlpha=SrcDelta= 0.;
      VelAlpha=VelDelta= 0.;
      CurAlpha=CurDelta= 0.;
      InpAzim=InpZdist = 0.;
      Diff_A=Diff_Z=Diff_P=0.0;
      pos_cor_A=pos_cor_Z=refract_Z = 0.;
      tel_cor_A=tel_cor_Z=tel_ref_Z = 0.;
      vel_objA=vel_objZ=vel_objP = 0.;
      diff_vA=diff_vZ=diff_vP=0.;
      speedA = speedZ = speedP = 0.;
      req_speedA = req_speedZ = req_speedP = 0.;
      mod_vel_A=mod_vel_Z=mod_vel_P=mod_vel_F=mod_vel_D=0.;
      code_KOST = 0;
      M_time = S_time = L_time = 0.;
      ppndd_A=ppndd_Z=ppndd_P=ppndd_B=0;
      dup_A=dup_Z=dup_P=dup_F=dup_D=0;
      low_A=low_Z=low_P=low_F=low_D=0;
      code_A=code_Z=code_P=code_B=code_F=code_D=code_T1=code_T2=code_T3=code_Wnd=0;
      val_A=val_Z=val_P=val_B=val_F=val_D=val_T1=val_T2=val_T3=val_Wnd=val_Alp=val_Del=0.;
      vel_A=vel_Z=vel_P=vel_F=vel_D=0.;
      for(i=0; i<MesgNum; i++) {
	 Sys_Mesg(i).seq_num = 0;
	 Sys_Mesg(i).type = MesgEmpty;
	 Sys_Mesg(i).text[0] = '\0';
      }
      code_Lev1=code_Lev2=code_Lev3=code_Lev4=code_Lev5=0;
      NetMask=NetWork=ACSMask=ACSNet=0;
      MeteoMode = 0;
      inp_B = 591.;
      inp_T1 = inp_T2 = inp_T3 = inp_Wnd = 0.0;
      Temper  = 0.0;
      Pressure  = 595.;
      Wnd10_time = Wnd15_time = Precip_time = 0.0;
      DUT1 = 0.0;
      A_time = P_time = Z_time = 0.0;
      speedAin = speedZin = speedZin = 0.0;
      acc_A = acc_Z = acc_P = 0.0;
      for(i=1; i<4; i++) {
	statusSEW(i)  = 0;
	speedSEW(i)   = 0.0;
	vel_SEW(i)    = 0.0;
	currentSEW(i) = 0.0;
	indexSEW(i)   = 0;
	valueSEW(i)   = 0;
      }
      PEP_code_A = 0x002aaa;
      PEP_code_Z = 0x002aaa;
      PEP_code_P = 0x002aaa;
      PEP_code_F = 0x002aaa;
      PEP_code_D = 0x002aaa;
      PEP_code_Rin = 0x0;
      PEP_code_Rout = 0x0;
      switch_A = switch_Z = switch_P = 0;
      PEP_A_On = PEP_Z_On = PEP_P_On = PEP_F_On = PEP_D_On = PEP_R_On = PEP_K_On = 0;
      polarX = polarY = 0;
      JDate=0.;
      EE_time = 0.0;
      worm_A = worm_Z = 0.0;
      val_Hmd = inp_Hmd = 0.0;
      LockFlags = 0;
      Dome_Speed = D_Off;
      DomeSEW_N=1;
      statusSEWD  = 0;
      speedSEWD   = 0.0;
      vel_SEWD    = 0.0;
      currentSEWD = 0.0;
      indexSEWD   = 0;
      valueSEWD   = 0;
      PEP_code_Din = 0x0;
      PEP_code_Dout = 0x0;

      PressOilA = PressOilZ = PressOilTank = 0.0;
      OilTemper1 = OilTemper2 = 0.0;
   }
}
static int  bta_data_check() {
   return( (sdt->magic == sdat.key.code) && (sdt->version == BTA_Data_Ver) );
}
static void bta_data_close() {
   if(sdat.side == ServerSide) {
      sdt->magic = 0;
      sdt->version = 0;
   }
}

/* ���� �������������� ��������� ??? */
/*struct SHM_Block info = {{'I','n','f','o',0},1024,1024,0444,SHM_RDONLY,NULL,NULL,NULL,0,-1,NULL};*/

/* Allocate shared memory segment */
static int get_shm_block( struct SHM_Block *sb, int server) {
   int getsize = (server)? sb->maxsize : sb->size;

   /* first try to find existing one */
   sb->id = shmget(sb->key.code, getsize, sb->mode);

   if (sb->id<0 && errno==ENOENT && server) {
      /* if no - try to create a new one */
      int cresize = sb->maxsize;
      if(sb->size > cresize) {
	 fprintf(stderr,"Wrong shm maxsize(%d) < realsize(%d)\n",sb->maxsize,sb->size);
	 cresize = sb->size;
      }
      sb->id = shmget(sb->key.code, cresize, IPC_CREAT|IPC_EXCL|sb->mode);
   }
   if (sb->id<0) {
      char msg[80];
      if(server)
	 sprintf(msg,"Can't create shared memory segment '%s'",sb->key.name);
      else
	 sprintf(msg,"Can't find shared segment '%s' (maybe no server process) ",sb->key.name);
      perror(msg);
      return 0;
   }
   /* attach it to our memory space */
   sb->addr = (unsigned char *)shmat ( sb->id, NULL, sb->atflag );

   if ((int)(sb->addr) == -1) {
      char msg[80];
      sprintf(msg,"Can't attach shared memory segment '%s'",sb->key.name);
      perror(msg);
      return 0;
   }
   if(server) {
      if((shmctl(sb->id, SHM_LOCK, NULL) < 0) < 0) {
	 char msg[80];
	 sprintf(msg,"Can't prevents swapping of shared memory segment '%s'",sb->key.name);
	 perror(msg);
	 return 0;
      }
   }
   fprintf(stderr,"Create&attach shared memory segment '%s' %dbytes at %x \n",
	   sb->key.name, sb->size, (uint)sb->addr);

   sb->side = server;

   if(sb->init!=NULL)
      sb->init();
    return 1;
}
static int close_shm_block( struct SHM_Block *sb) {
   int ret;
   if(sb->close != NULL)
      sb->close();
   if(sb->side == ServerSide) {
//      ret = shmctl(sb->id, SHM_UNLOCK, NULL);
    ret = shmctl(sb->id, IPC_RMID, NULL);
   }
   ret = shmdt (sb->addr);
   return(ret);
}
#endif

static int check_shm_block( struct SHM_Block *sb) {
   if(sb->check != NULL)
      return(sb->check());
   else return(0);
}

#ifndef BTA_MODULE
int snd_id=-1; /* ������� (� ������������?) ����� ������� ������ ������� */
int cmd_src_pid=0; /* ����� �������� ��������� ��� ����� ����.������� */
__uint32_t cmd_src_ip=0;  /* IP-���. ��������� ��� ����� ����.������� */
#else
extern int snd_id;
extern int cmd_src_pid;
extern __uint32_t cmd_src_ip;
#endif

#ifndef BTA_MODULE
/* Create|Find command queue */
static void get_cmd_queue( struct CMD_Queue *cq, int server) {
   if (!server && cq->id>=0) {  /* if already in use */
      snd_id = cq->id;          /*    set current... */
      return;
   }
   /* first try to find existing one */
   cq->id = msgget(cq->key.code, cq->mode);
   if (cq->id<0 && errno==ENOENT && server)
      /* if no - try to create a new one */
      cq->id = msgget(cq->key.code, IPC_CREAT|IPC_EXCL|cq->mode);
   if (cq->id<0) {
      char msg[80];
      if(server)
	 sprintf(msg,"Can't create comand queue '%s'",cq->key.name);
      else
	 sprintf(msg,"Can't find comand queue '%s' (maybe no server process) ",cq->key.name);
      perror(msg);
      return;
   }
   cq->side = server;
   if (server) {
      char buf[120];  /* ��������� ��� ������� �� ������� */
      while(msgrcv(cq->id, (struct msgbuf *)buf, 112, 0, IPC_NOWAIT)>0);
   } else
      snd_id = cq->id;
   cq->acckey = 0;
}
#endif

/* ��������� ����� ������� � ���. ������ */
static void set_acckey(uint newkey) {
   if (snd_id<0) return;
   if(ucmd.id==snd_id)      ucmd.acckey=newkey;
   else if(ocmd.id==snd_id) ocmd.acckey=newkey;
   else if(mcmd.id==snd_id) mcmd.acckey=newkey;
}

/* ��������� ������ ��������� ��� ����� ����.������� */
/* ���� �� �������� ���������: IP=0(��������� �������) � PID �����.�������� */
static void set_cmd_src(__uint32_t ip, int pid) {
    cmd_src_pid = pid;
    cmd_src_ip = ip;
}

/* ��������� ��������� */
struct my_msgbuf {
   __int32_t mtype;            /* type of message */
   __uint32_t acckey;  /* ���� ������� ������� */
   __uint32_t src_pid; /* ����� �������� ��������� */
   __uint32_t src_ip;  /* IP-���. ���������, =0 - ��������� ������� */
   char mtext[100];       /* message text */
};
/* ������� ������ ������� � ������� */
static void send_cmd(int cmd_code, char *buf, int size) {
   struct my_msgbuf mbuf;

   if (snd_id<0) return;
   if (size>100) size=100;
   if (cmd_code>0)
      mbuf.mtype = cmd_code;
   else
      return;
   if(ucmd.id==snd_id)      mbuf.acckey=ucmd.acckey;
   else if(ocmd.id==snd_id) mbuf.acckey=ocmd.acckey;
   else if(mcmd.id==snd_id) mbuf.acckey=mcmd.acckey;

   mbuf.src_pid = cmd_src_pid? cmd_src_pid : getpid();
   mbuf.src_ip = cmd_src_ip;
   cmd_src_pid = cmd_src_ip = 0;

   if(size>0)
      memcpy( mbuf.mtext, buf, size);
   else {
      mbuf.mtext[0] = 0;
      size = 1;
   }
   msgsnd( snd_id, (struct msgbuf *)&mbuf, size+12, IPC_NOWAIT);
}
static void send_cmd_noarg(int cmd_code) {
    send_cmd(cmd_code, NULL, 0);
}
static void send_cmd_str(int cmd_code, char *arg) {
    send_cmd(cmd_code, arg, strlen(arg)+1);
}
static void send_cmd_i1(int cmd_code, int arg1) {
    send_cmd(cmd_code, (char *)&arg1, sizeof(int));
}
static void send_cmd_i2(int cmd_code, int arg1, int arg2) {
    int ibuf[2];
    ibuf[0] = arg1;
    ibuf[1] = arg2;
    send_cmd(cmd_code, (char *)ibuf, 2*sizeof(int));
}
static void send_cmd_i3(int cmd_code, int arg1, int arg2, int arg3) {
    int ibuf[3];
    ibuf[0] = arg1;
    ibuf[1] = arg2;
    ibuf[2] = arg3;
    send_cmd(cmd_code, (char *)ibuf, 3*sizeof(int));
}
static void send_cmd_i4(int cmd_code, int arg1, int arg2, int arg3, int arg4) {
    int ibuf[4];
    ibuf[0] = arg1;
    ibuf[1] = arg2;
    ibuf[2] = arg3;
    ibuf[3] = arg4;
    send_cmd(cmd_code, (char *)ibuf, 4*sizeof(int));
}
static void send_cmd_d1(int cmd_code, double arg1) {
    send_cmd(cmd_code, (char *)&arg1, sizeof(double));
}
static void send_cmd_d2(int cmd_code, double arg1, double arg2) {
    double dbuf[2];
    dbuf[0] = arg1;
    dbuf[1] = arg2;
    send_cmd(cmd_code, (char *)dbuf, 2*sizeof(double));
}
static void send_cmd_i1d1(int cmd_code, int arg1, double arg2) {
    struct {
       int ival;
       double dval;
    } buf;
    buf.ival = arg1;
    buf.dval = arg2;
    send_cmd(cmd_code, (char *)&buf, sizeof(buf));
}
static void send_cmd_i2d1(int cmd_code, int arg1, int arg2, double arg3) {
    struct {
       int ival[2];
       double dval;
    } buf;
    buf.ival[0] = arg1;
    buf.ival[1] = arg2;
    buf.dval = arg3;
    send_cmd(cmd_code, (char *)&buf, sizeof(buf));
}
static void send_cmd_i3d1(int cmd_code, int arg1, int arg2, int arg3, double arg4) {
    struct {
       int ival[3];
       double dval;
    } buf;
    buf.ival[0] = arg1;
    buf.ival[1] = arg2;
    buf.ival[2] = arg3;
    buf.dval = arg4;
    send_cmd(cmd_code, (char *)&buf, sizeof(buf));
}

static void encode_lev_passwd(char *passwd, int nlev, uint *keylev, uint *codlev) {
    char salt[4];
    char *encr;
    union {
      uint ui;
      char c[4];
    } key,cod;
    sprintf(salt,"L%1d",nlev);
    encr = (char *)crypt(passwd, salt);
    cod.c[0] = encr[2];
    key.c[0] = encr[3];
    cod.c[1] = encr[4];
    key.c[1] = encr[5];
    cod.c[2] = encr[6];
    key.c[2] = encr[7];
    cod.c[3] = encr[8];
    key.c[3] = encr[9];
    *keylev = key.ui;
    *codlev = cod.ui;
}

static int find_lev_passwd(char *passwd, uint *keylev, uint *codlev) {
    int nlev;
    for(nlev=5; nlev>0; nlev--) {
       encode_lev_passwd(passwd, nlev, keylev, codlev);
       if(nlev == 1 && code_Lev1 == *codlev)  break;
       if(nlev == 2 && code_Lev2 == *codlev)  break;
       if(nlev == 3 && code_Lev3 == *codlev)  break;
       if(nlev == 4 && code_Lev4 == *codlev)  break;
       if(nlev == 5 && code_Lev5 == *codlev)  break;
    }
    return(nlev);
}

static int check_lev_passwd(char *passwd) {
    uint keylev,codlev;
    int nlev;
    nlev = find_lev_passwd(passwd, &keylev, &codlev);
    if(nlev>0) set_acckey(keylev);
    return(nlev);
}

#pragma GCC diagnostic pop

#endif // __BTA_SHDATA_H__
