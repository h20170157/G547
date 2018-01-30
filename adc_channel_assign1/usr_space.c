#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<fcntl.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<sys/ioctl.h>

char dev_file_name[]="adc";

int main(void)
{
	int channel;
	char channel_num=' ';
	int num_bytes;
	char voltage[20];
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

	// writing to char driver
	printf("Enter channel number:");
	scanf("%d",&channel);
	if(channel>7||channel<0)
	{
		printf("Enter channel number between 0-7 !!\n");
		return -2;
	}
	channel_num = '0' +(char)channel;
	printf("***************************************\n");
	printf("*      SWITCHING TO KERNEL SPACE      *\n");
	printf("***************************************\n");
	write(fd, channel_num, sizeof(channel_num));

	// reading from char driver
	printf("***************************************\n");
	printf("*    SWITCHING BACK TO USER SPACE     *\n");
	printf("***************************************\n");
	
	read(fd, &voltage, sizeof(voltage));
	printf("Voltage value:%s\n",voltage);

	// close the file descriptor
	close(fd);
	return 0;
}
