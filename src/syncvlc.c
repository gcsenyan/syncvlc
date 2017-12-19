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

//static void badArgs(char* argv[]);

////////////////////////////////////////////////////////////////////////////////
/// Entry function for syncvlc
////////////////////////////////////////////////////////////////////////////////
int main(int argc, char* argv[]) {
  for (int i = 0; i < argc; i++) {
    printf("%s\n", argv[i]);
  }
/*
  if (argc < 2) badArgs(argv);
  if (argv[1][0] == 'c') {
    if (argc < 4) badArgs(argv);
    uint16_t port = (uint16_t)atoi(argv[3]);
    sockInterface_t other;
    netInitClient(argv[2], port, &other);
  }
  else if (argv[1][0] == 's') {
    if (argc < 3) badArgs(argv);
    uint16_t port = (uint16_t)atoi(argv[2]);
    sockInterface_t other;
    netInitServer(port, &other);
  }
  else
    badArgs(argv);*/


  vlcInterface_t vlc;
  vlcInit("/home/senyan/vlc.sock", &vlc);
  //vlcSendCmd(VLC_OP_PAUSE, 0, &vlc);
  vlcSendPause(&vlc);
  //vlcPlayStat_t stat = vlcGetPlayStatus(&vlc);
  //bool_t isPlaying = vlcGetIsOpened(&vlc);
  //uint32_t time = vlcGetTime(&vlc);
  //uint32_t length = vlcGetLength(&vlc);
  //printf("isPlaying=%d, stat=%d, time=%d, length=%d\n", isPlaying, stat, time, length);
  while(1) {
    vlcPlayStat_t stat = vlcPollPlayStatus(&vlc);
    if (stat != VLC_STAT_INVALID) {
      switch (stat) {
        case VLC_STAT_PLAY: printf("play\n"); break;
        case VLC_STAT_PAUSE: printf("pause\n"); break;
      }
    }
  }
  close(vlc.s);
  return 0;
}

/*static void badArgs(char* argv[]) {
  fprintf(stderr, "Usage: %s s|c  \n", argv[0]);
  exit(1);
}*/

