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
#ifndef blackmoon_H
#define blackmoon_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <signal.h>

/* Error Defintions */

#define BM_ERROR_NONE 0
#define BM_ERROR_INVAL 1
#define BM_ERROR_FATAL 2
#define BM_ERROR_RETRY 3
#define BM_ERROR_TIMEOUT 4
#define BM_ERROR_SIGRCVD 5
#define BM_ERROR_BUFFER_FULL 6

/* Configuration Flags */

#define BM_FREE_INPUT 0
#define BM_UPDATE_INPUT 1
#define BM_SCOPY_PERMIT 2
#define BM_SCOPY_DELIMIT 3
#define BM_SSEEK_PERMIT 4
#define BM_SSEEK_DELIMIT 5
#define BM_MODE_AUTO_RETRY 6

typedef uint8_t bit;

typedef int (freeing_func)(void*);

struct bm_data {
	void *data;
	long size;
};

struct bm_pocket {
	struct bm_pocket *prev;
	void *data;
	long size;
	struct bm_pocket *next;
};

struct bm_bag {
	struct bm_pocket *start;
	long n_pkt;
	struct bm_pocket *end;
};

struct bm_flags {
	uint32_t f[1];
};

/* libblackmoon.c */

extern void print_hello ();

int bm_free(void *mem);

/* flags.c */

struct bm_flags bm_set_flags(int first, ...);

#define set_flags(...) bm_set_flags(__VA_ARGS__ __VA_OPT__(,) -1)

int isflag_set(struct bm_flags flags, int flag);

int bm_clear_flags(struct bm_flags flags, ...);

#define clear_flags(flags, ...) bm_clear_flags(flags, __VA_ARGS__, -1)

/* structures.c */

struct bm_data* create_bm_data(long bm_data_size);

struct free_bm_data {
	freeing_func *ffunc;
};

int free_bm_data(struct bm_data **_bm_data, struct free_bm_data va_list);

#define free_bm_data(_bm_data, ...) (free_bm_data)(_bm_data, \
		(struct free_bm_data){.ffunc = bm_free, __VA_ARGS__})

struct bm_bag* create_bm_bag();

struct free_bm_bag {
	freeing_func *ffunc;
}; 

int free_bm_bag(struct bm_bag **_bm_bag, struct free_bm_bag va_list);

#define free_bm_bag(_bm_bag, ...) (free_bm_bag)(_bm_bag, \
		(struct free_bm_bag) {.ffunc = bm_free, __VA_ARGS__})

int append_bm_pocket(struct bm_bag* bm_bag, long bm_pocket_size);

struct delete_bm_pocket {
	freeing_func *ffunc;
};

int delete_bm_pocket(struct bm_bag* bm_bag, struct bm_pocket** _bm_pocket, struct delete_bm_pocket va_list);

#define delete_bm_pocket(bm_bag, _bm_pocket, ...) (delete_bm_pocket)(bm_bag, _bm_pocket, \
		(struct delete_bm_pocket) {.ffunc = bm_free, __VA_ARGS__})

int place_bm_data(struct bm_bag *bm_bag, struct bm_data *bm_data);

struct bm_data* flatten_bm_bag(struct bm_bag *bm_bag);

/* str_functions.c */

struct strlocate {
	int hstart;
	int hend;
	int nstart;
	int nend;
};

char* strlocate(char* haystack, char* needle, struct strlocate va_list);

#define strlocate(haystack, needle, ...) ({ \
		typeof (haystack) _bm_haystack = haystack; \
		typeof (needle) _bm_needle = needle; \
		(strlocate)(_bm_haystack, _bm_needle, (struct strlocate) {.hstart = 0, \
				.hend = (int) strlen(_bm_haystack) - 1, .nstart = 0, \
				.nend = (int) strlen(_bm_needle) - 1, __VA_ARGS__}); \
				})

struct strcaselocate {
	int hstart;
	int hend;
	int nstart;
	int nend;
};

char* strcaselocate(char* haystack, char* needle, struct strcaselocate va_list);

#define strcaselocate(haystack, needle, ...) ({ \
		typeof (haystack) _bm_haystack = haystack; \
		typeof (needle) _bm_needle = needle; \
		(strcaselocate)(_bm_haystack, _bm_needle, (struct strcaselocate) {.hstart = 0, \
				.hend = (int) strlen(_bm_haystack) - 1, .nstart = 0, \
				.nend = (int) strlen(_bm_needle) - 1, __VA_ARGS__}); \
				})

struct sseek {
	struct bm_data **update;
	long max_seek;
	struct bm_flags flags;
};

int sseek(struct bm_data* bm_data, char* seq_str, struct sseek va_list);

#define sseek(bm_data, seq_str, ...) (sseek)(bm_data, seq_str, (struct sseek) {.update = NULL, \
		.max_seek = LONG_MAX, .flags = set_flags(BM_SSEEK_DELIMIT), __VA_ARGS__})

struct scopy {
	struct bm_data **update;
	long max_copy;
	struct bm_flags flags;
};

char* scopy(struct bm_data* bm_data, char* seq_str, struct scopy va_list);

#define scopy(bm_data, seq_str, ...) (scopy)(bm_data, seq_str, (struct scopy) {.update = NULL, \
		.flags = set_flags(BM_SCOPY_DELIMIT), .max_copy = LONG_MAX, __VA_ARGS__})

char* bm_strappend(char *first, ...);

#define strappend(...) bm_strappend(__VA_ARGS__ __VA_OPT__(,) NULL)

char* null_strappend(long nargs, ...);

/* bit.c */

int set_bit(void* bit_array, unsigned long bit_pos);

int clear_bit(void* bit_array, unsigned long bit_pos);

int toggle_bit(void* bit_array, unsigned long bit_pos);

int assign_bit(void* bit_array, unsigned long bit_pos, bit bit_val);

bit get_bit(void* bit_array, unsigned long bit_pos);

uint32_t bits_to_int(bit* bits, unsigned int _bit_count);

uint32_t bitarray_to_int(void* bit_array, unsigned long bit_start, unsigned int _bit_count);

int int_to_bitarray(uint32_t value, void* bit_array, unsigned long bit_start, unsigned int _bit_count);

/* socket.c */

struct bm_socket_write {
	long *status;
	struct bm_flags flags;
	long io_timeout;
	sigset_t *sigmask;
};

int bm_socket_write(int sockfd, struct bm_data *bm_data, struct bm_socket_write va_list);

#define bm_socket_write(sockfd, bm_data, ...) (bm_socket_write)(sockfd, bm_data, (struct bm_socket_write) \
		{.status = NULL, .flags = set_flags(BM_MODE_AUTO_RETRY), .io_timeout = -1, .sigmask = NULL, __VA_ARGS__})

struct bm_socket_read {
	long *status;
	struct bm_flags flags;
	long io_timeout;
	sigset_t *sigmask;
};

int bm_socket_read(int sockfd, struct bm_data *bm_data, struct bm_socket_read va_list);

#define bm_socket_read(sockfd, bm_data, ...) (bm_socket_read)(sockfd, bm_data, (struct bm_socket_read) \
		{.status = NULL, .flags = set_flags(), .io_timeout = -1, .sigmask = NULL, __VA_ARGS__})

#endif
