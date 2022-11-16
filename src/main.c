/**
 * Copyright (c) 2022 Vladimir Sviridov <v.shtille@gmail.com>.
 * Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
 */

#include "server.h"

#include "tinycthread.h"

#include <stdio.h>
#include <string.h>
#include <signal.h>

static mtx_t mutex;
static cnd_t condition_variable;
static struct server_t * server;
static int finishing = 0;

void on_terminate(int func)
{
	// Stop the server
	if (server != NULL)
	{
		server__stop(server);
		server__free(server);
		printf("\nThe server has been stopped\n");
	}

	mtx_lock(&mutex);
	finishing = 1;
	cnd_signal(&condition_variable);
	mtx_unlock(&mutex);
}

void print_usage(const char * name)
{
	printf("Usage: %s <option(s)>\n"
		   "Options:\n"
		   "\t-h,--help\tShow this help message\n"
		   "\t-p,--port\tPort to listen (default is 80)\n"
		   "\t-r,--root\tRoot directory for files (by default it's disabled)\n"
		, name);
}

/**
 * Parses arguments
 * 
 * @param[in] argc   Arguments count.
 * @param[in] argv   Arguments array.
 * @param[out] help  Whether help is requested.
 * @param[out] port  Port number to listen.
 * @param[out] root  File root directory.
 * @return Return value.
 */
int parse_arguments(int argc, char *const *argv, int * help, int * port, char const ** root)
{
	*help = 0;
	for (int i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
		{
			print_usage(argv[0]);
			*help = 1;
			return 0;
		}
		else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0)
		{
			if (i+1 < argc)
				*port = atoi(argv[++i]);
			else
			{
				printf("%s option requires an argument\n", argv[i]);
				return 1;
			}
		}
		else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--root") == 0)
		{
			if (i+1 < argc)
				*root = argv[++i];
			else
			{
				printf("%s option requires an argument\n", argv[i]);
				return 1;
			}
		}
		else
		{
			printf("unknown argument %s\n", argv[i]);
			return 1;
		}
	}
	return 0;
}

int main(int argc, char *const *argv)
{
	const char * file_root = NULL;
	int port = 80; // default port
	int ret;

	// Parse arguments
	if (parse_arguments(argc, argv, &ret, &port, &file_root) != 0)
		return 1;
	if (ret == 1) // help was requested
		return 0;

	// Init mutex and condition variable
	if (mtx_init(&mutex, mtx_plain) == thrd_error)
	{
		printf("Mutex init failed\n");
		return 2;
	}
	if (cnd_init(&condition_variable) == thrd_error)
	{
		printf("Condition variable init failed\n");
		mtx_destroy(&mutex);
		return 3;
	}

	// Init a server
	server = server__init(256, 256, 3);
	if (server == NULL)
	{
		printf("Server init failed\n");
		cnd_destroy(&condition_variable);
		mtx_destroy(&mutex);
		return 4;
	}

	// Start the server
	ret = server__start(server, port, file_root);
	if (ret != 0)
	{
		server__free(server);
		cnd_destroy(&condition_variable);
		mtx_destroy(&mutex);
		return 5;
	}

	// Set interruption callback
	signal(SIGINT, on_terminate);
	signal(SIGTERM, on_terminate);

	// Wait for interruption signal
	// (void)getc(stdin);
	mtx_lock(&mutex);
	while (finishing == 0)
		cnd_wait(&condition_variable, &mutex);
	mtx_unlock(&mutex);

	// Finally
	cnd_destroy(&condition_variable);
	mtx_destroy(&mutex);

	return 0;
}