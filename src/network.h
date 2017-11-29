/*
 * Project: syncvlc
 * File: network.h
 * 
 * UIUC ECE438 Fall 2017 Final Project
 * Synchronized Video Playback Control
 * 
 * Author: Ce Guo (ceguo2) 
 *         Xinyi Xie (xinyix6)
 * 
 */

#ifndef SOCKET_H
#define SOCKET_H

#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>

inline void diep(char *s) {perror(s); exit(1);}



#endif // SOCKET_H