@echo off
::=========================================================================
::  ����ݲ�ͬ�ĵ��ԣ������Լ��Ļ�������
set jflash=D:\"Program Files (x86)"\SEGGER\JLink_V630d\JFlash.exe
::set fromelf=D:\Keil\ARM\ARMCC\bin\fromelf.exe
set fromelf=D:\Keil\ARM\ARMCC\bin\fromelf.exe
::=========================================================================

::pcb�汾��
set hconfig=""
set configfile=""
find "#define USING_HARDWARE_CONFIG" ..\app\config.h >tmp1.txt
for /f "tokens=3 delims== " %%a in (tmp1.txt) do (if %hconfig%=="" set hconfig=%%a) 
if %configfile%==""  set configfile="..\app\%hconfig:~1,-1%"

find  "PCB_VERSION"  %configfile% >tmp.txt
set pversion=""
set pver=""
for /f "tokens=2 delims==(" %%a in (tmp.txt) do (if %pversion%=="" set pversion=%%a) 
if %pver%==""  set pver=%pversion:~0,-2%

::����汾��
find "#define SOFTWARE_VERSION" %configfile% >tmp.txt
set version=""
set ver=""
for /f "tokens=2 delims==x" %%a in (tmp.txt) do (
  if %version%=="" set version=%%a
) 
if %ver%==""  set ver=%version:~0,4%

::Ӳ���汾��
find "#define HARDWARE_VERSION" %configfile% >tmp.txt
set hversion=""
set hver=""
for /f "tokens=2 delims==x" %%a in (tmp.txt) do (
  if %hversion%=="" set hversion=%%a
) 
if %hver%==""  set hver=%hversion:~0,4%

::bootloader�汾115200 19200
find "#define BAUDRATE" %configfile% >tmp.txt
set baud=""
set hbaud=""
for /f "tokens=3 delims== " %%a in (tmp.txt) do (
  if %hbaud%=="" set hbaud=%%a
) 
if %baud%==""  set baud=%hbaud%
set bootname=bootloader-%baud%
echo bootloader=%bootname%

::�������
set name=%pver%(%hver%-%ver%)
echo �ļ���=%name%
del tmp.txt
del tmp1.txt

::�����汾
set /a hhver=0x%hver%
set /a vver=0x%ver%
set release=fw_slot_nas_t%hhver%_%vver%
set release0=fw_slot_nas_t0_%vver%
echo ������=%release%


::���ɾ�����ļ���
set folder=%pver%
if not exist ..\����\%folder% (
	md ..\����\%folder%
)
del ..\����\%folder%\*.bin   /F /Q
del ..\����\%folder%\*.hex   /F /Q

%fromelf% --bin --output=.\objects\v6.bin  .\objects\stm32f0-contiki.axf
@echo on

::���ɷ����汾���ں�̨����
cmd.exe /C copy .\objects\v6.bin               ..\����\%folder%\%name%.bin
cmd.exe /C copy .\objects\v6.bin               ..\����\%folder%\%release%.bin
cmd.exe /C copy .\objects\v6.bin               ..\����\%folder%\%release0%.bin
cmd.exe /C copy .\objects\stm32f0-contiki.hex  ..\����\%folder%\%name%.hex

::���������Բ��ļ� bootloader+app
set bd=..\˵��\bootloader\%bootname%.hex
set prj=..\˵��\bootloader\stm32f03.jflash
%jflash% -openprj%prj% -open%bd% -merge..\����\%folder%\%name%.hex -saveas..\����\%folder%\%name%(������¼).hex -exit
exit

