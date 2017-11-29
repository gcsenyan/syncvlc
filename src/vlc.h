

#ifndef VLC_H
#define VLC_H

socket_t vlcConnect(char *path);
void vlcSendCmd(socket_t s, vlcCmd_t *cmd);

#endif // VLC_H