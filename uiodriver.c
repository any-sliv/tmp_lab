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
#define SLV_REG_2 *((unsigned *)(ptr + 8))
#define SLV_REG_3 *((unsigned *)(ptr + 12))

static char GetChoice(void);
static int GetIntFromConsole(void);

int main()
{
    char choice;
    void *ptr; 
    unsigned pageSize = sysconf(_SC_PAGESIZE);

    choice = GetChoice();

    while(choice != 'x')
    {
        // open the interrupt handling file
        int fd = open ("/dev/uio1", O_RDWR);
        if (fd < 1) { return -1; }

        // Redirect stdout/printf into the /dev/kmsg file 
        // (it will be possible to print using printk)
        freopen ("/dev/kmsg","w",stdout);

        /**********************************************************************
         * TASK 1: Write a SINGLE LINE COMMAND to map the physical address    *
         *         into this program's virtual address space. Point to the    *
         *         starting address of the new mapping with the 'ptr' pointer *
         *         (already declared above)                                   *
         **********************************************************************
        * HINT 0: You can look at how you did this in the /dev/mem task.     *
        *         However, when mapping in UIO, there are some differences   *
        *         from doing it in /dev/mem, check the "Mapping usage in     *
        *         UIO" section in Lab 3 additional materials for details.    *
        **********************************************************************/


        ptr = mmap(NULL, 4, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

        SLV_REG_0 = GetIntFromConsole();
        SLV_REG_1 = GetIntFromConsole();

        int IRQEnable = 1; 
        /**********************************************************************
         * TASK 2: Write a SINGLE LINE COMMAND to enable the interrupt        *
         **********************************************************************
        * HINT 0: You need to write the value of the IRQEnable variable      *
        *         (defined above) into the file designed for interrupt       *
        *         handling. You can find more information in the "Userspace  *
        *         I/O (UIO)" section of the LAB 3 additional material        *
        **********************************************************************/
        

        write(fd, &IRQEnable, 1);
        printf("Written '1' into /dev/uio");


        /**********************************************************************
         * TASK 3: Write a SINGLE LINE COMMAND that blocks the program's      *
         *         execution until the multiplier sends its interrupt signal  *                         *
         **********************************************************************
        * HINT 0: You need to read a specific file. You can find more        *
        *         information in the "Userspace I/O (UIO)" section of the    *
        *         LAB 3 additional material                                  *
        * HINT 2: Use the IRQEnable variable for storing the output of the   *
        *         function                                                   *
        **********************************************************************/


        IRQEnable = open ("/dev/uio1", O_RDWR);

        // when you read from a file into this buffer, it will give you the 
        // total number of interrupts 
        printf("Interrupt count: = %d \n", IRQEnable);

        //Read and print the result of the IP calculation
        unsigned ans = SLV_REG_3;
        printf("READ: from offset of %d, a value of %d\n", 12, ans);

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
    char c;
    printf("Write 'm' to modify parameters, write 'x' to exit\n");
    return gets(c);
}

static int GetIntFromConsole(void)
{
    char buffer[3] = {0,0,0};    
    printf("Choose value for one channel (0-255)\n");
    gets(buffer);
    return atoi(buffer);
}