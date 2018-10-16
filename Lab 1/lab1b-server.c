#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <signal.h>
#include <poll.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <zlib.h>


pid_t processID = -1;
int fd_socket = -1;

z_stream toClient;
z_stream fromClient;

void cleanup() {
	deflateEnd(&toClient);
	inflateEnd(&fromClient);
}

void prepareCompression() {
	if (atexit(cleanup) < 0) {
		fprintf(stderr, "Error at compression exit. %s\n", strerror(errno));
		exit(1);
	}
	toClient.zalloc = Z_NULL;
	toClient.zfree = Z_NULL;
	toClient.opaque = Z_NULL;
	if (deflateInit(&toClient, Z_DEFAULT_COMPRESSION) != Z_OK) {
		fprintf(stderr, "Error with deflateInit(): %s\n", toClient.msg);
		exit(1);
	}
	fromClient.zalloc = Z_NULL;
	fromClient.zfree = Z_NULL;
	fromClient.opaque = Z_NULL;
	if (inflateInit(&fromClient) != Z_OK)
	{
		fprintf(stderr, "Error with inflateInit(): %s\n", toClient.msg);
		exit(1);
	}
}

int compression(char* buffer, int numBytes) {
	int re_num;
	char t_buffer[2048];
	memcpy(t_buffer, buffer, numBytes);

	toClient.avail_in = numBytes;
	toClient.next_in = (Bytef *)t_buffer;
	toClient.avail_out = 2048;
	toClient.next_out = (Bytef *)buffer;

	deflate(&toClient, Z_SYNC_FLUSH);
	while (toClient.avail_in > 0) {
		deflate(&toClient, Z_SYNC_FLUSH);
	}

	re_num = 2048 - toClient.avail_out;
	return re_num;
}

int decompression(char* buffer, int numBytes) {
	int re_num;
	char t_buffer[2048];
	memcpy(t_buffer, buffer, numBytes);

	fromClient.avail_in = numBytes;
	fromClient.next_in = (Bytef *)t_buffer;
	fromClient.avail_out = 2048;
	fromClient.next_out = (Bytef *)buffer;

	inflate(&fromClient, Z_SYNC_FLUSH);
	while (fromClient.avail_in > 0) {
		inflate(&fromClient, Z_SYNC_FLUSH);
	}

	re_num = 2048 - fromClient.avail_out;
	return re_num;
}

void signal_handler(int signum)
{
	if (signum == SIGPIPE)
	{
		fprintf(stderr, "Segmentation fault. Error %d\n", signum);
		exit(0);
	}
}

void reset() {
	shutdown(fd_socket, SHUT_RDWR);
	close(fd_socket);
	int waitStatus;
	if (waitpid(processID, &waitStatus, 0) < 0) {
		printf("%s", "Wait process error.\n");
		exit(1);
	}
	int t_signal = 0, t_status = 0;
	if (WIFSIGNALED(waitStatus)) {
		t_signal = WTERMSIG(waitStatus);
	}
	if (WIFEXITED(waitStatus)) {
		t_status = WEXITSTATUS(waitStatus);
	}
	fprintf(stderr, "SHELL EXIT SIGNAL=%d, STATUS=%d\r\n", t_signal, t_status);
}

int createSocket(int portn) {

	int t_socket, t_newsocket;
	socklen_t clientlen;
	struct sockaddr_in serverAddr, clientAddr;

	t_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (t_socket < 0) {
		printf("%s", "Error creating socket.\n");
	}

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(portn);

	if (bind(t_socket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0) {
		printf("%s", "Error binding socket.\n");
	}

	listen(t_socket, 5);
	clientlen = sizeof(clientAddr);

	t_newsocket = accept(t_socket, (struct sockaddr *)&clientAddr, &clientlen);
	if (t_newsocket < 0) {
		printf("%s", "Error accepting socket.\n");
	}

	return t_newsocket;
}


int main(int argc, char ** argv) {
	int opt;
	int port = -1, compress_f = 0;
	char currChar_server[2048];
	char currChar_shell[2048];

	static struct option long_options[] = {
		{ "port", required_argument, 0, 'p' },
		{ "compress", no_argument, 0, 'c' },
		{ 0, 0, 0, 0 }
	};

	while ((opt = getopt_long(argc, argv, "p:c", long_options, NULL)) != -1) {	// -1 if there is no option left on the command line
		switch (opt) {
		case 'p':
			port = atoi(optarg);
			break;
		case 'c':
			compress_f = 1;
			prepareCompression();
			break;
		default:
			printf("%s", "Undefined argument. Use --port=portnumber, or --compress.\n");
			exit(1);
		}
	}

	if (port == -1) {
		printf("%s", "Port error.\n");
		exit(1);
	}

	// Create port socket
	fd_socket = createSocket(port);

	// READ WRITE ==============================================

	int pipeTo[2];
	int pipeFrom[2];
	if (pipe(pipeTo) < 0 || pipe(pipeFrom) < 0) {
		printf("%s", "Error creating pipe.\n");
		exit(1);
	}

	processID = fork();

	if (processID < 0) {
		printf("%s", "Error forking process.\n");
		exit(1);
	}
	else if (processID == 0) { // Child
		close(pipeTo[1]);
		close(pipeFrom[0]);
		close(fd_socket);
		dup2(pipeTo[0], 0);
		close(pipeTo[0]);
		dup2(pipeFrom[1], 1);
		dup2(pipeFrom[1], 2);
		close(pipeFrom[1]);

		char *temp_arg[1] = { NULL };
		if (execvp("/bin/bash", temp_arg) < 0) {
			printf("%s", "Error executing shell.\n");
			exit(1);
		}
	}
	else { // Parent
		struct pollfd fds[2];
		int i, val;
		int numBytesRead;

		close(pipeTo[0]);
		close(pipeFrom[1]);
		fds[0].fd = fd_socket;
		fds[1].fd = pipeFrom[0];
		fds[0].events = POLLIN | POLLERR | POLLHUP;
		fds[1].events = POLLIN | POLLERR | POLLHUP;

		if (signal(SIGPIPE, signal_handler) == SIG_ERR) {
			printf("%s", "Signal Error.\n");
		}

		if (atexit(reset) < 0) {
			printf("%s", "Exit Error.\n");
		}

		while (1) {
			val = poll(fds, 2, 0);
			if (val > 0) {
				// Shell Out
				if (fds[1].revents == POLLIN) {
					i = 0;
					numBytesRead = read(pipeFrom[0], currChar_shell, 2048);
					if (numBytesRead < 0) {
						printf("%s", "Error reading input.\n");
						exit(1);
					}
					while (i < numBytesRead) {
						switch (currChar_shell[i]) {
						case 4:
							exit(0);
						default:
							i++;
							break;
						}
					}
					if (compress_f) {
						numBytesRead = compression(currChar_shell, numBytesRead);
					}
					write(fd_socket, currChar_shell, numBytesRead);
				}
				// Shell Out Error
				else if (fds[1].revents == POLLHUP || fds[1].revents == POLLERR) {
					exit(0);
				}
				// Keyboard
				if (fds[0].revents == POLLIN) {
					i = 0;
					numBytesRead = read(fd_socket, currChar_server, 2048);
					if (numBytesRead < 0) {
						printf("%s", "Error reading input.\n");
						exit(1);
					}
					if (compress_f) {
						numBytesRead = decompression(currChar_server, numBytesRead);
					}
					while (i < numBytesRead) {
						switch (currChar_server[i]) {
						case 3: // ^C
							kill(processID, SIGINT);
							i++;
							break;
						case 4: // ^D
							close(pipeTo[0]);
							i++;
							break;
						case '\n':
						case '\r':
							;
							char el = 0x0A;
							write(pipeTo[1], &el, sizeof(char));
							i++;
							break;
						default:
							write(pipeTo[1], &currChar_server[i], sizeof(char));
							i++;
							break;
						}
					}
				}
				// Keyboard Error
				else if (fds[0].revents == POLLHUP || fds[0].revents == POLLERR) {
					exit(0);
				}
			}
			else if (val < 0) {
				exit(1);
			}
		}
	}
	exit(0);
}
