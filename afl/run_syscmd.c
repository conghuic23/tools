#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>

static struct option long_options[] = {
	{"file", required_argument, 0, 'f'},
	{"cmd", required_argument, 0, 'c'},
	{"help", no_argument, 0, 'h'},
};


static char optstr[] = "f:c:h";
int main(int argc, char *argv[])
{
	int option_idx = 0;
	char *filename=NULL;
	char *syscmd=NULL;
	char *buf;
	char c;
	int count, fd;
	struct stat st;

        while ((c = getopt_long(argc, argv, optstr, long_options,
                        &option_idx)) != -1) {
                switch (c) {
                case 'f':
			filename = strdup(optarg);
                        break;
		case 'c':
			syscmd = strdup(optarg);
			break;
                case 'h':
			printf("run with '-c <system cmd> -f <input file>'\n");
                        return 0;
                default:
			printf("not support\n");
                        return 0;
                }
        }

	if (!filename || !syscmd) {
		printf("please set -f and -c\n");
		goto err;
	}

	printf("filename=%s\n", filename);
	fd = open(filename, O_RDONLY);
	if (fd < 0) {
		printf("failed to open file %s\n", filename);
		return 0;
	}
	fstat(fd,&st);

	if (st.st_size) {
		buf = malloc(st.st_size);
		if (!buf)
			goto err;

		if (read(fd, buf, st.st_size) != st.st_size) {
			printf("read error !!\n");
			goto err;
		}
	} else
		goto err;

	if (execl(syscmd, buf) < 0)
		printf("run failed\n");

err:
	if (buf)
		free(buf);
	if (filename)
		free(filename);
	if (syscmd)
		free(syscmd);
	return 0;
}
