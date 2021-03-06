/*
 * Copyright (c) 2006,2008,2009,2017 Solar Designer <solar at openwall.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted.
 *
 * There's ABSOLUTELY NO WARRANTY, express or implied.
 */

#ifndef _BLISTS_PARAMS_H
#define _BLISTS_PARAMS_H

/*
 * The directory with mailboxes and index files, used by the CGI program
 * only.  (bindex assumes that full pathnames are supplied to it.)
 */
#define MAIL_SPOOL_PATH			"../../blists"

#define MIN_YEAR			1970
#define MAX_YEAR			2038

/*
 * The suffix to append to a mailbox filename to form the corresponding
 * index file's name.
 */
#define INDEX_FILENAME_SUFFIX		".idx"

/*
 * Maximum message size in bytes (longer ones are truncated at this size).
 */
#define MAX_MESSAGE_SIZE		(10 * 1024 * 1024)

/*
 * Maximum attachment filename length (longer ones are truncated at this
 * length) prior to appending the .txt or .bin suffix.
 */
#define MAX_FILENAME_LENGTH		100

/*
 * Don't turn URLs longer than this number of characters into hyperlinks.
 */
#define MAX_URL_LENGTH			1000

/*
 * Maximum number of messages per day on month index pages.
 */
#define MAX_SHORT_MSG_LIST		100

/*
 * Maximum number of recent messages on the mailing list's main page.
 */
#define MAX_RECENT_MSG_LIST		100

/*
 * Introduce some sane limits on the mailbox size in order to prevent
 * a single huge mailbox from stopping the entire service.
 * Well, that was the original intent; these got insane for blists, which
 * does incremental index updates and supports 64-bit file offsets now.
 */
#define MAX_MAILBOX_MESSAGES		(100 * 1000 * 1000)
#define MAX_MAILBOX_BYTES		(100ULL * 1024 * 1024 * 1024)

/*
 * Locking method your system uses for user mailboxes.  It is important
 * that you set this correctly.
 *
 * *BSDs use flock(2), others typically use fcntl(2).
 */
#define LOCK_FCNTL			1
#define LOCK_FLOCK			0

/*
 * File buffer size to use while parsing the mailbox.  Can be changed.
 */
#define FILE_BUFFER_SIZE		0x10000

/*
 * The mailbox parsing code isn't allowed to truncate lines earlier than
 * this length.  Keep this at least as large as the longest header line
 * that we need to parse, but not too large for performance reasons.
 */
#define LINE_BUFFER_SIZE		0x1000

#endif
