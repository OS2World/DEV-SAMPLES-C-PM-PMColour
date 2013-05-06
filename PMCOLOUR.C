#define	INCL_WIN
#include <os2.h>
#include <stdio.h>
#include <malloc.h>
#include "pmcolour.h"

#define	CWN_CHANGE	0x601
#define	CWM_SETCOLOUR	0x602

typedef struct _RGB {
    BYTE	bBlue;
    BYTE	bGreen;
    BYTE	bRed;
} RGB;

typedef	struct CWPARAM {
    USHORT	cb;
    RGB		rgb;
} CWPARAM;

typedef	struct CWDATA {
    USHORT	updatectl;
    HWND	hwndCol;
    HWND	hwndSpinR;
    HWND	hwndSpinG;
    HWND	hwndSpinB;
    RGB		*rgb;
    RGB		rgbold;
} CWDATA;

MRESULT EXPENTRY DlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);


main(void)
{
    HAB		hab;
    HMQ		hmq;
    HLIB	hlib;
    HWND	hwndDlg;
    CWPARAM	cwparam;
    char	msgbuf[64];

    hmq = WinCreateMsgQueue (hab = WinInitialize(0), 0);
    hlib = WinLoadLibrary (hab, "WPCONFIG.DLL");
    cwparam.cb = sizeof(CWPARAM);
    cwparam.rgb.bRed = 63;
    cwparam.rgb.bGreen = 127;
    cwparam.rgb.bBlue = 255;
    hwndDlg = WinLoadDlg (HWND_DESKTOP, HWND_DESKTOP, DlgProc, NULLHANDLE,
	ID_DLG, &cwparam);
    WinProcessDlg(hwndDlg);
    WinDestroyWindow(hwndDlg);
    sprintf (msgbuf, "Returned values are:\n  R = %u, G = %u, B = %u",
	cwparam.rgb.bRed, cwparam.rgb.bGreen, cwparam.rgb.bBlue);
    WinMessageBox (HWND_DESKTOP, HWND_DESKTOP, msgbuf, "Edit Colour", 0,
	MB_INFORMATION | MB_OK);
    WinDeleteLibrary (hab, hlib);
    WinDestroyMsgQueue(hmq);
    WinTerminate(hab);
    return(0);
}


MRESULT EXPENTRY DlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    CWDATA	*cwdata;
    HWND	hwndSpin;
    ULONG	newval;

    cwdata = (CWDATA *) WinQueryWindowULong (hwnd, QWL_USER);
    switch(msg)
    {
	case WM_INITDLG:
	    if (!(cwdata = malloc(sizeof(CWDATA))) ||
		!mp2 || !(cwdata->rgb = &(((CWPARAM *) mp2)->rgb)))
	    {
		WinPostMsg (hwnd, WM_CLOSE, MPVOID, MPVOID);
		break;
	    }

	    WinSetWindowULong (hwnd, QWL_USER, (ULONG) cwdata);
	    cwdata->rgbold = *cwdata->rgb;
	    cwdata->hwndCol = WinWindowFromID (hwnd, ID_COL);
	    cwdata->hwndSpinR = WinWindowFromID (hwnd, ID_SPINR);
	    cwdata->hwndSpinG = WinWindowFromID (hwnd, ID_SPING);
	    cwdata->hwndSpinB = WinWindowFromID (hwnd, ID_SPINB);
	    cwdata->updatectl = FALSE;
	    WinSendMsg (cwdata->hwndSpinR, SPBM_SETLIMITS,
		MPFROMSHORT(255), MPFROMSHORT(0));
	    WinSendMsg (cwdata->hwndSpinG, SPBM_SETLIMITS,
		MPFROMSHORT(255), MPFROMSHORT(0));
	    WinSendMsg (cwdata->hwndSpinB, SPBM_SETLIMITS,
		MPFROMSHORT(255), MPFROMSHORT(0));
	    WinSendMsg (hwnd, CWN_CHANGE,
		MPFROMLONG(*((ULONG *) cwdata->rgb) & 0xFFFFFF), MPVOID);
	    WinSendMsg (cwdata->hwndCol, CWM_SETCOLOUR,
		MPFROMLONG(*((ULONG *) cwdata->rgb) & 0xFFFFFF), MPVOID);
	    break;

	case WM_COMMAND:
	    switch(SHORT1FROMMP(mp1))
	    {
		case ID_UNDO:
		    WinSendMsg (hwnd, CWN_CHANGE,
			MPFROMLONG(*((ULONG *) &cwdata->rgbold) & 0xFFFFFF),
			MPVOID);
		    WinSendMsg (cwdata->hwndCol, CWM_SETCOLOUR,
			MPFROMLONG(*((ULONG *) &cwdata->rgbold) & 0xFFFFFF),
			MPVOID);
		    return ((MPARAM) 0);
	    }

	    break;

	case CWN_CHANGE:
	    *cwdata->rgb = *((RGB *) &mp1);
	    cwdata->updatectl = FALSE;
	    WinSendMsg (cwdata->hwndSpinR, SPBM_SETCURRENTVALUE,
		MPFROMSHORT(cwdata->rgb->bRed), MPVOID);
	    WinSendMsg (cwdata->hwndSpinG, SPBM_SETCURRENTVALUE,
		MPFROMSHORT(cwdata->rgb->bGreen), MPVOID);
	    WinSendMsg (cwdata->hwndSpinB, SPBM_SETCURRENTVALUE,
		MPFROMSHORT(cwdata->rgb->bBlue), MPVOID);
	    cwdata->updatectl = TRUE;
	    break;

	case WM_CONTROL:
            switch(SHORT1FROMMP(mp1))
            {
                case ID_SPINR:
                case ID_SPING:
                case ID_SPINB:
		    hwndSpin = WinWindowFromID (hwnd, SHORT1FROMMP(mp1));
                    switch(SHORT2FROMMP(mp1))
                    {
                        case SPBN_CHANGE:
                        case SPBN_KILLFOCUS:
			    if (cwdata->updatectl)
                            {
				WinSendMsg (hwndSpin, SPBM_QUERYVALUE,
				    (MPARAM) &newval,
				    MPFROM2SHORT(0, SPBQ_ALWAYSUPDATE));
				switch(SHORT1FROMMP(mp1))
				{
				    case ID_SPINR:
					cwdata->rgb->bRed = (BYTE) newval;
					break;

				    case ID_SPING:
					cwdata->rgb->bGreen = (BYTE) newval;
					break;

				    case ID_SPINB:
					cwdata->rgb->bBlue = (BYTE) newval;
				}

				WinSendMsg (cwdata->hwndCol, CWM_SETCOLOUR,
				    MPFROMLONG(*((ULONG *) cwdata->rgb) &
				    0xFFFFFF), MPVOID);
			    }
                    }
	    }

	    break;

	case WM_DESTROY:
	    free(cwdata);
    }

    return (WinDefDlgProc (hwnd, msg, mp1, mp2));
}
