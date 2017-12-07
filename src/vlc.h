

#ifndef VLC_H
#define VLC_H

#include "common.h"
#include <poll.h>

typedef struct {
  socket_t s;
  struct pollfd pfds[1];
}vlcInterface_t;

socket_t vlcConnect(char *path);
void vlcSendCmd(socket_t s, vlcCmd_t *cmd);

#endif // VLC_H