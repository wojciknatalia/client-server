#include "producent.h"

int opt=TRUE;
int masterSocket;
int addrlen;
int newSocket;
int clientSocket[1000];
int maxClients=1000;
int active;
int valread;
int sd;
int max_sd;

static void timerHandler(int sig, siginfo_t *si, void *uc)
{
	timer_t *tidp;
	tidp=si->si_value.sival_ptr;
	if(*tidp==first)
		writeRaport(&cB);
	else if(*tidp==second)
	{
		genData(&cB, A);
		A++;
		if(A>'t')
			A='A';
	}
	else
		return;

}

static int makeTimer(char* name, timer_t *timerID, float interval)
{
	struct sigevent te;
	struct itimerspec its;
	struct sigaction sa;
	int sigNo=SIGRTMIN;
	static long secToNano=1000000000;

	sa.sa_flags=SA_SIGINFO;
	sa.sa_sigaction=timerHandler;
	sigemptyset(&sa.sa_mask);
	if(sigaction(sigNo,&sa,NULL)==-1)
		error("sigaction");

	te.sigev_notify=SIGEV_SIGNAL;
	te.sigev_signo=sigNo;
	te.sigev_value.sival_ptr=timerID;
	timer_create(CLOCK_REALTIME,&te,timerID);

	its.it_value.tv_sec=(time_t)interval;
	its.it_value.tv_nsec=(interval -(time_t)interval)*secToNano;
	its.it_interval.tv_sec=(time_t)interval;
	its.it_interval.tv_nsec=(interval - (time_t)interval)*secToNano;

	if(timer_settime(*timerID, 0, &its, NULL) == -1)
		error("timer settime");

	return 1;
}

void initializeConnection(int maxClients, struct sockaddr_in* addressMain, int PORT, int *masterSocket, int* clientSocket, int *addrlen, int opt)
{
	struct sockaddr_in address;
	address=*addressMain;

	for(int i=0; i<maxClients; i++)
		clientSocket[i]=0;

	//create master socket
	if((*masterSocket=socket(AF_INET, SOCK_STREAM, 0))==0)
		error("Error masterSocket");

	//set masterSocket to allow multiple connections
	if(setsockopt(*masterSocket, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt))<0)
		error("Error setsockopt");

	//type of created socket
	address.sin_family=AF_INET;
	address.sin_addr.s_addr=INADDR_ANY;
	address.sin_port=htons(PORT);

	//bind the socket
	if(bind(*masterSocket,(struct sockaddr*)&address, sizeof(address))<0)
		error("Error bindsocket");

	printf("Listener on port %d\n", PORT);

	if(listen(*masterSocket,3)<0)
		error("Error listen");

	//accept indoming connection
	*addrlen=sizeof(address);
	puts("Welcome to the Olympic Connections Games...");

	*addressMain=address;
}

void ioOperation(int maxClients, struct sockaddr_in* addressMain, FILE** fdR, volatile int *connectedClients, volatile int *poped, volatile int *popedStart, int* clientSocket, int* sd, fd_set* readfds, char* bufRead, int* valread)
{
	struct sockaddr_in address;
	address=*addressMain;

	for(int i=0; i<maxClients; i++)
		{
			*sd=clientSocket[i];

			if(FD_ISSET(*sd, *(&readfds)))
			{
				//check if it was for closing, read the incoming msg
				if((*valread=read(*sd,bufRead,1024))==0)
				{
					//somebody disconeccted, print his data
					getpeername(*sd,(struct sockaddr*)&address, (socklen_t*)&addrlen);
					clock_gettime(CLOCK_MONOTONIC, &monotime);
					time_t curtime=boottime+monotime.tv_sec;
					clock_gettime(CLOCK_REALTIME, &realtime);
					fprintf(*fdR,"Host disconnected, Monotonic time is %s, WallTime is %s, ip %s, port %d, sent blocks: %d \n", ctime(&curtime), ctime(&realtime.tv_sec), inet_ntoa(address.sin_addr), ntohs(address.sin_port), *poped-*popedStart);
					fflush(*fdR);
					*connectedClients-=1;

					//close the socket, mark 0 for reuse
					close(*sd);
					clientSocket[i]=0;
				}

				//send data, that was not disconnecting host
				else
				{	
					//set the terminating NULL on the end of the data read
					bufRead[*valread]='\0';	
					char* arr=malloc(sizeof(char)*(114689));
					arr[114689]='\0';
					cbPop(&cB, arr);
					send(*sd,arr,114688,0);
				}
			}
		}

	*addressMain=address;
}

void newConnection(int maxClients, struct sockaddr_in* addressMain, FILE** fdR, volatile int *connectedClients, volatile int *poped, volatile int* popedStart, int* clientSocket, int *newSocket)
{
	struct sockaddr_in address;
	address=*addressMain;

	clock_gettime(CLOCK_MONOTONIC, &monotime);
	time_t curtime=boottime+monotime.tv_sec;
	clock_gettime(CLOCK_REALTIME, &realtime);
	*popedStart=*poped;
	fprintf(*fdR,"New connection: Monotonic time is %s, WallTime is %s, socket fs is %d, ip is %s, port: %d\n", ctime(&curtime), ctime(&realtime.tv_sec), *newSocket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
	fflush(*fdR);
	printf("New connection: Monotonic time is %s, WallTime is %s, socket fs is %d, ip is %s, port: %d\n", ctime(&curtime), ctime(&realtime.tv_sec), *newSocket, inet_ntoa(address.sin_addr), ntohs(address.sin_port));
			
	*connectedClients+=1;
	
	//add new socket to array of sockets
	for(int i=0; i<maxClients; i++)
	{
		//if emty pos
		if(clientSocket[i]==0)
		{
			clientSocket[i]=*newSocket;
			break;
		}
	}
	
	*addressMain=address;
}

int main(int argc, char* argv[])
{

	connectedClients=0;

	char* road=NULL;
	float tempo=-1;
	char* hostName=NULL;

	int k;
	while((k=getopt(argc, argv, "r:t:")) != -1)
		switch(k)
		{
			case 'r':
				road=(char*)malloc((strlen(optarg)+1)*sizeof(char));
				strcpy(road, optarg);
				break;
			case 't':
				if(strtod(optarg,NULL) <1 || strtod(optarg,NULL) >8)
					error("Error with -t arg, bad value!");
				tempo=(60.f/9600.f)/strtof(optarg,NULL);
				break;
		}

	if(!argv[optind])
		error("Bad usage od parameters, pass mandatory PORT!");	
	
	int len=strlen(argv[optind]);
	hostName=(char*)malloc((len+1)*sizeof(char));
	char *e=strchr(argv[optind],':');
	char portC[5];
	int index=(int)(e-argv[optind]);
	int PORT=-1;

	if(e==NULL) //hostName=localHost
	{
		hostName=(char*)malloc((15)*sizeof(char));
		strcpy(hostName, "localhost");
		strcpy(portC, argv[optind]);
		int j=0;
		while(j<strlen(portC)){
				if(isdigit(portC[j]) == 0)
					error("Passed port is not a number!");
				j++;}
		PORT=strtol(portC,0,0);
	}
	else
	{
		strncpy(hostName,argv[optind], index);
		int index2=(int)(len-index);
		strncpy(portC, argv[optind]+index+1,index2);
		int j=0;
		while(j<strlen(portC)){
				if(isdigit(portC[j]) == 0)
					error("Passed port is not a number!");
				j++;}
		PORT=strtol(portC,0,0);
	}

	if(road == NULL || tempo == -1 || hostName == NULL || PORT == -1)
		error("Bad usage of arguments, pass mandatory parameters!");
	
	printf("sciezka: %s, tempo: %f, hostname: %s, port: %d\n", road,tempo,hostName, PORT);

	fdR=fopen(road, "w"); //Raport file desc
	if(fdR<0)
		error("Error opening file to write raports");

	//---------------------------------create cirle buffor
	A='A';
	cbInit(&cB, 1953, 640); //ile bloków, o jakich rozmiarach
	//----------------set timer
	makeTimer("first", &first, 5);
	makeTimer("second", &second, tempo);
	//------------time
	struct sysinfo info;
	sysinfo(&info);

	boottime=time(NULL)-info.uptime;
	
	struct sockaddr_in address;

	char bufRead[1024]; //data buffer

	//socket descriptors set
		
	initializeConnection(maxClients, &address, PORT, &masterSocket, clientSocket, &addrlen,opt);
	//initialise all clitentSocket tab to 0

	fd_set readfds;
	while(TRUE)
	{
		FD_ZERO(&readfds);

		//add master socket to set
		FD_SET(masterSocket, &readfds);
		max_sd=masterSocket;

		//add child sockets to set
		for(int i=0; i<maxClients; i++)
		{
			//socket descriptor
			sd=clientSocket[i];
			//if ok add to read list
			if(sd>0)
				FD_SET(sd, &readfds);
			//highest select desc
			if(sd>max_sd)
				max_sd=sd;
		}

		//wait indefinitly for an activity
		active=select(max_sd+1,&readfds, NULL, NULL, NULL);

		if((active<0) && (errno!=EINTR))
		{
			error("Error select");
		}

		//if something happened on master socket -> incoming connection
		if(FD_ISSET(masterSocket,&readfds))
		{
			if((newSocket=accept(masterSocket, (struct sockaddr *)&address, (socklen_t*)&addrlen))<0 && (errno==EINTR) ) 
				continue;
			
			newConnection(maxClients, &address, &fdR, &connectedClients, &poped, &popedStart, clientSocket, &newSocket);
		}
		//else its some io operation on some other socket
		ioOperation(maxClients, &address, &fdR, &connectedClients, &poped, &popedStart, clientSocket, &sd, &readfds, bufRead, &valread);		
	}

	fclose(fdR);
	printf("end\n");
	return 0;
}

