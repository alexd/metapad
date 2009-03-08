void __cdecl WinMainCRTStartup(void)
{
    int mainret;
    LPSTR lpszCommandLine;
    STARTUPINFO StartupInfo;

    lpszCommandLine = (LPSTR)GetCommandLine();

    if (*lpszCommandLine == '"') {
		lpszCommandLine++;
        while(*lpszCommandLine && (*lpszCommandLine != '"'))
            lpszCommandLine++;

        if (*lpszCommandLine == '"')
            lpszCommandLine++;
    }
    else {
        while (*lpszCommandLine > ' ')
            lpszCommandLine++;
    }

    while ( *lpszCommandLine && (*lpszCommandLine <= ' ') )
        lpszCommandLine++;

    StartupInfo.dwFlags = 0;
    GetStartupInfo(&StartupInfo);

    mainret = WinMain( GetModuleHandle(NULL),
                       NULL,
                       lpszCommandLine,
                       StartupInfo.dwFlags & STARTF_USESHOWWINDOW
                       ? StartupInfo.wShowWindow : SW_SHOWDEFAULT );

    ExitProcess(mainret);
}
/*
int __cdecl _isctype(int c , int mask) 
{
    if (((unsigned)(c + 1)) <= 256)
        return (_pctype[c] & mask);
    else
        return 0;
}
*/
/*
long __cdecl atol(const char* pstr)
{
    int  cCurr;
    long lTotal;
    int  iIsNeg;

    while (isspace (*pstr))
        ++pstr;

    cCurr = *pstr++;
    iIsNeg = cCurr;
    if (('-' == cCurr) || ('+' == cCurr))
        cCurr = *pstr++;

    lTotal = 0;

    while (isdigit(cCurr)) {
        lTotal = 10 * lTotal + (cCurr - '0');
        cCurr = *pstr++;
    }

    if ('-' == iIsNeg)
        return (-lTotal);
    else
        return (lTotal);
}

int __cdecl atoi(const char * pstr)
{
    return ((int)atol (pstr));
}
*/
/*
int __cdecl tolower(int c)
{
	if (c - 'A' + 'a' < 0)
		return c;
	else 
		return c - 'A' + 'a';
}
*/
char* __cdecl strchr(const char* str, int ch)
{
	while (*str != '\0') {
		if (*str == ch)
			return (char*)str;
		str++;
	}
	return NULL;
}

char* __cdecl strrchr(const char* str, int ch)
{
	char* strb = (char*)str;

	while (*str != '\0') {
		str++;
	}
	while (str != strb) {
		if (*str == ch)
			return (char*)str;
		str--;
	}
	return NULL;
}