#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <pthread.h>
#include <signal.h>
#include <mraa/aio.h>
#include <mraa/gpio.h>
#include <math.h>

// Flags
int opt_period = 1, opt_scaleC = 1, opt_log = 0;

// Log
char* logfile_name;
FILE* logfile;

// Sensor
mraa_aio_context temp_sensor;
mraa_gpio_context button;

// Commands
int lockCommands = 1;
int running = 1;

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
	else if (running)
		fprintf(stdout, "%s %.1f \n", time, temp);
}

void* temperature_func() {
	int sensor_value;
	float real_value;
	while (1) {
		sensor_value = mraa_aio_read(temp_sensor);
		if (opt_scaleC) // in C
			real_value = get_Cvalue(sensor_value);
		else // in F
			real_value = get_Fvalue(sensor_value);
		print_func(real_value);
		lockCommands = 0;
		sleep(opt_period);
	}
	return NULL;
}

void* button_func() {
	int button_value;
	while (1) {
		button_value = mraa_gpio_read(button);
		if (button_value) {
			char time[10];
			mark_time(time);
			printf("%s %s\n", time, "SHUTDOWN");
			if (opt_log)
				fprintf(logfile, "%s %s\n", time, "SHUTDOWN");
			exit(0);
		}
	}
}

void* commands_func() {
	char buffer[500];
	while (1) {
		if (lockCommands) 
			continue;
		int buffer_read = read(0, buffer, 500);
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
								fprintf(logfile, "%s%d%s", "PERIOD=", opt_period,"\n");
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
	return NULL;
}

int main(int argc, char *argv[]) {

	// Arguments
	int opt = 0;
	static struct option long_options[] = {
		{ "period", required_argument, 0, 'p' },
	{ "scale", required_argument, 0, 's' },
	{ "log", required_argument, 0, 'l' },
	{ 0, 0, 0, 0 }
	};
	while ((opt = getopt_long(argc, argv, "p:s:l:", long_options, NULL)) != -1) {	// -1 if there is no option left on the command line
		switch (opt) {
		case 'p':
			opt_period = strtol(optarg, NULL, 10);
			if (opt_period <= 0) {
				printf("%s", "Period must be an integer larger than 0.\n");
				exit(1);
			}
			break;
		case 's':
			if (optarg[0] == 'C')
				opt_scaleC = 1;
			else if (optarg[0] == 'F')
				opt_scaleC = 0;
			else {
				printf("%s", "Undefined option for scale. Use --scale=[C/F].\n");
				exit(1);
			}
			break;
		case 'l':
			opt_log = 1;
			logfile_name = optarg;
			break;
		default:
			printf("%s", "Undefined argument. Use --period=#, --scale=[C/F], or --log=[filename]. If no arguments are used, the default period is 1 sec and scale is in F.\n");
			exit(1);
		}
	}

	// File Open
	if (opt_log) {
		logfile = fopen(logfile_name, "a");
		if (logfile == NULL) {
			printf("%s", "Error opening file.\n");
			exit(2);
		}
	}

	// Initialize sensor and button
	temp_sensor = mraa_aio_init(1);
	if (temp_sensor == NULL) {
		printf("%s", "Error initiating temperature sensor.\n");
		exit(2);
	}
	button = mraa_gpio_init(62);
	if (!button) {
		printf("%s", "Error initiating button.\n");
		exit(2);
	}

	// Threads for parallelism
	int i;
	pthread_t *td = malloc(3 * sizeof(pthread_t));
	for (i = 0; i < 3; i++) {
		switch (i) {
		case 0:
			pthread_create(td + i, NULL, temperature_func, NULL);
			break;
		case 1:
			pthread_create(td + i, NULL, button_func, NULL);
			break;
		case 2:
			pthread_create(td + i, NULL, commands_func, NULL);
			break;
		}
	}
	for (i = 0; i < 3; i++) {
		pthread_join(td[i], NULL);
	}

	// Cleaning up
	printf("%s", "SHUTDOWN\n");
	free(td);
	if (opt_log) {
		fclose(logfile);
	}
	mraa_aio_close(temp_sensor);
	mraa_gpio_close(button);

	return 0;
}