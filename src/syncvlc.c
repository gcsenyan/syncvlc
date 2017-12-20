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
  sockInterface_t other;
  if (argc < 2) badArgs(argv);
  if (argv[1][0] == 'c') {
    if (argc < 4) badArgs(argv);
    uint16_t port = (uint16_t)atoi(argv[3]);
    netInitClient(argv[2], port, &other);
  }
  else if (argv[1][0] == 's') {
    if (argc < 3) badArgs(argv);
    uint16_t port = (uint16_t)atoi(argv[2]);
    netInitServer(port, &other);
  }
  else
    badArgs(argv);


  vlcInterface_t vlc;
  vlcInit("/home/senyan/vlc.sock", &vlc);
  while(1) {
    vlcStatus_t status;
    status = vlcPollStatus(&vlc);
    if (status.stat != VLC_STAT_INVALID) {
      switch (status.stat) {
        case VLC_STAT_PLAY: printf("play: %d\n", status.time); break;
        case VLC_STAT_PAUSE: printf("pause: %d\n", status.time); break;
      }
      pkt_t pkt;
      pkt.pktType = PKT_SYNC;
      pkt.vlcStat = status;
      netSendPacket(&pkt, &other);
    }
  }
  close(vlc.s);
  return 0;
}

static void badArgs(char* argv[]) {
  fprintf(stderr, "Usage: %s s|c  sever_addr server_port\n", argv[0]);
  exit(1);
}

