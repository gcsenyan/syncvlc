/*
 * Project: syncvlc
 * File: syncvlc.c
 * 
 * UIUC ECE438 Fall 2017 Final Project
 * Synchronized Video Playback Control
 * 
 * Author: Ce Guo (ceguo2) 
 *         Xinyi Xie (xinyix6)
 * 
 */

#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include "common.h"
#include "protocol.h"
#include "network.h"

static void diep(char *s) {perror(s); exit(1);}
static socket_t openPort(uint16_t port);
static void badArgs(char* argv[]);
static void ioHandler(int signal);
static void forwardPacket(pkt_t *pkt, sockInterface_t *other);

sockInterface_t sock[2];

////////////////////////////////////////////////////////////////////////////////
/// Entry function for syncvlc_relay
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {
  uint16_t port[2];

  if (argc < 3)
    badArgs(argv);

  for(int i = 0; i < 2; i++) {
    port[i] = (uint16_t)atoi(argv[i + 1]);
  }

  memset(sock, 0, sizeof(sock));

  for(int i = 0; i < 2; i++) {
    sock[i].slen = sizeof(sock[i].sadd);
    sock[i].s = openPort(port[i]);
    sock[i].pfds[0].fd = sock[i].s;
    sock[i].pfds[0].events = POLLIN;
  }

  signal(SIGIO,ioHandler);

  for(int i = 0; i < 2; i++) {
    if (fcntl(sock[i].s,F_SETOWN, getpid()) < 0){
      perror("fcntl F_SETOWN");
      exit(1);
    }
    if (fcntl(sock[i].s,F_SETFL,FASYNC) <0 ){
      perror("fcntl F_SETFL, FASYNC");
      exit(1);
    }
  }

  printf("init done\n");
  while(1);
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// Open given port and return the sockfd binded to it
////////////////////////////////////////////////////////////////////////////////
static socket_t openPort(uint16_t port) {
  printf("Opening port %d...", port);
  socket_t s;
  struct sockaddr_in si_me;

  if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    diep("socket");
  memset((char *)&si_me, 0, sizeof(struct sockaddr_in));
  si_me.sin_family = AF_INET;
  si_me.sin_port = htons(port);
  si_me.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind(s, (struct sockaddr*)&si_me, sizeof(struct sockaddr_in)) == -1)
    diep("bind()");
  printf("Done! sockfd=%d\n", s);
  return s;
}


////////////////////////////////////////////////////////////////////////////////
/// SIGIO handler
////////////////////////////////////////////////////////////////////////////////
static void ioHandler(int signal) {
  debug_ent_func(__func__);
  (void)signal;
  
  int pollRes, recvlen;
  pkt_t pkt;

  for(int i = 0; i < 2; i++) {
    while((pollRes = poll(sock[i].pfds, 1, 0)) > 0) {
      sock[i].slen = sizeof(sock[i].sadd);
      recvlen = recvfrom(sock[i].s, (void *)&pkt, sizeof(pkt_t), 0, 
                          (struct sockaddr *)&sock[i].sadd, &sock[i].slen);
      if (recvlen == -1)
        diep("recvfrom()");
      else if (recvlen == sizeof(pkt_t)) {
        printf("Pkt from sock[%d]:%s:%d\n", i, inet_ntoa(sock[i].sadd.sin_addr), sock[i].sadd.sin_port);
        sock[i].valid = TRUE;
        forwardPacket(&pkt, &sock[1 - i]);
      }
      else {
        printf("Invalid pkt from sock[%d]:%s:%d, len=%d\n", 
                i, inet_ntoa(sock[i].sadd.sin_addr), sock[i].sadd.sin_port, recvlen);
      }
    }
  }
  if (pollRes == -1) {
    diep("poll failed");
  }
  debug_exi_func(__func__);
}

////////////////////////////////////////////////////////////////////////////////
/// Forward packet to the other socket
////////////////////////////////////////////////////////////////////////////////
static void forwardPacket(pkt_t *pkt, sockInterface_t *other) {
  struct sockaddr_in *si_other =  &(other->sadd);
  socket_t s = other->s;
  socklen_t slen = other->slen;
  size_t pktSize = sizeof(pkt_t);
  if (!other->valid)
    return;
  if (sendto(s, pkt, pktSize, 0, (struct sockaddr *)si_other, slen) == -1)
    diep("sendto()");
}

////////////////////////////////////////////////////////////////////////////////
/// Bad argument
////////////////////////////////////////////////////////////////////////////////
static void badArgs(char* argv[]) {
  fprintf(stderr, "Usage: %s port1 port2\n", argv[0]);
  exit(1);
}


