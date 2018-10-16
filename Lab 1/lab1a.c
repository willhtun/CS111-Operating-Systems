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
#include <errno.h>

struct termios original_terminal;
struct termios new_terminal;

pid_t processID = -1;
int shell;

void restoreTerminal() {
	tcsetattr(0, TCSANOW, &original_terminal);
}

void signal_handler(int signum)
{
	if (shell && signum == SIGINT)
	{
		if (kill(processID, SIGINT) == -1) {
			fprintf(stderr, "ERROR: %s\n", strerror(errno));
			exit(1);
		}
	}
	if (signum == SIGPIPE)
	{
		exit(0);
	}
}

int main(int argc, char ** argv) {
	int opt;
	char currChar[256];
	int inputStatus;
	shell = 0;
	
	static struct option long_options[] = {
		{ "shell", optional_argument, 0, 's' },
		{ 0, 0, 0, 0 }
	};

	while ((opt = getopt_long(argc, argv, "s", long_options, NULL)) != -1) {	// -1 if there is no option left on the command line
		switch (opt) {
		case 's':
			shell = 1;	// shell option
			break;
		default:
			printf("%s", "Undefined argument. Use -s.\n");
			exit(1);
		}
	}

	// Save original termninal mode
	int t_getOrigTerm = tcgetattr(0, &original_terminal);
	if (t_getOrigTerm != 0) {
		fprintf(stderr, "Error getting attributes from terminal.\n");
		exit(1);
	}

	// Change attributes in new terminal mode
	atexit(restoreTerminal);
	int t_getNewTerm = tcgetattr(0, &new_terminal);
	if (t_getNewTerm != 0) {
		fprintf(stderr, "Error getting attributes from terminal.\n");
		exit(1);
	}
	new_terminal.c_iflag = ISTRIP;	/* only lower 7 bits */
	new_terminal.c_oflag = 0;		/* no processing */
	new_terminal.c_lflag = 0;		/* no processing */
	int t_changeTerm = tcsetattr(0, TCSANOW, &new_terminal);
	if (t_changeTerm != 0) {
		fprintf(stderr, "Error setting attributes from terminal.\n");
		exit(1);
	}

	// Non Shell
	if (!shell) {
		inputStatus = read(0, &currChar, sizeof(char));
		if (inputStatus < 0) {
			fprintf(stderr, "Error reading input.\n");
			exit(1);
		}
		while (inputStatus > 0) {
			switch (currChar[0]) {
			case 4: // ^D
				;
				// Restore
				int t_restoreTerm = tcsetattr(0, TCSANOW, &original_terminal);
				if (t_restoreTerm != 0) {
					fprintf(stderr, "Error restoring attributes from saved terminal.\n");
					exit(1);
				}
				exit(0);
			case '\n':
			case '\r':
				;
				char endlineBuffer[2] = { 0x0D, 0x0A };
				write(1, endlineBuffer, 2);
				break;
			default:
				write(1, &currChar, sizeof(char));
				break;
			}
			inputStatus = read(0, &currChar, sizeof(char));
			if (inputStatus < 0) {
				fprintf(stderr, "Error reading input.\n");
				exit(1);
			}
		}
	}
	// Shell
	else {
		int pipeTo[2];
		int pipeFrom[2];
		if (pipe(pipeTo) < 0 || pipe(pipeFrom) < 0) {
			fprintf(stderr, "Error creating pipe.\n");
			exit(1);
		}
		int val;


		
		signal(SIGPIPE, signal_handler);

		processID = fork();

		

		if (processID < 0) {
			fprintf(stderr, "Error forking process.\n");
			exit(1);
		}
		else if (processID == 0) { // Child
			close(pipeTo[1]);
			close(pipeFrom[0]);
			dup2(pipeTo[0], 0);
			dup2(pipeFrom[1], 1);
			dup2(pipeFrom[1], 2);
			close(pipeTo[0]);
			close(pipeFrom[1]);

			char *temp_arg[1] = { NULL };
			if (execvp("/bin/bash", temp_arg) < 0) {
				fprintf(stderr, "Error executing shell.\n");
				exit(1);
			}
		}
		else { // Parent
			struct pollfd fds[2];
			int i;
			int numBytesRead;
			int waitStatus;

			close(pipeTo[0]);
			close(pipeFrom[1]);
			fds[0].fd = 0;
			fds[1].fd = pipeFrom[0];
			fds[0].events = POLLIN | POLLERR | POLLHUP;
			fds[1].events = POLLIN | POLLERR | POLLHUP;
			while (1) {
				val = poll(fds, 2, 0);
				if (val > 0) {
					// Shell Out
					if (fds[1].revents == POLLIN) {
						i = 0;
						numBytesRead = read(fds[1].fd, &currChar, 256);
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
					// Shell Out Error
					else if (fds[1].revents == POLLHUP || fds[1].revents == POLLERR) {
						waitpid(processID, &waitStatus, 0);
						fprintf(stderr, "\n\n\rSHELL EXIT SIGNAL=%d STATUS=%d\n\r", WTERMSIG(waitStatus), WEXITSTATUS(waitStatus));
						close(pipeFrom[0]);
						break;
					}
					// Keyboard
					if (fds[0].revents == POLLIN) {
						i = 0;
						numBytesRead = read(0, &currChar, 256);
						if (numBytesRead < 0) {
							fprintf(stderr, "Error reading input.\n");
							exit(1);
						}
						while (i < numBytesRead) {
							switch (currChar[i]) {
							case 3: // ^C
								kill(processID, SIGINT);
								i++;
								break;
							case 4: // ^D
								close(pipeTo[1]);
								i++;
								break;
							case '\n':
							case '\r':
								;
								char endlineBuffer[2] = { 0x0D, 0x0A };
								write(1, endlineBuffer, 2);
								char el = 0x0A;
								write(pipeTo[1], &el, sizeof(char));
								i++;
								break;
							default:
								write(1, &currChar[i], sizeof(char));
								write(pipeTo[1], &currChar[i], sizeof(char));
								i++;
								break;
							}
						}
					}
					// Keyboard Error
					else if (fds[0].revents == POLLHUP || fds[0].revents == POLLERR) {
						kill(processID, SIGINT);
						waitpid(processID, &waitStatus, 0);
						fprintf(stderr, "\n\n\rSHELL EXIT SIGNAL=%d STATUS=%d\n\r", WTERMSIG(waitStatus), WEXITSTATUS(waitStatus));
						close(pipeFrom[0]);
						exit(1);
					}
				}
				else if (val < 0) {
					break;
				}
			}
		}
	}
	exit(0);
}
