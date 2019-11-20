#include <netinet/tcp.h>
#include <netinet/in.h>
#include <string.h>
#include <thread>
#include <unistd.h>

//Semaphore lib
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/ioctl.h>

#include "socket.h"
#include "debugger.h"

#define MAX_LOOP 10000

#define SEM_SOCKET_FD 0
#define SEM_READ_LOCK 1
#define SEM_WRITE_LOCK 2

namespace ASSC 
{

/**
 * configure default settings for socket
 */
void configureSocketOpts(int sfd){
    int curFlags = ::fcntl(sfd, F_GETFL, 0);

    // if(fcntl(sfd, F_SETFL, curFlags|O_NONBLOCK|O_DSYNC) < 0){
    //     throw FAIL_NON_BLOCK_SOCKET;
    // }
    if(fcntl(sfd, F_SETFL, curFlags|O_DSYNC) < 0){
        throw FAIL_NON_BLOCK_SOCKET;
    }
    int flag = 1;
    // No need for unix domain socket.
    // if (setsockopt(
    //         sfd,            /* socket affected */
    //         SOL_SOCKET,     /* set option at TCP level */
    //         TCP_NODELAY,     /* name of option */
    //         (char *) &flag,  /* the cast is historical cruft */
    //         sizeof(int) /* length of option value */
    //     ) < 0){
    //     throw errno;
        // EACCES,EOPNOTSUPP
    // }
}
    
/**
 * Default initializer
 */
void Socket::init(){
    FD_ZERO(&fds);
    FD_SET(sfd, &fds);
    tv.tv_sec=0;
    tv.tv_usec=0;
    
    try{
        configureSocketOpts(sfd);
        int ilen = sizeof(int);
        sendBufferSize=0;
        getsockopt(sfd, SOL_SOCKET, SO_SNDBUF, &sendBufferSize, (socklen_t *)&ilen);
        DBG::printf("inital sendb : %d",sendBufferSize);    
    }catch(int err){
        DBG::printf("err %d",err);    
        if(err == FAIL_NON_BLOCK_SOCKET)
            throw "Could not create Non-blocking socket";
    }
}

Socket::Socket(){
    // Create socket pair
    int sfds[2];
    if(socketpair(AF_UNIX, SOCK_STREAM, AF_LOCAL, sfds)<0){
        //fail to create pair
        throw "Socket Creation failed";
    }
    sfd = sfds[0];
    csfd = sfds[1];
    // Create Semaphore
    semId = semget(IPC_PRIVATE, 3, 0666| IPC_CREAT);
    DBG::printf("semId : %d, socket Id : %d", semId, csfd);
    semVal.val = csfd;
    if(semctl(semId, 0, SETVAL, semVal)== -1){
        // Error setting semaphore;
        throw "Semaphore error";
    };
    semVal.val = 1;
    semctl(semId, LOCK_SOCKET_READ, SETVAL, semVal);
    semctl(semId, LOCK_SOCKET_WRITE, SETVAL, semVal);
    init();
}
            
Socket::Socket(int semId):semId(semId){
    csfd = -1;
    // Retrieve fd from semaphore 
    sfd = semctl(semId, 0, GETVAL, semVal);
    if(sfd <  1){
        // Error setting semaphore;
        throw "Semaphore error";
    };
    DBG::printf("semId : %d, socket Id : %d", semId, sfd);
    init();
}

// Dealloc : socket close, 
Socket::~Socket(){
    // Master process only closes sockets and semaphores
    if(csfd > 0){
        close(sfd);
        close(csfd);
        // Delete semaphore
        semctl(semId, 0, IPC_RMID);
    }
}

char * Socket::read(){
    // Non-blocking thread block.
    if(!readMtx.try_lock()){
        return nullptr;
    }
    //Mutex start
    try {
        // throw when Nothing to read.
        if(select(sfd + 1, &fds, NULL, NULL, &tv) < 0)  throw 0;
        //Peek valid header stream
        bzero(headerBuffer, PACKET_HEADER_LENGTH);
        byteRed = ::recv(sfd, headerBuffer, PACKET_HEADER_LENGTH , MSG_PEEK|MSG_DONTWAIT);
        if(byteRed < 0 ) throw 1;
        //Header not fully received or header is not valid
        if((unsigned int)byteRed < PACKET_HEADER_LENGTH || !Packet::hasHeader(headerBuffer)) throw 2;
        // Acquire read lock.
        if(!lock(LOCK_SOCKET_READ)) {
            DBG::printf("lock failed");
            throw 3;
        }
        //File Lock start
        try{
            // Remove header from stream.
            DBG::printf("Remove Header Stream");
            bzero(headerBuffer, PACKET_HEADER_LENGTH);
            byteRed = ::recv(sfd, headerBuffer, PACKET_HEADER_LENGTH, MSG_WAITALL);
            if(byteRed < 0 || (unsigned int)byteRed < PACKET_HEADER_LENGTH || !Packet::hasHeader(headerBuffer)){
                DBG::printf("ERR remove header err- %d, %d\n", errno, byteRed);
                throw 4;
            }
            Packet::Header header = Packet::getHeader(headerBuffer);

            // Collect content stream
            DBG::printf("Collect Content Stream");
            unsigned long bytesLeft = header.dataLength + 1;
            char* buffer = new char[bytesLeft];
            char* bptr = buffer;
            bzero(buffer, bytesLeft);
            int cnt = 0;
            //Buffur allocation catch.
            try {
                while(bytesLeft > 0 && cnt++ < MAX_LOOP){
                    byteRed = ::recv(sfd, bptr, bytesLeft, MSG_WAITALL);
                    if(byteRed > -1){
                        cnt = 0;
                        bytesLeft-= byteRed;
                        bptr = bptr + byteRed;
                        DBG::printf("Byte Received :  %d, Begin Byte : %d",byteRed, buffer[0]);
                        continue;
                    }
                    // Wait if stream not arrived yet.
                    if(errno == EAGAIN){
                        std::this_thread::sleep_for(std::chrono::milliseconds(1));
                        continue;
                    }
                    DBG::printf("Socket Read Error");
                    throw 5;
                }
                // Infinite loop failsafe.
                if(cnt >= MAX_LOOP){
                    DBG::printf("Infinite loop");
                    throw 6;
                }
                if(buffer[header.dataLength] != PACKET_CONTENT_BYTE_END){
                    DBG::printf("Content End mark currupted : %d,%d,%d, %s",buffer[header.dataLength],header.dataLength,bytesLeft, buffer);
                    throw 7;
                }
                buffer[header.dataLength] = 0;
                DBG::printf("Received : %s", buffer);
                unlock(LOCK_SOCKET_READ);
                readMtx.unlock();
                return buffer;
            }catch(...){
                delete[] buffer;
                throw;
            }
        }catch(...){
            DBG::printf("unlock socket on Err");
            unlock(LOCK_SOCKET_READ);
            throw;
        }
    }catch(...){
        readMtx.unlock();
        return nullptr;
    }
}

/**
 * Write to socket
 * Should be Synchronous task to garantee writing.
 */
bool ASSC::Socket::write(const char * data, unsigned long length){
    writeMtx.lock();
    unsigned remainBytes = PACKET_HEADER_LENGTH + length + 1;
    // write Mutex start
    try{
        // throw when unable to write.
        if(select(sfd + 1, NULL, &fds, NULL, &tv) < 0)  throw 0;

        // Throw when send buffer is full(Prevent block)
        int curBufferSize=0;
        ioctl(sfd, TIOCOUTQ, &curBufferSize );  // alternative 1
        // ioctl( socket_descriptor, SIOCOUTQ, &size );  // alternative 2
        if((sendBufferSize - curBufferSize) < (int)remainBytes){
            DBG::printf("Send Buffer not enough. total :  %d,unSent : %d, remain : %d, toSend : %d.",sendBufferSize ,curBufferSize, sendBufferSize - curBufferSize , remainBytes);
            throw 1;
        }
        //Socket Lock
        if(!lock(LOCK_SOCKET_WRITE)) throw 1;
        DBG::printf("write locked");
        try {
            char *wrappedData = Packet::wrap(data, length);
            char *dataPtr = wrappedData;
            
            //SMemery management
            try{
                int cnt = 0;
                while(remainBytes > 0 && cnt++ < MAX_LOOP){
                    byteWriten = ::send(sfd, dataPtr, remainBytes, 0);
                    DBG::printf("writen :  %d, %d",byteWriten, errno);
                    if (byteWriten < 0) throw FAIL_WRITE_SOCKET;
                    if(byteWriten > 0){
                        DBG::printf("writen :  %d",byteWriten);
                        remainBytes -= byteWriten;
                        dataPtr += byteWriten;
                        cnt = 0;
                        continue;
                    }
                    DBG::printf("Socket Write 0");
                    std::this_thread::sleep_for(std::chrono::milliseconds(1));
                }
                DBG::printf("write done : %s", data);
                delete[] wrappedData;
                writeMtx.unlock();
                unlock(LOCK_SOCKET_WRITE);
                return remainBytes == 0;
            }catch(...){
                delete[] wrappedData;
                throw;
            }
        }catch(...){
            unlock(LOCK_SOCKET_WRITE);
            throw;
        }
    }catch(int err){
        //unlock write mutex
        writeMtx.unlock();
        return false;
    }

    writeMtx.unlock();
    return remainBytes == 0;
}

/**
 * Lock 
 * Since threads can share memory, Securing context memory is primary for reuse flock struct.
 * fd_lck should be two.
 * flock() cannot block parent child processes. (However, might be useful for case when shared lock in forked processes.)
 * Mutex required for access restriction of flock struct.
 */
bool ASSC::Socket::lock(unsigned short type){
    DBG::printf("locking");
    sops.sem_num = type;
    sops.sem_op =  -1;
    sops.sem_flg =  SEM_UNDO;  //Block on write, rollback on process fail;
    // Check semaphore value
    if(semctl(semId, type, GETVAL, semVal) == 0){
        DBG::printf("lock failed1 : %d", type);
        return false;
    }
    // Immediate fail on read.
    sops.sem_flg |= IPC_NOWAIT;
    if(semop(semId, &sops,1) == -1){
        DBG::printf("lock failed2:%d",errno);
        return false;
    }
    return true;
}

/**
 * Unlock :should be in order : process-thread.
 * Since threads can share memory, Securing context memory is primary for reuse flock struct.
 * flock() cannot block parent child processes. (However, might be useful for case when shared lock in forked processes.)
 */
bool ASSC::Socket::unlock(unsigned short type){
    DBG::printf("unlocking");
    // Check semaphore value
    int val = semctl(semId, type, GETVAL, semVal);
    if(val == 1){
        DBG::printf("unlocked already: %d, %d",type, val);
        return true;
    }
    sops.sem_num = type;
    sops.sem_op = 1;
    semop(semId, &sops,1);
    val = semctl(semId, type, GETVAL, semVal);
    DBG::printf("unlocked: %d, %d",type, val);
    return true;
}

} //namespace ASSC