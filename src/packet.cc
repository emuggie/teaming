#include "packet.h"
#include <sys/file.h>
#include <string.h>

namespace Packet 
{

char* wrap(const char* data, unsigned long length){
    unsigned long byteLength = PACKET_HEADER_LENGTH + length + 1;
    
    // TODO : Ensure unique
    int msgNo = 1;
    // int msgNo = std::chrono::duration_cast<std::chrono::milliseconds>(
    //     std::chrono::time_point_cast<std::chrono::milliseconds>(
    //         std::chrono::high_resolution_clock::now()
    //     ).time_since_epoch()
    // ).count();

    char* buffer = new char[byteLength];
    bzero(buffer, byteLength);
    try{
        char* pos = buffer;
        pos[0] = PACKET_HEADER_BYTE_BEGIN;
        pos = pos + 1;
        pos = (char*)mempcpy(pos, &msgNo, sizeof(msgNo));
        pos = (char*)mempcpy(pos, &length, sizeof(unsigned long));
        pos[0]=PACKET_HEADER_BYTE_END;
        pos = pos + 1;
        pos = (char*)mempcpy(pos, data, length);
        pos[0] = PACKET_CONTENT_BYTE_END;
        return buffer;
    }catch(int err){
        delete[] buffer;
        throw err;
    }
}

bool hasHeader(const char* data){
    if(data[0] != PACKET_HEADER_BYTE_BEGIN){
        return false;
    }
    if(data[PACKET_HEADER_LENGTH - 1] != PACKET_HEADER_BYTE_END){
        return false;
    }       
    return true;
}

Header getHeader(const char* data){
    if(!hasHeader(data)){
        Header header(-1,0);
        return header;    
    }
    Header header((int)data[1],(unsigned long)data[5]);
    return header;
}

std::shared_ptr<char> unwrap(char* data){
    return nullptr;
}

}