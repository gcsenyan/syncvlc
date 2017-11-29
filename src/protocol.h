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


enum opCode_t {
  OP_NOP,
  OP_PLAY,
  OP_PAUSE,
  OP_SEEK,
  NUM_OP
};

typedef uint32_t vlcOpCode_t;
typedef struct {
  vlcOpCode_t op;
  uint32_t para;
}vlcCmd_t;

typedef struct {
  uint64_t seqNum;
  vlcCmd_t vlcCmd;
}pkt_t;
#endif // PROTOCOL_H
