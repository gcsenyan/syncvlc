

#ifndef VLC_H
#define VLC_H

#include "common.h"
#include <poll.h>
// VLC command operation code.
enum {
  VLC_OP_NOP,
  VLC_OP_PLAY,
  VLC_OP_PAUSE,
  VLC_OP_SEEK,
  VLC_OP_GET_TIME,
  VLC_OP_GET_LENGTH,
  VLC_OP_GET_STATUS,
  VLC_OP_IS_PLAYING,
  NUM_OP
};

// VLC play states.
enum {
  VLC_STAT_PAUSE,
  VLC_STAT_PLAY,
  VLC_STAT_SEEK,
  VLC_STAT_INVALID,
  NUM_STAT
};

typedef uint32_t vlcOpCode_t;
typedef uint32_t vlcPlayState_t;

typedef struct {
  vlcOpCode_t op;
  uint32_t para;
}vlcCmd_t;

typedef struct {
  vlcPlayState_t stat;
  uint32_t time;
}vlcStatus_t;

typedef struct {
  socket_t s;
  struct pollfd pfds[1];
}vlcInterface_t;

void vlcInit(char *path, vlcInterface_t *sock);
socket_t vlcConnect(char *path);
void vlcSendCmd(vlcOpCode_t op, int para, vlcInterface_t *sock);
uint32_t vlcGetLength(vlcInterface_t *sock);
uint32_t vlcGetTime(vlcInterface_t *sock);
void vlcGetStatus(vlcInterface_t *sock, vlcStatus_t *stat);
bool_t vlcGetIsOpened(vlcInterface_t *sock);
void vlcSetStatus(vlcStatus_t *stat);
vlcPlayState_t vlcPollPlayState(vlcInterface_t *sock, int *para);
vlcStatus_t vlcPollStatus(vlcInterface_t *sock);

#endif // VLC_H
