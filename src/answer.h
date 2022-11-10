/**
 * Copyright (c) 2022 Vladimir Sviridov <v.shtille@gmail.com>.
 * Distributed under the MIT License (license terms are at http://opensource.org/licenses/MIT).
 */

#ifndef __ANSWER_H__
#define __ANSWER_H__

#include <microhttpd.h>
#include <stdlib.h>

/* Older microhttpd library versions use int instead of enum. */
#if defined(_MHD_FIXED_ENUM)
# define __MHD_INT_RESULT enum MHD_Result
#else
# define __MHD_INT_RESULT int
#endif

__MHD_INT_RESULT
answer_callback(void *cls, struct MHD_Connection *connection,
	const char *url, const char *method, const char *version,
	const char *upload_data, size_t *upload_data_size, void **ptr);

#endif