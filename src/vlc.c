
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

void diep(char *s) {perror(s); exit(1);}

static void _getCmdStr(char *cmdStr, vlcCmd_t *op);

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

void vlcSendCmd(socket_t s, vlcCmd_t *cmd) {
  char cmdStr[VLC_MAX_CMD_LEN];
  _getCmdStr(cmdStr, cmd);
  printf("Sending: %s\n", cmdStr);
  if (send(s, cmdStr, strlen(cmdStr), 0) == -1) 
    diep("vlc send()");
}

static void _getCmdStr(char *cmdStr, vlcCmd_t *cmd) {
  memset(cmdStr, 0, VLC_MAX_CMD_LEN);
  switch(cmd->op) {
    case OP_PAUSE: 
      sprintf(cmdStr, "pause"); break;
    case OP_PLAY:
      sprintf(cmdStr, "pause"); break;
    case OP_SEEK:
      sprintf(cmdStr, "seek %d", cmd->para);
      break;
  }
}