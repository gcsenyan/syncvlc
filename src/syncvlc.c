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
#include "common.h"
#include "protocol.h"
#include "vlc.h"

////////////////////////////////////////////////////////////////////////////////
/// Entry function for syncvlc
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {
  for (int i = 0; i < argc; i++) {
    printf("%s\n", argv[i]);
  }

  vlcCmd_t cmd;
  cmd.op = OP_SEEK;
  cmd.para = 1000;
  socket_t vlcSock = vlcConnect("/home/senyan/vlc.sock");
  vlcSendCmd(vlcSock, &cmd);
  close(vlcSock);
  return 0;
}
