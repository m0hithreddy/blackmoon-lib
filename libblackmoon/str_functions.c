/*******************************************************************************
 * Copyright (C) 2020 - 2021, Mohith Reddy <dev.m0hithreddy@gmail.com>
 *
 * This file is part of blackmoon-lib <https://github.com/m0hithreddy/blackmoon-lib>
 *
 * blackmoon-lib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * blackmoon-lib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *******************************************************************************/
#define _GNU_SOURCE
#include "blackmoon.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

char* (strlocate)(char* haystack, char* needle, struct strlocate va_list) {
	if (haystack == NULL || needle == NULL || va_list.hstart < 0 || \
			va_list.hend < 0 || va_list.nstart < 0 || \
			va_list.nend < 0 || va_list.hend < va_list.hstart || \
			va_list.nend < va_list.nstart) {
		return NULL;
	}

	/* Return the string occurrence pointer */

	return memmem(haystack + va_list.hstart, va_list.hend - va_list.hstart + 1, \
			needle + va_list.nstart, va_list.nend - va_list.nstart + 1);
}

char* (strcaselocate)(char* haystack, char* needle, struct strcaselocate va_list) {
	if (haystack == NULL || needle == NULL || va_list.hstart < 0 || \
			va_list.hend < 0 || va_list.nstart < 0 || \
			va_list.nend < 0 || va_list.hend < va_list.hstart || \
			va_list.nend < va_list.nstart) {
		return NULL;
	}

	/* Create a new sub_haystack */

	char *sub_haystack = strndup(haystack + va_list.hstart, va_list.hend - \
			va_list.hstart + 1);

	/* Create a new sub_needle */

	char *sub_needle = strndup(needle + va_list.nstart, va_list.nend - \
			va_list.nstart + 1);

	/* Determine the occurrence of needle in sub_haystack */

	char *hs_needle = strcasestr(sub_haystack, sub_needle);

	free(sub_haystack);
	free(sub_needle);

	if (hs_needle == NULL)
		return NULL;

	/* haystack + relative position is the result */

	return haystack + va_list.hstart + ((long) (hs_needle) - (long) (sub_haystack));
}

int (sseek)(struct bm_data* bm_data, char* seq_str, struct sseek va_list) {
	if (bm_data == NULL || bm_data->data == NULL || bm_data->size <= 0 || \
			va_list.max_seek <= 0 || seq_str == NULL) {	// Invalid Request
		va_list.update != NULL ? *(va_list.update) = bm_data : 0;
		return -1;
	}

	/* Seek through the characters */

	char seek_buf[2]; seek_buf[1] = '\0';
	long seek_count = 0;

	for ( ; seek_count < bm_data->size && seek_count < va_list.max_seek; seek_count++) {
		seek_buf[0] = ((char*) bm_data->data)[seek_count];

		if (isflag_set(va_list.flags, BM_SSEEK_PERMIT)) {	// If requested for delimiting operation
			if (strstr(seq_str, seek_buf) == NULL)
				break;
		}
		else {
			if (strstr(seq_str, seek_buf) != NULL)
				break;
		}
	}

	/* Send a sseek update */

	struct bm_data *update = NULL;

	if (isflag_set(va_list.flags, BM_UPDATE_INPUT)) {
		bm_data->size = bm_data->size - seek_count;
		memmove(bm_data->data, bm_data->data + seek_count, bm_data->size);

		update = bm_data;
	}
	else if (va_list.update != NULL) {
		update = create_bm_data(bm_data->size - seek_count);
		memcpy(update->data, bm_data->data + seek_count, update->size);
	}

	if (isflag_set(va_list.flags, BM_FREE_INPUT)) {
		free_bm_data(&bm_data);

		if (isflag_set(va_list.flags, BM_UPDATE_INPUT))
			update = NULL;
	}

	va_list.update != NULL ? *(va_list.update) = update : 0;

	return seek_count;
}

char* (scopy)(struct bm_data* bm_data, char* seq_str, struct scopy va_list) {
	if (bm_data == NULL || bm_data->data == NULL || bm_data->size <= 0 \
			|| va_list.max_copy <= 0 || seq_str == NULL) {	// Invalid Request

		va_list.update != NULL ? *(va_list.update) = bm_data : 0;
		return NULL;
	}

	/* Copy the characters into a bag*/

	char copy_buf[2]; copy_buf[1] = '\0';
	long copy_count = 0, seq_len = strlen(seq_str);

	for ( ; copy_count < bm_data->size && copy_count < va_list.max_copy; copy_count++) {
		copy_buf[0] = ((char*) bm_data->data)[copy_count];

		if (isflag_set(va_list.flags, BM_SCOPY_PERMIT)) {	// If requested for delimiting operation
			if (strstr(seq_str, copy_buf) == NULL)
				break;
		}
		else {
			if(strstr(seq_str, copy_buf) != NULL)
				break;
		}
	}

	/* Copy the results */

	char *result = NULL;

	if (copy_count) {
		result = malloc(copy_count + 1);
		memcpy(result, bm_data->data, copy_count);
		result[copy_count] = '\0'; 
	}

	/* Make a scopy update */

	struct bm_data *update = NULL;

	if (isflag_set(va_list.flags, BM_UPDATE_INPUT)) {
		bm_data->size = bm_data->size - copy_count;
		memmove(bm_data->data, bm_data->data + copy_count, bm_data->size);

		update = bm_data;
	}
	else if (va_list.update != NULL) {
		update = create_bm_data(bm_data->size - copy_count);
		memcpy(update->data, bm_data->data + copy_count, update->size);
	}

	/* If caller requested for input freeing */

	if (isflag_set(va_list.flags, BM_FREE_INPUT)) {
		free_bm_data(&bm_data);

		if (isflag_set(va_list.flags, BM_UPDATE_INPUT))
			update = NULL;
	}

	/* If caller requested for any seeking operation */

	if (isflag_set(va_list.flags, BM_SSEEK_DELIMIT)) {
		sseek(update, seq_str, .flags = set_flags(BM_UPDATE_INPUT, BM_SSEEK_DELIMIT));
	}
	else if (isflag_set(va_list.flags, BM_SSEEK_PERMIT)) {
		sseek(update, seq_str, .flags = set_flags(BM_UPDATE_INPUT, BM_SSEEK_PERMIT));
	}

	va_list.update != NULL ? *(va_list.update) = update : 0;

	return result;
}

char* bm_strappend(char *first, ...) {
	if (first == NULL)
		return NULL;

	va_list ap;
	/* Compute the memory requirements for new string */

	va_start(ap, first);
	long t_size = strlen(first);
	char *str;

	for (char *str = va_arg(ap, char*); str != NULL; \
			str = va_arg(ap, char*)) {
		t_size = t_size + strlen(str);
	}
	va_end(ap);

	/* Allocate memory and prepare new string */

	char* t_str = (char*) malloc(sizeof(char) * (t_size + 1));

	if (t_str == NULL)
		return NULL;

	*t_str = '\0';
	strcat(t_str, first);

	va_start(ap, first);
	for (char *str = va_arg(ap, char*); str != NULL; \
			str = va_arg(ap, char*)) {
		strcat(t_str, str);
	}
	va_end(ap);

	return t_str;
}

char* null_strappend(long nargs, ...) {
	va_list ap;
	/* Compute the memory requirements for new string */

	va_start(ap, nargs);
	long t_size = 0;
	char *str;

	for (long arg_count = 0; arg_count < nargs; arg_count++) {
		str = va_arg(ap, char*);
		if (str != NULL)
			t_size = t_size + strlen(str);
	}
	va_end(ap);

	/* Allocate memory and prepare new string */

	char* t_str = (char*) malloc(sizeof(char) * (t_size + 1));

	if (t_str == NULL)
		return NULL;

	*t_str = '\0';

	va_start(ap, nargs);
	for (long arg_count = 0; arg_count < nargs; arg_count++) {
		str = va_arg(ap, char*);
		if (str != NULL)
			strcat(t_str, str);
	}
	va_end(ap);

	return t_str;
}
