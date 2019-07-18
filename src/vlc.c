
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/un.h>
#include <string.h>
#include <assert.h>
#include "common.h"
#include "protocol.h"
#include "vlc.h"

#define VLC_MAX_CMD_LEN   20
#define VLC_MAX_ATTEMPT   10
#define VLC_MAX_READ_BUF  2048

#define IS_DIGIT(c)       (((c) >= 0x30) && ((c) <= 0x39))

static void diep(char *s) {perror(s); exit(1);}

static void _getCmdStr(char *cmdStr, vlcOpCode_t op, uint32_t para);
static void _sendCmd(vlcOpCode_t op, int para, vlcInterface_t *sock); 
static int _readOneBlocking(char *buf, vlcInterface_t *sock);
static int _readOneNonBlocking(char *buf, vlcInterface_t *sock);
int _readInteger(vlcInterface_t *sock);
int _sendAndReadInteger(vlcOpCode_t op, vlcInterface_t *sock);
vlcPlayState_t _analyzePlayState(char *buf, int *para);
void _exhaustReturnData(vlcInterface_t *sock);
void _sendPause(vlcInterface_t *sock);
void _seekTime(int para, vlcInterface_t *sock);


////////////////////////////////////////////////////////////////////////////////
/// Initialize VLC interface.
////////////////////////////////////////////////////////////////////////////////
void vlcInit(char *path, vlcInterface_t *sock) {
  debug_ent_func(__func__);
  sock->s = vlcConnect(path);
  sock->pfds[0].fd = sock->s;
  sock->pfds[0].events = POLLIN;
  // VLC stops sending data through the socket when a new file
  // is opened until some command get sent to it through the socket.
  _sendCmd(VLC_OP_GET_STATUS, 0, sock);
  _exhaustReturnData(sock);
  debug_exi_func(__func__);
}

////////////////////////////////////////////////////////////////////////////////
/// Connet the unix socket created by VLC for Remote Control (RC).
////////////////////////////////////////////////////////////////////////////////
socket_t vlcConnect(char *path) {
  debug_ent_func(__func__);
  socket_t s;
  struct sockaddr_un vlcAddr;
  socklen_t len;

  if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    diep("vlc socket()");

  printf("Connecting to VLC...");
  memset(&vlcAddr, 0, sizeof(struct sockaddr_un));
  vlcAddr.sun_family = AF_UNIX;
  
  strcpy(vlcAddr.sun_path, path);

  len = strlen(vlcAddr.sun_path) + sizeof(vlcAddr.sun_family) + 1;

  if (connect(s, (struct sockaddr *)&vlcAddr, len) == -1) {
    printf("ERROR\n");
    diep("vlc connect()");
  }
  printf("Connected\n");
  debug_exi_func(__func__);
  return s;
}


////////////////////////////////////////////////////////////////////////////////
/// Translate a command specified by op and para to a VLC RC compatible string.
////////////////////////////////////////////////////////////////////////////////
static void _getCmdStr(char *cmdStr, vlcOpCode_t op, uint32_t para) {
  memset(cmdStr, 0, VLC_MAX_CMD_LEN);
  switch(op) {
    case VLC_OP_PAUSE: 
      sprintf(cmdStr, "pause\n"); break;
    case VLC_OP_PLAY:
      sprintf(cmdStr, "pause\n"); break;
    case VLC_OP_SEEK:
      sprintf(cmdStr, "seek %d\n", para);
      break;
    case VLC_OP_GET_TIME:
      sprintf(cmdStr, "get_time\n");
      break;
    case VLC_OP_GET_LENGTH:
      sprintf(cmdStr, "get_length\n");
      break;
    case VLC_OP_GET_STATUS:
      sprintf(cmdStr, "status\n");
      break;
    case VLC_OP_IS_PLAYING:
      sprintf(cmdStr, "is_playing\n");
      break;
    default: assert(1);
  }
}

////////////////////////////////////////////////////////////////////////////////
/// Send a command to VLC
////////////////////////////////////////////////////////////////////////////////
void vlcSendCmd(vlcOpCode_t op, int para, vlcInterface_t *sock) {
  debug_ent_func(__func__);
  _sendCmd(op, para, sock);
  debug_exi_func(__func__);
}
////////////////////////////////////////////////////////////////////////////////
/// Send a command pointed by cmdStr through sock.
////////////////////////////////////////////////////////////////////////////////
static void _sendCmd(vlcOpCode_t op, int para, vlcInterface_t *sock) {
  char cmdStr[VLC_MAX_CMD_LEN];
  _exhaustReturnData(sock);
  _getCmdStr(cmdStr, op, para);
  debugf("_sendCmd: %s\n", cmdStr);
  if (send(sock->s, cmdStr, strlen(cmdStr), 0) == -1) {
      diep("vlc send()");
  }
}
////////////////////////////////////////////////////////////////////////////////
/// Read one message from VLC sock. Blocking if nothing to read.  
////////////////////////////////////////////////////////////////////////////////
static int _readOneBlocking(char *buf, vlcInterface_t *sock) {
    int t;
    if ((t=recv(sock->s, buf, VLC_MAX_READ_BUF, 0)) > 0) {
      buf[t] = '\0';
    }
    else {
      if (t < 0) diep("vlc recv()");
      else {
        printf("Lost connection to VLC.\n");
        exit(0);
      }
    }
    return t;
}
////////////////////////////////////////////////////////////////////////////////
/// Read one message from VLC sock. Non-blocking if nothing to read.  
/// Return number of bytes read.
////////////////////////////////////////////////////////////////////////////////
static int _readOneNonBlocking(char *buf, vlcInterface_t *sock) {
  int t = 0, pollRes;
  if ((pollRes = poll(sock->pfds, 1, 0)) > 0) {
    if ((t=recv(sock->s, buf, VLC_MAX_READ_BUF, 0)) > 0) {
      buf[t] = '\0';
    }
    else {
      if (t < 0) diep("vlc recv()");
      else printf("Lost connection to VLC.\n");
      exit(0);
    }
  }
  return t;
}

////////////////////////////////////////////////////////////////////////////////
/// Exhaust everything current available in buffer.
////////////////////////////////////////////////////////////////////////////////
void _exhaustReturnData(vlcInterface_t *sock) {
  debug_ent_func(__func__);
  char buf[VLC_MAX_READ_BUF];
  int t;
  do {
    t = _readOneNonBlocking(buf, sock);
  } while (t > 0);
  debug_exi_func(__func__);
}

////////////////////////////////////////////////////////////////////////////////
/// Move the string to next line.
////////////////////////////////////////////////////////////////////////////////
void _moveBufToNextLine(char *buf) {
  int length = strlen(buf);
  char *ptr = strstr(buf, "\r\n");
  if (ptr){
    int p = (int)(ptr - buf) + 2;
    for (int i = 0; p + i <= length; i++) {
      buf[i] = buf[p + i];
    }
  }
}
////////////////////////////////////////////////////////////////////////////////
/// Read a interger from VLC sock.
////////////////////////////////////////////////////////////////////////////////
int _readInteger(vlcInterface_t *sock) {
  int n;
  bool_t gotIt = FALSE;
  char buf[VLC_MAX_READ_BUF];
  while (1) {
    _readOneBlocking(buf, sock);
    int prevLength = strlen(buf);
    while (strlen(buf) > 0) {
      if (IS_DIGIT(buf[0])) {
        n = atoi(buf);
        gotIt = TRUE;
      }
      _moveBufToNextLine(buf);
      if (strlen(buf) == prevLength) {
        _exhaustReturnData(sock);
        return -1;
      }
      else
        prevLength = strlen(buf);
    }
    if (gotIt)
      break;
  } 
  return n;
} 

////////////////////////////////////////////////////////////////////////////////
/// Send a command and read back the result (integer).
////////////////////////////////////////////////////////////////////////////////
int _sendAndReadInteger(vlcOpCode_t op, vlcInterface_t *sock) {
  int n;
  do {
    _sendCmd(op, 0, sock);
    n = _readInteger(sock);
  } while(n < 0);
  return n;
}
////////////////////////////////////////////////////////////////////////////////
/// Analyze play state from given buffer.
////////////////////////////////////////////////////////////////////////////////
vlcPlayState_t _analyzePlayState(char *buf, int *para) {
  char *ptr;
  vlcPlayState_t res = VLC_STAT_INVALID;
  if (strstr(buf, "play")) {
    res = VLC_STAT_PLAY;
  }
  else if (strstr(buf, "pause")) {
    res = VLC_STAT_PAUSE;
  }
  else if ((ptr = strstr(buf, "time")) != NULL) {
    int time = atoi(&ptr[6]);
    *para = time;
    res = VLC_STAT_SEEK;
  }
  debugf("_analyzePlayState: %d\n", res);
  return res;
}

////////////////////////////////////////////////////////////////////////////////
/// Analyze play state from given buffer for polling.
/// Only return valid state when the message is generated by vlc proactively.
////////////////////////////////////////////////////////////////////////////////
vlcPlayState_t _analyzePlayStatePoll(char *buf, int *para) {
  char *ptr;
  vlcPlayState_t res = VLC_STAT_INVALID;
  if (strstr(buf, "status change")) {
    if (strstr(buf, "play")) {
      res = VLC_STAT_PLAY;
    }
    else if (strstr(buf, "pause")) {
      res = VLC_STAT_PAUSE;
    }
    else if ((ptr = strstr(buf, "time")) != NULL) {
      int time = atoi(&ptr[6]);
      *para = time;
      res = VLC_STAT_SEEK;
    }
  }
  debugf("_analyzePlayStatePoll: %d\n", res);
  return res;
}
////////////////////////////////////////////////////////////////////////////////
/// Get lenght of the playing video in second.
////////////////////////////////////////////////////////////////////////////////
uint32_t vlcGetLength(vlcInterface_t *sock) {
  // send "get_length" command to VLC and read back the lenght
  int len = _sendAndReadInteger(VLC_OP_GET_LENGTH, sock);
  return len;
}


////////////////////////////////////////////////////////////////////////////////
/// Get time elapsed since the begining of the video in second.
////////////////////////////////////////////////////////////////////////////////
uint32_t vlcGetTime(vlcInterface_t *sock) {
  // send "get_time" command to VLC and read back current time
  int time = _sendAndReadInteger(VLC_OP_GET_TIME, sock);
  return time;
}

////////////////////////////////////////////////////////////////////////////////
/// Get current playback state, including whether it's playing or paused
///   and time elapsed.
////////////////////////////////////////////////////////////////////////////////
vlcPlayState_t vlcGetPlayState(vlcInterface_t *sock) {
  // send "status" to VLC to get current playing status.
  //  This can be a little tricky since it seems the return of this command is
  //  not consistent. For example, if "status" is the very first command ever
  //  get sent to a brand-newly opened VLC, it doesn't return current playback
  //  status. Therefore we may need to consider send some dummy command to VLC
  //  as a part of VLC RC sock connecting process.
  debug_ent_func(__func__);
  char buf[VLC_MAX_READ_BUF];
  int t, para;
  vlcPlayState_t stat = VLC_STAT_INVALID;
  while (stat > VLC_STAT_PLAY) {
    _sendCmd(VLC_OP_GET_STATUS, 0, sock);
    while ((t = _readOneBlocking(buf, sock)) > 0) {
      stat = _analyzePlayState(buf, &para);
      if (stat <= VLC_STAT_PLAY) {
        break;
      }
    }
  }
  _exhaustReturnData(sock);
  debug_exi_func(__func__);
  return stat;
}

////////////////////////////////////////////////////////////////////////////////
/// Poll current VLC playback state.
////////////////////////////////////////////////////////////////////////////////
vlcPlayState_t vlcPollPlayState(vlcInterface_t *sock, int *para) {
  //debug_ent_func(__func__);
  char buf[VLC_MAX_READ_BUF];
  int t;
  vlcPlayState_t stat = VLC_STAT_INVALID;
  while ((t = _readOneNonBlocking(buf, sock)) > 0) {
    debugf("%s: %s\n", __func__, buf);
    stat = _analyzePlayStatePoll(buf, para);
    if (stat != VLC_STAT_INVALID) {
      break;
    }
  }
  //debug_exi_func(__func__);
  return stat;
}

////////////////////////////////////////////////////////////////////////////////
/// Poll current VLC playback status.
////////////////////////////////////////////////////////////////////////////////
vlcStatus_t vlcPollStatus(vlcInterface_t *sock) {
  //debug_ent_func(__func__);
  int para;
  vlcStatus_t status;
  status.stat = vlcPollPlayState(sock, &para);
  if (status.stat != VLC_STAT_INVALID) {
    switch(status.stat) {
      case VLC_STAT_PAUSE:
      case VLC_STAT_PLAY:
        status.time = vlcGetTime(sock);
        break;
      case VLC_STAT_SEEK:
        status.stat = vlcGetPlayState(sock);
        status.time = para;
        break;
      default: assert(1);
    }

  }
  //debug_exi_func(__func__);
  return status;
}
////////////////////////////////////////////////////////////////////////////////
/// Get whether a video is opened in VLC. 
////////////////////////////////////////////////////////////////////////////////
bool_t vlcGetIsOpened(vlcInterface_t *sock) {
  // This fucntion gets whether a file is opened in VLC,
  // which mathches the ruturn of "is_playing".
  // CAUTION: Return true as long as a file is opened in VLC, even it's paused.
  int playing = _sendAndReadInteger(VLC_OP_IS_PLAYING, sock);
  return (playing ? TRUE : FALSE);
}

////////////////////////////////////////////////////////////////////////////////
/// 
////////////////////////////////////////////////////////////////////////////////
void vlcSetStatus(vlcStatus_t *status, vlcInterface_t *sock){
  // The program shouldn't make any assumption of the current status of VLC
  // It's important to check current VLC status before set the status since command "pause"
  // toggles play and pause.
  debug_ent_func(__func__);
  vlcPlayState_t currStat = vlcGetPlayState(sock);
  bool_t isPaused = (currStat == VLC_STAT_PAUSE);
  switch (status->stat) {
    case VLC_STAT_PLAY:
      if (isPaused) _sendPause(sock);
      _seekTime(status->time, sock);
      break;
    case VLC_STAT_PAUSE:
      if (isPaused) _sendPause(sock);
      _seekTime(status->time, sock);
      _sendPause(sock);
      break;
  }
  debug_exi_func(__func__);
}

////////////////////////////////////////////////////////////////////////////////
/// Send a "pause" command to VLC
////////////////////////////////////////////////////////////////////////////////
void _sendPause(vlcInterface_t *sock) {
  _sendCmd(VLC_OP_PAUSE, 0, sock);
  _exhaustReturnData(sock);
}

////////////////////////////////////////////////////////////////////////////////
/// Send a "seek" command to VLC
////////////////////////////////////////////////////////////////////////////////
void _seekTime(int para, vlcInterface_t *sock) {
  _sendCmd(VLC_OP_SEEK, para, sock);
  _exhaustReturnData(sock);
}

