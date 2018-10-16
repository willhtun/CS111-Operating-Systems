#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include <poll.h>
#include <time.h>
#include <pthread.h>
#include <mraa.h>
#include <aio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <sys/time.h>
#include <math.h>
#include <limits.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <signal.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/evp.h>

// Flags
int opt_period = 1, opt_scaleC = 1, opt_log = 0, opt_id = 0, opt_host = 0;

// Log
char* logfile_name;
FILE* logfile;
time_t time_marker;

// Sensor
mraa_aio_context temp_sensor;

// Commands
int running = 1;

// Port
int port;

// Socket
int socket_fd;
struct sockaddr_in socket_in;

// Host
struct hostent * server;
char* host;

// ID
int id;

// SSL
SSL_CTX * ssl_context;
SSL * ssl;

float get_Cvalue(int sensor_val) {
	const int B = 4275;
	const int R0 = 100000;

	int a = sensor_val;
	float R = 1023.0 / a - 1.0;
	R = R0 * R;
	float temperature = 1.0 / (log(R / R0) / B + 1 / 298.15) - 273.15;
	return temperature;
}

float get_Fvalue(int sensor_val) {
	float f = get_Cvalue(sensor_val);
	return (f * 1.8) + 32.0;
}

void mark_time(char *m_time) {
	time_t t;
	struct tm* local_t;

	time(&t);
	local_t = localtime(&t);
	strftime(m_time, 9, "%H:%M:%S", local_t);
}

void print_func(float temp) {
	char time[10];
	mark_time(time);
	if (running && opt_log)
		fprintf(logfile, "%s %.1f \n", time, temp);
	else if (running) {
		char server_buffer[50];
		sprintf(server_buffer, "%s %2.1f\n", time, temp);
		if (SSL_write(ssl, server_buffer, strlen(server_buffer)) < 0) {
			printf("%s", "Error writing to SSL.\n");
			exit(2);
		}
	}
}

void temperature_func() {
	int sensor_value;
	float real_value;

	sensor_value = mraa_aio_read(temp_sensor);
	if (opt_scaleC) // in C
		real_value = get_Cvalue(sensor_value);
	else // in F
		real_value = get_Fvalue(sensor_value);
	print_func(real_value);
}

void poll_func() {
	struct pollfd poll_input[1];
	poll_input[0].fd = socket_fd;
	poll_input[0].events = POLLIN;
	time_marker = 0;

	char buffer[500];
	while (1) {
		int ret = poll(poll_input, 1, 0);

		if (ret < 0) {
			printf("%s", "Polling error.\n");
			exit(2);
		}

		if (poll_input[0].revents & POLLIN) {
			int buffer_read = read(socket_fd, buffer, 500);
			if (buffer_read < 0) {
				printf("%s", "Error reading commands.\n");
				exit(2);
			}
			else if (buffer_read > 0) {
				int i;
				int length = 0;
				for (i = 0; i < buffer_read; i++) {
					if (buffer[i] == '\n') {
						buffer[i] = 0;
						if (strcmp(buffer + length, "START") == 0) {
							running = 1;
							if (opt_log)
								fprintf(logfile, "%s", "START\n");
						}
						else if (strcmp(buffer + length, "STOP") == 0) {
							running = 0;
							if (opt_log)
								fprintf(logfile, "%s", "STOP\n");
						}
						else if (strncmp(buffer + length, "PERIOD=", strlen("PERIOD=")) == 0) {
							char *str = (buffer + length) + strlen("PERIOD=");
							char *end;
							int new_period = strtol(str, &end, 10);
							if (new_period > 0) {
								opt_period = new_period;
								if (opt_log)
									fprintf(logfile, "%s%d%s", "PERIOD=", opt_period, "\n");
							}
						}
						else if (strncmp(buffer + length, "LOG ", strlen("LOG") + 1) == 0) {
							printf("%s %s", "LOG ", buffer + length);
							if (opt_log)
								fprintf(logfile, "%s %s", "LOG ", buffer + length);
						}
						else if (strcmp(buffer + length, "SCALE=C") == 0) {
							opt_scaleC = 1;
							if (opt_log)
								fprintf(logfile, "%s", "SCALE=C\n");
						}
						else if (strcmp(buffer + length, "SCALE=F") == 0) {
							opt_scaleC = 0;
							if (opt_log)
								fprintf(logfile, "%s", "SCALE=F\n");
						}
						else if (strcmp(buffer + length, "OFF") == 0) {
							char time[10];
							mark_time(time);
							if (opt_log) {
								fprintf(logfile, "%s", "OFF\n");
								fprintf(logfile, "%s %s", time, "SHUTDOWN\n");
							}
							exit(0);
						}
						else {
							if (opt_log) {
								fprintf(logfile, "%s", "Error reading command.\n");
							}
							exit(2);
						}
						length = i + 1;
					}
				}
			}
		}

		time_t current_time = time(0);
		if (current_time - time_marker >= opt_period && running) {
			temperature_func();
			time_marker = time(0);
		}
	}
}

int main(int argc, char *argv[]) {

	// Arguments
	int opt = 0;
	id = 111111111;
	host = "lever.cs.ucla.edu";
	if (argc < 2) {
		printf("%s", "No port error.\n");
		exit(1);
	}
	static struct option long_options[] = {
		{ "id", required_argument, 0, 'i' },
	{ "host", required_argument, 0, 'h' },
	{ "log", required_argument, 0, 'l' },
	{ 0, 0, 0, 0 }
	};
	while ((opt = getopt_long(argc, argv, "i:h:l:", long_options, NULL)) != -1) {	// -1 if there is no option left on the command line
		switch (opt) {
		case 'i':
			opt_id = 1;
			id = strtol(optarg, NULL, 10);
			break;
		case 'h':
			opt_host = 1;
			host = optarg;
			break;
		case 'l':
			opt_log = 1;
			logfile_name = optarg;
			break;
		default:
			printf("%s", "Undefined argument. Use --id=[ID], --host=[host], --log=[filename].\n");
			exit(1);
		}
	}

	// Port Open
	port = strtol(argv[optind], NULL, 10);
	if (port == 0) {
		printf("%s", "Port error.\n");
		exit(2);
	}

	// Socket Open
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (socket_fd < 0) {
		printf("%s", "Socket Error.\n");
		exit(2);
	}

	// Host Open
	server = gethostbyname(host);
	if (!server) {
		printf("%s", "Host error.\n");
		exit(2);
	}

	// Connection
	socket_in.sin_port = htons(port);
	socket_in.sin_family = AF_INET;
	memcpy((char *)&socket_in.sin_addr.s_addr, (char *)server->h_addr, server->h_length);
	if (connect(socket_fd, (struct sockaddr*) &socket_in, sizeof(socket_in)) < 0) {
		printf("%s", "Server connection error.\n");
		exit(2);
	}

	// SSL
	if (SSL_library_init() < 0) {
		printf("%s", "Error initializing SSL library.\n");
		exit(2);
	}
	OpenSSL_add_all_algorithms();

	ssl_context = SSL_CTX_new(TLSv1_client_method());
	if (!ssl_context) {
		printf("%s", "Error initializing SSL library.\n");
		exit(2);
	}

	ssl = SSL_new(ssl_context);
	if (SSL_set_fd(ssl, socket_fd) == 0) {
		printf("%s", "Error with socket.\n");
		exit(2);
	}
	if (SSL_connect(ssl) != 1) {
		printf("%s", "Error connecting to SSL.\n");
		exit(2);
	}

	// ID
	char id_SSL[50];
	sprintf(id_SSL, "ID=%s\n", id);
	if (SSL_write(ssl, id_SSL, strlen(id_SSL)) < 0) {
		printf("%s", "SSL ID write error.\n");
		exit(2);
	}
	if (opt_log) {
		fprintf(logfile, "ID=%s\n", id);
	}

	// Initialize sensor
	temp_sensor = mraa_aio_init(1);
	if (temp_sensor == NULL) {
		printf("%s", "Error initiating temperature sensor.\n");
		exit(2);
	}

	// Functions
	poll_func();

	// Cleaning up
	close(socket_fd < 0);
	SSL_shutdown(ssl);
	mraa_aio_close(temp_sensor);

	return 0;
}