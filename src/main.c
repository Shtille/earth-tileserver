/**
 * Copyright (c) 2022 Vladimir Sviridov <v.shtille@gmail.com>.
 * Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
 */

#include "server.h"

#include <stdio.h>

int main(int argc, char *const *argv)
{
	struct server_t * server;
	int port;
	int ret;

	// Get port from argument
	if (argc != 2)
	{
		printf ("%s PORT\n", argv[0]);
		return 1;
	}
	port = atoi(argv[1]);

	// Init a server
	server = server__init(256, 256, 3);
	if (server == NULL)
	{
		printf("Server init failed");
		return 2;
	}

	// Start the server
	ret = server__start(server, port);
	if (ret != 0)
	{
		server__free(server);
		return 3;
	}

	// Wait for interruption signal
	(void)getc(stdin);

	// Stop the server
	server__stop(server);
	server__free(server);

	return 0;
}