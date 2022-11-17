#include "mime_type.h"

#include <assert.h>
#include <string.h>

static void _strncpy(char * dest, const char * source, size_t max)
{
	size_t len = strlen(source);
	if (len < max)
	{
		strncpy(dest, source, len);
		dest[len] = '\0';
	}
	else
	{
		assert(!"Mime type string buffer overflow");
	}
}

void translate_url_to_mime_type(const char * url, char * mime_type, size_t max)
{
	const char * ptr = strrchr(url, '.');
	if (ptr == NULL)
	{
		_strncpy(mime_type, "text/plain", max);
		return;
	}
	++ptr;
	if (strcmp(ptr, "html") == 0)
		_strncpy(mime_type, "text/html", max);
	else if (strcmp(ptr, "css") == 0)
		_strncpy(mime_type, "text/css", max);
	else if (strcmp(ptr, "js") == 0)
		_strncpy(mime_type, "application/javascript", max);
	else
		_strncpy(mime_type, "text/plain", max);
}