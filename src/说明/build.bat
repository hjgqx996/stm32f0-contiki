@echo off
::=========================================================================
::  请根据不同的电脑，设置自己的环境变量
set jflash=D:\"Program Files (x86)"\SEGGER\JLink_V630d\JFlash.exe
::set fromelf=D:\Keil\ARM\ARMCC\bin\fromelf.exe
set fromelf=D:\Keil\ARM\ARMCC\bin\fromelf.exe
::=========================================================================

::pcb版本号
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

::软件版本号
find "#define SOFTWARE_VERSION" %configfile% >tmp.txt
set version=""
set ver=""
for /f "tokens=2 delims==x" %%a in (tmp.txt) do (
  if %version%=="" set version=%%a
) 
if %ver%==""  set ver=%version:~0,4%

::硬件版本号
find "#define HARDWARE_VERSION" %configfile% >tmp.txt
set hversion=""
set hver=""
for /f "tokens=2 delims==x" %%a in (tmp.txt) do (
  if %hversion%=="" set hversion=%%a
) 
if %hver%==""  set hver=%hversion:~0,4%

::bootloader版本115200 19200
find "#define BAUDRATE" %configfile% >tmp.txt
set baud=""
set hbaud=""
for /f "tokens=3 delims== " %%a in (tmp.txt) do (
  if %hbaud%=="" set hbaud=%%a
) 
if %baud%==""  set baud=%hbaud%
set bootname=bootloader-%baud%
echo bootloader=%bootname%

::输出名字
set name=%pver%(%hver%-%ver%)
echo 文件名=%name%
del tmp.txt
del tmp1.txt

::发布版本
set /a hhver=0x%hver%
set /a vver=0x%ver%
set release=fw_slot_nas_t%hhver%_%vver%
set release0=fw_slot_nas_t0_%vver%
echo 发布名=%release%


::生成镜像的文件夹
set folder=%pver%
if not exist ..\生成\%folder% (
	md ..\生成\%folder%
)
del ..\生成\%folder%\*.bin   /F /Q
del ..\生成\%folder%\*.hex   /F /Q

%fromelf% --bin --output=.\objects\v6.bin  .\objects\stm32f0-contiki.axf
@echo on

::生成发布版本用于后台升级
cmd.exe /C copy .\objects\v6.bin               ..\生成\%folder%\%name%.bin
cmd.exe /C copy .\objects\v6.bin               ..\生成\%folder%\%release%.bin
cmd.exe /C copy .\objects\v6.bin               ..\生成\%folder%\%release0%.bin
cmd.exe /C copy .\objects\stm32f0-contiki.hex  ..\生成\%folder%\%name%.hex

::生成生产试产文件 bootloader+app
set bd=..\说明\bootloader\%bootname%.hex
set prj=..\说明\bootloader\stm32f03.jflash
%jflash% -openprj%prj% -open%bd% -merge..\生成\%folder%\%name%.hex -saveas..\生成\%folder%\%name%(工厂烧录).hex -exit
exit

