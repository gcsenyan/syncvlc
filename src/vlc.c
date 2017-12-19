
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
vlcPlayStat_t _analyzePlayStatus(char *buf);


////////////////////////////////////////////////////////////////////////////////
/// Initialize VLC interface.
////////////////////////////////////////////////////////////////////////////////
void vlcInit(char *path, vlcInterface_t *sock) {
  sock->s = vlcConnect(path);
  sock->pfds[0].fd = sock->s;
  sock->pfds[0].events = POLLIN;
}

////////////////////////////////////////////////////////////////////////////////
/// Connet the unix socket created by VLC for Remote Control (RC).
////////////////////////////////////////////////////////////////////////////////
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
void vlcSendCmd(vlcOpCode_t op, int para, vlcInterface_t *sock) {
  _sendCmd(op, para, sock);
}
////////////////////////////////////////////////////////////////////////////////
/// Send a command pointed by cmdStr through sock.
////////////////////////////////////////////////////////////////////////////////
static void _sendCmd(vlcOpCode_t op, int para, vlcInterface_t *sock) {
  char cmdStr[VLC_MAX_CMD_LEN];
  _getCmdStr(cmdStr, op, para);
  //printf("Sending: %s\n", cmdStr);
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
    }
  }
  return t;
}

void _exaustReturnData(vlcInterface_t *sock) {
  char buf[VLC_MAX_READ_BUF];
  int t;
  do {
    t = _readOneNonBlocking(buf, sock);
  } while (t > 0);
}

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
    while (strlen(buf) > 0) {
      if (IS_DIGIT(buf[0])) {
        n = atoi(buf);
        gotIt = TRUE;
      }
      _moveBufToNextLine(buf);
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
  _sendCmd(op, 0, sock);
  int n = _readInteger(sock);
  return n;
}
////////////////////////////////////////////////////////////////////////////////
/// Analyze play status from given buffer. 
////////////////////////////////////////////////////////////////////////////////
vlcPlayStat_t _analyzePlayStatus(char *buf) {
  if (strstr(buf, "play")) {
    return VLC_STAT_PLAY;
  }
  else if (strstr(buf, "pause")) {
    return VLC_STAT_PAUSE;
  }
  else {
    return VLC_STAT_INVALID;
  }
}
////////////////////////////////////////////////////////////////////////////////
/// Get lenght of the playing video in second.
////////////////////////////////////////////////////////////////////////////////
uint32_t vlcGetLength(vlcInterface_t *sock) {
  //TODO send "get_length" command to VLC and read back the lenght
  int len = _sendAndReadInteger(VLC_OP_GET_LENGTH, sock);
  return len;
}


////////////////////////////////////////////////////////////////////////////////
/// Get time elapsed since the begining of the video in second.
////////////////////////////////////////////////////////////////////////////////
uint32_t vlcGetTime(vlcInterface_t *sock) {
  //TODO send "get_time" command to VLC and read back current time
  int time = _sendAndReadInteger(VLC_OP_GET_TIME, sock);
  return time;
}

////////////////////////////////////////////////////////////////////////////////
/// Get current playback status, including whether it's playing or paused 
///   and time elapsed.
////////////////////////////////////////////////////////////////////////////////
vlcPlayStat_t vlcGetPlayStatus(vlcInterface_t *sock) {
  //TODO send "status" to VLC to get current playing status.
  //  This can be a little tricky since it seems the return of this command is
  //  not consistent. For example, if "status" is the very first command ever
  //  get sent to a brand-newly opened VLC, it doesn't return current playback
  //  status. Therefore we may need to consider send some dummy command to VLC
  //  as a part of VLC RC sock connecting process.
  char buf[VLC_MAX_READ_BUF];
  int t;
  vlcPlayStat_t stat = VLC_STAT_INVALID;
  while (stat == VLC_STAT_INVALID) {
    _sendCmd(VLC_OP_GET_STATUS, 0, sock);
    while ((t = _readOneNonBlocking(buf, sock)) > 0) {
      stat = _analyzePlayStatus(buf);
      if (stat != VLC_STAT_INVALID) {
        break;
      }
    }
  }
  return stat;
}

vlcPlayStat_t vlcPollPlayStatus(vlcInterface_t *sock) {
  char buf[VLC_MAX_READ_BUF];
  int t;
  vlcPlayStat_t stat = VLC_STAT_INVALID;
  while ((t = _readOneNonBlocking(buf, sock)) > 0) {
    stat = _analyzePlayStatus(buf);
    if (stat != VLC_STAT_INVALID) {
      break;
    }
  }
  return stat;
}
////////////////////////////////////////////////////////////////////////////////
/// Get whether a video is opened in VLC. 
////////////////////////////////////////////////////////////////////////////////
bool_t vlcGetIsOpened(vlcInterface_t *sock) {
  //TODO
  // This fucntion gets whether a file is opened in VLC,
  // which mathches the ruturn of "is_playing".
  // CAUTION: Return true as long as a file is opened in VLC, even it's paused.
  int playing = _sendAndReadInteger(VLC_OP_IS_PLAYING, sock);
  return (playing ? TRUE : FALSE);
}

////////////////////////////////////////////////////////////////////////////////
/// 
////////////////////////////////////////////////////////////////////////////////
void vlcSetStatus(vlcStatus_t *stat){
  //TODO
  // The program shouldn't make any assumption of the current status of VLC
  // It's important to check current VLC status before set the status since command "pause"
  // toggles play and pause.

}

void vlcSendPause(vlcInterface_t *sock) {
  _sendCmd(VLC_OP_PAUSE, 0, sock);
  _exaustReturnData(sock);
}

