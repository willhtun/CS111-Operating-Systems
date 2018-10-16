#include <stdio.h>
#include <unistd.h> //getopt
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <fcntl.h> //open()

void causeSegFault() {
	char * ptr = NULL;
	*ptr = 'x';
}

void catchSegFault(int err) {
	if (err == SIGSEGV) {
		fprintf(stderr, "Segmentation Fault: Error = %d, %s \n", errno, strerror(errno));
		exit(4);
	}
}

int main(int argc, char *argv[]) {
	char * filein = NULL;
	char * fileout = NULL;
	int segfault = 0;
	int catchsf = 0;
	int opt;

	static struct option long_options[] = {
		{ "input", required_argument, 0, 'i'},
		{ "output", required_argument, 0, 'o' },
		{ "segfault", no_argument, 0, 's' },
		{ "catch", no_argument, 0, 'c' },
		{ 0, 0, 0, 0 }
	};

	while ((opt = getopt_long(argc, argv, "i:o:sc", long_options, NULL)) != -1) {	// -1 if there is no option left on the command line
		switch (opt) {
		case 'i':
			filein = optarg;	// optarg is the argument passed. Will be overwritten on next loop
			break;
		case 'o':
			fileout = optarg;
			break;
		case 's':
			segfault = 1;
			break;
		case 'c':
			catchsf = 1;
			break;
		default:
			printf("%s\n", "Undefined argument. Use -i [file], -o [file], -s, -c.\n");
			exit(1);
		}
	}

	if (catchsf) {
		signal(SIGSEGV, catchSegFault);
	}
	if (segfault) {
		causeSegFault();
	}
	if (filein) {
		int filein_d = open(filein, O_RDONLY);
		if (filein_d >= 0) {
			close(0);
			dup(filein_d);
			close(filein_d);
		}
		else {
			fprintf(stderr, "Cannot open file: Error = %d, %s \n", errno, strerror(errno));
			exit(2);
		}
	}
	if (fileout) {
		int fileout_d = creat(fileout, 0666);
		if (fileout_d >= 0) {
			close(1);
			dup(fileout_d);
			close(fileout_d);
		}
		else {
			fprintf(stderr, "Cannot create file: Error = %d, %s \n", errno, strerror(errno));
			exit(3);
		}
	}

	char * buffer[10];
	int r = read(0, buffer, 1);
	while (r != 0) {
		write(1, buffer, r);
		r = read(0, buffer, 1);
	}

	exit(0);
}
