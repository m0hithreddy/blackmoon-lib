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
#include "blackmoon.h"
#include <stdlib.h>

struct bm_data* create_bm_data(long bm_data_size) {
	if (bm_data_size < 0)
		return NULL;
	else if (bm_data_size == 0)
		return calloc(1, sizeof(struct bm_data));

	struct bm_data *bm_data = malloc(sizeof(struct bm_data));

	if (bm_data == NULL)
		return NULL;

	bm_data->data = calloc(1, bm_data_size);

	if (bm_data->data == NULL) {
		free(bm_data);
		return NULL;
	}

	bm_data->size = bm_data_size;

	return bm_data;
}

int (free_bm_data)(struct bm_data **_bm_data, struct free_bm_data va_list) {
	if (_bm_data == NULL || *_bm_data == NULL)
		return BM_ERROR_INVAL;

	int fd_status = BM_ERROR_NONE;

	if (va_list.ffunc != NULL && (*(va_list.ffunc))((*_bm_data)->data) != BM_ERROR_NONE)
		fd_status = BM_ERROR_INVAL;

	free(*_bm_data);

	*_bm_data = NULL;

	return fd_status;
}

struct bm_bag* create_bm_bag() {
	return calloc(1, sizeof(struct bm_bag));
}

int (free_bm_bag)(struct bm_bag** _bm_bag, struct free_bm_bag va_list) {
	if (_bm_bag == NULL || *_bm_bag == NULL)
		return BM_ERROR_INVAL;

	struct bm_pocket *at = NULL, *prev = NULL;

	/* Iterate and free the bm_pocket{} */
	int fb_status = BM_ERROR_NONE;

	for (at = (*_bm_bag)->end; at != NULL; at = prev) {
		prev = at->prev;
		if (va_list.ffunc != NULL &&(*(va_list.ffunc))(at->data) != BM_ERROR_NONE)
			fb_status = BM_ERROR_INVAL;
		free(at);
	}

	free(*_bm_bag);
	*_bm_bag = NULL;

	return fb_status;
}

int append_bm_pocket(struct bm_bag* bm_bag, long bm_pocket_size) {
	if (bm_bag == NULL)
		return BM_ERROR_INVAL;

	/* Create a new bm_pocket{} */

	struct bm_pocket *bm_pocket = malloc(sizeof(struct bm_pocket));

	if (bm_pocket == NULL)
		return BM_ERROR_FATAL;

	/* Allocate data for bm_pocket{}->data */

	bm_pocket->data = bm_pocket_size > 0 ? malloc(bm_pocket_size) : NULL;

	if (bm_pocket->data == NULL && bm_pocket_size > 0) {
		free(bm_pocket);
		return BM_ERROR_FATAL;
	}

	bm_pocket->size = bm_pocket_size > 0 ? bm_pocket_size : 0;

	/* Append bm_pocket{} to bm_bag{} */

	if (bm_bag->n_pkt == 0) {	// If bm_bag{} is empty
		bm_pocket->prev = NULL;
		bm_pocket->next = NULL;

		bm_bag->start = bm_pocket;
		bm_bag->end = bm_pocket;
		bm_bag->n_pkt = 1;
	}
	else {
		bm_pocket->prev = bm_bag->end;
		bm_pocket->next = NULL;

		bm_bag->end->next = bm_pocket;
		bm_bag->end = bm_pocket;
		bm_bag->n_pkt = bm_bag->n_pkt + 1;
	}

	return BM_ERROR_NONE;
}

int (delete_bm_pocket)(struct bm_bag* bm_bag, struct bm_pocket** _bm_pocket, struct delete_bm_pocket va_list) {
	if (bm_bag == NULL || _bm_pocket == NULL)
		return BM_ERROR_INVAL;

	struct bm_pocket *bm_pocket = *_bm_pocket;

	int db_status = BM_ERROR_NONE;

	/* Update the bm_bag{} depending on the position of bm_pocket{} */

	if (bm_pocket->prev == NULL && \
			bm_pocket->next == NULL) {	// bm_pocket{} == bm_bag{}->start == bm_bag{}->end
		bm_bag->start = NULL;
		bm_bag->end = NULL;
	}
	else if (bm_pocket->prev == NULL) {	// bm_pocket{} == bm_bag{}->start
		bm_bag->start = bm_pocket->next;
		bm_pocket->next->prev = NULL;
	}
	else if (bm_pocket->next == NULL) {	// bm_pocket{} == bm_bag{}->end
		bm_bag->end = bm_pocket->prev;
		bm_pocket->prev->next = NULL;
	}
	else {	// bm_pocket{} != bm_bag{}->start != bm_bag{}->end
		bm_pocket->prev->next = bm_pocket->next;
		bm_pocket->next->prev = bm_pocket->prev;
	}

	bm_bag->n_pkt = bm_bag->n_pkt - 1;

	/* Free bm_pocket{} */

	if (va_list.ffunc != NULL && (*(va_list.ffunc))(bm_pocket->data) != BM_ERROR_NONE)
		db_status = BM_ERROR_INVAL;

	free(bm_pocket);

	*_bm_pocket = NULL;

	return db_status;
}

int place_bm_data(struct bm_bag* bm_bag, struct bm_data* bm_data) {
	if (bm_bag == NULL || bm_data == NULL)
		return BM_ERROR_INVAL;

	/* Append a bm_pocket{} */

	int ap_status = append_bm_pocket(bm_bag, (bm_data->data == NULL || bm_data->size <= 0) \
			? 0 : bm_data->size);

	if (ap_status != BM_ERROR_NONE)
		return ap_status;

	/* Copy bm_data{}->data to newly created bm_pocket{}->data */

	if (bm_data->data != NULL && bm_data->size > 0)
		memcpy(bm_bag->end->data, bm_data->data, bm_data->size);

	return BM_ERROR_NONE;
}

struct bm_data* flatten_bm_bag(struct bm_bag *bm_bag)
{
	struct bm_data* bm_data = (struct bm_data*) calloc(1, sizeof(struct bm_data));

	if (bm_bag == NULL)
		return bm_data;

	/* Compute the memory requirements of new bm_data{} */

	long t_size = 0;

	for (struct bm_pocket* bm_pocket = bm_bag->start; bm_pocket != NULL; \
			bm_pocket = bm_pocket->next) {
		t_size = t_size + bm_pocket->size;
	}

	if (t_size <= 0)
		return bm_data;

	/* Copy the data in bm_bag{} to bm_data{}->data */

	bm_data->data = malloc(t_size);

	if (bm_data->data == NULL)
		return bm_data;

	for (struct bm_pocket* bm_pocket = bm_bag->start; bm_pocket != NULL; bm_pocket = bm_pocket->next) {
		if (bm_pocket->data != NULL && bm_pocket->size > 0) {
			memcpy(bm_data->data + bm_data->size, bm_pocket->data, bm_pocket->size);
			bm_data->size = bm_data->size + bm_pocket->size;
		}
	}

	return bm_data;
}
