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

static void diep(char *s) {perror(s); exit(1);}

socket_t netInitClient(char *hostname, uint16_t port, sockaddr_in_t *other) {
  socket_t s;
  struct sockaddr_in *si_other = &(other->sadd);
  other->slen = sizeof(struct sockaddr_in);
  if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    diep("socket");
  memset((char *)si_other, 0, sizeof(struct sockaddr_in));
  si_other->sin_family = AF_INET;
  si_other->sin_port = htons(port);
  if (inet_aton(hostname, &(si_other->sin_addr)) == 0)
    diep("inet_aton() failed");
  return s;
}

socket_t netInitServer(uint16_t port, sockaddr_in_t *me, sockaddr_in_t *other) {
  socket_t s;
  struct sockaddr_in *si_me = &(me->sadd);
  socklen_t *me_slen = &(me->slen);
  struct sockaddr_in *si_other = &(other->sadd);
  other->slen = sizeof(struct sockaddr_in);

  if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    diep("socket");
  memset((char *)si_me, 0, sizeof(struct sockaddr_in));
  si_me->sin_family = AF_INET;
  si_me->sin_port = htons(port);
  si_me->sin_addr.s_addr = htonl(INADDR_ANY);
  *me_slen = sizeof(struct sockaddr_in);
  printf("Now binding\n");
  if (bind(s, (struct sockaddr*)si_me, *me_slen) == -1)
    diep("bind()");

  // Wait for client.
  pkt_t newPkt;
  printf("Waiting for client...\n");
  if (recvfrom(s, &newPkt, sizeof(pkt_t), 0, (struct sockaddr *)si_other, &other->slen) == -1)
    diep("recvfrom()");
  
  printf("Client Connected: %s:%d\n", inet_ntoa(si_other->sin_addr), si_other->sin_port);

  return s;
}


void netSendPacket(pkt_t *pkt, socket_t s, sockaddr_in_t *other) {
  struct sockaddr_in *si_other =  &(other->sadd);
  socklen_t slen = other->slen;
  size_t pktSize = sizeof(pkt_t);
  if (sendto(s, pkt, pktSize, 0, (struct sockaddr *)si_other, slen) == -1)
    diep("sendto()");
}

