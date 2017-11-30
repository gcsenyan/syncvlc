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
#include "protocol.h"

typedef struct {
  struct sockaddr_in sadd;
  socklen_t slen;
}sockaddr_in_t;


socket_t netInitClient(char *hostname, uint16_t port, sockaddr_in_t *other);
socket_t netInitServer(uint16_t port, sockaddr_in_t *me, sockaddr_in_t *other);
void netSendPacket(pkt_t *pkt, socket_t s, sockaddr_in_t *other);

#endif // SOCKET_H