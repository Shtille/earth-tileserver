/**
 * Copyright (c) 2022 Vladimir Sviridov <v.shtille@gmail.com>.
 * Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
 */

#ifndef __SERVER_H__
#define __SERVER_H__

#include "saim.h"
#include "tinycthread.h"
#include <microhttpd.h>

struct server_t
{
	struct saim_instance * saim;
	struct MHD_Daemon * daemon;
	unsigned char * buffer;
	size_t buffer_size;
	int width;
	int height;
	int bytes_per_pixel;
	char * file_root;
	mtx_t mutex;
};

struct server_t * server__init(int width, int height, int bytes_per_pixel);
int server__start(struct server_t * server, int port, const char * file_root);
void server__stop(struct server_t * server);
void server__free(struct server_t * server);

#endif