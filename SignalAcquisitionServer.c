//Server code for signal acquisition with FPGA

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

//Reset FIFOs
void rst_FIFOs(volatile void *addrs);
//Configure FIFOs
void cfg_FIFOs(volatile void *addrs,int newsockfd);
//Save data to text file
void saveData(volatile void *addrs_1, volatile void *addrs_2, FILE *data_1, FILE *data_2,char DataFile_ch1[],char DataFile_ch2[], int data_ch1[], int data_ch2[]);


int main(int argc, char **argv)
{
	volatile void *gpio_0; 
	volatile void *gpio_1; 		
	int fd;
	
	//Channel 1 data
	int data_ch1[64];
	//Channel 2 data
	int data_ch2[64];	
	
	//text file to save data from channel 1
	FILE *data_1;	
	char DataFile_ch1[] = "/tmp/dataAD_ch1.txt";
	
	//text file to save data from channel 2
	FILE *data_2;	
	char DataFile_ch2[] = "/tmp/dataAD_ch2.txt";

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
	gpio_0 = mmap(NULL, sysconf(_SC_PAGESIZE),PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x41220000);
	gpio_1 = mmap(NULL, sysconf(_SC_PAGESIZE),PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x41210000);
	
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
		
		//Configure and start data acquisition
		if (strncmp(buffer_word, "SADC", 4) == 0)
		{								
			//Reset FIFOs
			rst_FIFOs(gpio_2);
			
			//Configure and start data acquisition
			cfg_FIFOs(gpio_2, newsockfd);			
		}
		
		//Sava data in txt file
		else if (strncmp(buffer_word, "SAVE", 4) == 0)		
		{
			saveData(gpio_3,gpio_2,data_1,data_2,DataFile_ch1,DataFile_ch2,data_ch1,data_ch2);			
		}			
	}
}

//Reset FIFOs
void rst_FIFOs(volatile void *addrs)
{
	int gpio_data;
	  
	for (int i = 0 ; i < 5 ; i++)
	{		
		(*((uint32_t *)(addrs))) = ((uint32_t)4); 
		gpio_data = (*((uint32_t *)(addrs))); 		
		usleep(1000);
				
		(*((uint32_t *)(addrs))) = ((uint32_t)(8 + 4));
		gpio_data = (*((uint32_t *)(addrs))); 		
		usleep(1000);	
	}		
}

//Configure FIFOs
void cfg_FIFOs(volatile void *addrs,int newsockfd)
{	
	int gpio_data;
	int err;	
	
	//Frequency code received from client - string
	char frequencyString [10];
	//Frequency code received from client - integer
	int frequencyInteger;
	 
	//Receive sampling frequency code	 
	 err = recv(newsockfd,&frequencyString,sizeof(frequencyString),0);
	 
	 //Data conversion 
	frequencyInteger = atoi(frequencyString); 
				
	//Basics FIFOs configuration
	frequencyInteger = (frequencyInteger << 4);			
	(*((uint32_t *)(addrs))) =  0 ;
	usleep(1000); 
	(*((uint32_t *)(addrs))) &=  ~4 ; 
	usleep(1000); 
	(*((uint32_t *)(addrs))) |=  4 ; 
	usleep(1000); 
	
	for (int j = 0;j++;j<5) 
	{
		(*((uint32_t *)(addrs))) &=  ~8 ; 
		usleep(1000); 
		(*((uint32_t *)(addrs))) |=  8 ; 
		usleep(1000); 
	}
	(*((uint32_t *)(addrs))) &=  ~4 ; 
		usleep(1000); 
	
	//Acquisition rate configuration
	(*((uint32_t *)(addrs))) |= frequencyInteger; 	
	usleep(1000);	
	
	//Enable write mode 
	(*((uint32_t *)(addrs))) |= (uint32_t) 1;
	usleep(1000);
	gpio_data = (*((uint32_t *)(addrs)));	
}

//Save data to text file
void saveData(volatile void *addrs_1, volatile void *addrs_2, FILE *data_1, FILE *data_2,char DataFile_ch1[],char DataFile_ch2[], int data_ch1[], int data_ch2[])
{
	while(1)
	{		
		volatile void *gpio_1 = addrs_1;
		volatile void *gpio_2 = addrs_2;
		int gpio_data;
				
		//Monitoring FIFO's state
		int full_ch1;		
		full_ch1 = (*((uint32_t *)(gpio_2 + 8)) & (1));		
		int full_ch2; 		
		full_ch2 = (*((uint32_t *)(gpio_2 + 8)) & (4));				
		
		if(full_ch1 && full_ch2) 
		{				
			(*((uint32_t *)(gpio_2))) &= (0); 
			gpio_data = (*((uint32_t *)(gpio_2)));		
			(*((uint32_t *)(gpio_2))) = ((uint32_t)2); 
			gpio_data = (*((uint32_t *)(gpio_2)));
					
			data_1 = fopen( DataFile_ch1, "w");	
			data_2 = fopen( DataFile_ch2, "w");					
			
				for (int i = 0; i < 64 ; i++)
				{
					//Write to text file data from channel 1
					data_ch1[i] = (*((uint32_t *)(gpio_1)));										
					fprintf(data_1,"%d\n",data_ch1[i]);
					//Write to text file data from channel 2
					data_ch2[i] = (*((uint32_t *)(gpio_1 + 8))); 
					fprintf(data_2,"%d\n",data_ch2[i]);
													
					(*((uint32_t *)(gpio_2))) = ((uint32_t)2);
					gpio_data = (*((uint32_t *)(gpio_2)));					
					usleep(1000);					
					(*((uint32_t *)(gpio_2))) = ((uint32_t)10);
					gpio_data = (*((uint32_t *)(gpio_2)));					
					usleep(1000);
				}				
			fclose(data_1);
			fclose(data_2);			
			break;							
		}						
	}	
}

