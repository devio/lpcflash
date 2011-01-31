// 
//  msg.c
//  lpcflash
//  
//  Created by dagu on 2011-01-30.
//  Copyright 2011 David Gullasch. All rights reserved.
// 


#include <stdio.h>
#include <stdarg.h>

#include "msg.h"

int msglevel = MSGLEVEL_NORMAL;

void msg(int level, const char *function, const char *fmt, ...)
{
	va_list args;

	if (level < msglevel)
		return;

	printf("%s(): ", function);
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
}
