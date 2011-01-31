// 
//  serial_cmd.h
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
 

#ifndef _SERIAL_CMD_H
#define _SERIAL_CMD_H

int serial_cmd_set_baudrate(int, unsigned long, unsigned int);       // B baudrate, stopbit
int serial_cmd_echo(int, unsigned int);                              // A setting
int serial_cmd_blankcheck_sector(int, unsigned int, unsigned int);   // I start sector, end sector

int serial_cmd_write_to_ram(int, unsigned int, unsigned int, unsigned char *);        // start addr, number of bytes, databuf
int serial_cmd_read_memory(int, char *,unsigned int, unsigned int);         // start addr, number of bytes
int serial_cmd_copy_ram_to_flash(int, unsigned int, unsigned int, unsigned int); // flash addr, ram addr, number   
int serial_cmd_go(int, unsigned int);                                            // addr
int serial_cmd_compare(int, unsigned int, unsigned int, unsigned int);           // addr1, addr2, number of bytes

int serial_cmd_read_partid(int, unsigned int *);
int serial_cmd_read_bootcode_version(int, unsigned int *);
int serial_cmd_read_device_serialno(int, unsigned int *);
int serial_cmd_prepare_sector(int, unsigned int, unsigned int);
int serial_cmd_unlock(int);
int serial_cmd_erase_sector(int, unsigned int, unsigned int);

void serial_cmd_remap_bootvect(int, unsigned long); 

int serial_synchronize(int, unsigned int);
int serial_retcode(char *, int);
int serial_check_ok(char *, int);

/* helper stuff ... move it */
void print_hex_ascii_line(const unsigned char *, int, int);

#endif
