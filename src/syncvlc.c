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
#include "common.h"
#include "protocol.h"
#include "network.h"
#include "vlc.h"

static void badArgs(char* argv[]);

////////////////////////////////////////////////////////////////////////////////
/// Entry function for syncvlc
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {
  for (int i = 0; i < argc; i++) {
    printf("%s\n", argv[i]);
  }

  if (argc < 2) badArgs(argv);

  vlcInterface_t vlc;
  sockInterface_t other;
  

  if (argv[2][0] == 'c') {
    if (argc < 5) badArgs(argv);
    vlcInit(argv[1], &vlc);
    uint16_t port = (uint16_t)atoi(argv[4]);
    netInitClient(argv[3], port, &other);
  }
  else if (argv[2][0] == 's') {
    if (argc < 4) badArgs(argv);
    vlcInit(argv[1], &vlc);
    uint16_t port = (uint16_t)atoi(argv[3]);
    netInitServer(port, &other);
  }
  else
    badArgs(argv);


  while(1) {
    vlcStatus_t status;
    pkt_t inPkt;
    bool_t gotPkt = netPollPacket(&inPkt, &other);
    if (gotPkt) {
      printf("Incoming pkt(%d): %d:%d\n", inPkt.seqNum, inPkt.vlcStat.stat, inPkt.vlcStat.time);
      vlcSetStatus(&inPkt.vlcStat, &vlc);
    }
    status = vlcPollStatus(&vlc);
    if (status.stat != VLC_STAT_INVALID) {
      switch (status.stat) {
        case VLC_STAT_PLAY: printf("play: %d\n", status.time); break;
        case VLC_STAT_PAUSE: printf("pause: %d\n", status.time); break;
      }
      pkt_t pkt;
      pkt.pktType = PKT_SYNC;
      pkt.vlcStat = status;
      printf("sending: %d, %d\n", pkt.vlcStat.stat, pkt.vlcStat.time);
      netSendPacket(&pkt, &other);
    }
    usleep(50);
  }
  close(vlc.s);
  return 0;
}

static void badArgs(char* argv[]) {
  fprintf(stderr, "Usage: %s vlc_sock s server_port | c  server_addr server_port\n", argv[0]);
  exit(1);
}

