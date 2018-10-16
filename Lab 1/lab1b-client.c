

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
#include <sys/socket.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <zlib.h>

struct termios original_terminal;
struct termios new_terminal;

z_stream toShell;
z_stream fromShell;

void restoreTerminal() {
	if (tcsetattr(0, TCSANOW, &original_terminal) != 0) {
		exit(1);
	}
}

void cleanup() {
	deflateEnd(&toShell);
	inflateEnd(&fromShell);
}

void prepareCompression() {
	if (atexit(cleanup) < 0) {
		fprintf(stderr, "Error at compression exit. %s\n", strerror(errno));
	}
	toShell.zalloc = Z_NULL;
	toShell.zfree = Z_NULL;
	toShell.opaque = Z_NULL;
	if (deflateInit(&toShell, Z_DEFAULT_COMPRESSION) != Z_OK) {
		fprintf(stderr, "Error with deflateInit(): %s\n", toShell.msg);
		exit(1);
	}
	fromShell.zalloc = Z_NULL;
	fromShell.zfree = Z_NULL;
	fromShell.opaque = Z_NULL;
	if (inflateInit(&fromShell) != Z_OK)
	{
		fprintf(stderr, "Error with inflateInit(): %s\n", fromShell.msg);
		exit(1);
	}
}

int compression(char* buffer, int numBytes) {
	int re_num;
	char t_buffer[2048];
	memcpy(t_buffer, buffer, numBytes);

	toShell.avail_in = numBytes;
	toShell.next_in = (Bytef *)t_buffer;
	toShell.avail_out = 2048;
	toShell.next_out = (Bytef *)buffer;

	deflate(&toShell, Z_SYNC_FLUSH);
	while (toShell.avail_in > 0) {
		deflate(&toShell, Z_SYNC_FLUSH);
	}

	re_num = 2048 - toShell.avail_out;
	return re_num;
}

int decompression(char* buffer, int numBytes) {
	int re_num;
	char t_buffer[2048];
	memcpy(t_buffer, buffer, numBytes);

	fromShell.avail_in = numBytes;
	fromShell.next_in = (Bytef *)t_buffer;
	fromShell.avail_out = 2048;
	fromShell.next_out = (Bytef *)buffer;

	inflate(&fromShell, Z_SYNC_FLUSH);
	while (fromShell.avail_in > 0) {
		inflate(&fromShell, Z_SYNC_FLUSH);
	}

	re_num = 2048 - fromShell.avail_out;
	return re_num;
}

int createSocket(int portn) {

	int t_socket;
	struct sockaddr_in serverAddr;
	struct hostent *server;

	t_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (t_socket < 0) {
		printf("%s", "Error creating socket.\n");
		exit(1);
	}

	server = gethostbyname("localhost");
	if (server == NULL)
	{
		printf("%s", "Error with gethostbyname().\n");
		exit(1);
	}

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;

	memcpy((char *)&serverAddr.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
	serverAddr.sin_port = htons(portn);

	connect(t_socket, (struct sockaddr *) &serverAddr, sizeof(serverAddr));

	return t_socket;
}

int main(int argc, char ** argv) {
	int opt;
	int port = -1, log = 0, compress_f = 0;
	int logServ = -1;

	static struct option long_options[] = {
		{ "port", required_argument, 0, 'p' },
		{ "log", required_argument, 0, 'l' },
		{ "compress", no_argument, 0, 'c' },
		{ 0, 0, 0, 0 }
	};

	while ((opt = getopt_long(argc, argv, "p:l:c", long_options, NULL)) != -1) {	// -1 if there is no option left on the command line
		switch (opt) {
		case 'p':
			port = strtol(optarg, NULL, 10);
			break;
		case 'l':
			log = 1;
			logServ = creat(optarg, 0666);
			if (logServ < 0) {
				fprintf(stderr, "Error creating log file. %s\n", strerror(errno));
			}
			break;
		case 'c':
			compress_f = 1;
			prepareCompression();
			break;
		default:
			printf("%s", "Undefined argument. Use --port=portnumber, --log=filename, or --compress.\n");
			exit(1);
		}
	}

	if (port == -1) {
		printf("%s", "Port error.\n");
		exit(1);
	}

	// Create port socket
	int fd_socket = createSocket(port);

	// Save original termninal mode
	int t_getOrigTerm = tcgetattr(0, &original_terminal);
	if (t_getOrigTerm != 0) {
		printf("%s", "Error getting attributes from terminal.\n");
		exit(1);
	}

	// Change attributes in new terminal mode
	atexit(restoreTerminal);
	int t_getNewTerm = tcgetattr(0, &new_terminal);
	if (t_getNewTerm != 0) {
		printf("%s", "Error getting attributes from terminal.\n");
		exit(1);
	}
	new_terminal.c_iflag = ISTRIP;	/* only lower 7 bits */
	new_terminal.c_oflag = 0;		/* no processing */
	new_terminal.c_lflag = 0;		/* no processing */
	int t_changeTerm = tcsetattr(0, TCSANOW, &new_terminal);
	if (t_changeTerm != 0) {
		printf("%s", "Error setting attributes from terminal.\n");
		exit(1);
	}

	// READ WRITE ==============================================

	struct pollfd fds[2];
	int i, val;
	int numBytesRead;
	char currChar[2048];

	fds[0].fd = 0;
	fds[1].fd = fd_socket;
	fds[0].events = POLLIN;
	fds[1].events = POLLIN;
	while (1) {
		val = poll(fds, 2, 0);
		if (val > 0) {
			// From Server
			if (fds[1].revents == POLLIN) {
				i = 0;
				numBytesRead = read(fd_socket, currChar, 2048);
				if (numBytesRead == 0) {
					restoreTerminal();
					exit(0);
				}
				if (log) {
					if (dprintf(logServ, "RECEIVED %d bytes: ", numBytesRead) < 0)
					{
						fprintf(stderr, "dprintf() failed. %s\n", strerror(errno));
						exit(1);
					}
					if (write(logServ, currChar, numBytesRead) < 0)
					{
						fprintf(stderr, "write() failed. %s\n", strerror(errno));
						exit(1);
					}
					if (dprintf(logServ, "\n") < 0)
					{
						fprintf(stderr, "dprintf() failed. %s\n", strerror(errno));
						exit(1);
					}
				}
				if (compress_f) {
					numBytesRead = decompression(currChar, numBytesRead);
				}
				while (i < numBytesRead) {
					switch (currChar[i]) {
					case '\n':
					case '\r':
						;
						char endlineBuffer[2] = { 0x0D, 0x0A };
						write(1, endlineBuffer, 2);
						i++;
						break;
					default:
						write(1, &currChar[i], sizeof(char));
						i++;
						break;
					}
				}
			}
			// From Server Error
			else if (fds[1].revents == POLLHUP || fds[1].revents == POLLERR) {
				restoreTerminal();
				exit(0);
			}
			// To Server
			if (fds[0].revents == POLLIN) {
				i = 0;
				numBytesRead = read(0, &currChar, 2048);
				if (numBytesRead < 0) {
					printf("%s", "Error reading input.\n");
					exit(1);
				}

				while (i < numBytesRead) {
					switch (currChar[i]) {
					case '\n':
					case '\r':
						;
						char endlineBuffer[2] = { 0x0D, 0x0A };
						write(1, endlineBuffer, 2);
						i++;
						break;
					default:
						write(1, &currChar[i], sizeof(char));
						i++;
						break;
					}
				}
				if (compress_f) {
					numBytesRead = compression(currChar, numBytesRead);
				}
				write(fd_socket, currChar, numBytesRead);
				if (log) {
					if (dprintf(logServ, "SENT %d bytes: ", numBytesRead) < 0)
					{
						fprintf(stderr, "dprintf() failed. %s\n", strerror(errno));
						exit(1);
					}
					if (write(logServ, currChar, numBytesRead) < 0) {
						fprintf(stderr, "write() failed. %s\n", strerror(errno));
						exit(1);
					}
					if (dprintf(logServ, "\n") < 0) {
						fprintf(stderr, "dprintf() failed. %s\n", strerror(errno));
						exit(1);
					}
				}
			}
			// To Server Error
			else if (fds[0].revents == POLLHUP || fds[0].revents == POLLERR) {
				restoreTerminal();
				exit(0);
			}
		}
		else if (val < 0) {
			restoreTerminal();
			exit(1);
		}
	}
	exit(0);
}
