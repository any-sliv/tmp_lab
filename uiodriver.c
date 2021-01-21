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

int main(int argc, char *argv[])
{
    if (argc < 3) {
        printf("\n");
        printf("Usage: ./uiodriver\n");
        printf("   or: ./uiodriver FACTOR_1 FACTOR_2\n");
        printf("Description:\n");
        printf("    This program multiplies two numbers using the multiplier\n");
        printf("    IP synthesized in the FPGA (It requires the uio_pdrv_genirq\n");
        printf("    kernel driver). If executed without arguments, these usage\n");
        printf("    instructions will be displayed.\n");
        printf("Valid arguments are:\n");
        printf("    FACTOR_1    This is one of the factors of the multiplication\n");
        printf("    FACTOR_2    This is the other factors of the multiplication\n");
        printf("Execution examples:\n");
        printf("    ./uiodriver\n");
        printf("    ./uiodriver 6 4\n");
        printf("\n");
    }
    else { 
        // read factors from the user input
        unsigned multiplierInput1 = atoi(argv[1]);
        unsigned multiplierInput2 = atoi(argv[2]);
        printf("opening fd");

        // open the interrupt handling file
        int fd = open ("/dev/uio0", O_RDWR);
        if (fd < 1) { perror(argv[0]); return -1; }
 
        printf("1");

        // Redirect stdout/printf into the /dev/kmsg file 
        // (it will be possible to print using printk)
        freopen ("/dev/kmsg","w",stdout);
 
        printf("2");


        // get architecture specific page size
        unsigned pageSize = sysconf(_SC_PAGESIZE);

        printf("3");

        // pointer to the virtual address space, where physical memory will 
        // be mapped
        void *ptr; 
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

        // WRITE YOUR SINGLE LINE COMMAND IN HERE //////////////////////////////
        
        printf("before mapping");
        ptr = mmap(NULL, 4, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        printf("Mapped");

        ////////////////////////////////////////////////////////////////////////
 
        // feed the read factors into their corresponding registers
        SLV_REG_0 = multiplierInput1;
        SLV_REG_1 = multiplierInput2;

        int IRQEnable = 1; 
        /**********************************************************************
         * TASK 2: Write a SINGLE LINE COMMAND to enable the interrupt        *
         **********************************************************************
         * HINT 0: You need to write the value of the IRQEnable variable      *
         *         (defined above) into the file designed for interrupt       *
         *         handling. You can find more information in the "Userspace  *
         *         I/O (UIO)" section of the LAB 3 additional material        *
         **********************************************************************/
                
        // WRITE YOUR SINGLE LINE COMMAND IN HERE //////////////////////////////
        
        write(fd, &IRQEnable, 1);
        printf("Written '1' into /dev/uio");

        ////////////////////////////////////////////////////////////////////////

        // signal the multiplier to start the calculation
        SLV_REG_2 = 1;        
        SLV_REG_2 = 0;
 
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

        // WRITE YOUR SINGLE LINE COMMAND IN HERE //////////////////////////////
        

        IRQEnable = open ("/dev/uio0", O_RDWR);

        ////////////////////////////////////////////////////////////////////////
 
        // if you direct stdio into the correct file, this printf will be
        // written into printk, and will get time-stamp on message
        printf("DEBUG_USERSPACE : IRQ\n");
 
        // when you read from a file into this buffer, it will give you the 
        // total number of interrupts 
        printf("Interrupt count: = %d \n", IRQEnable);
 
        //Read and print the result of the IP calculation
 		unsigned ans = SLV_REG_3;
		printf("READ: from offset of %d, a value of %d\n", 12, ans);
 
        //unmap
        munmap(ptr, pageSize);
 
        //close
        fclose(stdout);
    }
    return 0;
}