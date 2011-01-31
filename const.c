// 
//  const.c
//  lpcflash
//  
//  Created by ths on 2011-01-17.
//  Copyright 2011 ths @ dev.io. All rights reserved.
// 

/* 
 #
 # Copyright 2011 Thorsten Schroeder < ths  (at)  dev  (dot)  io >. All rights reserved.
 #
 # Redistribution and use in source and binary forms, with or without modification, are
 # permitted provided that the following conditions are met:
 #
 #   1. Redistributions of source code must retain the above copyright notice, this list of
 #      conditions and the following disclaimer.
 #
 #   2. Redistributions in binary form must reproduce the above copyright notice, this list
 #      of conditions and the following disclaimer in the documentation and/or other materials
 #      provided with the distribution.
 #
 # THIS SOFTWARE IS PROVIDED BY THORSTEN SCHROEDER ``AS IS'' AND ANY EXPRESS OR IMPLIED
 # WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 # FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THORSTEN SCHROEDER OR
 # CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 # CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 # SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 # ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 # NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 # ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 # The views and conclusions contained in the software and documentation are those of the
 # authors and should not be interpreted as representing official policies, either expressed
 # or implied, of Thorsten Schroeder.
 #

 *
 * */
 
#include <stdlib.h>
#include <string.h>

const char *lpcisp_status_msg[] = {
   "CMD_SUCCESS",
   "INVALID_COMMAND",
   "SRC_ADDR_ERROR",
   "DST_ADDR_ERROR",
   "SRC_ADDR_NOT_MAPPED",
   "DST_ADDR_NOT_MAPPED",
   "COUNT_ERROR",
   "INVALID_SECTOR",
   "SECTOR_NOT_BLANK",
   "SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION",
   "COMPARE_ERROR",
   "BUSY",
   "PARAM_ERROR",
   "ADDR_ERROR",
   "ADDR_NOT_MAPPED",
   "CMD_LOCKED",
   "INVALID_CODE",
   "INVALID_BAUD_RATE",
   "INVALID_STOP_BIT",
   "CODE_READ_PROTECTION_ENABLED",
   NULL
};

/* array of arrays with start and end addresses of each sector 
 *    - this is necessary, since sector sizes are different
 */
const unsigned long sector_address[][2] = {
   {0x00000000,0x00000FFF},
   {0x00001000,0x00001FFF},
   {0x00002000,0x00002FFF},
   {0x00003000,0x00003FFF},
   {0x00004000,0x00004FFF},
   {0x00005000,0x00005FFF},
   {0x00006000,0x00006FFF},
   {0x00007000,0x00007FFF},
   {0x00008000,0x00008FFF},
   {0x00009000,0x00009FFF},
   {0x0000A000,0x0000AFFF},
   {0x0000B000,0x0000BFFF},
   {0x0000C000,0x0000CFFF},
   {0x0000D000,0x0000DFFF},
   {0x0000E000,0x0000EFFF},
   {0x0000F000,0x0000FFFF},
   {0x00010000,0x00017FFF},
   {0x00018000,0x0001FFFF},
   {0x00020000,0x00027FFF},
   {0x00028000,0x0002FFFF},
   {0x00030000,0x00037FFF},
   {0x00038000,0x0003FFFF},
   {0x00040000,0x00047FFF},
   {0x00048000,0x0004FFFF},
   {0x00050000,0x00057FFF},
   {0x00058000,0x0005FFFF},
   {0x00060000,0x00067FFF},
   {0x00068000,0x0006FFFF},
   {0x00070000,0x00077FFF},
   {0x00078000,0x0007FFFF}
};

