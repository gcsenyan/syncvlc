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
  if (argv[1][0] == 'c') {
    if (argc < 4) badArgs(argv);
    uint16_t port = (uint16_t)atoi(argv[3]);
    sockaddr_in_t other;
    socket_t s = netInitClient(argv[2], port, &other);
    pkt_t pkt;
    pkt.pktType = PKT_HANDSHAKE;
    pkt.seqNum = 0;
    netSendPacket(&pkt, s, &other);
  }
  else if (argv[1][0] == 's') {
    if (argc < 3) badArgs(argv);
    sockaddr_in_t other, me;
    uint16_t port = (uint16_t)atoi(argv[2]);
    socket_t s = netInitServer(port, &me, &other);
  }
  else
    badArgs(argv);
  /*
  vlcCmd_t cmd;
  cmd.op = OP_SEEK;
  cmd.para = 1000;
  socket_t vlcSock = vlcConnect("/home/senyan/vlc.sock");
  vlcSendCmd(vlcSock, &cmd);
  close(vlcSock);*/
  return 0;
}

static void badArgs(char* argv[]) {
  fprintf(stderr, "Usage: %s s|c  \n", argv[0]);
  exit(1);
}