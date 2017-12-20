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
#include "vlc.h"
#include "network.h"


static void diep(char *s) {perror(s); exit(1);}
static void _pktHton(pkt_t *p);
static void _pktNtoh(pkt_t *p);

void netInitClient(char *hostname, uint16_t port, sockInterface_t *other) {
  socket_t s;
  struct sockaddr_in *si_other = &(other->sadd);
  other->slen = sizeof(struct sockaddr_in);
  if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    diep("socket");
  other->s = s;
  memset((char *)si_other, 0, sizeof(struct sockaddr_in));
  si_other->sin_family = AF_INET;
  si_other->sin_port = htons(port);
  if (inet_aton(hostname, &(si_other->sin_addr)) == 0)
    diep("inet_aton() failed");

  other->outSeqNum = 0;
  other->inSeqNum = -1;

  other->pfds[0].fd = s;
  other->pfds[0].events = POLLIN;

  // Send handshake packet
  pkt_t pkt;
  pkt.pktType = PKT_HANDSHAKE;
  pkt.seqNum = 0;
  netSendPacket(&pkt, other);
}

void netInitServer(uint16_t port, sockInterface_t *other) {
  socket_t s;
  struct sockaddr_in si_me;
  struct sockaddr_in *si_other = &(other->sadd);
  other->slen = sizeof(struct sockaddr_in);

  if ((s = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    diep("socket");
  memset((char *)&si_me, 0, sizeof(struct sockaddr_in));
  si_me.sin_family = AF_INET;
  si_me.sin_port = htons(port);
  si_me.sin_addr.s_addr = htonl(INADDR_ANY);
  printf("Now binding\n");
  if (bind(s, (struct sockaddr*)&si_me, sizeof(struct sockaddr_in)) == -1)
    diep("bind()");
  
  other->outSeqNum = 0;
  other->inSeqNum = -1;
  
  other->pfds[0].fd = s;
  other->pfds[0].events = POLLIN;

  // Wait for client.
  pkt_t newPkt;
  printf("Waiting for client...\n");
  if (recvfrom(s, &newPkt, sizeof(pkt_t), 0, (struct sockaddr *)si_other, &other->slen) == -1)
    diep("recvfrom()");
  
  printf("Client Connected: %s:%d\n", inet_ntoa(si_other->sin_addr), si_other->sin_port);

  other->s = s;
}


void netSendPacket(pkt_t *pkt, sockInterface_t *other) {
  struct sockaddr_in *si_other =  &(other->sadd);
  socket_t s = other->s;
  pkt->seqNum = other->outSeqNum++;
  // Convert byte order before send.
  _pktHton(pkt);
  socklen_t slen = other->slen;
  size_t pktSize = sizeof(pkt_t);
  if (sendto(s, pkt, pktSize, 0, (struct sockaddr *)si_other, slen) == -1)
    diep("sendto()");
}

void netForwardPacket(pkt_t *pkt, sockInterface_t *other) {
  struct sockaddr_in *si_other =  &(other->sadd);
  socket_t s = other->s;
  socklen_t slen = other->slen;
  size_t pktSize = sizeof(pkt_t);
  if (sendto(s, pkt, pktSize, 0, (struct sockaddr *)si_other, slen) == -1)
    diep("sendto()");
}


bool_t netPollPacket(pkt_t *pkt, sockInterface_t *other) {
  int pollRes;
  socklen_t slen;
  struct sockaddr_in si_other = other->sadd;
  bool_t gotNewPkt = FALSE;
  socket_t s = other->s;
  while ((pollRes = poll(other->pfds, 1, 0)) > 0) {
    // We got at least one incoming ACK
    if (recvfrom(s, pkt, sizeof(pkt_t), 0, (struct sockaddr *)&si_other, &slen) == -1)
      diep("recvfrom()");
    _pktNtoh(pkt);
    // Make sure the new packet is from the same source
    if (strcmp(inet_ntoa(si_other.sin_addr), inet_ntoa(other->sadd.sin_addr)) == 0) {
      if (pkt->seqNum > other->inSeqNum) {
        gotNewPkt = TRUE;
        other->inSeqNum = pkt->seqNum;
        break;
      }
    }
  }
  if (pollRes == -1) {
    diep("poll failed");
  }
  return gotNewPkt;
}

static void _pktHton(pkt_t *p) {
  p->seqNum = htonl(p->seqNum);
  p->pktType = htonl(p->pktType);
  p->vlcStat.stat = htonl(p->vlcStat.stat);
  p->vlcStat.time = htonl(p->vlcStat.time);
}

static void _pktNtoh(pkt_t *p) {
  p->seqNum = ntohl(p->seqNum);
  p->pktType = ntohl(p->pktType);
  p->vlcStat.stat = ntohl(p->vlcStat.stat);
  p->vlcStat.time = ntohl(p->vlcStat.time);
}
