/****************************************************************************/
/*                                                                          */
/*   metapad 3.6                                                            */
/*                                                                          */
/*   Copyright (C) 1999-2011 Alexander Davidson                             */
/*                                                                          */
/*   This program is free software: you can redistribute it and/or modify   */
/*   it under the terms of the GNU General Public License as published by   */
/*   the Free Software Foundation, either version 3 of the License, or      */
/*   (at your option) any later version.                                    */
/*                                                                          */
/*   This program is distributed in the hope that it will be useful,        */
/*   but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          */
/*   GNU General Public License for more details.                           */
/*                                                                          */
/*   You should have received a copy of the GNU General Public License      */
/*   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */
/*                                                                          */
/****************************************************************************/

#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0400

#ifdef BUILD_METAPAD_UNICODE
#define _UNICODE
#include <wchar.h>
#else
#undef _UNICODE
#undef UNICODE
#endif

/* Experimental/not working yet */
//#define STREAMING
//#define USE_BOOKMARKS
//#define BUILD_METAPAD_UNICODE

#include <windows.h>
#include <shellapi.h>
#include <commdlg.h>
#include <commctrl.h> 
#include <winuser.h>
#include <tchar.h> 

#ifndef TBSTYLE_FLAT
#define TBSTYLE_FLAT 0x0800
#endif

#ifdef USE_RICH_EDIT
#include <richedit.h>
#endif

#include "resource.h"

#if defined(__MINGW32__)
 #define PROPSHEETHEADER_V1_SIZE 40
#else
 #include "w32crt.h"
#endif

#include "cencode.h"
#include "cdecode.h"

extern atol(const char*);
extern atoi(const char*);

#pragma intrinsic(memset)

//#pragma comment(linker, "/OPT:NOWIN98" )

///// Consts /////

#define MAXFN 300
#define MAXFONT 100
#define MAXARGS 100
#define MAXQUOTE 100
#define MAXFIND 100
#define NUMPANES 5
#define MAXFAVESIZE 2000
#define MAXSTRING 500
//#define MAXFAVES 16
#define MAXMACRO 1001

#define NUMCUSTOMBITMAPS 6
#ifdef USE_RICH_EDIT
#define NUMBUTTONS 26
#else
#define NUMBUTTONS 25
#endif

#define CUSTOMBMPBASE 15
#define NUMFINDS 10
#define EGGNUM 15
#define SBPANE_TYPE 0
#define SBPANE_INS 1
#define SBPANE_LINE 2
#define SBPANE_COL 3
#define SBPANE_MESSAGE 4
#define STATUS_FONT_CONST 1.4

#ifdef USE_RICH_EDIT
#define RECENTPOS (options.bReadOnlyMenu ? 15 : 14)
#define CONVERTPOS 8
#else
#define RECENTPOS (options.bReadOnlyMenu ? 14 : 13)
#define CONVERTPOS 7
#endif

#define FILEFORMATPOS (options.bReadOnlyMenu ? 8 : 7)
#define EDITPOS (options.bRecentOnOwn ? 2 : 1)
#define READONLYPOS 4
#define FAVEPOS (options.bRecentOnOwn ? 3 : 2)

#define ID_CLIENT 100
#define ID_STATUSBAR 101
#define ID_TOOLBAR 102

#define FILE_FORMAT_DOS 0
#define FILE_FORMAT_UNIX 1
#define FILE_FORMAT_UNICODE 2
#define FILE_FORMAT_UNICODE_BE 3
#define FILE_FORMAT_UTF_8 4

#define SIZEOFBOM_UTF_8 3
#define SIZEOFBOM_UTF_16 2

#define TYPE_UNKNOWN 0
#define TYPE_UTF_8 1
#define TYPE_UTF_16 2
#define TYPE_UTF_16_BE 3

#define WS_EX_LAYERED 0x00080000
#define LWA_COLORKEY 0x00000001
#define LWA_ALPHA 0x00000002

///// Strings /////

#ifdef BUILD_METAPAD_UNICODE
#ifdef USE_RICH_EDIT
#define STR_ABOUT_NORMAL _T("metapad 3.xU ALPHA 0")
#else
#define STR_ABOUT_NORMAL _T("metapad LE 3.xU ALPHA 0")
#endif
#else
#ifdef USE_RICH_EDIT
#define STR_ABOUT_NORMAL _T("metapad 3.6")
#else
#define STR_ABOUT_NORMAL _T("metapad LE 3.6")
#endif
#endif

#define STR_ABOUT_HACKER _T("m374p4d i5 d4 5hi7!")

#ifdef USE_RICH_EDIT
#define STR_RICHDLL _T("RICHED20.DLL")
#endif

#define STR_METAPAD _T("metapad")
#define STR_FAV_FILE _T("metafav.ini")
#define STR_CAPTION_FILE _T("%s - metapad")
#define STR_URL _T("http://liquidninja.com/metapad")
#define STR_REGKEY _T("SOFTWARE\\metapad")
#define STR_FAV_APPNAME _T("Favourites")
#define STR_COPYRIGHT _T("© 1999-2011 Alexander Davidson")

///// Macros /////

#define ERROROUT(_x) MessageBox(hwnd, _x, STR_METAPAD, MB_OK | MB_ICONEXCLAMATION)
#define MSGOUT(_x) MessageBox(hwnd, _x, STR_METAPAD, MB_OK | MB_ICONINFORMATION)
#define DBGOUT(_x, _y) MessageBox(hwnd, _x, _y, MB_OK | MB_ICONEXCLAMATION)

///// Typedefs /////
#include <pshpack1.h>
typedef struct DLGTEMPLATEEX
{
	WORD dlgVer;
	WORD signature;
	DWORD helpID;
	DWORD exStyle;
	DWORD style;
	WORD cDlgItems;
	short x;
	short y;
	short cx;
	short cy;
} DLGTEMPLATEEX, *LPDLGTEMPLATEEX;
#include <poppack.h>

#ifndef USE_RICH_EDIT
typedef struct _charrange
{
	LONG	cpMin;
	LONG	cpMax;
} CHARRANGE;
#endif

typedef BOOL (WINAPI *SLWA)(HWND hwnd, COLORREF crKey, BYTE bAlpha, DWORD dwFlags);

///// Globals /////

SLWA SetLWA = NULL;
HINSTANCE hinstThis = NULL;
HINSTANCE hinstLang = NULL;
HWND hwnd = NULL;
HWND client = NULL;
HWND status = NULL;
HWND toolbar = NULL;
HWND hdlgCancel = NULL;
HWND hdlgFind = NULL;
HANDLE hthread;
HMENU hrecentmenu = NULL;
HFONT hfontmain = NULL;
HFONT hfontfind = NULL;
WNDPROC wpOrigEditProc;
WNDPROC wpOrigFindProc;
TCHAR szCaptionFile[MAXFN], szFile[MAXFN];
TCHAR szFav[MAXFN];
TCHAR szMetapadIni[MAXFN];
TCHAR szDir[MAXFN];
TCHAR szFindText[MAXFIND];
TCHAR szReplaceText[MAXFIND];
TCHAR szStatusMessage[MAXSTRING];
TCHAR _szString[MAXSTRING];
LPTSTR lpszShadow;
BOOL bDirtyFile, bLoading, bMatchCase, bDown, bWholeWord, bUnix, bReadOnly, bBinaryFile;
BOOL bWordWrap, bPrimaryFont, bPrint, bSmartSelect, bShowStatus /*, bWin2k*/;
BOOL bReplacingAll, bShowToolbar, bAlwaysOnTop, bCloseAfterFind, bHasFaves, bNoFindHidden;
BOOL bTransparent;
//BOOL bLinkMenu;
UINT nMRUTop;
UINT nShadowSize;
UINT uFindReplaceMsg;
int nStatusHeight, nToolbarHeight;
TCHAR szCustomFilter[2*MAXSTRING];
BOOL bInsertMode, bHideMessage;
int nReplaceMax;
HWND hwndSheet;
TCHAR FindArray[NUMFINDS][MAXFIND];
TCHAR ReplaceArray[NUMFINDS][MAXFIND];
int nEncodingType;
BOOL g_bDisablePluginVersionChecking;
BOOL g_bIniMode = FALSE;

#ifdef USE_RICH_EDIT
BOOL bUpdated, bHyperlinks;
#endif

#ifndef USE_RICH_EDIT
HBRUSH BackBrush;
BOOL bQuitApp;
#endif

int _fltused = 0x9875; // see CMISCDAT.C for more info on this

CHAR szBOM_UTF_8[] = "\xEF\xBB\xBF";  // 0xEF, 0xBB, 0xBF / "\357\273\277" - leave off _T() macro.
CHAR szBOM_UTF_16[] = "\377\376";  // 0xFF, 0xFE - leave off _T() macro.
CHAR szBOM_UTF_16_BE[] = "\376\377";  // 0xFE, 0xFF - leave off _T() macro.

typedef struct tag_options {
	BOOL bQuickExit;
	BOOL bSaveWindowPlacement;
	BOOL bSaveMenuSettings;
	BOOL bSaveDirectory;
	BOOL bLaunchClose;
	int nTabStops;
	int nPrimaryFont;
	int nSecondaryFont;
	int nLaunchSave;
	RECT rMargins;
	LOGFONT PrimaryFont, SecondaryFont;
	TCHAR szBrowser[MAXFN];
	TCHAR szArgs[MAXARGS];
	TCHAR szBrowser2[MAXFN];
	TCHAR szArgs2[MAXARGS];
	TCHAR szQuote[MAXQUOTE];
	BOOL bFindAutoWrap;
	BOOL bAutoIndent;
	BOOL bInsertSpaces;
	BOOL bNoCaptionDir;
	BOOL bHideGotoOffset;
	BOOL bRecentOnOwn;
	BOOL bDontInsertTime;
	BOOL bNoWarningPrompt;
	BOOL bUnFlatToolbar;
	BOOL bStickyWindow;
	BOOL bReadOnlyMenu;
	UINT nStatusFontWidth;
	UINT nSelectionMarginWidth;
	UINT nMaxMRU;
	UINT nFormatIndex;
	UINT nTransparentPct;
	BOOL bSystemColours;
	BOOL bSystemColours2;
	COLORREF BackColour, FontColour;
	COLORREF BackColour2, FontColour2;
	BOOL bNoSmartHome;
	BOOL bNoAutoSaveExt;
	BOOL bContextCursor;
	BOOL bCurrentFindFont;
#ifndef USE_RICH_EDIT
	BOOL bDefaultPrintFont;
	BOOL bAlwaysLaunch;
#else
	BOOL bSuppressUndoBufferPrompt;
	BOOL bLinkDoubleClick;
	BOOL bHideScrollbars;
#endif
	BOOL bNoFaves;
	BOOL bPrintWithSecondaryFont;
	BOOL bNoSaveHistory;
	BOOL bNoFindAutoSelect;
	TCHAR szLangPlugin[MAXFN];
	TCHAR MacroArray[10][MAXMACRO];
	TCHAR szFavDir[MAXFN];
} option_struct;

option_struct options;

///// Prototypes /////

void MakeNewFile(void);
BOOL SaveIfDirty(void);
BOOL GetCheckedState(HMENU hmenu, UINT nID, BOOL bToggle);
void CreateClient(HWND hParent, LPCTSTR szText, BOOL bWrap);
LPCTSTR GetShadowBuffer(void);
BOOL CALLBACK AbortDlgProc(HDC hdc, int nCode);
LRESULT CALLBACK AbortPrintJob(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam);
BOOL DoSearch(LPCTSTR szText, LONG lStart, LONG lEnd, BOOL bDown, BOOL bWholeWord, BOOL bCase, BOOL bFromTop);
void ExpandFilename(LPTSTR szBuffer);
void PrintContents(void);
void ReportLastError(void);
void LaunchPrimaryExternalViewer(void);
void LaunchSecondaryExternalViewer(void);
void LoadOptionString(HKEY hKey, LPCSTR name, BYTE* lpData, DWORD cbData);
void LoadBoundedOptionString(HKEY hKey, LPCSTR name, BYTE* lpData, DWORD cbData);
BOOL LoadOptionNumeric(HKEY hKey, LPCSTR name, BYTE* lpData, DWORD cbData);
void LoadOptionBinary(HKEY hKey, LPCSTR name, BYTE* lpData, DWORD cbData);
void LoadOptions(void);
void SaveOptions(void);
void LoadWindowPlacement(int* left, int* top, int* width, int* height, int* nShow);
void SaveWindowPlacement(HWND hWndSave);
void CenterWindow(HWND hwndCenter);
void SelectWord(BOOL bFinding, BOOL bSmart, BOOL bAutoSelect);
void LoadFile(LPTSTR szFilename, BOOL bCreate, BOOL bMRU);
BOOL SaveFile(LPCTSTR szFilename);
void SetFont(HFONT* phfnt, BOOL bPrimary);
void SetTabStops(void);
void NextWord(BOOL bRight, BOOL bSelect);
void UpdateStatus(void);
BOOL SetClientFont(BOOL bPrimary);
BOOL SearchFile(LPCTSTR szText, BOOL bMatchCase, BOOL bReplaceAll, BOOL bDown, BOOL bWholeWord);
BOOL CALLBACK AboutDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK AdvancedPageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK Advanced2PageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK GeneralPageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK ViewPageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
LONG WINAPI MainWndProc(HWND hwndMain, UINT Msg, WPARAM wParam, LPARAM lParam);
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow);
LRESULT APIENTRY EditProc(HWND hwndEdit, UINT uMsg, WPARAM wParam, LPARAM lParam);
void PopulateMRUList(void);
void SaveMRUInfo(LPCTSTR szFullPath);
void SwitchReadOnly(BOOL bNewVal);
BOOL EncodeWithEscapeSeqs(TCHAR* szText);

///// Implementation /////

LPTSTR GetString(UINT uID)
{
	LoadString(hinstLang, uID, _szString, MAXSTRING);
	return _szString;
}

HINSTANCE LoadAndVerifyLanguagePlugin(LPCTSTR szPlugin)
{
	HINSTANCE hinstTemp;
	
	hinstTemp = LoadLibrary(szPlugin);
	if (hinstTemp == NULL) {
		ERROROUT(GetString(IDS_INVALID_PLUGIN_ERROR));
		return NULL;
	}

	{
		TCHAR szVersionThis[25];
		TCHAR szVersionPlug[25];

		if (LoadString(hinstTemp, IDS_VERSION_SYNCH, szVersionPlug, 25) == 0) {
			ERROROUT(GetString(IDS_BAD_STRING_PLUGIN_ERROR));
			FreeLibrary(hinstTemp);
			return NULL;
		}
		LoadString(hinstThis, IDS_VERSION_SYNCH, szVersionThis, 25);
		if (!g_bDisablePluginVersionChecking && lstrcmpi(szVersionThis, szVersionPlug) != 0) {
			TCHAR szVersionError[550];
			wsprintf(szVersionError, GetString(IDS_PLUGIN_MISMATCH_ERROR), szVersionPlug, szVersionThis);
			ERROROUT(szVersionError);
		}
	}

	return hinstTemp;
}


void FindAndLoadLanguagePlugin(void)
{
	HINSTANCE hinstTemp;

	hinstLang = hinstThis;

	if (options.szLangPlugin[0] == '\0')
		return;

	{
		WIN32_FIND_DATA FileData;
		HANDLE hSearch;

		hSearch = FindFirstFile(options.szLangPlugin, &FileData);
		if (hSearch == INVALID_HANDLE_VALUE) {
			ERROROUT(_T("Could not find the language plugin DLL."));
			goto badplugin;
		}
		else {
			FindClose(hSearch);
		}
	}

	hinstTemp = LoadAndVerifyLanguagePlugin(options.szLangPlugin);
	if (hinstTemp) {
		hinstLang = hinstTemp;
		return;
	}

badplugin:
	ERROROUT(_T("Temporarily reverting language to Default (English)\n\nCheck the language plugin setting."));
}

BOOL EncodeWithEscapeSeqs(TCHAR* szText)
{
	TCHAR szStore[MAXMACRO];
	INT i,j;
	BOOL bSlashFound = FALSE;

	for (i = 0, j = 0; szText[i] && i < MAXMACRO; ++i) {
		switch (szText[i]) {
		case '\n':
			break;
		case '\r':
			szStore[j++] = '\\';
			szStore[j++] = 'n';
			break;
		case '\t':
			szStore[j++] = '\\';
			szStore[j++] = 't';
			break;
		case '\\':
			szStore[j++] = '\\';
			szStore[j++] = '\\';
			break;
		default:
			szStore[j++] = szText[i];
		}
		if (j >= MAXMACRO - 1) { 
			return FALSE; 
		}
	}
	szStore[j] = '\0';
	lstrcpy(szText, szStore);
	return TRUE;
}

void ParseForEscapeSeqs(TCHAR* szText)
{
	TCHAR szStore[MAXMACRO];
	INT i,j;
	BOOL bSlashFound = FALSE;

	for (i = 0, j = 0; szText[i] && i < MAXMACRO; ++i) {
		if (bSlashFound) {
			switch (szText[i]) {
			case 'n':
#ifdef USE_RICH_EDIT
				szStore[j] = '\r';
#else
				szStore[j++] = '\r';
				szStore[j] = '\n';
#endif
				break;
			case 't':
				szStore[j] = '\t';
				break;
			default:
				szStore[j] = szText[i];
			}
			bSlashFound = FALSE;
		}
		else {
			if (szText[i] == '\\') {
				bSlashFound = TRUE;
				continue;
			}
			szStore[j] = szText[i];
		}
		++j;
	}
	szStore[j] = '\0';
	lstrcpy(szText, szStore);
}

BOOL ExecuteProgram(LPCTSTR lpExecutable, LPCTSTR lpCommandLine)
{
	TCHAR szCmdLine[1024];
	LPTSTR lpFormat;

	if (lpExecutable[0] == _T('"') && lpExecutable[lstrlen(lpExecutable) - 1] == _T('"')) {
		// quotes already present
		lpFormat = _T("%s %s");
	}
	else {
		// executable file must be quoted to conform to Win32 file name
		// specs.
		lpFormat = _T("\"%s\" %s");
	}

	wsprintf(szCmdLine, lpFormat, lpExecutable, lpCommandLine);

	if (lstrcmpi(lpExecutable + (lstrlen(lpExecutable) - 4), ".exe") != 0) {
		if ((int)ShellExecute(NULL, NULL, lpExecutable, szCmdLine, szDir, SW_SHOWNORMAL) <= 32) {
			return FALSE;
		}
	}
	else {
		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(STARTUPINFO));

		si.cb = sizeof(STARTUPINFO);
		si.wShowWindow = SW_SHOWNORMAL;
		si.dwFlags = STARTF_USESHOWWINDOW;

		if (!CreateProcess(lpExecutable, szCmdLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
			return FALSE;
		}
		else {
			// We don't use the handles so close them now
			CloseHandle(pi.hProcess);
			CloseHandle(pi.hThread);
		}
	}
	return TRUE;
}


BOOL IsBOM(PBYTE pb, int bomType)
{
	if (bomType == TYPE_UTF_8) {
		if ((*pb == 0xEF) & (*(pb+1) == 0xBB) & (*(pb+2) == 0xBF))
			return TRUE;
		else
			return FALSE;
	}
	else if (bomType == TYPE_UTF_16) {
		if ((*pb == 0xFF) & (*(pb+1) == 0xFE))
			return TRUE;
		else
			return FALSE;
	}
	else if (bomType == TYPE_UTF_16_BE) {
		if ((*pb == 0xFE) & (*(pb+1) == 0xFF))
			return TRUE;
		else
			return FALSE;
	}
	return FALSE;
}

void ReverseBytes(PBYTE buffer, LONG size)
{
	BYTE temp;
	long i, end;

	end = size - 2;
	for (i = 0; i <= end; i+=2) {
		temp = buffer[i];
		buffer[i] = buffer[i+1];
		buffer[i+1] = temp;
	}
}

long CalculateFileSize(void)
{
	long nBytes;
	if (nEncodingType == TYPE_UTF_16 || nEncodingType == TYPE_UTF_16_BE) {
		nBytes = GetWindowTextLength(client) * 2 + SIZEOFBOM_UTF_16;
	}
	else if (nEncodingType == TYPE_UTF_8) {
		nBytes = GetWindowTextLength(client) + SIZEOFBOM_UTF_8;
	}
	else {
		nBytes = GetWindowTextLength(client) - (bUnix ? (SendMessage(client, EM_GETLINECOUNT, 0, 0)) - 1 : 0);
	}
	return nBytes;
}

#ifndef USE_RICH_EDIT
void GetClientRange(int min, int max, LPTSTR szDest)
{
	long lFileSize;
	LPTSTR szBuffer;
	TCHAR ch;

	lFileSize = GetWindowTextLength(client);
	szBuffer = (LPTSTR)GetShadowBuffer();
	ch = szBuffer[max];
	szBuffer[max] = '\0';
	lstrcpy(szDest, szBuffer + min);
	szBuffer[max] = ch;
}
#endif

void UpdateCaption(void)
{
	TCHAR szBuffer[MAXFN];

	ExpandFilename(szFile);

	if (bDirtyFile) {
		szBuffer[0] = _T(' ');
		szBuffer[1] = _T('*');
		szBuffer[2] = _T(' ');
		wsprintf(szBuffer+3, STR_CAPTION_FILE, szCaptionFile);
	}
	else
		wsprintf(szBuffer, STR_CAPTION_FILE, szCaptionFile);

	if (bReadOnly) {
		lstrcat(szBuffer, _T(" "));
		lstrcat(szBuffer, GetString(IDS_READONLY_INDICATOR));
	}

	SetWindowText(hwnd, szBuffer);
}

void LoadFileFromMenu(WORD wMenu, BOOL bMRU)
{
	HMENU hmenu = GetMenu(hwnd);
	HMENU hsub = NULL;
	MENUITEMINFO mio;
	TCHAR szBuffer[MAXFN] = _T("\n");

	if (!SaveIfDirty())
		return;

	if (bMRU) {
		if (options.bRecentOnOwn)
			hsub = GetSubMenu(hmenu, 1);
		else
			hsub = GetSubMenu(GetSubMenu(hmenu, 0), RECENTPOS);
	}
	else if (!options.bNoFaves) {
		hsub = GetSubMenu(hmenu, FAVEPOS);
	}
	else {
		return;
	}

	mio.cbSize = sizeof(MENUITEMINFO);
	mio.fMask = MIIM_TYPE;
	mio.fType = MFT_STRING;
	mio.cch = MAXFN;
	mio.dwTypeData = szBuffer;
	GetMenuItemInfo(hsub, wMenu, FALSE, &mio);

	lstrcpy(szFile, szBuffer + 3);

	if (lstrlen(szFile) > 0) {

		if (!bMRU) {
			GetPrivateProfileString(STR_FAV_APPNAME, szFile, _T("error"), szFile, MAXFN, szFav);
			if (lstrcmp(szFile, _T("error")) == 0) {
				ERROROUT(GetString(IDS_ERROR_FAVOURITES));
				MakeNewFile();
				return;
			}
		}

		bLoading = TRUE;
		bHideMessage = FALSE;
		ExpandFilename(szFile);
		lstrcpy(szStatusMessage, GetString(IDS_FILE_LOADING));
		UpdateStatus();
		LoadFile(szFile, FALSE, TRUE);
		if (bLoading) {
			bLoading = FALSE;
			bDirtyFile = FALSE;
			UpdateCaption();
		}
		else {
			MakeNewFile();
		}
	}
}

void CleanUp(void)
{
	if (hrecentmenu)
		DestroyMenu(hrecentmenu);
	if (lpszShadow)
		GlobalFree((HGLOBAL) lpszShadow);
	if (hfontmain)
		DeleteObject(hfontmain);
	if (hfontfind)
		DeleteObject(hfontfind);
	if (hthread)
		CloseHandle(hthread);

	if (hinstLang != hinstThis)
		FreeLibrary(hinstLang);

#ifdef USE_RICH_EDIT
	DestroyWindow(client);
	FreeLibrary(GetModuleHandle(STR_RICHDLL));
#else
	if (BackBrush)
		DeleteObject(BackBrush);
#endif
}

void UpdateWindowText(void)
{
	LPCTSTR szBuffer = GetShadowBuffer();
	bLoading = TRUE;
	SetWindowText(client, szBuffer);
	bLoading = FALSE;
	SetTabStops();
	SendMessage(client, EM_EMPTYUNDOBUFFER, 0, 0);
	UpdateStatus();
	InvalidateRect(client, NULL, TRUE);
}

void SetFileFormat(int nFormat)
{
	switch (nFormat) {
	case FILE_FORMAT_DOS:
		SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_DOS_FILE, 0), 0);
		break;
	case FILE_FORMAT_UNIX:
		SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_UNIX_FILE, 0), 0);
		break;
	case FILE_FORMAT_UTF_8:
		SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_UTF_8_FILE, 0), 0);
		break;
	case FILE_FORMAT_UNICODE:
		SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_UNICODE_FILE, 0), 0);
		break;
	case FILE_FORMAT_UNICODE_BE:
		SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_UNICODE_BE_FILE, 0), 0);
		break;
	}
}

void MakeNewFile(void)
{
	bLoading = TRUE;
	SetFileFormat(options.nFormatIndex);
	SetWindowText(client, _T(""));
	bDirtyFile = FALSE;
	bBinaryFile = FALSE;
	bLoading = FALSE;

	{
		TCHAR szBuffer[100];
		wsprintf(szBuffer, STR_CAPTION_FILE, GetString(IDS_NEW_FILE));
		SetWindowText(hwnd, szBuffer);
	}

	SwitchReadOnly(FALSE);
	szFile[0] = '\0';
	lstrcpy(szCaptionFile, GetString(IDS_NEW_FILE));
	UpdateStatus();
	if (lpszShadow)
		lpszShadow[0] = '\0';
	bLoading = FALSE;
}

int GetStatusHeight(void)
{
	return (bShowStatus ? nStatusHeight : 0);
}

int GetToolbarHeight(void)
{
	return (bShowToolbar ? nToolbarHeight : 0);
}

void SwitchReadOnly(BOOL bNewVal)
{
	bReadOnly = bNewVal;
	if (GetCheckedState(GetMenu(hwnd), ID_READONLY, FALSE) != bNewVal) {
		GetCheckedState(GetMenu(hwnd), ID_READONLY, TRUE);
	}
}

void FixFilterString(LPTSTR szIn)
{
	int i;

	for (i = 0; szIn[i]; ++i) {
		if (szIn[i] == '|') {
			szIn[i] = '\0';
		}
	}
	szIn[i+1] = '\0';
}

BOOL SaveCurrentFileAs(void)
{
	OPENFILENAME ofn;
	TCHAR szTmp[MAXFN];
	TCHAR* pch;

	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = client;

	if (options.bNoAutoSaveExt) {
		ofn.lpstrFilter = GetString(IDS_DEFAULT_FILTER_TEXT);
		FixFilterString((LPTSTR)ofn.lpstrFilter);
		ofn.lpstrDefExt = NULL;
	}
	else {
		ofn.lpstrFilter = szCustomFilter;
		ofn.lpstrDefExt = _T("txt");
	}
	
	ofn.lpstrCustomFilter = (LPTSTR)NULL;
	ofn.nMaxCustFilter = 0L;
	ofn.nFilterIndex = 1L;

	pch = _tcsrchr(szFile, _T('\\'));
	if (pch == NULL)
		lstrcpy(szTmp, szFile);
	else
		lstrcpy(szTmp, pch+1);

	ofn.lpstrFile = szTmp;
	ofn.nMaxFile = sizeof(szTmp);
	
	ofn.lpstrFileTitle = (LPTSTR)NULL;
	ofn.nMaxFileTitle = 0L;
	ofn.lpstrInitialDir = szDir;
	ofn.lpstrTitle = (LPTSTR)NULL;
	ofn.Flags = OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT | OFN_PATHMUSTEXIST;
	ofn.nFileOffset = 0;
	ofn.nFileExtension = 0;

	if (GetSaveFileName(&ofn)) {
		lstrcpy(szFile, szTmp);
		if (!SaveFile(szFile))
			return FALSE;
		SaveMRUInfo(szFile);

		SwitchReadOnly(FALSE);
		bLoading = FALSE;
		bDirtyFile = FALSE;
		UpdateStatus();
		UpdateCaption();
		return TRUE;
	}
	else
		return FALSE;
}

BOOL SaveCurrentFile(void)
{
	SetCurrentDirectory(szDir);
	
	if (lstrlen(szFile) > 0) {
		TCHAR szTmp[MAXFN];
		DWORD dwResult = GetFileAttributes(szFile);

		if (dwResult != 0xffffffff && bReadOnly != (BOOL)(dwResult & FILE_ATTRIBUTE_READONLY)) {
			bReadOnly = dwResult & FILE_ATTRIBUTE_READONLY;
			UpdateCaption();
		}
		if (bReadOnly) {
			if (MessageBox(hwnd, GetString(IDS_READONLY_WARNING), STR_METAPAD, MB_ICONEXCLAMATION | MB_OKCANCEL) == IDOK) {
				return SaveCurrentFileAs();			
			}
			return FALSE;
		}

		if (!SaveFile(szFile))
			return FALSE;
		ExpandFilename(szFile);
		wsprintf(szTmp, STR_CAPTION_FILE, szCaptionFile);
		SetWindowText(hwnd, szTmp);
		bDirtyFile = FALSE;
		return TRUE;
	}
	else
		return SaveCurrentFileAs();
}

BOOL SaveIfDirty(void)
{
	if (bDirtyFile) {
		TCHAR szBuffer[MAXFN];
		if (lstrlen(szFile) == 0) {
			if (GetWindowTextLength(client) == 0) {
				return TRUE;
			}
		}
		wsprintf(szBuffer, GetString(IDS_DIRTYFILE), szCaptionFile);
		switch (MessageBox(hwnd, szBuffer, STR_METAPAD, MB_ICONEXCLAMATION | MB_YESNOCANCEL)) {
			case IDYES:
				if (!SaveCurrentFile())
					return FALSE;
			case IDNO:
				return TRUE;
			case IDCANCEL:
				return FALSE;
		}
	}
	return TRUE;
}

BOOL GetCheckedState(HMENU hmenu, UINT nID, BOOL bToggle)
{
	MENUITEMINFO mio;

	mio.cbSize = sizeof(MENUITEMINFO);
	mio.fMask = MIIM_STATE;
	GetMenuItemInfo(hmenu, nID, FALSE, &mio);
	if (mio.fState == MFS_CHECKED) {
		if (bToggle) {
			mio.fState = MFS_UNCHECKED;
			SetMenuItemInfo(hmenu, nID, FALSE, &mio);
		}
		return TRUE;
	}
	else {
		if (bToggle) {
			mio.fState = MFS_CHECKED;
			SetMenuItemInfo(hmenu, nID, FALSE, &mio);
		}
		return FALSE;
	}
}

void CreateToolbar(void)
{
	DWORD dwStyle = WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | TBSTYLE_TOOLTIPS;
	TBADDBITMAP tbab = {hinstThis, IDB_TOOLBAR};

	TBBUTTON tbButtons [] = {
		{0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0},
		//{CUSTOMBMPBASE+6, ID_NEW_INSTANCE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
		{STD_FILENEW, ID_MYFILE_NEW, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
		{STD_FILEOPEN, ID_MYFILE_OPEN, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
		{STD_FILESAVE, ID_MYFILE_SAVE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
		{CUSTOMBMPBASE+1, ID_RELOAD_CURRENT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
		{0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0},
		{CUSTOMBMPBASE+2, ID_FILE_LAUNCHVIEWER, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
		{CUSTOMBMPBASE+3, ID_LAUNCH_SECONDARY_VIEWER, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
		{0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0},
		{STD_PRINT, ID_PRINT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
		{0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0},
		{STD_FIND, ID_FIND, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
		{STD_REPLACE, ID_REPLACE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
		{0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0},
		{STD_CUT, ID_MYEDIT_CUT, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
		{STD_COPY, ID_MYEDIT_COPY, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
		{STD_PASTE, ID_MYEDIT_PASTE, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
		{0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0},
		{STD_UNDO, ID_MYEDIT_UNDO, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
#ifdef USE_RICH_EDIT
		{STD_REDOW, ID_MYEDIT_REDO, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
#endif
		{0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0},
		{CUSTOMBMPBASE, ID_EDIT_WORDWRAP, TBSTATE_ENABLED, TBSTYLE_CHECK, 0L, 0},
		{CUSTOMBMPBASE+4, ID_FONT_PRIMARY, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
		{CUSTOMBMPBASE+5, ID_ALWAYSONTOP, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
		{0, 0, TBSTATE_ENABLED, TBSTYLE_SEP, 0L, 0},
		{STD_PROPERTIES, ID_VIEW_OPTIONS, TBSTATE_ENABLED, TBSTYLE_BUTTON, 0L, 0},
	};

	if (!options.bUnFlatToolbar)
		dwStyle |= TBSTYLE_FLAT;

	toolbar = CreateToolbarEx(hwnd, dwStyle,
		ID_TOOLBAR, CUSTOMBMPBASE, HINST_COMMCTRL, IDB_STD_SMALL_COLOR, 
		(LPCTBBUTTON)&tbButtons, NUMBUTTONS, 0, 0, 16, 16, sizeof(TBBUTTON));

	if (SendMessage(toolbar, TB_ADDBITMAP, (WPARAM)NUMCUSTOMBITMAPS, (LPARAM)&tbab) < 0)
		ReportLastError();

	{
		RECT rect;
		GetWindowRect(toolbar, &rect);
		nToolbarHeight = rect.bottom - rect.top;
	}
}


void CreateStatusbar(void)
{
	int nPaneSizes[NUMPANES] = {0,0,0};

	status = CreateWindowEx(
		WS_EX_DLGMODALFRAME,
		STATUSCLASSNAME,
		_T(""),
		WS_VISIBLE | WS_CHILD | WS_CLIPSIBLINGS | SBT_NOBORDERS | SBARS_SIZEGRIP,
		0, 0, 0, 0,
		hwnd,
		(HMENU) ID_STATUSBAR,
		hinstThis,
		NULL);

	/*
	if (!hstatusfont) {
		hstatusfont = CreateFont(8, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, VARIABLE_PITCH | FF_SWISS, "MS Sans Serif");
	}
	SendMessage(status, WM_SETFONT, (WPARAM)hstatusfont, 0);
	*/

	SendMessage(status, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), 0);

	{
		RECT rect;
		GetWindowRect(status, &rect);
		nStatusHeight = rect.bottom - rect.top - 5;
	}

	SendMessage(status, SB_SETPARTS, NUMPANES, (DWORD)(LPINT)nPaneSizes);
}

void CreateClient(HWND hParent, LPCTSTR szText, BOOL bWrap)
{
#ifdef USE_RICH_EDIT
	DWORD dwStyle = 
					ES_AUTOHSCROLL |
					ES_AUTOVSCROLL |
					ES_NOHIDESEL |
					ES_MULTILINE |
					WS_HSCROLL |
					WS_VSCROLL |
					WS_CHILD |
					0;

	if (!options.bHideScrollbars) {
		dwStyle |= ES_DISABLENOSCROLL;
	}

	client = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		RICHEDIT_CLASS,
		szText,
		dwStyle,
		0, 0, 0, 0,
		hParent,
		(HMENU)ID_CLIENT,
		hinstThis,
		NULL);

	if (!client) ReportLastError();

	SendMessage(client, EM_SETTARGETDEVICE, (WPARAM)0, (LPARAM)(LONG) !bWrap);
	SendMessage(client, EM_AUTOURLDETECT, (WPARAM)bHyperlinks, 0);
	SendMessage(client, EM_SETEVENTMASK, 0, (LPARAM)(ENM_LINK | ENM_CHANGE));
	SendMessage(client, EM_EXLIMITTEXT, 0, (LPARAM)(DWORD)0x7fffffff);

	// sort of fixes font problems but cannot set tab size
	//SendMessage(client, EM_SETTEXTMODE, (WPARAM)TM_PLAINTEXT, 0);

	wpOrigEditProc = (WNDPROC) SetWindowLong(client, GWL_WNDPROC, (LONG) EditProc);
#else
	DWORD dwStyle = ES_NOHIDESEL | WS_VSCROLL | ES_MULTILINE | WS_CHILD;
	
	if (!bWrap)
		dwStyle |= WS_HSCROLL;

	/*
	if (bReadOnly)
		dwStyle |= ES_READONLY;
	*/

	client = CreateWindowEx(
		WS_EX_CLIENTEDGE,
		_T("EDIT"),
		szText,
		dwStyle,
		0, 0, 0, 0,
		hParent,
		(HMENU)ID_CLIENT,
		hinstThis,
		NULL);

	SendMessage(client, EM_LIMITTEXT, 0, 0);
	wpOrigEditProc = (WNDPROC) SetWindowLong(client, GWL_WNDPROC, (LONG) EditProc);
#endif
}

void UpdateStatus(void)
{
	LONG lLine, lLineIndex, lLines;
	TCHAR szPane[50];
	int nPaneSizes[NUMPANES];
	LPTSTR szBuffer;
	long lLineLen, lCol = 1;
	int i = 0;
	CHARRANGE cr;

	if (toolbar && bShowToolbar) {

#ifdef USE_RICH_EDIT
		SendMessage(client, EM_EXGETSEL, 0, (LPARAM)&cr);
#else
		SendMessage(client, EM_GETSEL, (WPARAM)&cr.cpMin, (LPARAM)&cr.cpMax);
#endif
		SendMessage(toolbar, TB_CHECKBUTTON, (WPARAM)ID_EDIT_WORDWRAP, MAKELONG(bWordWrap, 0));
		SendMessage(toolbar, TB_CHECKBUTTON, (WPARAM)ID_FONT_PRIMARY, MAKELONG(bPrimaryFont, 0));
		SendMessage(toolbar, TB_CHECKBUTTON, (WPARAM)ID_ALWAYSONTOP, MAKELONG(bAlwaysOnTop, 0));
		SendMessage(toolbar, TB_ENABLEBUTTON, (WPARAM)ID_RELOAD_CURRENT, MAKELONG(szFile[0] != '\0', 0));
		SendMessage(toolbar, TB_ENABLEBUTTON, (WPARAM)ID_MYEDIT_CUT, MAKELONG((cr.cpMin != cr.cpMax), 0));
		SendMessage(toolbar, TB_ENABLEBUTTON, (WPARAM)ID_MYEDIT_COPY, MAKELONG((cr.cpMin != cr.cpMax), 0));
		SendMessage(toolbar, TB_ENABLEBUTTON, (WPARAM)ID_MYEDIT_UNDO, MAKELONG(SendMessage(client, EM_CANUNDO, 0, 0), 0));
		SendMessage(toolbar, TB_ENABLEBUTTON, (WPARAM)ID_MYEDIT_PASTE, MAKELONG(IsClipboardFormatAvailable(CF_TEXT), 0));
#ifdef USE_RICH_EDIT
		SendMessage(toolbar, TB_ENABLEBUTTON, (WPARAM)ID_MYEDIT_REDO, MAKELONG(SendMessage(client, EM_CANREDO, 0, 0), 0));
#endif
	}

	if (status == NULL || !bShowStatus)
		return;

#ifdef USE_RICH_EDIT
	if (!bUpdated)
		return;
#endif

	nPaneSizes[SBPANE_TYPE] = 4 * options.nStatusFontWidth;

	if (bBinaryFile) {
		wsprintf(szPane, _T("  BIN"));
	}
	else if (nEncodingType == TYPE_UTF_8) {
		wsprintf(szPane, _T(" UTF-8"));
		nPaneSizes[SBPANE_TYPE] = 5 * options.nStatusFontWidth + 4;
	}
	else if (nEncodingType == TYPE_UTF_16) {
		wsprintf(szPane, _T(" Unicode"));
		nPaneSizes[SBPANE_TYPE] = 6 * options.nStatusFontWidth + 4;
	}
	else if (nEncodingType == TYPE_UTF_16_BE) {
		wsprintf(szPane, _T(" Unicode BE"));
		nPaneSizes[SBPANE_TYPE] = 8 * options.nStatusFontWidth + 4;
	}
	else if (bUnix) {
		wsprintf(szPane, _T("UNIX"));
	}
	else {
		wsprintf(szPane, _T(" DOS"));
	}

	SendMessage(status, SB_SETTEXT, (WPARAM) SBPANE_TYPE, (LPARAM)(LPTSTR)szPane);

	lLines = SendMessage(client, EM_GETLINECOUNT, 0, 0);

#ifdef USE_RICH_EDIT
	if (bInsertMode)
		wsprintf(szPane, _T(" INS"));
	else
		wsprintf(szPane, _T("OVR"));
	nPaneSizes[SBPANE_INS] = nPaneSizes[SBPANE_INS - 1] + 4 * options.nStatusFontWidth - 3;
	SendMessage(status, SB_SETTEXT, (WPARAM) SBPANE_INS, (LPARAM)(LPTSTR)szPane);
#else
	wsprintf(szPane, _T(" Bytes: %d "), CalculateFileSize());
	nPaneSizes[SBPANE_INS] = nPaneSizes[SBPANE_INS - 1] + (int)((options.nStatusFontWidth/STATUS_FONT_CONST) * lstrlen(szPane));
	SendMessage(status, SB_SETTEXT, (WPARAM) SBPANE_INS, (LPARAM)(LPTSTR)szPane);
#endif

	/*
	nPaneSizes[SBPANE_READ] = nPaneSizes[SBPANE_READ - 1] + 2 * options.nStatusFontWidth + 4;
	if (bReadOnly)
		wsprintf(szPane, "READ");
	else
		wsprintf(szPane, " WRI");
	SendMessage(status, SB_SETTEXT, (WPARAM) SBPANE_READ, (LPARAM)(LPTSTR)szPane);
	*/

#ifdef USE_RICH_EDIT
	SendMessage(client, EM_EXGETSEL, 0, (LPARAM)&cr);
	lLine = SendMessage(client, EM_EXLINEFROMCHAR, 0, (LPARAM)cr.cpMax);
#else
	SendMessage(client, EM_GETSEL, (WPARAM)&cr.cpMin, (LPARAM)&cr.cpMax);
	lLine = SendMessage(client, EM_LINEFROMCHAR, (WPARAM)cr.cpMax, 0);
#endif
	wsprintf(szPane, _T(" Line: %d/%d"), lLine+1, lLines);
	SendMessage(status, SB_SETTEXT, (WPARAM) SBPANE_LINE, (LPARAM)(LPTSTR)szPane);

	nPaneSizes[SBPANE_LINE] = nPaneSizes[SBPANE_LINE - 1] + (int)((options.nStatusFontWidth/STATUS_FONT_CONST) * lstrlen(szPane) + 2);

	lLineLen = SendMessage(client, EM_LINELENGTH, (WPARAM)cr.cpMax, 0);
	szBuffer = (LPTSTR) GlobalAlloc(GPTR, (lLineLen+2) * sizeof(TCHAR));
	*((LPWORD)szBuffer) = (USHORT)(lLineLen + 1);
	SendMessage(client, EM_GETLINE, (WPARAM)lLine, (LPARAM) (LPCTSTR)szBuffer);
	szBuffer[lLineLen] = '\0';
	
	lLineIndex = SendMessage(client, EM_LINEINDEX, (WPARAM)lLine, 0);
	while (lLineLen && szBuffer[i] && i < cr.cpMax - lLineIndex) {
		if (szBuffer[i] == '\t')
			lCol += options.nTabStops - (lCol-1) % options.nTabStops;
		else
			lCol++;
		i++;
	}
	wsprintf(szPane, _T(" Col: %d"), lCol);
	SendMessage(status, SB_SETTEXT, (WPARAM) SBPANE_COL, (LPARAM)(LPTSTR)szPane);
	nPaneSizes[SBPANE_COL] = nPaneSizes[SBPANE_COL - 1] + (int)((options.nStatusFontWidth/STATUS_FONT_CONST) * lstrlen(szPane) + 2);

	/*
	if (bHideMessage)
		szPane[0] = '\0';
	else
		lstrcpy(szPane, szStatusMessage);
	*/

	nPaneSizes[SBPANE_MESSAGE] = nPaneSizes[SBPANE_MESSAGE - 1] + 1000;//(int)((options.nStatusFontWidth/STATUS_FONT_CONST) * lstrlen(szPane));
	SendMessage(status, SB_SETTEXT, (WPARAM) SBPANE_MESSAGE | SBT_NOBORDERS, (LPARAM)(bHideMessage ? "" : szStatusMessage));

	SendMessage(status, SB_SETPARTS, NUMPANES, (DWORD)(LPINT)nPaneSizes);
	GlobalFree((HGLOBAL)szBuffer);
}

LRESULT APIENTRY FindProc(HWND hwndFind, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_COMMAND:
		if ((LOWORD(wParam) == IDC_ESCAPE || LOWORD(wParam) == IDC_ESCAPE2) && HIWORD(wParam) == BN_CLICKED) {
			HMENU hmenu = LoadMenu(hinstLang, (LPCTSTR)IDR_ESCAPE_SEQUENCES);
			HMENU hsub = GetSubMenu(hmenu, 0);
			RECT rect;
			UINT id;
			TCHAR szText[MAXFIND];

			SendMessage(hwnd, WM_INITMENUPOPUP, (WPARAM)hsub, MAKELPARAM(1, FALSE));
			if (LOWORD(wParam) == IDC_ESCAPE) {
				GetWindowRect(GetDlgItem(hwndFind, IDC_ESCAPE), &rect);
				SendDlgItemMessage(hwndFind, ID_DROP_FIND, WM_GETTEXT, (WPARAM)MAXFIND, (LPARAM)szText);
			}
			else {
				GetWindowRect(GetDlgItem(hwndFind, IDC_ESCAPE2), &rect);
				SendDlgItemMessage(hwndFind, ID_DROP_REPLACE, WM_GETTEXT, (WPARAM)MAXFIND, (LPARAM)szText);
			}

			if (bNoFindHidden) {
				MENUITEMINFO mio;

				mio.cbSize = sizeof(MENUITEMINFO);
				mio.fMask = MIIM_STATE;
				mio.fState = MFS_CHECKED;
				SetMenuItemInfo(hsub, ID_ESCAPE_DISABLE, FALSE, &mio);
				EnableMenuItem(hsub, ID_ESCAPE_NEWLINE, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(hsub, ID_ESCAPE_TAB, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(hsub, ID_ESCAPE_BACKSLASH, MF_BYCOMMAND | MF_GRAYED);
			}

			id = TrackPopupMenuEx(hsub, TPM_RETURNCMD | TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, rect.left, rect.bottom, hwnd, NULL);

			switch (id) {
			case ID_ESCAPE_NEWLINE:
				lstrcat(szText, "\\n");
				break;
			case ID_ESCAPE_TAB:
				lstrcat(szText, "\\t");
				break;
			case ID_ESCAPE_BACKSLASH:
				lstrcat(szText, "\\\\");
				break;
			case ID_ESCAPE_DISABLE:
				bNoFindHidden = !GetCheckedState(hsub, ID_ESCAPE_DISABLE, TRUE);
				break;
			}

			if (LOWORD(wParam) == IDC_ESCAPE) {
				SendDlgItemMessage(hwndFind, ID_DROP_FIND, WM_SETTEXT, (WPARAM)(BOOL)FALSE, (LPARAM)szText);
			}
			else {
				SendDlgItemMessage(hwndFind, ID_DROP_REPLACE, WM_SETTEXT, (WPARAM)(BOOL)FALSE, (LPARAM)szText);
			}

			DestroyMenu(hmenu);
		}
		break;
	}
	return CallWindowProc(wpOrigFindProc, hwndFind, uMsg, wParam, lParam);
}

LRESULT APIENTRY EditProc(HWND hwndEdit, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
#ifdef USE_RICH_EDIT
	case WM_VSCROLL:
	case WM_HSCROLL:
		if ((uMsg == WM_HSCROLL && LOWORD(wParam) == SB_LINERIGHT || LOWORD(wParam) == SB_PAGERIGHT) ||
		   (uMsg == WM_VSCROLL && LOWORD(wParam) == SB_LINEDOWN || LOWORD(wParam) == SB_PAGEDOWN)) {
			SCROLLINFO si;
			UINT nSbType = (uMsg == WM_VSCROLL ? SB_VERT : SB_HORZ);

			SendMessage(hwndEdit, WM_SETREDRAW, (WPARAM)FALSE, 0);
			if (EnableScrollBar(hwndEdit, nSbType, ESB_ENABLE_BOTH)) {
				EnableScrollBar(hwndEdit, nSbType, ESB_DISABLE_BOTH);
				SendMessage(hwndEdit, WM_SETREDRAW, (WPARAM)TRUE, 0);
				return 0;
			}
			SendMessage(hwndEdit, WM_SETREDRAW, (WPARAM)TRUE, 0);

			si.cbSize = sizeof(si);
			si.fMask = SIF_PAGE|SIF_POS|SIF_RANGE;
			GetScrollInfo(hwndEdit, nSbType, &si);
			if (si.nPos >= si.nMax - (int)si.nPage + 1)
				return 0;

			CallWindowProc(wpOrigEditProc, hwndEdit, uMsg, wParam, lParam);

			if (LOWORD(wParam) == SB_PAGERIGHT) {
				GetScrollInfo(hwndEdit, nSbType, &si);
				SendMessage(hwndEdit, uMsg, MAKEWPARAM(SB_THUMBPOSITION, si.nPos), 0);
			}
		}
		else {
			CallWindowProc(wpOrigEditProc, hwndEdit, uMsg, wParam, lParam);
		}
		return 0;
	case WM_MOUSEWHEEL:
		{
			UINT i, nLines = 3;

			SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &nLines, 0);
			if (nLines > 1000) {
				PostMessage(hwndEdit, WM_VSCROLL, (short)HIWORD(wParam) > 0 ? SB_PAGEUP : SB_PAGEDOWN, 0);
			}
			else {
				for (i = 0; i < nLines; ++i) {
					PostMessage(hwndEdit, WM_VSCROLL, (short)HIWORD(wParam) > 0 ? SB_LINEUP : SB_LINEDOWN, 0);
				}
			}
		}
		return 0;
	case WM_PASTE:
		return 0;
#endif

#ifdef USE_BOOKMARKS
	case WM_PAINT:
		{
			HDC clientDC = GetDC(client);
			TEXTMETRIC tm;
			LRESULT lRes = CallWindowProc(wpOrigEditProc, hwndEdit, uMsg, wParam, lParam);
			INT nFontHeight, nVis;

			INT nLine = 5;

			SelectObject (clientDC, hfontmain);

			if (!GetTextMetrics(clientDC, &tm))
				ReportLastError();

			nFontHeight = (tm.tmHeight /*- tm.tmInternalLeading*/);

			nVis = CallWindowProc(wpOrigEditProc, hwndEdit, EM_GETFIRSTVISIBLELINE, 0, 0);

			if (nVis < nLine) {
				nLine -= nVis;
				SetPixel(clientDC, 4, -(nFontHeight/2) + nFontHeight * nLine, RGB(255, 0, 255));
				SetPixel(clientDC, 3, -(1+nFontHeight/2) + nFontHeight * nLine, RGB(255, 0, 255));
				SetPixel(clientDC, 4, -(1+nFontHeight/2) + nFontHeight * nLine, RGB(255, 0, 255));
				SetPixel(clientDC, 5, -(1+nFontHeight/2) + nFontHeight * nLine, RGB(255, 0, 255));
				SetPixel(clientDC, 4, -(2+nFontHeight/2) + nFontHeight * nLine, RGB(255, 0, 255));
			}
			
			ReleaseDC(client, clientDC);
			return lRes;
		}
		break;
#endif
	case WM_LBUTTONUP:
	case WM_LBUTTONDOWN:
		{
			LRESULT lRes = CallWindowProc(wpOrigEditProc, hwndEdit, uMsg, wParam, lParam);
			UpdateStatus();
			return lRes;
		}
	case WM_LBUTTONDBLCLK:
#ifndef USE_RICH_EDIT
		if (bSmartSelect) {

			CHARRANGE cr;

			SendMessage(client, EM_GETSEL, (WPARAM)&cr.cpMin, (LPARAM)&cr.cpMax);
			CallWindowProc(wpOrigEditProc, hwndEdit, uMsg, wParam, lParam);
			SendMessage(client, EM_SETSEL, (WPARAM)cr.cpMin, (LPARAM)cr.cpMax);
			SelectWord(FALSE, TRUE, TRUE);
			return 0;
		}
		else {
			return CallWindowProc(wpOrigEditProc, hwndEdit, uMsg, wParam, lParam);
		}
#else
		{
			CHARRANGE cr;

			SendMessage(client, EM_EXGETSEL, 0, (LPARAM)&cr);
			CallWindowProc(wpOrigEditProc, hwndEdit, uMsg, wParam, lParam);
			SendMessage(client, EM_EXSETSEL, 0, (LPARAM)&cr);
			SelectWord(FALSE, bSmartSelect, TRUE);
			return 0;
		}
//		if (!bSmartSelect) {
/*		}
		else {
			return CallWindowProc(wpOrigEditProc, hwndEdit, uMsg, wParam, lParam);
		}*/
#endif
	case WM_RBUTTONUP:
		{
			HMENU hmenu = LoadMenu(hinstLang, MAKEINTRESOURCE(IDR_POPUP));
			HMENU hsub = GetSubMenu(hmenu, 0);
			POINT pt;
			UINT id;
			CHARRANGE cr;

#ifndef USE_RICH_EDIT
			DeleteMenu(hsub, 1, MF_BYPOSITION);
#endif

/*			if (bLinkMenu) {
				bLinkMenu = FALSE;
			}
*/
#ifdef USE_RICH_EDIT
			SendMessage(client, EM_EXGETSEL, 0, (LPARAM)&cr);
#else
			SendMessage(client, EM_GETSEL, (WPARAM)&cr.cpMin, (LPARAM)&cr.cpMax);
#endif
			if (options.bContextCursor) {
				if (cr.cpMin == cr.cpMax) {
#ifdef USE_RICH_EDIT
					BOOL bOld = options.bLinkDoubleClick;
					options.bLinkDoubleClick = TRUE;
#endif
					SendMessage(hwndEdit, WM_LBUTTONDOWN, wParam, lParam);
					SendMessage(hwndEdit, WM_LBUTTONUP, wParam, lParam);
#ifdef USE_RICH_EDIT
					options.bLinkDoubleClick = bOld;
#endif
				}
			}
			SendMessage(hwnd, WM_INITMENUPOPUP, (WPARAM)hsub, MAKELPARAM(1, FALSE));
			GetCursorPos(&pt);
			id = TrackPopupMenuEx(hsub, TPM_RETURNCMD | TPM_TOPALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, hwnd, NULL);
			PostMessage(hwnd, WM_COMMAND, MAKEWPARAM(id, 0), 0);
			DestroyMenu(hmenu);
			return 0;
		}
	case WM_KEYUP:
	case WM_KEYDOWN:
		{
			LRESULT lRes = CallWindowProc(wpOrigEditProc, hwndEdit, uMsg, wParam, lParam);
			UpdateStatus();
			return lRes;
		}
	case WM_CHAR:
		if ((TCHAR) wParam == '\r') {
			CallWindowProc(wpOrigEditProc, hwndEdit, uMsg, wParam, lParam);
			if (options.bAutoIndent) {
				int i = 0;
				LONG lLineLen, lLine;	
				LPTSTR szBuffer, szIndent;
				CHARRANGE cr;

#ifdef USE_RICH_EDIT
				CallWindowProc(wpOrigEditProc, client, EM_EXGETSEL, 0, (LPARAM)&cr);
				lLine = CallWindowProc(wpOrigEditProc, client, EM_EXLINEFROMCHAR, 0, (LPARAM)cr.cpMin);
#else
				CallWindowProc(wpOrigEditProc, client, EM_GETSEL, (WPARAM)&cr.cpMin, 0);
				lLine = CallWindowProc(wpOrigEditProc, client, EM_LINEFROMCHAR, (WPARAM)cr.cpMin, 0);
#endif
				lLineLen = CallWindowProc(wpOrigEditProc, client, EM_LINELENGTH, (WPARAM)cr.cpMin, 0);

				lLineLen = CallWindowProc(wpOrigEditProc, client, EM_LINELENGTH, (WPARAM)cr.cpMin - 1, 0);
				
				szBuffer = (LPTSTR) GlobalAlloc(GPTR, (lLineLen+2) * sizeof(TCHAR));
				szIndent = (LPTSTR) GlobalAlloc(GPTR, (lLineLen+2) * sizeof(TCHAR));

				*((LPWORD)szBuffer) = (USHORT)(lLineLen + 1);
				CallWindowProc(wpOrigEditProc, client, EM_GETLINE, (WPARAM)lLine - 1, (LPARAM)(LPCTSTR)szBuffer);
				szBuffer[lLineLen] = '\0';
				while (szBuffer[i] == '\t' || szBuffer[i] == ' ') {
					szIndent[i] = szBuffer[i];
					i++;
				}
				szIndent[i] = '\0';
				CallWindowProc(wpOrigEditProc, client, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)szIndent);
				GlobalFree((HGLOBAL)szBuffer);
				GlobalFree((HGLOBAL)szIndent);
			}
			return 0;
		}
		break;
	case EM_REPLACESEL:
		{
			LRESULT lTmp;
			LONG lStartLine, lEndLine;
			CHARRANGE cr;

#ifdef USE_RICH_EDIT
			SendMessage(client, EM_EXGETSEL, 0, (LPARAM)&cr);
			lStartLine = SendMessage(client, EM_EXLINEFROMCHAR, 0, (LPARAM)cr.cpMin);
			lEndLine = SendMessage(client, EM_EXLINEFROMCHAR, 0, (LPARAM)cr.cpMax);
#else
			SendMessage(client, EM_GETSEL, (WPARAM)&cr.cpMin, (LPARAM)&cr.cpMax);
			lStartLine = SendMessage(client, EM_LINEFROMCHAR, (WPARAM)cr.cpMin, 0);
			lEndLine = SendMessage(client, EM_LINEFROMCHAR, (WPARAM)cr.cpMax, 0);
#endif

			if (cr.cpMin == cr.cpMax || lStartLine == lEndLine) break;

			if (!bReplacingAll)
				SendMessage(hwndEdit, WM_SETREDRAW, (WPARAM)FALSE, 0);
			lTmp = CallWindowProc(wpOrigEditProc, hwndEdit, uMsg, wParam, lParam);
			if (!bReplacingAll)
				SendMessage(hwndEdit, WM_SETREDRAW, (WPARAM)TRUE, 0);
			SetWindowPos(client, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
			
			return lTmp;
		}
	} 
	return CallWindowProc(wpOrigEditProc, hwndEdit, uMsg, wParam, lParam);
}
 
BOOL CALLBACK AbortDlgProc(HDC hdc, int nCode) 
{ 
	MSG msg; 
 
	while (PeekMessage((LPMSG) &msg, (HWND) NULL, 0, 0, PM_REMOVE)) { 
		if (!IsDialogMessage(hdlgCancel, (LPMSG) &msg)) { 
			TranslateMessage((LPMSG) &msg); 
			DispatchMessage((LPMSG) &msg); 
		} 
	} 
	return bPrint;
}

LRESULT CALLBACK AbortPrintJob(HWND hwndDlg, UINT message, WPARAM wParam, LPARAM lParam)
{ 
	switch (message) { 
		case WM_INITDIALOG:
			CenterWindow(hwndDlg);
			SetDlgItemText(hwndDlg, IDD_FILE, szFile); 
			return TRUE; 
		case WM_COMMAND:
			bPrint = FALSE; 
			return TRUE; 
		default: 
			return FALSE;
	} 
} 

int FixShortFilename(TCHAR *szSrc, TCHAR *szDest)
{
	WIN32_FIND_DATA FindFileData;
	HANDLE hHandle;
	TCHAR sDir[MAXFN], sName[MAXFN];
	int nDestPos=0, nSrcPos=0, i;
	BOOL bOK = TRUE;

	// Copy drive letter over
	if (szSrc[1] == ':') {
		szDest[nDestPos++] = szSrc[nSrcPos++];
		szDest[nDestPos++] = szSrc[nSrcPos++];
	}

	while (szSrc[nSrcPos]) {
		// If the next TCHAR is '\' we are starting from the root and want to add '\*' to sDir.
		// Otherwise we are doing relative search, so we just append '*' to sDir
		if (szSrc[nSrcPos]=='\\') {
			szDest[nDestPos++] = szSrc[nSrcPos++];

			if (szSrc[nSrcPos] == '\\') { // get UNC server name
				szDest[nDestPos++] = szSrc[nSrcPos++];

				while (szSrc[nSrcPos] && szSrc[nSrcPos - 1]!='\\') {
					szDest[nDestPos++] = szSrc[nSrcPos++];
				}
			}
		}
	
		_tcsncpy(sDir, szDest, nDestPos);		
		sDir[nDestPos] = '*';
		sDir[nDestPos + 1] = '\0';

		for (i=0; szSrc[nSrcPos] && szSrc[nSrcPos]!='\\'; i++)
			sName[i] = szSrc[nSrcPos++];
		sName[i] = '\0';

		hHandle = FindFirstFile(sDir, &FindFileData);
		bOK = (hHandle != INVALID_HANDLE_VALUE);
		while (bOK && lstrcmpi(FindFileData.cFileName, sName) != 0 && lstrcmpi(FindFileData.cAlternateFileName, sName) != 0)
			bOK = FindNextFile(hHandle, &FindFileData);

    	if (bOK)
    		_tcscpy(&szDest[nDestPos], FindFileData.cFileName);
    	else
    		_tcscpy(&szDest[nDestPos], sName);

		// Fix the length of szDest
		nDestPos = _tcslen(szDest);
		if (hHandle)
			FindClose(hHandle);
	}
	return !bOK;
}

void ExpandFilename(LPTSTR szBuffer)
{
	WIN32_FIND_DATA FileData;
	HANDLE hSearch;
	TCHAR szTmp[MAXFN];

	lstrcpy(szTmp, szBuffer);
	FixShortFilename(szTmp, szBuffer);

	if (szDir[0] != _T('\0'))
		SetCurrentDirectory(szDir);

	hSearch = FindFirstFile(szBuffer, &FileData);
	szCaptionFile[0] = _T('\0');
	if (hSearch != INVALID_HANDLE_VALUE) {
		int result;
		LPCTSTR pdest;
		pdest = _tcsrchr(szBuffer, _T('\\'));
		if (pdest) {
			result = pdest - szBuffer + 1;
			lstrcpyn(szDir, szBuffer, result);
		}		
		if (szDir[lstrlen(szDir) - 1] != _T('\\'))
			lstrcat(szDir, _T("\\"));
		
		if (!options.bNoCaptionDir) {
			lstrcat(szCaptionFile, szDir);
		}
		lstrcat(szCaptionFile, FileData.cFileName);
		FindClose(hSearch);
	}
	else {
		if (!options.bNoCaptionDir) {
			lstrcat(szCaptionFile, szDir);
		}
		lstrcat(szCaptionFile, szFile);
	}
}

#ifndef USE_RICH_EDIT
void PrintContents()
{
	PRINTDLG pd;
	DOCINFO di;
	RECT rectdev, rect;
	int nHeight, nError;
	LPCTSTR startAt;
	LONG lStringLen;
	int totalDone = 0;
	UINT page;
	HFONT hprintfont = NULL, *oldfont = NULL;
	LOGFONT storefont;
	HDC clientDC;
	LONG lHeight;
	CHARRANGE cr;
	LPTSTR szBuffer;
	BOOL bUseDefault;

	ZeroMemory(&pd, sizeof(PRINTDLG));
	pd.lStructSize = sizeof(PRINTDLG);
	pd.hDevMode = (HANDLE) NULL;
	pd.hDevNames = (HANDLE) NULL;
	pd.Flags = PD_RETURNDC | PD_NOPAGENUMS;
	pd.hwndOwner = hwnd;
	pd.hDC = (HDC) NULL;
	pd.nCopies = 1;

	SendMessage(client, EM_GETSEL, (WPARAM)&cr.cpMin, (LPARAM)&cr.cpMax);

	if (cr.cpMin != cr.cpMax) {
		pd.Flags |= PD_SELECTION;
	}
	else {
		pd.Flags |= PD_NOSELECTION;
	}

	if (!PrintDlg(&pd)) {
		DWORD x = CommDlgExtendedError();

		if (x && pd.hDC == NULL) {
			ERROROUT(GetString(IDS_PRINTER_NOT_FOUND));
			return;
		}

		if (x) {
			TCHAR szBuff[50];
			wsprintf(szBuff, GetString(IDS_PRINT_INIT_ERROR), x);
			ERROROUT(szBuff);
		}
		return;
	}

	if (pd.Flags & PD_SELECTION) {
		szBuffer = (LPTSTR) GlobalAlloc(GPTR, (cr.cpMax - cr.cpMin + 1) * sizeof(TCHAR));
		GetClientRange(cr.cpMin, cr.cpMax, szBuffer);
	}
	else {
		szBuffer = (LPTSTR)GetShadowBuffer();
		if (szBuffer == NULL) {
			ReportLastError();
			return;
		}
	}
	lStringLen = lstrlen(szBuffer);
	
	bPrint = TRUE; 

	if (SetAbortProc(pd.hDC, AbortDlgProc) == SP_ERROR) {
		ERROROUT(GetString(IDS_PRINT_ABORT_ERROR));
		return;
	}
 
	hdlgCancel = CreateDialog(hinstLang, MAKEINTRESOURCE(IDD_ABORT_PRINT), hwnd, (DLGPROC) AbortPrintJob); 
	ShowWindow(hdlgCancel, SW_SHOW);
 
	EnableWindow(hwnd, FALSE); 

	bUseDefault = options.bDefaultPrintFont;

	if (bPrimaryFont)
		bUseDefault |= (options.nPrimaryFont == 0);
	else
		bUseDefault |= (options.nSecondaryFont == 0);

	if (!bUseDefault) {
		long lScale = 1;
		int nPrinter, nScreen;
		
		clientDC = GetDC(client);
		nPrinter = GetDeviceCaps(pd.hDC, LOGPIXELSY);
		nScreen = GetDeviceCaps(clientDC, LOGPIXELSY);

		//if (nPrinter > nScreen)
			lScale = (long)(nPrinter / nScreen);
		//else
		//	lScale = (long)(nScreen / nPrinter);

		if (bPrimaryFont) {
			storefont = options.PrimaryFont;
			lHeight = options.PrimaryFont.lfHeight * lScale;
			options.PrimaryFont.lfHeight = lHeight;
			SetFont(&hprintfont, bPrimaryFont);
			options.PrimaryFont = storefont;
		}
		else {
			storefont = options.SecondaryFont;
			lHeight = options.SecondaryFont.lfHeight * lScale;
			options.SecondaryFont.lfHeight = lHeight;
			SetFont(&hprintfont, bPrimaryFont);
			options.SecondaryFont = storefont;
		}
		ReleaseDC(client, clientDC);
		oldfont = SelectObject(pd.hDC, hprintfont);
	}

	ZeroMemory(&di, sizeof(DOCINFO));
	di.cbSize = sizeof(DOCINFO);
	di.lpszDocName = szFile;
	
	nError = StartDoc(pd.hDC, &di);
	if (nError <= 0) {
		ReportLastError();
		ERROROUT(GetString(IDS_PRINT_START_ERROR));
		goto Error;
	}
	
	/*
	rectdev.left = rectdev.top = 0;
	rectdev.right = GetDeviceCaps(pd.hDC, HORZRES);
	rectdev.bottom = GetDeviceCaps(pd.hDC, VERTRES);
	*/
/*
	rectdev.left = (long)(options.rMargins.left / 1000. * 1440);
	rectdev.top = (long)(options.rMargins.top / 1000. * 1440);
	rectdev.right = GetDeviceCaps(pd.hDC, HORZRES) - (long)(options.rMargins.right / 1000. * 1440);
	rectdev.bottom = GetDeviceCaps(pd.hDC, VERTRES) - (long)(options.rMargins.bottom / 1000. * 1440);
*/	
	{
		int nDPIx = GetDeviceCaps(pd.hDC, LOGPIXELSX);
		int nDPIy = GetDeviceCaps(pd.hDC, LOGPIXELSY);
		//int nMarginX = GetDeviceCaps(pd.hDC, PHYSICALOFFSETX);
		//int nMarginY = GetDeviceCaps(pd.hDC, PHYSICALOFFSETY);

		rectdev.left = (long)(options.rMargins.left / 1000. * nDPIx);
		rectdev.top = (long)(options.rMargins.top / 1000. * nDPIy);
		rectdev.right = GetDeviceCaps(pd.hDC, HORZRES) - (long)(options.rMargins.right / 1000. * nDPIx);
		rectdev.bottom = GetDeviceCaps(pd.hDC, VERTRES) - (long)(options.rMargins.bottom / 1000. * nDPIy);
	}

	startAt = szBuffer;

	for (page = pd.nMinPage; (nError > 0) && (totalDone < lStringLen); page++)
	{
		int nLo = 0;
		int nHi = lStringLen - totalDone;
		int nCount = nHi;
		int nRet = 0;

		nError = StartPage(pd.hDC);
		if (nError <= 0) break;

		SetMapMode(pd.hDC, MM_TEXT);
		if (hprintfont != NULL)
			SelectObject(pd.hDC, hprintfont);

		rect = rectdev;
		while (nLo < nHi) {
			rect.right = rectdev.right;
			nHeight = DrawText(pd.hDC, startAt, nCount, &rect, DT_CALCRECT|DT_WORDBREAK|DT_NOCLIP|DT_EXPANDTABS|DT_NOPREFIX);
			if (nHeight < rectdev.bottom)
				nLo = nCount;
			if (nHeight > rectdev.bottom)
				nHi = nCount;
			if (nLo == nHi - 1)
				nLo = nHi;
			if (nLo < nHi)
				nCount = nLo + (nHi - nLo)/2;
		}
		nRet = DrawText(pd.hDC, startAt, nCount, &rect, DT_WORDBREAK|DT_NOCLIP|DT_EXPANDTABS|DT_NOPREFIX);
		if (nRet == 0) {
			ERROROUT(GetString(IDS_DRAWTEXT_ERROR));
			break;
		}
		startAt += nCount;
		totalDone += nCount;

		nError = EndPage(pd.hDC);
	}

	if (nError > 0) {
		EndDoc(pd.hDC);
	}
	else {
		AbortDoc(pd.hDC);
		ReportLastError();
		ERROROUT(GetString(IDS_PRINT_ERROR));
	}
	
Error:
	if (!options.bDefaultPrintFont) {
		SelectObject(pd.hDC, oldfont);
		DeleteObject(hprintfont);
	}
	if (pd.Flags & PD_SELECTION) {
		GlobalFree((HGLOBAL)szBuffer);
	}

	EnableWindow(hwnd, TRUE);
	DestroyWindow(hdlgCancel);
	DeleteDC(pd.hDC);
}

#else // USE_RICH_EDIT

void PrintContents(void)
{
	PRINTDLG pd;
	DOCINFO di;
	int nError;
	FORMATRANGE fr;
	int nHorizRes;
	int nVertRes;
	int nLogPixelsX;
	int nLogPixelsY;
	LONG lTextLength;
	LONG lTextPrinted;
	int i;
	CHARRANGE cr;

	ZeroMemory(&pd, sizeof(PRINTDLG));
	pd.lStructSize = sizeof(PRINTDLG);
	pd.hDevMode = (HANDLE) NULL;
	pd.hDevNames = (HANDLE) NULL;
	pd.Flags = PD_RETURNDC | PD_NOPAGENUMS;

	SendMessage(client, EM_EXGETSEL, 0, (LPARAM)&cr);

	if (cr.cpMin != cr.cpMax) {
		pd.Flags |= PD_SELECTION;
	}
	else {
		pd.Flags |= PD_NOSELECTION;
	}

	pd.hwndOwner = hwnd;
	pd.hDC = (HDC) NULL;
	pd.nCopies = 1;

	if (!PrintDlg(&pd)) {
		DWORD x = CommDlgExtendedError();

		if (x && pd.hDC == NULL) {
			ERROROUT(GetString(IDS_PRINTER_NOT_FOUND));
			return;
		}
		
		if (x) {
			TCHAR szBuff[50];
			wsprintf(szBuff, GetString(IDS_PRINT_INIT_ERROR), x);
			ERROROUT(szBuff);
		}
		return;
	}
	
	bPrint = TRUE;

	if (SetAbortProc(pd.hDC, AbortDlgProc) == SP_ERROR) {
		ERROROUT(GetString(IDS_PRINT_ABORT_ERROR));
		return;
	}
 
	hdlgCancel = CreateDialog(hinstLang, MAKEINTRESOURCE(IDD_ABORT_PRINT), hwnd, (DLGPROC) AbortPrintJob); 
	ShowWindow(hdlgCancel, SW_SHOW);
 
	EnableWindow(hwnd, FALSE); 

	nHorizRes = GetDeviceCaps(pd.hDC, HORZRES);
	nVertRes = GetDeviceCaps(pd.hDC, VERTRES);
	nLogPixelsX = GetDeviceCaps(pd.hDC, LOGPIXELSX);
	nLogPixelsY = GetDeviceCaps(pd.hDC, LOGPIXELSY);

	SetMapMode(pd.hDC, MM_TEXT);

	ZeroMemory(&fr, sizeof(fr));
	fr.hdc = fr.hdcTarget = pd.hDC;

	fr.rcPage.left = fr.rcPage.top = 0;
	fr.rcPage.right = (nHorizRes/nLogPixelsX) * 1440;
	fr.rcPage.bottom = (nVertRes/nLogPixelsY) * 1440;

	fr.rc.left = fr.rcPage.left + (long)(options.rMargins.left / 1000. * 1440);
	fr.rc.top = fr.rcPage.top + (long)(options.rMargins.top / 1000. * 1440);
	fr.rc.right = fr.rcPage.right - (long)(options.rMargins.right / 1000. * 1440);
	fr.rc.bottom = fr.rcPage.bottom - (long)(options.rMargins.bottom / 1000. * 1440);

	ZeroMemory(&di, sizeof(di));
	di.cbSize = sizeof(di);
	di.lpszDocName = szCaptionFile;
	di.lpszOutput = NULL;

	lTextLength = GetWindowTextLength(client) - (SendMessage(client, EM_GETLINECOUNT, 0, 0) - 1);

	for (i = 0; i < pd.nCopies; ++i) {

		if (!(pd.Flags & PD_SELECTION)) {
			fr.chrg.cpMin = 0;
			fr.chrg.cpMax = -1;
		}
		else fr.chrg = cr;

		if (StartDoc(pd.hDC, &di) <= 0) {
			ReportLastError();
			ERROROUT(GetString(IDS_PRINT_START_ERROR));
			goto Error;
		}
		
		do {
			nError = StartPage(pd.hDC);
			if (nError <= 0) break;

			lTextPrinted = SendMessage(client, EM_FORMATRANGE, FALSE, (LPARAM)&fr);
			SendMessage(client, EM_DISPLAYBAND, 0, (LPARAM)&fr.rc);

			nError = EndPage(pd.hDC);
			if (nError <= 0) break;

			fr.chrg.cpMin = lTextPrinted;

		} while (lTextPrinted < lTextLength && fr.chrg.cpMin != fr.chrg.cpMax);

		SendMessage(client, EM_FORMATRANGE, 0, (LPARAM)NULL);

		if (nError > 0) {
			EndDoc(pd.hDC);
		}
		else {
			AbortDoc(pd.hDC);
			ReportLastError();
			ERROROUT(GetString(IDS_PRINT_ERROR));
			break;
		}
	}

Error:
	EnableWindow(hwnd, TRUE);
	DestroyWindow(hdlgCancel);
	DeleteDC(pd.hDC);
}
#endif

void ReportLastError(void)
{
	LPVOID lpMsgBuf;

	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM, NULL,
				GetLastError(), MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				(LPTSTR) &lpMsgBuf, 0, NULL);
	MessageBox(NULL, lpMsgBuf, STR_METAPAD, MB_OK | MB_ICONSTOP);

	LocalFree(lpMsgBuf);
/*
#ifndef	_DEBUG
	PostQuitMessage(0);
#endif
*/
}

void LaunchPrimaryExternalViewer(void)
{
	TCHAR szLaunch[MAXFN] = {'\0'};
	
	lstrcat(szLaunch, options.szArgs);
	lstrcat(szLaunch, _T(" \""));
	lstrcat(szLaunch, szFile);
	lstrcat(szLaunch, _T("\""));
	if (!ExecuteProgram(options.szBrowser, szLaunch))
		ERROROUT(GetString(IDS_PRIMARY_VIEWER_ERROR));
}

void LaunchSecondaryExternalViewer(void)
{
	TCHAR szLaunch[MAXFN] = {'\0'};
	
	lstrcat(szLaunch, options.szArgs2);
	lstrcat(szLaunch, _T(" \""));
	lstrcat(szLaunch, szFile);
	lstrcat(szLaunch, _T("\""));
	if (!ExecuteProgram(options.szBrowser2, szLaunch))
		ERROROUT(GetString(IDS_SECONDARY_VIEWER_ERROR));
}

void LaunchInViewer(BOOL bCustom, BOOL bSecondary)
{
	if (bCustom) {
		if (!bSecondary && options.szBrowser[0] == '\0') {
			MessageBox(hwnd, GetString(IDS_PRIMARY_VIEWER_MISSING), STR_METAPAD, MB_OK|MB_ICONEXCLAMATION);
			SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_VIEW_OPTIONS, 0), 0);
			return;
		}
//DBGOUT(options.szBrowser2, "run szBrowser2 value:");
		if (bSecondary && options.szBrowser2[0] == '\0') {
			MessageBox(hwnd, GetString(IDS_SECONDARY_VIEWER_MISSING), STR_METAPAD, MB_OK|MB_ICONEXCLAMATION);
			SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_VIEW_OPTIONS, 0), 0);
			return;
		}
	}

	if (szFile[0] == '\0' || (bDirtyFile && options.nLaunchSave != 2)) {
		int res = IDYES;
		if (options.nLaunchSave == 0) {
			TCHAR szBuffer[MAXFN];
			wsprintf(szBuffer, GetString(IDS_DIRTYFILE), szCaptionFile);
			res = MessageBox(hwnd, szBuffer, STR_METAPAD, MB_ICONEXCLAMATION | MB_YESNOCANCEL);
		}
		if (res == IDCANCEL) {
			return;
		}
		else if (res == IDYES) {
			if (!SaveCurrentFile()) {
				return;
			}
		}
	}
	if (szFile[0] != '\0') {
		if (bCustom) {
			if (bSecondary) {
				LaunchSecondaryExternalViewer();
			}
			else {
				LaunchPrimaryExternalViewer();
			}
		}
		else {
			int ret = (int)ShellExecute(NULL, _T("open"), szFile, NULL, szDir, SW_SHOW);
			if (ret <= 32) {
				switch (ret) {
				case SE_ERR_NOASSOC:
					ERROROUT(GetString(IDS_NO_DEFAULT_VIEWER));
					break;
				default:
					ERROROUT(GetString(IDS_DEFAULT_VIEWER_ERROR));
				}
			}
		}
	}

	if (options.bLaunchClose) {
		DestroyWindow(hwnd);
	}
}

void LoadOptions(void)
{
	HKEY key = NULL;

	options.nTabStops = 4;
	options.rMargins.top = options.rMargins.bottom = options.rMargins.left = options.rMargins.right = 500;
	options.nLaunchSave = options.nPrimaryFont = options.nSecondaryFont = 0;
	options.bNoCaptionDir = options.bFindAutoWrap = options.bSaveWindowPlacement = options.bLaunchClose = options.bQuickExit = TRUE;
	options.bAutoIndent = TRUE;
	options.bInsertSpaces = FALSE;
	options.bSystemColours = TRUE;
	options.bSystemColours2 = TRUE;
	options.bSaveMenuSettings = TRUE;
	options.bSaveDirectory = TRUE;
	options.bNoSmartHome = FALSE;
	options.bNoAutoSaveExt = FALSE;
	options.bContextCursor = FALSE;
	options.bCurrentFindFont = FALSE;
	options.bPrintWithSecondaryFont = FALSE;
	options.bNoSaveHistory = FALSE;
	options.bNoFindAutoSelect = FALSE;
	options.bNoFaves = FALSE;

	lstrcpy(options.szQuote, "> ");
	ZeroMemory(options.szArgs, sizeof(options.szArgs));
	ZeroMemory(options.szBrowser, sizeof(options.szBrowser));
	ZeroMemory(options.szArgs2, sizeof(options.szArgs2));
	ZeroMemory(options.szBrowser2, sizeof(options.szBrowser2));
	ZeroMemory(options.szLangPlugin, sizeof(options.szLangPlugin));
	ZeroMemory(options.szFavDir, sizeof(options.szFavDir));
	ZeroMemory(&options.PrimaryFont, sizeof(LOGFONT));
	ZeroMemory(&options.SecondaryFont, sizeof(LOGFONT));
	ZeroMemory(&options.MacroArray, sizeof(options.MacroArray));
	options.bHideGotoOffset = FALSE;
	options.bRecentOnOwn = FALSE;
	options.bDontInsertTime = FALSE;
	options.bNoWarningPrompt = FALSE;
	options.bUnFlatToolbar = TRUE;
	options.bStickyWindow = FALSE;
	options.bReadOnlyMenu = FALSE;
	//options.nStatusFontWidth = 16;
	options.nSelectionMarginWidth = 10;
	options.nMaxMRU = 8;
	options.nFormatIndex = 0;
	options.nTransparentPct = 25;
	options.BackColour = GetSysColor(COLOR_WINDOW);
	options.FontColour = GetSysColor(COLOR_WINDOWTEXT);
	options.BackColour2 = GetSysColor(COLOR_WINDOW);
	options.FontColour2 = GetSysColor(COLOR_WINDOWTEXT);

#ifndef USE_RICH_EDIT
	options.bDefaultPrintFont = FALSE;
	options.bAlwaysLaunch = FALSE;
#else
	options.bHideScrollbars = FALSE;
	options.bLinkDoubleClick = FALSE;
	options.bSuppressUndoBufferPrompt = FALSE;
#endif

	if (!g_bIniMode) {
		if (RegOpenKeyEx(HKEY_CURRENT_USER, STR_REGKEY, 0, KEY_ALL_ACCESS, &key) != ERROR_SUCCESS) {
			return;
		}
	}

	{
		DWORD dwBufferSize;

		dwBufferSize = sizeof(int);
		
		LoadOptionNumeric(key, _T("bHideGotoOffset"), (LPBYTE)&options.bHideGotoOffset, dwBufferSize);
		LoadOptionNumeric(key, _T("bSystemColours"), (LPBYTE)&options.bSystemColours, dwBufferSize);
		LoadOptionNumeric(key, _T("bSystemColours2"), (LPBYTE)&options.bSystemColours2, dwBufferSize);
		LoadOptionNumeric(key, _T("bNoSmartHome"), (LPBYTE)&options.bNoSmartHome, dwBufferSize);
		LoadOptionNumeric(key, _T("bNoAutoSaveExt"), (LPBYTE)&options.bNoAutoSaveExt, dwBufferSize);
		LoadOptionNumeric(key, _T("bContextCursor"), (LPBYTE)&options.bContextCursor, dwBufferSize);
		LoadOptionNumeric(key, _T("bCurrentFindFont"), (LPBYTE)&options.bCurrentFindFont, dwBufferSize);
		LoadOptionNumeric(key, _T("bPrintWithSecondaryFont"), (LPBYTE)&options.bPrintWithSecondaryFont, dwBufferSize);
		LoadOptionNumeric(key, _T("bNoSaveHistory"), (LPBYTE)&options.bNoSaveHistory, dwBufferSize);
		LoadOptionNumeric(key, _T("bNoFindAutoSelect"), (LPBYTE)&options.bNoFindAutoSelect, dwBufferSize);
		LoadOptionNumeric(key, _T("bRecentOnOwn"), (LPBYTE)&options.bRecentOnOwn, dwBufferSize);
		LoadOptionNumeric(key, _T("bDontInsertTime"), (LPBYTE)&options.bDontInsertTime, dwBufferSize);
		LoadOptionNumeric(key, _T("bNoWarningPrompt"), (LPBYTE)&options.bNoWarningPrompt, dwBufferSize);
		LoadOptionNumeric(key, _T("bUnFlatToolbar"), (LPBYTE)&options.bUnFlatToolbar, dwBufferSize);
		LoadOptionNumeric(key, _T("bStickyWindow"), (LPBYTE)&options.bStickyWindow, dwBufferSize);
		LoadOptionNumeric(key, _T("bReadOnlyMenu"), (LPBYTE)&options.bReadOnlyMenu, dwBufferSize);
//		LoadOptionNumeric(key, _T("nStatusFontWidth"), (LPBYTE)&options.nStatusFontWidth, dwBufferSize);
		LoadOptionNumeric(key, _T("nSelectionMarginWidth"), (LPBYTE)&options.nSelectionMarginWidth, dwBufferSize);		
		LoadOptionNumeric(key, _T("nMaxMRU"), (LPBYTE)&options.nMaxMRU, dwBufferSize);
		LoadOptionNumeric(key, _T("nFormatIndex"), (LPBYTE)&options.nFormatIndex, dwBufferSize);
		LoadOptionNumeric(key, _T("nTransparentPct"), (LPBYTE)&options.nTransparentPct, dwBufferSize);
		LoadOptionNumeric(key, _T("bNoCaptionDir"), (LPBYTE)&options.bNoCaptionDir, dwBufferSize);
		LoadOptionNumeric(key, _T("bAutoIndent"), (LPBYTE)&options.bAutoIndent, dwBufferSize);
		LoadOptionNumeric(key, _T("bInsertSpaces"), (LPBYTE)&options.bInsertSpaces, dwBufferSize);
		LoadOptionNumeric(key, _T("bFindAutoWrap"), (LPBYTE)&options.bFindAutoWrap, dwBufferSize);
		LoadOptionNumeric(key, _T("bQuickExit"), (LPBYTE)&options.bQuickExit, dwBufferSize);
		LoadOptionNumeric(key, _T("bSaveWindowPlacement"), (LPBYTE)&options.bSaveWindowPlacement, dwBufferSize);
		LoadOptionNumeric(key, _T("bSaveMenuSettings"), (LPBYTE)&options.bSaveMenuSettings, dwBufferSize);
		LoadOptionNumeric(key, _T("bSaveDirectory"), (LPBYTE)&options.bSaveDirectory, dwBufferSize);
		LoadOptionNumeric(key, _T("bLaunchClose"), (LPBYTE)&options.bLaunchClose, dwBufferSize);
		LoadOptionNumeric(key, _T("bNoFaves"), (LPBYTE)&options.bNoFaves, dwBufferSize);
#ifndef USE_RICH_EDIT
		LoadOptionNumeric(key, _T("bDefaultPrintFont"), (LPBYTE)&options.bDefaultPrintFont, dwBufferSize);
		LoadOptionNumeric(key, _T("bAlwaysLaunch"), (LPBYTE)&options.bAlwaysLaunch, dwBufferSize);
#else
		LoadOptionNumeric(key, _T("bLinkDoubleClick"), (LPBYTE)&options.bLinkDoubleClick, dwBufferSize);
		LoadOptionNumeric(key, _T("bHideScrollbars"), (LPBYTE)&options.bHideScrollbars, dwBufferSize);
		LoadOptionNumeric(key, _T("bSuppressUndoBufferPrompt"), (LPBYTE)&options.bSuppressUndoBufferPrompt, dwBufferSize);
#endif
		LoadOptionNumeric(key, _T("nLaunchSave"), (LPBYTE)&options.nLaunchSave, dwBufferSize);
		LoadOptionNumeric(key, _T("nTabStops"), (LPBYTE)&options.nTabStops, dwBufferSize);
		LoadOptionNumeric(key, _T("nPrimaryFont"), (LPBYTE)&options.nPrimaryFont, dwBufferSize);
		LoadOptionNumeric(key, _T("nSecondaryFont"), (LPBYTE)&options.nSecondaryFont, dwBufferSize);
		dwBufferSize = sizeof(LOGFONT);
#ifdef BUILD_METAPAD_UNICODE
		LoadOptionBinary(key, _T("PrimaryFont_U"), (LPBYTE)&options.PrimaryFont, dwBufferSize);
		LoadOptionBinary(key, _T("SecondaryFont_U"), (LPBYTE)&options.SecondaryFont, dwBufferSize);
#else
		LoadOptionBinary(key, _T("PrimaryFont"), (LPBYTE)&options.PrimaryFont, dwBufferSize);
		LoadOptionBinary(key, _T("SecondaryFont"), (LPBYTE)&options.SecondaryFont, dwBufferSize);
#endif
		dwBufferSize = sizeof(options.szBrowser);
		LoadOptionString(key, _T("szBrowser"), (LPBYTE)&options.szBrowser, dwBufferSize);
		dwBufferSize = sizeof(options.szBrowser2);
		LoadOptionString(key, _T("szBrowser2"), (LPBYTE)&options.szBrowser2, dwBufferSize);
		dwBufferSize = sizeof(options.szLangPlugin);
		LoadOptionString(key, _T("szLangPlugin"), (LPBYTE)&options.szLangPlugin, dwBufferSize);
		dwBufferSize = sizeof(options.szFavDir);
		LoadOptionString(key, _T("szFavDir"), (LPBYTE)&options.szFavDir, dwBufferSize);
		dwBufferSize = sizeof(options.szArgs);
		LoadOptionString(key, _T("szArgs"), (LPBYTE)&options.szArgs, dwBufferSize);
		dwBufferSize = sizeof(options.szArgs2);
		LoadOptionString(key, _T("szArgs2"), (LPBYTE)&options.szArgs2, dwBufferSize);
		dwBufferSize = sizeof(options.szQuote);
		LoadOptionBinary(key, _T("szQuote"), (LPBYTE)&options.szQuote, dwBufferSize);

		if (key != NULL) {
			dwBufferSize = sizeof(options.MacroArray);
			LoadOptionBinary(key, _T("MacroArray"), (LPBYTE)&options.MacroArray, dwBufferSize);
		}
		else {
			char keyname[14];
			int i;
	
			for (i = 0; i < 10; ++i) {
				wsprintf(keyname, "szMacroArray%d", i);
				LoadBoundedOptionString(key, keyname, (LPBYTE)&options.MacroArray[i], MAXMACRO);
			}
		}
		
		dwBufferSize = sizeof(COLORREF);
		LoadOptionBinary(key, _T("BackColour"), (LPBYTE)&options.BackColour, dwBufferSize);
		LoadOptionBinary(key, _T("FontColour"), (LPBYTE)&options.FontColour, dwBufferSize);
		LoadOptionBinary(key, _T("BackColour2"), (LPBYTE)&options.BackColour2, dwBufferSize);
		LoadOptionBinary(key, _T("FontColour2"), (LPBYTE)&options.FontColour2, dwBufferSize);
		
		dwBufferSize = sizeof(RECT);
		LoadOptionBinary(key, _T("rMargins"), (LPBYTE)&options.rMargins, dwBufferSize);

		if (key != NULL) {
			RegCloseKey(key);
		}
	}

#ifndef USE_RICH_EDIT
	if (options.bSystemColours) {
		options.BackColour = GetSysColor(COLOR_WINDOW);
		options.FontColour = GetSysColor(COLOR_WINDOWTEXT);
	}
	if (options.bSystemColours2) {
		options.BackColour2 = GetSysColor(COLOR_WINDOW);
		options.FontColour2 = GetSysColor(COLOR_WINDOWTEXT);
	}
#endif
}

void LoadOptionString(HKEY hKey, LPCSTR name, BYTE* lpData, DWORD cbData)
{
	if (hKey) {
		RegQueryValueEx(hKey, name, NULL, NULL, lpData, &cbData);
	}
	else {
		GetPrivateProfileString("Options", name, (char*)lpData, (char*)lpData, cbData, szMetapadIni);
	}
}

void LoadBoundedOptionString(HKEY hKey, LPCSTR name, BYTE* lpData, DWORD cbData)
{
	if (hKey) {
		RegQueryValueEx(hKey, name, NULL, NULL, lpData, &cbData);
	}
	else {
		char *bounded = (LPTSTR)GlobalAlloc(GPTR, cbData + 2);
		GetPrivateProfileString("Options", name, bounded, bounded, cbData + 2, szMetapadIni);
		if (lstrlen(bounded) >= 2 && bounded[0] == '[' && bounded[lstrlen(bounded)-1] == ']') {
			strncpy((char*)lpData, bounded+1, lstrlen(bounded) - 2);
		}
		else {
			strncpy((char*)lpData, bounded, lstrlen(bounded));
		}
	}
}

BOOL LoadOptionNumeric(HKEY hKey, LPCSTR name, BYTE* lpData, DWORD cbData)
{
	if (hKey) {
		return (RegQueryValueEx(hKey, name, NULL, NULL, lpData, &cbData) == ERROR_SUCCESS);
	}
	else {
		char val[10];
		if (GetPrivateProfileString("Options", name, NULL, val, 10, szMetapadIni) > 0) {
			long int longInt = atoi(val);
			lpData[3] = (int)((longInt >> 24) & 0xFF) ;
			lpData[2] = (int)((longInt >> 16) & 0xFF) ;
			lpData[1] = (int)((longInt >> 8) & 0XFF);
			lpData[0] = (int)((longInt & 0XFF));
			return TRUE;
		}
		else {
			return FALSE;
		}
	}
}

void LoadOptionBinary(HKEY hKey, LPCSTR name, BYTE* lpData, DWORD cbData)
{
	if (hKey) {
		RegQueryValueEx(hKey, name, NULL, NULL, lpData, &cbData);
	}
	else {
		base64_decodestate state;
		char *szBuffer = (LPTSTR)GlobalAlloc(GPTR, cbData * sizeof(TCHAR) * 2);

		if (GetPrivateProfileString("Options", name, (char*)lpData, szBuffer, cbData * 2, szMetapadIni) > 0) {
			int i;
			for (i = 0; i < lstrlen(szBuffer); ++i) {
				if (szBuffer[i] == '-') {
					szBuffer[i] = '=';
				}
			}
			base64_init_decodestate(&state);
			base64_decode_block(szBuffer, lstrlen(szBuffer), (char*)lpData, &state);
		}
	}
}

BOOL SaveOption(HKEY hKey, LPCSTR name, DWORD dwType, CONST BYTE* lpData, DWORD cbData) 
{
	if (hKey) {
		return (RegSetValueEx(hKey, name, 0, dwType, lpData, cbData) == ERROR_SUCCESS);
	}
	else {
		BOOLEAN succeeded = TRUE;
		switch (dwType) {
			case REG_DWORD: {
				char val[10];
				int int32 = 0;
				int32 = (int32 << 8) + lpData[3];
				int32 = (int32 << 8) + lpData[2];
				int32 = (int32 << 8) + lpData[1];
				int32 = (int32 << 8) + lpData[0];
				wsprintf(val, "%d", int32);
				succeeded = WritePrivateProfileString("Options", name, val, szMetapadIni);
				break;
			}
			case REG_SZ:
				succeeded = WritePrivateProfileString("Options", name, (char*)lpData, szMetapadIni);
				break;
			case REG_BINARY: {
				base64_encodestate state_in;
				INT i;
				char *szBuffer = (LPTSTR)GlobalAlloc(GPTR, 2 * cbData * sizeof(TCHAR));
				ZeroMemory(szBuffer, 2 * cbData * sizeof(TCHAR));
				base64_init_encodestate(&state_in);
				base64_encode_block((char*)lpData, cbData, szBuffer, &state_in);
				for (i = 0; i < lstrlen(szBuffer); ++i) {
					if (szBuffer[i] == '=') {
						szBuffer[i] = '-';
					}
				}
				succeeded = WritePrivateProfileString("Options", name, szBuffer, szMetapadIni);
				break;
			}
		}
		return succeeded;
	}
}


void SaveOptions(void)
{
	HKEY key = NULL;
	BOOL writeSucceeded = TRUE;

	if (!g_bIniMode) {
		RegCreateKeyEx(HKEY_CURRENT_USER, STR_REGKEY, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &key, NULL);
	}

	writeSucceeded &= SaveOption(key, _T("bSystemColours"), REG_DWORD, (LPBYTE)&options.bSystemColours, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bSystemColours2"), REG_DWORD, (LPBYTE)&options.bSystemColours2, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bNoSmartHome"), REG_DWORD, (LPBYTE)&options.bNoSmartHome, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bNoAutoSaveExt"), REG_DWORD, (LPBYTE)&options.bNoAutoSaveExt, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bContextCursor"), REG_DWORD, (LPBYTE)&options.bContextCursor, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bCurrentFindFont"), REG_DWORD, (LPBYTE)&options.bCurrentFindFont, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bPrintWithSecondaryFont"), REG_DWORD, (LPBYTE)&options.bPrintWithSecondaryFont, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bNoSaveHistory"), REG_DWORD, (LPBYTE)&options.bNoSaveHistory, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bNoFindAutoSelect"), REG_DWORD, (LPBYTE)&options.bNoFindAutoSelect, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bHideGotoOffset"), REG_DWORD, (LPBYTE)&options.bHideGotoOffset, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bRecentOnOwn"), REG_DWORD, (LPBYTE)&options.bRecentOnOwn, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bDontInsertTime"), REG_DWORD, (LPBYTE)&options.bDontInsertTime, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bNoWarningPrompt"), REG_DWORD, (LPBYTE)&options.bNoWarningPrompt, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bUnFlatToolbar"), REG_DWORD, (LPBYTE)&options.bUnFlatToolbar, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bReadOnlyMenu"), REG_DWORD, (LPBYTE)&options.bReadOnlyMenu, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bStickyWindow"), REG_DWORD, (LPBYTE)&options.bStickyWindow, sizeof(BOOL));
//	writeSucceeded &= SaveOption(key, _T("nStatusFontWidth"), REG_DWORD, (LPBYTE)&options.nStatusFontWidth, sizeof(int));
	writeSucceeded &= SaveOption(key, _T("nSelectionMarginWidth"), REG_DWORD, (LPBYTE)&options.nSelectionMarginWidth, sizeof(int));
	writeSucceeded &= SaveOption(key, _T("nMaxMRU"), REG_DWORD, (LPBYTE)&options.nMaxMRU, sizeof(int));
	writeSucceeded &= SaveOption(key, _T("nFormatIndex"), REG_DWORD, (LPBYTE)&options.nFormatIndex, sizeof(int));
	writeSucceeded &= SaveOption(key, _T("nTransparentPct"), REG_DWORD, (LPBYTE)&options.nTransparentPct, sizeof(int));
	writeSucceeded &= SaveOption(key, _T("bNoCaptionDir"), REG_DWORD, (LPBYTE)&options.bNoCaptionDir, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bAutoIndent"), REG_DWORD, (LPBYTE)&options.bAutoIndent, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bInsertSpaces"), REG_DWORD, (LPBYTE)&options.bInsertSpaces, sizeof(BOOL));
 	writeSucceeded &= SaveOption(key, _T("bFindAutoWrap"), REG_DWORD, (LPBYTE)&options.bFindAutoWrap, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bQuickExit"), REG_DWORD, (LPBYTE)&options.bQuickExit, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bSaveWindowPlacement"), REG_DWORD, (LPBYTE)&options.bSaveWindowPlacement, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bSaveMenuSettings"), REG_DWORD, (LPBYTE)&options.bSaveMenuSettings, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bSaveDirectory"), REG_DWORD, (LPBYTE)&options.bSaveDirectory, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bLaunchClose"), REG_DWORD, (LPBYTE)&options.bLaunchClose, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bNoFaves"), REG_DWORD, (LPBYTE)&options.bNoFaves, sizeof(BOOL));
#ifndef USE_RICH_EDIT
	writeSucceeded &= SaveOption(key, _T("bDefaultPrintFont"), REG_DWORD, (LPBYTE)&options.bDefaultPrintFont, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bAlwaysLaunch"), REG_DWORD, (LPBYTE)&options.bAlwaysLaunch, sizeof(BOOL));
#else
	writeSucceeded &= SaveOption(key, _T("bLinkDoubleClick"), REG_DWORD, (LPBYTE)&options.bLinkDoubleClick, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bHideScrollbars"), REG_DWORD, (LPBYTE)&options.bHideScrollbars, sizeof(BOOL));
	writeSucceeded &= SaveOption(key, _T("bSuppressUndoBufferPrompt"), REG_DWORD, (LPBYTE)&options.bSuppressUndoBufferPrompt, sizeof(BOOL));
#endif
	writeSucceeded &= SaveOption(key, _T("nLaunchSave"), REG_DWORD, (LPBYTE)&options.nLaunchSave, sizeof(int));
	writeSucceeded &= SaveOption(key, _T("nTabStops"), REG_DWORD, (LPBYTE)&options.nTabStops, sizeof(int));
	writeSucceeded &= SaveOption(key, _T("nPrimaryFont"), REG_DWORD, (LPBYTE)&options.nPrimaryFont, sizeof(int));
	writeSucceeded &= SaveOption(key, _T("nSecondaryFont"), REG_DWORD, (LPBYTE)&options.nSecondaryFont, sizeof(int));
#ifdef BUILD_METAPAD_UNICODE
	writeSucceeded &= SaveOption(key, _T("PrimaryFont_U"), REG_BINARY, (LPBYTE)&options.PrimaryFont, sizeof(LOGFONT));
	writeSucceeded &= SaveOption(key, _T("SecondaryFont_U"), REG_BINARY, (LPBYTE)&options.SecondaryFont, sizeof(LOGFONT));
#else
	writeSucceeded &= SaveOption(key, _T("PrimaryFont"), REG_BINARY, (LPBYTE)&options.PrimaryFont, sizeof(LOGFONT));
	writeSucceeded &= SaveOption(key, _T("SecondaryFont"), REG_BINARY, (LPBYTE)&options.SecondaryFont, sizeof(LOGFONT));
#endif
	writeSucceeded &= SaveOption(key, _T("szBrowser"), REG_SZ, (LPBYTE)&options.szBrowser, sizeof(options.szBrowser));
	writeSucceeded &= SaveOption(key, _T("szArgs"), REG_SZ, (LPBYTE)&options.szArgs, sizeof(options.szArgs));
	writeSucceeded &= SaveOption(key, _T("szBrowser2"), REG_SZ, (LPBYTE)&options.szBrowser2, sizeof(options.szBrowser2));
	writeSucceeded &= SaveOption(key, _T("szArgs2"), REG_SZ, (LPBYTE)&options.szArgs2, sizeof(options.szArgs2));
	writeSucceeded &= SaveOption(key, _T("szQuote"), REG_BINARY, (LPBYTE)&options.szQuote, sizeof(options.szQuote));
	writeSucceeded &= SaveOption(key, _T("szLangPlugin"), REG_SZ, (LPBYTE)&options.szLangPlugin, sizeof(options.szLangPlugin));
	writeSucceeded &= SaveOption(key, _T("szFavDir"), REG_SZ, (LPBYTE)&options.szFavDir, sizeof(options.szFavDir));
	if (key) {
		writeSucceeded &= SaveOption(key, _T("MacroArray"), REG_BINARY, (LPBYTE)&options.MacroArray, sizeof(options.MacroArray));
	}
	else {
		char keyname[14];
		int i;
		char bounded[MAXMACRO + 2];
		for (i = 0; i < 10; ++i) {
			wsprintf(keyname, "szMacroArray%d", i);
			wsprintf(bounded, "[%s]", options.MacroArray[i]);
			writeSucceeded &= SaveOption(key, keyname, REG_SZ, (LPBYTE)&bounded, MAXMACRO);
		}
	}
	writeSucceeded &= SaveOption(key, _T("BackColour"), REG_BINARY, (LPBYTE)&options.BackColour, sizeof(COLORREF));
	writeSucceeded &= SaveOption(key, _T("FontColour"), REG_BINARY, (LPBYTE)&options.FontColour, sizeof(COLORREF));
	writeSucceeded &= SaveOption(key, _T("BackColour2"), REG_BINARY, (LPBYTE)&options.BackColour2, sizeof(COLORREF));
	writeSucceeded &= SaveOption(key, _T("FontColour2"), REG_BINARY, (LPBYTE)&options.FontColour2, sizeof(COLORREF));
	writeSucceeded &= SaveOption(key, _T("rMargins"), REG_BINARY, (LPBYTE)&options.rMargins, sizeof(RECT));
	
	if (!writeSucceeded) {
		ReportLastError();
	}
	
	if (key) {
		RegCloseKey(key);
	}
}

void LoadWindowPlacement(int* left, int* top, int* width, int* height, int* nShow)
{
	HKEY key = NULL;
	DWORD dwBufferSize = sizeof(int);
	BOOL bSuccess = TRUE;

	if (!g_bIniMode) {
		if (RegOpenKeyEx(HKEY_CURRENT_USER, STR_REGKEY, 0, KEY_ALL_ACCESS, &key) != ERROR_SUCCESS) {
			bSuccess = FALSE;
		}
	}

	if (bSuccess) {
		bSuccess &= LoadOptionNumeric(key, _T("w_Left"), (LPBYTE)left, dwBufferSize);
		bSuccess &= LoadOptionNumeric(key, _T("w_Top"), (LPBYTE)top, dwBufferSize);
		bSuccess &= LoadOptionNumeric(key, _T("w_Width"), (LPBYTE)width, dwBufferSize);
		bSuccess &= LoadOptionNumeric(key, _T("w_Height"), (LPBYTE)height, dwBufferSize);
		bSuccess &= LoadOptionNumeric(key, _T("w_WindowState"), (LPBYTE)nShow, dwBufferSize);
	}

	if (key != NULL) {
		RegCloseKey(key);
	}

	if (!bSuccess) {
		*left = *top = *width = *height = CW_USEDEFAULT;
		*nShow = SW_SHOWNORMAL;
		options.bStickyWindow = 0;
	}
}

void LoadMenusAndData(void)
{
	HKEY key = NULL;
	BOOL bLoad = TRUE;

	if (!g_bIniMode) {
		if (RegOpenKeyEx(HKEY_CURRENT_USER, STR_REGKEY, 0, KEY_ALL_ACCESS, &key) != ERROR_SUCCESS) {
			bLoad = FALSE;
		}
	}

	if (bLoad) {
		if (options.bSaveMenuSettings) {
			LoadOptionNumeric(key, _T("m_WordWrap"), (LPBYTE)&bWordWrap, sizeof(BOOL));
			LoadOptionNumeric(key, _T("m_PrimaryFont"), (LPBYTE)&bPrimaryFont, sizeof(BOOL));
			LoadOptionNumeric(key, _T("m_SmartSelect"), (LPBYTE)&bSmartSelect, sizeof(BOOL));
#ifdef USE_RICH_EDIT
			LoadOptionNumeric(key, _T("m_Hyperlinks"), (LPBYTE)&bHyperlinks, sizeof(BOOL));
#endif
			LoadOptionNumeric(key, _T("m_ShowStatus"), (LPBYTE)&bShowStatus, sizeof(BOOL));
			LoadOptionNumeric(key, _T("m_ShowToolbar"), (LPBYTE)&bShowToolbar, sizeof(BOOL));
			LoadOptionNumeric(key, _T("m_AlwaysOnTop"), (LPBYTE)&bAlwaysOnTop, sizeof(BOOL));
			LoadOptionNumeric(key, _T("m_Transparent"), (LPBYTE)&bTransparent, sizeof(BOOL));
			LoadOptionNumeric(key, _T("bCloseAfterFind"), (LPBYTE)&bCloseAfterFind, sizeof(BOOL));
			LoadOptionNumeric(key, _T("bNoFindHidden"), (LPBYTE)&bNoFindHidden, sizeof(BOOL));
		}
		LoadOptionString(key, _T("FileFilter"), (LPBYTE)&szCustomFilter, sizeof(szCustomFilter));
		
		if (!options.bNoSaveHistory) {
#ifdef BUILD_METAPAD_UNICODE
			dwBufferSize = sizeof(TCHAR) * NUMFINDS * MAXFIND;
			RegQueryValueEx(key, _T("FindArray_U"), NULL, NULL, (LPBYTE)&FindArray, &dwBufferSize);
			RegQueryValueEx(key, _T("ReplaceArray_U"), NULL, NULL, (LPBYTE)&ReplaceArray, &dwBufferSize);
			ASSERT(FALSE);
#else
			if (key) {
				LoadOptionString(key, _T("FindArray"), (LPBYTE)&FindArray, sizeof(FindArray));
				LoadOptionString(key, _T("ReplaceArray"), (LPBYTE)&ReplaceArray, sizeof(ReplaceArray));
			}
			else {
				char keyname[16];
				int i;
				for (i = 0; i < 10; ++i) {
					wsprintf(keyname, "szFindArray%d", i);
					LoadBoundedOptionString(key, keyname, (LPBYTE)&FindArray[i], MAXFIND);
				}
				for (i = 0; i < 10; ++i) {
					wsprintf(keyname, "szReplaceArray%d", i);
					LoadBoundedOptionString(key, keyname, (LPBYTE)&ReplaceArray[i], MAXFIND);
				}
			}
#endif	
		}

		if (options.bSaveDirectory) {
			LoadOptionString(key, _T("szLastDirectory"), (LPBYTE)&szDir, sizeof(szDir));
		}
	}

	lstrcpy(szReplaceText, ReplaceArray[0]);

	RegCloseKey(key);
}

void SaveWindowPlacement(HWND hWndSave)
{
	HKEY key = NULL;
	BOOL writeSucceeded = TRUE;
	RECT rect;
	int left, top, width, height, max;
	WINDOWPLACEMENT wndpl;

	wndpl.length = sizeof(WINDOWPLACEMENT);
	GetWindowPlacement(hWndSave, &wndpl);

	if (wndpl.showCmd == SW_SHOWNORMAL) {
		GetWindowRect(hWndSave, &rect);
		max = SW_SHOWNORMAL;
	}
	else {
		if (wndpl.showCmd == SW_SHOWMAXIMIZED)
			max = SW_SHOWMAXIMIZED;
		else
			max = SW_SHOWNORMAL;
		rect = wndpl.rcNormalPosition;
	}

	left = rect.left;
	top = rect.top;
	width = rect.right - left;
	height = rect.bottom - top;

	if (!g_bIniMode) {
		if (RegCreateKeyEx(HKEY_CURRENT_USER, STR_REGKEY, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &key, NULL) != ERROR_SUCCESS) {
			ReportLastError();
			return;
		}
	}

	SaveOption(key, _T("w_WindowState"), REG_DWORD, (LPBYTE)&max, sizeof(int));
	SaveOption(key, _T("w_Left"), REG_DWORD, (LPBYTE)&left, sizeof(int));
	SaveOption(key, _T("w_Top"), REG_DWORD, (LPBYTE)&top, sizeof(int));
	SaveOption(key, _T("w_Width"), REG_DWORD, (LPBYTE)&width, sizeof(int));
	SaveOption(key, _T("w_Height"), REG_DWORD, (LPBYTE)&height, sizeof(int));

	if (key != NULL) {
		RegCloseKey(key);
	}
}

void SaveMenusAndData(void)
{
	HKEY key = NULL;

	if (!g_bIniMode) {
		if (RegCreateKeyEx(HKEY_CURRENT_USER, STR_REGKEY, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &key, NULL) != ERROR_SUCCESS) {
			ReportLastError();
			return;
		}
	}
	if (options.bSaveMenuSettings) {
		SaveOption(key, _T("m_WordWrap"), REG_DWORD, (LPBYTE)&bWordWrap, sizeof(BOOL));
		SaveOption(key, _T("m_PrimaryFont"), REG_DWORD, (LPBYTE)&bPrimaryFont, sizeof(BOOL));
		SaveOption(key, _T("m_SmartSelect"), REG_DWORD, (LPBYTE)&bSmartSelect, sizeof(BOOL));
#ifdef USE_RICH_EDIT
		SaveOption(key, _T("m_Hyperlinks"), REG_DWORD, (LPBYTE)&bHyperlinks, sizeof(BOOL));
#endif
		SaveOption(key, _T("m_ShowStatus"), REG_DWORD, (LPBYTE)&bShowStatus, sizeof(BOOL));
		SaveOption(key, _T("m_ShowToolbar"), REG_DWORD, (LPBYTE)&bShowToolbar, sizeof(BOOL));
		SaveOption(key, _T("m_AlwaysOnTop"), REG_DWORD, (LPBYTE)&bAlwaysOnTop, sizeof(BOOL));
		SaveOption(key, _T("m_Transparent"), REG_DWORD, (LPBYTE)&bTransparent, sizeof(BOOL));
		SaveOption(key, _T("bCloseAfterFind"), REG_DWORD, (LPBYTE)&bCloseAfterFind, sizeof(BOOL));
		SaveOption(key, _T("bNoFindHidden"), REG_DWORD, (LPBYTE)&bNoFindHidden, sizeof(BOOL));
	}
	
	if (!options.bNoSaveHistory) {
#ifdef BUILD_METAPAD_UNICODE
		SaveOption(key, _T("FindArray_U"), REG_BINARY, (LPBYTE)&FindArray, sizeof(TCHAR) * NUMFINDS * MAXFIND);
		SaveOption(key, _T("ReplaceArray_U"), REG_BINARY, (LPBYTE)&ReplaceArray, sizeof(TCHAR) * NUMFINDS * MAXFIND);
		ASSERT(0);
#else
		if (key) {
			SaveOption(key, _T("FindArray"), REG_BINARY, (LPBYTE)&FindArray, sizeof(FindArray));
			SaveOption(key, _T("ReplaceArray"), REG_BINARY, (LPBYTE)&ReplaceArray, sizeof(ReplaceArray));
		}
		else {
			char keyname[16];
			char bounded[MAXFIND + 2];
			int i;
			for (i = 0; i < 10; ++i) {
				wsprintf(keyname, "szFindArray%d", i);
				wsprintf(bounded, "[%s]", &FindArray[i]);
				SaveOption(key, keyname, REG_SZ, (LPBYTE)bounded, MAXFIND);
			}
			for (i = 0; i < 10; ++i) {
				wsprintf(keyname, "szReplaceArray%d", i);
				wsprintf(bounded, "[%s]", &ReplaceArray[i]);
				SaveOption(key, keyname, REG_SZ, (LPBYTE)bounded, MAXFIND);
			}
		}
#endif
	}

	if (options.bSaveDirectory) {
		SaveOption(key, _T("szLastDirectory"), REG_SZ, (LPBYTE)szDir, sizeof(TCHAR) * (lstrlen(szDir) + 1));
	}

	if (key != NULL) {
		RegCloseKey(key);
	}
}

void SaveMRUInfo(LPCTSTR szFullPath)
{
	HKEY key = NULL;
	TCHAR szKey[7];
	TCHAR szBuffer[MAXFN];
	TCHAR szTopVal[MAXFN];
	DWORD dwBufferSize;
	UINT i = 1;

	if (options.nMaxMRU == 0)
		return;	

	if (!g_bIniMode && RegCreateKeyEx(HKEY_CURRENT_USER, STR_REGKEY, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &key, NULL) != ERROR_SUCCESS) {
		ReportLastError();
		return;
	}

	wsprintf(szKey, _T("mru_%d"), nMRUTop);
	dwBufferSize = sizeof(szBuffer);
	LoadOptionNumeric(key, szKey, (LPBYTE)(&szBuffer), dwBufferSize);
	
	if (lstrcmp(szFullPath, szBuffer) != 0) {
		if (++nMRUTop > options.nMaxMRU) {
			nMRUTop = 1;
		}

		SaveOption(key, _T("mru_Top"), REG_DWORD, (LPBYTE)&nMRUTop, sizeof(int));
		wsprintf(szKey, _T("mru_%d"), nMRUTop);
		dwBufferSize = sizeof(szTopVal);
		szTopVal[0] = '\0';
		LoadOptionString(key, szKey, (LPBYTE)&szTopVal, dwBufferSize);
		SaveOption(key, szKey, REG_SZ, (LPBYTE)szFullPath, sizeof(TCHAR) * lstrlen(szFullPath) + 1);

		for (i = 1; i <= options.nMaxMRU; ++i) {
			if (i == nMRUTop) continue;

			szBuffer[0] = szKey[0] = '\0';
			dwBufferSize = sizeof(szBuffer);
			wsprintf(szKey, _T("mru_%d"), i);
			LoadOptionString(key, szKey, (LPBYTE)&szBuffer, dwBufferSize);
			if (lstrcmpi(szBuffer, szFullPath) == 0) {
				SaveOption(key, szKey, REG_SZ, (LPBYTE)szTopVal, sizeof(TCHAR) * lstrlen(szTopVal) + 1);
				break;
			}
		}
	}

	if (key != NULL) {
		RegCloseKey(key);
	}

	PopulateMRUList();
}

void PopulateFavourites(void)
{
	TCHAR szBuffer[MAXFAVESIZE];
	TCHAR szName[MAXFN], szMenu[MAXFN];
	INT i, j, cnt, accel;
	MENUITEMINFO mio;
	HMENU hmenu = GetMenu(hwnd);
	HMENU hsub = GetSubMenu(hmenu, FAVEPOS);

	bHasFaves = FALSE;

	while (GetMenuItemCount(hsub) > 4) {
		DeleteMenu(hsub, 4, MF_BYPOSITION);
	}

	mio.cbSize = sizeof(MENUITEMINFO);
	mio.fMask = MIIM_TYPE | MIIM_ID;
	
	if (GetPrivateProfileString(STR_FAV_APPNAME, NULL, NULL, szBuffer, MAXFAVESIZE, szFav)) {
		bHasFaves = TRUE;
		for (i = 0, j = 0, cnt = accel = 1; /*cnt <= MAXFAVES*/; ++j, ++i) {
			szName[j] = szBuffer[i];
			if (szBuffer[i] == '\0') {
				if (lstrcmp(szName, "-") == 0) {
					mio.fType = MFT_SEPARATOR;
					InsertMenuItem(hsub, cnt + 3, TRUE, &mio);
					++cnt;
					j = -1;
				}
				else {
					mio.fType = MFT_STRING;
					wsprintf(szMenu, (accel < 10 ? _T("&%d ") : _T("%d ")), accel++);
					lstrcat(szMenu, szName);
					j = -1;
					mio.dwTypeData = szMenu;
					mio.wID = ID_FAV_RANGE_BASE + cnt;
					InsertMenuItem(hsub, cnt + 3, TRUE, &mio);
					++cnt;
				}
			}
			if (szBuffer[i] == 0 && szBuffer[i+1] == 0) break;
		}
	}
}

void PopulateMRUList(void)
{
	HKEY key = NULL;
	DWORD nPrevSave = 0;
	TCHAR szBuffer[MAXFN+4];
	TCHAR szBuff2[MAXFN];
	HMENU hmenu = GetMenu(hwnd);
	HMENU hsub;
	TCHAR szKey[7];
	MENUITEMINFO mio;

	if (options.bRecentOnOwn)
		hsub = GetSubMenu(hmenu, 1);
	else
		hsub = GetSubMenu(GetSubMenu(hmenu, 0), RECENTPOS);

	if (options.nMaxMRU == 0) {
		if (options.bRecentOnOwn)
			EnableMenuItem(hmenu, 1, MF_BYPOSITION | MF_GRAYED);
		else
			EnableMenuItem(GetSubMenu(hmenu, 0), RECENTPOS, MF_BYPOSITION | MF_GRAYED);
		return;	
	}
	else {
		if (options.bRecentOnOwn)
			EnableMenuItem(hmenu, 1, MF_BYPOSITION | MF_ENABLED);
		else
			EnableMenuItem(GetSubMenu(hmenu, 0), RECENTPOS, MF_BYPOSITION | MF_ENABLED);
	}

	if (!g_bIniMode && RegCreateKeyEx(HKEY_CURRENT_USER, STR_REGKEY, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &key, &nPrevSave) != ERROR_SUCCESS)
		ReportLastError();

	if (g_bIniMode || nPrevSave == REG_OPENED_EXISTING_KEY) {
		UINT i, num = 1, cnt = 0;
		DWORD dwBufferSize = sizeof(int);
		
		while (GetMenuItemCount(hsub)) {
			DeleteMenu(hsub, 0, MF_BYPOSITION);
		}

		LoadOptionNumeric(key, _T("mru_Top"), (LPBYTE)&nMRUTop, dwBufferSize);
		mio.cbSize = sizeof(MENUITEMINFO);
		mio.fMask = MIIM_TYPE | MIIM_ID;
		mio.fType = MFT_STRING;

		i = nMRUTop;
		while (cnt < options.nMaxMRU) {
			szBuff2[0] = _T('\0');
			wsprintf(szKey, _T("mru_%d"), i);
			wsprintf(szBuffer, (num < 10 ? _T("&%d ") : _T("%d ")), num);

			dwBufferSize = sizeof(szBuff2);
			LoadOptionString(key, szKey, (LPBYTE)(&szBuff2), dwBufferSize);

			if (lstrlen(szBuff2) > 0) {
				lstrcat(szBuffer, szBuff2);
				mio.dwTypeData = szBuffer;
				mio.wID = ID_MRU_BASE + i;
				InsertMenuItem(hsub, num, TRUE, &mio);
				num++;
			}
			if (i < 2)
				i = options.nMaxMRU;
			else
				i--;
			cnt++;
		}
	}
	if (key != NULL) {
		RegCloseKey(key);
	}
}

void CenterWindow(HWND hwndCenter)
{
	RECT r1, r2;
	GetWindowRect(GetParent(hwndCenter), &r1);
	GetWindowRect(hwndCenter, &r2);
	SetWindowPos(hwndCenter, HWND_TOP, (((r1.right - r1.left) - (r2.right - r2.left)) / 2) + r1.left, (((r1.bottom - r1.top) - (r2.bottom - r2.top)) / 2) + r1.top, 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);
}

void SelectWord(BOOL bFinding, BOOL bSmart, BOOL bAutoSelect)
{
	LONG lLine, lLineIndex, lLineLen;
	LPTSTR szBuffer;
	CHARRANGE cr;

#ifdef USE_RICH_EDIT
	SendMessage(client, EM_EXGETSEL, 0, (LPARAM)&cr);
	lLine = SendMessage(client, EM_EXLINEFROMCHAR, 0, (LPARAM)cr.cpMin);
	if (lLine == SendMessage(client, EM_EXLINEFROMCHAR, 0, (LPARAM)cr.cpMax)) {
#else
	SendMessage(client, EM_GETSEL, (WPARAM)&cr.cpMin, (LPARAM)&cr.cpMax);
	lLine = SendMessage(client, EM_LINEFROMCHAR, (WPARAM)cr.cpMin, 0);
	if (lLine == SendMessage(client, EM_LINEFROMCHAR, (WPARAM)cr.cpMax, 0)) {
#endif
		lLineLen = SendMessage(client, EM_LINELENGTH, (WPARAM)cr.cpMin, 0);
		szBuffer = (LPTSTR) GlobalAlloc(GPTR, (lLineLen+2) * sizeof(TCHAR));
		*((LPWORD)szBuffer) = (USHORT)(lLineLen + 1);
		SendMessage(client, EM_GETLINE, (WPARAM)lLine, (LPARAM) (LPCTSTR)szBuffer);
		szBuffer[lLineLen] = '\0';
		lLineIndex = SendMessage(client, EM_LINEINDEX, (WPARAM)lLine, 0);
		cr.cpMin -= lLineIndex;
		cr.cpMax -= lLineIndex;
		if (cr.cpMin == cr.cpMax && bAutoSelect) {
			if (bSmart) {
				if (_istprint(szBuffer[cr.cpMax]) && !_istspace(szBuffer[cr.cpMax])) {
					cr.cpMax++;
				}
				else {
					while (cr.cpMin && (_istalnum(szBuffer[cr.cpMin-1]) || szBuffer[cr.cpMin-1] == '_'))
						cr.cpMin--;
				}
				if (_istalnum(szBuffer[cr.cpMin]) || szBuffer[cr.cpMin] == '_') {
					while (cr.cpMin && (_istalnum(szBuffer[cr.cpMin-1]) || szBuffer[cr.cpMin-1] == '_'))
						cr.cpMin--;
					while (cr.cpMax < lLineLen && (_istalnum(szBuffer[cr.cpMax]) || szBuffer[cr.cpMax] == '_'))
						cr.cpMax++;
				}
			}
			else {
				if (_istprint(szBuffer[cr.cpMax])) {
					while (cr.cpMin && (!_istspace(szBuffer[cr.cpMin-1])))
						cr.cpMin--;
					while (cr.cpMax < lLineLen && (!_istspace(szBuffer[cr.cpMax])))
						cr.cpMax++;
				}
				else {
					while (cr.cpMin && (_istspace(szBuffer[cr.cpMin-1])))
						cr.cpMin--;
					while (cr.cpMin && (!_istspace(szBuffer[cr.cpMin-1])))
						cr.cpMin--;
				}
				while (cr.cpMax < lLineLen && _istspace(szBuffer[cr.cpMax]) && szBuffer[cr.cpMax])
					cr.cpMax++;
			}
		}
		if (bAutoSelect || cr.cpMin != cr.cpMax) {
			if (bFinding) {
				if (cr.cpMax - cr.cpMin > MAXFIND) {
					lstrcpyn(szFindText, szBuffer + cr.cpMin, MAXFIND);
					szFindText[MAXFIND - 1] = '\0';
				}
				else {
					lstrcpyn(szFindText, szBuffer + cr.cpMin, cr.cpMax - cr.cpMin + 1);
					szFindText[cr.cpMax - cr.cpMin] = '\0';
				}
			}
			cr.cpMin += lLineIndex;
			cr.cpMax += lLineIndex;
#ifdef USE_RICH_EDIT
			SendMessage(client, EM_EXSETSEL, 0, (LPARAM)&cr);
#else
			SendMessage(client, EM_SETSEL, (WPARAM)cr.cpMin, (LPARAM)cr.cpMax);
#endif
		}
		GlobalFree((HGLOBAL)szBuffer);
		UpdateStatus();
	}
}

#ifdef STREAMING
DWORD CALLBACK EditStreamIn(DWORD dwCookie, LPBYTE pbBuff, LONG cb, LONG *pcb)
{
	LPTSTR szBuffer = (LPTSTR)dwCookie;
	LONG lBufferLength = lstrlen(szBuffer);
	static LONG nBytesDone = 0;

	wsprintf(szStatusMessage, "Loading file... %d", nBytesDone);
	UpdateStatus();

	if (*pcb == 1) nBytesDone = 0;

	if (nBytesDone == lBufferLength) {
		*pcb = 0;
		return 0;
	}

	if (cb > lBufferLength - nBytesDone) {
		cb = lBufferLength - nBytesDone;
	}

	memcpy(pbBuff, szBuffer + nBytesDone, cb * sizeof(TCHAR));
	nBytesDone += cb;
	*pcb = cb;
	return 0;
}
#endif

#ifdef USE_RICH_EDIT
int FixTextBuffer(LPTSTR szText)
{
	int cnt = 0;
	int i = 0, j = 0;
	LPTSTR szNew = (LPTSTR)GlobalAlloc(GPTR, (lstrlen(szText) + 1) * sizeof(TCHAR));

	if (lstrlen(szText) < 3) {
		if (szText[i]) szNew[j++] = szText[i++];
		if (szText[i]) szNew[j++] = szText[i++];
	}

	while (szText[i] && szText[i+1] && szText[i+2]) {
		if (szText[i] == '\r' && szText[i+1] == '\r' && szText[i+2] == '\n') {
			++cnt;
		}
		else {
			szNew[j++] = szText[i];
		}
		++i;
	}

	if (szText[i]) szNew[j++] = szText[i++];
	if (szText[i]) szNew[j++] = szText[i++];

	if (cnt) lstrcpy(szText, szNew);

	GlobalFree((HGLOBAL)szNew);

	if (cnt) {
		if (!options.bNoWarningPrompt) {
			ERROROUT(GetString(IDS_CARRIAGE_RETURN_WARNING));
		}
		SetFocus(client);
	}

	return cnt;
}
#else
void FixTextBufferLE(LPTSTR* pszBuffer)
{
	UINT i = 0, cnt = 0, j;
	LPTSTR szNewBuffer = NULL;

	if ((*pszBuffer)[0] == '\n') {
		++cnt;
	}
	while ((*pszBuffer)[i] && (*pszBuffer)[i+1]) {
		if ((*pszBuffer)[i] != '\r' && (*pszBuffer)[i+1] == '\n')
			++cnt;
		else if ((*pszBuffer)[i] == '\r' && (*pszBuffer)[i+1] != '\n')
			++cnt;
		++i;
	}

	szNewBuffer = (LPTSTR)GlobalAlloc(GPTR, (lstrlen(*pszBuffer)+cnt+2) * sizeof(TCHAR));
	ZeroMemory(szNewBuffer, (lstrlen(*pszBuffer)+cnt+2) * sizeof(TCHAR));
	i = j = 0;
	if ((*pszBuffer)[0] == '\n') {
		szNewBuffer[0] = '\r';
		++j;
	}
	while ((*pszBuffer)[i] != '\0') {
		if ((*pszBuffer)[i] != '\r' && (*pszBuffer)[i+1] == '\n') {
			szNewBuffer[j++] = (*pszBuffer)[i];
			szNewBuffer[j] = '\r';
		}
		else if ((*pszBuffer)[i] == '\r' && (*pszBuffer)[i+1] != '\n') {
			szNewBuffer[j++] = (*pszBuffer)[i];
			szNewBuffer[j] = '\n';
		}
		else {
			szNewBuffer[j] = (*pszBuffer)[i];
		}
		j++; i++;
	}
	szNewBuffer[j] = '\0';
	GlobalFree((HGLOBAL) *pszBuffer);
	*pszBuffer = szNewBuffer;
}
#endif

int ConvertAndSetWindowText(LPTSTR szText)
{	
	UINT i = 0, cnt = 0, mcnt = 0, j;
	LPTSTR szBuffer = NULL;

	if (szText[0] == '\n') {
		++cnt;
	}
	while (szText[i] && szText[i+1]) {
		if (szText[i] != '\r' && szText[i+1] == '\n')
			++cnt;
		else if (szText[i] == '\r' && szText[i+1] != '\n')
			++mcnt;
		++i;
	}

	/*
	if (mcnt && cnt) {
		ERROROUT("Malformed text file detected!");
	}
	*/

	if (mcnt) {
		if (!options.bNoWarningPrompt)
			ERROROUT(GetString(IDS_MAC_FILE_WARNING));

		if (szText[i] == '\r') ++mcnt;

		for (i = 0; szText[i] && szText[i+1]; ++i) {
			if (szText[i] == '\r' && szText[i+1] != '\n')
				szText[i] = '\n';
		}
		
		if (szText[i] == '\r') szText[i] = '\n';

		cnt += mcnt;
	}

	bUnix = FALSE;
	if (cnt) {
		if (nEncodingType == TYPE_UTF_8) {
			bUnix = FALSE;
		}
		else {
			bUnix = TRUE;
		}
		szBuffer = (LPTSTR)GlobalAlloc(GPTR, (lstrlen(szText)+cnt+2) * sizeof(TCHAR));
		ZeroMemory(szBuffer, (lstrlen(szText)+cnt+2) * sizeof(TCHAR));
		i = j = 0;
		if (szText[0] == '\n') {
			szBuffer[0] = '\r';
			++j;
		}
		while (szText[i] != '\0') {
			if (szText[i] != '\r' && szText[i+1] == '\n') {
				szBuffer[j++] = szText[i];
				szBuffer[j] = '\r';
			}
			else {
				szBuffer[j] = szText[i];
			}
			j++; i++;
		}
		szBuffer[j] = '\0';
#ifdef STREAMING
		{
		EDITSTREAM es;
		es.dwCookie = (DWORD)szBuffer;
		es.dwError = 0;
		es.pfnCallback = EditStreamIn;
		SendMessage(client, EM_STREAMIN, (WPARAM)SF_TEXT, (LPARAM)&es);
		}
#else
		SetWindowText(client, szBuffer);
#endif	
		GlobalFree((HGLOBAL) szBuffer);
	}
#ifdef STREAMING
	else {
		EDITSTREAM es;
		es.dwCookie = (DWORD)szText;
		es.dwError = 0;
		es.pfnCallback = EditStreamIn;
		SendMessage(client, EM_STREAMIN, (WPARAM)SF_TEXT, (LPARAM)&es);
	}
#else
	else {
		SetWindowText(client, szText);
/*
typedef struct _settextex {
	DWORD flags; 
	UINT codepage; 
} SETTEXTEX;

SETTEXTEX ste;
CHARSETINFO csi;
ste.flags = ST_SELECTION;
TranslateCharsetInfo((DWORD FAR*) GREEK_CHARSET,&csi,TCI_SRCCHARSET );
ste.codepage = csi.ciACP;
SendMessage(client, WM_USER+97, (WPARAM) &ste, (LPARAM)szText);
*/
	}
#endif

	return cnt;
}

DWORD LoadFileIntoBuffer(HANDLE hFile, PBYTE* ppBuffer, ULONG* plBufferLength, INT* pnFileEncoding)
{
	DWORD dwBytesRead = 0;
	BOOL bResult;

	*plBufferLength = GetFileSize(hFile, NULL);

	*ppBuffer = (PBYTE) GlobalAlloc(GPTR, (*plBufferLength+2) * sizeof(TCHAR));
	if (*ppBuffer == NULL)
		ReportLastError();

	ZeroMemory(*ppBuffer, (*plBufferLength+2) * sizeof(TCHAR));

	*pnFileEncoding = TYPE_UNKNOWN;

	// check for bom
	if (*plBufferLength >= SIZEOFBOM_UTF_8) {

		bResult = ReadFile(hFile, *ppBuffer, SIZEOFBOM_UTF_8, &dwBytesRead, NULL);
		if (!bResult || dwBytesRead != (DWORD)SIZEOFBOM_UTF_8)	
			ReportLastError();

		if (IsBOM(*ppBuffer, TYPE_UTF_8)) {
			*pnFileEncoding = TYPE_UTF_8;
			*plBufferLength -= SIZEOFBOM_UTF_8;
		}
		else {
			SetFilePointer(hFile, -SIZEOFBOM_UTF_8, NULL, FILE_CURRENT);
		}
	}
	if (*pnFileEncoding == TYPE_UNKNOWN && *plBufferLength >= SIZEOFBOM_UTF_16) {

		bResult = ReadFile(hFile, *ppBuffer, SIZEOFBOM_UTF_16, &dwBytesRead, NULL);
		if (!bResult || dwBytesRead != (DWORD)SIZEOFBOM_UTF_16)
			ReportLastError();

		if (IsBOM(*ppBuffer, TYPE_UTF_16)) {
			*pnFileEncoding = TYPE_UTF_16;
			*plBufferLength -= SIZEOFBOM_UTF_16;
		}
		else if (IsBOM(*ppBuffer, TYPE_UTF_16_BE)) {
			*pnFileEncoding = TYPE_UTF_16_BE;
			*plBufferLength -= SIZEOFBOM_UTF_16;
		}
		else {
			SetFilePointer(hFile, -SIZEOFBOM_UTF_16, NULL, FILE_CURRENT);
		}
	}

	bResult = ReadFile(hFile, *ppBuffer, *plBufferLength, &dwBytesRead, NULL);
	if (!bResult || dwBytesRead != (DWORD)*plBufferLength)	
		ReportLastError();

	// check if unicode even if no bom
	if (*pnFileEncoding == TYPE_UNKNOWN) {
		if (dwBytesRead > 2 && IsTextUnicode(*ppBuffer, *plBufferLength, NULL)) {
			*pnFileEncoding = TYPE_UTF_16;

			// add unicode null - already zeroed
			/*
			(*ppBuffer)[*plBufferLength] = 0;
			(*ppBuffer)[*plBufferLength+1] = 0;
			*/
		}
	}

	if (*pnFileEncoding == TYPE_UTF_16 || *pnFileEncoding == TYPE_UTF_16_BE && *plBufferLength) {
#ifndef BUILD_METAPAD_UNICODE
		long nBytesNeeded;
		BOOL bUsedDefault;
		PBYTE pNewBuffer = NULL;
#endif
		if (*pnFileEncoding == TYPE_UTF_16_BE) {
			ReverseBytes(*ppBuffer, *plBufferLength);
		}
		*plBufferLength = *plBufferLength / 2;

#ifndef BUILD_METAPAD_UNICODE
		nBytesNeeded = WideCharToMultiByte(CP_ACP, 0, (LPCWSTR)*ppBuffer, 
			*plBufferLength, NULL, 0, NULL, NULL);

		pNewBuffer = (PBYTE) GlobalAlloc(GPTR, nBytesNeeded+1);
		if (pNewBuffer == NULL)
			ReportLastError();

		if (!WideCharToMultiByte(CP_ACP, 0,	(LPCWSTR)*ppBuffer, *plBufferLength,
			(LPSTR)pNewBuffer, nBytesNeeded, NULL, &bUsedDefault)) {
			ReportLastError();
			ERROROUT(GetString(IDS_UNICODE_CONVERT_ERROR));
			dwBytesRead = 0;
		}

		if (bUsedDefault) {
			ERROROUT(GetString(IDS_UNICODE_CHARS_WARNING));
		}

		GlobalFree((HGLOBAL)*ppBuffer);
		*ppBuffer = pNewBuffer;
#endif				
	}

	return dwBytesRead;
}

void LoadFile(LPTSTR szFilename, BOOL bCreate, BOOL bMRU)
{
	HANDLE hFile = NULL;
	ULONG lBufferLength;
	PBYTE pBuffer = NULL;
	DWORD dwActualBytesRead;
	HCURSOR hcur;
	int i;

	bHideMessage = FALSE;
	lstrcpy(szStatusMessage, GetString(IDS_FILE_LOADING));
	UpdateStatus();

	hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

	for (i = 0; i < 2; ++i) {
		hFile = (HANDLE)CreateFile(szFilename, GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		if (hFile == INVALID_HANDLE_VALUE) {
			DWORD dwError = GetLastError();
			if (dwError == ERROR_FILE_NOT_FOUND && bCreate) {
				TCHAR buffer[MAXFN + 40];

				if (i == 0) {
					if (_tcschr(szFilename, _T('.')) == NULL) {
						lstrcat(szFilename, _T(".txt"));
						continue;
					}
				}

				wsprintf(buffer, GetString(IDS_CREATE_FILE_MESSAGE), szFilename);
				switch (MessageBox(hwnd, buffer, STR_METAPAD, MB_YESNOCANCEL | MB_ICONEXCLAMATION)) {
				case IDYES:
					{
						hFile = (HANDLE)CreateFile(szFilename, GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
						if (hFile == INVALID_HANDLE_VALUE) {
							ERROROUT(GetString(IDS_FILE_CREATE_ERROR));
							bHideMessage = TRUE;
							UpdateStatus();
							bLoading = FALSE;
							SetCursor(hcur);
							return;
						}
						break;
					}
				case IDNO:
					{
						bHideMessage = TRUE;
						MakeNewFile();
						SetCursor(hcur);
						return;
					}
				case IDCANCEL:
					{
						if (bLoading)
							PostQuitMessage(0);
						bHideMessage = TRUE;
						SetCursor(hcur);
						return;
					}
				}
				break;
			}
			else {
				if (dwError == ERROR_FILE_NOT_FOUND) {
					ERROROUT(GetString(IDS_FILE_NOT_FOUND));
				}
				else if (dwError == ERROR_SHARING_VIOLATION) {
					ERROROUT(GetString(IDS_FILE_LOCKED_ERROR));
				}
				else {
					SetLastError(dwError);
					ReportLastError();
				}
				bHideMessage = TRUE;
				UpdateStatus();
				bLoading = FALSE;
				SetCursor(hcur);
				return;
			}
		}
		else {
			SwitchReadOnly(GetFileAttributes(szFilename) & FILE_ATTRIBUTE_READONLY);
			break;
		}
	}

	if (bMRU)
		SaveMRUInfo(szFilename);


	dwActualBytesRead = LoadFileIntoBuffer(hFile, &pBuffer, &lBufferLength, &nEncodingType);

//	if (dwActualBytesRead < 0) goto fini;

	if (nEncodingType == TYPE_UTF_16 || nEncodingType == TYPE_UTF_16_BE) {
		if (lBufferLength) {
			SendMessage(client, WM_SETREDRAW, (WPARAM)FALSE, 0);
			SetWindowText(client, (LPCTSTR)pBuffer);
			SendMessage(client, WM_SETREDRAW, (WPARAM)TRUE, 0);
		}
		else {
			SetWindowText(client, _T(""));
		}

		bDirtyFile = FALSE;
		bUnix = FALSE;

		switch (nEncodingType) {
		case TYPE_UTF_16:
			SetFileFormat(FILE_FORMAT_UNICODE);
			break;
		case TYPE_UTF_16_BE:
			SetFileFormat(FILE_FORMAT_UNICODE_BE);
			break;
		}
	}
	else if (nEncodingType != TYPE_UTF_16 && nEncodingType != TYPE_UTF_16_BE) {

#ifdef USE_RICH_EDIT
		dwActualBytesRead -= FixTextBuffer((LPTSTR)pBuffer);
#endif

		SendMessage(client, WM_SETREDRAW, (WPARAM)FALSE, 0);
		dwActualBytesRead += ConvertAndSetWindowText((LPTSTR)pBuffer);
		SendMessage(client, WM_SETREDRAW, (WPARAM)TRUE, 0);

		if (lBufferLength == 0) {
			SetFileFormat(options.nFormatIndex);
		}
		else {
			if (nEncodingType == TYPE_UTF_8)
				SetFileFormat(FILE_FORMAT_UTF_8);
			else
				SetFileFormat(bUnix ? FILE_FORMAT_UNIX : FILE_FORMAT_DOS);
		}

		bDirtyFile = FALSE;
		bBinaryFile = FALSE;

		if (dwActualBytesRead != (DWORD)GetWindowTextLength(client) && bLoading) {
			if (options.bNoWarningPrompt || MessageBox(hwnd, GetString(IDS_BINARY_FILE_WARNING), STR_METAPAD, MB_ICONQUESTION|MB_YESNO) == IDYES) {
				UINT i;
				for (i = 0; i < lBufferLength; i++) {
					if (pBuffer[i] == '\0')
						pBuffer[i] = ' ';
				}
				SendMessage(client, WM_SETREDRAW, (WPARAM)FALSE, 0);
#ifdef STREAMING
				{
					EDITSTREAM es;
					es.dwCookie = (DWORD)pBuffer;
					es.dwError = 0;
					es.pfnCallback = EditStreamIn;
					SendMessage(client, EM_STREAMIN, (WPARAM)SF_TEXT, (LPARAM)&es);
				}
#else
				SetWindowText(client, (LPTSTR)pBuffer);
#endif
				SendMessage(client, WM_SETREDRAW, (WPARAM)TRUE, 0);
				bBinaryFile = TRUE;
			}
			else {
				MakeNewFile();
			}
		}
	}

	SendMessage(client, EM_SETMODIFY, (WPARAM)TRUE, 0);

	SetTabStops();
	SendMessage(client, EM_EMPTYUNDOBUFFER, 0, 0);

	if (pBuffer[0] == '.' && 
		pBuffer[1] == 'L' &&
		pBuffer[2] == 'O' &&
		pBuffer[3] == 'G') {
		CHARRANGE cr;
		cr.cpMin = cr.cpMax = lBufferLength;

#ifdef USE_RICH_EDIT
		SendMessage(client, EM_EXSETSEL, 0, (LPARAM)&cr);
#else
		SendMessage(client, EM_SETSEL, (WPARAM)cr.cpMin, (LPARAM)cr.cpMax);
#endif
		SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_DATE_TIME, 0), 0);
		SendMessage(client, EM_SCROLLCARET, 0, 0);
	}

//#ifndef BUILD_METAPAD_UNICODE
//fini:
//#endif
	bHideMessage = TRUE;
	UpdateStatus();
	CloseHandle(hFile);
	GlobalFree((HGLOBAL) pBuffer);
	InvalidateRect(client, NULL, TRUE);
	SetCursor(hcur);
}

#ifndef USE_RICH_EDIT
void ConvertToUnix(LPTSTR szBuffer)
{
	UINT i = 0, j = 0;
	LPTSTR szTemp = (LPTSTR) GlobalAlloc(GPTR, (lstrlen(szBuffer)+1) * sizeof(TCHAR));

	lstrcpy(szTemp, szBuffer);

	while (szTemp[i] != '\0') {
		if (szTemp[i] != '\r') {
			szBuffer[j] = szTemp[i];
			j++;
		}
		i++;
	}
	szBuffer[j] = '\0';
	GlobalFree((HGLOBAL)szTemp);
}
#else
void RichModeToDos(LPTSTR *szBuffer)
{
	int cnt = 0;
	int i = 0, j;

	while ((*szBuffer)[i] != '\0') {
		if ((*szBuffer)[i] == '\r')
			cnt++;
		i++;
	}

	if (cnt) {
		LPTSTR szNewBuffer = (LPTSTR)GlobalAlloc(GPTR, (lstrlen(*szBuffer)+cnt+2) * sizeof(TCHAR));
		ZeroMemory(szNewBuffer, (lstrlen(*szBuffer)+cnt+2) * sizeof(TCHAR));
		i = j = 0;
		while ((*szBuffer)[i] != '\0') {
			if ((*szBuffer)[i] == '\r') {
				szNewBuffer[j++] = (*szBuffer)[i];
				szNewBuffer[j] = '\n';
			}
			else {
				szNewBuffer[j] = (*szBuffer)[i];
			}
			j++; i++;
		}
		szNewBuffer[j] = '\0';
	
		GlobalFree((HGLOBAL) *szBuffer);
		*szBuffer = szNewBuffer;
	}
}
#endif

BOOL SaveFile(LPCTSTR szFilename)
{
	HANDLE hFile;
	LONG lFileSize;
	DWORD dwActualBytesWritten = 0;
	LPTSTR szBuffer;
	HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));
	LPTSTR pNewBuffer = NULL;

	lFileSize = GetWindowTextLength(client) * sizeof(TCHAR);
	szBuffer = (LPTSTR) GlobalAlloc(GPTR, lFileSize + (1 * sizeof(TCHAR)));

	hFile = (HANDLE)CreateFile(szFilename, GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (hFile == INVALID_HANDLE_VALUE) {
		ReportLastError();
		return FALSE;
	}

#ifdef USE_RICH_EDIT
		{
			TEXTRANGE tr;
			
			tr.chrg.cpMin = 0;
			tr.chrg.cpMax = -1;
			tr.lpstrText = szBuffer;

			SendMessage(client, EM_GETTEXTRANGE, 0, (LPARAM)&tr);
		}
#else
		GetWindowText(client, szBuffer, lFileSize+1);
#endif

	if (nEncodingType == TYPE_UTF_16 || nEncodingType == TYPE_UTF_16_BE) {

		long nBytesNeeded = 0;

		if (lFileSize) {

#ifdef USE_RICH_EDIT
			RichModeToDos(&szBuffer);
#endif

#ifdef BUILD_METAPAD_UNICODE
			pNewBuffer = szBuffer;
#else
			nBytesNeeded = 2 * (1+MultiByteToWideChar(CP_ACP, 0, (LPCSTR)szBuffer, 
				lFileSize, NULL, 0));

			pNewBuffer = (LPTSTR) GlobalAlloc(GPTR, nBytesNeeded+1);
			if (pNewBuffer == NULL)
				ReportLastError();

			if (!MultiByteToWideChar(CP_ACP, 0,	(LPCSTR)szBuffer, lFileSize,
				(LPWSTR)pNewBuffer, nBytesNeeded)) {
				ReportLastError();
				ERROROUT(GetString(IDS_UNICODE_STRING_ERROR));
			}
#endif
		}

		if (nEncodingType == TYPE_UTF_16_BE) {
			ReverseBytes((PBYTE)pNewBuffer, nBytesNeeded);
			WriteFile(hFile, szBOM_UTF_16_BE, SIZEOFBOM_UTF_16, &dwActualBytesWritten, NULL);
		}
		else if (nEncodingType == TYPE_UTF_16) {
			WriteFile(hFile, szBOM_UTF_16, SIZEOFBOM_UTF_16, &dwActualBytesWritten, NULL);
		}

		if (dwActualBytesWritten != SIZEOFBOM_UTF_16) {
			ERROROUT(GetString(IDS_UNICODE_BOM_ERROR));
		}

		if (lFileSize) {
			if (!WriteFile(hFile, pNewBuffer, nBytesNeeded-2, &dwActualBytesWritten, NULL)) {
				ReportLastError();
			}
			lFileSize = nBytesNeeded-2;
#ifndef BUILD_METAPAD_UNICODE
			GlobalFree((HGLOBAL)pNewBuffer);
#endif
		}
		else {
			dwActualBytesWritten = 0;
		}
	}
	else {

#ifdef USE_RICH_EDIT
		if (bUnix) {
			int i = 0;
			while (szBuffer[i] != '\0') {
				if (szBuffer[i] == '\r')
					szBuffer[i] = '\n';
				i++;
			}
			lFileSize = lstrlen(szBuffer);
		}
		else {
			RichModeToDos(&szBuffer);
		}
#else
		if (bUnix) {
			ConvertToUnix(szBuffer);
			lFileSize = lstrlen(szBuffer);
		}
#endif
		if (nEncodingType == TYPE_UTF_8) {
			WriteFile(hFile, szBOM_UTF_8, SIZEOFBOM_UTF_8, &dwActualBytesWritten, NULL);

			if (dwActualBytesWritten != SIZEOFBOM_UTF_8) {
				ERROROUT(GetString(IDS_UNICODE_BOM_ERROR));
			}
		}

		if (!WriteFile(hFile, szBuffer, lFileSize, &dwActualBytesWritten, NULL)) {
			ReportLastError();
		}
	}

	SetEndOfFile(hFile);
	CloseHandle(hFile);

	SetCursor(hcur);
	GlobalFree((HGLOBAL)szBuffer);
	if (dwActualBytesWritten != (DWORD)lFileSize) {
		ERROROUT(GetString(IDS_ERROR_LOCKED));
		return FALSE;
	}
	else
		return TRUE;
}

void SetFont(HFONT* phfnt, BOOL bPrimary)
{
	LOGFONT logfind;

	if (*phfnt)
		DeleteObject(*phfnt);

	if (hfontfind)
		DeleteObject(hfontfind);

	if (bPrimary) {
		if (options.nPrimaryFont == 0) {
			*phfnt = GetStockObject(SYSTEM_FIXED_FONT);
			hfontfind = *phfnt;
			return;
		}
		else {
			*phfnt = CreateFontIndirect(&options.PrimaryFont);
			CopyMemory((PVOID)&logfind, (CONST VOID*)&options.PrimaryFont, sizeof(LOGFONT));
		}
	}
	else {
		if (options.nSecondaryFont == 0) {
			*phfnt = GetStockObject(ANSI_FIXED_FONT);
			hfontfind = *phfnt;
			return;
		}
		else {
			*phfnt = CreateFontIndirect(&options.SecondaryFont);
			CopyMemory((PVOID)&logfind, (CONST VOID*)&options.SecondaryFont, sizeof(LOGFONT));
		}
	}

	{
		HDC clientdc = GetDC(client);
		logfind.lfHeight = -MulDiv(LOWORD(GetDialogBaseUnits())+2, GetDeviceCaps(clientdc, LOGPIXELSY), 72);
		hfontfind = CreateFontIndirect(&logfind);
		ReleaseDC(client, clientdc);
	}
}

BOOL SetClientFont(BOOL bPrimary)
{
#ifdef USE_RICH_EDIT
	HDC clientDC;
	CHARFORMAT cf;
	TCHAR szFace[MAXFONT];
	TEXTMETRIC tm;
	CHARRANGE cr;
	BOOL bUseSystem;

	bUseSystem = bPrimary ? options.bSystemColours : options.bSystemColours2;
	//if (!bPrimary && options.bSecondarySystemColour) bUseSystem = TRUE;

	if (SendMessage(client, EM_CANUNDO, 0, 0)) {
		if (!options.bSuppressUndoBufferPrompt && MessageBox(hwnd, GetString(IDS_FONT_UNDO_WARNING), STR_METAPAD, MB_ICONEXCLAMATION | MB_OKCANCEL) == IDCANCEL) {
			return FALSE;
		}
	}

	SendMessage(client, EM_EXGETSEL, 0, (LPARAM)&cr);

	SendMessage(client, WM_SETREDRAW, (WPARAM)FALSE, 0);

	SetFont(&hfontmain, bPrimary);

	cf.cbSize = sizeof (cf);
	cf.dwMask = CFM_COLOR | CFM_SIZE | CFM_FACE | CFM_BOLD | CFM_CHARSET | CFM_ITALIC;

	SendMessage(client, EM_GETCHARFORMAT, TRUE, (LPARAM)&cf);

	clientDC = GetDC(client);
	SelectObject (clientDC, hfontmain);

	if (!GetTextFace(clientDC, MAXFONT, szFace))
		ReportLastError();

	lstrcpy(cf.szFaceName, szFace);

	if (bPrimary) {
		cf.bCharSet = options.PrimaryFont.lfCharSet;
		cf.bPitchAndFamily = options.PrimaryFont.lfPitchAndFamily;
	}
	else {
		cf.bCharSet = options.SecondaryFont.lfCharSet;
		cf.bPitchAndFamily = options.SecondaryFont.lfPitchAndFamily;
	}

	if (!GetTextMetrics(clientDC, &tm))
		ReportLastError();

	cf.yHeight = 15 * (tm.tmHeight - tm.tmInternalLeading);

	cf.dwEffects = 0;
	if (tm.tmWeight == FW_BOLD) {
		cf.dwEffects |= CFE_BOLD;
	}
	if (tm.tmItalic) {
		cf.dwEffects |= CFE_ITALIC;
	}
	if (bUseSystem)
		cf.dwEffects |= CFE_AUTOCOLOR;
	else
		cf.crTextColor = bPrimary ? options.FontColour : options.FontColour2;

	bLoading = TRUE;
	SendMessage(client, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf);
	bLoading = FALSE;
	ReleaseDC(client, clientDC);

	UpdateWindowText();

	SendMessage(client, EM_SETBKGNDCOLOR, (WPARAM)bUseSystem, (LPARAM)(bPrimary ? options.BackColour : options.BackColour2));
	SendMessage(client, EM_EXSETSEL, 0, (LPARAM)&cr);
	SendMessage(client, WM_SETREDRAW, (WPARAM)TRUE, 0);

	InvalidateRect(hwnd, NULL, TRUE);

	return TRUE;
#else
	SetFont(&hfontmain, bPrimary);
	SendMessage(client, WM_SETFONT, (WPARAM)hfontmain, 0);
	SendMessage(client, EM_SETMARGINS, (WPARAM)EC_LEFTMARGIN, MAKELPARAM(options.nSelectionMarginWidth, 0));
	SetTabStops();
	return TRUE;
#endif
}

void SetTabStops(void)
{
#ifdef USE_RICH_EDIT
	UINT nTmp;
	PARAFORMAT pf;
	int nWidth;
	HDC clientDC;
	BOOL bOldDirty = bDirtyFile;
	CHARRANGE cr, cr2;

	SendMessage(client, EM_EXGETSEL, 0, (LPARAM)&cr);

	SendMessage(client, WM_SETREDRAW, (WPARAM)FALSE, 0);
	cr2.cpMin = 0;
	cr2.cpMax = -1;
	SendMessage(client, EM_EXSETSEL, 0, (LPARAM)&cr2);

	clientDC = GetDC(client);
	SelectObject (clientDC, hfontmain);

	if (!GetCharWidth(clientDC, (UINT)VK_SPACE, (UINT)VK_SPACE, &nWidth))
		ERROROUT(GetString(IDS_TCHAR_WIDTH_ERROR));

	nTmp = nWidth * 15 * options.nTabStops;

	ZeroMemory(&pf, sizeof(pf)); 
	pf.cbSize = sizeof(PARAFORMAT);
	SendMessage(client, EM_GETPARAFORMAT, 0, (LPARAM)&pf);

	pf.dwMask = PFM_TABSTOPS;
	pf.cTabCount = MAX_TAB_STOPS;
	{
		int itab;
		for (itab = 0; itab < pf.cTabCount; itab++)
			pf.rgxTabs[itab] = (itab+1) * nTmp;
	}
	ReleaseDC(client, clientDC);

	bDirtyFile = TRUE;
	
	if (!SendMessage(client, EM_SETPARAFORMAT, 0, (LPARAM)(PARAFORMAT FAR *)&pf))
		ERROROUT(GetString(IDS_PARA_FORMAT_ERROR));

	bDirtyFile = bOldDirty;

	SendMessage(client, EM_EXSETSEL, 0, (LPARAM)&cr);	
	SendMessage(client, WM_SETREDRAW, (WPARAM)TRUE, 0);
	InvalidateRect(hwnd, NULL, TRUE);
#else
	UINT nTmp = options.nTabStops * 4;

	SendMessage(client, EM_SETTABSTOPS, (WPARAM)1, (LPARAM)&nTmp);
#endif
}

LPCTSTR GetShadowBuffer(void)
{
	if (lpszShadow == NULL || SendMessage(client, EM_GETMODIFY, 0, 0)) {
		UINT nSize = GetWindowTextLength(client)+1;
		if (nSize > nShadowSize) {
			if (lpszShadow != NULL)
				GlobalFree((HGLOBAL) lpszShadow);
			lpszShadow = (LPTSTR) GlobalAlloc(GPTR, nSize * sizeof(TCHAR));
			nShadowSize = nSize;
		}
		
#ifdef USE_RICH_EDIT
		{
			TEXTRANGE tr;
			tr.chrg.cpMin = 0;
			tr.chrg.cpMax = -1;
			tr.lpstrText = lpszShadow;

			SendMessage(client, EM_GETTEXTRANGE, 0, (LPARAM)&tr);
		}
#else
		GetWindowText(client, lpszShadow, nSize);
#endif
		SendMessage(client, EM_SETMODIFY, (WPARAM) FALSE, 0);
	}
	
	return lpszShadow;
}

#ifdef USE_RICH_EDIT
BOOL DoSearch(LPCTSTR szText, LONG lStart, LONG lEnd, BOOL bDown, BOOL bWholeWord, BOOL bCase, BOOL bFromTop)
{
	LONG lPrevStart = 0;
	UINT nFlags = FR_DOWN;
	FINDTEXT ft;
	CHARRANGE cr;

	if (bWholeWord)
		nFlags |= FR_WHOLEWORD;

	if (bCase)
		nFlags |= FR_MATCHCASE;

	ft.lpstrText = (LPTSTR)szText;
	
	if (bDown) {
		if (bFromTop)
			ft.chrg.cpMin = 0;
		else
			ft.chrg.cpMin = lStart + (lStart == lEnd ? 0 : 1);
		ft.chrg.cpMax = nReplaceMax;

		cr.cpMin = SendMessage(client, EM_FINDTEXT, (WPARAM)nFlags, (LPARAM)&ft);

#ifdef USE_RICH_EDIT
		if (strchr(szText, '\r')) {
			LONG lLine, lLines;

			lLine = SendMessage(client, EM_EXLINEFROMCHAR, 0, (LPARAM)cr.cpMin);
			lLines = SendMessage(client, EM_GETLINECOUNT, 0, 0);
			
			if (lLine == lLines - 1) {
				return FALSE;
			}
		}
#endif

		if (cr.cpMin == -1)
			return FALSE;
		cr.cpMax = cr.cpMin + lstrlen(szText);

		SendMessage(client, EM_EXSETSEL, 0, (LPARAM)&cr);
	}
	else {
		BOOL bFlag = 0;

		ft.chrg.cpMin = 0;
		if (bFromTop)
			ft.chrg.cpMax = -1;
		else
			ft.chrg.cpMax = (lStart == lEnd ? lEnd : lEnd - 1);
		lStart = SendMessage(client, EM_FINDTEXT, (WPARAM)nFlags, (LPARAM)&ft);
		if (lStart == -1)
			return FALSE;
		else {
			while (lStart != -1) {
				lPrevStart = lStart;
				lStart = SendMessage(client, EM_FINDTEXT, (WPARAM)nFlags, (LPARAM)&ft);
				ft.chrg.cpMin = lStart + 1;
				bFlag = 1;
			}
		}
		cr.cpMin = lPrevStart;
		cr.cpMax = lPrevStart + lstrlen(szText);
		SendMessage(client, EM_EXSETSEL, 0, (LPARAM)&cr);
	}
	if (!bReplacingAll)	UpdateStatus();
	return TRUE;
}
#else
typedef int (WINAPI* CMPFUNC)(LPCTSTR str1, LPCTSTR str2);

BOOL DoSearch(LPCTSTR szText, LONG lStart, LONG lEnd, BOOL bDown, BOOL bWholeWord, BOOL bCase, BOOL bFromTop)
{
	LONG lSize;
	int nFindLen = lstrlen(szText);
	LPCTSTR szBuffer = GetShadowBuffer();
	LPCTSTR lpszStop, lpsz, lpszFound = NULL;
	CMPFUNC pfnCompare = bCase ? lstrcmp : lstrcmpi;

	lSize = GetWindowTextLength(client);
	
	if (!szBuffer) {
		ReportLastError();
		return FALSE;
	}

	if (bDown) {
		if (nReplaceMax > -1) {
			lpszStop = szBuffer + nReplaceMax - 1;
		}
		else {
			lpszStop = szBuffer + lSize - 1;
		}
		lpsz = szBuffer + (bFromTop ? 0 : lStart + (lStart == lEnd ? 0 : 1));
	}
	else {
		lpszStop = szBuffer + (bFromTop ? lSize : lStart + (nFindLen == 1 && lStart > 0 ? -1 : 0));
		lpsz = szBuffer;
	}

	while (lpszStop != szBuffer && lpsz <= lpszStop - (bDown ? 0 : nFindLen-1) && (!bDown || (bDown && lpszFound == NULL))) {
		if ((bCase && *lpsz == *szText) || (TCHAR)(DWORD)CharLower((LPTSTR)(DWORD)(BYTE)*lpsz) == (TCHAR)(DWORD)CharLower((LPTSTR)(DWORD)(BYTE)*szText)) {
			LPTSTR lpch = (LPTSTR)(lpsz + nFindLen);
			TCHAR chSave = *lpch;
			int nResult;

			*lpch = '\0';
			nResult = (*pfnCompare)(lpsz, szText);
			*lpch = chSave;


			if (bWholeWord && lpsz > szBuffer && (_istalnum(*(lpsz-1)) || *(lpsz-1) == '_' || _istalnum(*(lpsz + nFindLen)) || *(lpsz + nFindLen) == '_'))
			;
			else if (nResult == 0) {
				lpszFound = lpsz;
			}
		}
		lpsz++;
	}

	if (lpszFound != NULL) {
		LONG lEnd;

		lStart = lpszFound - szBuffer;
		lEnd = lStart + nFindLen;
		SendMessage(client, EM_SETSEL, (WPARAM)lStart, (LPARAM)lEnd);
		if (!bReplacingAll)	UpdateStatus();
		return TRUE;
	}
	return FALSE;
}
#endif

BOOL SearchFile(LPCTSTR szText, BOOL bCase, BOOL bReplaceAll, BOOL bDown, BOOL bWholeWord)
{
	BOOL bRes;
	HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));
	CHARRANGE cr;

#ifdef USE_RICH_EDIT
	SendMessage(client, EM_EXGETSEL, 0, (LPARAM)&cr);
#else
	SendMessage(client, EM_GETSEL, (WPARAM)&cr.cpMin, (LPARAM)&cr.cpMax);
#endif
	bRes = DoSearch(szText, cr.cpMin, cr.cpMax, bDown, bWholeWord, bCase, FALSE);

	if (bRes || bReplaceAll) {
		SetCursor(hcur);
		return bRes;
	}

	if (!options.bFindAutoWrap && MessageBox(hdlgFind ? hdlgFind : client, bDown ? GetString(IDS_QUERY_SEARCH_TOP) : GetString(IDS_QUERY_SEARCH_BOTTOM), STR_METAPAD, MB_OKCANCEL|MB_ICONQUESTION) == IDCANCEL) {
		SetCursor(hcur);
		return FALSE;
	}
	else if (options.bFindAutoWrap) MessageBeep(MB_OK);

	bRes = DoSearch(szText, cr.cpMin, cr.cpMax, bDown, bWholeWord, bCase, TRUE);

	SetCursor(hcur);
	if (!bRes)
		MessageBox(hdlgFind ? hdlgFind : client, GetString(IDS_ERROR_SEARCH), STR_METAPAD, MB_OK|MB_ICONINFORMATION);

	return bRes;
}

void FixReadOnlyMenu(void)
{
	HMENU hmenu = GetMenu(hwnd);
	if (options.bReadOnlyMenu) {
		MENUITEMINFO mio;
		mio.cbSize = sizeof(MENUITEMINFO);
		mio.fMask = MIIM_TYPE | MIIM_ID | MIIM_STATE;
		mio.fType = MFT_STRING;
		mio.dwTypeData = GetString(IDS_READONLY_MENU);
		mio.wID = ID_READONLY;
		if (bReadOnly)
			mio.fState = MFS_CHECKED;
		else
			mio.fState = 0;

		InsertMenuItem(GetSubMenu(hmenu, 0), READONLYPOS, TRUE, &mio);
	}
	else {
		DeleteMenu(GetSubMenu(hmenu, 0), READONLYPOS, MF_BYPOSITION);
	}
}

void FixMRUMenus(void)
{
	HMENU hmenu = GetMenu(hwnd);
	MENUITEMINFO mio;

	mio.cbSize = sizeof(MENUITEMINFO);
	mio.fMask = MIIM_TYPE | MIIM_SUBMENU;
	mio.fType = MFT_STRING;
	mio.hSubMenu = CreateMenu();	

	if (options.bRecentOnOwn) {
		mio.dwTypeData = GetString(IDS_RECENT_MENU);
		InsertMenuItem(hmenu, 1, TRUE, &mio);
		if (hrecentmenu)
			DestroyMenu(hrecentmenu);
		hrecentmenu = mio.hSubMenu;

		DeleteMenu(GetSubMenu(hmenu, 0), RECENTPOS, MF_BYPOSITION);
		DeleteMenu(GetSubMenu(hmenu, 0), RECENTPOS, MF_BYPOSITION);
	}
	else {
		mio.dwTypeData = GetString(IDS_RECENT_FILES_MENU);
		InsertMenuItem(GetSubMenu(hmenu, 0), RECENTPOS, TRUE, &mio);
		if (hrecentmenu)
			DestroyMenu(hrecentmenu);
		hrecentmenu = mio.hSubMenu;
		mio.hSubMenu = 0;
		mio.fType = MFT_SEPARATOR;
		mio.fMask = MIIM_TYPE;
		InsertMenuItem(GetSubMenu(hmenu, 0), RECENTPOS + 1, TRUE, &mio);
		DeleteMenu(hmenu, 1, MF_BYPOSITION);
	}
	DrawMenuBar(hwnd);
}

void GotoLine(LONG lLine, LONG lOffset)
{
	LONG lTmp, lLineLen;
	CHARRANGE cr;

	if (lLine == -1 || lOffset == -1) return;

	lTmp = SendMessage(client, EM_GETLINECOUNT, 0, 0);
	if (lLine > lTmp)
		lLine = lTmp;
	else if (lLine < 1)
		lLine = 1;
	lTmp = SendMessage(client, EM_LINEINDEX, (WPARAM)lLine-1, 0);
	lLineLen = SendMessage(client, EM_LINELENGTH, (WPARAM)lTmp, 0);

	if (!options.bHideGotoOffset) {

		if (lOffset > lLineLen)
			lOffset = lLineLen + 1;
		else if (lOffset < 1)
			lOffset = 1;
		lTmp += lOffset - 1;

		cr.cpMin = cr.cpMax = lTmp;
#ifdef USE_RICH_EDIT
		SendMessage(client, EM_EXSETSEL, 0, (LPARAM)&cr);
#else
		SendMessage(client, EM_SETSEL, (WPARAM)cr.cpMin, (LPARAM)cr.cpMax);
#endif
	}
	else {
		cr.cpMin = lTmp;
		cr.cpMax = lTmp + lLineLen;
#ifdef USE_RICH_EDIT
		SendMessage(client, EM_EXSETSEL, 0, (LPARAM)&cr);
#else
		SendMessage(client, EM_SETSEL, (WPARAM)cr.cpMin, (LPARAM)cr.cpMax);
#endif
	}
	SendMessage(client, EM_SCROLLCARET, 0, 0);
	UpdateStatus();
}

BOOL CALLBACK AboutDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static BOOL bIcon = FALSE;
	switch (uMsg) {
		case WM_INITDIALOG: {
			if (bIcon) {
				SendDlgItemMessage(hwndDlg, IDC_DLGICON, STM_SETICON, (WPARAM)LoadIcon(hinstThis, MAKEINTRESOURCE(IDI_EYE)), 0);
				SetDlgItemText(hwndDlg, IDC_STATICX, STR_ABOUT_HACKER);
			}
			else {
				SetDlgItemText(hwndDlg, IDC_STATICX, STR_ABOUT_NORMAL);
			}

			CenterWindow(hwndDlg);
			SetWindowPos(client, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED); // hack for icon jitter
			SetDlgItemText(hwndDlg, IDC_EDIT_URL, STR_URL);
			SetDlgItemText(hwndDlg, IDOK, GetString(IDS_OK_BUTTON));
			SetDlgItemText(hwndDlg, IDC_STATIC_COPYRIGHT, STR_COPYRIGHT);
			SetDlgItemText(hwndDlg, IDC_STATIC_COPYRIGHT2, GetString(IDS_ALLRIGHTS));
			break;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
				case IDCANCEL:
					EndDialog(hwndDlg, wParam);
					break;
				case IDC_DLGICON:
					if (HIWORD(wParam) == STN_DBLCLK) {
						HICON hicon;
						int i;
						RECT r;
						int rands[EGGNUM] = {0, -3, 6, -8, 0, -6, 10, -3, -8, 5, 9, 2, -3, -4, 0};

						GetWindowRect(hwndDlg, &r);

						for (i = 0; i < 300; ++i)
							SetWindowPos(hwndDlg, HWND_TOP, r.left+rands[i % EGGNUM], r.top+rands[(i+1) % EGGNUM], 0, 0, SWP_NOZORDER | SWP_NOSIZE | SWP_NOACTIVATE);

						if (bIcon) {
							hicon = LoadIcon(hinstThis, MAKEINTRESOURCE(IDI_PAD));
							SetDlgItemText(hwndDlg, IDC_STATICX, STR_ABOUT_NORMAL);

							{
								HMENU hmenu = GetSubMenu(GetSubMenu(GetMenu(hwnd), EDITPOS), CONVERTPOS);
								DeleteMenu(hmenu, 0, MF_BYPOSITION);
								DeleteMenu(hmenu, 0, MF_BYPOSITION);
							}
						}
						else {
							hicon = LoadIcon(hinstThis, MAKEINTRESOURCE(IDI_EYE));
							SetDlgItemText(hwndDlg, IDC_STATICX, STR_ABOUT_HACKER);
				
							{
								HMENU hmenu = GetSubMenu(GetSubMenu(GetMenu(hwnd), EDITPOS), CONVERTPOS);

								MENUITEMINFO mio;
								mio.cbSize = sizeof(MENUITEMINFO);
								mio.fMask = MIIM_TYPE | MIIM_ID;
								mio.fType = MFT_STRING;
								mio.dwTypeData = _T("31337 h4Ck3r\tCtrl+*");
								mio.wID = ID_HACKER;

								InsertMenuItem(hmenu, 0, TRUE, &mio);
								mio.fType = MFT_SEPARATOR;
								mio.fMask = MIIM_TYPE;
								InsertMenuItem(hmenu, 1, TRUE, &mio);
							}
						}
						
						SetClassLong(GetParent(hwndDlg), GCL_HICON, (LONG) hicon);
						SendDlgItemMessage(hwndDlg, IDC_DLGICON, STM_SETICON, (WPARAM)hicon, 0);
						bIcon = !bIcon;

					}
			}
			break;
		default:
			return FALSE;
	}
	return TRUE;
}

BOOL CALLBACK AboutPluginDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		case WM_INITDIALOG: {
			SetDlgItemText(hwndDlg, IDC_EDIT_PLUGIN_LANG, GetString(IDS_PLUGIN_LANGUAGE));
			SetDlgItemText(hwndDlg, IDC_EDIT_PLUGIN_RELEASE, GetString(IDS_PLUGIN_RELEASE));
			SetDlgItemText(hwndDlg, IDC_EDIT_PLUGIN_TRANSLATOR, GetString(IDS_PLUGIN_TRANSLATOR));
			SetDlgItemText(hwndDlg, IDC_EDIT_PLUGIN_EMAIL, GetString(IDS_PLUGIN_EMAIL));
			CenterWindow(hwndDlg);
			return TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK:
				case IDCANCEL:
					EndDialog(hwndDlg, wParam);
			}
			return TRUE;
		default:
			return FALSE;
	}
}


BOOL CALLBACK GotoDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		case WM_INITDIALOG: {
			LONG lLine, lLineIndex;
			TCHAR szLine[6];
			CHARRANGE cr;

#ifdef USE_RICH_EDIT
			SendMessage(client, EM_EXGETSEL, 0, (LPARAM)&cr);
			CenterWindow(hwndDlg);
			lLine = SendMessage(client, EM_EXLINEFROMCHAR, 0, (LPARAM)cr.cpMax);
#else
			SendMessage(client, EM_GETSEL, (WPARAM)&cr.cpMin, (LPARAM)&cr.cpMax);
			CenterWindow(hwndDlg);
			lLine = SendMessage(client, EM_LINEFROMCHAR, (LPARAM)cr.cpMax, 0);
#endif
			wsprintf(szLine, _T("%d"), lLine+1);
			SetDlgItemText(hwndDlg, IDC_LINE, szLine);

			if (options.bHideGotoOffset) {
				HWND hwndItem = GetDlgItem(hwndDlg, IDC_OFFSET);
				ShowWindow(hwndItem, SW_HIDE);
				hwndItem = GetDlgItem(hwndDlg, IDC_OFFSET_TEXT);
				ShowWindow(hwndItem, SW_HIDE);
			}
			else {
				lLineIndex = SendMessage(client, EM_LINEINDEX, (WPARAM)lLine, 0);
				wsprintf(szLine, _T("%d"), 1 + cr.cpMax - lLineIndex);
				SetDlgItemText(hwndDlg, IDC_OFFSET, szLine);
				SendDlgItemMessage(hwndDlg, IDC_LINE, EM_SETSEL, 0, (LPARAM)-1);
			}
			return TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK: {
					LONG lLine, lCol = 0;
					TCHAR szLine[10];
					
					GetDlgItemText(hwndDlg, IDC_LINE, szLine, 10);
					lLine = _ttol(szLine);
					if (!options.bHideGotoOffset) {
						GetDlgItemText(hwndDlg, IDC_OFFSET, szLine, 10);
						lCol = _ttol(szLine);
					}
					GotoLine(lLine, lCol);
				}
				case IDCANCEL:
					EndDialog(hwndDlg, wParam);
			}
			return TRUE;
		default:
			return FALSE;
	}
}

BOOL CALLBACK AddFavDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
		case WM_INITDIALOG: {
			SetDlgItemText(hwndDlg, IDC_DATA, szFile);
			CenterWindow(hwndDlg);
			return TRUE;
		}
		case WM_COMMAND:
			switch (LOWORD(wParam)) {
				case IDOK: {
					TCHAR szName[MAXFN];
					GetDlgItemText(hwndDlg, IDC_DATA, szName, MAXFN);
					WritePrivateProfileString(STR_FAV_APPNAME, szName, szFile, szFav);
					PopulateFavourites();
				}
				case IDCANCEL:
					EndDialog(hwndDlg, wParam);
			}
			return TRUE;
		default:
			return FALSE;
	}
}

BOOL CALLBACK AdvancedPageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		{
			TCHAR szInt[5];

			SendDlgItemMessage(hwndDlg, IDC_GOTO_OFFSET, BM_SETCHECK, (WPARAM) options.bHideGotoOffset, 0);
			SendDlgItemMessage(hwndDlg, IDC_RECENT, BM_SETCHECK, (WPARAM) options.bRecentOnOwn, 0);
			SendDlgItemMessage(hwndDlg, IDC_SMARTHOME, BM_SETCHECK, (WPARAM) options.bNoSmartHome, 0);
			SendDlgItemMessage(hwndDlg, IDC_NO_SAVE_EXTENSIONS, BM_SETCHECK, (WPARAM) options.bNoAutoSaveExt, 0);
			SendDlgItemMessage(hwndDlg, IDC_CONTEXT_CURSOR, BM_SETCHECK, (WPARAM) options.bContextCursor, 0);
			SendDlgItemMessage(hwndDlg, IDC_CURRENT_FIND_FONT, BM_SETCHECK, (WPARAM) options.bCurrentFindFont, 0);
			SendDlgItemMessage(hwndDlg, IDC_SECONDARY_PRINT_FONT, BM_SETCHECK, (WPARAM) options.bPrintWithSecondaryFont, 0);
			SendDlgItemMessage(hwndDlg, IDC_NO_SAVE_HISTORY, BM_SETCHECK, (WPARAM) options.bNoSaveHistory, 0);
			SendDlgItemMessage(hwndDlg, IDC_NO_FIND_SELECT, BM_SETCHECK, (WPARAM) options.bNoFindAutoSelect, 0);
			SendDlgItemMessage(hwndDlg, IDC_NO_FAVES, BM_SETCHECK, (WPARAM) options.bNoFaves, 0);
			SendDlgItemMessage(hwndDlg, IDC_INSERT_TIME, BM_SETCHECK, (WPARAM) options.bDontInsertTime, 0);
			SendDlgItemMessage(hwndDlg, IDC_PROMPT_BINARY, BM_SETCHECK, (WPARAM) options.bNoWarningPrompt, 0);
			SendDlgItemMessage(hwndDlg, IDC_STICKY_WINDOW, BM_SETCHECK, (WPARAM) options.bStickyWindow, 0);
			SendDlgItemMessage(hwndDlg, IDC_READONLY_MENU, BM_SETCHECK, (WPARAM) options.bReadOnlyMenu, 0);
#ifdef USE_RICH_EDIT
			SendDlgItemMessage(hwndDlg, IDC_LINK_DC, BM_SETCHECK, (WPARAM) options.bLinkDoubleClick, 0);
			SendDlgItemMessage(hwndDlg, IDC_HIDE_SCROLLBARS, BM_SETCHECK, (WPARAM) options.bHideScrollbars, 0);
			SendDlgItemMessage(hwndDlg, IDC_SUPPRESS_UNDO_PROMPT, BM_SETCHECK, (WPARAM) options.bSuppressUndoBufferPrompt, 0);
#else
			SendDlgItemMessage(hwndDlg, IDC_DEFAULT_PRINT, BM_SETCHECK, (WPARAM) options.bDefaultPrintFont, 0);
			SendDlgItemMessage(hwndDlg, IDC_ALWAYS_LAUNCH, BM_SETCHECK, (WPARAM) options.bAlwaysLaunch, 0);
			SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_DEFAULT_PRINT, 0), 0);
#endif
			SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_STICKY_WINDOW, 0), 0);

			SendDlgItemMessage(hwndDlg, IDC_COMBO_FORMAT, CB_ADDSTRING, 0, (LPARAM)_T("DOS Text"));
			SendDlgItemMessage(hwndDlg, IDC_COMBO_FORMAT, CB_ADDSTRING, 0, (LPARAM)_T("UNIX Text"));
			SendDlgItemMessage(hwndDlg, IDC_COMBO_FORMAT, CB_ADDSTRING, 0, (LPARAM)_T("Unicode"));
			SendDlgItemMessage(hwndDlg, IDC_COMBO_FORMAT, CB_ADDSTRING, 0, (LPARAM)_T("Unicode BE"));
			SendDlgItemMessage(hwndDlg, IDC_COMBO_FORMAT, CB_ADDSTRING, 0, (LPARAM)_T("UTF-8"));


			SendDlgItemMessage(hwndDlg, IDC_COMBO_FORMAT, CB_SETCURSEL, (WPARAM)options.nFormatIndex, 0);

			wsprintf(szInt, _T("%d"), options.nMaxMRU);
			SetDlgItemText(hwndDlg, IDC_EDIT_MAX_MRU, szInt);

			return TRUE;
		}
	case WM_NOTIFY:
		switch (((NMHDR FAR *)lParam)->code) {
		case PSN_KILLACTIVE:
			{
				TCHAR szInt[10];
				INT nTmp;

				GetDlgItemText(hwndDlg, IDC_EDIT_MAX_MRU, szInt, 10);
				nTmp = _ttoi(szInt);
				if (nTmp < 0 || nTmp > 16) {
					ERROROUT(GetString(IDS_MAX_RECENT_WARNING));
					SetFocus(GetDlgItem(hwndDlg, IDC_EDIT_MAX_MRU));
					SetWindowLong (hwndDlg, DWL_MSGRESULT, TRUE);
				}								
				return TRUE;
			}
		case PSN_APPLY:
			{
				TCHAR szInt[5];
				INT nTmp;

				GetDlgItemText(hwndDlg, IDC_EDIT_MAX_MRU, szInt, 3);
				nTmp = _ttoi(szInt);
				options.nMaxMRU = nTmp;

				options.bHideGotoOffset = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_GOTO_OFFSET, BM_GETCHECK, 0, 0));
				options.bRecentOnOwn = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_RECENT, BM_GETCHECK, 0, 0));
				options.bNoSmartHome = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_SMARTHOME, BM_GETCHECK, 0, 0));
				options.bNoAutoSaveExt = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_NO_SAVE_EXTENSIONS, BM_GETCHECK, 0, 0));
				options.bContextCursor = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_CONTEXT_CURSOR, BM_GETCHECK, 0, 0));
				options.bCurrentFindFont = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_CURRENT_FIND_FONT, BM_GETCHECK, 0, 0));
				options.bPrintWithSecondaryFont = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_SECONDARY_PRINT_FONT, BM_GETCHECK, 0, 0));
				options.bNoSaveHistory = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_NO_SAVE_HISTORY, BM_GETCHECK, 0, 0));
				options.bNoFindAutoSelect = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_NO_FIND_SELECT, BM_GETCHECK, 0, 0));
				options.bNoFaves = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_NO_FAVES, BM_GETCHECK, 0, 0));
				options.bDontInsertTime = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_INSERT_TIME, BM_GETCHECK, 0, 0));
				options.bNoWarningPrompt = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_PROMPT_BINARY, BM_GETCHECK, 0, 0));
				options.bStickyWindow = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_STICKY_WINDOW, BM_GETCHECK, 0, 0));
				options.bReadOnlyMenu = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_READONLY_MENU, BM_GETCHECK, 0, 0));
#ifndef USE_RICH_EDIT
				options.bDefaultPrintFont = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_DEFAULT_PRINT, BM_GETCHECK, 0, 0));
				options.bAlwaysLaunch = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_ALWAYS_LAUNCH, BM_GETCHECK, 0, 0));
#else
				options.bLinkDoubleClick = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_LINK_DC, BM_GETCHECK, 0, 0));
				options.bHideScrollbars = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_HIDE_SCROLLBARS, BM_GETCHECK, 0, 0));
				options.bSuppressUndoBufferPrompt = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_SUPPRESS_UNDO_PROMPT, BM_GETCHECK, 0, 0));
#endif
				options.nFormatIndex = SendDlgItemMessage(hwndDlg, IDC_COMBO_FORMAT, CB_GETCURSEL, 0, 0);
				if (options.nFormatIndex == CB_ERR)
					options.nFormatIndex = 0;

				return TRUE;
			}
		}
		return FALSE;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_STICKY_WINDOW:
			if (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_STICKY_WINDOW, BM_GETCHECK, 0, 0))
				EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_STICK), TRUE);
			else
				EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_STICK), FALSE);
			break;
#ifndef USE_RICH_EDIT
		case IDC_DEFAULT_PRINT:
			if (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_DEFAULT_PRINT, BM_GETCHECK, 0, 0))
				EnableWindow(GetDlgItem(hwndDlg, IDC_SECONDARY_PRINT_FONT), FALSE);
			else
				EnableWindow(GetDlgItem(hwndDlg, IDC_SECONDARY_PRINT_FONT), TRUE);
			break;
#endif
		case IDC_BUTTON_STICK:
			SaveWindowPlacement(hwnd);
			MessageBox(hwndDlg, GetString(IDS_STICKY_MESSAGE), STR_METAPAD, MB_ICONINFORMATION);
			break;
		case IDC_BUTTON_CLEAR_FIND:
			if (MessageBox(hwndDlg, GetString(IDS_CLEAR_FIND_WARNING), STR_METAPAD, MB_ICONEXCLAMATION | MB_OKCANCEL) == IDOK) {
				ZeroMemory(FindArray, sizeof(FindArray));
				ZeroMemory(ReplaceArray, sizeof(ReplaceArray));
			}
			break;
		case IDC_BUTTON_CLEAR_RECENT:
			if (MessageBox(hwndDlg, GetString(IDS_CLEAR_RECENT_WARNING), STR_METAPAD, MB_ICONEXCLAMATION | MB_OKCANCEL) == IDOK) {
				HKEY key = NULL;
				TCHAR szKey[6];
				UINT i;

				if (!g_bIniMode && RegCreateKeyEx(HKEY_CURRENT_USER, STR_REGKEY, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &key, NULL) != ERROR_SUCCESS) {
					ReportLastError();
					break;
				}

				for (i = 1; i <= options.nMaxMRU; ++i) {
					wsprintf(szKey, _T("mru_%d"), i);
					SaveOption(key, szKey, REG_SZ, (LPBYTE)_T(""), 1);
				}

				if (key != NULL) {
					RegCloseKey(key);
				}

				PopulateMRUList();
			}
			break;
		}
		return FALSE;
	default:
		return FALSE;
	}
}

BOOL CALLBACK Advanced2PageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		{
			SendDlgItemMessage(hwndDlg, IDC_EDIT_LANG_PLUGIN, EM_LIMITTEXT, (WPARAM)MAXFN-1, 0);
			SendDlgItemMessage(hwndDlg, IDC_MACRO_1, EM_LIMITTEXT, (WPARAM)MAXMACRO-1, 0);
			SendDlgItemMessage(hwndDlg, IDC_MACRO_2, EM_LIMITTEXT, (WPARAM)MAXMACRO-1, 0);
			SendDlgItemMessage(hwndDlg, IDC_MACRO_3, EM_LIMITTEXT, (WPARAM)MAXMACRO-1, 0);
			SendDlgItemMessage(hwndDlg, IDC_MACRO_4, EM_LIMITTEXT, (WPARAM)MAXMACRO-1, 0);
			SendDlgItemMessage(hwndDlg, IDC_MACRO_5, EM_LIMITTEXT, (WPARAM)MAXMACRO-1, 0);
			SendDlgItemMessage(hwndDlg, IDC_MACRO_6, EM_LIMITTEXT, (WPARAM)MAXMACRO-1, 0);
			SendDlgItemMessage(hwndDlg, IDC_MACRO_7, EM_LIMITTEXT, (WPARAM)MAXMACRO-1, 0);
			SendDlgItemMessage(hwndDlg, IDC_MACRO_8, EM_LIMITTEXT, (WPARAM)MAXMACRO-1, 0);
			SendDlgItemMessage(hwndDlg, IDC_MACRO_9, EM_LIMITTEXT, (WPARAM)MAXMACRO-1, 0);
			SendDlgItemMessage(hwndDlg, IDC_MACRO_10, EM_LIMITTEXT, (WPARAM)MAXMACRO-1, 0);
			SetDlgItemText(hwndDlg, IDC_EDIT_LANG_PLUGIN, options.szLangPlugin);
			
			SetDlgItemText(hwndDlg, IDC_MACRO_1, options.MacroArray[0]);
			SetDlgItemText(hwndDlg, IDC_MACRO_2, options.MacroArray[1]);
			SetDlgItemText(hwndDlg, IDC_MACRO_3, options.MacroArray[2]);
			SetDlgItemText(hwndDlg, IDC_MACRO_4, options.MacroArray[3]);
			SetDlgItemText(hwndDlg, IDC_MACRO_5, options.MacroArray[4]);
			SetDlgItemText(hwndDlg, IDC_MACRO_6, options.MacroArray[5]);
			SetDlgItemText(hwndDlg, IDC_MACRO_7, options.MacroArray[6]);
			SetDlgItemText(hwndDlg, IDC_MACRO_8, options.MacroArray[7]);
			SetDlgItemText(hwndDlg, IDC_MACRO_9, options.MacroArray[8]);
			SetDlgItemText(hwndDlg, IDC_MACRO_10, options.MacroArray[9]);

			if (options.szLangPlugin[0] == '\0') {
				SendDlgItemMessage(hwndDlg, IDC_RADIO_LANG_DEFAULT, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
				SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_RADIO_LANG_DEFAULT, 0), 0);
			}
			else {
				SendDlgItemMessage(hwndDlg, IDC_RADIO_LANG_PLUGIN, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
				SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_RADIO_LANG_PLUGIN, 0), 0);
			}
		}
		return TRUE;
	case WM_NOTIFY:
		switch (((NMHDR FAR *)lParam)->code) {
		case PSN_KILLACTIVE:
			{
				if (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_RADIO_LANG_PLUGIN, BM_GETCHECK, 0, 0)) {
					TCHAR szPlugin[MAXFN];
					GetDlgItemText(hwndDlg, IDC_EDIT_LANG_PLUGIN, szPlugin, MAXFN);
					if (szPlugin[0] == '\0') {
						ERROROUT(GetString(IDS_SELECT_PLUGIN_WARNING));
						SetFocus(GetDlgItem(hwndDlg, IDC_EDIT_LANG_PLUGIN));
						SetWindowLong(hwndDlg, DWL_MSGRESULT, TRUE);
					}
				}
				return TRUE;
			}
			break;
		case PSN_APPLY:
			{
				GetDlgItemText(hwndDlg, IDC_MACRO_1, options.MacroArray[0], MAXMACRO);
				GetDlgItemText(hwndDlg, IDC_MACRO_2, options.MacroArray[1], MAXMACRO);
				GetDlgItemText(hwndDlg, IDC_MACRO_3, options.MacroArray[2], MAXMACRO);
				GetDlgItemText(hwndDlg, IDC_MACRO_4, options.MacroArray[3], MAXMACRO);
				GetDlgItemText(hwndDlg, IDC_MACRO_5, options.MacroArray[4], MAXMACRO);
				GetDlgItemText(hwndDlg, IDC_MACRO_6, options.MacroArray[5], MAXMACRO);
				GetDlgItemText(hwndDlg, IDC_MACRO_7, options.MacroArray[6], MAXMACRO);
				GetDlgItemText(hwndDlg, IDC_MACRO_8, options.MacroArray[7], MAXMACRO);
				GetDlgItemText(hwndDlg, IDC_MACRO_9, options.MacroArray[8], MAXMACRO);
				GetDlgItemText(hwndDlg, IDC_MACRO_10, options.MacroArray[9], MAXMACRO);

				if (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_RADIO_LANG_DEFAULT, BM_GETCHECK, 0, 0))
					options.szLangPlugin[0] = '\0';
				else if (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_RADIO_LANG_PLUGIN, BM_GETCHECK, 0, 0))
					GetDlgItemText(hwndDlg, IDC_EDIT_LANG_PLUGIN, options.szLangPlugin, MAXFN);
			}
			break;
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_RADIO_LANG_DEFAULT:
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_LANG_PLUGIN), FALSE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_BROWSE), FALSE);
			break;
		case IDC_RADIO_LANG_PLUGIN:
			EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_LANG_PLUGIN), TRUE);
			EnableWindow(GetDlgItem(hwndDlg, IDC_BUTTON_BROWSE), TRUE);
			break;
		case IDC_BUTTON_BROWSE:
			{
				OPENFILENAME ofn;
				TCHAR szDefExt[] = _T("dll");
				TCHAR szFilter[] = _T("metapad language plugins (*.dll)\0*.dll\0All Files (*.*)\0*.*\0");
				TCHAR szResult[MAXFN];

				szResult[0] = '\0';

				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = hwndDlg;
				ofn.lpstrFilter = szFilter;
				ofn.lpstrCustomFilter = (LPTSTR)NULL;
				ofn.nMaxCustFilter = 0L;
				ofn.nFilterIndex = 1L;
				ofn.lpstrFile = szResult;
				ofn.nMaxFile = sizeof(szResult);
				ofn.lpstrFileTitle = (LPTSTR)NULL;
				ofn.nMaxFileTitle = 0L;
				ofn.lpstrInitialDir = (LPTSTR) NULL;
				ofn.lpstrTitle = (LPTSTR)NULL;
				ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
				ofn.nFileOffset = 0;
				ofn.nFileExtension = 0;
				ofn.lpstrDefExt = szDefExt;

				if (GetOpenFileName(&ofn)) {
					if (g_bDisablePluginVersionChecking) {
						SetDlgItemText(hwndDlg, IDC_EDIT_LANG_PLUGIN, szResult);
					}
					else {
						HINSTANCE hinstTemp = LoadAndVerifyLanguagePlugin(szResult);
						if (hinstTemp) {
							SetDlgItemText(hwndDlg, IDC_EDIT_LANG_PLUGIN, szResult);
							FreeLibrary(hinstTemp);
						}
						else {
							SendDlgItemMessage(hwndDlg, IDC_RADIO_LANG_DEFAULT, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
							SendDlgItemMessage(hwndDlg, IDC_RADIO_LANG_PLUGIN, BM_SETCHECK, (WPARAM)BST_UNCHECKED, 0);
							SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_RADIO_LANG_DEFAULT, 0), 0);
						}
					}
				}
			}
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

BOOL CALLBACK ViewPageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static LOGFONT TmpPrimaryFont, TmpSecondaryFont;
	static HFONT hfont = NULL, hfont2 = NULL;
	static COLORREF CustomClrs[16], TmpBackColour, TmpFontColour, TmpBackColour2, TmpFontColour2;
	static HBRUSH hbackbrush, hfontbrush;
	static HBRUSH hbackbrush2, hfontbrush2;

	switch (uMsg) {
	case WM_INITDIALOG:
		{
			TCHAR szInt[5];

			TmpPrimaryFont = options.PrimaryFont;
			TmpSecondaryFont = options.SecondaryFont;
			TmpBackColour = options.BackColour;
			TmpFontColour = options.FontColour;
			TmpBackColour2 = options.BackColour2;
			TmpFontColour2 = options.FontColour2;

			hbackbrush = CreateSolidBrush(TmpBackColour);
			hfontbrush = CreateSolidBrush(TmpFontColour);
			hbackbrush2 = CreateSolidBrush(TmpBackColour2);
			hfontbrush2 = CreateSolidBrush(TmpFontColour2);

			if (options.nPrimaryFont == 0)
				SendDlgItemMessage(hwndDlg, IDC_FONT_PRIMARY, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else
				SendDlgItemMessage(hwndDlg, IDC_FONT_PRIMARY_2, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);

			if (options.nSecondaryFont == 0)
				SendDlgItemMessage(hwndDlg, IDC_FONT_SECONDARY, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else
				SendDlgItemMessage(hwndDlg, IDC_FONT_SECONDARY_2, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);

			/*
			wsprintf(szInt, "%d", options.nStatusFontWidth);
			SetDlgItemText(hwndDlg, IDC_EDIT_SB_FONT_WIDTH, szInt);
			*/

			wsprintf(szInt, _T("%d"), options.nSelectionMarginWidth);
			SetDlgItemText(hwndDlg, IDC_EDIT_SM_WIDTH, szInt);

			if (SetLWA) {
				wsprintf(szInt, _T("%d"), options.nTransparentPct);
				SetDlgItemText(hwndDlg, IDC_EDIT_TRANSPARENT, szInt);
			}
			else {
				EnableWindow(GetDlgItem(hwndDlg, IDC_STAT_TRANS), FALSE);
				EnableWindow(GetDlgItem(hwndDlg, IDC_EDIT_TRANSPARENT), FALSE);
			}

			SendDlgItemMessage(hwndDlg, IDC_FLAT_TOOLBAR, BM_SETCHECK, (WPARAM) options.bUnFlatToolbar, 0);
			SendDlgItemMessage(hwndDlg, IDC_SYSTEM_COLOURS, BM_SETCHECK, (WPARAM) options.bSystemColours, 0);
			SendDlgItemMessage(hwndDlg, IDC_SYSTEM_COLOURS2, BM_SETCHECK, (WPARAM) options.bSystemColours2, 0);
			SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_SYSTEM_COLOURS, 0), 0);
			SendMessage(hwndDlg, WM_COMMAND, MAKEWPARAM(IDC_SYSTEM_COLOURS2, 0), 0);

			hfont = CreateFontIndirect(&TmpPrimaryFont);
			SendDlgItemMessage(hwndDlg, IDC_BTN_FONT1, WM_SETFONT, (WPARAM)hfont, 0);
			hfont2 = CreateFontIndirect(&TmpSecondaryFont);
			SendDlgItemMessage(hwndDlg, IDC_BTN_FONT2, WM_SETFONT, (WPARAM)hfont2, 0);
		}
		return TRUE;
	case WM_NOTIFY:
		switch (((NMHDR FAR *)lParam)->code) {
		case PSN_KILLACTIVE:
			{
				TCHAR szInt[10];
				INT nTmp;

				/*
				GetDlgItemText(hwndDlg, IDC_EDIT_SB_FONT_WIDTH, szInt, 10);
				nTmp = _ttoi(szInt);
				if (nTmp < 1 || nTmp > 1000) {
					ERROROUT("Enter a font size between 1 and 1000");
					SetFocus(GetDlgItem(hwndDlg, IDC_EDIT_SB_FONT_WIDTH));
					SetWindowLong (hwndDlg, DWL_MSGRESULT, TRUE);
				}
				*/
				
				GetDlgItemText(hwndDlg, IDC_EDIT_SM_WIDTH, szInt, 10);
				nTmp = _ttoi(szInt);
				if (nTmp < 0 || nTmp > 300) {
					ERROROUT(GetString(IDS_MARGIN_WIDTH_WARNING));
					SetFocus(GetDlgItem(hwndDlg, IDC_EDIT_SM_WIDTH));
					SetWindowLong (hwndDlg, DWL_MSGRESULT, TRUE);
				}
				

				GetDlgItemText(hwndDlg, IDC_EDIT_TRANSPARENT, szInt, 10);
				nTmp = _ttoi(szInt);
				if (SetLWA && (nTmp < 1 || nTmp > 99)) {
					ERROROUT(GetString(IDS_TRANSPARENCY_WARNING));
					SetFocus(GetDlgItem(hwndDlg, IDC_EDIT_TRANSPARENT));
					SetWindowLong (hwndDlg, DWL_MSGRESULT, TRUE);
				}

				return TRUE;
			}
		case PSN_APPLY:
			{

				TCHAR szInt[5];
				INT nTmp;
/*
				GetDlgItemText(hwndDlg, IDC_EDIT_SB_FONT_WIDTH, szInt, 5);
				nTmp = _ttoi(szInt);
				options.nStatusFontWidth = nTmp;
*/
				GetDlgItemText(hwndDlg, IDC_EDIT_SM_WIDTH, szInt, 5);
				nTmp = _ttoi(szInt);
				options.nSelectionMarginWidth = nTmp;

				GetDlgItemText(hwndDlg, IDC_EDIT_TRANSPARENT, szInt, 5);
				nTmp = _ttoi(szInt);
				options.nTransparentPct = nTmp;
				
				if (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_FONT_PRIMARY, BM_GETCHECK, 0, 0))
					options.nPrimaryFont = 0;
				else
					options.nPrimaryFont = 1;

				if (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_FONT_SECONDARY, BM_GETCHECK, 0, 0))
					options.nSecondaryFont = 0;
				else
					options.nSecondaryFont = 1;

				if (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_SYSTEM_COLOURS, BM_GETCHECK, 0, 0))
					options.bSystemColours = TRUE;
				else
					options.bSystemColours = FALSE;

				if (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_SYSTEM_COLOURS2, BM_GETCHECK, 0, 0))
					options.bSystemColours2 = TRUE;
				else
					options.bSystemColours2 = FALSE;

				options.bUnFlatToolbar = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_FLAT_TOOLBAR, BM_GETCHECK, 0, 0));
				options.PrimaryFont = TmpPrimaryFont;
				options.SecondaryFont = TmpSecondaryFont;
				options.BackColour = TmpBackColour;
				options.FontColour = TmpFontColour;
				options.BackColour2 = TmpBackColour2;
				options.FontColour2 = TmpFontColour2;
			}
		case PSN_RESET:
			if (hfont)
				DeleteObject(hfont);
			if (hfont2)
				DeleteObject(hfont2);
			if (hbackbrush)
				DeleteObject(hbackbrush);
			if (hfontbrush)
				DeleteObject(hfontbrush);
			if (hbackbrush2)
				DeleteObject(hbackbrush2);
			if (hfontbrush2)
				DeleteObject(hfontbrush2);
			break;
		}
		break;
	case WM_CTLCOLORBTN:
		if (GetDlgItem(hwndDlg, IDC_COLOUR_BACK) == (HWND)lParam) {
			SetBkColor((HDC)wParam, TmpBackColour);
			return (BOOL)hbackbrush;
		}
		if (GetDlgItem(hwndDlg, IDC_COLOUR_FONT) == (HWND)lParam) {
			SetBkColor((HDC)wParam, TmpFontColour);
			return (BOOL)hfontbrush;
		}
		if (GetDlgItem(hwndDlg, IDC_COLOUR_BACK2) == (HWND)lParam) {
			SetBkColor((HDC)wParam, TmpBackColour2);
			return (BOOL)hbackbrush2;
		}
		if (GetDlgItem(hwndDlg, IDC_COLOUR_FONT2) == (HWND)lParam) {
			SetBkColor((HDC)wParam, TmpFontColour2);
			return (BOOL)hfontbrush2;
		}
		return (BOOL)NULL;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_COLOUR_BACK:
		case IDC_COLOUR_FONT:
		case IDC_COLOUR_BACK2:
		case IDC_COLOUR_FONT2:
			{
				CHOOSECOLOR cc;

				ZeroMemory(&cc, sizeof(CHOOSECOLOR));
				cc.lStructSize = sizeof(CHOOSECOLOR);
				cc.hwndOwner = hwndDlg;
				cc.lpCustColors = (LPDWORD) CustomClrs;
				cc.Flags = CC_FULLOPEN | CC_RGBINIT;
				switch (LOWORD(wParam)) {
				case IDC_COLOUR_BACK:
					cc.rgbResult = TmpBackColour;
					if (ChooseColor(&cc)) {
						TmpBackColour = cc.rgbResult;
						if (hbackbrush)
							DeleteObject(hbackbrush);
						hbackbrush = CreateSolidBrush(TmpBackColour);
						InvalidateRect(GetDlgItem(hwndDlg, IDC_COLOUR_BACK), NULL, TRUE);
					}
					break;
				case IDC_COLOUR_FONT:
					cc.rgbResult = TmpFontColour;
					if (ChooseColor(&cc)) {
						TmpFontColour = cc.rgbResult;
						if (hfontbrush)
							DeleteObject(hfontbrush);
						hfontbrush = CreateSolidBrush(TmpFontColour);
						InvalidateRect(GetDlgItem(hwndDlg, IDC_COLOUR_FONT), NULL, TRUE);
					}
					break;
				case IDC_COLOUR_BACK2:
					cc.rgbResult = TmpBackColour2;
					if (ChooseColor(&cc)) {
						TmpBackColour2 = cc.rgbResult;
						if (hbackbrush2)
							DeleteObject(hbackbrush2);
						hbackbrush2 = CreateSolidBrush(TmpBackColour2);
						InvalidateRect(GetDlgItem(hwndDlg, IDC_COLOUR_BACK2), NULL, TRUE);
					}
					break;
				case IDC_COLOUR_FONT2:
					cc.rgbResult = TmpFontColour2;
					if (ChooseColor(&cc)) {
						TmpFontColour2 = cc.rgbResult;
						if (hfontbrush2)
							DeleteObject(hfontbrush2);
						hfontbrush2 = CreateSolidBrush(TmpFontColour2);
						InvalidateRect(GetDlgItem(hwndDlg, IDC_COLOUR_FONT2), NULL, TRUE);
					}
					break;
				}
			}
			return TRUE;
		case IDC_BTN_FONT1:
			{
				CHOOSEFONT cf;

				ZeroMemory(&cf, sizeof(CHOOSEFONT));
				cf.lStructSize = sizeof (CHOOSEFONT);
				cf.hwndOwner = hwndDlg;
				cf.lpLogFont = &TmpPrimaryFont;
				cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
				if (ChooseFont(&cf)) {
					if (hfont)
						DeleteObject(hfont);

					hfont = CreateFontIndirect(&TmpPrimaryFont);
					SendDlgItemMessage(hwndDlg, IDC_BTN_FONT1, WM_SETFONT, (WPARAM)hfont, 0);
					SendDlgItemMessage(hwndDlg, IDC_FONT_PRIMARY, BM_SETCHECK, (WPARAM)FALSE, 0);
					SendDlgItemMessage(hwndDlg, IDC_FONT_PRIMARY_2, BM_SETCHECK, (WPARAM)TRUE, 0);
				}
				InvalidateRect(GetDlgItem(hwndDlg, IDC_BTN_FONT1), NULL, TRUE);
			}
			break;
		case IDC_BTN_FONT2:
			{
				CHOOSEFONT cf;

				ZeroMemory(&cf, sizeof(CHOOSEFONT));
				cf.lStructSize = sizeof(CHOOSEFONT);
				cf.hwndOwner = hwndDlg;
				cf.lpLogFont = &TmpSecondaryFont;
				cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT;
				if (ChooseFont(&cf)) {
					if (hfont2)
						DeleteObject(hfont2);
					hfont2 = CreateFontIndirect(&TmpSecondaryFont);
					SendDlgItemMessage(hwndDlg, IDC_BTN_FONT2, WM_SETFONT, (WPARAM)hfont2, 0);
					SendDlgItemMessage(hwndDlg, IDC_FONT_SECONDARY, BM_SETCHECK, (WPARAM)FALSE, 0);
					SendDlgItemMessage(hwndDlg, IDC_FONT_SECONDARY_2, BM_SETCHECK, (WPARAM)TRUE, 0);
				}
				InvalidateRect(GetDlgItem(hwndDlg, IDC_BTN_FONT2), NULL, TRUE);
			}
			break;
		case IDC_SYSTEM_COLOURS:
			{
				BOOL bEnable;
				if (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_SYSTEM_COLOURS, BM_GETCHECK, 0, 0)) {
					if (hfontbrush)
						DeleteObject(hfontbrush);
					hfontbrush = GetSysColorBrush(COLOR_WINDOWTEXT);
					TmpFontColour = GetSysColor(COLOR_WINDOWTEXT);

					if (hbackbrush)
						DeleteObject(hbackbrush);
					hbackbrush = GetSysColorBrush(COLOR_WINDOW);
					TmpBackColour = GetSysColor(COLOR_WINDOW);

					InvalidateRect(GetDlgItem(hwndDlg, IDC_COLOUR_BACK), NULL, TRUE);
					InvalidateRect(GetDlgItem(hwndDlg, IDC_COLOUR_FONT), NULL, TRUE);
					bEnable = FALSE;
				}
				else
					bEnable = TRUE;
				EnableWindow(GetDlgItem(hwndDlg, IDC_STAT_FONT), bEnable);
				EnableWindow(GetDlgItem(hwndDlg, IDC_STAT_WIND), bEnable);
				EnableWindow(GetDlgItem(hwndDlg, IDC_COLOUR_BACK), bEnable);
				EnableWindow(GetDlgItem(hwndDlg, IDC_COLOUR_FONT), bEnable);
			}
			return FALSE;
		case IDC_SYSTEM_COLOURS2:
			{
				BOOL bEnable;
				if (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_SYSTEM_COLOURS2, BM_GETCHECK, 0, 0)) {
					if (hfontbrush2)
						DeleteObject(hfontbrush2);
					hfontbrush2 = GetSysColorBrush(COLOR_WINDOWTEXT);
					TmpFontColour2 = GetSysColor(COLOR_WINDOWTEXT);

					if (hbackbrush2)
						DeleteObject(hbackbrush2);
					hbackbrush2 = GetSysColorBrush(COLOR_WINDOW);
					TmpBackColour2 = GetSysColor(COLOR_WINDOW);

					InvalidateRect(GetDlgItem(hwndDlg, IDC_COLOUR_BACK2), NULL, TRUE);
					InvalidateRect(GetDlgItem(hwndDlg, IDC_COLOUR_FONT2), NULL, TRUE);
					bEnable = FALSE;
				}
				else
					bEnable = TRUE;
				EnableWindow(GetDlgItem(hwndDlg, IDC_STAT_FONT2), bEnable);
				EnableWindow(GetDlgItem(hwndDlg, IDC_STAT_WIND2), bEnable);
				EnableWindow(GetDlgItem(hwndDlg, IDC_COLOUR_BACK2), bEnable);
				EnableWindow(GetDlgItem(hwndDlg, IDC_COLOUR_FONT2), bEnable);
			}
			return FALSE;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

BOOL CALLBACK GeneralPageProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg) {
	case WM_INITDIALOG:
		{
			TCHAR szInt[5];
		
			CenterWindow(hwndSheet);

			SendDlgItemMessage(hwndDlg, IDC_EDIT_BROWSER, EM_LIMITTEXT, (WPARAM)MAXFN-1, 0);
			SendDlgItemMessage(hwndDlg, IDC_EDIT_ARGS, EM_LIMITTEXT, (WPARAM)MAXARGS-1, 0);
			SendDlgItemMessage(hwndDlg, IDC_EDIT_QUOTE, EM_LIMITTEXT, (WPARAM)MAXQUOTE-1, 0);
			SetDlgItemText(hwndDlg, IDC_EDIT_BROWSER, options.szBrowser);
			SetDlgItemText(hwndDlg, IDC_EDIT_ARGS, options.szArgs);
			SetDlgItemText(hwndDlg, IDC_EDIT_BROWSER2, options.szBrowser2);
			SetDlgItemText(hwndDlg, IDC_EDIT_ARGS2, options.szArgs2);
			SetDlgItemText(hwndDlg, IDC_EDIT_QUOTE, options.szQuote);
			SendDlgItemMessage(hwndDlg, IDC_CHECK_QUICKEXIT, BM_SETCHECK, (WPARAM) options.bQuickExit, 0);
			SendDlgItemMessage(hwndDlg, IDC_CHECK_SAVEMENUSETTINGS, BM_SETCHECK, (WPARAM) options.bSaveMenuSettings, 0);
			SendDlgItemMessage(hwndDlg, IDC_CHECK_SAVEWINDOWPLACEMENT, BM_SETCHECK, (WPARAM) options.bSaveWindowPlacement, 0);
			SendDlgItemMessage(hwndDlg, IDC_CHECK_SAVEDIRECTORY, BM_SETCHECK, (WPARAM) options.bSaveDirectory, 0);
			SendDlgItemMessage(hwndDlg, IDC_CHECK_LAUNCH_CLOSE, BM_SETCHECK, (WPARAM) options.bLaunchClose, 0);
			SendDlgItemMessage(hwndDlg, IDC_FIND_AUTO_WRAP, BM_SETCHECK, (WPARAM)options.bFindAutoWrap, 0);
			SendDlgItemMessage(hwndDlg, IDC_AUTO_INDENT, BM_SETCHECK, (WPARAM)options.bAutoIndent, 0);
			SendDlgItemMessage(hwndDlg, IDC_INSERT_SPACES, BM_SETCHECK, (WPARAM)options.bInsertSpaces, 0);
			SendDlgItemMessage(hwndDlg, IDC_NO_CAPTION_DIR, BM_SETCHECK, (WPARAM)options.bNoCaptionDir, 0);

			wsprintf(szInt, _T("%d"), options.nTabStops);
			SetDlgItemText(hwndDlg, IDC_TAB_STOP, szInt);

			if (options.nLaunchSave == 0)
				SendDlgItemMessage(hwndDlg, IDC_LAUNCH_SAVE0, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else if (options.nLaunchSave == 1)
				SendDlgItemMessage(hwndDlg, IDC_LAUNCH_SAVE1, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
			else
				SendDlgItemMessage(hwndDlg, IDC_LAUNCH_SAVE2, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
		}
		return TRUE;
	case WM_NOTIFY:
		switch (((NMHDR FAR *)lParam)->code) {
		case PSN_KILLACTIVE:
			{
				TCHAR szInt[5];
				int nTmp;

				GetDlgItemText(hwndDlg, IDC_TAB_STOP, szInt, 5);
				nTmp = _ttoi(szInt);
				if (nTmp < 1 || nTmp > 100) {
					ERROROUT(GetString(IDS_TAB_SIZE_WARNING));
					SetFocus(GetDlgItem(hwndDlg, IDC_TAB_STOP));
					SetWindowLong (hwndDlg, DWL_MSGRESULT, TRUE);
				}
				return TRUE;
			}
			break;
		case PSN_APPLY:
			{
				int nTmp;
				TCHAR szInt[5];

				GetDlgItemText(hwndDlg, IDC_TAB_STOP, szInt, 5);
				nTmp = _ttoi(szInt);
				options.nTabStops = nTmp;

				GetDlgItemText(hwndDlg, IDC_EDIT_BROWSER, options.szBrowser, MAXFN);
				GetDlgItemText(hwndDlg, IDC_EDIT_ARGS, options.szArgs, MAXARGS);
				GetDlgItemText(hwndDlg, IDC_EDIT_BROWSER2, options.szBrowser2, MAXFN);
				GetDlgItemText(hwndDlg, IDC_EDIT_ARGS2, options.szArgs2, MAXARGS);
				GetDlgItemText(hwndDlg, IDC_EDIT_QUOTE, options.szQuote, MAXQUOTE);

				if (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_LAUNCH_SAVE0, BM_GETCHECK, 0, 0))
					options.nLaunchSave = 0;
				else if (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_LAUNCH_SAVE1, BM_GETCHECK, 0, 0))
					options.nLaunchSave = 1;
				else
					options.nLaunchSave = 2;

				options.bNoCaptionDir = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_NO_CAPTION_DIR, BM_GETCHECK, 0, 0));
				options.bAutoIndent = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_AUTO_INDENT, BM_GETCHECK, 0, 0));
				options.bInsertSpaces = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_INSERT_SPACES, BM_GETCHECK, 0, 0));
				options.bFindAutoWrap = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_FIND_AUTO_WRAP, BM_GETCHECK, 0, 0));
				options.bLaunchClose = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_CHECK_LAUNCH_CLOSE, BM_GETCHECK, 0, 0));
				options.bQuickExit = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_CHECK_QUICKEXIT, BM_GETCHECK, 0, 0));
				options.bSaveWindowPlacement = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_CHECK_SAVEWINDOWPLACEMENT, BM_GETCHECK, 0, 0));
				options.bSaveMenuSettings = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_CHECK_SAVEMENUSETTINGS, BM_GETCHECK, 0, 0));
				options.bSaveDirectory = (BST_CHECKED == SendDlgItemMessage(hwndDlg, IDC_CHECK_SAVEDIRECTORY, BM_GETCHECK, 0, 0));
			}
			break;
		}
		break;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_BUTTON_BROWSE:
		case IDC_BUTTON_BROWSE2:
			{
				OPENFILENAME ofn;
				TCHAR szDefExt[] = _T("exe");
				TCHAR szFilter[] = _T("Executable Files (*.exe)\0*.exe\0All Files (*.*)\0*.*\0");
				TCHAR szResult[MAXFN];

				szResult[0] = '\0';

				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = hwndDlg;
				ofn.lpstrFilter = szFilter;
				ofn.lpstrCustomFilter = (LPTSTR)NULL;
				ofn.nMaxCustFilter = 0L;
				ofn.nFilterIndex = 1L;
				ofn.lpstrFile = szResult;
				ofn.nMaxFile = sizeof(szResult);
				ofn.lpstrFileTitle = (LPTSTR)NULL;
				ofn.nMaxFileTitle = 0L;
				ofn.lpstrInitialDir = (LPTSTR) NULL;
				ofn.lpstrTitle = (LPTSTR)NULL;
				ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
				ofn.nFileOffset = 0;
				ofn.nFileExtension = 0;
				ofn.lpstrDefExt = szDefExt;

				if (GetOpenFileName(&ofn)) {
					if (LOWORD(wParam) == IDC_BUTTON_BROWSE) {
						SetDlgItemText(hwndDlg, IDC_EDIT_BROWSER, szResult);
					}
					else {
						SetDlgItemText(hwndDlg, IDC_EDIT_BROWSER2, szResult);
					}
				}
			}
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

int CALLBACK SheetInitProc(HWND hwndDlg, UINT uMsg, LPARAM lParam)
{
	if (uMsg == PSCB_PRECREATE) {
		if (((LPDLGTEMPLATEEX)lParam)->signature == 0xFFFF) {
			((LPDLGTEMPLATEEX)lParam)->style &= ~DS_CONTEXTHELP;
		}
		else {
			((LPDLGTEMPLATE)lParam)->style &= ~DS_CONTEXTHELP;
		}
	}
	else if (hwndDlg) {
		hwndSheet = hwndDlg;
	}
	return TRUE;
}	

LONG WINAPI MainWndProc(HWND hwndMain, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	if (Msg == WM_COMMAND && ID_FAV_RANGE_BASE < LOWORD(wParam) && LOWORD(wParam) <= ID_FAV_RANGE_MAX) {
		LoadFileFromMenu(LOWORD(wParam), FALSE);
		return FALSE;
	}
	
	switch(Msg) {
//	case WM_ERASEBKGND:
//		return (LRESULT)1;
	case WM_DESTROY:
		if (options.bSaveWindowPlacement && !options.bStickyWindow)
			SaveWindowPlacement(hwndMain);
		SaveMenusAndData();
		CleanUp();
		PostQuitMessage(0);
		break;
	case WM_ACTIVATE:
		UpdateStatus();
		SetFocus(client);
		break;
	case WM_DROPFILES:
		{
			HDROP hDrop = (HDROP) wParam;

			if (!SaveIfDirty())
				break;

			DragQueryFile(hDrop, 0, szFile, MAXFN);
			DragFinish(hDrop);

			bLoading = TRUE;
			bHideMessage = FALSE;
			lstrcpy(szStatusMessage, GetString(IDS_FILE_LOADING));
			UpdateStatus();
			LoadFile(szFile, FALSE, TRUE);
			if (bLoading) {
				bLoading = FALSE;
				bDirtyFile = FALSE;
				UpdateCaption();
			}
			else {
				MakeNewFile();
			}
			break;
		}
	case WM_SIZING:
		break;
	case WM_SIZE: {
		if (client == NULL) break;
		SetWindowPos(client, 0, 0, GetToolbarHeight(), LOWORD(lParam), HIWORD(lParam) - GetStatusHeight() - GetToolbarHeight(), SWP_SHOWWINDOW);
		if (bWordWrap) {
			SetWindowLong(client, GWL_STYLE, GetWindowLong(client, GWL_STYLE) & ~WS_HSCROLL);
			SetWindowPos(client, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
			UpdateStatus();
		}
		if (bShowStatus) {
			SendMessage(status, WM_SIZE, 0, 0);
			InvalidateRect(status, NULL, TRUE);
		}

		if (bShowToolbar)
			SendMessage(toolbar, WM_SIZE, 0, 0);
		
		break;
	}
#ifndef USE_RICH_EDIT
	case WM_CTLCOLOREDIT:
		if ((HWND)lParam == client) {
			/*
			if (!bPrimaryFont && options.bSecondarySystemColour) {
				SetBkColor((HDC)wParam, GetSysColor(COLOR_WINDOW));
				SetTextColor((HDC)wParam, GetSysColor(COLOR_WINDOWTEXT));
				if (BackBrush)
					DeleteObject(BackBrush);
				BackBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
			}
			*/
			SetBkColor((HDC)wParam, bPrimaryFont ? options.BackColour : options.BackColour2);
			SetTextColor((HDC)wParam, bPrimaryFont ? options.FontColour : options.FontColour2);
			if (BackBrush)
				DeleteObject(BackBrush);
			BackBrush = CreateSolidBrush(bPrimaryFont ? options.BackColour : options.BackColour2);
			return (long)BackBrush;
		}
		break;
#endif
	case WM_CLOSE:
		if (!SaveIfDirty())
			break;
		DestroyWindow(hwndMain);
		break;
	case WM_QUERYENDSESSION:
		if (!SaveIfDirty())
			return FALSE;
		else
			return TRUE;
	case WM_INITMENUPOPUP: {
		HMENU hmenuPopup = (HMENU) wParam;
		INT nPos = LOWORD(lParam);
		if ((BOOL)HIWORD(lParam) == TRUE)
			break;
		
		if (nPos == EDITPOS) {
			CHARRANGE cr;

			if (IsClipboardFormatAvailable(CF_TEXT))
				EnableMenuItem(hmenuPopup, ID_MYEDIT_PASTE, MF_BYCOMMAND | MF_ENABLED);
			else
				EnableMenuItem(hmenuPopup, ID_MYEDIT_PASTE, MF_BYCOMMAND | MF_GRAYED);
/*
#ifdef USE_RICH_EDIT
			if (SendMessage(client, EM_CANPASTE, 0, 0))
				EnableMenuItem(hmenuPopup, ID_MYEDIT_PASTE, MF_BYCOMMAND | MF_ENABLED);
			else
				EnableMenuItem(hmenuPopup, ID_MYEDIT_PASTE, MF_BYCOMMAND | MF_GRAYED);
#endif
*/
			if (bWordWrap)
				EnableMenuItem(hmenuPopup, ID_COMMIT_WORDWRAP, MF_BYCOMMAND | MF_ENABLED);
			else
				EnableMenuItem(hmenuPopup, ID_COMMIT_WORDWRAP, MF_BYCOMMAND | MF_GRAYED);

#ifdef USE_RICH_EDIT
			SendMessage(client, EM_EXGETSEL, 0, (LPARAM)&cr);
#else
			SendMessage(client, EM_GETSEL, (WPARAM)&cr.cpMin, (LPARAM)&cr.cpMax);
#endif
			if (cr.cpMin == cr.cpMax) {
				EnableMenuItem(hmenuPopup, ID_MYEDIT_CUT, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(hmenuPopup, ID_MYEDIT_COPY, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(hmenuPopup, ID_MYEDIT_DELETE, MF_BYCOMMAND | MF_GRAYED);
			}
			else {
				EnableMenuItem(hmenuPopup, ID_MYEDIT_CUT, MF_BYCOMMAND | MF_ENABLED);
				EnableMenuItem(hmenuPopup, ID_MYEDIT_COPY, MF_BYCOMMAND | MF_ENABLED);
				EnableMenuItem(hmenuPopup, ID_MYEDIT_DELETE, MF_BYCOMMAND | MF_ENABLED);
			}
			if (SendMessage(client, EM_CANUNDO, 0, 0))
				EnableMenuItem(hmenuPopup, ID_MYEDIT_UNDO, MF_BYCOMMAND | MF_ENABLED);
			else
				EnableMenuItem(hmenuPopup, ID_MYEDIT_UNDO, MF_BYCOMMAND | MF_GRAYED);
#ifdef USE_RICH_EDIT
			if (SendMessage(client, EM_CANREDO, 0, 0))
				EnableMenuItem(hmenuPopup, ID_MYEDIT_REDO, MF_BYCOMMAND | MF_ENABLED);
			else
				EnableMenuItem(hmenuPopup, ID_MYEDIT_REDO, MF_BYCOMMAND | MF_GRAYED);
#endif
		}
		else if (nPos == 0) {
			if (szFile[0] != '\0') {
				EnableMenuItem(hmenuPopup, ID_RELOAD_CURRENT, MF_BYCOMMAND | MF_ENABLED);
				EnableMenuItem(hmenuPopup, ID_READONLY, MF_BYCOMMAND | MF_ENABLED);
			}
			else {
				EnableMenuItem(hmenuPopup, ID_RELOAD_CURRENT, MF_BYCOMMAND | MF_GRAYED);
				EnableMenuItem(hmenuPopup, ID_READONLY, MF_BYCOMMAND | MF_GRAYED);
			}
		}
		else if (!options.bNoFaves && nPos == FAVEPOS) {
			if (szFile[0] != '\0') {
				EnableMenuItem(hmenuPopup, ID_FAV_ADD, MF_BYCOMMAND | MF_ENABLED);
			}
			else {
				EnableMenuItem(hmenuPopup, ID_FAV_ADD, MF_BYCOMMAND | MF_GRAYED);
			}
			if (bHasFaves) {
				EnableMenuItem(hmenuPopup, ID_FAV_EDIT, MF_BYCOMMAND | MF_ENABLED);
			}
			else {
				EnableMenuItem(hmenuPopup, ID_FAV_EDIT, MF_BYCOMMAND | MF_GRAYED);
			}

		}
		break;
	}
	case WM_NOTIFY: {
		switch (((LPNMHDR)lParam)->code) {
			case TTN_NEEDTEXT: {
				LPTOOLTIPTEXT lpttt = (LPTOOLTIPTEXT)lParam;
				switch (lpttt->hdr.idFrom) {
					/*
					case ID_NEW_INSTANCE:
						lpttt->lpszText = GetString(IDS_NEW_INSTANCE);
						break;
					*/
					case ID_MYFILE_NEW:
						lpttt->lpszText = GetString(IDS_TB_NEWFILE);
						break;
					case ID_MYFILE_OPEN:
						lpttt->lpszText = GetString(IDS_TB_OPENFILE);
						break;
					case ID_MYFILE_SAVE:
						lpttt->lpszText = GetString(IDS_TB_SAVEFILE);
						break;
					case ID_PRINT:
						lpttt->lpszText = GetString(IDS_TB_PRINT);
						break;
					case ID_FIND:
						lpttt->lpszText = GetString(IDS_TB_FIND);
						break;
					case ID_REPLACE:
						lpttt->lpszText = GetString(IDS_TB_REPLACE);
						break;
					case ID_MYEDIT_CUT:
						lpttt->lpszText = GetString(IDS_TB_CUT);
						break;
					case ID_MYEDIT_COPY:
						lpttt->lpszText = GetString(IDS_TB_COPY);
						break;
					case ID_MYEDIT_PASTE:
						lpttt->lpszText = GetString(IDS_TB_PASTE);
						break;
					case ID_MYEDIT_UNDO:
						lpttt->lpszText = GetString(IDS_TB_UNDO);
						break;
#ifdef USE_RICH_EDIT
					case ID_MYEDIT_REDO:
						lpttt->lpszText = GetString(IDS_TB_REDO);
						break;
#endif
					case ID_VIEW_OPTIONS:
						lpttt->lpszText = GetString(IDS_TB_SETTINGS);
						break;
					case ID_RELOAD_CURRENT:
						lpttt->lpszText = GetString(IDS_TB_REFRESH);
						break;
					case ID_EDIT_WORDWRAP:
						lpttt->lpszText = GetString(IDS_TB_WORDWRAP);
						break;
					case ID_FONT_PRIMARY:
						lpttt->lpszText = GetString(IDS_TB_PRIMARYFONT);
						break;
					case ID_ALWAYSONTOP:
						lpttt->lpszText = GetString(IDS_TB_ONTOP);
						break;
					case ID_FILE_LAUNCHVIEWER:
						lpttt->lpszText = GetString(IDS_TB_PRIMARYVIEWER);
						break;
					case ID_LAUNCH_SECONDARY_VIEWER:
						lpttt->lpszText = GetString(IDS_TB_SECONDARYVIEWER);
						break;
				}
				break;
			}
#ifdef USE_RICH_EDIT
			case EN_LINK: {
				ENLINK* pLink = (ENLINK *) lParam;
				switch (pLink->msg) {
					case WM_SETCURSOR: {
						HCURSOR hcur;
						if (options.bLinkDoubleClick)
							hcur = LoadCursor(NULL, IDC_ARROW);
						else
							hcur = LoadCursor(hinstThis, MAKEINTRESOURCE(IDC_MYHAND));
						SetCursor(hcur);
						return (LRESULT)1;
					}
/*
					case WM_RBUTTONDOWN:
						bLinkMenu = TRUE;
						break;
					case WM_RBUTTONUP:
						bLinkMenu = FALSE;
						break;
*/
					case WM_LBUTTONUP:
						if (options.bLinkDoubleClick) break;
					case WM_LBUTTONDBLCLK: {
						TEXTRANGE tr;

						tr.chrg.cpMin = pLink->chrg.cpMin;
						tr.chrg.cpMax = pLink->chrg.cpMax;
						tr.lpstrText = (LPTSTR) GlobalAlloc(GPTR, (pLink->chrg.cpMax - pLink->chrg.cpMin + 1) * sizeof(TCHAR));

						SendMessage(client, EM_GETTEXTRANGE, 0, (LPARAM)&tr);

						ShellExecute(NULL, NULL, tr.lpstrText, NULL, NULL, SW_SHOWNORMAL);
						GlobalFree((HGLOBAL)tr.lpstrText);
						break;
					}
				}
			}
#endif
		}
		break;
	}
	case WM_COMMAND:
		switch LOWORD(wParam) {
			case ID_CLIENT:
				switch (HIWORD(wParam)) {
#ifdef USE_RICH_EDIT
					case EN_UPDATE:
						if (!bUpdated) {
							bUpdated = TRUE;
							UpdateStatus();
						}
						break;
#endif
					case EN_CHANGE: {
						if (!bDirtyFile && !bLoading) {
							int nMax = GetWindowTextLength(hwndMain);
							TCHAR szTmp[MAXFN];
							szTmp[0] = ' ';
							szTmp[1] = '*';
							szTmp[2] = ' ';
							GetWindowText(hwndMain, szTmp + 3, nMax + 1);
							SetWindowText(hwndMain, szTmp);
							bDirtyFile = TRUE;
						}
						bHideMessage = TRUE;
						break;
					}
#ifdef USE_RICH_EDIT
					case EN_STOPNOUNDO:
						ERROROUT(GetString(IDS_CANT_UNDO_WARNING));
						break;
#endif
					case EN_ERRSPACE:
					case EN_MAXTEXT:
#ifdef USE_RICH_EDIT
						{
							TCHAR szBuffer[100];
							wsprintf(szBuffer, GetString(IDS_MEMORY_LIMIT), GetWindowTextLength(client));
							ERROROUT(szBuffer);
						}
#else
						if (bLoading) {
							if (options.bAlwaysLaunch || MessageBox(hwnd, GetString(IDS_QUERY_LAUNCH_VIEWER), STR_METAPAD, MB_ICONQUESTION | MB_YESNO) == IDYES) {
								if (options.szBrowser[0] == '\0') {
									MessageBox(hwnd, GetString(IDS_PRIMARY_VIEWER_MISSING), STR_METAPAD, MB_OK|MB_ICONEXCLAMATION);
									SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_VIEW_OPTIONS, 0), 0);
									szFile[0] = '\0';
									break;
								}
								LaunchPrimaryExternalViewer();
							}
							bLoading = FALSE;
							if (!IsWindowVisible(hwnd))
								bQuitApp = TRUE;
						}
						else {
							MessageBox(hwnd, GetString(IDS_LE_MEMORY_LIMIT), STR_METAPAD, MB_ICONEXCLAMATION | MB_OK);
						}
#endif
				}
				break;
			case ID_SAVE_AND_QUIT:
				if (!SaveCurrentFile())
					break;
				options.bQuickExit = TRUE;
			case ID_MYFILE_QUICK_EXIT:
				if (!options.bQuickExit || bLoading)
					break;
			case ID_MYFILE_EXIT:
				SendMessage(hwndMain, WM_CLOSE, 0, 0L);
				break;
			case ID_FIND_NEXT:
				if (szFindText[0] != '\0') {
					SearchFile(szFindText, bMatchCase, FALSE, TRUE, bWholeWord);
					SendMessage(client, EM_SCROLLCARET, 0, 0);
				}
				else {
					bDown = TRUE;
					SendMessage(hwndMain, WM_COMMAND, MAKEWPARAM(ID_FIND, 0), 0);
				}
				break;
			case ID_FIND_PREV_WORD:
			case ID_FIND_NEXT_WORD:
				SelectWord(TRUE, TRUE, TRUE);
				if (szFindText[0] != '\0') {
					SearchFile(szFindText, bMatchCase, FALSE, (LOWORD(wParam) == ID_FIND_NEXT_WORD ? TRUE : FALSE), bWholeWord);
					SendMessage(client, EM_SCROLLCARET, 0, 0);
				}
				break;
			case ID_FIND_PREV:
				if (szFindText[0] != '\0') {
					SearchFile(szFindText, bMatchCase, FALSE, FALSE, bWholeWord);
					SendMessage(client, EM_SCROLLCARET, 0, 0);
				}
				else {
					bDown = FALSE;
					SendMessage(hwndMain, WM_COMMAND, MAKEWPARAM(ID_FIND, 0), 0);
				}
				break;
			case ID_FIND: {
				static FINDREPLACE fr;

				if (hdlgFind) {
					SetFocus(hdlgFind);
					break;
				}

				lstrcpy(szFindText, FindArray[0]);

				SelectWord(TRUE, TRUE, !options.bNoFindAutoSelect);

				ZeroMemory(&fr, sizeof(FINDREPLACE));
				fr.lStructSize = sizeof(FINDREPLACE);
				fr.hwndOwner = hwndMain;
				fr.lpstrFindWhat = szFindText;
				fr.wFindWhatLen = MAXFIND * sizeof(TCHAR);

				fr.hInstance = hinstLang;
				fr.lpTemplateName = MAKEINTRESOURCE(IDD_FIND);
				fr.Flags = FR_ENABLETEMPLATE;

				if (bWholeWord)
					fr.Flags |= FR_WHOLEWORD;
				if (bDown)
					fr.Flags |= FR_DOWN;
				if (bMatchCase)
					fr.Flags |= FR_MATCHCASE;

				hdlgFind = FindText(&fr);

				wpOrigFindProc = (WNDPROC)SetWindowLong(hdlgFind, GWL_WNDPROC, (LONG)FindProc);

				{
					CHARRANGE cr;
					int i;

#ifdef USE_RICH_EDIT
					SendMessage(client, EM_EXGETSEL, 0, (LPARAM)&cr);
#else
					SendMessage(client, EM_GETSEL, (WPARAM)&cr.cpMin, (LPARAM)&cr.cpMax);
#endif
					SendDlgItemMessage(hdlgFind, IDC_CLOSE_AFTER_FIND, BM_SETCHECK, (WPARAM) bCloseAfterFind, 0);

					SetWindowText(GetDlgItem(hdlgFind, 1152), _T("dummy_find"));

					{
						HBITMAP hb = CreateMappedBitmap(hinstThis, IDB_DROP_ARROW, 0, NULL, 0);
						SendDlgItemMessage(hdlgFind, IDC_ESCAPE, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)(HANDLE)hb);
					}

					if (options.bCurrentFindFont) {
						SendDlgItemMessage(hdlgFind, ID_DROP_FIND, WM_SETFONT, (WPARAM)hfontfind, 0);
					}
					SendDlgItemMessage(hdlgFind, ID_DROP_FIND, CB_LIMITTEXT, (WPARAM)MAXFIND, 0);

					for (i = 0; i < NUMFINDS; ++i) {
						if (lstrlen(FindArray[i]))
							SendDlgItemMessage(hdlgFind, ID_DROP_FIND, CB_ADDSTRING, 0, (WPARAM)FindArray[i]);
					}

					if (lstrlen(szFindText)) {
						SendDlgItemMessage(hdlgFind, ID_DROP_FIND, WM_SETTEXT, 0, (LPARAM)szFindText);
						SendDlgItemMessage(hdlgFind, ID_DROP_FIND, CB_SETEDITSEL, 0, MAKELPARAM(0, -1));
					}
				}
				break;
			}
			case ID_REPLACE: {
				static FINDREPLACE fr;

				if (hdlgFind) {
					SetFocus(hdlgFind);
					break;
				}

				lstrcpy(szFindText, FindArray[0]);
				lstrcpy(szReplaceText, ReplaceArray[0]);

				SelectWord(TRUE, TRUE, !options.bNoFindAutoSelect);

				ZeroMemory(&fr, sizeof(FINDREPLACE));
				fr.lStructSize = sizeof(FINDREPLACE);
				fr.hwndOwner = hwndMain;
				fr.lpstrFindWhat = szFindText;
				fr.wFindWhatLen = MAXFIND * sizeof(TCHAR);
				fr.lpstrReplaceWith = szReplaceText;
				fr.wReplaceWithLen = MAXFIND * sizeof(TCHAR);

				fr.hInstance = hinstLang;
				fr.lpTemplateName = MAKEINTRESOURCE(IDD_REPLACE);
				fr.Flags = FR_ENABLETEMPLATE;

				if (bWholeWord)
					fr.Flags |= FR_WHOLEWORD;
				if (bDown)
					fr.Flags |= FR_DOWN;
				if (bMatchCase)
					fr.Flags |= FR_MATCHCASE;


				hdlgFind = ReplaceText(&fr);

				wpOrigFindProc = (WNDPROC)SetWindowLong(hdlgFind, GWL_WNDPROC, (LONG)FindProc);

				{
					CHARRANGE cr;
					int i;
#ifdef USE_RICH_EDIT
					SendMessage(client, EM_EXGETSEL, 0, (LPARAM)&cr);
#else
					SendMessage(client, EM_GETSEL, (WPARAM)&cr.cpMin, (LPARAM)&cr.cpMax);
#endif

					SetWindowText(GetDlgItem(hdlgFind, 1152), _T("dummy_repl"));

					{
						HBITMAP hb = CreateMappedBitmap(hinstThis, IDB_DROP_ARROW, 0, NULL, 0);
						SendDlgItemMessage(hdlgFind, IDC_ESCAPE, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)(HANDLE)hb);
						SendDlgItemMessage(hdlgFind, IDC_ESCAPE2, BM_SETIMAGE, IMAGE_BITMAP, (LPARAM)(HANDLE)hb);
					}

					SendDlgItemMessage(hdlgFind, ID_DROP_FIND, CB_LIMITTEXT, (WPARAM)MAXFIND, 0);
					SendDlgItemMessage(hdlgFind, ID_DROP_REPLACE, CB_LIMITTEXT, (WPARAM)MAXFIND, 0);

					for (i = 0; i < NUMFINDS; ++i) {
						if (lstrlen(FindArray[i]))
							SendDlgItemMessage(hdlgFind, ID_DROP_FIND, CB_ADDSTRING, 0, (LPARAM)FindArray[i]);
						if (lstrlen(ReplaceArray[i]))
							SendDlgItemMessage(hdlgFind, ID_DROP_REPLACE, CB_ADDSTRING, 0, (LPARAM)ReplaceArray[i]);
					}

					if (options.bCurrentFindFont) {
						SendDlgItemMessage(hdlgFind, ID_DROP_FIND, WM_SETFONT, (WPARAM)hfontfind, 0);
						SendDlgItemMessage(hdlgFind, ID_DROP_REPLACE, WM_SETFONT, (WPARAM)hfontfind, 0);
					}
					SendDlgItemMessage(hdlgFind, ID_DROP_REPLACE, CB_SETEDITSEL, 0, 0);

					if (lstrlen(szFindText)) {
						SendDlgItemMessage(hdlgFind, ID_DROP_FIND, WM_SETTEXT, 0, (LPARAM)szFindText);
						SendDlgItemMessage(hdlgFind, ID_DROP_FIND, CB_SETEDITSEL, 0, MAKELPARAM(0, -1));
					}

#ifdef USE_RICH_EDIT
					if (SendMessage(client, EM_EXLINEFROMCHAR, 0, (LPARAM)cr.cpMin) == SendMessage(client, EM_EXLINEFROMCHAR, 0, (LPARAM)cr.cpMax))
#else
					if (SendMessage(client, EM_LINEFROMCHAR, (WPARAM)cr.cpMin, 0) == SendMessage(client, EM_LINEFROMCHAR, (WPARAM)cr.cpMax, 0))
#endif
						SendDlgItemMessage(hdlgFind, IDC_RADIO_WHOLE, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
					else
						SendDlgItemMessage(hdlgFind, IDC_RADIO_SELECTION, BM_SETCHECK, (WPARAM)BST_CHECKED, 0);
				}
				break;
			}
			case ID_MYEDIT_DELETE:
				SendMessage(client, WM_CLEAR, 0, 0);
				UpdateStatus();
				break;
#ifdef USE_RICH_EDIT
			case ID_MYEDIT_REDO:
				SendMessage(client, EM_REDO, 0, 0);
				UpdateStatus();
				break;
#endif
			case ID_MYEDIT_UNDO: {
				/*
				BOOL bOldDirty = FALSE;
				if (bDirtyFile)
					bOldDirty = TRUE;
				*/
				SendMessage(client, EM_UNDO, 0, 0);
				/*
				if (bOldDirty && bDirtyFile && !SendMessage(client, EM_CANUNDO, 0, 0)) {
					TCHAR szBuffer[MAXFN];
					
					bDirtyFile = FALSE;
					GetWindowText(hwndMain, szBuffer, GetWindowTextLength(hwndMain) + 1);
					SetWindowText(hwndMain, szBuffer + 3);
					bLoading = FALSE;
				}
				*/
				UpdateStatus();
				break;
			}
#ifdef USE_RICH_EDIT
			case ID_MYEDIT_CUT:
			case ID_MYEDIT_COPY:
				{
#if TRUE // new cut/copy (for chinese crash)
					HGLOBAL hMem, hMem2;
					LPTSTR szOrig, szNew;

					if (LOWORD(wParam) == ID_MYEDIT_CUT) {
						SendMessage(client, WM_CUT, 0, 0);
					}
					else {
						SendMessage(client, WM_COPY, 0, 0);
					}

					if (!OpenClipboard(hwnd)) {
						// unknown error - used to popup a message here
						// but removed it since it was happening spuriously
						// and did not actually affect the copy (multiple incoming copy msgs?)
						break;
					}
					else {
						hMem = GetClipboardData(CF_TEXT);
						if (hMem) {
							DWORD dwLastError;
							szOrig = GlobalLock(hMem);
				
							hMem2 = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, sizeof(TCHAR) * (lstrlen(szOrig) + 2));
							szNew = GlobalLock(hMem2);
							if (szNew && szOrig) {
								lstrcpy(szNew, szOrig);
							}
							else {
								ERROROUT(GetString(IDS_GLOBALLOCK_ERROR));
								break;
							}
							SetLastError(NO_ERROR);
							if (!GlobalUnlock(hMem) && (dwLastError = GetLastError()) != NO_ERROR) {
								ERROROUT(GetString(IDS_CLIPBOARD_UNLOCK_ERROR));
								SetLastError(dwLastError);
								ReportLastError();
								break;
							}
							if (!GlobalUnlock(hMem2) && (dwLastError = GetLastError()) != NO_ERROR) {
								ERROROUT(GetString(IDS_CLIPBOARD_UNLOCK_ERROR));
								SetLastError(dwLastError);
								ReportLastError();
								break;
							}
							if (!EmptyClipboard()) {
								ReportLastError();
								break;
							}
							if (!SetClipboardData(CF_TEXT, hMem2)) {
								ReportLastError();
								break;
							}
						}
						CloseClipboard();
					}
#else // old cut/copy

					HGLOBAL hMem;
					LPTSTR strTmp;
					LPTSTR szSrc;
					CHARRANGE cr;
					DWORD dwLastError;

					SendMessage(client, EM_EXGETSEL, 0, (LPARAM)&cr);
					if (cr.cpMin == cr.cpMax) break;
/*
{
	TCHAR str[100];
	wsprintf(str, "Selection length = %d", cr.cpMax - cr.cpMin);
	ERROROUT(str);
}
*/
					szSrc = (LPTSTR) GlobalAlloc(GPTR, (cr.cpMax - cr.cpMin + 1) * sizeof(TCHAR));
					SendMessage(client, EM_GETSELTEXT, 0, (LPARAM)szSrc);

//DBGOUT(szSrc, "original string");
					RichModeToDos(&szSrc);

//DBGOUT(szSrc, "modified string");

					hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_DDESHARE, sizeof(TCHAR) * (lstrlen(szSrc) + 2));
					strTmp = GlobalLock(hMem);
					if (strTmp) {
						lstrcpy(strTmp, szSrc);
					}
					else {
						ERROROUT(GetString(IDS_GLOBALLOCK_ERROR));
						break;
					}

//DBGOUT(strTmp, "adding to clipboard");

/*
{
	TCHAR str[100];
	wsprintf(str, "hMem = %d", hMem);
	ERROROUT(str);
}
*/
					SetLastError(NO_ERROR);

					if (!GlobalUnlock(hMem) && (dwLastError = GetLastError()) != NO_ERROR) {
						ERROROUT(GetString(IDS_CLIPBOARD_UNLOCK_ERROR));
						SetLastError(dwLastError);
						ReportLastError();
						break;
					}
					
					if (OpenClipboard(NULL)) {
						EmptyClipboard();
						SetClipboardData(CF_TEXT, hMem);
						CloseClipboard();
					}
					else {
						ERROROUT(GetString(IDS_CLIPBOARD_OPEN_ERROR));
					}

					GlobalFree((HGLOBAL)szSrc);
/*
{
	HGLOBAL hMem;
	LPTSTR szTmp;
	OpenClipboard(NULL);
	hMem = GetClipboardData(CF_TEXT);
	if (hMem) {
		szTmp = GlobalLock(hMem);
		DBGOUT(szTmp, "clipboard contents");
		GlobalUnlock(hMem);
	}
	CloseClipboard();
}
*/
					if (LOWORD(wParam) == ID_MYEDIT_CUT) {
						SendMessage(client, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)NULL);
						InvalidateRect(client, NULL, TRUE);
					}
#endif
				}
				UpdateStatus();
				break;
#else
			case ID_MYEDIT_CUT:
				SendMessage(client, WM_CUT, 0, 0);
				UpdateStatus();
				break;
			case ID_MYEDIT_COPY:
				SendMessage(client, WM_COPY, 0, 0);
				UpdateStatus();
				break;
#endif
			case ID_MYEDIT_PASTE: {
#ifdef USE_RICH_EDIT
				HGLOBAL hMem;
				LPTSTR szTmp;
				OpenClipboard(NULL);
				hMem = GetClipboardData(CF_TEXT);
				if (hMem) {
					szTmp = GlobalLock(hMem);
					SendMessage(client, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)szTmp);
					GlobalUnlock(hMem);
				}
				CloseClipboard();
				InvalidateRect(client, NULL, TRUE);
#else
				SendMessage(client, WM_PASTE, 0, 0);
#endif
				UpdateStatus();
				break;
			}
			case ID_HOME:
				{
					LONG lStartLine, lStartLineIndex, lLineLen, i;
					CHARRANGE cr;
					TCHAR* szTemp;

					if (options.bNoSmartHome) {
						SendMessage(client, WM_KEYDOWN, (WPARAM)VK_HOME, 0);
						SendMessage(client, WM_KEYUP, (WPARAM)VK_HOME, 0);
						break;
					}

#ifdef USE_RICH_EDIT
					SendMessage(client, EM_EXGETSEL, 0, (LPARAM)&cr);
					lStartLine = SendMessage(client, EM_EXLINEFROMCHAR, 0, (LPARAM)cr.cpMin);
#else
					SendMessage(client, EM_GETSEL, (WPARAM)&cr.cpMin, (LPARAM)&cr.cpMax);
					lStartLine = SendMessage(client, EM_LINEFROMCHAR, (WPARAM)cr.cpMin, 0);
#endif
					lStartLineIndex = SendMessage(client, EM_LINEINDEX, (WPARAM)lStartLine, 0);
					lLineLen = SendMessage(client, EM_LINELENGTH, (WPARAM)lStartLineIndex, 0);
					
					szTemp = (LPTSTR) GlobalAlloc(GPTR, (lLineLen+2) * sizeof(TCHAR));
					*((LPWORD)szTemp) = (USHORT)(lLineLen + 1);
					SendMessage(client, EM_GETLINE, (WPARAM)lStartLine, (LPARAM)(LPCTSTR)szTemp);
					szTemp[lLineLen] = '\0';

					for (i = 0; i < lLineLen; ++i)
						if (szTemp[i] != '\t' && szTemp[i] != ' ')
							break;
					
					if (cr.cpMin - lStartLineIndex == i)
						cr.cpMin = cr.cpMax = lStartLineIndex;
					else
						cr.cpMin = cr.cpMax = lStartLineIndex + i;

#ifdef USE_RICH_EDIT
					SendMessage(client, EM_EXSETSEL, 0, (LPARAM)&cr);
#else
					SendMessage(client, EM_SETSEL, (WPARAM)cr.cpMin, (LPARAM)cr.cpMax);
#endif
					GlobalFree((HGLOBAL)szTemp);
					SendMessage(client, EM_SCROLLCARET, 0, 0);
				}
				break;
			case ID_MYEDIT_SELECTALL: {
				CHARRANGE cr;
				cr.cpMin = 0;
				cr.cpMax = -1;

#ifdef USE_RICH_EDIT
				SendMessage(client, EM_EXSETSEL, 0, (LPARAM)&cr);
#else
				SendMessage(client, EM_SETSEL, (WPARAM)cr.cpMin, (LPARAM)cr.cpMax);
#endif
				UpdateStatus();
				break;
			}
#ifdef USE_RICH_EDIT
			case ID_SHOWHYPERLINKS:
				{
					CHARRANGE cr;
					HCURSOR hcur;

					if (SendMessage(client, EM_CANUNDO, 0, 0)) {
						if (!options.bSuppressUndoBufferPrompt && MessageBox(hwnd, GetString(IDS_UNDO_HYPERLINKS_WARNING), STR_METAPAD, MB_ICONEXCLAMATION | MB_OKCANCEL) == IDCANCEL) {
							break;
						}
					}
					hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));
					bHyperlinks = !GetCheckedState(GetMenu(hwndMain), ID_SHOWHYPERLINKS, TRUE);
					SendMessage(client, EM_AUTOURLDETECT, (WPARAM)bHyperlinks, 0);

					SendMessage(client, WM_SETREDRAW, (WPARAM)FALSE, 0);
					SendMessage(client, EM_EXGETSEL, 0, (LPARAM)&cr);
					UpdateWindowText();
					SendMessage(client, EM_EXSETSEL, 0, (LPARAM)&cr);
					SendMessage(client, WM_SETREDRAW, (WPARAM)TRUE, 0);
					SetCursor(hcur);
				}
				break;
#endif
			case ID_SMARTSELECT:
				bSmartSelect = !GetCheckedState(GetMenu(hwndMain), ID_SMARTSELECT, TRUE);
				break;
			case ID_ALWAYSONTOP:
				bAlwaysOnTop = !GetCheckedState(GetMenu(hwndMain), ID_ALWAYSONTOP, TRUE);
				SetWindowPos(hwnd, (bAlwaysOnTop ? HWND_TOPMOST : HWND_NOTOPMOST), 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
				UpdateStatus();
				break;
			case ID_TRANSPARENT: {
				if (SetLWA) {
					bTransparent = !GetCheckedState(GetMenu(hwndMain), ID_TRANSPARENT, TRUE);
					if (bTransparent) {
						SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
						SetLWA(hwnd, 0, (BYTE)((255 * (100 - options.nTransparentPct)) / 100), LWA_ALPHA);
					}
					else {
						SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) & ~WS_EX_LAYERED);
						RedrawWindow(hwnd, NULL, NULL, RDW_UPDATENOW | RDW_ALLCHILDREN | RDW_ERASE | RDW_INVALIDATE | RDW_FRAME);
					}
				}
				break;
			}
			case ID_SHOWTOOLBAR: {
				RECT rect;

				if (!IsWindow(toolbar))
					CreateToolbar();

				bShowToolbar = !GetCheckedState(GetMenu(hwndMain), ID_SHOWTOOLBAR, TRUE);
				if (bShowToolbar) {
					UpdateStatus();
					ShowWindow(toolbar, SW_SHOW);
				}
				else
					ShowWindow(toolbar, SW_HIDE);

				SendMessage(toolbar, WM_SIZE, 0, 0);
				GetClientRect(hwndMain, &rect);
				SetWindowPos(client, 0, 0, bShowToolbar ? GetToolbarHeight() : rect.top - GetToolbarHeight(), rect.right - rect.left, rect.bottom - rect.top - GetStatusHeight() - GetToolbarHeight(), SWP_NOZORDER | SWP_SHOWWINDOW);
				break;
			}
			case ID_INSERT_FILE: {
				OPENFILENAME ofn;
				TCHAR szDefExt[] = _T("txt");
				TCHAR szFilename[MAXFN] = _T("");

				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = client;
				ofn.lpstrFilter = szCustomFilter;
				ofn.lpstrCustomFilter = (LPTSTR)NULL;
				ofn.nMaxCustFilter = 0L;
				ofn.nFilterIndex = 1L;
				ofn.lpstrFile = szFilename;
				ofn.nMaxFile = sizeof(szFilename);
				ofn.lpstrFileTitle = (LPTSTR)NULL;
				ofn.nMaxFileTitle = 0L;
				ofn.lpstrInitialDir = szDir;
				ofn.lpstrTitle = (LPTSTR)NULL;
				ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
				ofn.nFileOffset = 0;
				ofn.nFileExtension = 0;
				ofn.lpstrDefExt = szDefExt;

				if (GetOpenFileName(&ofn)) {
					HANDLE hFile = NULL;
					ULONG lBufferLength;
					PBYTE pBuffer = NULL;
					DWORD dwActualBytesRead;
					HCURSOR hcur;
					INT nFileEncoding;

					hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

					hFile = (HANDLE)CreateFile(szFilename, GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
					if (hFile == INVALID_HANDLE_VALUE) {
						ERROROUT(GetString(IDS_FILE_READ_ERROR));
						goto endinsertfile;
					}

					dwActualBytesRead = LoadFileIntoBuffer(hFile, &pBuffer, &lBufferLength, &nFileEncoding);
#ifndef BUILD_METAPAD_UNICODE
					if (memchr((const void*)pBuffer, '\0', lBufferLength) != NULL) {
						if (options.bNoWarningPrompt || MessageBox(hwnd, GetString(IDS_BINARY_FILE_WARNING), STR_METAPAD, MB_ICONQUESTION|MB_YESNO) == IDYES) {
							UINT i;
							for (i = 0; i < lBufferLength; i++) {
								if (pBuffer[i] == '\0')
									pBuffer[i] = ' ';
							}
						}
						else goto endinsertfile;
					}
#endif

					if (nFileEncoding != TYPE_UTF_16 && nFileEncoding != TYPE_UTF_16_BE) {
#ifdef USE_RICH_EDIT
						FixTextBuffer((LPTSTR)pBuffer);
#else
						FixTextBufferLE((LPTSTR*)&pBuffer);
#endif
					}
					SendMessage(client, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)(LPTSTR)pBuffer);
#ifdef USE_RICH_EDIT
					InvalidateRect(client, NULL, TRUE);
#endif
endinsertfile:
					CloseHandle(hFile);
					if (pBuffer) GlobalFree((HGLOBAL) pBuffer);
					SetCursor(hcur);
				}
				break;
			}
			case ID_SHOWSTATUS: {
				RECT rect;

				if (!IsWindow(status))
					CreateStatusbar();

				bShowStatus = !GetCheckedState(GetMenu(hwndMain), ID_SHOWSTATUS, TRUE);
				if (bShowStatus) {
					UpdateStatus();
					ShowWindow(status, SW_SHOW);
				}
				else
					ShowWindow(status, SW_HIDE);

				GetClientRect(hwndMain, &rect);
				SetWindowPos(client, 0, 0, GetToolbarHeight(), rect.right - rect.left, rect.bottom - rect.top - GetStatusHeight() - GetToolbarHeight(), SWP_NOMOVE | SWP_NOZORDER | SWP_SHOWWINDOW);
				SendMessage(status, WM_SIZE, 0, 0);
				break;
			}
			case ID_READONLY:
				{
					TCHAR szTmp[MAXFN];
					int nRes;
						
					if (!options.bReadOnlyMenu) break;

					bReadOnly = !GetCheckedState(GetMenu(hwndMain), ID_READONLY, FALSE);
					if (bReadOnly)
						nRes = SetFileAttributes(szFile, GetFileAttributes(szFile) | FILE_ATTRIBUTE_READONLY);
					else
						nRes = SetFileAttributes(szFile, ((GetFileAttributes(szFile) & ~FILE_ATTRIBUTE_READONLY) == 0 ? FILE_ATTRIBUTE_NORMAL : GetFileAttributes(szFile) & ~FILE_ATTRIBUTE_READONLY));

					if (nRes == 0) {
						DWORD dwError = GetLastError();
						if (dwError == ERROR_ACCESS_DENIED) {
							ERROROUT(GetString(IDS_CHANGE_READONLY_ERROR));
						}
						else {
							ReportLastError();
						}
						bReadOnly = !bReadOnly;
						break;
					}
					
					GetWindowText(hwndMain, szTmp, GetWindowTextLength(hwndMain) + 1);
					
					if (bReadOnly) {
						lstrcat(szTmp, _T(" "));
						lstrcat(szTmp, GetString(IDS_READONLY_INDICATOR));
						SetWindowText(hwndMain, szTmp);
					}
					else
						//szTmp[GetWindowTextLength(hwndMain) - 12] = '\0';
						UpdateCaption();

					SwitchReadOnly(bReadOnly);

					UpdateStatus();
				}
				break;
			case ID_FONT_PRIMARY:
				bPrimaryFont = !GetCheckedState(GetMenu(hwndMain), ID_FONT_PRIMARY, TRUE);
				if (!SetClientFont(bPrimaryFont)) {
					GetCheckedState(GetMenu(hwndMain), ID_FONT_PRIMARY, TRUE);
					bPrimaryFont = !bPrimaryFont;
				}
#ifndef USE_RICH_EDIT
				UpdateStatus();
#endif
				break;
			case ID_VIEW_OPTIONS:
				{
					PROPSHEETHEADER psh;
					PROPSHEETPAGE pages[4];

					ZeroMemory(&pages[0], sizeof(pages[0]));
					pages[0].dwSize = sizeof(PROPSHEETPAGE);
					//pages[0].dwSize = PROPSHEETPAGE_V1_SIZE;

					pages[0].dwFlags = PSP_DEFAULT;
					pages[0].hInstance = hinstLang;
					pages[0].pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_GENERAL);
					pages[0].pfnDlgProc = (DLGPROC)GeneralPageProc;

					ZeroMemory(&pages[1], sizeof(pages[1]));
					pages[1].dwSize = sizeof(PROPSHEETPAGE);
					//pages[1].dwSize = PROPSHEETPAGE_V1_SIZE;

					pages[1].hInstance = hinstLang;
					pages[1].dwFlags = PSP_DEFAULT;
					pages[1].pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_VIEW);
					pages[1].pfnDlgProc = (DLGPROC)ViewPageProc;

					ZeroMemory(&pages[2], sizeof(pages[2]));
					pages[2].dwSize = sizeof(PROPSHEETPAGE);
					//pages[2].dwSize = PROPSHEETPAGE_V1_SIZE;

					pages[2].hInstance = hinstLang;
					pages[2].dwFlags = PSP_DEFAULT;
					pages[2].pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_A2);
					pages[2].pfnDlgProc = (DLGPROC)Advanced2PageProc;

					ZeroMemory(&pages[3], sizeof(pages[3]));
					pages[3].dwSize = sizeof(PROPSHEETPAGE);
					//pages[3].dwSize = PROPSHEETPAGE_V1_SIZE;

					pages[3].hInstance = hinstLang;
					pages[3].dwFlags = PSP_DEFAULT;
#ifdef USE_RICH_EDIT
					pages[3].pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_A1);
#else
					pages[3].pszTemplate = MAKEINTRESOURCE(IDD_PROPPAGE_A1_LE);
#endif
					pages[3].pfnDlgProc = (DLGPROC)AdvancedPageProc;

					ZeroMemory(&psh, sizeof(psh));
					psh.dwSize = sizeof(PROPSHEETHEADER);

					//psh.dwSize = PROPSHEETHEADER_V1_SIZE;

					psh.dwFlags = PSH_USECALLBACK | PSH_NOAPPLYNOW | PSH_PROPSHEETPAGE;
					psh.hwndParent = hwndMain;
					psh.nPages = 4;
					psh.pszCaption = GetString(IDS_SETTINGS_TITLE);
					psh.ppsp = (LPCPROPSHEETPAGE) pages;
					psh.pfnCallback = SheetInitProc;

					{
						int retval;
/*
						BOOL bOldMRU = options.bRecentOnOwn;
						int nOldFontOption, nOldTabs = options.nTabStops;
						LOGFONT oldFont;
						BOOL bOldReadOnlyMenu = options.bReadOnlyMenu;
						BOOL bOldFlatToolbar = options.bUnFlatToolbar;
						BOOL bOldSystemColours = options.bSystemColours;
						BOOL bOldSystemColours2 = options.bSystemColours2;
						BOOL bOldNoFaves = options.bNoFaves;
#ifdef USE_RICH_EDIT
						BOOL bOldHideScrollbars = options.bHideScrollbars;
#endif
						COLORREF oldBackColour = options.BackColour, oldFontColour = options.FontColour;
						COLORREF oldBackColour2 = options.BackColour2, oldFontColour2 = options.FontColour2;
						TCHAR szOldLangPlugin[MAXFN];

						lstrcpy(szOldLangPlugin, options.szLangPlugin);
*/
						option_struct oldOptions;
						memcpy(&oldOptions, &options, sizeof(option_struct));
						LoadOptions();
/*
						if (bPrimaryFont) {
							nOldFontOption = options.nPrimaryFont;
							oldFont = options.PrimaryFont;
						}
						else {
							nOldFontOption = options.nSecondaryFont;
							oldFont = options.SecondaryFont;
						}
*/
						retval = PropertySheet(&psh);
						if (retval) {
							SaveOptions();

							if (options.bLaunchClose && options.nLaunchSave == 2) {
								MessageBox(hwndMain, GetString(IDS_LAUNCH_WARNING), STR_METAPAD, MB_ICONEXCLAMATION);
							}

							if (options.bReadOnlyMenu != oldOptions.bReadOnlyMenu)
								FixReadOnlyMenu();

							if (options.bRecentOnOwn != oldOptions.bRecentOnOwn) {
								FixMRUMenus();
								PopulateMRUList();
							}
							
							if (oldOptions.bUnFlatToolbar != options.bUnFlatToolbar) {
								DestroyWindow(toolbar);
								CreateToolbar();
							}								

							if ((memcmp((LPVOID)(LPVOID)(bPrimaryFont ? &oldOptions.PrimaryFont : &oldOptions.SecondaryFont), (LPVOID)(bPrimaryFont ? &options.PrimaryFont : &options.SecondaryFont), sizeof(LOGFONT)) != 0) ||
								(oldOptions.nTabStops != options.nTabStops) ||
								((bPrimaryFont && oldOptions.nPrimaryFont != options.nPrimaryFont) || (!bPrimaryFont && oldOptions.nSecondaryFont != options.nSecondaryFont)) ||
								(oldOptions.bSystemColours != options.bSystemColours) ||
								(oldOptions.bSystemColours2 != options.bSystemColours2) ||
								(memcmp((LPVOID)&oldOptions.BackColour, (LPVOID)&options.BackColour, sizeof(COLORREF)) != 0) ||
								(memcmp((LPVOID)&oldOptions.FontColour, (LPVOID)&options.FontColour, sizeof(COLORREF)) != 0) ||
								(memcmp((LPVOID)&oldOptions.BackColour2, (LPVOID)&options.BackColour2, sizeof(COLORREF)) != 0) ||
								(memcmp((LPVOID)&oldOptions.FontColour2, (LPVOID)&options.FontColour2, sizeof(COLORREF)) != 0))
								if (!SetClientFont(bPrimaryFont)) {
									options.nTabStops = oldOptions.nTabStops;
									if (bPrimaryFont) {
										options.nPrimaryFont = oldOptions.nPrimaryFont;
										memcpy((LPVOID)&options.PrimaryFont, (LPVOID)&oldOptions.PrimaryFont, sizeof(LOGFONT));
									}
									else {
										options.nSecondaryFont = oldOptions.nSecondaryFont;
										memcpy((LPVOID)&options.SecondaryFont, (LPVOID)&oldOptions.SecondaryFont, sizeof(LOGFONT));
									}
								}

							if (szFile[0] != '\0') {
								UpdateCaption();
							}

							SendMessage(client, EM_SETMARGINS, (WPARAM)EC_LEFTMARGIN, MAKELPARAM(options.nSelectionMarginWidth, 0));

							if (bTransparent) {
								SetLWA(hwnd, 0, (BYTE)((255 * (100 - options.nTransparentPct)) / 100), LWA_ALPHA);
							}
#ifdef USE_RICH_EDIT
							if (oldOptions.bHideScrollbars != options.bHideScrollbars) {
								ERROROUT(GetString(IDS_RESTART_HIDE_SB));
							}
#endif
							if (oldOptions.bNoFaves != options.bNoFaves) {
								ERROROUT(GetString(IDS_RESTART_FAVES));
							}

							if (lstrcmp(oldOptions.szLangPlugin, options.szLangPlugin) != 0) {
								ERROROUT(GetString(IDS_RESTART_LANG));
							}

							PopulateMRUList();
							UpdateStatus();
						}
						else if (retval < 0) {
							ReportLastError();
						}
					}
					break;
				}
			case ID_HELP_ABOUT:
				DialogBox(hinstThis, MAKEINTRESOURCE(IDD_ABOUT), hwndMain, (DLGPROC)AboutDialogProc);
				break;
			case ID_ABOUT_PLUGIN:
				if (hinstThis != hinstLang)
					DialogBox(hinstLang, MAKEINTRESOURCE(IDD_ABOUT_PLUGIN), hwndMain, (DLGPROC)AboutPluginDialogProc);
				break;
			case ID_MYFILE_OPEN: {
				OPENFILENAME ofn;
				TCHAR szDefExt[] = _T("txt");
				TCHAR szTmp[MAXFN] = _T("");

				SetCurrentDirectory(szDir);
				if (!SaveIfDirty())
					break;

				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = client;
				ofn.lpstrFilter = szCustomFilter;
				ofn.lpstrCustomFilter = (LPTSTR)NULL;
				ofn.nMaxCustFilter = 0L;
				ofn.nFilterIndex = 1L;
				ofn.lpstrFile = szTmp;
				ofn.nMaxFile = sizeof(szTmp);
				ofn.lpstrFileTitle = (LPTSTR)NULL;
				ofn.nMaxFileTitle = 0L;
				ofn.lpstrInitialDir = szDir;
				ofn.lpstrTitle = (LPTSTR)NULL;
				ofn.Flags = OFN_HIDEREADONLY | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
				ofn.nFileOffset = 0;
				ofn.nFileExtension = 0;
				ofn.lpstrDefExt = szDefExt;

				if (GetOpenFileName(&ofn)) {
					GetCurrentDirectory(MAXFN, szDir);
					bLoading = TRUE;
					bHideMessage = FALSE;
					lstrcpy(szStatusMessage, GetString(IDS_FILE_LOADING));
					UpdateStatus();
					lstrcpy(szFile, szTmp);
					LoadFile(szFile, FALSE, TRUE);
					if (bLoading) {
						/*
						bLoading = FALSE;
						ExpandFilename(szFile);
						wsprintf(szTmp, STR_CAPTION_FILE, szCaptionFile);
						SetWindowText(hwndMain, szTmp);
						bDirtyFile = FALSE;
						*/
						bLoading = FALSE;
						bDirtyFile = FALSE;
						UpdateCaption();
					}
					else {
						MakeNewFile();
					}
				}
				UpdateStatus();
				break;
			}
			case ID_MYFILE_NEW: {
				if (!SaveIfDirty())
					break;
				MakeNewFile();
				break;
			}
			case ID_EDIT_SELECTWORD: {
				CHARRANGE cr;

#ifdef USE_RICH_EDIT
				SendMessage(client, EM_EXGETSEL, 0, (LPARAM)&cr);
				cr.cpMin = cr.cpMax;
				SendMessage(client, EM_EXSETSEL, 0, (LPARAM)&cr);
#else
				SendMessage(client, EM_GETSEL, (WPARAM)&cr.cpMin, (LPARAM)&cr.cpMax);
				cr.cpMin = cr.cpMax;
				SendMessage(client, EM_SETSEL, (WPARAM)cr.cpMin, (LPARAM)cr.cpMax);
#endif
				SelectWord(FALSE, bSmartSelect, TRUE);
				break;
			}
			case ID_GOTOLINE: {
				DialogBox(hinstLang, MAKEINTRESOURCE(IDD_GOTO), hwndMain, (DLGPROC)GotoDialogProc);
				break;
			}
			case ID_EDIT_WORDWRAP:
				{
					HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));
#ifdef USE_RICH_EDIT
					bWordWrap = !GetCheckedState(GetMenu(hwndMain), ID_EDIT_WORDWRAP, TRUE);
					SendMessage(client, EM_SETTARGETDEVICE, (WPARAM)0, (LPARAM)(LONG) !bWordWrap);

					SendMessage(client, WM_HSCROLL, (WPARAM)SB_LEFT, 0);
					SendMessage(client, EM_SCROLLCARET, 0, 0);

					if (bWordWrap)
						SetWindowLong(client, GWL_STYLE, GetWindowLong(client, GWL_STYLE) & ~WS_HSCROLL);
					else if (!options.bHideScrollbars)
						SetWindowLong(client, GWL_STYLE, GetWindowLong(client, GWL_STYLE) | WS_HSCROLL);
					
					SetWindowPos(client, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
#else
					LONG lFileSize = GetWindowTextLength(client);
					LPTSTR szBuffer;
					RECT rect;
					CHARRANGE cr;

					szBuffer = (LPTSTR) GlobalAlloc(GPTR, (lFileSize+1) * sizeof(TCHAR));

					bWordWrap = !GetCheckedState(GetMenu(hwnd), ID_EDIT_WORDWRAP, TRUE);

					SendMessage(client, EM_GETSEL, (WPARAM)&cr.cpMin, (LPARAM)&cr.cpMax);

					GetWindowText(client, szBuffer, lFileSize+1);
					if (!DestroyWindow(client))
						ReportLastError();

					CreateClient(hwnd, _T(""), bWordWrap);
					
					SetWindowText(client, szBuffer);

					SendMessage(client, EM_SETSEL, (WPARAM)cr.cpMin, (LPARAM)cr.cpMax);

					GlobalFree((HGLOBAL)szBuffer);
					SetClientFont(bPrimaryFont);

					GetClientRect(hwnd, &rect);
					SetWindowPos(client, 0, 0, bShowToolbar ? GetToolbarHeight() : rect.top - GetToolbarHeight(), rect.right - rect.left, rect.bottom - rect.top - GetStatusHeight() - GetToolbarHeight(), SWP_NOZORDER | SWP_SHOWWINDOW);
					SetFocus(client);
					SendMessage(client, EM_SCROLLCARET, 0, 0);
#endif
					SetCursor(hcur);
					UpdateStatus();
					break;
				}
			case ID_FILE_LAUNCHVIEWER:
				LaunchInViewer(TRUE, FALSE);
				break;
			case ID_LAUNCH_SECONDARY_VIEWER:
				LaunchInViewer(TRUE, TRUE);
				break;
			case ID_MYFILE_SAVE:
				SaveCurrentFile();
				break;
			case ID_MYFILE_SAVEAS:
				SaveCurrentFileAs();
				break;
			/*
			case ID_RUN_HELP:
				{
					TCHAR* pch;
					TCHAR buf[200];

					GetModuleFileName(hinst, buf, MAXFN);
					
					pch = _tcsrchr(buf, _T('\\'));
					++pch;
					*pch = '\0';

					lstrcat(buf, "metapad.chm");

					ShellExecute(NULL, NULL, buf, NULL, szDir, SW_SHOWNORMAL);
				}
				break;
			*/
			case ID_STRIPCHAR: 
			case ID_INDENT:
			case ID_UNINDENT: {
				LONG lStartLine, lEndLine, lStartLineIndex, lEndLineIndex, lMaxLine, lEndLineLen;
				BOOL bIndent = LOWORD(wParam) == ID_INDENT;
				LPTSTR szPrefix = (LPTSTR) GlobalAlloc(GPTR, (options.nTabStops+1) * sizeof(TCHAR));
				BOOL bEnd = FALSE;
				BOOL bStrip = LOWORD(wParam) == ID_STRIPCHAR;
				CHARRANGE cr;
				BOOL bRewrap = FALSE;

#ifdef USE_RICH_EDIT
				SendMessage(client, EM_EXGETSEL, 0, (LPARAM)&cr);
				lStartLine = SendMessage(client, EM_EXLINEFROMCHAR, 0, (LPARAM)cr.cpMin);
				lEndLine = SendMessage(client, EM_EXLINEFROMCHAR, 0, (LPARAM)cr.cpMax);
#else
				SendMessage(client, EM_GETSEL, (WPARAM)&cr.cpMin, (LPARAM)&cr.cpMax);
				lStartLine = SendMessage(client, EM_LINEFROMCHAR, (WPARAM)cr.cpMin, 0);
				lEndLine = SendMessage(client, EM_LINEFROMCHAR, (WPARAM)cr.cpMax, 0);
#endif

				if (bWordWrap && !bStrip && lStartLine != lEndLine) {
					SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_EDIT_WORDWRAP, 0), 0);
#ifdef USE_RICH_EDIT
					SendMessage(client, EM_EXGETSEL, 0, (LPARAM)&cr);
					lStartLine = SendMessage(client, EM_EXLINEFROMCHAR, 0, (LPARAM)cr.cpMin);
					lEndLine = SendMessage(client, EM_EXLINEFROMCHAR, 0, (LPARAM)cr.cpMax);
#else
					SendMessage(client, EM_GETSEL, (WPARAM)&cr.cpMin, (LPARAM)&cr.cpMax);
					lStartLine = SendMessage(client, EM_LINEFROMCHAR, (WPARAM)cr.cpMin, 0);
					lEndLine = SendMessage(client, EM_LINEFROMCHAR, (WPARAM)cr.cpMax, 0);
#endif
					bRewrap = TRUE;
					//ERROROUT(_T("This function will not work with word wrap enabled."));
					//break;
				}
				
				lStartLineIndex = SendMessage(client, EM_LINEINDEX, (WPARAM)lStartLine, 0);
				lEndLineIndex = SendMessage(client, EM_LINEINDEX, (WPARAM)lEndLine, 0);
				lMaxLine = SendMessage(client, EM_GETLINECOUNT, 0, 0) - 1;

				if (options.bInsertSpaces) {
					memset(szPrefix, ' ', options.nTabStops);
					if (cr.cpMin == cr.cpMax) {
						szPrefix[options.nTabStops - (cr.cpMax - lEndLineIndex) % options.nTabStops] = '\0';
					}
					else
						szPrefix[options.nTabStops] = '\0';
				}
				else {
					lstrcpy(szPrefix, _T("\t"));
				}
				
				lEndLineLen = SendMessage(client, EM_LINELENGTH, (WPARAM)lEndLineIndex, 0);
				if ((lEndLine == lMaxLine) && (cr.cpMin != cr.cpMax) && (cr.cpMax != lEndLineIndex) && 
					((lEndLine != lStartLine) || (cr.cpMin == lEndLineIndex && (cr.cpMax - cr.cpMin - 1) == lEndLineLen))) {
					bEnd = TRUE;
				}
				if (lStartLine != lEndLine || bEnd || bStrip) {
					ULONG lFileSize = GetWindowTextLength(client);
					LPTSTR szBuffer;
					int i, j = 0, diff = 0;
					LPTSTR szTemp;
					LONG lLineLen, iIndex, lBefore;
					if (lEndLineIndex != cr.cpMax || (bStrip && cr.cpMin == cr.cpMax && lEndLineIndex == cr.cpMax)) {
						long lTmp = SendMessage(client, EM_LINEINDEX, (WPARAM)lEndLine+1, 0);
						lEndLineIndex = lTmp;
						++lEndLine;
					}
					szBuffer = (LPTSTR) GlobalAlloc(GPTR, (1 + lFileSize + (lMaxLine + 1) * options.nTabStops) * sizeof(TCHAR));
					szBuffer[0] = '\0';

					for (i = lStartLine; i < lEndLine; i++) {
						iIndex = SendMessage(client, EM_LINEINDEX, (WPARAM)i, 0);
						lLineLen = SendMessage(client, EM_LINELENGTH, (WPARAM)iIndex, 0);

						diff = 0;
						szTemp = (LPTSTR) GlobalAlloc(GPTR, (lLineLen+2) * sizeof(TCHAR));
						*((LPWORD)szTemp) = (USHORT)(lLineLen + 1); // + 1 to fix weird xp bug (skipping length 1 lines)!!
						SendMessage(client, EM_GETLINE, (WPARAM)i, (LPARAM)(LPCTSTR)szTemp);
						szTemp[lLineLen] = '\0';
						if (bIndent) {
							lstrcat(szBuffer, szPrefix);
							lstrcat(szBuffer, szTemp);
							diff = -(lstrlen(szPrefix));
						}
						else {
							if ((bStrip && lLineLen > 0) || szTemp[0] == '\t') {
								diff = 1;
							}
							else if (szTemp[0] == ' ') {
								diff = 1;
								while (szTemp[diff] == ' ' && diff < options.nTabStops)
									diff++;
							}
							lstrcpy(szBuffer+j, szTemp + diff);
						}
						if (bEnd && i == lEndLine - 1) {
							j += lLineLen - diff;
						}
						else {
							szBuffer[lLineLen+j-diff] = '\r';
							szBuffer[lLineLen+j+1-diff] = '\n';
							j += lLineLen + 2 - diff;
						}
						GlobalFree((HGLOBAL)szTemp);
					}

					lBefore = SendMessage(client, EM_GETLINECOUNT, 0, 0);
					cr.cpMin = lStartLineIndex;
					cr.cpMax = lEndLineIndex;
#ifdef USE_RICH_EDIT
					SendMessage(client, EM_EXSETSEL, 0, (LPARAM)&cr);
#else
					SendMessage(client, EM_SETSEL, (WPARAM)cr.cpMin, (LPARAM)cr.cpMax);
#endif
					SendMessage(client, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)szBuffer);
					if (SendMessage(client, EM_GETLINECOUNT, 0, 0) > lBefore) {
						SendMessage(client, EM_UNDO, 0, 0);							
					}
					else {
						lEndLineIndex = SendMessage(client, EM_LINEINDEX, (WPARAM)lEndLine, 0);
						cr.cpMin = lStartLineIndex;
						cr.cpMax = lEndLineIndex;
#ifdef USE_RICH_EDIT
						SendMessage(client, EM_EXSETSEL, 0, (LPARAM)&cr);
#else
						SendMessage(client, EM_SETSEL, (WPARAM)cr.cpMin, (LPARAM)cr.cpMax);
#endif
					}
					GlobalFree((HGLOBAL)szBuffer);
				}
				else {
					if (bIndent) {
						SendMessage(client, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)szPrefix);
					}
				}
				GlobalFree((HGLOBAL)szPrefix);

				if (bRewrap) {
					SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_EDIT_WORDWRAP, 0), 0);
				}

#ifdef USE_RICH_EDIT
				InvalidateRect(client, NULL, TRUE);
#endif
				UpdateStatus();
				break;
			}
#ifdef USE_RICH_EDIT
			case ID_INSERT_MODE:
				bInsertMode = !bInsertMode;
				SendMessage(client, WM_KEYDOWN, (WPARAM)VK_INSERT, (LPARAM)0x510001);
				SendMessage(client, WM_KEYUP, (WPARAM)VK_INSERT, (LPARAM)0xC0510001);
				UpdateStatus();
				break;
#endif
			case ID_DATE_TIME_LONG:
			case ID_DATE_TIME: {
				TCHAR szTime[100], szDate[100];
				if (bLoading) {
					szTime[0] = '\r';
					szTime[1] = '\n';
					szTime[2] = '\0';
				}
				else {
					szTime[0] = '\0';
				}
				if (!options.bDontInsertTime) {
					GetTimeFormat(LOCALE_USER_DEFAULT, TIME_NOSECONDS, NULL, NULL, (bLoading ? szTime + 2 : szTime), 100);
					lstrcat(szTime, _T(" "));
				}
				GetDateFormat(LOCALE_USER_DEFAULT, (LOWORD(wParam) == ID_DATE_TIME_LONG ? DATE_LONGDATE : 0), NULL, NULL, szDate, 100);
				lstrcat(szTime, szDate);
				if (bLoading) {
					lstrcat(szTime, _T("\r\n"));
				}
				SendMessage(client, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)szTime);
#ifdef USE_RICH_EDIT
				InvalidateRect(client, NULL, TRUE);
#endif

				if (bLoading)
					bDirtyFile = FALSE;
				UpdateStatus();
				break;
			}
			case ID_PAGESETUP:
				{
					LPTSTR szLocale = _T("1");
					BOOL bMetric;
					PAGESETUPDLG psd;

					ZeroMemory(&psd, sizeof(PAGESETUPDLG));
					psd.lStructSize = sizeof(PAGESETUPDLG);
					psd.Flags |= /*PSD_INTHOUSANDTHSOFINCHES | */PSD_DISABLEORIENTATION | PSD_DISABLEPAPER | PSD_DISABLEPRINTER | PSD_ENABLEPAGESETUPTEMPLATE | PSD_MARGINS;
					psd.hwndOwner = hwndMain;
					psd.hInstance = hinstLang;
					psd.rtMargin = options.rMargins;
					psd.lpPageSetupTemplateName = MAKEINTRESOURCE(IDD_PAGE_SETUP);
					
					if (!GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IMEASURE, (LPTSTR)&szLocale, sizeof(szLocale))) {
						ReportLastError();
					}
					bMetric = (lstrcmp(szLocale, _T("0")) == 0);
					
					if (bMetric) {
						psd.rtMargin.bottom = (long)(psd.rtMargin.bottom * 2.54);
						psd.rtMargin.top = (long)(psd.rtMargin.top * 2.54);
						psd.rtMargin.left = (long)(psd.rtMargin.left * 2.54);
						psd.rtMargin.right = (long)(psd.rtMargin.right * 2.54);
					}

					if (PageSetupDlg(&psd)) {

						if (bMetric) {
							psd.rtMargin.bottom = (long)(psd.rtMargin.bottom * 0.3937);
							psd.rtMargin.top = (long)(psd.rtMargin.top * 0.3937);
							psd.rtMargin.left = (long)(psd.rtMargin.left * 0.3937);
							psd.rtMargin.right = (long)(psd.rtMargin.right * 0.3937);
						}

						options.rMargins = psd.rtMargin;
						SaveOptions();
					}
				}
				break;
			case ID_PRINT: {
				BOOL bFontChanged = FALSE;
				if (options.bPrintWithSecondaryFont && bPrimaryFont) {
					SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_FONT_PRIMARY, 0), 0);
					bFontChanged = TRUE;
				}
				PrintContents();
				if (bFontChanged) {
					SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_FONT_PRIMARY, 0), 0);
				}
				break;
			}
			case ID_HACKER:
			case ID_TABIFY:
			case ID_UNTABIFY:
			case ID_QUOTE:
			case ID_STRIP_CR:
			case ID_STRIP_CR_SPACE:
			case ID_STRIP_TRAILING_WS:
			case ID_MAKE_LOWER:
			case ID_MAKE_UPPER:
			case ID_MAKE_INVERSE:
			case ID_MAKE_SENTENCE:
			case ID_MAKE_TITLE:
			case ID_MAKE_OEM:
			case ID_MAKE_ANSI: {
				CHARRANGE cr;
				LPTSTR szSrc;
				LPTSTR szDest;
				LONG nSize;
				HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

#ifdef USE_RICH_EDIT
				SendMessage(client, EM_EXGETSEL, 0, (LPARAM)&cr);
#else
				SendMessage(client, EM_GETSEL, (WPARAM)&cr.cpMin, (LPARAM)&cr.cpMax);
#endif
				if (cr.cpMin == cr.cpMax) {
					ERROROUT(GetString(IDS_NO_SELECTED_TEXT));
					break;
				}
				nSize = cr.cpMax - cr.cpMin + 1;

				szSrc = (LPTSTR) GlobalAlloc(GPTR, nSize * sizeof(TCHAR));
				szDest = (LPTSTR) GlobalAlloc(GPTR, nSize * sizeof(TCHAR));

#ifdef USE_RICH_EDIT
				SendMessage(client, EM_GETSELTEXT, 0, (LPARAM)szSrc);
#else
				GetClientRange(cr.cpMin, cr.cpMax, szSrc);
#endif
				switch (LOWORD(wParam)) {
				case ID_UNTABIFY:
				case ID_TABIFY:
					{
						CHARRANGE cr2 = cr;
						TCHAR szFnd[101], szRepl[101];

						if (LOWORD(wParam) == ID_UNTABIFY) {
							lstrcpy(szFnd, _T("\t"));
							memset(szRepl, ' ', options.nTabStops);
							szRepl[options.nTabStops] = '\0';
						}
						else {
							lstrcpy(szRepl, _T("\t"));
							memset(szFnd, ' ', options.nTabStops);
							szFnd[options.nTabStops] = '\0';
						}

						nReplaceMax = cr.cpMax;
						cr2.cpMax = cr2.cpMin;

#ifdef USE_RICH_EDIT
						SendMessage(client, EM_EXSETSEL, 0, (LPARAM)&cr2);
#else
						SendMessage(client, EM_SETSEL, (WPARAM)cr2.cpMin, (LPARAM)cr2.cpMax);
#endif
						SendMessage(client, WM_SETREDRAW, (WPARAM)FALSE, 0);
						bReplacingAll = TRUE;
						while (SearchFile(szFnd, FALSE, TRUE, TRUE, FALSE)) {
							SendMessage(client, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)szRepl);
							nReplaceMax -= lstrlen(szFnd) - lstrlen(szRepl);
						}
						bReplacingAll = FALSE;
						cr.cpMax = nReplaceMax;
						nReplaceMax = -1;
						SendMessage(client, WM_SETREDRAW, (WPARAM)TRUE, 0);
						InvalidateRect(hwnd, NULL, TRUE);
					}
					break;
				case ID_QUOTE:
					{
						LONG i, j, nLines = 1;
						INT nQuoteLen = lstrlen(options.szQuote);

						for (i = 0; i < nSize-1; ++i) {
#ifdef USE_RICH_EDIT
							if (szSrc[i] == '\r') ++nLines;
#else
							if (szSrc[i] == '\n') ++nLines;
#endif
						}

						GlobalFree((HGLOBAL)szDest);
						szDest = (LPTSTR) GlobalAlloc(GPTR, nSize * sizeof(TCHAR) + nLines * nQuoteLen);

						lstrcpy(szDest, options.szQuote);
						for (i = 0, j = nQuoteLen; i < nSize-1; ++i) {
							szDest[j++] = szSrc[i];
#ifdef USE_RICH_EDIT
							if (szSrc[i] == '\r') {
#else
							if (szSrc[i] == '\n') {
#endif
								if (i == nSize - 2) {
									--nLines;
									break;
								}
								lstrcat(szDest, options.szQuote);
								j += nQuoteLen;
							}
						}

						cr.cpMax += nLines * nQuoteLen;
					}
					break;
				case ID_STRIP_CR_SPACE:
				case ID_STRIP_CR:
					{
						LONG i, j;
						for (i = 0, j = 0; i < nSize; ++i) {
#ifdef USE_RICH_EDIT
							if (szSrc[i] != '\r') {
								szDest[j++] = szSrc[i];
							}
							else if (szSrc[i+1] == '\r') {
								szDest[j++] = szSrc[i++];
								szDest[j++] = szSrc[i];
							}
							else if (LOWORD(wParam) == ID_STRIP_CR_SPACE){
								szDest[j++] = ' ';
							}
#else
							if (szSrc[i] != '\n' && szSrc[i] != '\r') {
								szDest[j++] = szSrc[i];
							}
							else if (szSrc[i] == '\r' && szSrc[i+1] == '\n' && szSrc[i+2] == '\r' && szSrc[i+3] == '\n') {
								szDest[j++] = szSrc[i++];
								szDest[j++] = szSrc[i++];
								szDest[j++] = szSrc[i++];
								szDest[j++] = szSrc[i];
							}
							else if (LOWORD(wParam) == ID_STRIP_CR_SPACE && szSrc[i] == '\n') {
								szDest[j++] = ' ';
							}
#endif
						}
						cr.cpMax -= i - j;
					}
					break;
				case ID_STRIP_TRAILING_WS:
					{
						INT i, j;
						BOOL bStrip = TRUE;

						szDest[lstrlen(szSrc)] = '\0';
						for (i = lstrlen(szSrc)-1, j = i; i >= 0; --i) {
							
							if (bStrip) {
								if (szSrc[i] == '\t' || szSrc[i] == ' ') {
									continue;
								}
								else if (szSrc[i] != '\r' && szSrc[i] != '\n') {
									bStrip = FALSE;
								}
							}
							else {
								if (szSrc[i] == '\r') {
									bStrip = TRUE;
								}
							}
							szDest[j--] = szSrc[i];
						}
						cr.cpMax -= j + 1;
						lstrcpy(szDest, szDest + j + 1);
					}
					break;
				case ID_HACKER:
					{
						LONG i;
						lstrcpy(szDest, szSrc);
						
						for (i = 0; i < nSize; i++) {
							if (_istascii(szDest[i])) {
								switch ((TCHAR)CharLower((LPTSTR)szDest[i])) {
								case 'a':
									szDest[i] = '4';
									break;
								case 't':
									szDest[i] = '7';
									break;
								case 'e':
									szDest[i] = '3';
									break;
								case 'l':
									szDest[i] = '1';
									break;
								case 's':
									szDest[i] = '5';
									break;
								case 'g':
									szDest[i] = '6';
									break;
								case 'o':
									szDest[i] = '0';
									break;
								}
							}
						}
					}
					break;
				case ID_MAKE_LOWER:	
					lstrcpy(szDest, szSrc);
					CharLowerBuff(szDest, nSize);
					break;
				case ID_MAKE_UPPER:
					lstrcpy(szDest, szSrc);
					CharUpperBuff(szDest, nSize);
					break;
				case ID_MAKE_INVERSE:
					{
						LONG i;

						lstrcpy(szDest, szSrc);
						for (i = 0; i < nSize; i++) {
							if (IsCharAlpha(szDest[i])) {
								if (IsCharUpper(szDest[i])) {
									CharLowerBuff(szDest + i, 1);
									//szDest[i] = (TCHAR)_tolower(szDest[i]);
								}
								else if (IsCharLower(szDest[i])) {
									CharUpperBuff(szDest + i, 1);
									//szDest[i] = (TCHAR)_toupper(szDest[i]);
								}
							}
						}
					}
					break;
				case ID_MAKE_TITLE:
				case ID_MAKE_SENTENCE:
					{
						LONG i;
						BOOL bNextUpper = TRUE;

						lstrcpy(szDest, szSrc);
						for (i = 0; i < nSize; i++) {
							if (IsCharAlpha(szDest[i])) {
								if (bNextUpper) {
									bNextUpper = FALSE;
									if (IsCharLower(szDest[i])) {
										CharUpperBuff(szDest + i, 1);
										//szDest[i] = (TCHAR)_toupper(szDest[i]);
									}
								}
								else {
									if (IsCharUpper(szDest[i])) {
										CharLowerBuff(szDest + i, 1);
										//szDest[i] = (TCHAR)_tolower(szDest[i]);
									}
								}
							}
							else {
								if (LOWORD(wParam) == ID_MAKE_TITLE && szDest[i] != '\'') {
									bNextUpper = TRUE;
								}
								else if (szDest[i] == '.'
									|| szDest[i] == '?'
									|| szDest[i] == '!'
									|| szDest[i] == '\r') {
									bNextUpper = TRUE;
								}

							}
						}
					}
					break;
				case ID_MAKE_OEM:
#ifdef BUILD_METAPAD_UNICODE
					ERROROUT(_T("Not supported in UNICODE version."));
#else
					CharToOemBuff(szSrc, szDest, nSize);
#endif
					break;
				case ID_MAKE_ANSI:
#ifdef BUILD_METAPAD_UNICODE
					ERROROUT(_T("Not supported in UNICODE version."));
#else
					OemToCharBuff(szSrc, szDest, nSize);
#endif
					break;
				}

				if (LOWORD(wParam) != ID_UNTABIFY && LOWORD(wParam) != ID_TABIFY) {
					if (lstrcmp(szSrc, szDest) != 0) {
						SendMessage(client, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)szDest);
					}
				}
#ifdef USE_RICH_EDIT
				SendMessage(client, EM_EXSETSEL, 0, (LPARAM)&cr);
				InvalidateRect(client, NULL, TRUE);
#else
				SendMessage(client, EM_SETSEL, (WPARAM)cr.cpMin, (LPARAM)cr.cpMax);
#endif
				GlobalFree((HGLOBAL)szDest);
				GlobalFree((HGLOBAL)szSrc);
				SetCursor(hcur);
				break;
			}
			case ID_NEW_INSTANCE:
				{
					TCHAR szBuffer[MAXFN];
					GetModuleFileName(hinstThis, szBuffer, MAXFN);
					//ShellExecute(NULL, NULL, szBuffer, NULL, szDir, SW_SHOWNORMAL);
					ExecuteProgram(szBuffer, "");
				}
				break;
			case ID_LAUNCH_ASSOCIATED_VIEWER:
				LaunchInViewer(FALSE, FALSE);
				break;
			case ID_UTF_8_FILE:
			case ID_UNICODE_FILE:
			case ID_UNICODE_BE_FILE:
			case ID_DOS_FILE:
			case ID_UNIX_FILE: {
				HMENU hmenu = GetSubMenu(GetSubMenu(GetMenu(hwndMain), 0), FILEFORMATPOS);

				bBinaryFile = FALSE;
				if (!bLoading) {
					if (LOWORD(wParam) == ID_UTF_8_FILE && nEncodingType == TYPE_UTF_8)
						break;
					else if (LOWORD(wParam) == ID_UNICODE_FILE && nEncodingType == TYPE_UTF_16)
						break;
					else if (LOWORD(wParam) == ID_UNICODE_BE_FILE && nEncodingType == TYPE_UTF_16_BE)
						break;
					else if (LOWORD(wParam) == ID_UNIX_FILE && bUnix && nEncodingType == TYPE_UNKNOWN)
						break;
					else if (LOWORD(wParam) == ID_DOS_FILE && !bUnix && nEncodingType == TYPE_UNKNOWN)
						break;
				}

				bUnix = LOWORD(wParam) == ID_UNIX_FILE;

				if (LOWORD(wParam) == ID_UTF_8_FILE)
					nEncodingType = TYPE_UTF_8;
				else if (LOWORD(wParam) == ID_UNICODE_FILE)
					nEncodingType = TYPE_UTF_16;
				else if (LOWORD(wParam) == ID_UNICODE_BE_FILE)
					nEncodingType = TYPE_UTF_16_BE;
				else
					nEncodingType = TYPE_UNKNOWN;

				CheckMenuRadioItem(hmenu, ID_DOS_FILE, ID_UTF_8_FILE, LOWORD(wParam), MF_BYCOMMAND);

				if (!bDirtyFile && !bLoading) {
					TCHAR szTmp[MAXFN];
					szTmp[0] = ' ';
					szTmp[1] = '*';
					szTmp[2] = ' ';
					GetWindowText(hwndMain, szTmp + 3, GetWindowTextLength(hwndMain) + 1);
					SetWindowText(hwndMain, szTmp);
					bDirtyFile = TRUE;
				}
				if (!bLoading) {
					bHideMessage = TRUE;
					UpdateStatus();
				}
				break;
			}
			case ID_RELOAD_CURRENT: {
				if (szFile[0] != '\0') {
					CHARRANGE cr;

					if (!SaveIfDirty())
						break;

					bHideMessage = FALSE;
					lstrcpy(szStatusMessage, GetString(IDS_FILE_LOADING));
					UpdateStatus();

					bLoading = TRUE;

#ifdef USE_RICH_EDIT
					SendMessage(client, EM_EXGETSEL, 0, (LPARAM)&cr);
					LoadFile(szFile, FALSE, FALSE);
					SendMessage(client, EM_EXSETSEL, 0, (LPARAM)&cr);
#else
					SendMessage(client, EM_GETSEL, (WPARAM)&cr.cpMin, (LPARAM)&cr.cpMax);
					LoadFile(szFile, FALSE, FALSE);
					SendMessage(client, EM_SETSEL, (WPARAM)cr.cpMin, (LPARAM)cr.cpMax);
					SendMessage(client, EM_SCROLLCARET, 0, 0);
#endif

					UpdateStatus();
					if (bLoading) {
						/*
						TCHAR szBuffer[MAXFN];
						wsprintf(szBuffer, STR_CAPTION_FILE, szCaptionFile);
						SetWindowText(hwndMain, szBuffer);
						bLoading = FALSE;
						bDirtyFile = FALSE;
						*/
						bLoading = FALSE;
						bDirtyFile = FALSE;
						UpdateCaption();
					}
				}
				break;
			}
#ifdef USE_RICH_EDIT
			case ID_SHOWFILESIZE:
				{
					HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));

					bHideMessage = FALSE;
					wsprintf(szStatusMessage, GetString(IDS_BYTE_LENGTH), CalculateFileSize());
					UpdateStatus();
					if (!bShowStatus)
						SendMessage(hwnd, WM_COMMAND, MAKEWPARAM(ID_SHOWSTATUS, 0), 0);
					SetCursor(hcur);
				}
				break;
#endif
			case ID_MRU_1:
			case ID_MRU_2:
			case ID_MRU_3:
			case ID_MRU_4:
			case ID_MRU_5:
			case ID_MRU_6:
			case ID_MRU_7:
			case ID_MRU_8:
			case ID_MRU_9:
			case ID_MRU_10:
			case ID_MRU_11:
			case ID_MRU_12:
			case ID_MRU_13:
			case ID_MRU_14:
			case ID_MRU_15:
			case ID_MRU_16:
				LoadFileFromMenu(LOWORD(wParam), TRUE);
				break;
			case ID_SET_MACRO_1:
			case ID_SET_MACRO_2:
			case ID_SET_MACRO_3:
			case ID_SET_MACRO_4:
			case ID_SET_MACRO_5:
			case ID_SET_MACRO_6:
			case ID_SET_MACRO_7:
			case ID_SET_MACRO_8:
			case ID_SET_MACRO_9:
			case ID_SET_MACRO_10: {
				CHARRANGE cr;
				int macroIndex = LOWORD(wParam) - ID_SET_MACRO_1;

#ifdef USE_RICH_EDIT
				SendMessage(client, EM_EXGETSEL, 0, (LPARAM)&cr);
#else
				SendMessage(client, EM_GETSEL, (WPARAM)&cr.cpMin, (LPARAM)&cr.cpMax);
#endif
				if (cr.cpMax - cr.cpMin > MAXMACRO - 1) {
					ERROROUT(GetString(IDS_MACRO_LENGTH_ERROR));
					break;
				}
#ifdef USE_RICH_EDIT
				SendMessage(client, EM_GETSELTEXT, 0, (LPARAM)options.MacroArray[macroIndex]);
#else
				GetClientRange(cr.cpMin, cr.cpMax, options.MacroArray[macroIndex]);
#endif

				if (!EncodeWithEscapeSeqs(options.MacroArray[macroIndex])) {
					ERROROUT(GetString(IDS_MACRO_LENGTH_ERROR));
					break;
				}
				else {
					HKEY key;

					if (!g_bIniMode) {
						RegCreateKeyEx(HKEY_CURRENT_USER, STR_REGKEY, 0, NULL, 0, KEY_ALL_ACCESS, NULL, &key, NULL);
						RegSetValueEx(key, _T("MacroArray"), 0, REG_BINARY, (LPBYTE)&options.MacroArray, sizeof(options.MacroArray));
						RegCloseKey(key);
					}
					else {
						char entry[14];
						wsprintf(entry, "szMacroArray%d", macroIndex);
						if (!SaveOption(NULL, entry, REG_SZ, (LPBYTE)&options.MacroArray[macroIndex], MAXMACRO)) {
							ReportLastError();
						}
					}
				}

				break;
			}
			case ID_MACRO_1:
			case ID_MACRO_2:
			case ID_MACRO_3:
			case ID_MACRO_4:
			case ID_MACRO_5:
			case ID_MACRO_6:
			case ID_MACRO_7:
			case ID_MACRO_8:
			case ID_MACRO_9:
			case ID_MACRO_10: {
				TCHAR szMacro[MAXMACRO];
				lstrcpy(szMacro, options.MacroArray[LOWORD(wParam) - ID_MACRO_1]);
				ParseForEscapeSeqs(szMacro);
				SendMessage(client, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)szMacro);
#ifdef USE_RICH_EDIT
				InvalidateRect(client, NULL, TRUE);
#endif
				break;
			}
			case ID_COMMIT_WORDWRAP:
				if (bWordWrap) {
					LONG lFileSize;
					LONG lLineLen, lMaxLine;
					LPTSTR szBuffer;
					LONG i, iIndex;
					LPTSTR szTemp;
					CHARRANGE cr;
					HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));
								
					lFileSize = GetWindowTextLength(client);
					lMaxLine = SendMessage(client, EM_GETLINECOUNT, 0, 0);
					szBuffer = (LPTSTR) GlobalAlloc(GPTR, (lFileSize + lMaxLine + 1) * sizeof(TCHAR));

					for (i = 0; i < lMaxLine; i++) {
						iIndex = SendMessage(client, EM_LINEINDEX, (WPARAM)i, 0);
						lLineLen = SendMessage(client, EM_LINELENGTH, (WPARAM)iIndex, 0);

						szTemp = (LPTSTR) GlobalAlloc(GPTR, (lLineLen+3) * sizeof(TCHAR));
						*((LPWORD)szTemp) = (USHORT)(lLineLen + 1);
						SendMessage(client, EM_GETLINE, (WPARAM)i, (LPARAM)(LPCTSTR)szTemp);
						szTemp[lLineLen] = '\0';
						lstrcat(szBuffer, szTemp);
						if (i < lMaxLine - 1) {
#ifdef USE_RICH_EDIT
							lstrcat(szBuffer, _T("\r"));
#else
							lstrcat(szBuffer, _T("\r\n"));
#endif
						}
						GlobalFree((HGLOBAL)szTemp);
					}

					cr.cpMin = 0;
					cr.cpMax = -1;
					SendMessage(client, WM_SETREDRAW, (WPARAM)FALSE, 0);

#ifdef USE_RICH_EDIT
					SendMessage(client, EM_EXSETSEL, 0, (LPARAM)&cr);
#else
					SendMessage(client, EM_SETSEL, (WPARAM)cr.cpMin, (LPARAM)cr.cpMax);
#endif
					SendMessage(client, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)szBuffer);
					cr.cpMax = 0;
#ifdef USE_RICH_EDIT
					SendMessage(client, EM_EXSETSEL, 0, (LPARAM)&cr);
#else
					SendMessage(client, EM_SETSEL, (WPARAM)cr.cpMin, (LPARAM)cr.cpMax);
#endif
					SendMessage(client, WM_SETREDRAW, (WPARAM)TRUE, 0);
					GlobalFree((HGLOBAL)szBuffer);
					InvalidateRect(client, NULL, TRUE);
					SetCursor(hcur);
				}
				break;
			case ID_SHIFT_ENTER:
			case ID_CONTROL_SHIFT_ENTER:
#ifdef USE_RICH_EDIT
			{
				BYTE keys[256]; 
				GetKeyboardState(keys); 
				keys[VK_SHIFT] &= 0x7F; 
				SetKeyboardState(keys); 
				SendMessage(client, WM_KEYDOWN, (WPARAM)VK_RETURN, 0); 
				keys[VK_SHIFT] |= 0x80; 
				SetKeyboardState(keys); 
			}
#else
				SendMessage(client, WM_CHAR, (WPARAM)'\r', 0);
#endif
				break;
			case ID_CONTEXTMENU:
				SendMessage(client, WM_RBUTTONUP, 0, 0);
				break;
			case ID_SCROLLUP:
				PostMessage(client, WM_VSCROLL, (WPARAM)SB_LINEUP, 0);
				break;
			case ID_SCROLLDOWN:
				PostMessage(client, WM_VSCROLL, (WPARAM)SB_LINEDOWN, 0);
				break;
			case ID_SCROLLLEFT:
				PostMessage(client, WM_HSCROLL, (WPARAM)SB_PAGELEFT, 0);
//				SendMessage(client, WM_SYSKEYUP, (WPARAM)VK_MENU, (LPARAM)0);
				break;
			case ID_SCROLLRIGHT:
				PostMessage(client, WM_HSCROLL, (WPARAM)SB_PAGERIGHT, 0);
//				SendMessage(client, WM_CHAR, (WPARAM)VK_MENU, (LPARAM)0);
				break;
			case ID_FAV_ADD:
				DialogBox(hinstLang, MAKEINTRESOURCE(IDD_FAV_NAME), hwndMain, (DLGPROC)AddFavDialogProc);
				break;
			case ID_FAV_EDIT: {
				TCHAR szBuffer[MAXFN];
				GetModuleFileName(hinstThis, szBuffer, MAXFN);
				//ShellExecute(NULL, NULL, szBuffer, szFav, szDir, SW_SHOWNORMAL);
				ExecuteProgram(szBuffer, szFav);
				break;
			}
			case ID_FAV_RELOAD:
				PopulateFavourites();
				break;
		}
		break;
	default:
		if (Msg == uFindReplaceMsg) {
			LPFINDREPLACE lpfr = (LPFINDREPLACE)lParam;
			TCHAR szBuffer[MAXFIND];
			int i, nIdx;
			BOOL bFinding = FALSE;
#ifdef USE_RICH_EDIT
			BOOL bFixCRProblem = FALSE;
			TCHAR szFindHold[MAXFIND];
			TCHAR szReplaceHold[MAXFIND];
#endif

			if (lpfr->Flags & FR_DIALOGTERM) {
				for (i = 0; i < NUMFINDS; i++) {
					SendDlgItemMessage(hdlgFind, ID_DROP_FIND, CB_GETLBTEXT, i, (WPARAM)FindArray[i]);
					if (lpfr->lpstrReplaceWith != NULL)
						SendDlgItemMessage(hdlgFind, ID_DROP_REPLACE, CB_GETLBTEXT, i, (WPARAM)ReplaceArray[i]);
				}
				hdlgFind = NULL;
				return FALSE;
			}

			bFinding = (lstrcmp(lpfr->lpstrFindWhat, _T("dummy_find")) == 0);
			
			SendDlgItemMessage(hdlgFind, ID_DROP_FIND, WM_GETTEXT, MAXFIND, (WPARAM)szBuffer);
			lstrcpy(lpfr->lpstrFindWhat, szBuffer);
						
			nIdx = SendDlgItemMessage(hdlgFind, ID_DROP_FIND, CB_FINDSTRINGEXACT, 0, (WPARAM)szBuffer);
			if (nIdx == CB_ERR) {
				if (SendDlgItemMessage(hdlgFind, ID_DROP_FIND, CB_GETCOUNT, 0, 0) >= NUMFINDS) {
					SendDlgItemMessage(hdlgFind, ID_DROP_FIND, CB_DELETESTRING, (LPARAM)NUMFINDS-1, 0);
				}
			}
			else {
				SendDlgItemMessage(hdlgFind, ID_DROP_FIND, CB_DELETESTRING, (LPARAM)nIdx, 0);
			}
			SendDlgItemMessage(hdlgFind, ID_DROP_FIND, CB_INSERTSTRING, 0, (WPARAM)szBuffer);
			SendDlgItemMessage(hdlgFind, ID_DROP_FIND, CB_SETCURSEL, (LPARAM)0, 0);

			if (lpfr->lpstrReplaceWith != NULL) {
				nIdx = SendDlgItemMessage(hdlgFind, ID_DROP_REPLACE, CB_FINDSTRINGEXACT, 0, (WPARAM)lpfr->lpstrReplaceWith);
				if (nIdx == CB_ERR) {
					if (SendDlgItemMessage(hdlgFind, ID_DROP_REPLACE, CB_GETCOUNT, 0, 0) >= NUMFINDS) {
						SendDlgItemMessage(hdlgFind, ID_DROP_REPLACE, CB_DELETESTRING, (LPARAM)NUMFINDS-1, 0);
					}
				}
				else {
					SendDlgItemMessage(hdlgFind, ID_DROP_REPLACE, CB_DELETESTRING, (LPARAM)nIdx, 0);
				}
				SendDlgItemMessage(hdlgFind, ID_DROP_REPLACE, CB_INSERTSTRING, 0, (WPARAM)lpfr->lpstrReplaceWith);
				SendDlgItemMessage(hdlgFind, ID_DROP_REPLACE, CB_SETCURSEL, (LPARAM)0, 0);
			
				if (!bNoFindHidden) {
					ParseForEscapeSeqs(lpfr->lpstrReplaceWith);
				}
			}

			if (!bNoFindHidden) {
				ParseForEscapeSeqs(lpfr->lpstrFindWhat);
			}

			bMatchCase = (BOOL) (lpfr->Flags & FR_MATCHCASE);
			bDown = (BOOL) (lpfr->Flags & FR_DOWN);
			bWholeWord = (BOOL) (lpfr->Flags & FR_WHOLEWORD);
			if (lpfr->Flags & FR_REPLACEALL) {
				HCURSOR hcur = SetCursor(LoadCursor(NULL, IDC_WAIT));
				UINT nCnt = 0;
				TCHAR szMsg[25];
				CHARRANGE cr, store = {0,0};
				BOOL bSelection = BST_CHECKED == SendDlgItemMessage(hdlgFind, IDC_RADIO_SELECTION, BM_GETCHECK, 0, 0);
				
				if (bSelection) {
#ifdef USE_RICH_EDIT
					SendMessage(client, EM_EXGETSEL, 0, (LPARAM)&cr);
#else
					SendMessage(client, EM_GETSEL, (WPARAM)&cr.cpMin, (LPARAM)&cr.cpMax);
#endif
					store.cpMax = cr.cpMax;
					store.cpMin = cr.cpMin;

					nReplaceMax = cr.cpMax;
					cr.cpMax = cr.cpMin;
				}
				else
					cr.cpMin = cr.cpMax = 0;

#ifdef USE_RICH_EDIT
				SendMessage(client, EM_EXSETSEL, 0, (LPARAM)&cr);
#else
				SendMessage(client, EM_SETSEL, (WPARAM)cr.cpMin, (LPARAM)cr.cpMax);
#endif
				SendMessage(client, WM_SETREDRAW, (WPARAM)FALSE, 0);
				bReplacingAll = TRUE;

#ifdef USE_RICH_EDIT // warning -- big kluge to fix problem in richedit re newlines!
				if (lpfr->lpstrFindWhat[lstrlen(lpfr->lpstrFindWhat) - 1] == '\r') {
					if (lpfr->lpstrReplaceWith[lstrlen(lpfr->lpstrReplaceWith) - 1] != '\r') {
						bFixCRProblem = TRUE;
					}
				}
#endif
				while (SearchFile(lpfr->lpstrFindWhat, bMatchCase, TRUE, TRUE, bWholeWord)) {
#ifdef USE_RICH_EDIT // warning -- big kluge!
					TCHAR ch[2] = {'\0', '\0'};

					lstrcpy(szReplaceHold, lpfr->lpstrReplaceWith);

					if (bFixCRProblem) {
						SendMessage(client, EM_EXGETSEL, 0, (LPARAM)&cr);
						++cr.cpMax;
						SendMessage(client, EM_EXSETSEL, 0, (LPARAM)&cr);
						SendMessage(client, EM_GETSELTEXT, 0, (LPARAM)szFindHold);
						if (lstrlen(szFindHold) == cr.cpMax - cr.cpMin) {
							ch[0] = szFindHold[lstrlen(szFindHold) - 1];
							lstrcat(szReplaceHold, ch);
						}
					}

					SendMessage(client, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)szReplaceHold);

					if (bFixCRProblem) {
						SendMessage(client, EM_EXGETSEL, 0, (LPARAM)&cr);
						--cr.cpMin;
						--cr.cpMax;
						SendMessage(client, EM_EXSETSEL, 0, (LPARAM)&cr);
					}

#else
					SendMessage(client, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)lpfr->lpstrReplaceWith);
#endif
					if (bSelection)
						nReplaceMax -= lstrlen(lpfr->lpstrFindWhat) - lstrlen(lpfr->lpstrReplaceWith);
					nCnt++;
				}
				bReplacingAll = FALSE;
				nReplaceMax = -1;

				if (bSelection) {
					cr.cpMin = store.cpMin;
					cr.cpMax = store.cpMax + nCnt * (lstrlen(lpfr->lpstrReplaceWith) - lstrlen(lpfr->lpstrFindWhat));
				}

#ifdef USE_RICH_EDIT
				SendMessage(client, EM_EXSETSEL, 0, (LPARAM)&cr);
#else
				SendMessage(client, EM_SETSEL, (WPARAM)cr.cpMin, (LPARAM)cr.cpMax);
#endif
				
				SendMessage(client, WM_SETREDRAW, (WPARAM)TRUE, 0);

				InvalidateRect(client, NULL, TRUE);
				SetCursor(hcur);

				UpdateStatus();
				wsprintf(szMsg, GetString(IDS_ITEMS_REPLACED), nCnt);
				MessageBox(hdlgFind, szMsg, STR_METAPAD, MB_OK|MB_ICONINFORMATION);
			}
			else if (lpfr->Flags & FR_FINDNEXT) {
				SearchFile(lpfr->lpstrFindWhat, bMatchCase, FALSE, bDown, bWholeWord);
				if (bFinding) {
					bCloseAfterFind = (BST_CHECKED == SendDlgItemMessage(hdlgFind, IDC_CLOSE_AFTER_FIND, BM_GETCHECK, 0, 0));
					if (bCloseAfterFind) {
						int i;
						for (i = 0; i < NUMFINDS; i++) {
							SendDlgItemMessage(hdlgFind, ID_DROP_FIND, CB_GETLBTEXT, i, (WPARAM)FindArray[i]);
						}
						DestroyWindow(hdlgFind);
						hdlgFind = NULL;
					}
				}
			}
			else if (lpfr->Flags & FR_REPLACE) {
				CHARRANGE cr;
#ifdef USE_RICH_EDIT
				SendMessage(client, EM_EXGETSEL, 0, (LPARAM)&cr);
				cr.cpMax = cr.cpMin;
				SendMessage(client, EM_EXSETSEL, 0, (LPARAM)&cr);
#else
				SendMessage(client, EM_GETSEL, (WPARAM)&cr.cpMin, (LPARAM)&cr.cpMax);
				cr.cpMax = cr.cpMin;
				SendMessage(client, EM_SETSEL, (WPARAM)cr.cpMin, (LPARAM)cr.cpMax);
#endif

#ifdef USE_RICH_EDIT // warning -- big kluge!
				if (lpfr->lpstrFindWhat[lstrlen(lpfr->lpstrFindWhat) - 1] == '\r') {
					if (lpfr->lpstrReplaceWith[lstrlen(lpfr->lpstrReplaceWith) - 1] != '\r') {
						bFixCRProblem = TRUE;
					}
				}
				lstrcpy(szReplaceHold, lpfr->lpstrReplaceWith);

#endif
				if (SearchFile(lpfr->lpstrFindWhat, bMatchCase, FALSE, bDown, bWholeWord)) {
#ifdef USE_RICH_EDIT // warning -- big kluge!
					TCHAR ch[2] = {'\0', '\0'};

					if (bFixCRProblem) {
						SendMessage(client, EM_EXGETSEL, 0, (LPARAM)&cr);
						++cr.cpMax;
						SendMessage(client, EM_EXSETSEL, 0, (LPARAM)&cr);
						SendMessage(client, EM_GETSELTEXT, 0, (LPARAM)szFindHold);
						if (lstrlen(szFindHold) == cr.cpMax - cr.cpMin) {
							ch[0] = szFindHold[lstrlen(szFindHold) - 1];
							lstrcat(szReplaceHold, ch);
						}
					}

					SendMessage(client, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)szReplaceHold);

					if (bFixCRProblem) {
						SendMessage(client, EM_EXGETSEL, 0, (LPARAM)&cr);
						--cr.cpMin;
						--cr.cpMax;
						SendMessage(client, EM_EXSETSEL, 0, (LPARAM)&cr);
					}

					InvalidateRect(client, NULL, TRUE);
#else
					SendMessage(client, EM_REPLACESEL, (WPARAM)TRUE, (LPARAM)lpfr->lpstrReplaceWith);
#endif
					SearchFile(lpfr->lpstrFindWhat, bMatchCase, FALSE, bDown, bWholeWord);
				}
			}
			SendMessage(client, EM_SCROLLCARET, 0, 0);
		
			return FALSE;
		}
		return DefWindowProc(hwndMain, Msg, wParam, lParam);
	}
	return FALSE;
}

DWORD WINAPI LoadThread(LPVOID lpParameter)
{
	LoadFile(szFile, TRUE, TRUE);
	if (bLoading) {
		bLoading = FALSE;
		if (lstrlen(szFile) == 0) {
			MakeNewFile();
		}
#ifdef USE_RICH_EDIT
		else {
			if (lpParameter != NULL) {
				GotoLine(((CHARRANGE*)lpParameter)->cpMin, ((CHARRANGE*)lpParameter)->cpMax);
			}
			else {
				SendMessage(client, EM_SCROLLCARET, 0, 0);
			}
		}
#endif
		UpdateCaption();
	}
	else {
		MakeNewFile();
	}
	SendMessage(client, EM_SETREADONLY, (WPARAM)FALSE, 0);

#ifdef USE_RICH_EDIT
	if (!bWordWrap && !options.bHideScrollbars) { // Hack for initially drawing hscrollbar for Richedit
		SetWindowLong(client, GWL_STYLE, GetWindowLong(client, GWL_STYLE) | WS_HSCROLL);
		SetWindowPos(client, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
	}
#endif

	EnableWindow(hwnd, TRUE);
	SendMessage(hwnd, WM_ACTIVATE, 0, 0);
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	WNDCLASS wc;
	MSG msg;
	HACCEL accel = NULL;
	int left, top, width, height;
	HMENU hmenu;
	MENUITEMINFO mio;
	CHARRANGE crLineCol = {-1, -1};
	LPTSTR szCmdLine;
	BOOL bSkipLanguagePlugin = FALSE;

	if (!hPrevInstance) {
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW);
		wc.style = /*CS_BYTEALIGNWINDOW CS_VREDRAW | CS_HREDRAW;*/ 0;
		wc.lpfnWndProc = MainWndProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInstance;
		wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PAD));
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.lpszMenuName = /*MAKEINTRESOURCE(IDR_MENU)*/0;
		wc.lpszClassName = STR_METAPAD;
		if (!RegisterClass(&wc)) {
			ReportLastError();
			return FALSE;
		}
	}

	hinstThis = hInstance;

#ifdef BUILD_METAPAD_UNICODE
	szCmdLine = GetCommandLine();
	szCmdLine = _tcschr(szCmdLine, _T(' ')) + 1;
#else
	szCmdLine = lpCmdLine;
#endif

	{
		TCHAR* pch;
		GetModuleFileName(hinstThis, szMetapadIni, MAXFN);
			
		pch = _tcsrchr(szMetapadIni, _T('\\'));
		++pch;
		*pch = '\0';
		lstrcat(szMetapadIni, "metapad.ini");
	}

	if (lstrlen(szCmdLine) > 0) {
		int nCmdLen = lstrlen(szCmdLine);
		if (nCmdLen > 1 && szCmdLine[0] == '/') {

			TCHAR chOption = (TCHAR)CharLower((LPTSTR)szCmdLine[1]);
			if (chOption == 's') {
				bSkipLanguagePlugin = TRUE;
				szCmdLine += 2;
				if (szCmdLine[0] == ' ') ++szCmdLine;
			}
			else if (chOption == 'v') {
				g_bDisablePluginVersionChecking = TRUE;
				szCmdLine += 2;
				if (szCmdLine[0] == ' ') ++szCmdLine;
			}
			else if (chOption == 'i') {
				g_bIniMode = TRUE;
				szCmdLine += 2;
				if (szCmdLine[0] == ' ') ++szCmdLine;
			}
			else if (chOption == 'm') {
				LoadOptions();
				g_bIniMode = TRUE;
				SaveOptions();
				MSGOUT("Migration to INI completed.");
				return FALSE;
			}
		}
	}

	if (!g_bIniMode) {
		WIN32_FIND_DATA FindFileData;
		HANDLE handle;

		if ((handle = FindFirstFile(szMetapadIni, &FindFileData)) != INVALID_HANDLE_VALUE) {
			FindClose(handle);
			g_bIniMode = TRUE;
		}
	}
	GetCurrentDirectory(MAXFN, szDir);
	LoadOptions();

	options.nStatusFontWidth = LOWORD(GetDialogBaseUnits());

	bWordWrap = FALSE;
	bPrimaryFont = TRUE;
	bSmartSelect = TRUE;
#ifdef USE_RICH_EDIT
	bHyperlinks = TRUE;
#endif
	bShowStatus = TRUE;
	bShowToolbar = TRUE;
	bAlwaysOnTop = FALSE;
	bCloseAfterFind = FALSE;
	bNoFindHidden = TRUE;
	bTransparent = FALSE;

	lstrcpy(szCustomFilter, GetString(IDS_DEFAULT_FILTER));
	LoadMenusAndData();

	FixFilterString(szCustomFilter);

	if (options.bSaveWindowPlacement || options.bStickyWindow)
		LoadWindowPlacement(&left, &top, &width, &height, &nCmdShow);
	else {
		left = top = width = height = CW_USEDEFAULT;
		nCmdShow = SW_SHOWNORMAL;
	}
	
	/*
	{
		OSVERSIONINFO vi;
		
		vi.dwOSVersionInfoSize = sizeof(vi);
		GetVersionEx(&vi);
		if (vi.dwPlatformId == VER_PLATFORM_WIN32_NT && vi.dwMajorVersion >= 5) {
			bWin2k = TRUE;
		}
	}
	*/

	{
		TCHAR szBuffer[100];
		wsprintf(szBuffer, STR_CAPTION_FILE, GetString(IDS_NEW_FILE));

		hwnd = CreateWindowEx(
			WS_EX_ACCEPTFILES,
			STR_METAPAD,
			szBuffer,
			WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
			left, top, width, height,
			NULL,
			NULL,
			hInstance,
			NULL);
	}

	if (!hwnd) {
		ReportLastError();
		return FALSE;
	}

	if (!bSkipLanguagePlugin) {
		FindAndLoadLanguagePlugin();
	}

	{
		HMENU hm = LoadMenu(hinstLang, MAKEINTRESOURCE(IDR_MENU));

		if (hm == NULL) {
			ReportLastError();
			return FALSE;
		}

#ifndef USE_RICH_EDIT
		{
			HMENU hsub = GetSubMenu(hm, 0);

			DeleteMenu(hsub, 10, MF_BYPOSITION);
			hsub = GetSubMenu(hm, 3);
			DeleteMenu(hsub, 8, MF_BYPOSITION);
		}
#endif

		SetMenu(hwnd, hm);

		if (hinstLang != hinstThis) {
			HMENU hsub = GetSubMenu(hm, 4);
			MENUITEMINFO mio;

			mio.cbSize = sizeof(MENUITEMINFO);
			mio.fMask = MIIM_TYPE | MIIM_ID;
			mio.fType = MFT_STRING;
			mio.wID = ID_ABOUT_PLUGIN;
			mio.dwTypeData = GetString(IDS_MENU_LANGUAGE_PLUGIN);
			InsertMenuItem(hsub, 1, TRUE, &mio);
		}
	}


/*
	if (bWin2k) {
		HMODULE mh;
			
		mh = GetModuleHandle(_T("user32.dll"));
		SetLWA = (SLWA)(GetProcAddress(mh, _T("SetLayeredWindowAttributes")));
		if (SetLWA) {
			SetLWA(hwnd, 0, 255, LWA_ALPHA);
		}
		else {
			ERROROUT(_T("Cannot find SetLayeredWindowAttributes in USER32.DLL"));
		}
	}
	else {
		HMENU hsub = GetSubMenu(GetMenu(hwnd), 3);
		DeleteMenu(hsub, 4, MF_BYPOSITION);
	}
*/
	{
		HMODULE hm;
			
		hm = GetModuleHandle(_T("user32.dll"));
		SetLWA = (SLWA)(GetProcAddress(hm, _T("SetLayeredWindowAttributes")));
	
		if (SetLWA) {
			SetLWA(hwnd, 0, 255, LWA_ALPHA);
		}
		else {
			HMENU hsub = GetSubMenu(GetMenu(hwnd), 3);
			DeleteMenu(hsub, 4, MF_BYPOSITION);
		}
	}
	
	accel = LoadAccelerators(hinstLang, MAKEINTRESOURCE(IDR_ACCELERATOR));
	if (!accel) {
		ReportLastError();
		return FALSE;
	}

#ifdef USE_RICH_EDIT
	if (LoadLibrary(STR_RICHDLL) == NULL) {
		ReportLastError();
		ERROROUT(GetString(IDS_RICHED_MISSING_ERROR));
		return FALSE;
	}
#endif

	CreateClient(hwnd, NULL, bWordWrap);
	if (!client) {
		ReportLastError();
		return FALSE;
	}
	SetClientFont(bPrimaryFont);

	uFindReplaceMsg = RegisterWindowMessage(FINDMSGSTRING);

	hmenu = GetMenu(hwnd);
	mio.cbSize = sizeof(MENUITEMINFO);
	mio.fMask = MIIM_STATE;
	if (bWordWrap) {
		mio.fState = MFS_CHECKED;
		SetMenuItemInfo(hmenu, ID_EDIT_WORDWRAP, 0, &mio);
	}
	if (!bSmartSelect) {
		mio.fState = MFS_UNCHECKED;
		SetMenuItemInfo(hmenu, ID_SMARTSELECT, 0, &mio);
	}
#ifdef USE_RICH_EDIT
	if (!bHyperlinks) {
		mio.fState = MFS_UNCHECKED;
		SetMenuItemInfo(hmenu, ID_SHOWHYPERLINKS, 0, &mio);
	}
#endif
	if (bTransparent && SetLWA) {
		mio.fState = MFS_CHECKED;
		SetMenuItemInfo(hmenu, ID_TRANSPARENT, 0, &mio);
		SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
		SetLWA(hwnd, 0, (BYTE)((255 * (100 - options.nTransparentPct)) / 100), LWA_ALPHA);
	}
	if (bShowStatus) {
		mio.fState = MFS_CHECKED;
		SetMenuItemInfo(hmenu, ID_SHOWSTATUS, 0, &mio);
		CreateStatusbar();
	}
	if (bShowToolbar) {
		mio.fState = MFS_CHECKED;
		SetMenuItemInfo(hmenu, ID_SHOWTOOLBAR, 0, &mio);
		CreateToolbar();
	}
	if (!bPrimaryFont) {
		mio.fState = MFS_UNCHECKED;
		SetMenuItemInfo(hmenu, ID_FONT_PRIMARY, 0, &mio);
	}
	if (bAlwaysOnTop) {
		mio.fState = MFS_CHECKED;
		SetMenuItemInfo(hmenu, ID_ALWAYSONTOP, 0, &mio);
		SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
	}

	bDirtyFile = FALSE;
	bDown = TRUE;
	bWholeWord = FALSE;
	bReplacingAll = FALSE;
	nReplaceMax = -1;
	bInsertMode = TRUE;

	InitCommonControls();

	if (options.bRecentOnOwn)
		FixMRUMenus();

	if (!options.bReadOnlyMenu)
		FixReadOnlyMenu();

	PopulateMRUList();

	if (options.bNoFaves) {
		DeleteMenu(hmenu, FAVEPOS, MF_BYPOSITION);
		SetWindowPos(hwnd, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
	}
	else {
		WIN32_FIND_DATA FindFileData;
		HANDLE handle;

		if (options.szFavDir[0] == '\0' || (handle = FindFirstFile(options.szFavDir, &FindFileData)) == INVALID_HANDLE_VALUE) {
			TCHAR* pch;
			GetModuleFileName(hInstance, szFav, MAXFN);
			
			pch = _tcsrchr(szFav, _T('\\'));
			++pch;
			*pch = '\0';
		}
		else {
			FindClose(handle);
			lstrcpy(szFav, options.szFavDir);
			lstrcat(szFav, "\\");
		}

		lstrcat(szFav, STR_FAV_FILE);

		PopulateFavourites();
	}

	MakeNewFile();

	if (lstrlen(szCmdLine) > 0) {
		int nCmdLen;
	
		nCmdLen = lstrlen(szCmdLine);

		if (nCmdLen > 1 && szCmdLine[0] == '/') {
			TCHAR chOption = (TCHAR)CharLower((LPTSTR)szCmdLine[1]);
			
			if (chOption == 'p') {
				if (szCmdLine[3] == _T('\"') && szCmdLine[nCmdLen - 1] == _T('\"'))
					lstrcpyn(szFile, szCmdLine + 4, nCmdLen - 1);
				else
					lstrcpyn(szFile, szCmdLine + 3, nCmdLen + 1);

				LoadFile(szFile, FALSE, FALSE);
				lstrcpy(szCaptionFile, szFile);
				PrintContents();
				CleanUp();
				return TRUE;
			}
			else if (chOption == 'g') {
				TCHAR szNum[6];
				int nRlen, nClen;

				ZeroMemory(szNum, sizeof(szNum));
				for (nRlen = 0; _istdigit(szCmdLine[nRlen + 3]); nRlen++)
					szNum[nRlen] = szCmdLine[nRlen + 3];
				crLineCol.cpMin = _ttol(szNum);

				ZeroMemory(szNum, sizeof(szNum));
				for (nClen = 0; _istdigit(szCmdLine[nClen + nRlen + 4]); nClen++)
					szNum[nClen] = szCmdLine[nRlen + nClen + 4];
				crLineCol.cpMax = _ttol(szNum);

				if (szCmdLine[5 + nClen + nRlen] == '\"' && szCmdLine[nCmdLen - 1] == '\"')
					lstrcpyn(szFile, szCmdLine + nClen + nRlen + 6, nCmdLen - 1);
				else
					lstrcpyn(szFile, szCmdLine + nClen + nRlen + 5, nCmdLen + 1);
			}
			else if (chOption == 'e') {
				if (szCmdLine[3] == _T('\"') && szCmdLine[nCmdLen - 1] == _T('\"'))
					lstrcpyn(szFile, szCmdLine + 4, nCmdLen - 1);
				else
					lstrcpyn(szFile, szCmdLine + 3, nCmdLen + 1);

				crLineCol.cpMax = crLineCol.cpMin = 0x7fffffff;
			}
			else {
				ERROROUT(GetString(IDS_COMMAND_LINE_OPTIONS));
				CleanUp();
				return TRUE;
			}
		}
		else {
			//ERROROUT(szCmdLine);
			if (szCmdLine[0] == '\"') {
				lstrcpyn(szFile, szCmdLine + 1, _tcschr(szCmdLine+1, _T('\"')) - szCmdLine);
			}
			else {
				lstrcpyn(szFile, szCmdLine, nCmdLen + 1);
			}
		}
		bLoading = TRUE;

		GetFullPathName(szFile, MAXFN, szFile, NULL);
		
		ExpandFilename(szFile);
#ifdef USE_RICH_EDIT
		{
			DWORD dwID;
			TCHAR szBuffer[MAXFN];

			wsprintf(szBuffer, STR_CAPTION_FILE, szCaptionFile);
			SetWindowText(hwnd, szBuffer);
			SendMessage(client, EM_SETREADONLY, (WPARAM)TRUE, 0);
			ShowWindow(hwnd, nCmdShow);

			EnableWindow(hwnd, FALSE);
			hthread = CreateThread(NULL, 0, LoadThread, (LPVOID)&(crLineCol), 0, &dwID);
		}
#else
		LoadThread(NULL);
		if (bQuitApp)
			PostQuitMessage(0);
		else
			ShowWindow(hwnd, nCmdShow);

		SendMessage(client, EM_SCROLLCARET, 0, 0);
		GotoLine(crLineCol.cpMin, crLineCol.cpMax);
#endif
	}
	else {
		ShowWindow(hwnd, nCmdShow);
	}

#ifdef USE_RICH_EDIT
	if (!bWordWrap && !options.bHideScrollbars) { // Hack for initially drawing hscrollbar for Richedit
		SetWindowLong(client, GWL_STYLE, GetWindowLong(client, GWL_STYLE) | WS_HSCROLL);
		SetWindowPos(client, 0, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
	}
#endif

	UpdateStatus();

	SendMessage(client, EM_SETMARGINS, (WPARAM)EC_LEFTMARGIN, MAKELPARAM(options.nSelectionMarginWidth, 0));
	SetFocus(client);

	while (GetMessage(&msg, NULL, 0,0)) {
		if (!(hdlgFind && IsDialogMessage(hdlgFind, &msg)) && !TranslateAccelerator(hwnd, accel, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return msg.wParam;
}
