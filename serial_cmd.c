// 
//  serial_cmd.c
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
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <err.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/time.h>
#include <ctype.h>

#include "serial.h"
#include "serial_cmd.h"
#include "chksum.h"
#include "lpcisp.h"
#include "base64.h"

extern char **lpcisp_status_msg;
extern const unsigned long sector_address[][2];

// W start addr, number of bytes
int serial_cmd_write_to_ram(int serial_fd, unsigned int ramstart, unsigned int count, unsigned char *data)
{
   unsigned char uuout[62];
   unsigned char *puuout = uuout;
   unsigned char *pdata = data;
   unsigned int len=0;
   unsigned int checksum=0;
   
   char buf[128];
   char cmd[128];

   unsigned int num_lines = 0;      // total number of lines
   unsigned int num_blocks = 0;     // total number of uublocks a 20 lines
   unsigned int lastblock_len = 0;  // total number of lines in last block
   unsigned int lastline_len = 0;   // length of last line in last block

   int ret=-1;
   
   int nl=0;
   int nb=0;
   int lbl=0;

   num_lines      = (unsigned int)(count / 45);       
   lastline_len   = (unsigned int)(count % 45);
   
   if (lastline_len)
      num_lines++;
      
   num_blocks     = (unsigned int)(num_lines / 20);
   lastblock_len  = (unsigned int)(num_lines % 20);
   
   memset(&buf,0,sizeof(buf));
   memset(&uuout,0,sizeof(uuout));
   
   // TODO check ramaddr word boundary -ths

   snprintf(cmd, sizeof(cmd), "W %d %d\r\n", ramstart, count);
   serial_send(serial_fd, strlen(cmd), cmd);
   
   len = serial_readline(buf, sizeof(buf), serial_fd);
   ret = serial_retcode(buf, len);
   len = 0;
   
   memset(&buf,0,sizeof(buf));
   len = 0;
   
   if(ret != CMD_SUCCESS)
      return ret;
   
   // first block(s)
   for(nb=0; nb<num_blocks;nb++) {
      checksum = serial_checksum_init();
      
      for(nl=0;nl<20;nl++) {
         
         memset(&uuout,0,sizeof(uuout));
         puuout = uuout;
         *(puuout++) = (45 + 32);
         
         checksum = serial_checksum_update(checksum, pdata, 45);
         
         raw_uu_encode(puuout, 45, pdata);
         
         pdata  += 45;
         
         memset(&buf,0,sizeof(buf));
         snprintf(buf, sizeof(buf), "%s\r\n", uuout);
         
         serial_send(serial_fd, strlen((char *)buf), buf);                  
      }
      
      //printf("checksum: %d\n",checksum);
      snprintf(cmd, sizeof(cmd), "%d\r\n", checksum);
      
      serial_send(serial_fd, strlen(cmd), cmd);
      len=serial_readline(buf, sizeof(buf), serial_fd);

      if(serial_check_ok(buf, len))
         ;
      else
         printf("E: checksum (%d) NOT OK!\n", checksum); // XXX need to resend stuff

      //ret = serial_retcode(buf, len);
      memset(&buf,0,sizeof(buf));
      len=0;
   }
   
   // send last block
   
   checksum = serial_checksum_init();
   
   for(lbl=0;lbl<(lastblock_len-1);lbl++) {
      unsigned int nwritten = 0;
      
      memset(&uuout,0,sizeof(uuout));
      puuout = uuout;
      
      // length
      *(puuout++) = (45 + 32);
      
      checksum = serial_checksum_update(checksum, pdata, 45);
      
      raw_uu_encode(puuout, 45, pdata);
         
      pdata  += 45;
      
      memset(&buf,0,sizeof(buf));
      snprintf(buf, sizeof(buf), "%s\r\n", uuout);
     
      nwritten = serial_send(serial_fd, strlen((char *)buf), buf);
      memset(&buf,0,sizeof(buf));

   }
         
   memset(&uuout,0,sizeof(uuout));
   puuout = uuout;
   *(puuout++) = (lastline_len + 32);
   
   checksum = serial_checksum_update(checksum, pdata, lastline_len);
   
   raw_uu_encode(puuout, lastline_len, pdata);
         
   pdata  += lastline_len;
   
   memset(&buf,0,sizeof(buf));
   snprintf(buf, sizeof(buf), "%s\r\n", uuout);
   
   serial_send(serial_fd, (unsigned int)strlen((char *)buf), buf);
   
   memset(&buf,0,sizeof(buf));

   // send checksum
   snprintf(cmd, sizeof(cmd), "%d\r\n", checksum);
   serial_send(serial_fd, strlen(cmd), cmd);
   len = serial_readline(buf, sizeof(buf), serial_fd);

   if(serial_check_ok(buf, len))
      ;
   else
      printf("E: checksum (%d) NOT OK!\n", checksum); // XXX need to resend stuff      
   
   //printf("\n---\nchksum check ret: \n");
   //print_hex_ascii_line((unsigned char *)buf, len, 0);

   memset(&buf,0,sizeof(buf));
   len=0;
   
   return ret;
}


/*
When LPC1700 BOOT ROM has been started, the BOOT ROM default reset & interrupt vectors are mapped to
FLASH ROM address 0x00000000. To access the original firmware's vector, the interrupt vector 
must be remapped, using the Register MEMMAP (0x400FC040) (bit0 = 1 <- usermode)

Therefore we must place the following code somewhere in memory and execute it, using the 
GO ISP cmd.

trying to get this into RAM, using some sort of shellcode-like code placement. after this 
code has been executed by the boot rom loader, the int vector is remapped and can be accessed
using ISP/IAP cmds.
-ths
*/

const static unsigned char remap_code[] = 
   "\x4c\xf2\x40\x03"      // movw r3, #0xC040
   "\xc4\xf2\x0f\x03"      // movt r3, #0x400F
   "\x4f\xf0\x01\x02"      // mov.w r2, #1
   "\x1a\x70"              // strb r2, [r3]
   "\x70\x47";             // bx lr

void serial_cmd_remap_bootvect(int serial_fd, unsigned long ramaddr) 
{
   printf("[+] remapping boot interrupt vector.\n");
   
   // inject code to ram
   serial_cmd_write_to_ram(serial_fd, ramaddr, 16, (unsigned char*)remap_code);
   serial_cmd_go(serial_fd, ramaddr);

   return;
}

// start addr, number of bytes
int serial_cmd_read_memory(int serial_fd, char *imgpath, unsigned int start, unsigned int count)
{
   char uuline[62];
   unsigned char rawdata[46]; // 61 byte uuencoded data can hold 45 byte data
   
   char buf[128];
   char cmd[128];
   int len=0;
   unsigned int checksum=0;
   unsigned int sumlen=0;
   unsigned char *memout = NULL;
   unsigned char *pmemout = NULL;
   int ret=-1;

   int bin_fd = 0;

   memout=(unsigned char *)malloc(count);
   if(memout != NULL)
      memset(memout,0,count);
      
   pmemout=memout;
   
   if(imgpath != NULL) {
      bin_fd = open(imgpath, O_TRUNC|O_CREAT|O_WRONLY, 0644);
      if(bin_fd < 0) {
         err(0, "open binary file");
         return -1;
      }
   } else
      bin_fd = -1;

   memset(&buf,0,sizeof(buf));
   len = 0;

   snprintf(cmd, sizeof(cmd), "R %d %d\r\n", start, count);
   serial_send(serial_fd, strlen(cmd), cmd);
   len = serial_readline(buf, sizeof(buf), serial_fd);
   
//   printf("\n---\nread mem ret: \n");
//   print_hex_ascii_line((unsigned char *)buf, len, 0);

   ret = serial_retcode(buf, len);
   memset(&buf,0,sizeof(buf));
   len = 0;

   checksum = serial_checksum_init();

   if(ret != CMD_SUCCESS)  {
      if(memout!=NULL)
         free(memout);
      return ret;
   }
      
   while(1) {
      
      int j=0;
      unsigned int binlen=0;
      unsigned int uulen=0;
   
      len=serial_readline(buf, sizeof(buf), serial_fd);
      
      //printf("\n---\nread mem ret: \n");
      //print_hex_ascii_line((unsigned char *)buf, len, 0);
               
      memset(&uuline,0,sizeof(uuline));
      memset(&rawdata,0,sizeof(rawdata));
   
      binlen = (buf[0]-32);                              // length of one line (max 45)
   
      for(j=0;j<sizeof(uuline);j++) {
         
         if(buf[j] == 0x0d)
            break;
         
         uuline[j]=(unsigned char)buf[j];
      }
      
      uulen = strlen(uuline);
      
      if(binlen > uulen) { // checksum
         
         if(uulen > 1) {
            int sent = 0; 
            int got_chksum = strtol(uuline, NULL, 10);
               
            if(got_chksum == checksum)
               sent = serial_send(serial_fd, 4, "OK\r\n");
            
            else { //XXX implement proper resend request handling!
               
               printf("\n[e] (%d > %d ?) sumlen=%d - count=%d - checksum mismatch: %x == %x\n", 
                     binlen, uulen, sumlen, count, got_chksum, checksum);
               
               sent = serial_send(serial_fd, 4, "RESEND\r\n");
            }
            
            checksum = serial_checksum_init();
            
            if(sumlen >= count) {
               break;
            }
            
            if(sizeof(buf)<sent)
               len=sizeof(buf);
            else
               len=sent;
         }

      } else if (uulen > 2) { // definitvely no checksum
         
         binlen   =  raw_uu_decode((unsigned char *)uuline, rawdata);
         checksum =  serial_checksum_update(checksum, rawdata, binlen);         
         sumlen   += binlen;
         printf("[*] read %08x of %08x Bytes from address %08x\r", sumlen, count, start);
         
         // if we have a valid file descriptor, write everything to file, otherwise hexdump
         //    all data to stdout
         if(bin_fd > 0)
            write(bin_fd, rawdata, binlen);
         else {
            if(memout!=NULL) {
               memcpy(pmemout, rawdata, binlen);
               pmemout += binlen;
            } else
               print_hex_ascii_line((unsigned char *)rawdata, binlen, sumlen-binlen);
         }
      }         
   }
   
   printf("\n");
   
   // if we don't have a valid file descriptor, hexdump all data to stdout 
   //   (as long as memout is not NULL) and free resources
   if((memout!=NULL) && (bin_fd < 0)) {
      print_hex_ascii_line(memout, sumlen, start);
      free(memout);
   }
   return ret;
}

// flash addr, ram addr, number   
int serial_cmd_copy_ram_to_flash(int serial_fd, unsigned int flash_addr, unsigned int ram_addr, unsigned int count)
{
   char buf[128];
   int len=0;
   char cmd[128];
   int ret=-1;
   
   snprintf(cmd, sizeof(cmd), "C %d %d %d\r\n", flash_addr, ram_addr, count);
   
   memset(&buf,0,sizeof(buf));
   len=0;
   
   serial_send(serial_fd, strlen(cmd), cmd);
   len=serial_readline(buf, sizeof(buf), serial_fd);   
   
   //printf("\n---\ncopy ram to flash ret: \n");
   //print_hex_ascii_line((unsigned char *)buf, len, 0);
   
   ret = serial_retcode(buf, len);
   
   if(ret != CMD_SUCCESS)
      printf("[e] failed cmd: %s => ret=%d\n", cmd, ret);

   return ret;
}


// addr, mode
int serial_cmd_go(int serial_fd, unsigned int addr)
{
   int ret=-1;
   
   char buf[128];
   char cmd[128];
   int len=0;
   
   memset(&buf,0,sizeof(buf));
   memset(&cmd,0,sizeof(cmd));
   
   len=0;

   snprintf(cmd, sizeof(cmd), "G %u T\r\n", addr);
   
   serial_send(serial_fd, strlen(cmd), cmd);
   len=serial_readline(buf, sizeof(buf), serial_fd);
   
   ret = serial_retcode(buf, len);
   
   if(ret != CMD_SUCCESS)
      printf("[e] failed cmd: %s => ret=%d\n", cmd, ret);
   
   return ret;
}
     
// addr1, addr2, number of bytes
int serial_cmd_compare(int serial_fd, unsigned int addr1, unsigned int addr2, unsigned int len)
{
   int ret=-1;

   /* implement me :-) */

   //if(ret != CMD_SUCCESS)
   //   printf("[e] failed cmd: %s => ret=%d\n", cmd, ret);
   
   return ret;
}


int serial_retcode(char *buf, int buflen) 
{
   int result=0;
   
   if(buflen < 3)
      return -1;
   
   if(buflen >= 4) { 
      if(isdigit(buf[buflen-4]))
         result=10*(buf[buflen-4]-'0');
   }
   
   if(isdigit(buf[buflen-3])) {
      result+=buf[buflen-3]-'0';  
   }
   return result;
}


int serial_cmd_prepare_sector(int serial_fd, unsigned int from_sector, unsigned int to_sector)
{
   char buf[128];
   int len=0;
   char cmd[128];
   int ret=-1;
   
   snprintf(cmd, sizeof(cmd), "P %d %d\r\n", from_sector, to_sector);
   
   //printf("D Prepare: %s\n", cmd);
   
   memset(&buf,0,sizeof(buf));
   len=0;

   serial_send(serial_fd, strlen(cmd), cmd);

   len=serial_readline(buf, sizeof(buf), serial_fd);
   ret = serial_retcode(buf, len);

   if(ret != CMD_SUCCESS)
      printf("[e] failed cmd: %s => ret=%d\n", cmd, ret);   
   
   return ret;
}


int serial_cmd_erase_sector(int serial_fd, unsigned int from_sector, unsigned int to_sector)
{
   char buf[128];
   int len=0;
   char cmd[128];
   int ret=-1;
   
   snprintf(cmd, sizeof(cmd), "E %d %d\r\n", from_sector, to_sector);
   
   memset(&buf,0,sizeof(buf));
   len=0;
   
   serial_send(serial_fd, strlen(cmd), cmd);

   len = serial_readline(buf, sizeof(buf), serial_fd);
   ret = serial_retcode(buf, len);
   
   if(ret != CMD_SUCCESS)
      printf("[e] failed cmd: %s => ret=%d\n", cmd, ret);
         
   return ret;
}


int serial_cmd_unlock(int serial_fd)
{
   char buf[128];
   int len=0;
   char cmd[] = "U 23130\r\n"; // this depends on the part or definition. XXX change it
   int ret=-1;
      
   memset(&buf,0,sizeof(buf));
   len=0;
   
   serial_send(serial_fd, strlen(cmd), cmd);

   len = serial_readline(buf, sizeof(buf), serial_fd);
   ret = serial_retcode(buf, len);

   if(ret != CMD_SUCCESS)
      printf("[e] failed cmd: %s => ret=%d\n", cmd, ret);
         
   return ret;
   
}


int serial_cmd_read_device_serialno(int serial_fd, unsigned int *ser)
{
   char buf[128];
   int len = 0;
   char cmd[] = "N\r\n";
   int ret = -1;
   int i = 0;
   
   memset(&buf,0,sizeof(buf));
   len = 0;

   serial_send(serial_fd, strlen(cmd), cmd);
   
   while(ret != CMD_SUCCESS) {
      
      len = serial_readline(buf, sizeof(buf), serial_fd);
      
      ret = serial_retcode(buf, len);
      memset(&buf,0,sizeof(buf));
      len = 0;
   }
   
   for(i=0;i<4;i++) {
      len = serial_readline(buf, sizeof(buf), serial_fd);
      ser[i]=(unsigned int)atoi(buf);      
   }

   if(ret != CMD_SUCCESS)
      printf("[e] failed cmd: %s => ret=%d\n", cmd, ret);

   return ret;
}


int serial_cmd_read_bootcode_version(int serial_fd, unsigned int *ver)
{
   char buf[128];
   int len=0;
   char cmd[] = "K\r\n";
   int ret=-1;
   unsigned int result=0;
   
   memset(&buf,0,sizeof(buf));
   len=0;

   serial_send(serial_fd, strlen(cmd), cmd);
   
   while(ret != CMD_SUCCESS) {
      len = serial_readline(buf, sizeof(buf), serial_fd);
      ret = serial_retcode(buf, len);
      memset(&buf,0,sizeof(buf));
      len = 0;
   }
   
   // read response 0 - partid
   len = serial_readline(buf, sizeof(buf), serial_fd);

   result = (unsigned int)atoi(buf);
   
   memcpy(ver, &result, 4);
   
   if(ret != CMD_SUCCESS)
      printf("[e] failed cmd: %s => ret=%d\n", cmd, ret);

   return ret;
}


int serial_cmd_read_partid(int serial_fd, unsigned int *id)
{
   char buf[128];
   int len=0;
   char cmd[] = "J\r\n";
   int ret=-1;
   unsigned int result=0;
   
   memset(&buf,0,sizeof(buf));
   len=0;
   
   serial_send(serial_fd, strlen(cmd), cmd);
   
   while(ret != CMD_SUCCESS) {
      len = serial_readline(buf, sizeof(buf), serial_fd);
      ret = serial_retcode(buf, len);
      memset(&buf,0,sizeof(buf));
      len = 0;
   }
   
   // read response 0 - partid
   len = serial_readline(buf, sizeof(buf), serial_fd);

   result = (unsigned int)atoi(buf);
   
   memcpy(id, &result, 4);
   
   if(ret != CMD_SUCCESS)
      printf("[e] failed cmd: %s => ret=%d\n", cmd, ret);

   return ret;
}

void print_hex_ascii_line(const unsigned char *payload, int slen, int offset)
{
   int i;
   int gap, len;
   const unsigned char *ch, *ch2;

   len = 16;
   ch2 = ch = payload;

   do {

      /* offset */
      printf("%05x   ", offset);

      /* hex */
      if(slen < len)
         len=slen;

      for(i = 0; i < len; i++) {
         printf("%02x ", *ch);
         ch++;
         /* print extra space after 8th byte for visual aid */
         if (i == 7)
            putc(' ', stdout);
      }
      /* print space to handle line less than 8 bytes */
      if (len < 8)
         putc(' ', stdout);

      /* fill hex gap with spaces if not full line */
      if (len < 16) {
         gap = 16 - len;
         for (i = 0; i < gap; i++) {
            printf("   ");
         }
      }          
      printf("   ");

      /* ascii (if printable) */
      for(i = 0; i < len; i++) {
         
         if (isprint(*ch2)) {
            printf("%c", *ch2);
         } else
            putc(' ', stdout);
         ch2++;
      }

      printf("\n\r");
      slen -= 16;
      offset += len;
   } while( slen > 0 );
   return;
}

//int serial_cmd_set_baudrate(int, unsigned long, unsigned int);       // B baudrate, stopbit
int serial_cmd_set_baudrate(int serial_fd, unsigned long baudrate, unsigned int stopbit)
{
   char buf[128];
   int len=0;
   char cmd[128];
   int ret=-1;

   if(stopbit > 1)
      stopbit = 2;

   snprintf(cmd, sizeof(cmd), "B %ld %d\r\n", baudrate, stopbit);

   memset(&buf,0,sizeof(buf));
   len = 0;

   serial_send(serial_fd, strlen(cmd), cmd);

   len = serial_readline(buf, sizeof(buf), serial_fd);
   ret = serial_retcode(buf, len);
    
   if(ret != CMD_SUCCESS)
      printf("[e] failed cmd: %s => ret=%d\n", cmd, ret);
       
   return ret;
}

//int serial_cmd_echo(int, unsigned int);                              // A setting
int serial_cmd_echo(int serial_fd, unsigned int echo)
{
   char buf[128];
   int len = 0;
   char cmd[128];
   int ret = -1;

   if(echo > 0)
      echo = 1;

   snprintf(cmd, sizeof(cmd), "A %d\r\n", echo);

   memset(&buf,0,sizeof(buf));
   len = 0;

   serial_send(serial_fd, strlen(cmd), cmd);

   len = serial_readline(buf, sizeof(buf), serial_fd);
   ret = serial_retcode(buf, len);

   if(ret != CMD_SUCCESS)
      printf("[e] failed cmd: %s => ret=%d\n", cmd, ret);
       
   return ret; 
}

/*
 * This command is used to blank check one or more sectors of on-chip flash memory.
 */
int serial_cmd_blankcheck_sector(int serial_fd, unsigned int start_sector, unsigned int end_sector)
{
   char buf[128];
   char cmd[128];
   int len = 0;
   int ret = -1;

   /* End Sector Number: Should be greater than or equal to start sector number. */
   if(end_sector > start_sector)
      return PARAM_ERROR;

   snprintf(cmd, sizeof(cmd), "I %d %d\r\n", start_sector, end_sector);

   memset(&buf,0,sizeof(buf));
   len = 0;

   serial_send(serial_fd, strlen(cmd), cmd);

   len = serial_readline(buf, sizeof(buf), serial_fd);
   ret = serial_retcode(buf, len);
    
   if(ret != CMD_SUCCESS)
      printf("[e] failed cmd: %s => ret=%d\n", cmd, ret);
       
   return ret;  
}

int serial_check_ok(char *buf, int buflen) 
{
   if(buflen < 4)
      return 0;
   return memcmp(&buf[buflen-4], "OK\r\n", 4) == 0;
}

int serial_synchronize(int serial_fd, unsigned int freq) 
{
   char buf[128];
   int len = 0;

   memset(&buf,0,sizeof(buf));
   serial_send(serial_fd, 1, "?");
   len = serial_readline(buf, sizeof(buf), serial_fd);

   serial_send(serial_fd, len, buf);
   memset(&buf,0,sizeof(buf));
   len = serial_readline(buf, sizeof(buf), serial_fd);

   if(!serial_check_ok(buf, len))
      return -1;

   memset(&buf,0,sizeof(buf));
   len = 0;

   snprintf(buf, sizeof(buf), "%d\r\n", freq);
   serial_send(serial_fd, (int)strlen(buf), buf);

   memset(&buf,0,sizeof(buf));
   len = 0;

   len=serial_readline(buf, sizeof(buf), serial_fd);

   if(!serial_check_ok(buf, len))
      return -1;

   return 0;
}


