Multitech AT Firmware version 4.1.5, compile with libmdot 4.1.33 and mbed-os 6.8.0

Tested with ARM GCC 9, use the 9-2020-q2-update release: https://developer.arm.com/downloads/-/gnu-rm

Linux x64: https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-x86_64-linux.tar.bz2?revision=05382cca-1721-44e1-ae19-1e7c3dc96118&rev=05382cca172144e1ae191e7c3dc96118&hash=FDE675133A099796BD1507A3FF215AC4

Windows: https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-win32.exe?revision=50c95fb2-67ca-4df7-929b-55396266b4a1&rev=50c95fb267ca4df7929b55396266b4a1&hash=141EB9F7934D9167A2ABCED1DD8B3979

Mac: https://developer.arm.com/-/media/Files/downloads/gnu-rm/9-2020q2/gcc-arm-none-eabi-9-2020-q2-update-mac.pkg?revision=05bece2b-0d55-4247-a46c-25d6539c5a6e&rev=05bece2b0d554247a46c25d6539c5a6e&hash=9717195FCAD882DDCC20991C119AB689

Set Mbed Studio to use ARM GCC: https://os.mbed.com/docs/mbed-studio/current/installing/switching-to-gcc.html

Mbed Studio may not be available and will not be supported after July 2026, so we need to sort out the CLI build process before then.

To build directly from Multitech's source:

Use Mercurial to clone their AT-Firmware: 'hg clone https://os.mbed.com/teams/MultiTech/code/Dot-AT-Firmware/'

Then switch to a version that works with the libmdot 4.1.x releases, which are recommended for the mDot: 'hg update -C 35'

Running 'hg summary' should return "parent: 35:e17e00b8e022"

Load this workspace into Mbed Studio, add libmdot (https://github.com/MultiTechSystems/libmDot) and mbed-os (https://github.com/ARMmbed/mbed-os) to the Libraries section. Select libmdot 4.1.33 and mbed-os 6.8.0, and compile for the mDot (MTS_MDOT_F411RE) target.

The compiled Dot-AT-Firmware.bin file should be within the build directory. It can be flashed onto a mDot that has programming headers installed using the USB devkit.

The mDot must be configured on the gateway for class C operation to function.

See pdf for Multitech docs.

YMODEM TRANSFER: if the mDot does not have programming headers installed you can still flash firmware using ymodem transfer. The ymodem.bin file is already set up for this.

If building a new firmware release: with a current bootloader on the mDot you need to add a CRC to the _application.bin firmware file created during compilation. Do this with the python mtsmultitool package:

`pip install mtsmultitool`

then: `multitool device crc -o output.bin mDot-AT-firmware_application.bin`

Now 'output.bin' is the file you want to transfer to the mDot.

Use the tio program. Run it in the directory your firmware file is in.

Connect USB UART adapter to mDot: power to VOD pin 1, ground to GND pin 10, white RX to USBTX pin 31, green TX to USBRX pin 30.

`tio /dev/ttyUSB0` or other device

To enter the bootloader you might need to hold down a key and reset the power on the mDot

Once the 'bootloader :>' prompt appears, type `transfer ymodem` and hit enter to tell the mDot you will start the transfer. 

ctrl-t y will tell tio you want to use ymodem transfer.

Enter the name of the file you want to transfer, in this example 'output.bin', and hit enter.

Tio will display dots to show the transfer progress and will print 'Done' once completed.

Now you can actually flash this firmware by entering `flash`. This will take a minute and once done it will restore the gen app key from OTP backup.

Disconnect from the USB debug and connect to the normal UART on the mDot, pins 2 and 3. Test and configure the firmware as usual.

ymodem related documentation:

https://os.mbed.com/teams/MultiTech/wiki/updating-firmware-using-MTS-bootloader

https://www.multitech.net/developer/forums/topic/flashing-firmware-compiled-using-mbed-onlineoffline-compiler-causes-crc-errors/

https://github.com/MultiTechSystems/bootloaders

https://pypi.org/project/mtsmultitool/

If the mDot is stuck in network join mode and failing to connect you can re-enter AT command mode by entering "+++" shortly after reseting the mDot. Alternatively enter the bootloader and erase lora.cfg 

https://www.multitech.net/developer/forums/topic/mdot-at-command-program-issue/

If there are issues connecting to the gateway you can try reseting the join nonces with 'at+jn=0,0' on the device and Chirpstack, or manually setting values for device address, network sesssion key, and application session key on the mDot using the Activation values in Chirpstack.
