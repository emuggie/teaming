#ifndef SOCKET_H_
#define SOCKET_H_

#include <sys/socket.h>
#include <memory>
#include <mutex>
#include <fcntl.h>
#include <sys/sem.h>

#include "packet.h"

#define FAIL_NON_BLOCK_SOCKET  0    // Failed to enable non-blocking socket.
#define FAIL_DISABLE_NAGLE_AGL  1   // Failed to disable nagle algorithm.

#define FAIL_READ_SOCKET 2
#define FAIL_WRITE_SOCKET 3
#define ERR_CONTENT_MIX 4
#define ERR_LOOP_INFINITE 5

#define LOCK_SOCKET_READ SEM_READ_LOCK
#define LOCK_SOCKET_WRITE SEM_WRITE_LOCK

// Lock statue
#define LOCK_STATUS_UNLOCKED 0
#define LOCK_STATUS_LOCKED 1

namespace ASSC
{

union SemVal {
    int val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
    struct seminfo  *__buf;  /* Buffer for IPC_INFO(Linux-specific) */
};

class Socket {
    public:
        Socket();
        Socket(int fd);
        ~Socket();
        char* read();
        bool write(const char * data, unsigned long length);

        int getSfd() {
            return sfd;
        }

        int getCsfd() {
            return csfd;
        }
        
        int getSemId() {
            return semId;
        }
    private:
        void init();
        bool lock(unsigned short type);
        bool unlock(unsigned short type);
        int sfd;
        int csfd;
        fd_set fds;
        timeval tv;
        char headerBuffer[PACKET_HEADER_LENGTH];
        int sendBufferSize;
        int byteRed;
        int byteWriten;
        std::mutex readMtx;
        std::mutex writeMtx;
        int semId;
        SemVal semVal;
        flock fdLock ;
        sembuf sops;
};

}
#endif