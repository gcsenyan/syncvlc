
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <string.h>
#include "common.h"
#include "protocol.h"
#include "vlc.h"

#define VLC_MAX_CMD_LEN   20

static void diep(char *s) {perror(s); exit(1);}

static void _getCmdStr(char *cmdStr, vlcOpCode_t op, uint32_t para);

socket_t vlcConnect(char *path) {
  socket_t s;
  struct sockaddr_un vlcAddr;
  socklen_t len;

  if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    diep("vlc socket()");

  printf("Connecting to VLC...");
  vlcAddr.sun_family = AF_UNIX;
  strcpy(vlcAddr.sun_path, path);
  len = strlen(vlcAddr.sun_path) + sizeof(vlcAddr.sun_family);
  if (connect(s, (struct sockaddr *)&vlcAddr, len) == -1) {
    printf("ERROR\n");
    diep("vlc connect()");
  }
  printf("Connected\n");

  return s;
}

void vlcSetStatus(vlcInterface_t *sock, vlcOpCode_t op, uint32_t para) {
  char cmdStr[VLC_MAX_CMD_LEN];
  _getCmdStr(cmdStr, op, para);
  printf("Sending: %s\n", cmdStr);
  if (send(sock->s, cmdStr, strlen(cmdStr), 0) == -1) 
    diep("vlc send()");
}

static void _getCmdStr(char *cmdStr, vlcOpCode_t op, uint32_t para) {
  memset(cmdStr, 0, VLC_MAX_CMD_LEN);
  switch(op) {
    case OP_PAUSE: 
      sprintf(cmdStr, "pause"); break;
    case OP_PLAY:
      sprintf(cmdStr, "pause"); break;
    case OP_SEEK:
      sprintf(cmdStr, "seek %d", para);
      break;
  }
}

////////////////////////////////////////////////////////////////////////////////
/// Get lenght of the playing video in second.
////////////////////////////////////////////////////////////////////////////////
uint32_t vlcGetLength(vlcInterface_t *sock) {
  //TODO send "get_length" command to VLC and read back the lenght
  return 0;
}


////////////////////////////////////////////////////////////////////////////////
/// Get time elapsed since the begining of the video in second.
////////////////////////////////////////////////////////////////////////////////
uint32_t vlcGetTime(vlcInterface_t *sock) {
  //TODO send "get_time" command to VLC and read back current time
  return 0;
}

////////////////////////////////////////////////////////////////////////////////
/// Get current playback status, including whether it's playing or paused 
///   and time elapsed.
////////////////////////////////////////////////////////////////////////////////
void vlcGetStatus(vlcInterface_t *sock, vlcStatus_t *stat) {
  //TODO send "status" to VLC to get current playing status.
  //  This can be a little tricky since it seems the return of this command is
  //  not consistent. For example, if "status" is the very first command ever
  //  get sent to a brand-newly opened VLC, it doesn't return current playback
  //  status. Therefore we may need to consider send some dummy command to VLC
  //  as a part of VLC RC sock connecting process. 
  
  
}

////////////////////////////////////////////////////////////////////////////////
/// Get whether a video is opened in VLC. 
////////////////////////////////////////////////////////////////////////////////
bool_t vlcGetIsOpened(vlcInterface_t *sock) {
  //TODO
  // This fucntion gets whether a file is opened in VLC,
  // which mathches the ruturn of "is_playing".
  // CAUTION: Return true as long as a file is opened in VLC, even it's paused.
  return FALSE;
}

void vlcSetStatus(vlcStatus_t *stat){
  //TODO
  // The program shouldn't make any assumption of the current status of VLC
  // It's important to check current VLC status before set the status since command "pause"
  // toggles play and pause.
}

