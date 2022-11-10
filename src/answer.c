/**
 * Copyright (c) 2022 Vladimir Sviridov <v.shtille@gmail.com>.
 * Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
 */

#include "answer.h"
#include "server.h"
#include "image_format.h"

#include "saim_decoder_jpeg.h"
#include "saim_decoder_png.h"

#include <stdio.h>
#include <string.h>

typedef struct {
	int face, lod, x, y;
} arguments_t;

static const char* kServerError = "<html><body>An internal server error has occurred!</body></html>";

static const char* format_to_mime_type(enum image_format_t format)
{
	switch (format)
	{
		case FORMAT_JPEG:
			return "image/jpeg";
		case FORMAT_PNG:
		default:
			return "image/png";
	}
}

static bool get_image_format(struct MHD_Connection *connection, enum image_format_t * format)
{
	const char *key, *value;

	key = "format";
	value = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, key);
	if (value == NULL)
	{
		// Use JPEG by default
		*format = FORMAT_JPEG;
		return true;
	}
	if (strcmp(value, "png") == 0 ||
		strcmp(value, "PNG") == 0)
	{
		*format = FORMAT_PNG;
		return true;
	}
	if (strcmp(value, "jpg") == 0 || strcmp(value, "jpeg") == 0 ||
		strcmp(value, "JPG") == 0 || strcmp(value, "JPEG") == 0)
	{
		*format = FORMAT_JPEG;
		return true;
	}
	printf("image format '%s' is unknown\n", value);
	return false;
}

static bool get_tile_key(struct MHD_Connection *connection, int * face, int * lod, int * x, int *y)
{
	const char *key, *value;

	key = "face";
	value = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, key);
	if (value == NULL)
	{
		printf("key '%s' is missing\n", key);
		return false;
	}
	*face = atoi(value);

	key = "lod";
	value = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, key);
	if (value == NULL)
	{
		printf("key '%s' is missing\n", key);
		return false;
	}
	*lod = atoi(value);

	key = "x";
	value = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, key);
	if (value == NULL)
	{
		printf("key '%s' is missing\n", key);
		return false;
	}
	*x = atoi(value);

	key = "y";
	value = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, key);
	if (value == NULL)
	{
		printf("key '%s' is missing\n", key);
		return false;
	}
	*y = atoi(value);

	return true;
}

static bool parse_cube_arguments(struct MHD_Connection *connection, arguments_t * args)
{
	// Get tile coordinates
	if (!get_tile_key(connection, &args->face, &args->lod, &args->x, &args->y))
		return false;

	return true;
}

static int make_server_error_response(struct MHD_Connection *connection)
{
	struct MHD_Response * response;
	int ret;

	response = MHD_create_response_from_buffer(strlen(kServerError), (void*)kServerError, MHD_RESPMEM_PERSISTENT);
	ret = (int)MHD_queue_response(connection, MHD_HTTP_INTERNAL_SERVER_ERROR, response);
	MHD_destroy_response(response);

	return ret;
}

static int make_help_response(struct MHD_Connection *connection)
{
	struct MHD_Response * response;
	int ret;
	FILE * f;
	f = fopen("help.html", "rt");
	if (f == NULL)
	{
		printf("help file hasn't been found\n");
		return make_server_error_response(connection);
	}

	fseek(f, 0, SEEK_END);
	long fsize = ftell(f);
	fseek(f, 0, SEEK_SET);
	char *string = malloc(fsize + 1);
	(void)fread(string, fsize, 1, f);
	fclose(f);
	string[fsize] = '\0';

	response = MHD_create_response_from_buffer((size_t)fsize, (void*)string, MHD_RESPMEM_PERSISTENT);
	MHD_add_response_header(response, "Content-Type", "text/html");
	ret = (int)MHD_queue_response(connection, MHD_HTTP_OK, response);
	MHD_destroy_response(response);

	free(string);

	return ret;
}

static int make_image_response(struct MHD_Connection *connection, saim_string * data, enum image_format_t format)
{
	struct MHD_Response * response;
	int ret;
	const char* mime_type;

	mime_type = format_to_mime_type(format);
	response = MHD_create_response_from_buffer((size_t)data->length, (void*)data->data, MHD_RESPMEM_PERSISTENT);
	MHD_add_response_header(response, "Content-Type", mime_type);
	MHD_add_response_header(response, "Access-Control-Allow-Origin", "*");
	ret = (int)MHD_queue_response(connection, MHD_HTTP_OK, response);
	MHD_destroy_response(response);

	return ret;
}

static int make_jpeg_response(struct MHD_Connection *connection, struct server_t * server)
{
	saim_bitmap bitmap;
	saim_string data;
	unsigned char* dest_ptr = NULL;
	unsigned long dest_size = 0;
	int ret;

	// Make buffer string
	bitmap.data = server->buffer;
	/* 
	Quality 100 causes first image to have wrong header.
	So that's a library bug. We should either try to update to the latest libjpeg 
	or use lesser quality. Now I set it to 95.
	*/
	if (!saim_decoder_jpeg__save_to_buffer(&bitmap, 95, false,
		server->width, server->height, server->bytes_per_pixel, &dest_ptr, &dest_size))
	{
		if (dest_ptr != NULL)
			free(dest_ptr);
		return make_server_error_response(connection);
	}
	if (dest_ptr == NULL)
		return make_server_error_response(connection);
	data.data = (char*)dest_ptr;
	data.length = (unsigned int)dest_size;
	ret = make_image_response(connection, &data, FORMAT_JPEG);
	free(dest_ptr);
	return ret;
}

static int make_png_response(struct MHD_Connection *connection, struct server_t * server)
{
	saim_bitmap bitmap;
	saim_string data;
	int ret;

	// Make buffer string
	bitmap.data = server->buffer;
	saim_string_create(&data);
	if (!saim_decoder_png__save_to_buffer(&bitmap, false,
		server->width, server->height, server->bytes_per_pixel, &data))
	{
		saim_string_destroy(&data);
		return make_server_error_response(connection);
	}
	ret = make_image_response(connection, &data, FORMAT_PNG);
	saim_string_destroy(&data);
	return ret;
}

static bool render_mapped_cube(struct MHD_Connection *connection, const arguments_t * args, struct server_t * server)
{
	int tiles_left;

	mtx_lock(&server->mutex);
	do
	{
		tiles_left = saim_render_mapped_cube(server->saim, args->face, args->lod, args->x, args->y);
		thrd_yield();
	}
	while (tiles_left > 0);
	mtx_unlock(&server->mutex);

	// -1 means that inner error has occured
	return tiles_left == 0;
}

static bool process_cube_request(struct MHD_Connection *connection, struct server_t * server)
{
	arguments_t args;

	// Parse request arguments
	if (!parse_cube_arguments(connection, &args))
		return false;

	// Render to buffer
	return render_mapped_cube(connection, &args, server);
}

static int process_request(struct MHD_Connection *connection, const char* url, struct server_t * server)
{
	enum image_format_t format;

	if (strcmp(url, "/help") == 0)
		return make_help_response(connection);

	// Get image format
	if (!get_image_format(connection, &format))
		return false;

	// Process request
	if (!process_cube_request(connection, server))
		return make_server_error_response(connection);

	// Make response depending on requested image format
	switch (format)
	{
	case FORMAT_JPEG:
		return make_jpeg_response(connection, server);
	case FORMAT_PNG:
		return make_png_response(connection, server);
	default:
		return make_server_error_response(connection);
	}
}

__MHD_INT_RESULT
answer_callback(void *cls, struct MHD_Connection *connection,
	const char *url, const char *method, const char *version,
	const char *upload_data, size_t *upload_data_size, void **ptr)
{
	static int aptr;
	struct server_t * server = (struct server_t *) cls;

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

	return (__MHD_INT_RESULT) process_request(connection, url, server);
}