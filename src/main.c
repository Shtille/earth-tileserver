/**
 * Copyright (c) 2022 Vladimir Sviridov <v.shtille@gmail.com>.
 * Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
 */

#include "server.h"

#include "tinycthread.h"

#include <stdio.h>
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

int main(int argc, char *const *argv)
{
	int port;
	int ret;

	// Get port from argument
	if (argc != 2)
	{
		printf("%s PORT\n", argv[0]);
		return 1;
	}
	port = atoi(argv[1]);

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
	ret = server__start(server, port);
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