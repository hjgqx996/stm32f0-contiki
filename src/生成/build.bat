@echo off
del ..\����\*.bin   /F /Q
del ..\����\*.hex   /F /Q

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

::�������
set name=%pver%-%hver%-%ver%
echo �ļ���=%name%
del tmp.txt
del tmp1.txt

D:\Keil_v5\ARM\ARMCC\bin\fromelf.exe --bin --output=.\objects\v6.bin  .\objects\stm32f0-contiki.axf
cmd.exe /C copy .\objects\v6.bin               ..\����\%name%.bin
cmd.exe /C copy .\objects\stm32f0-contiki.hex  ..\����\%name%.hex

::���������Բ��ļ� bootloader+app
set bd=..\����\bootloader\bootloader.hex
set prj=..\����\bootloader\stm32f03.jflash
set jflash=D:\"Program Files (x86)"\SEGGER\JLink_V630d\JFlash.exe
%jflash% -openprj%prj% -open%bd% -merge..\����\%name%.hex -saveas..\����\%name%(������¼).hex -exit
exit

