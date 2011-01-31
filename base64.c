// 
//  base64.c
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
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "base64.h"

unsigned int raw_uu_encode( unsigned char *out,  unsigned int binlen, unsigned char *in) 
{
   unsigned b=0;
   int incnt = 0;
   int outcnt = 0;
   int modcnt = 0;
   
   modcnt = (binlen%3);

   incnt = binlen / 3;
   incnt += modcnt;

   for(b=0;b<(incnt);b++) {
      
      unsigned int _in = 0;
      
      _in |= (*in     << 16) & 0x00ff0000;
      
      if(b==incnt-1) {
         if(modcnt == 2) {
            _in |= (0 <<  0) & 0x0000ff00;
            _in |= (0 <<  0) & 0x000000ff;         
         } else if(modcnt == 1) {
            _in |= (*(in+1) <<  8) & 0x0000ff00;
            _in |= (0 <<  0)       & 0x000000ff;
         } else if (modcnt == 0) {
            _in |= (*(in+1) <<  8) & 0x0000ff00;
            _in |= (*(in+2) <<  0) & 0x000000ff;
         } 
      } else {
         _in |= (*(in+1) <<  8) & 0x0000ff00;
         _in |= (*(in+2) <<  0) & 0x000000ff;
      }
      
      in+=3;
      
      out[outcnt+0] = ((_in >> 18) & 0x3F) + 0x20;
      out[outcnt+1] = ((_in >> 12) & 0x3F) + 0x20;
      out[outcnt+2] = ((_in >>  6) & 0x3F) + 0x20;
      out[outcnt+3] = ((_in >>  0) & 0x3F) + 0x20;
        
      outcnt+=4;
   }
   return outcnt;
}


unsigned int raw_uu_decode_set( unsigned char *in,  unsigned char *set) 
{
   set[0]=set[1]=set[2]=0;
     
   set[0] =((in[0]-32)<<2);
   set[0]|=((in[1]-32)>>4) & 0x03;
    
   set[1] =((in[1]-32)<<4) & 0xf0; 
   set[1]|=((in[2]-32)>>2) & 0x0f;
   
   set[2] =((in[2]-32)<<6) & 0xC0;
   set[2]|=((in[3]-32))    & 0x3F; 

   return 3;
}


unsigned int raw_uu_decode( unsigned char *in, unsigned char *out) 
{
   unsigned int binlen = 0;
   unsigned char o1=0;
   unsigned char o2=0;
   unsigned char o3=0;
   unsigned int i=0;
   unsigned b=0;
   
   binlen = (*(in++) - 32) & 0xFF;

   for(i=0;i<60;i+=4) {

      o1=o2=o3=0;
      
      o1 =((in[i+0]-32)<<2);
      o1|=((in[i+1]-32)>>4) & 0x03;
       
      o2 =((in[i+1]-32)<<4) & 0xf0; 
      o2|=((in[i+2]-32)>>2) & 0x0f;
      
      o3 =((in[i+2]-32)<<6) & 0xC0;
      o3|=((in[i+3]-32))    & 0x3F; 
   
      out[b++] = o1;
      out[b++] = o2;
      out[b++] = o3;

      if (b >= binlen) {
         b=binlen;
         break;
      }
   }
   return b;
}
