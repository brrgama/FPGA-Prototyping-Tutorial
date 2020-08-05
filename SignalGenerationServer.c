//Server code for signal generation with FPGA

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>

#define PORT_SERVER 3030 //Connection port

//Configure and start signal generation
void start_DAC(int newsockfd, volatile void *addrs_ch1,volatile void *addrs_ch2);

int main(int argc, char **argv)
{
	volatile void *gpio_0; 
	volatile void *gpio_1; 
	int fd;
	
	//Generated signal frequency
	unsigned int signalFrequency = 0;
	
	//Variable to receive command message from client
	char buffer_word[5];	
	buffer_word[4] = '\0'; 
	
	char *name = "/dev/mem";
	if((fd = open(name, O_RDWR)) < 0) 
	{
		perror("open");
		return 1;
	}
	
	//Memory mapped	
	gpio_0 = mmap(NULL, sysconf(_SC_PAGESIZE),PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x41200000);
	gpio_1 = mmap(NULL, sysconf(_SC_PAGESIZE),PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x41260000);	
	
	//Server Configuration
	int sockfd, newsockfd, portno;
	socklen_t clilen;
	struct sockaddr_in serv_addr, cli_addr;

	//socket()
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	//To reuse an existent socket 
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)) < 0)
		error("setsockopt(SO_REUSEADDR) failed");

	if (sockfd < 0)	
	{		
		perror("ERROR opening socket");		
		return -1;	
	}

	printf("[socket]:OK!\n");	

	bzero((char *) &serv_addr, sizeof(serv_addr));		
	serv_addr.sin_family = AF_INET;		
	serv_addr.sin_addr.s_addr = INADDR_ANY; 
	serv_addr.sin_port = htons(PORT_SERVER);

	//bind()
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
	{		
		perror("ERROR on binding");
		return -1;
	}

	printf("[bind]:OK!\n");

	//listen()
	listen(sockfd,5);	 	
		
	//Start server loop
	while(1)
	{
		printf("Waiting for connection to port %d\n",ntohs(serv_addr.sin_port));
		clilen = sizeof(cli_addr); 

		//Accepts client connection
		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
			
		if (newsockfd < 0)
		{ 
			perror("ERROR on accept client");
			//Close connection
			close(newsockfd);
		}		
							
		bzero(buffer_word,4);			

		//Receives client command
		if(recv(newsockfd,buffer_word,4,0) < 0)
		{
			perror("ERROR on receiving command");								
			close(newsockfd);
		}	
		
		printf("Received command: %s\n",buffer_word);
		
		//Execute function related to received command						
		
		//Configure and start signal generation
		if (strncmp( buffer_word,"SDAC", 4) == 0)
		{			
			start_DAC(newsockfd,gpio_0,gpio_1);		
		}
	}
}

//Configure and start DAC
void start_DAC(int newsockfd, volatile void *addrs_ch1,volatile void *addrs_ch2)
{
	//Frequency value received from client - string
	char frequencyString [10];
	//Frequency value received from client - integer
	int frequencyInteger;
	//Frequency value - DDS mode
	unsigned long long int frequencyDDS;		
			
	//Receive frequency from client	 
	recv(newsockfd,&frequencyString,sizeof(frequencyString),0);
     	 
	//Data conversion 
	frequencyInteger = atoi(frequencyString); 
	printf("Defined frequency: %dHz\n",frequencyInteger);
	 	 
	//DDS configuration
	frequencyDDS = frequencyInteger*4294967296/125000000;
	uint32_t freq_uint32 = frequencyDDS;  			

	*((uint32_t *)(addrs_ch1)) = freq_uint32;
	*((uint32_t *)(addrs_ch2)) = freq_uint32;		
}
