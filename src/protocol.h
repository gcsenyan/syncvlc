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
#include "vlc.h"


// Packet type.
enum {
  PKT_HANDSHAKE,
  PKT_HEARTBEAT,
  PKT_SYNC
};

typedef uint32_t pktType_t;

typedef struct {
  uint32_t seqNum;
  uint64_t timestamp;
  pktType_t pktType;
  vlcStatus_t vlcStat;
}pkt_t;
#endif // PROTOCOL_H
