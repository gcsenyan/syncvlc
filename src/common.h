/*
 * Project: syncvlc
 * File: common.h
 * 
 * UIUC ECE438 Fall 2017 Final Project
 * Synchronized Video Playback Control
 * 
 * Author: Ce Guo (ceguo2) 
 *         Xinyi Xie (xinyix6)
 * 
 */

#ifndef COMMON_H
#define COMMON_H

// Boolean type
typedef unsigned int bool_t;
typedef int socket_t;
#define     TRUE    1
#define     FALSE   0

#define     DEBUG_MODE  TRUE
#define     DEBUG_SINK  stdout
#define     debugf(fmt, ...)        {if(DEBUG_MODE) fprintf(DEBUG_SINK, (fmt), __VA_ARGS__);}
#define     debug_str(str)          debugf("%s\n", (str))
#define     debug_ent_func(func)    debugf("%s entry\n", (func))  
#define     debug_exi_func(func)    debugf("%s exit\n", (func))

#endif // COMMON_H