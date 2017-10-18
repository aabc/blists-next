/*
 * Charset encoding processing.
 *
 * Copyright (c) 2011,2014 ABC <abc at openwall.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted.
 *
 * There's ABSOLUTELY NO WARRANTY, express or implied.
 */

#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <iconv.h>
#include <errno.h>

#include "buffer.h"
#include "encoding.h"

static const char *charset_whitelist[] = {
	"us-ascii$",
	"iso",
	"utf-7",
	"koi8-r$",
	"koi8-u$",
	"windows-",
	"cp",
	"utf-8",
	NULL
};

static int simple_tolower(char ch)
{
	if (ch >= 'A' && ch <= 'Z')
		return ch + 'a' - 'A';
	return ch;
}

static int match_charset(const char *charset, const char *mask)
{
	int i;

	for (; *mask; mask++, charset++) {
		if (*mask == '$')
			return !*charset;
		if (*mask != simple_tolower(*charset))
			return 0;
	}
	/* allow eight chars of digits and dashes */
	for (i = 0; *charset && i < 8; i++, charset++) {
		char ch = *charset;

		if (!((ch >= '0' && ch <= '9') || ch == '-'))
			return 0;
	}
	return !*charset;
}

static int whitelisted_charset(const char *charset)
{
	const char **p;

	for (p = charset_whitelist; *p; p++)
		if (match_charset(charset, *p))
			return 1;
	return 0;
}

/* convert text from `enc' buffer to `dst' by `charset' (non-const) */
void encoding_to_utf8(struct buffer *dst, struct buffer *enc, char *charset)
{
	char *iptr = enc->start;
	size_t inlen = enc->ptr - enc->start;
	char *p;

	if (!charset)
		charset = UNKNOWN_CHARSET;

	/* sanitize charset string */
	p = charset;
	while ((*p >= 'a' && *p <= 'z') ||
	    (*p >= 'A' && *p <= 'Z') ||
	    (*p >= '0' && *p <= '9') ||
	    (*p == '-'))
		p++;
	if (*p == '?')
		*p = 0;
	if (*p)
		charset = UNKNOWN_CHARSET;

	if (!strcasecmp(UTF8_CHARSET, charset) ||
	    !whitelisted_charset(charset))
		buffer_append(dst, iptr, inlen);
	else {
		iconv_t cd = iconv_open(UTF8_CHARSET, charset);
		char out[ICONV_BUF_SIZE];

		if (cd == (iconv_t)(-1))
			cd = iconv_open(UTF8_CHARSET, UNKNOWN_CHARSET);
		assert(cd != (iconv_t)(-1));
		do {
			char *optr = out;
			size_t outlen = sizeof(out);
			int e = iconv(cd, &iptr, &inlen, &optr, &outlen);
			buffer_append(dst, out, optr - out);
			/* if output buffer is full (errno == E2BIG) we
			 * will just continue processing (it will be
			 * resumed on next iteration, because iconv()
			 * also updates iptr and inlen), otherwise
			 * report conversion error with REPLACEMENT
			 * CHARACTER (U+FFFD), which looks like <?>. */
			if (e == -1 && errno != E2BIG) {
				buffer_appenduc(dst, 0xFFFD);
				iptr++;
				inlen--;
			}
		} while ((int)inlen > 0);
		iconv_close(cd);
	}

	enc->ptr = enc->start;
}

/* remove partial utf8 character from string by reducing its len */
/* return how many bytes are removed, *lenp is modified to reflect new
 * length */
int utf8_remove_trailing_partial_character(char *ptr, int *lenp)
{
	int len;

	for (len = *lenp; len; ) {
		int s_size = 1; /* sequence size */
		unsigned char ch = *ptr;

		if (ch >= 0xf3)
			/* illegal multi-byte sequence */;
		else if (ch >= 0xf0)
			s_size = 4;
		else if (ch >= 0xe0)
			s_size = 3;
		else if (ch >= 0xc0)
			s_size = 2;
		if (len < s_size)
			break;
		len -= s_size;
		ptr += s_size;
	}

	*lenp -= len;
	return len;
}