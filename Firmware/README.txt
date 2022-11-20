	       Firmware for Malahit SDR (model 1)
	       ----------------------------------

Bin files with the "_p" suffix contain my patches (example FW_1_10c_p)

Other bin files are only patched to work with any CPU ID (example FW_1_10c)


To program your Malahit radio:
------------------------------
- gunzip the binary file
- put the radio in DFU mode (either by button sequence if you have firmware
already running on it, or by temporarily bridging jumper JP1)
- then use the following command:
	dfu-util -R -a 0 -s 0x8000000 -D FW_xxxxx.bin

If you need .hex or .dfu files instead use the utilities from the parent
directory:

../stm2ihex.exe (converts bin to ihex)
../hex2dfu.exe	(converts ihex to DFU)
