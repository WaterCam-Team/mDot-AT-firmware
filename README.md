Multitech AT Firmware version 4.1.5, compile with libmdot 4.1.18 and mbed-os 6.8

Can be compiled using ARM compiler through Mbed Studio on a local machine, with mDot as target device. 

GCC 9 also works: https://developer.arm.com/downloads/-/gnu-rm

https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2?revision=05382cca-1721-44e1-ae19-1e7c3dc96118&rev=05382cca172144e1ae191e7c3dc96118&hash=FDE675133A099796BD1507A3FF215AC4

https://os.mbed.com/docs/mbed-studio/current/installing/switching-to-gcc.html

Fails to compile in Keil Studio online editor and needs cmake configuration to compile with mbed-tools workflow.

Mbed Studio will not be available after July 2026, so we need to sort out the mbed CLI build process before then.
