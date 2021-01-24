/*
* Simple app to read/write into custom IP in PL via /dev/uoi0 interface
* To compile for arm: make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-
* insmod uio_pdrv_genirq.ko of_id=generic-uio
* ( Based on Kjans Tsotnep's app )
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/socket.h>     
#include <arpa/inet.h>   
#include <netinet/in.h>     
#include <string.h>         
#include <sys/types.h> 		
#include <sys/stat.h>	
#include "ZedboardOLED.h"

#define IP_ADDR "10.1.1.11"
#define PORT    7891

#define VOL_0_REG_0 *((unsigned *)(ptr_vol_0 + 0))
#define VOL_0_REG_1 *((unsigned *)(ptr_vol_0 + 4))

#define VOL_1_REG_0 *((unsigned *)(ptr_vol_1 + 0))
#define VOL_1_REG_1 *((unsigned *)(ptr_vol_1 + 4))

#define AXI_AUDIO_REG_0 *((unsigned *)(ptr_axi_audio + 0))


int udp_setup(char *broadcast_address, int broadcast_port);
int udp_recieve(int16_t *buffer, int buffer_size );
void *recv_function(void *arg);
void *send_audio(void *arg);
static char GetChoice(void);
static int GetIntFromConsole(void);
int GetUioFile(int uio);

//Network specific
struct sockaddr_in receiving_address;
int client_socket;
socklen_t addr_size;
char * fifo = "/tmp/myfifo";
int f_fifo;

//Threads
pthread_t audio_thread;
pthread_t eth_audio_thread;
pthread_t gpio_thread;

int uio_axi_audio;

//Addresses of uios
void *ptr_vol_0;
void *ptr_vol_1;
void *ptr_axi_audio;
void *ptr_oled;

char displayBuffer[16];

int main()
{
    mkfifo("/tmp/myfifo", 0644);
    char choice;
    unsigned pageSize = sysconf(_SC_PAGESIZE);

    choice = GetChoice();

    uio_axi_audio = GetUioFile(1);
    ptr_vol_0 = mmap(0, pageSize, PROT_READ | PROT_WRITE, MAP_SHARED, GetUioFile(0), 0);    //uio0
    ptr_vol_1 = mmap(0, pageSize, PROT_READ | PROT_WRITE, MAP_SHARED, uio_axi_audio, 0);    //uio1
    ptr_axi_audio = mmap(0, pageSize, PROT_READ | PROT_WRITE, MAP_SHARED, GetUioFile(3), 0);//uio3
    ptr_oled = mmap(0, pageSize, PROT_READ | PROT_WRITE, MAP_SHARED, GetUioFile(2), 0);     //uio2

    if(ptr_vol_0 == -1 || ptr_vol_1 == -1 || ptr_axi_audio == -1 || ptr_oled == -1) printf("Map failed");
    else printf("Maps set");

    pthread_create(&audio_thread, NULL, send_audio, NULL);
	pthread_create(&eth_audio_thread, NULL, recv_function, NULL);
    //pthread_create(&gpio_thread, NULL, );

    VOL_0_REG_0 = 255;
    VOL_0_REG_1 = 255;
    VOL_1_REG_0 = 255;
    VOL_1_REG_1 = 255;

    while(choice != 'x')
    {
        oledClear(ptr_oled);
        printf("What should I print on the OLED screen? \n");
        gets(displayBuffer);
        oledUpdate(ptr_oled);

        printf("Write volume line-in channel L: \n");
        VOL_0_REG_0 = GetIntFromConsole();
        printf("Write volume line-in channel R: \n");
        VOL_0_REG_1 = GetIntFromConsole();

        printf("Write volume eth-audio channel L: \n");
        VOL_1_REG_0 = GetIntFromConsole();
        printf("Write volume eth-aduio channel R: \n");
        VOL_1_REG_1 = GetIntFromConsole();


        int IRQEnable = 1; 

        // when you read from a file into this buffer, it will give you the 
        // total number of interrupts 
        printf("Interrupt count: = %d \n", IRQEnable);

        choice = GetChoice();
    }
    //unmap
    munmap(ptr_vol_0, pageSize);
    munmap(ptr_vol_1, pageSize);
    munmap(ptr_axi_audio, pageSize);
    munmap(ptr_oled, pageSize);

    //close
    fclose(stdout);
    
    return 0;
}


int udp_setup(char *broadcast_address, int broadcast_port) {

	/* Create UDP socket */
	client_socket = socket(PF_INET, SOCK_DGRAM, 0);

	int broadcast_enabled = 1;

	/* Enable broadcast */
	setsockopt(client_socket, SOL_SOCKET, SO_BROADCAST, (void *) &broadcast_enabled, sizeof(broadcast_enabled));
	
	/* Configure settings in address struct */
	receiving_address.sin_family = AF_INET;
	receiving_address.sin_port = htons(broadcast_port);
	receiving_address.sin_addr.s_addr = inet_addr(broadcast_address);
	memset(receiving_address.sin_zero, '\0', sizeof receiving_address.sin_zero);

	/* Connect to the server (broadcast) */
	bind(client_socket, (struct sockaddr *) &receiving_address, sizeof(receiving_address));

	/* Initialize size variable to be used later on */
	addr_size = sizeof receiving_address;

	return 0;
}

int udp_recieve(int16_t *buffer, int buffer_size ) {
	if ((recvfrom(client_socket, buffer, buffer_size, 0, (struct sockaddr *)&receiving_address, &addr_size)) != -1 )
		return 0;
	else
		return 1;
}

void *recv_function(void *arg)
{
	int count= 512;
	int16_t samples[count];
	
	printf("AUDIORX thread started\n");
	
	printf("Opening the FIFO for writing\n");
	f_fifo = open(fifo, O_WRONLY);

	// setup the UDP receive
	udp_setup(IP_ADDR, PORT);
	
	while (1){
		udp_recieve(samples,sizeof(samples));
		write(f_fifo, samples, sizeof(samples));		
	}
	
	printf("FILLER thread exited\n");	
}

void *send_audio(void *arg)
{
int count= 512;
	int16_t samples[count];
	int16_t ssample;
	int32_t sample;
    int IRQEnable = 1, IRQCount; 

    printf("Opening the FIFO for reading\n");
	int fifo_read = open(fifo, O_RDONLY);
	
	while (1){
		
		// read the chunk of samples from the fifo
		read(fifo_read, &samples, sizeof(samples));
		for (int i= 0; i<count; i++){
			// enable the interrupt
			write(uio_axi_audio, &IRQEnable, sizeof(IRQEnable));
			sample = samples[i];			
			
			// wait for the interrupt
			read(uio_axi_audio, &IRQCount, sizeof(IRQCount));
			
			// write the sample to the device
			*((unsigned *)(uio_axi_audio + 0)) = sample;			
		}
	
	}
}

int GetUioFile(int uio)
{
    int fd;
    switch(uio)
    {
        case 0:
            fd = open("/dev/uio0", O_RDWR);
            break;
        case 1:
            fd = open("/dev/uio1", O_RDWR);
            break;
        case 2:
            fd = open("/dev/uio2", O_RDWR);
            break;
        case 3:
            fd = open("/dev/uio3", O_RDWR);
            break;
        case 4:
            fd = open("/dev/uio4", O_RDWR);
            break;
    }
    if(fd > 0) return fd;
    else printf("Fail opening uio file\n");
    return 0;
}

static char GetChoice(void)
{
    char c[5];
    printf("Write anything to modify parameters, write 'x' to exit\n");
    printf("After this write value for one channel amplification k = 1.0 @ 255 \n");
    gets(c);   
    return c[0];
}

static int GetIntFromConsole(void)
{
    char buffer[3] = {0,0,0};    

    gets(buffer);
    return atoi(buffer);
}