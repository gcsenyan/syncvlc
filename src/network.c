/*
 * Project: syncvlc
 * File: network.c
 * 
 * UIUC ECE438 Fall 2017 Final Project
 * Synchronized Video Playback Control
 * 
 * Author: Ce Guo (ceguo2) 
 *         Xinyi Xie (xinyix6)
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/time.h>
#include <stdint.h>
#include <poll.h>
#include <assert.h>
#include <string.h>

#include "common.h"
#include "network.h"
#include "protocol.h"

socket_t netCreateSock(char* hostname, uint16_t port, struct sockaddr_in *si_other) {
  int s;
  if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    diep("socket");
  memset((char *) &si_other, 0, sizeof (struct sockaddr_in));
  si_other->sin_family = AF_INET;
  si_other->sin_port = htons(port);
  if (inet_aton(hostname, &(si_other->sin_addr)) == 0) {
    fprintf(stderr, "inet_aton() failed\n");
    exit(1);
  }
  return s;
}

void netSendPacket(pkt_t *pkt, socket_t s, struct sockaddr_in *si_other, socklen_t slen) {
  size_t pktSize = sizeof(pkt_t);
  if (sendto(s, pkt, pktSize, 0, (struct sockaddr *)si_other, slen) == -1)
    diep("sendto()");
}

