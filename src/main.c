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

struct arguments_t {
	const char * file_root;
	const char * index_file;
	int port;
	int help;
};

void print_usage(const char * name)
{
	printf("Usage: %s <option(s)>\n"
		   "Options:\n"
		   "\t-h,--help\tShow this help message\n"
		   "\t-p,--port\tPort to listen (default is 80)\n"
		   "\t-r,--root\tRoot directory for files (by default it's disabled)\n"
		   "\t-i,--index\tIndex file (default is index.html)\n"
		, name);
}

/**
 * Parses arguments
 * 
 * @param[in] argc        Arguments count.
 * @param[in] argv        Arguments array.
 * @param[out] arguments  Arguments output structure.
 * @return Return value.
 */
int parse_arguments(int argc, char *const *argv, struct arguments_t * arguments)
{
	arguments->help = 0;
	for (int i = 1; i < argc; ++i)
	{
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
		{
			print_usage(argv[0]);
			arguments->help = 1;
			return 0;
		}
		else if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--port") == 0)
		{
			if (i+1 < argc)
				arguments->port = atoi(argv[++i]);
			else
			{
				printf("%s option requires an argument\n", argv[i]);
				return 1;
			}
		}
		else if (strcmp(argv[i], "-r") == 0 || strcmp(argv[i], "--root") == 0)
		{
			if (i+1 < argc)
				arguments->file_root = argv[++i];
			else
			{
				printf("%s option requires an argument\n", argv[i]);
				return 1;
			}
		}
		else if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--index") == 0)
		{
			if (i+1 < argc)
				arguments->index_file = argv[++i];
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
	struct arguments_t arguments;
	int ret;

	arguments.file_root = NULL;
	arguments.index_file = NULL;
	arguments.port = 80; // default port

	// Parse arguments
	if (parse_arguments(argc, argv, &arguments) != 0)
		return 1;
	if (arguments.help == 1) // help was requested
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
	ret = server__start(server, arguments.port, arguments.file_root, arguments.index_file);
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