
del ..\生成\*.bin   /F /Q

D:\Keil_v5\ARM\ARMCC\bin\fromelf.exe --bin -o .\objects\v5.bin  .\objects\stm32f0-contiki.axf
cmd.exe /C copy .\objects\v5.bin  ..\生成\v5.bin
