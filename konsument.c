#include "konsument.h"

char *getMD5sum(const char *data, int len);

void error(const char *msg)
{
	perror(msg);
	exit(1);
}

void checkNum(const char* arg, int len)
{
	int j=0;
	while(j<len){
		if(isdigit(arg[j])==0)
			error("Invalid usage of parameter");
		j++;}
}

double checkPara(double para, const char* arg)
{
	char *endptr;
	errno=0;	
	para=strtod(arg, &endptr);
	if(errno !=0 || *endptr!='\0')
		error("Bad usage of parameter!");
	return para;
}

void rflag(int numCom, char* buffer, char buf[][100], char* init, int *sockfd, struct timespec ts, struct timeval *start, struct timeval stop, struct timeval st, struct timeval end, long* secs, long* secs2, long* micro, long* micro2)
{
	int i=0;

	for(int j=0; j<numCom; j++)
	{
		gettimeofday(&start[i],NULL);
		if(send(*sockfd,init,strlen(init),0)<0)
			error("sending init error");	
		nanosleep(&ts,NULL);
		i++;
	}
	
	for(i=0; i<numCom;i++) //receiving data
	{
			//bzero(buffer,114688);

			gettimeofday(&st,NULL);			
			gettimeofday(&stop,NULL);
			
			ssize_t rec=0;
			do{
				int res=recv(*sockfd,&buffer[rec],114688-rec,0);
				if(res<0)
					error("receiving data error");
				else
					rec+=res;
			}while(rec<114688);


			gettimeofday(&end,NULL);

			strcpy(buf[i], getMD5sum(buffer, strlen(buffer)));
			
			secs[i]=(end.tv_sec-st.tv_sec);
			micro[i]=((secs[i]*10000000)+end.tv_usec)-(st.tv_usec);
		
			secs2[i]=(stop.tv_sec-start[i].tv_sec); //pomiedzy komunkatem a blokiem
			micro2[i]=((secs2[i]*10000000)+stop.tv_usec)-(start[i].tv_usec);
	}
}

void sflag(int numCom, char* buffer, char buf[][100], char* init, int *sockfd, struct timespec ts, struct timeval *start, struct timeval stop, struct timeval st, struct timeval end, long* secs, long* secs2, long* micro, long* micro2)
{
	for(int i=0; i<numCom;i++)
	{	
		gettimeofday(&start[i],NULL);
		if(send(*sockfd,init,strlen(init),0)<0)
			error("sending init error");
		//receiving data

		gettimeofday(&st,NULL);
		gettimeofday(&stop,NULL);

		ssize_t rec=0;
		do{
			int res=recv(*sockfd,&buffer[rec],114688-rec,0);
			if(res<0)
				error("receiving data error");
			else
				rec+=res;
		}while(rec<114688);

		gettimeofday(&end,NULL);

		strcpy(buf[i], getMD5sum(buffer, strlen(buffer)));

		secs[i]=(end.tv_sec-st.tv_sec);
		micro[i]=((secs[i]*10000000)+end.tv_usec)-(st.tv_usec);

		secs2[i]=(stop.tv_sec-start[i].tv_sec); //pomiedzy komunkatem a blokiem
		micro2[i]=((secs2[i]*10000000)+stop.tv_usec)-(start[i].tv_usec);
	
		nanosleep(&ts,NULL);
	}
}


int main(int argc, char* argv[])
{
	//-----------params
	int numCom=-1; //ile komunikatow
	int rFlag=0;
	int sFlag=0;
	double delay; //odstępy czasu między kom
	double l;
	double r;//if two numbers with : passed
	//char *c;
	int len;
	//----------------------
	int k;
	srand(time(NULL)); 

	while((k=getopt(argc,argv,"#:r:s:")) != -1)
		switch(k)
		{
			case '#':
				len=strlen(optarg);
				char *c;
				if((c=strchr(optarg, ':'))==NULL)
				{
					numCom=strtol(optarg,0,0);	
					checkNum(optarg, len);
				}
				
				else{
					srand(time(NULL));
					int in=(int)(c-optarg);
					char* l1=(char*)malloc((len+1)*sizeof(char));
					strncpy(l1, optarg, in);
					checkNum(l1, strlen(l1));	
		
					char* l2=(char*)malloc((len+1)*sizeof(char));
					strncpy(l2, optarg+1+in, len-in);
					checkNum(l2, strlen(l2));
					
					numCom=(rand()%((strtol(l2,0,0)-strtol(l1,0,0)+1)))+strtol(l1,0,0);
				}
				break;

			case 'r':
				rFlag=1;
				if(sFlag)
					error("You can pass R or S, not both!");
				len=strlen(optarg);
				if((c=strchr(optarg, ':'))==NULL)
				{	
					delay=checkPara(delay,optarg);
				}
				else{
					int in=(int)(c-optarg);
					char* l1=(char*)malloc((len+1)*sizeof(char));
					strncpy(l1, optarg, in);
			
					char* l2=(char*)malloc((len+1)*sizeof(char));
					strncpy(l2, optarg+1+in, len-in);
					
					l=checkPara(l,l1);
					r=checkPara(r,l2);				

					delay=((double)rand()/RAND_MAX*(r-l+1))+l;
				}
				break;

			case 's':
				sFlag=1;
				if(rFlag)
					error("You can pass R or S, not both!");
				len=strlen(optarg);
				if((c=strchr(optarg, ':'))==NULL)
				{	
					delay=checkPara(delay,optarg);
				}
				else{
					int in=(int)(c-optarg);
					char* l1=(char*)malloc((len+1)*sizeof(char));
					strncpy(l1, optarg, in);
			
					char* l2=(char*)malloc((len+1)*sizeof(char));
					strncpy(l2, optarg+1+in, len-in);
					
					l=checkPara(l,l1);
					r=checkPara(r,l2);	
					
					delay=((double)rand()/RAND_MAX*(r-l+1))+l;
				}
				break;
		}

	int rs=0;
	if(rFlag || sFlag)
		rs=1;

	if(numCom==-1 || !rs || !(argv[optind]))
		error("Bad usage of parameters, pass mandatory parameters!");

	char* location=(char*)malloc((strlen(argv[optind])+1)*sizeof(char));
	len=strlen(argv[optind]);
	char *e=strchr(argv[optind], ':');
	int index=(int)(e-argv[optind]);
	char portC[5];
	int portno;
	if(e==NULL)
	{
		location=(char*)malloc((15)*sizeof(char));
		strcpy(location,"localhost");
		strcpy(portC, argv[optind]);
		checkNum(portC, strlen(portC));
		portno=strtol(portC, 0,0);
	}
	else
	{
		strncpy(location,argv[optind],index);
		int index2=(int)(len-index);
		strncpy(portC, argv[optind]+index+1,index2);
		checkNum(portC, strlen(portC));
		portno=strtol(portC,0,0);
	}
	
	printf("numCom: %d, delay: %f, location: %s, port: %d\n", numCom, delay, location, portno);	
	//---------------------------
	int sockfd;
	struct sockaddr_in servAddr;
	struct hostent *server;

	char *buffer=malloc(sizeof(char)*114689);
	char init[4]="abc"; //initial msg
	sockfd=socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd<0)
		error("error opening socket");
	
	server=gethostbyname(location);
	if(server==NULL)
	{
		fprintf(stderr, "error, no such host\n");
		exit(0);
	}

	bzero((char*) &servAddr, sizeof(servAddr));
	servAddr.sin_family=AF_INET;
	bcopy((char*)server->h_addr, (char*)&servAddr.sin_addr.s_addr, server->h_length);
	servAddr.sin_port=htons(portno);
	if(connect(sockfd,(struct sockaddr*)&servAddr, sizeof(servAddr)) < 0)
		error("error connecting");
	printf("Connected\n");
	bzero(buffer,114688);

	static long secToNano=1000000000;

	struct timespec ts;
	ts.tv_sec=(time_t)delay;
	ts.tv_nsec=(delay-(time_t)delay)*secToNano;

	struct timeval start[numCom];
	struct timeval stop;
	struct timeval st, end;
	char buf[numCom][100];
	long secs[numCom];
	long secs2[numCom];
	long micro[numCom];
	long micro2[numCom];

	if(rFlag)
		rflag(numCom, buffer, buf, init, &sockfd, ts, (struct timeval*)start, stop, st, end, secs, secs2, micro, micro2);

	else //------------------sflag
		sflag(numCom, buffer, buf, init, &sockfd, ts, (struct timeval*)start, stop, st, end, secs, secs2, micro, micro2);
	
	close(sockfd);

	struct timespec monotime, realtime;
	struct sysinfo info;
	sysinfo(&info);
	time_t boottime=time(NULL)-info.uptime;
	clock_gettime(CLOCK_MONOTONIC, &monotime);
	time_t curtime=boottime+monotime.tv_sec;
	clock_gettime(CLOCK_REALTIME, &realtime);
	printf("Mono time: %s, WallTime: %s\n",ctime(&curtime), ctime(&realtime.tv_sec));

	for(int i=0; i<numCom;i++)
	{
		printf("odczytanie bajtu, a odczytanie calosci: %ld microSec\n", micro[i]);
		printf("wyslanie kom, a odczytanie bloku: %ld microSec\n", micro2[i]);
		printf("md5sum: %s\n", buf[i]);
		printf("\n");
	}
	
	printf("PID kosumenta: %d, adres (port, z ktorego sie laczono): %d \n",getpid(), servAddr.sin_port=htons(portno));
	return 0;

}

char *getMD5sum(const char *data, int len)
{
	MD5_CTX c;
	unsigned char digest[16];
	char *out=(char*)malloc(33);

	MD5_Init(&c);
	while(len>0){
		if(len>512){
			MD5_Update(&c, data, 512);
		}
		else {
			MD5_Update(&c, data, len);
		}
		len-=512;
		data+=512;
	}

	MD5_Final(digest, &c);
	int i;
	for(i=0;i<16;++i)
	{
		snprintf(&(out[i*2]), 16*2, "%02x", (unsigned int)digest[i]);
	}

	return out;
}
