#ifndef PRO_H
#define PRO_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <signal.h>
#include <netdb.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/select.h>
#include <errno.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stddef.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <sys/timerfd.h>
#include <stdint.h>
#include <ctype.h>
#include <openssl/md5.h>

#define TRUE 1
#define FALSE 0

volatile char A;
volatile int connectedClients;
timer_t first;
timer_t second;
FILE* fdR;
time_t boottime;
struct timespec monotime;
struct timespec realtime;
volatile int generated;
volatile int poped;
volatile int popedStart;

typedef struct circ_buffer_t {
	char* buffer; //data buf
	char* buffer_end; //end of data buf;
	size_t capacity; //max num of items in the buf: 1,25MB/640B=1953B
	size_t count; //num of items in the buffer
	size_t sz; //size of each item - 640B	
	char* head;
	char* tail;
} circ_buffer_t;

circ_buffer_t cB;

void error(const char *msg);
void cbInit(circ_buffer_t *cb, size_t capacity, size_t sz);
void cbFree(circ_buffer_t *cb);
void cbPush(circ_buffer_t *cb, const void* item);
void cbPop(circ_buffer_t *cb, void* item);
void genData(circ_buffer_t *cb, char A);
void writeRaport(circ_buffer_t *cb);

#endif
