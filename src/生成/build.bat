
del ..\生成\*.bin   /F /Q
del ..\生成\*.hex   /F /Q

stm32f0-contiki.hex
D:\Keil_v5\ARM\ARMCC\bin\fromelf.exe --bin --output=.\objects\v6.bin  .\objects\stm32f0-contiki.axf
cmd.exe /C copy .\objects\v6.bin               ..\生成\LD-MZ-DRIVER-5-A-V6.bin
cmd.exe /C copy .\objects\stm32f0-contiki.hex  ..\生成\LD-MZ-DRIVER-5-A-V6.hex
