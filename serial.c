// 
//  serial.c
//  lpcflash
//  
//  Created by ths on 2011-01-16.
//
//  Authors:
//      Thorsten Schroeder
//      David Gullasch
//
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
#include <time.h>
#include <ctype.h>

#include "msg.h"
#include "serial.h"
#include "serial_cmd.h"

#ifdef WITH_MAGIC_SLEEP

// this function is (by now) necessary on linux, since we ran into trouble without
// this break. This is completely stupid, since using this sleep, it does not even matter
// wether we use 115200 or 38400 baud... therefore we have a new
// XXX TODO: find a solution for the strange behavior on linux

static void magic(void)
{
   struct timespec ts;
   ts.tv_sec = 0;
   ts.tv_nsec = 10000000;
   nanosleep(&ts,NULL);
}
#endif

int serial_readline(char *buf, int len, int serial_fd)
{
   char *p = buf;
   size_t remaining = len;
   ssize_t nread;

   memset(buf, 0, remaining);

   while(remaining>0) {
      
      // need to read one character at a time to detect '\n'
      nread = read(serial_fd, p, 1);
      
      switch (nread) {
         
         case -1:
            if (errno == EAGAIN || errno == EINTR) {
	            WARN("read interrupted");
	            continue;
	         }
	         err(1, "read(): ");
            break;
	      
	      case 0:
	         errx(1, "unexpected EOF read");
            break;
      }

      remaining -= nread;
      p += nread;
      if (p[-1] == '\n')
	      break;
   }

   if(p-buf <= 2)
      WARN("short line");
   else if (p[-2] != '\r' || p[-1] != '\n')
      WARN("no CR NL at end of line%s", remaining ? "" : " (buffer too small?)");
      
   return p - buf;
}

int serial_send(int serial_fd, int len, char *data)
{
   ssize_t nwritten;
   char *p = data;
   size_t remaining = len;

#ifdef WITH_MAGIC_SLEEP
   // linux problem??
   magic();
#endif

   while (remaining > 0) {
      errno = 0;
      
      nwritten = write(serial_fd, (unsigned char *)p, remaining);
      
      switch (nwritten) {
	      case -1:
	         if (errno == EAGAIN || errno == EINTR) {
	            WARN("write interrupted");
	            continue;
	         }
	         err(1,"write(): ");
            break;
            
	      case 0:
	         warn("write(): ");
            break;
      }

      p += nwritten;
      remaining -= nwritten;
   }

   return p - data;
}

int serial_open(char *device, int baudrate)
{
   struct termios ttyio;
   speed_t speed;
   int fd;

   fd=open(device, O_RDWR);
   if(fd<0)
      err(1,"serial_open: open(): ");
   
   //DEBUG("fd = %d\n", fd);

   if(tcgetattr(fd,&ttyio)<0)
      err(1,"tcgetattr(): ");

   /*
    * c_iflag
    */

   CLR(ttyio.c_iflag,IGNBRK);
   // Ignore BREAK condition on input.

   CLR(ttyio.c_iflag,BRKINT);
   // If IGNBRK is set, a BREAK is ignored. If it is not set but BRKINT
   // is set, then a BREAK causes the input and output queues to be
   // flushed, and if the terminal is the controlling terminal of a
   // foreground process group, it will cause a SIGINT to be sent to
   // this foreground process group. When neither IGNBRK nor BRKINT are
   // set, a BREAK reads as a null byte ('\0'), except when PARMRK is
   // set, in which case it reads as the sequence \377 \0 \0.

   SET(ttyio.c_iflag,IGNPAR);
   // Ignore framing errors and parity errors.

   CLR(ttyio.c_iflag,PARMRK);
   // If IGNPAR is not set, prefix a character with a parity error or
   // framing error with \377 \0. If neither IGNPAR nor PARMRK is set,
   // read a character with a parity error or framing error as \0.

   CLR(ttyio.c_iflag,INPCK);
   // Enable input parity checking.

   CLR(ttyio.c_iflag,ISTRIP);
   // Strip off eighth bit.

   CLR(ttyio.c_iflag,INLCR);
   // Translate NL to CR on input.

   CLR(ttyio.c_iflag,IGNCR);
   // Ignore carriage return on input.

   CLR(ttyio.c_iflag,ICRNL);
   // Translate carriage return to newline on input (unless IGNCR is
   // set).

   SET(ttyio.c_iflag,IXON);
   // Enable XON/XOFF flow control on output.

   CLR(ttyio.c_iflag,IXANY);
   // (XSI) Typing any character will restart stopped output. (The
   // default is to allow just the START character to restart output.)

   SET(ttyio.c_iflag,IXOFF);
   // Enable XON/XOFF flow control on input.

   /*
    * c_oflag
    */

   CLR(ttyio.c_oflag,OPOST);
   // Enable implementation-defined output processing.

   CLR(ttyio.c_oflag,ONLCR);
   // (XSI) Map NL to CR-NL on output.

   CLR(ttyio.c_oflag,OCRNL);
   // Map CR to NL on output.

   CLR(ttyio.c_oflag,ONOCR);
   // Don't output CR at column 0.

   CLR(ttyio.c_oflag,ONLRET);
   // Don't output CR.

   CLR(ttyio.c_oflag,OFILL);
   // Send fill characters for a delay, rather than using a timed delay.

   /*
    * c_cflag
    */

   CLR(ttyio.c_cflag,CSIZE);
   SET(ttyio.c_cflag,CS8);
   // Character size mask. Values are CS5, CS6, CS7, or CS8.

   CLR(ttyio.c_cflag,CSTOPB);
   // Set two stop bits, rather than one.

   SET(ttyio.c_cflag,CREAD);
   // Enable receiver.

   CLR(ttyio.c_cflag,PARENB);
   // Enable parity generation on output and parity checking for input.

   CLR(ttyio.c_cflag,PARODD);
   // If set, then parity for input and output is odd; otherwise even
   // parity is used.

   CLR(ttyio.c_cflag,HUPCL);
   // Lower modem control lines after last process closes the device
   // (hang up).

   SET(ttyio.c_cflag,CLOCAL);
   // Ignore modem control lines.

   /*
    * c_lflag
    */

   CLR(ttyio.c_cflag,ISIG);
   // When any of the characters INTR, QUIT, SUSP, or DSUSP are
   // received, generate the corresponding signal.

   SET(ttyio.c_cflag,ICANON);
   // Enable canonical mode (described below).

   CLR(ttyio.c_cflag,ECHO);
   // Echo input characters.

   CLR(ttyio.c_cflag,ECHOE);
   // If ICANON is also set, the ERASE character erases the preceding
   // input character, and WERASE erases the preceding word.

   CLR(ttyio.c_cflag,ECHOK);
   // If ICANON is also set, the KILL character erases the current line.

   CLR(ttyio.c_cflag,ECHONL);
   // If ICANON is also set, echo the NL character even if ECHO is not
   // set.

   CLR(ttyio.c_cflag,NOFLSH);
   // Disable flushing the input and output queues when generating
   // signals for the INT, QUIT, and SUSP characters.

   CLR(ttyio.c_cflag,TOSTOP);
   // Send the SIGTTOU signal to the process group of a background
   // process which tries to write to its controlling terminal.

   CLR(ttyio.c_cflag,IEXTEN);
   // Enable implementation-defined input processing. This flag, as well
   // as ICANON must be enabled for the special characters EOL2, LNEXT,
   // REPRINT, WERASE to be interpreted, and for the IUCLC flag to be
   // effective.

   /* set baudrate */
   switch(baudrate) {
      case 115200: 
	      speed=B115200; 
	      break;
      case 57600: 
	      speed=B57600; 
	      break;
      case 38400: 
	      speed=B38400; 
	      break;
      case 19200: 
	      speed=B19200; 
	      break;
      default: 
	      MSG("invalid baud rate (%d)\n",baudrate);
	      baudrate=9600;
	 // fallthrough
      case 9600: 
	      speed=B9600; 
	      break;
      case 4800: 
	      speed=B4800; 
	      break;
   }

   if(cfsetispeed(&ttyio,speed)<0)
      err(1,"cfsetispeed(): ");
   
   if(cfsetospeed(&ttyio,speed)<0)
      err(1,"cfsetospeed(): ");

   //INFO("baud rate set to %d\n",baudrate);

   if(tcsetattr(fd, TCSANOW, &ttyio)<0)
      err(1,"tcsetattr(): ");

   return(fd);
}




