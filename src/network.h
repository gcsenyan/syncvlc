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
#include <poll.h>
#include "protocol.h"

typedef struct {
  socket_t s;
  uint32_t outSeqNum;
  uint32_t inSeqNum;
  struct sockaddr_in sadd;
  socklen_t slen;
  struct pollfd pfds[1];
}sockInterface_t;


void netInitClient(char *hostname, uint16_t port, sockInterface_t *other);
void netInitServer(uint16_t port, sockInterface_t *other);
void netSendPacket(pkt_t *pkt, sockInterface_t *other);
bool_t netPollPacket(pkt_t *pkt, sockInterface_t *other);
void netForwardPacket(pkt_t *pkt, sockInterface_t *other);

#endif // SOCKET_H
