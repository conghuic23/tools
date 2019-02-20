#include <stdio.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

static struct option long_options[] = {
	{"file", required_argument, 0, 'f'},
	{"cmd", required_argument, 0, 'c'},
	{"paramfile", required_argument, 0, 'p'},
	{"cmdfuzz", no_argument, 0, 'e'},
	{"help", no_argument, 0, 'h'},
	{0, 0, 0, 0}
};


static char ** change_buf_to_argv(char *buf, int *m_argc)
{
  char* ptr = buf;
 static char* ret[1024];
  int   rc  = 0;
  int   i   =0;

  while (*ptr) {

    ret[rc] = ptr;
    if (ret[rc][0] == 0x02 && !ret[rc][1]) ret[rc]++;
    while(ret[rc][0] == 0x20) ret[rc]++;
    rc++;

    while (*ptr && *ptr!=0x0A) ptr++;
    *ptr='\0';
    ptr++;

  }

  *m_argc = rc;
  ret[rc] = NULL;
  return ret; 
}

char *fuzz_cmdline(char *filename)
{
	int fd;
	struct stat st;
	char *buf;

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
	close(fd);
	return buf;
err:
	close(fd);
	if (buf)
		free(buf);
	return NULL;
}

char *fuzz_virtio_blk_file(char *paramfile, char *filename)
{
	int fd_p;
	struct stat st_p;
	char *buf_p;
	char add_param[] = "-s\n20,virtio-blk,";
	int alloc_len=0;
	
	if (paramfile) {
		fd_p = open(paramfile, O_RDONLY);
		if (fd_p < 0) {
			printf("failed to open parameter file %s\n", paramfile);
			return NULL;
		}
	}
	fstat(fd_p,&st_p);

	if (st_p.st_size) {
		alloc_len = st_p.st_size + strlen(add_param) + strlen(filename);
		buf_p = malloc(alloc_len);
		if (!buf_p)
			goto err;
		if (read(fd_p, buf_p, st_p.st_size) != st_p.st_size) {
			printf("read error !!\n");
			goto err;
		}
		printf("cmd: %s", buf_p);
	} else
		goto err;

	sprintf(buf_p + st_p.st_size ,"%s%s",add_param,filename);
	close(fd_p);
	return buf_p;

err:
	close(fd_p);
	if (buf_p)
		free(buf_p);
	return NULL;
}


static char optstr[] = "f:c:p:eh";
int main(int argc, char *argv[])
{
	int option_idx = 0;
	char *filename=NULL, *paramfile=NULL;
	char *syscmd=NULL;
	char *buf;
	char c;
	int cmdline_fuzz = 0;
	char **m_argv;
	int m_argc=0;

        while ((c = getopt_long(argc, argv, optstr, long_options,
                        &option_idx)) != -1) {
                switch (c) {
                case 'f':
			filename = strdup(optarg);
                        break;
		case 'c':
			syscmd = strdup(optarg);
			break;
		case 'p':
			paramfile = strdup(optarg);
			break;
		case 'e':
			cmdline_fuzz = 1;
			break;
                case 'h':
			printf("run with\n"
					"'-c <system cmd>\n"
					"-p <parameters file>\n"
					"-f <input fuzzing file> '\n");
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

	if (cmdline_fuzz)
		buf = fuzz_cmdline(filename);
	else
		buf = fuzz_virtio_blk_file(paramfile, filename);

	if (!buf)
		goto err;

	m_argv = change_buf_to_argv(buf, &m_argc);
	for (int i=0;i<m_argc;i++)
		printf("m_argv[%d]=%s\n", i, m_argv[i]);
	if (m_argv && m_argc) {
		if (execvp(syscmd, m_argv) < 0)
			printf("run failed %d\n",errno);
		else
			printf("run ok\n");
	}

err:
	if (buf)
		free(buf);
	if (filename)
		free(filename);
	if (paramfile)
		free(paramfile);
	if (syscmd)
		free(syscmd);
	return 0;
}
