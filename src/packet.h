#ifndef PACKET_H_
#define PACKET_H_

#include <memory>
/**
 * Wrap data with header and content wrapper 
 * [ HST(1), msgNo(4), dataLength(8), HED(1), data(varies), BED(1)]
 */
#define PACKET_HEADER_LENGTH 2 + sizeof(int) + sizeof(unsigned long)
#define PACKET_HEADER_BYTE_BEGIN 0x01   //Header Start byte
#define PACKET_HEADER_BYTE_END 0x02 //Header End byte
#define PACKET_CONTENT_BYTE_END 0x03    //Content End bype

namespace Packet{
    struct Header {
        int msgNo;
        unsigned long dataLength;
        Header(int msgNo, unsigned long dataLength):msgNo(msgNo),dataLength(dataLength){}
    };

    bool hasHeader(const char* data);
    bool isValidContent();
    Header getHeader(const char* data);
    char* wrap(const char* data, unsigned long length);
    char* unwrap(const char* data);
}

#endif