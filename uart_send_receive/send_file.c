#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include "rdtsc.h"
#include <termios.h>

int main(int argc, char *argv[])
{
	char *buf;
	float speed, time;
	unsigned long long start, end;
	char *filename, *portname;
	long int times, block;
	char *endptr;
	int i;
	unsigned int sleep;
	struct termios options;
	int size;
	int tty_fd, file_fd;
	long int count=0;

	if (argc < 4) {
		printf("please input port name, block size, file name \n");
		return -1;
	}

	portname = argv[1];
	tty_fd = open(portname, O_RDWR | O_NOCTTY | O_NDELAY);
	fcntl(tty_fd, F_SETFL, 0);
	printf("open %s \n", portname);
	block = strtol(argv[2], &endptr, 10);

	printf("block size: %ld Bytes\n", block);
	filename = argv[3];

	file_fd = open(filename, O_RDONLY);
	printf("open file %s\n", filename);
	if (file_fd < 0) {
		printf("failed to open file %s \n", filename);
		return -1;
	}

	/*
	 *  * Get the current options for the port...
	 *   */
	tcgetattr(tty_fd, &options);
	cfsetispeed(&options, B115200);
	cfsetospeed(&options, B115200);
	/*
	 *  * Enable the receiver and set local mode...
	 *   */
	options.c_cflag |= (CLOCAL | CREAD);
	/*
	 *  * Set the new options for the port...
	 *   */
	options.c_cflag &= ~CSIZE; /* Mask the character size bits */
	options.c_cflag |= CS8;    /* Select 8 data bits */
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;
	options.c_cflag &= ~CRTSCTS;
	options.c_lflag = 0;
	options.c_oflag = 0; //&= ~OPOST;
	options.c_iflag = 0;
	options.c_cc[VTIME] = 5;
	options.c_cc[VMIN] = 0;

	tcsetattr(tty_fd, TCSANOW, &options);

	buf = malloc(block);

	start = rdtsc();
	while(1) {
		size = read(file_fd, buf, block);
		if (size) {
			write(tty_fd, buf, size);
			count += size;
		} else {
			break;
		}
	}
	end =rdtsc();

	time = (end-start)/1881600000.0; //s
	speed = count/time/1000; // KB/s

	printf("send = %ld bytes\n", count);
	printf("speed = %.2f KB/s\n", speed);
	close(tty_fd);
	close(file_fd);

	free(buf);
}

