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
void to_main_charset(struct buffer *dst, struct buffer *enc, char *charset)
{
	char *iptr = enc->start;
	size_t inlen = enc->ptr - enc->start;
	char *p;

	/* sanitize charset string */
	p = charset;
	while ((*p >= 'a' && *p <= 'z') ||
	    (*p >= 'A' && *p <= 'Z') ||
	    (*p >= '0' && *p <= '9') ||
	    (*p == '-'))
		p++;
	if (*p == '?')
		*p = 0;
	else
		charset = UNKNOWN_CHARSET;

	if (!strcasecmp(MAIN_CHARSET, charset) ||
	    !whitelisted_charset(charset))
		buffer_append(dst, iptr, inlen);
	else {
		iconv_t cd = iconv_open(MAIN_CHARSET, charset);
		char out[ICONV_BUF_SIZE];

		if (cd == (iconv_t)(-1))
			cd = iconv_open(MAIN_CHARSET, UNKNOWN_CHARSET);
		assert(cd != (iconv_t)(-1));
		do {
			char *optr = out;
			size_t outlen = sizeof(out);
			int e = iconv(cd, &iptr, &inlen, &optr, &outlen);
			buffer_append(dst, out, optr - out);
			if (inlen == 0)
				break;
			if (e == -1) {
				buffer_appendc(dst, '?');
				iptr++;
				inlen--;
			}
		} while ((int)inlen > 0);
		iconv_close(cd);
	}

	enc->ptr = enc->start;
}

