/**
 * Copyright (c) 2022 Vladimir Sviridov <v.shtille@gmail.com>.
 * Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
 */

#include "answer.h"
#include "server.h"

#include <stdio.h>
#include <string.h>

/* Choose which image format response we want */
#define MIME_TYPE_PNG

#ifdef MIME_TYPE_PNG
#include "saim_decoder_png.h"
#else
#include "saim_decoder_jpeg.h"
#endif

static const char* kImageMimeType = 
#ifdef MIME_TYPE_PNG
	"image/png";
#else
	"image/jpeg";
#endif
static const char* kServerError = "<html><body>An internal server error has occurred!</body></html>";

static enum MHD_Result get_tile_key(struct MHD_Connection *connection, int * face, int * lod, int * x, int *y)
{
	const char *key, *value;

	key = "face";
	value = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, key);
	if (value == NULL)
	{
		printf("key '%s' is missing\n", key);
		return MHD_NO;
	}
	*face = atoi(value);

	key = "lod";
	value = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, key);
	if (value == NULL)
	{
		printf("key '%s' is missing\n", key);
		return MHD_NO;
	}
	*lod = atoi(value);

	key = "x";
	value = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, key);
	if (value == NULL)
	{
		printf("key '%s' is missing\n", key);
		return MHD_NO;
	}
	*x = atoi(value);

	key = "y";
	value = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, key);
	if (value == NULL)
	{
		printf("key '%s' is missing\n", key);
		return MHD_NO;
	}
	*y = atoi(value);

	return MHD_YES;
}

static enum MHD_Result make_server_error_response(struct MHD_Connection *connection)
{
	struct MHD_Response * response;
	enum MHD_Result ret;

	response = MHD_create_response_from_buffer(strlen(kServerError), (void*)kServerError, MHD_RESPMEM_PERSISTENT);
	ret = MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
	MHD_destroy_response(response);

	return ret;
}

static enum MHD_Result make_image_response(struct MHD_Connection *connection, saim_string * data)
{
	struct MHD_Response * response;
	enum MHD_Result ret;

	response = MHD_create_response_from_buffer((size_t)data->length, (void*)data->data, MHD_RESPMEM_PERSISTENT);
	MHD_add_response_header(response, "Content-Type", kImageMimeType);
	ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
	MHD_destroy_response(response);

	saim_string_destroy(data);

	return ret;
}

enum MHD_Result answer_callback(void *cls, struct MHD_Connection *connection,
	const char *url, const char *method, const char *version,
	const char *upload_data, size_t *upload_data_size, void **ptr)
{
	static int aptr;
	struct server_t * server = (struct server_t *) cls;
	int face, lod, x, y, tiles_left;
	saim_bitmap bitmap;
	saim_string data;
#ifndef MIME_TYPE_PNG
	unsigned char* dest_ptr = NULL;
	unsigned long dest_size;
#endif

	(void) url;               /* Unused. Silent compiler warning. */
	(void) version;           /* Unused. Silent compiler warning. */
	(void) upload_data;       /* Unused. Silent compiler warning. */
	(void) upload_data_size;  /* Unused. Silent compiler warning. */

	if (0 != strcmp(method, "GET"))
		return MHD_NO;              /* unexpected method */

	if (&aptr != *ptr)
	{
		/* do never respond on first call */
		*ptr = &aptr;
		return MHD_YES;
	}
	*ptr = NULL;                  /* reset when done */

	// Parse request arguments
	if (MHD_YES != get_tile_key(connection, &face, &lod, &x, &y))
		return make_server_error_response(connection);

	// Render to buffer
	mtx_lock(&server->mutex);
	do
	{
		tiles_left = saim_render_mapped_cube(server->saim, face, lod, x, y);
		thrd_yield();
	}
	while (tiles_left > 0);
	mtx_unlock(&server->mutex);
	if (tiles_left == -1)
		return make_server_error_response(connection);

	// Make PNG buffer string
	bitmap.data = server->buffer;
	saim_string_create(&data);
#ifdef MIME_TYPE_PNG
	if (!saim_decoder_png__save_to_buffer(&bitmap, false,
		server->width, server->height, server->bytes_per_pixel, &data))
#else
	if (!saim_decoder_jpeg__save_to_buffer(&bitmap, 100, false,
		server->width, server->height, server->bytes_per_pixel, &dest_ptr, &dest_size))
#endif
	{
#ifndef MIME_TYPE_PNG
		if (dest_ptr != NULL)
			free(dest_ptr);
#endif
		saim_string_destroy(&data);
		return make_server_error_response(connection);
	}
#ifndef MIME_TYPE_PNG
	if (dest_ptr != NULL)
	{
		saim_string_append(&data, (const char*)dest_ptr, (unsigned int)dest_size);
		free(dest_ptr);
	}
#endif

	return make_image_response(connection, &data);
}