// 
//  msg.h
//  lpcflash
//  
//  Created by dagu on 2011-01-30.
//  Copyright 2011 David Gullasch. All rights reserved.
// 

#ifndef MSG_H
#define MSG_H

#include <err.h>

#define MSGLEVEL_DEBUG   3
#define MSGLEVEL_VERBOSE 2
#define MSGLEVEL_NORMAL  1
#define MSGLEVEL_WARN    0

extern int msglevel;

__attribute__((format(printf, 3, 4)))
void msg(int level, const char *function, const char *fmt, ...);

#define WARN(args...)  msg(MSGLEVEL_WARN,    __FUNCTION__, args)
#define MSG(args...)   msg(MSGLEVEL_NORMAL,  __FUNCTION__, args)
#define INFO(args...)  msg(MSGLEVEL_VERBOSE, __FUNCTION__, args)
#define DEBUG(args...) msg(MSGLEVEL_DEBUG,   __FUNCTION__, args)

#endif
