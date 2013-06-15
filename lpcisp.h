// 
//  lpcisp.h
//  lpcflash
//  
//  Created by ths on 2011-01-16.
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

 * */
 
#ifndef _LPCISP_H
#define _LPCISP_H

typedef enum _lpcisp_retcodes {
   CMD_SUCCESS,
   INVALID_COMMAND,
   SRC_ADDR_ERROR,
   DST_ADDR_ERROR,
   SRC_ADDR_NOT_MAPPED,
   DST_ADDR_NOT_MAPPED,
   COUNT_ERROR,
   INVALID_SECTOR,
   SECTOR_NOT_BLANK,
   SECTOR_NOT_PREPARED_FOR_WRITE_OPERATION,
   COMPARE_ERROR,       /* 10 */
   BUSY,
   PARAM_ERROR,
   ADDR_ERROR,
   ADDR_NOT_MAPPED,
   CMD_LOCKED,          /* 15 */
   INVALID_CODE,
   INVALID_BAUD_RATE,
   INVALID_STOP_BIT,
   CODE_READ_PROTECTION_ENABLED
} lpcisp_status_t;

#define PART_LPC1769 0x26113F37UL
#define PART_LPC1768 0x26013F37UL
#define PART_LPC1767 0x26012837UL
#define PART_LPC1766 0x26013F33UL
#define PART_LPC1765 0x26013733UL
#define PART_LPC1764 0x26011922UL
#define PART_LPC1763 0x26012033UL
#define PART_LPC1759 0x25113737UL
#define PART_LPC1758 0x25013F37UL
#define PART_LPC1756 0x25011723UL
#define PART_LPC1754 0x25011722UL
#define PART_LPC1752 0x25001121UL
#define PART_LPC1751 0x25001118UL
#define PART_LPC1343 0x3D00002BUL

typedef struct _cpu_info_t {
   unsigned int   id;
   unsigned int   serial[4];
   unsigned int   bootcode;
} cpu_info_t;

typedef struct _mem_info_t {
   cpu_info_t     cpu;
   unsigned int   rom_sector_base;
   unsigned long  rom_address_base;
   unsigned int   rom_sector_last;
   unsigned int   rom_total_size;
   unsigned int   img_sector_base;
   unsigned long  img_address_base;
   unsigned int   img_sector_last;
   unsigned int   img_total_size;
   unsigned long  ram_buffer_address;
   unsigned long  cclk;
} mem_info_t;

#define START  0
#define END    1

#endif