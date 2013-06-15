// 
//  lpcflash.c
//  lpcflash tool main file
//  
//  Created by ths on 2011-01-30.
//  Copyright 2011 ths @ dev.io. All rights reserved.
// 

/* 
 * lpcflash - an LPC17xx bootloader IAP/ISP flash programming tool
 * 
 * Thorsten Schroeder <ths  (at)   dev (dot) io>
 * 20110130, Berlin, Germany
 *
 
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

 *
 * */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <getopt.h>

#include <err.h>

#include "serial.h"
#include "serial_cmd.h"

#include "lpcisp.h"

// XXX THS: Todo: parse SP and PC value at binary file offset 0 to get sector base address
#define DEFAULT_SECTOR_BASE      0
#define DEFAULT_ROM_BASE_ADDR    (DEFAULT_SECTOR_BASE * 4096)
#define DEFAULT_BAUDRATE         115200UL
#define DEFAULT_DEVICE_CCLK      100000UL
#define DEFAULT_RAMBUFFER_ADDR   0x10000400UL

extern char **lpcisp_status_msg;

typedef enum _lpcflash_cmds {
      LPC_NONE,
      LPC_INFO,
      LPC_ERASE,
      LPC_DUMPFLASH,
      LPC_WRITEFLASH,
      LPC_READMEM
} lpcflash_cmd;

static unsigned int lpcflash_get_lastsector(unsigned int);
static void lpcflash_dumpinfo(mem_info_t *);
static void lpcflash_help(void);
static void lpcflash_erase(int, mem_info_t *);
static int lpcflash_image_dump(int, mem_info_t *, char *);
static int lpcflash_image_write(int, mem_info_t *, char *);

extern const unsigned long sector_address[][2];

int main(int argc, char **argv)
{
   char *device      = NULL;
   char *binfile     = NULL;
   char *outfile     = NULL;
   
   char c = 0;

   int serial_fd  = 0;
   int   verbose  = 0;
   
   mem_info_t  mem;

   unsigned int opt_flash_sector_all       = 0;
   unsigned int opt_flash_sector_from      = 0;
   unsigned int opt_flash_sector_to        = 0;

   unsigned long read_addr=0;
   unsigned long read_size=32;

   unsigned long baudrate           = DEFAULT_BAUDRATE;
   unsigned int  opt_lpcflash_cmd   = LPC_NONE;

   memset(&mem, 0, sizeof(mem));
   
   mem.ram_buffer_address  = DEFAULT_RAMBUFFER_ADDR;
   mem.rom_sector_base     = DEFAULT_SECTOR_BASE;
   mem.rom_address_base    = DEFAULT_ROM_BASE_ADDR;
   mem.cclk                = DEFAULT_DEVICE_CCLK;
   
   /* parse command line */
   while( ( c = getopt(argc, argv, "l:b:f:c:o:hvieAR:F:T:B:a:s:") ) != -1 ) {
      switch(c) {
         
         case 'l': /* line - cu/tty device file */
            device = optarg;
            break;

         case 'b': /* baudrate */
            baudrate = (int)atoi(optarg);
            break;
            
         case 'f': /* infile */
            binfile = optarg;
            opt_lpcflash_cmd = LPC_WRITEFLASH;
            break;
         
         case 'c': /* cclk in kHz */
            mem.cclk = (unsigned long)atol(optarg);;
            break;
                        
         case 'o': /* outfile */      
            outfile = optarg;
            opt_lpcflash_cmd = LPC_DUMPFLASH;
            break;
         
         case 'R': /* RAM buffer address */
	         mem.ram_buffer_address = (unsigned long)atol(optarg);
            break;
         
         case 'F': /* Flash ROM _F_rom sector */
            mem.img_sector_base = opt_flash_sector_from = (int)atoi(optarg);
            break;
            
         case 'T': /* Flash ROM _T_o sector */
            mem.img_sector_last = opt_flash_sector_to = (int)atoi(optarg);
            break;
            
         case 'A': /* Flash ROM _A_ll sectors */
            opt_flash_sector_all = 1;
            break;
         
         case 'B': /* Flash ROM sector _B_ase */
            mem.rom_sector_base  = (int)atoi(optarg);
            mem.rom_address_base = (unsigned long)(mem.rom_sector_base * 0x1000);
            break;
         
         case 'a': /* read mem addr */
            read_addr = (unsigned long)strtol(optarg, NULL, 16);
            opt_lpcflash_cmd = LPC_READMEM;
            
            break;
         case 's': /* read size bytes at mem addr */
            read_size = (unsigned long)strtol(optarg, NULL, 16);
            break;
             
         case 'v': /* verbose */
            verbose=1;
            break;            
            
         case 'i': /* dump cpu info */
            opt_lpcflash_cmd = LPC_INFO;
            break;            
         
         case 'e': /* erase flash rom and exit */
            opt_lpcflash_cmd = LPC_ERASE;
            break;
         
         case 'h': /* help */
         default:
            lpcflash_help();
            return 0;
      }
   }

   if ( !(
         (device) &&
         (binfile || outfile || opt_lpcflash_cmd)
         ) ) {
      lpcflash_help();
      exit(1);
   }
      
   /* open serial port */
   serial_fd = serial_open(device, baudrate);
   
   if(serial_fd<0) 
      return(1);

   serial_synchronize(serial_fd, mem.cclk);
   serial_cmd_read_partid(serial_fd, &mem.cpu.id);
   serial_cmd_read_bootcode_version(serial_fd, &mem.cpu.bootcode);
   serial_cmd_read_device_serialno(serial_fd, mem.cpu.serial);

   if(opt_flash_sector_from < mem.rom_sector_base)
      opt_flash_sector_from = mem.rom_sector_base;
      
   if(opt_flash_sector_to > lpcflash_get_lastsector(mem.cpu.id))
      opt_flash_sector_to = lpcflash_get_lastsector(mem.cpu.id);
      
   else if(opt_flash_sector_to < opt_flash_sector_from)
      opt_flash_sector_to = opt_flash_sector_from;
   
   mem.img_sector_base  = opt_flash_sector_from;
   mem.img_sector_last  = opt_flash_sector_to;
   mem.rom_sector_last  = lpcflash_get_lastsector(mem.cpu.id);
   mem.rom_address_base = mem.rom_sector_base * 0x1000;

   
   serial_cmd_unlock(serial_fd);

   if(opt_flash_sector_all != 0) {
      mem.img_sector_last = mem.rom_sector_last;
      mem.img_sector_base = mem.rom_sector_base;
   }

   lpcflash_dumpinfo(&mem);
   
   if (opt_lpcflash_cmd == LPC_ERASE) {
      
      // erase only
      lpcflash_erase(serial_fd, &mem);
      close(serial_fd);
      return 0;
      
   } else if(opt_lpcflash_cmd == LPC_INFO) {
      
      // dump info only
      close(serial_fd);
      return 0;
        
   } else if(opt_lpcflash_cmd == LPC_WRITEFLASH) {
      
      lpcflash_erase(serial_fd, &mem);
      lpcflash_image_write(serial_fd, &mem, binfile);

   } else if (opt_lpcflash_cmd == LPC_DUMPFLASH) {
      
      lpcflash_image_dump(serial_fd, &mem, outfile);

   } else if (opt_lpcflash_cmd == LPC_READMEM) {
      // echo off
      serial_cmd_echo(serial_fd, 0);
      serial_cmd_remap_bootvect(serial_fd, mem.ram_buffer_address);
      serial_cmd_read_memory(serial_fd, outfile, read_addr, read_size);
   }
   
   close(serial_fd);
   return(0);
}



static void lpcflash_erase(int serial_fd, mem_info_t *mem)
{
   /* erase [entire] (user) flash rom */
   printf("[+] erasing sectors %ld - %d",  mem->rom_address_base, mem->rom_sector_last);
   serial_cmd_prepare_sector(serial_fd,   mem->rom_address_base, mem->rom_sector_last);
   serial_cmd_erase_sector(serial_fd,     mem->rom_address_base, mem->rom_sector_last);
   printf(" - done.\n");   
   
   return;
}

static int lpcflash_image_write(int serial_fd, mem_info_t *mem, char *imgpath)
{
   
   struct stat bin_stat;
   unsigned char outbuf[512];
   unsigned int nwritten            = 0;
   unsigned int i                   = 0;
   
   int bin_fd = open(imgpath, O_RDONLY);
   if(bin_fd < 0) {
      err(0, "open binary file");
      return -1;
   }

   if(fstat(bin_fd, &bin_stat) < 0) {
      err(0, "fstat binary file");
      return -1;
   }

   /* 
    * sector size  4k from sector 0x00 - 0x0F (0000 0000 - 0000 FFFF)
    * sector size 32k from sector 0x10 - 0x15 (0001 0000 - 0001 7FFF)
    *
    */
   
   switch(mem->cpu.id) {
      case PART_LPC1343:
         // 32kB ROM
         mem->img_sector_last = (bin_stat.st_size + (0x1000 - (bin_stat.st_size % 0x1000))) / 0x1000;
         mem->rom_sector_last = 0x07;         
         mem->rom_total_size  = mem->rom_sector_last * 0x1000;
         break;      
      case PART_LPC1751:
         // 32kB ROM
         mem->img_sector_last = (bin_stat.st_size + (0x1000 - (bin_stat.st_size % 0x1000))) / 0x1000;
         mem->img_total_size  = mem->img_sector_last * 0x1000;
         mem->rom_sector_last = 0x07;
         break;
      
      case PART_LPC1752:
         // 64 kB ROM
         mem->img_sector_last = (bin_stat.st_size + (0x1000 - (bin_stat.st_size % 0x1000))) / 0x1000;
         mem->img_total_size  = mem->img_sector_last * 0x1000;
         mem->rom_sector_last = 0x0f;
         break;
      
      case PART_LPC1754:
      case PART_LPC1764:
         // 128 kB ROM
         /* determine sectors used by firmware image */
         if(bin_stat.st_size < 0x00010000) { // only 4k sectors are used
            mem->img_sector_last = (bin_stat.st_size + (0x1000 - (bin_stat.st_size % 0x1000))) / 0x1000;
            mem->img_total_size  = mem->img_sector_last * 0x1000;
         } else {
            mem->img_sector_last = 0x0f + ((bin_stat.st_size - 0x10000) + (0x8000 - ((bin_stat.st_size - 0x10000) % 0x8000))) / 0x8000;
            mem->img_total_size  = mem->img_sector_last * 0x8000;
         }
         mem->rom_sector_last = 0x11;
         break;
      
      case PART_LPC1756:
      case PART_LPC1763:
	  case PART_LPC1765:
      case PART_LPC1766:
         // 256kB ROM
         /* determine sectors used by firmware image */
         if(bin_stat.st_size < 0x00010000) { // only 4k sectors are used
            mem->img_sector_last = (bin_stat.st_size + (0x1000 - (bin_stat.st_size % 0x1000))) / 0x1000;
            mem->img_total_size  = mem->img_sector_last * 0x1000;
			printf("D1: mem->img_sector_last = %x\nD1: mem->img_total_size = %x\n", mem->img_sector_last, mem->img_total_size);
         } else {
            mem->img_sector_last = 0x0f + ((bin_stat.st_size - 0x10000) + (0x8000 - ((bin_stat.st_size - 0x10000) % 0x8000))) / 0x8000;
            mem->img_total_size  = mem->img_sector_last * 0x8000;
			printf("D2: mem->img_sector_last = %x\nD2: mem->img_total_size = %x\n", mem->img_sector_last, mem->img_total_size);
         }
         mem->rom_sector_last = 0x15;

         break;
      
      case PART_LPC1767:
      case PART_LPC1768:
      case PART_LPC1769:
      case PART_LPC1758:
         // 512 kB ROM
         /* determine sectors used by firmware image */
         if(bin_stat.st_size < 0x00010000) { // only 4k sectors are used
            mem->img_sector_last = (bin_stat.st_size + (0x1000 - (bin_stat.st_size % 0x1000))) / 0x1000;
            mem->img_total_size  = mem->img_sector_last * 0x1000;
         } else {
            mem->img_sector_last = 0x0f + ((bin_stat.st_size - 0x10000) + (0x8000 - ((bin_stat.st_size - 0x10000) % 0x8000))) / 0x8000;
            mem->img_total_size  = mem->img_sector_last * 0x8000;
         }
         mem->rom_sector_last = 0x1d;
         break;
      
      default:
         printf("[e] Unknown part id (%08x)\n", mem->cpu.id);
         mem->img_sector_last = 0;
         mem->img_total_size  = 0;
         mem->rom_sector_last = 0;

         return -1;
         break;   
   }

   // perform write
   
   printf("[*] Firmware image %s:\n", imgpath);
   printf("[*]       Size (bytes): 0x%X\n",           mem->img_total_size);
   printf("[*]   First ROM sector: 0x%X\n",           mem->img_sector_base);
   printf("[*]    Last ROM sector: 0x%X of 0x%X\n",   mem->img_sector_last+mem->rom_sector_base, mem->rom_sector_last);
   printf("[*] \n");
   
   if(mem->rom_sector_last < mem->img_sector_last+mem->rom_sector_base) {
      printf("[e] Firmware image is too large (size=0x%X) to fit onto this part (id=0x%08X)\n",
         (unsigned int)bin_stat.st_size, mem->cpu.id);
      close(bin_fd);
      return -1;
   }

   //serial_cmd_read_memory(serial_fd, 0, 128);
   memset(outbuf, 0xCC, sizeof(outbuf));
   
   // echo off
   serial_cmd_echo(serial_fd, 0);

   nwritten=0;
   
   // process 4kB sectors
   for(i=mem->rom_sector_base; i<=0xf && i<(mem->img_sector_last + mem->rom_sector_base); i++) {
      unsigned int j=0;
      		
		
      serial_cmd_prepare_sector(serial_fd, i, i);
      serial_cmd_erase_sector(serial_fd, i, i);
      
      for(j=0; j < (0x1000/0x200); j++) { // write in blocksizes of 512 byte
         
         int nr = read(bin_fd, &outbuf, 512);
         if(nr <= 0) { // ende gelaende.
            break;
         }
         
		 
         serial_cmd_write_to_ram(serial_fd, mem->ram_buffer_address, nr, outbuf);
         serial_cmd_prepare_sector(serial_fd, i, i);
         serial_cmd_copy_ram_to_flash(serial_fd, mem->rom_address_base + nwritten, mem->ram_buffer_address, 512);

         nwritten += nr;

         printf("[-] current 4k sector: 0x%02X - 0x%08X of 0x%08X bytes written\r\n", 
            i, nwritten, (unsigned int)bin_stat.st_size);
            
         fflush(stdout);
      }
   }

   // process 32kB sectors
   for(; i<=(mem->img_sector_last + mem->rom_sector_base); i++) {
      unsigned j=0;

      for(j=0; j < (0x8000/0x200); j++) { // write in blocksizes of 512 byte
         
         int nr = read(bin_fd, &outbuf, 512);
         if(nr <= 0) { // ende gelaende.
            break;
         }
		 
         serial_cmd_write_to_ram(serial_fd, mem->ram_buffer_address, nr, outbuf);
         serial_cmd_prepare_sector(serial_fd, i, i);
         serial_cmd_copy_ram_to_flash(serial_fd, mem->rom_address_base + nwritten, mem->ram_buffer_address, 512);

         nwritten += nr;

         printf("[-] current 32k sector: 0x%02X - 0x%08X of 0x%08X bytes written\r\n", 
            i, nwritten, (unsigned int)bin_stat.st_size);
            
         fflush(stdout);
      }
   }

   printf("[*] done.\n");

   close(bin_fd);
   return 0;
}

static int lpcflash_image_dump(int serial_fd, mem_info_t *mem, char *imgpath)
{
   

   // echo off
   serial_cmd_echo(serial_fd, 0);
   
   /* 
    * sector size  4k from sector 0x00 - 0x0F (0000 0000 - 0000 FFFF)
    * sector size 32k from sector 0x10 - 0x15 (0001 0000 - 0001 7FFF)
    *
    */
   
   switch(mem->cpu.id) {
      case PART_LPC1751:
         // 32kB ROM
         mem->rom_sector_last = 0x07;         
         mem->rom_total_size  = mem->rom_sector_last * 0x1000;
         break;
      
      case PART_LPC1752:
         // 64 kB ROM

         mem->rom_sector_last = 0x0f;
         mem->rom_total_size  = mem->rom_sector_last * 0x1000;

         break;
      
      case PART_LPC1754:
      case PART_LPC1764:
         // 128 kB ROM
         /* determine sectors used by firmware image */

         mem->rom_sector_last = 0x11;
         mem->rom_total_size  = 0x0F * 0x1000;
         mem->rom_total_size  += (mem->rom_sector_last-0x0F) * 0x8000;
         
         break;
      
      case PART_LPC1756:
      case PART_LPC1765:
      case PART_LPC1766:
         // 256kB ROM
         /* determine sectors used by firmware image */

         mem->rom_sector_last = 0x15;
         mem->rom_total_size  = 0x0F * 0x1000;
         mem->rom_total_size  += (mem->rom_sector_last-0x0F) * 0x8000;         

         break;
      
      case PART_LPC1767:
      case PART_LPC1768:
      case PART_LPC1769:
      case PART_LPC1758:
         // 512 kB ROM
         /* determine sectors used by firmware image */

         mem->rom_sector_last = 0x1d;
         mem->rom_total_size  = 0x0F * 0x1000;
         mem->rom_total_size  += (mem->rom_sector_last-0x0F) * 0x8000;
                  
         break;
      case PART_LPC1343:
         // 32kB ROM
         mem->rom_sector_last = 0x07;         
         mem->rom_total_size  = mem->rom_sector_last * 0x1000;
         break;
      default:
         printf("[e] Unknown part id (%08x)\n", mem->cpu.id);
         mem->img_sector_last = 0;
         mem->img_total_size  = 0;
         mem->rom_sector_last = 0;

         return -1;
         break;   
   }


   // perform read
   
   printf("[*] ROM image %s:\n", imgpath);
   printf("[*]       Size (bytes): 0x%X\n",  mem->rom_total_size);
   printf("[*]   First ROM sector: 0x%X\n",  mem->rom_sector_base);
   printf("[*]    Last ROM sector: 0x%X\n",  mem->rom_sector_last);
   printf("[*] \n");
   
   serial_cmd_remap_bootvect(serial_fd, mem->ram_buffer_address);
   
   printf("[+] reading sector %d - %d. Starting at address %08x, reading %lu bytes\n",
         mem->img_sector_base,
         mem->img_sector_last,
         (unsigned int)sector_address[mem->img_sector_base][START], 
         (unsigned int)sector_address[mem->img_sector_last][END]-sector_address[mem->img_sector_base][START]+1
      );
   
   // dump sectors
   serial_cmd_read_memory(serial_fd, imgpath, 
         sector_address[mem->img_sector_base][START], 
         sector_address[mem->img_sector_last][END]-sector_address[mem->img_sector_base][START]+1
      );
      
   return 0;
}

static void lpcflash_dumpinfo(mem_info_t *mem)
{
   
   printf("[*] \n");
   printf("[*] Detected part ID = 0x%08X:\n",               
            mem->cpu.id);
   printf("[*]    Total number of 4k/32k ROM sectors: 0x%02X\n", 
            mem->rom_sector_last);
   printf("[*]    Part serial no: 0x%08X %08X %08X %08X\n", 
            mem->cpu.serial[0], mem->cpu.serial[1], mem->cpu.serial[2], mem->cpu.serial[3]);
   printf("[*]         Boot code: 0x%X\n",
            mem->cpu.bootcode);
   printf("[*] \n");
   
   return;
}

static void lpcflash_help(void)
{
   // l:b:f:c:o:hviAR:F:T:B:a:s:
   puts("usage: lpcflash");
   puts("\t -l  <serial line>   (8N1 cu device)");
   puts("\t -b  <baudrate>      (default 115200UL)");
   puts("\t -f  <infile>        (.bin file)");
   puts("\t -o  <outfile>");
   puts("\t -c  <cclk in kHz>   (default: 100000UL)");
   puts("\t -A                  (all flash sectors)");
   puts("\t -R  <RAM address>   (default: 0x10000400UL)");
   puts("\t -F  <FLASH from sector>");
   puts("\t -T  <FLASH to sector>");
   puts("\t -B  <FLASH base sector>");
   puts("\t -a                  (ram/rom memory address in hex)");
   puts("\t -s                  (only for -a: size in hex - word aligned)");
   puts("\t[-v]                 (be verbose)");
   puts("\t[-h]                 ( _o/ )");
   puts("\t[-i]                 (dump cpu infos)");
   puts("\t[-e]                 (erase only (== erase and exit))");
   puts("[-] More infos at https://project.dev.io/code/arm/");   
   
   return;
}


static unsigned int lpcflash_get_lastsector(unsigned int id)
{
   unsigned int last_sector=0;
   
   switch(id) {
      case PART_LPC1751:
         // 32kB ROM
         last_sector = 0x07;
         break;
      
      case PART_LPC1752:
         // 64 kB ROM
         last_sector = 0x0f;
         break;
      
      case PART_LPC1754:
      case PART_LPC1764:
         // 128 kB ROM
         last_sector = 0x11;
         break;
      
      case PART_LPC1756:
      case PART_LPC1765:
      case PART_LPC1766:
         // 256kB ROM
         last_sector = 0x15;
         break;
      
      case PART_LPC1767:
      case PART_LPC1768:
      case PART_LPC1769:
      case PART_LPC1758:
         // 512 kB ROM
         last_sector = 0x1d;
         break;
      case PART_LPC1343:
         last_sector = 0x07;
         break;
      default:
         printf("[e] Unknown part id (%08x)\n", id);
         return 0;
   }   
   return last_sector;
}


