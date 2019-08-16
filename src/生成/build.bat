
del ..\生成\*.bin   /F /Q
del ..\生成\*.hex   /F /Q

::pcb版本号
find  "PCB_VERSION" ..\app\config.h >tmp.txt
set pversion=""
set pver=""
for /f "tokens=2 delims==(" %%a in (tmp.txt) do (if %pversion%=="" set pversion=%%a) 
if %pver%==""  set pver=%pversion:~0,-2%

::软件版本号
find "#define SOFTWARE_VERSION" ..\app\config.h >tmp.txt
set version=""
set ver=""
for /f "tokens=2 delims==x" %%a in (tmp.txt) do (
  if %version%=="" set version=%%a
) 
if %ver%==""  set ver=%version:~0,4%

::硬件版本号
find "#define HARDWARE_VERSION" ..\app\config.h >tmp.txt
set hversion=""
set hver=""
for /f "tokens=2 delims==x" %%a in (tmp.txt) do (
  if %hversion%=="" set hversion=%%a
) 
if %hver%==""  set hver=%hversion:~0,4%

::输出名字
set name=%pver%-%hver%-%ver%
echo %name%
del tmp.txt

D:\Keil_v5\ARM\ARMCC\bin\fromelf.exe --bin --output=.\objects\v6.bin  .\objects\stm32f0-contiki.axf
cmd.exe /C copy .\objects\v6.bin               ..\生成\%name%.bin
cmd.exe /C copy .\objects\stm32f0-contiki.hex  ..\生成\%name%.hex
exit

