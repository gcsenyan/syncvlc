/*
 * Project: syncvlc
 * File: protocol.h
 * 
 * UIUC ECE438 Fall 2017 Final Project
 * Synchronized Video Playback Control
 * 
 * Author: Ce Guo (ceguo2) 
 *         Xinyi Xie (xinyix6)
 * 
 */


#ifndef PROTOCOL_H
#define PROTOCOL_H
#include <stdint.h>

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

// VLC play status.
enum {
  VLC_STAT_PAUSE,
  VLC_STAT_PLAY,
  NUM_STAT
};

// Packet type.
enum {
  PKT_HANDSHAKE,
  PKT_HEARTBEAT,
  PKT_SYNC
};

typedef uint32_t vlcOpCode_t;
typedef uint32_t vlcPlayStat_t;
typedef uint32_t pktType_t;

typedef struct {
  vlcOpCode_t op;
  uint32_t para;
}vlcCmd_t;

typedef struct {
  vlcPlayStat_t stat;
  uint32_t time;
}vlcStatus_t;

typedef struct {
  uint32_t seqNum;
  pktType_t pktType;
  vlcStatus_t vlcStat;
}pkt_t;
#endif // PROTOCOL_H
