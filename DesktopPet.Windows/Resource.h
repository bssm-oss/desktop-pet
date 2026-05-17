#pragma once

#ifndef IDC_STATIC
#define IDC_STATIC          (-1)
#endif

#define IDR_TRAY_ICON       101
#define IDD_SETTINGS        201
#define IDC_CHK_PLAYING     2001
#define IDC_SLD_SPEED       2002
#define IDC_SLD_OPACITY     2003
#define IDC_SLD_SCALE       2004
#define IDC_CHK_ONTOP       2005
#define IDC_CHK_CLICKTHRU   2006
#define IDC_CHK_LOCKPOS     2007
#define IDC_CHK_FLIPH       2008
#define IDC_CHK_FLIPV       2009
#define IDC_LBL_SPEED       2010
#define IDC_LBL_OPACITY     2011
#define IDC_LBL_SCALE       2012
#define IDC_EDT_LABEL       2013
#define IDC_BTN_IMPORT      2014
#define IDC_BTN_REMOVE      2015
#define IDC_BTN_QUIT        2016
#define IDC_CHK_STARTUP     2017

#define WM_TRAYICON         (WM_USER + 100)
#define WM_PET_ADVANCE      (WM_USER + 101)
#define WM_PET_SETTINGS     (WM_USER + 102)
#define WM_PET_REMOVE       (WM_USER + 103)
#define WM_PET_ADD          (WM_USER + 104)
#define WM_PET_QUIT         (WM_USER + 105)
#define WM_APP_REMOVE_PET   (WM_USER + 300)
#define WM_CMD_STARTUP      (WM_USER + 106)
#define WM_PET_IMPORT_IDLE  (WM_USER + 400)
#define WM_PET_IMPORT_GESTURE (WM_USER + 500)
#define WM_PET_IMPORT_REACTION (WM_USER + 600)
#define WM_PET_IMPORT_LOCOMOTION (WM_USER + 700)
#define WM_PET_IMPORT_FOCUS (WM_USER + 800)
#define WM_PET_IMPORT_SPECIAL (WM_USER + 900)
#define WM_PET_PLAY_IDLE    (WM_USER + 1000)
#define WM_PET_PLAY_GESTURE (WM_USER + 1100)
#define WM_PET_PLAY_REACTION (WM_USER + 1200)
#define WM_PET_PLAY_LOCOMOTION (WM_USER + 1300)
#define WM_PET_PLAY_FOCUS   (WM_USER + 1400)
#define WM_PET_PLAY_SPECIAL (WM_USER + 1500)

#define TIMER_ANIMATION     1
#define TIMER_DISPLAY       2
