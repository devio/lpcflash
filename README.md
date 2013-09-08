# lpcflash (beta) 2013/09/08
### an opensource flash utility for NXP LPC17xx ARM CM3 series

Currently supporting:
  - boot rom/on-chip serial bootloader.
  - all lpc1700 boot rom loader compatible bootloaders, such as the usb cdc secondary bootloader included in the opensource LPCboot Suite (will be released soon)
 
lpcflash was written by Thorsten Schroeder <ths (at) dev (dot) io> and is released under the 2-clause BSD license. Even though it is released under a very flexible and free licence, feedback and feature requests are highly appreciated.

#### PLATFORMS

lpcflash was tested under Mac OSX 10.6.6 and Ubuntu 10.04 LTS. Target platforms are all LPC1700 based ARM Coretex-M3 processors.

#### BUILD

```
$ make
gcc -Wall -g     -c -o lpcflash.o lpcflash.c
gcc -Wall -g     -c -o serial.o serial.c
gcc -Wall -g     -c -o base64.o base64.c
gcc -Wall -g     -c -o serial_cmd.o serial_cmd.c
gcc -Wall -g     -c -o msg.o msg.c
gcc -Wall -g     -c -o const.o const.c
gcc -Wall -g     -c -o chksum.o chksum.c
gcc -Wall -g   -o lpcflash lpcflash.o serial.o base64.o serial_cmd.o msg.o const.o chksum.o
```

#### BUILD w/libftdi support (requires libftdi)

```
LIBS="$(libftdi1-config --libs)" INCS="$(libftdi1-config --cflags)" make libftdi
```


#### USAGE

```
$ ./lpcflash 
usage: lpcflash
	 -l  <serial line>   (8N1 cu device)
	 -b  <baudrate>      (default 115200UL)
	 -f  <infile>        (.bin file)
	 -o  <outfile>
	 -c  <cclk in kHz>   (default: 100000UL)
	 -A                  (all flash sectors)
	 -R  <RAM address>   (default: 0x10000400UL)
	 -F  <FLASH from sector>
	 -T  <FLASH to sector>
	 -B  <FLASH base sector>
	 -a                  (ram/rom memory address in hex)
	 -s                  (only for -a: size in hex - word aligned)
	[-v]                 (be verbose)
	[-h]                 ( _o/ )
	[-i]                 (dump cpu infos)
	[-e]                 (erase only (== erase and exit))
[-] More infos at https://project.dev.io/code/arm/
```

#### FEATURES

+ Erase all flash ROM sectors
+ Erase selected flash ROM sectors/ranges
+ Dump/Read flash ROM and SRAM memory (if not protected by CRP) by sector/ranges
+ Dump/Read flash ROM and SRAM memory (if not protected by CRP) by addresses/ranges
+ Download/Write flash ROM and SRAM memory (if not protected by CRP) by sector/ranges
+ Download/Write flash ROM and SRAM memory (if not protected by CRP) by addresses/ranges

Everything that is specified within the LPC17xx user documentation (ISP/IAP (flash) memory programming) has also been implemented in the USB CDC secondary bootloader firmware to be able using lpcflash also with this opensource bootloader skeletton (this also allows some kind of crypto layer for integrity checks of the firmware etc).

EXAMPLES:

#### Erase all sectors on an LPC1756 using the on-chip bootloader via serial line

```
$ ./lpcflash -l /dev/cu.usbserial-A1004cdd -b 115200 -e -A
[*] 
[*] Detected part ID = 0x25011723:
[*]    Total number of 4k/32k ROM sectors: 0x15
[*]    Part serial no: 0x05050502 53360752 4A83D292 F5000003
[*]         Boot code: 0x1
[*] 
[+] erasing sectors 0 - 21 - done.
```

#### Write a firmware image file to flash ROM sector 0, using the LPC1756 on-chip bootloader via serial line

```
$ ./lpcflash -l /dev/cu.usbserial-A1004cdd -f testbinaries/keykerikiv2-blinke-flashplace-0000h.bin
[*] 
[*] Detected part ID = 0x25011723:
[*]    Total number of 4k/32k ROM sectors: 0x15
[*]    Part serial no: 0x05050502 53360752 4A83D292 F5000003
[*]         Boot code: 0x1
[*] 
[+] erasing sectors 0 - 0 - done.
[*] Firmware image testbinaries/keykerikiv2-blinke-flashplace-0000h.bin:
[*]       Size (bytes): 0x1000
[*]   First ROM sector: 0x0
[*]    Last ROM sector: 0x1 of 0x15
[*] 
[-] current sector: 0x00 - 0x00000200 of 0x00000E00 bytes written
[-] current sector: 0x00 - 0x00000400 of 0x00000E00 bytes written
[-] current sector: 0x00 - 0x00000600 of 0x00000E00 bytes written
[-] current sector: 0x00 - 0x00000800 of 0x00000E00 bytes written
[-] current sector: 0x00 - 0x00000A00 of 0x00000E00 bytes written
[-] current sector: 0x00 - 0x00000C00 of 0x00000E00 bytes written
[-] current sector: 0x00 - 0x00000E00 of 0x00000E00 bytes written
[*] done.
```

#### hexdump (to stdout) flash ROM address 0x00000000-0x00001000 of an LPC1756 using the on-chip bootloader via serial line

```
$ ./lpcflash -l /dev/cu.usbserial-A1004cdd -b 115200 -a 0x00000000 -s 0x1000
[*] 
[*] Detected part ID = 0x25011723:
[*]    Total number of 4k/32k ROM sectors: 0x15
[*]    Part serial no: 0x05050502 53360752 4A83D292 F5000003
[*]         Boot code: 0x1
[*] 
[+] remapping boot interrupt vector.
[*] read 00001000 of 00001000 Bytes from address 00000000
00000   2c 01 00 10 e5 01 00 00  f7 01 00 00 f9 01 00 00    ,               
00010   fb 01 00 00 fd 01 00 00  ff 01 00 00 08 f3 ff ef                    
00020   00 00 00 00 00 00 00 00  00 00 00 00 01 02 00 00                    
00030   03 02 00 00 00 00 00 00  83 0d 00 00 25 04 00 00                %   
00040   09 02 00 00 65 02 00 00  0d 02 00 00 0f 02 00 00        e           
00050   11 02 00 00 13 02 00 00  15 02 00 00 17 02 00 00                    
00060   19 02 00 00 1b 02 00 00  1d 02 00 00 1f 02 00 00                    
00070   21 02 00 00 23 02 00 00  25 02 00 00 27 02 00 00    !   #   %   '   
00080   29 02 00 00 2b 02 00 00  2d 02 00 00 2f 02 00 00    )   +   -   /   
00090   31 02 00 00 33 02 00 00  35 02 00 00 37 02 00 00    1   3   5   7   
000a0   39 02 00 00 3b 02 00 00  3d 02 00 00 3f 02 00 00    9   ;   =   ?   
000b0   41 02 00 00 43 02 00 00  45 02 00 00 47 02 00 00    A   C   E   G   
000c0   49 02 00 00 4b 02 00 00  4d 02 00 00 2b 49 8d 46    I   K   M   +I F
000d0   2b 49 2c 48 0a 1a 04 d0  81 f3 09 88 02 22 82 f3    +I,H         "  
[...]
```

#### hexdump (to stdout) 256 bytes of SRAM address 0x10000000 of an LPC1756 using the on-chip bootloader via serial line

```
$ ./lpcflash -l /dev/cu.usbserial-A1004cdd -b 115200 -a 0x10000000 -s 0x100
[*] 
[*] Detected part ID = 0x25011723:
[*]    Total number of 4k/32k ROM sectors: 0x15
[*]    Part serial no: 0x05050502 53360752 4A83D292 F5000003
[*]         Boot code: 0x1
[*] 
[+] remapping boot interrupt vector.
[*] read 00000100 of 00000100 Bytes from address 10000000
10000000   00 e1 f5 05 01 00 00 00  00 00 00 00 00 00 00 00                    
10000010   00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00                    
10000020   00 00 00 00 00 00 00 00  00 00 00 00 00 00 00 00                    
10000030   80 00 00 00 94 18 48 9c  fd 3b 41 f4 49 02 24 69          H  ;A I $i
10000040   b4 8c 9c f8 49 27 0c 44  7c a3 67 ee fa c3 ba cf        I' D| g     
10000050   84 e5 fc 2c a2 49 10 c3  22 9f f7 b9 e5 00 70 b5       , I  "     p 
10000060   90 23 9a ec 81 0b 09 f1  8e d1 18 b4 39 08 4a 81     #          9 J 
10000070   8d 71 6d f8 16 08 2b 93  0c 2a 13 51 02 d7 88 9a     qm   +  * Q    
10000080   b8 77 43 35 72 e1 30 4c  85 f7 1e 04 1b 10 08 69     wC5r 0L       i
10000090   7d 05 15 78 17 84 00 18  48 ef b8 2e 30 df a0 d0    }  x    H  .0   
100000a0   b6 21 06 b9 d1 ba 6b 6c  4c 25 99 c9 a2 00 88 a1     !    klL%      
100000b0   64 24 a2 b7 78 1e c2 09  99 53 89 ee a9 29 1b b2    d$  x    S   )  
100000c0   9f 8f 4c 89 db 0f 54 a1  db 72 b5 5a 01 dc 8c 80      L   T  r Z    
100000d0   22 bb 93 b4 db 19 80 82  c8 fb d1 0e 99 98 2f cc    "             / 
100000e0   9b e1 40 0e 5b 54 b2 51  44 ad fb 01 41 96 45 6d      @ [T QD   A Em
100000f0   fc 5a 84 cf 2d cb fe 28  07 6b 75 a6 f8 c0 64 32     Z  -  ( ku   d2
```

#### Dumping all flash ROM sectors of an LPC1756 to file, starting at flash ROM base sector 0

```
$ ./lpcflash -l /dev/cu.usbserial-A1004cdd -b 115200 -A -B 0 -o flashrom.bin
[*] 
[*] Detected part ID = 0x25011723:
[*]    Total number of 4k/32k ROM sectors: 0x15
[*]    Part serial no: 0x05050502 53360752 4A83D292 F5000003
[*]         Boot code: 0x1
[*] 
[*] ROM image flashrom.bin:
[*]       Size (bytes): 0x3F000
[*]   First ROM sector: 0x0
[*]    Last ROM sector: 0x15
[*] 
[+] remapping boot interrupt vector.
[+] reading sector 0 - 21. Starting at address 00000000, reading 262144 bytes
[*] read 00040000 of 00040000 Bytes from address 00000000
```

#### Why?

The Official FlashMagic Tool for this task, which is recommended and referred to by NXP is available only for Microsoft Windows operating systems. There is no sourcecode available for porting it to different platforms, and the usage scenarios are quite restricted for non-paying customers. The free FlashMagicTool version is quite restrictive. LPCFlash utility is available as source code under the BSD license. It has been tested with Mac OSX and Linux. A complete list of supported platforms is available in the source code.

#### Where?


Please be aware, that this version is a very early beta testing version. Please send wishes, recommendations, bug-reports and comments to me, using email. Every feedback is highly appreciated!

#### What else?

I'm also working on an open-source LPCboot Suite

#### Support?

Hardware and Developer Boards are expensive. Hardware donations are highly appreciated...
