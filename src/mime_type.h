#ifndef __MIME_TYPE_H__
#define __MIME_TYPE_H__

#include <stddef.h>

void translate_url_to_mime_type(const char * url, char * mime_type, size_t max);

#endif