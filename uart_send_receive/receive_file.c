#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rdtsc.h"
#include <sys/time.h>
#include <termios.h>
#include <sys/ioctl.h>

int tty_fd;

int is_timeout() {
	fd_set rfds;
	struct timeval tv;
	int retval;

	/* Watch ttyfd to see when it has input. */
	FD_ZERO(&rfds);
	FD_SET(tty_fd, &rfds);
	/* Wait up to five seconds. */
	tv.tv_sec = 1;
	tv.tv_usec = 0;
	retval = select(FD_SETSIZE, &rfds, NULL, NULL, &tv);

	return !retval;
}

int main(int argc, char *argv[])
{
	long int count=0, bytes;
	char buf[1000];
	char *filename, *receivefile;
	int i;
	int num;
	int save_fd;
	struct termios options; 
	int start;
	int tmp;

	if (argc < 3) {
		printf("please input port name and receive file name \n");
		return -1;
	}

	filename = argv[1];
	tty_fd = open(filename, O_RDWR | O_NOCTTY | O_NDELAY);
	printf("open %s \n", filename);
	receivefile = argv[2];
	save_fd = open(receivefile, O_RDWR | O_CREAT);
	printf("create file %s \n", receivefile);

	tcgetattr(tty_fd, &options);

	/*
	 *  * Set the baud rates to 19200...
	 *   */

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
	//options.c_lflag &= ~(ICANON | ECHO | ISIG);
	options.c_lflag = 0;
	options.c_oflag = 0;
	options.c_iflag = 0;
	options.c_cc[VTIME] = 5;
	options.c_cc[VMIN] = 0;
	//options.c_oflag &= ~OPOST;
	//options.c_iflag |= (IXON | IXOFF | IXANY);

	tcsetattr(tty_fd, TCSANOW, &options);

	start = 0;
	while(1) {
		if (start && is_timeout())
			break;

		ioctl(tty_fd, FIONREAD, &bytes);
		if (bytes) {
			start = 1;
			while(bytes) {
				if (bytes > 1000) {
					tmp = 1000;
					bytes -= 1000;
				} else {
					tmp = bytes;
					bytes = 0;
				}

				num = read(tty_fd, buf, tmp);
				write(save_fd, buf, num);
				count += num;
			}
		}
	}

	printf("receive %ld bytes\n",count);
	close(tty_fd);
	close(save_fd);

	return 0;
}

