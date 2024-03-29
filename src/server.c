/**
 * Copyright (c) 2022 Vladimir Sviridov <v.shtille@gmail.com>.
 * Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
 */

#include "server.h"
#include "answer.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

struct server_t * server__init(int width, int height, int bytes_per_pixel)
{
	struct server_t * server;
	int saim_error;

	server = (struct server_t *) malloc(sizeof(struct server_t));
	if (server == NULL)
	{
		printf("Server allocation has failed O_o\n");
		return NULL;
	}

	server->saim = NULL;
	server->daemon = NULL;
	server->buffer = NULL;
	server->width = width;
	server->height = height;
	server->bytes_per_pixel = bytes_per_pixel;
	server->file_root = NULL;

	// Init saim
	server->saim = saim_init(
		"", // const char* path
		NULL, // saim_provider_info * provider_info
		0, // int flags
		1, // int service_count
		&saim_error); // int * error
	if (server->saim == NULL)
	{
		printf("Saim init failed with error %i\n", saim_error);
		free((void*)server);
		return NULL;
	}

	// Set saim target buffer
	server->buffer_size = sizeof(unsigned char) * width * height * bytes_per_pixel;
	server->buffer = malloc(server->buffer_size);
	if (server->buffer == NULL)
	{
		printf("Buffer allocation has failed O_o\n");
		saim_cleanup(server->saim);
		free((void*)server);
		return NULL;
	}
	saim_set_target(server->saim, server->buffer, width, height, bytes_per_pixel);
	saim_set_bitmap_cache_size(server->saim, 50);

	// Init mutex
	if (mtx_init(&server->mutex, mtx_plain) == thrd_error)
	{
		printf("Mutex init has failed O_o\n");
		saim_cleanup(server->saim);
		free((void*)server);
		return NULL;
	}

	return server;
}
int server__start(struct server_t * server, int port, const char * file_root, const char * index_file)
{
	if (server->daemon != NULL)
	{
		printf("Daemon has already been started\n");
		return 1;
	}
	// Copy file root
	if (file_root != NULL)
	{
		size_t len = strlen(file_root);
		server->file_root = (char*) malloc((len+1)*sizeof(char));
		if (server->file_root == NULL)
			return 2;
		strcpy(server->file_root, file_root);
		server->file_root[len] = '\0';
		printf("File root directory is set to: %s\n", server->file_root);
	}
	// Copy index file
	if (index_file != NULL)
	{
		size_t len = strlen(index_file);
		server->index_file = (char*) malloc((len+2)*sizeof(char));
		if (server->index_file == NULL)
			return 2;
		if (index_file[0] == '/')
		{
			strcpy(server->index_file, index_file);
			server->index_file[len] = '\0';
		}
		else // append slash
		{
			server->index_file[0] = '/';
			strcpy(server->index_file+1, index_file);
			server->index_file[len+1] = '\0';
		}
		printf("Index file is set to: %s\n", server->index_file);
	}
	// Start daemon
	server->daemon = MHD_start_daemon(
		MHD_USE_AUTO | MHD_USE_INTERNAL_POLLING_THREAD | MHD_USE_ERROR_LOG,
		port, // port
		NULL, // policy callback
		NULL, // policy context
		&answer_callback, // request callback
		(void*)server, // request context
		//MHD_OPTION_ARRAY, &array[0], MHD_OPTION_END
		MHD_OPTION_CONNECTION_TIMEOUT, (unsigned int) 120,
		MHD_OPTION_STRICT_FOR_CLIENT, (int) 1,
		MHD_OPTION_END);
	if (server->daemon == NULL)
	{
		printf("Daemon start error\n");
		return 2;
	}
	printf("Start listening on port %i\n", port);
	return 0;
}
void server__stop(struct server_t * server)
{
	if (server->daemon != NULL)
	{
		MHD_stop_daemon(server->daemon);
		server->daemon = NULL;
	}
}
void server__free(struct server_t * server)
{
	server__stop(server);
	mtx_destroy(&server->mutex);
	if (server->buffer != NULL)
	{
		free((void*)server->buffer);
		server->buffer = NULL;
	}
	if (server->saim != NULL)
	{
		saim_cleanup(server->saim);
		server->saim = NULL;
	}
	if (server->file_root != NULL)
	{
		free((void*)server->file_root);
		server->file_root = NULL;
	}
	if (server->index_file != NULL)
	{
		free((void*)server->index_file);
		server->index_file = NULL;
	}
	free((void*)server);
	server = NULL;
}