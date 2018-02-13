#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
char dev_file_name[]="timer0";

int main(void)
{
	int delayMS=0;
	char delay_ms[10];
	char pit_timer_val[20];
	int fd=0;
	char full_device_file_name[50]="/dev/";

	// creating full device path
	strcat(full_device_file_name,dev_file_name);

	// change file permissions
	if(chmod(full_device_file_name,0777)<0)
        {
                printf("Unable to change file permissions. Set it manually !!\n");
		return -1;
        }
        else
        {
                printf("Permissions changed successfully !!\n");
        }

	// opening file descriptor
	fd=open(full_device_file_name, O_RDWR, S_IREAD|S_IWRITE);
	if(fd<0)
	{
		printf("Cannot open device file. Aborting !!\n");
		exit(-1);
	}

	printf("Enter time (in ms):");
	scanf("%d",&delayMS);
	sprintf(delay_ms,"%d",delayMS);

	printf("***************************************\n");
	printf("*      SWITCHING TO KERNEL SPACE      *\n");
	printf("***************************************\n");

	// pass delay (in ms) from user space to kernel space
        write(fd, delay_ms, sizeof(delay_ms));

	// read PIT timer value from kernel space
	read(fd, &pit_timer_val, sizeof(pit_timer_val));

	// reading from char driver
	printf("***************************************\n");
	printf("*    SWITCHING BACK TO USER SPACE     *\n");
	printf("***************************************\n");

	printf("PIT timer value:%s\n",pit_timer_val);

	// close the file descriptor
	close(fd);
	return 0;
}
