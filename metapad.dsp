# Microsoft Developer Studio Project File - Name="metapad" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=metapad - Win32 Debug Unicode
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "metapad.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "metapad.mak" CFG="metapad - Win32 Debug Unicode"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "metapad - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE "metapad - Win32 Debug Unicode" (based on "Win32 (x86) Application")
!MESSAGE "metapad - Win32 Debug RichEdit" (based on "Win32 (x86) Application")
!MESSAGE "metapad - Win32 Debug RichEdit Unicode" (based on "Win32 (x86) Application")
!MESSAGE "metapad - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "metapad - Win32 Release Unicode" (based on "Win32 (x86) Application")
!MESSAGE "metapad - Win32 Release RichEdit" (based on "Win32 (x86) Application")
!MESSAGE "metapad - Win32 Release RichEdit Unicode" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "metapad - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /Zi /Od /D "_DEBUG" /D "_MBCS" /D "NO_RICH_EDIT" /D "WIN32" /D "_WINDOWS" /FD /GZ /c
# SUBTRACT CPP /Gf /Gy /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x1009 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "NO_RICH_EDIT"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 libc.lib comctl32.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib
# SUBTRACT LINK32 /profile

!ELSEIF  "$(CFG)" == "metapad - Win32 Debug Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "metapad___Win32_Debug_Unicode"
# PROP BASE Intermediate_Dir "metapad___Win32_Debug_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_Unicode"
# PROP Intermediate_Dir "Debug_Unicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /Zi /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FD /GZ /c
# SUBTRACT BASE CPP /Gf /Gy /YX
# ADD CPP /nologo /W3 /Gm /Zi /Od /D "_DEBUG" /D "UNICODE" /D "NO_RICH_EDIT" /D "WIN32" /D "_WINDOWS" /FD /GZ /c
# SUBTRACT CPP /Gf /Gy /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "NO_RICH_EDIT"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 libc.lib comctl32.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib
# SUBTRACT BASE LINK32 /profile
# ADD LINK32 libc.lib comctl32.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib
# SUBTRACT LINK32 /profile

!ELSEIF  "$(CFG)" == "metapad - Win32 Debug RichEdit"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "metapad___Win32_Debug_RichEdit"
# PROP BASE Intermediate_Dir "metapad___Win32_Debug_RichEdit"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_RichEdit"
# PROP Intermediate_Dir "Debug_RichEdit"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /Zi /Od /D "SLEDGEHAMMER" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FD /GZ /c
# SUBTRACT BASE CPP /Gf /Gy /YX
# ADD CPP /nologo /W3 /Gm /Zi /Od /D "USE_RICH_EDIT" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FD /GZ /c
# SUBTRACT CPP /Gf /Gy /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "USE_RICH_EDIT"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 libc.lib comctl32.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib
# SUBTRACT BASE LINK32 /profile
# ADD LINK32 libc.lib comctl32.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib
# SUBTRACT LINK32 /profile

!ELSEIF  "$(CFG)" == "metapad - Win32 Debug RichEdit Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "metapad___Win32_Debug_RichEdit_Unicode"
# PROP BASE Intermediate_Dir "metapad___Win32_Debug_RichEdit_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug_RichEdit_Unicode"
# PROP Intermediate_Dir "Debug_RichEdit_Unicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /Zi /Od /D "USE_RICH_EDIT" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FD /GZ /c
# SUBTRACT BASE CPP /Gf /Gy /YX
# ADD CPP /nologo /W3 /Gm /Zi /Od /D "USE_RICH_EDIT" /D "_DEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /FD /GZ /c
# SUBTRACT CPP /Gf /Gy /YX
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG" /d "USE_RICH_EDIT"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 libc.lib comctl32.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib
# SUBTRACT BASE LINK32 /profile
# ADD LINK32 libc.lib comctl32.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib
# SUBTRACT LINK32 /profile

!ELSEIF  "$(CFG)" == "metapad - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W4 /Ox /Og /Os /Gf /D "NDEBUG" /D "_MCBS" /D "NO_RICH_EDIT" /D "WIN32" /D "_WINDOWS" /FD /c
# SUBTRACT CPP /WX /Gy /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x1009 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "NO_RICH_EDIT"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 libc.lib comctl32.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib /nologo /version:4.0 /subsystem:windows /pdb:none /machine:I386 /nodefaultlib /opt:nowin98
# SUBTRACT LINK32 /map

!ELSEIF  "$(CFG)" == "metapad - Win32 Release Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "metapad___Win32_Release_Unicode"
# PROP BASE Intermediate_Dir "metapad___Win32_Release_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_Unicode"
# PROP Intermediate_Dir "Release_Unicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W4 /Ox /Og /Os /Gf /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MCBS" /FD /c
# SUBTRACT BASE CPP /WX /Gy /YX
# ADD CPP /nologo /W4 /Ox /Og /Os /Gf /D "NDEBUG" /D "UNICODE" /D "NO_RICH_EDIT" /D "WIN32" /D "_WINDOWS" /FD /c
# SUBTRACT CPP /WX /Gy /YX
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "NO_RICH_EDIT"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 libc.lib comctl32.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib /nologo /version:4.0 /subsystem:windows /pdb:none /machine:I386 /nodefaultlib /opt:nowin98
# SUBTRACT BASE LINK32 /map
# ADD LINK32 libc.lib comctl32.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib /nologo /version:4.0 /subsystem:windows /pdb:none /machine:I386 /nodefaultlib /opt:nowin98
# SUBTRACT LINK32 /map
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=bupx release\metapad.exe	copy Release\metapad.exe c:\winnt\system32\notepad.exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "metapad - Win32 Release RichEdit"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "metapad___Win32_Release_RichEdit"
# PROP BASE Intermediate_Dir "metapad___Win32_Release_RichEdit"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_RichEdit"
# PROP Intermediate_Dir "Release_RichEdit"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W4 /Ox /Og /Os /Gf /D "SLEDGEHAMMER" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MCBS" /FD /c
# SUBTRACT BASE CPP /WX /Gy /YX
# ADD CPP /nologo /W4 /Ox /Og /Os /Gf /D "USE_RICH_EDIT" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MCBS" /Zl /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "USE_RICH_EDIT"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 libc.lib comctl32.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib /nologo /version:4.0 /subsystem:windows /pdb:none /machine:I386 /nodefaultlib /opt:nowin98
# SUBTRACT BASE LINK32 /map
# ADD LINK32 libc.lib comctl32.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib /nologo /version:4.0 /subsystem:windows /pdb:none /machine:I386 /nodefaultlib /opt:nowin98
# SUBTRACT LINK32 /map
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=copy Release_RichEdit\metapad.exe c:\winxp\system32\notepad.exe
# End Special Build Tool

!ELSEIF  "$(CFG)" == "metapad - Win32 Release RichEdit Unicode"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "metapad___Win32_Release_RichEdit_Unicode"
# PROP BASE Intermediate_Dir "metapad___Win32_Release_RichEdit_Unicode"
# PROP BASE Ignore_Export_Lib 0
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release_RichEdit_Unicode"
# PROP Intermediate_Dir "Release_RichEdit_Unicode"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W4 /Ox /Og /Os /Gf /D "USE_RICH_EDIT" /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MCBS" /Zl /FD /c
# ADD CPP /nologo /W4 /Ox /Og /Os /Gf /D "USE_RICH_EDIT" /D "NDEBUG" /D "WIN32" /D "_WINDOWS" /D "UNICODE" /Zl /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG" /d "USE_RICH_EDIT"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 libc.lib comctl32.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib /nologo /version:4.0 /subsystem:windows /pdb:none /machine:I386 /nodefaultlib /opt:nowin98
# SUBTRACT BASE LINK32 /map
# ADD LINK32 libc.lib comctl32.lib kernel32.lib user32.lib gdi32.lib comdlg32.lib advapi32.lib shell32.lib /nologo /version:4.0 /subsystem:windows /pdb:none /machine:I386 /nodefaultlib /opt:nowin98
# SUBTRACT LINK32 /map
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Cmds=bupx Release_RichEdit\metapad.exe	copy Release_RichEdit\metapad.exe c:\winnt\system32\notepad.exe
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "metapad - Win32 Debug"
# Name "metapad - Win32 Debug Unicode"
# Name "metapad - Win32 Debug RichEdit"
# Name "metapad - Win32 Debug RichEdit Unicode"
# Name "metapad - Win32 Release"
# Name "metapad - Win32 Release Unicode"
# Name "metapad - Win32 Release RichEdit"
# Name "metapad - Win32 Release RichEdit Unicode"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\metapad.c
# End Source File
# Begin Source File

SOURCE=.\metapad.rc

!IF  "$(CFG)" == "metapad - Win32 Debug"

!ELSEIF  "$(CFG)" == "metapad - Win32 Debug Unicode"

!ELSEIF  "$(CFG)" == "metapad - Win32 Debug RichEdit"

!ELSEIF  "$(CFG)" == "metapad - Win32 Debug RichEdit Unicode"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "metapad - Win32 Release"

!ELSEIF  "$(CFG)" == "metapad - Win32 Release Unicode"

!ELSEIF  "$(CFG)" == "metapad - Win32 Release RichEdit"

!ELSEIF  "$(CFG)" == "metapad - Win32 Release RichEdit Unicode"

# PROP BASE Exclude_From_Build 1

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\resource.h

!IF  "$(CFG)" == "metapad - Win32 Debug"

!ELSEIF  "$(CFG)" == "metapad - Win32 Debug Unicode"

!ELSEIF  "$(CFG)" == "metapad - Win32 Debug RichEdit"

!ELSEIF  "$(CFG)" == "metapad - Win32 Debug RichEdit Unicode"

# PROP BASE Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "metapad - Win32 Release"

!ELSEIF  "$(CFG)" == "metapad - Win32 Release Unicode"

!ELSEIF  "$(CFG)" == "metapad - Win32 Release RichEdit"

!ELSEIF  "$(CFG)" == "metapad - Win32 Release RichEdit Unicode"

# PROP BASE Exclude_From_Build 1

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\w32crt.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\drop_arrow.bmp
# End Source File
# Begin Source File

SOURCE=.\eye3.ico
# End Source File
# Begin Source File

SOURCE=.\linkhand.cur

!IF  "$(CFG)" == "metapad - Win32 Debug"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "metapad - Win32 Debug Unicode"

!ELSEIF  "$(CFG)" == "metapad - Win32 Debug RichEdit"

!ELSEIF  "$(CFG)" == "metapad - Win32 Debug RichEdit Unicode"

!ELSEIF  "$(CFG)" == "metapad - Win32 Release"

# PROP Exclude_From_Build 1

!ELSEIF  "$(CFG)" == "metapad - Win32 Release Unicode"

!ELSEIF  "$(CFG)" == "metapad - Win32 Release RichEdit"

!ELSEIF  "$(CFG)" == "metapad - Win32 Release RichEdit Unicode"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\pad2.ico
# End Source File
# Begin Source File

SOURCE=.\toolbar.bmp
# End Source File
# End Group
# Begin Source File

SOURCE=.\metapad.exe.manifest
# End Source File
# Begin Source File

SOURCE=.\metapad.manifest
# End Source File
# End Target
# End Project
