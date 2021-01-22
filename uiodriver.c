/*
* Simple app to read/write into custom IP in PL via /dev/uoi0 interface
* To compile for arm: make ARCH=arm CROSS_COMPILE=arm-linux-gnueabihf-
* ( Based on Kjans Tsotnep's app )
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdint.h>

#define SLV_REG_0 *((unsigned *)(ptr + 0))
#define SLV_REG_1 *((unsigned *)(ptr + 4))
// #define SLV_REG_2 *((unsigned *)(ptr + 8))
// #define SLV_REG_3 *((unsigned *)(ptr + 12))

static char GetChoice(void);
static int GetIntFromConsole(void);

int main()
{
    char choice;
    void *ptr; 
    unsigned pageSize = sysconf(_SC_PAGESIZE);

    choice = GetChoice();

    int fd = open ("/dev/uio1", O_RDWR);
    if (fd < 1) 
    { 
        printf("can't open /dev/uiox file");
        return -1; 
    }
    ptr = mmap(0, pageSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if(ptr == MAP_FAILED)
    {
        printf("Map failed");
        return 0;
    }

    freopen ("/dev/kmsg","w",stdout);

    while(choice != 'x')
    {
        SLV_REG_0 = GetIntFromConsole();
        SLV_REG_1 = GetIntFromConsole();

        int IRQEnable = 1; 

        // when you read from a file into this buffer, it will give you the 
        // total number of interrupts 
        printf("Interrupt count: = %d \n", IRQEnable);

        choice = GetChoice();
    }
    //unmap
    munmap(ptr, pageSize);

    //close
    fclose(stdout);
    
    return 0;
}

static char GetChoice(void)
{
    char c[5];
    printf("Write 'm' to modify parameters, write 'x' to exit\n");
    gets(c);   
    return c[0];
}

static int GetIntFromConsole(void)
{
    char buffer[3] = {0,0,0};    
    printf("Choose value for one channel (0-255)\n");
    gets(buffer);
    return atoi(buffer);
}