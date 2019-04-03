#include "producent.h"

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

void cbInit(circ_buffer_t *cb, size_t capacity, size_t sz)
{
	cb->buffer=malloc(capacity*sz);
	if(cb->buffer==NULL)
		error("cb_init: malloc error");
	cb->buffer_end=(char*)cb->buffer+capacity*sz;
	cb->capacity=capacity;
	cb->count=0;
	cb->sz=sz;
	cb->head=cb->buffer;
	cb->tail=cb->buffer;
}

void cbFree(circ_buffer_t *cb)
{
	free(cb->buffer);
}

void cbPush(circ_buffer_t *cb, const void* item)
{
	memcpy(cb->head, item, cb->sz);
	cb->head=(char*)cb->head+cb->sz;
	if(cb->head==cb->buffer_end)
		cb->head=cb->buffer;
	cb->count++;
	generated++;
}

void cbPop(circ_buffer_t *cb, void* item) //wyślij 112KB=114 688 bajtów
{	
	while(cb->count<179); //za malo blokow w buf - w 112KB jest 179 bloków po 640B
	if(poped%10==0) //1,25MB/114 688B=10
		cb->tail=cb->buffer;
	memcpy(item,cb->tail,114688);
	cb->tail=(char*)cb->tail+114688;
	if(cb->tail==cb->buffer_end)
		cb->tail=cb->buffer;
	cb->count-=179;
	poped++;
}

void genData(circ_buffer_t *cb, char A) //stworz blok rozmariu 640bajtów
{
	if(cb->count==cb->capacity)
		return;

	char arr[640]; //amount of data i want to generate+1;
	if(A<='Z')
	{
		memset(arr,(int)A, 640); 
		//arr[641]='\0'; 
		cbPush(cb,arr); //dodaj tyle bajtów do buffora
	}
	if(A>'Z')
	{
		memset(arr,(int)A+6,640);
		//arr[641]='\0';
		cbPush(cb,arr);
	}
}

void writeRaport(circ_buffer_t *cb)
{
	clock_gettime(CLOCK_MONOTONIC, &monotime);
	time_t curtime=boottime+monotime.tv_sec;
	clock_gettime(CLOCK_REALTIME, &realtime);
	size_t inMag=cb->count*cb->sz;
	size_t percent=inMag*100/1250000;	
	fprintf(fdR,"Connected clients: %d, in magazine: %zu, percent: %zu. Mono time: %s, WallTime: %s. Amount of sent blocks: %d, flow of data: %d\n", connectedClients, inMag, percent, ctime(&curtime), ctime(&realtime.tv_sec), poped, ((generated-poped)*640));
	fflush(fdR);
}
