# Microsoft Developer Studio Project File - Name="rpg2kdev" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=rpg2kdev - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "rpg2kdev.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "rpg2kdev.mak" CFG="rpg2kdev - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "rpg2kdev - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "rpg2kdev - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "rpg2kdev - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x411 /d "NDEBUG"
# ADD RSC /l 0x411 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /out:"Bin/rpg2kdev.exe"

!ELSEIF  "$(CFG)" == "rpg2kdev - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD BASE RSC /l 0x411 /d "_DEBUG"
# ADD RSC /l 0x411 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /out:"Bin/rpg2kdev_d.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "rpg2kdev - Win32 Release"
# Name "rpg2kdev - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "SDK_cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SDK\CBer.cpp
# End Source File
# Begin Source File

SOURCE=.\SDK\CBerStream.cpp
# End Source File
# Begin Source File

SOURCE=.\SDK\CRpgArray.cpp
# End Source File
# Begin Source File

SOURCE=.\SDK\CRpgEvent.cpp
# End Source File
# Begin Source File

SOURCE=.\SDK\CRpgEventStream.cpp
# End Source File
# Begin Source File

SOURCE=.\SDK\CRpgImage.cpp
# End Source File
# Begin Source File

SOURCE=.\SDK\CRpgIOBase.cpp
# End Source File
# Begin Source File

SOURCE=.\SDK\CRpgLdb.cpp
# End Source File
# Begin Source File

SOURCE=.\SDK\CRpgLmu.cpp
# End Source File
# Begin Source File

SOURCE=.\SDK\CRpgLsd.cpp
# End Source File
# Begin Source File

SOURCE=.\SDK\CRpgUtil.cpp
# End Source File
# End Group
# Begin Group "sueLib_cpp"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\..\Lib\sueLib\CBasicStream.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Lib\sueLib\CImage\CImage.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Lib\sueLib\CImage\CImgUtil.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Lib\sueLib\CImage\CXyzIO.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Lib\sueLib\CZLib.cpp
# End Source File
# End Group
# Begin Source File

SOURCE=.\main.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "SDK_h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\SDK\CBer.h
# End Source File
# Begin Source File

SOURCE=.\SDK\CBerStream.h
# End Source File
# Begin Source File

SOURCE=.\SDK\CRpgArray.h
# End Source File
# Begin Source File

SOURCE=.\SDK\CRpgEvent.h
# End Source File
# Begin Source File

SOURCE=.\SDK\CRpgEventStream.h
# End Source File
# Begin Source File

SOURCE=.\SDK\CRpgImage.h
# End Source File
# Begin Source File

SOURCE=.\SDK\CRpgIOBase.h
# End Source File
# Begin Source File

SOURCE=.\SDK\CRpgLdb.h
# End Source File
# Begin Source File

SOURCE=.\SDK\CRpgLmu.h
# End Source File
# Begin Source File

SOURCE=.\SDK\CRpgLsd.h
# End Source File
# Begin Source File

SOURCE=.\SDK\CRpgUtil.h
# End Source File
# Begin Source File

SOURCE=.\SDK\index.h
# End Source File
# End Group
# Begin Group "sueLib_h"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\..\..\..\Lib\sueLib\CBasicStream.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Lib\sueLib\CImage\CBmpImage.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Lib\sueLib\CImage\CBmpIO.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Lib\sueLib\CDimension.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Lib\sueLib\CImage\CImage.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Lib\sueLib\CImage\CImageIO.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Lib\sueLib\CImage\CImgUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Lib\sueLib\CMapTable.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Lib\sueLib\CImage\CPngIO.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Lib\sueLib\CImage\CXyzIO.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Lib\sueLib\CZLib.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Lib\png.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Lib\pngconf.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Lib\sueLib\smart_array.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Lib\sueLib\smart_buffer.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Lib\sueLib\sueLib_Compile.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Lib\zconf.h
# End Source File
# Begin Source File

SOURCE=..\..\..\..\..\Lib\zlib.h
# End Source File
# End Group
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
