# overlord


# Myir FZ3 Xilinx Ultrascale+ based board

The Myir FZ3 is the board I'm using as the high spec Ikuy arcade board.
Its based on a Xilinx UltraScale+ ZU3EG Hybrid SoC with a large bank of fast memory, lots of cores and a large FPGA.
It costs about $300 including tax and delivery and comes with everything except a JTAG cable which costs ~$20 if you don't have one.

## Hardware Specs

### SoC / FPGA

#### A Xilinx Zynq Ultrascale+ ZU3EG Hybrid SoC and FPGA

* 4 x Arm A53 1.2GHz 64 bit AArch64 cores
* 2 x Arm R5F 500MHz 32 bit Armv7 cores
* 1 x (hard) Microblaze 180MHz 32 bit core
* 71K LUT (150K System Logic Cells), 360 DSPs, 7.6Mbit BRAM FPGA
* Display Port 1.2 hard core
* Mali-400 GPU (1 x vertex, 2 x Pixel)

7 CPU cores + 3 GPU cores (whether these will be practically useable due to driver/docs is another question)

FPGA DSP blocks give us around 200 GMACs for 28 bit integers,
the FPGA is big enough for 100 x 200Mhz small CPU cores, so lots of room to play with.

The hard DP 1.2 handling the monitor/tv outputs saves space in the FPGA,
The board itself has a mini-DP connector and comes with a adapter to HDMI.
It supports up 4K@30Hz and 12 bit colour per channel, though neither are my focus.
Its a 2 lane DP implementation with stereo audio (sadly no multichannel support), has 6 DMA channels from DDR with basic video and mixing.
Also has a dedicated channel to/from the FPGA which is where we can build our custom graphics chips.
As its a hard core, the FPGA logic doesn't need to be set up which is nice for early boot.

### Memory

#### 4 GB 2400Mhz 64bit DDR4 ram

Games always eat memory bandwidth, so this is one of the killer features of this board.
2400MT/s at 64 bit is huge for this level of board (Theoritical 19.2 GB/s transfer rate)
With no OS or other software, that 4GB is all ours which is plenty.

### Storage

* 8GB eMMC (200MB/s)
* SD 2.0 Card slot (the chip does support 3.0 but its not wired on this board)
* 32MB QSPI
* 32KB EEPROM

The QSPI is perfect size for a boot loader / monitor / game select.
The actual games can stored on eMMC or SD Card.
Its unclear what the EEPROM will be used for, but settings/user info would seem to be the obvious thing

IO

* 1 x USB 3.0 Host + 1 x USB 2 Host
* Gbit Ethernet
* 2 Camera ports
* I2C to various onboard chips

Both USB 2 and 3 means even complex input devices have plenty of space.
A hardware Gbit ethernet port allow internet and LAN multiplayer.
Supports MIPI CSI and another camera connection, if thats useful.
I2C is connected to PMIC, EEPROM and other auxilerly chips on the board itself

## Compared to 'golden era'

Thats a lot of power for a 2D arcade system, compared to golden era hardware its hard to comprehand how fast it is.
A arcade of the 80s might have had a 12MHz 68000 and a 3Mhz Z80 for the CPUs.
A few small custom graphics and sound chips with maybe a few MB of RAM and storage.
CPUs then weren't even single cycle pipelined designs, so just the CPUs alone have 100x the performance.
