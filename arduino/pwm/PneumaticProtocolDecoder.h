#ifndef PNEUMATIC_PROTOCOL_DECODER_H__
#define PNEUMATIC_PROTOCOL_DECODER_H__

#include "pneumatic.h"

class PneumaticProtocolDecoder {
private:
public:

enum {
   IDLE = 0,
   CMD_BYTE_RECVD
   } status;

   int msgBytesLeft;
   int bufIndex;

   byte cmd;
   byte buf[8];

   PneumaticProtocolDecoder() {
      status = IDLE;
      msgBytesLeft = 0;
      bufIndex = 0;
      cmd = 0;
   }

   void handleByte(const byte b);
   void handleCommand();

};

#endif /* PNEUMATIC_PROTOCOL_DECODER_H__ */