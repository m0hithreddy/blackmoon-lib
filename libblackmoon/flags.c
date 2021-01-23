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
#include <stdarg.h>
#include <string.h>

struct bm_flags bm_set_flags(int first, ...) {
	struct bm_flags flags;
	memset(&flags, 0, sizeof(struct bm_flags));

	if (first == -1) // Return with no flag set
		return flags;

	/* Iterate through variadac arguments and set flags */
	va_list ap;

	va_start(ap, first);
	int flag;
	for ( ; ; ) {
		flag = va_arg(ap, int);
		if (flag == -1)
			break;

		set_bit((void*) flags.f, flag);
	}
	va_end(ap);

	return flags;
}

int isflag_set(struct bm_flags flags, int flag) {
	if (flag == -1)
		return 0;

	/* Compute flag location */
	int q = flag / sizeof(uint32_t);
	int r = flag % sizeof(uint32_t);

	return (int) get_bit((void*) &(flags.f[q]), r);
}

int bm_clear_flags(struct bm_flags flags, ...) {

	/* Iterate through variadic arguments and clear flags */
	va_list ap;

	va_start(ap, flags);
	int flag;
	for ( ; ; ) {
		flag = va_arg(ap, int);
		if (flag == -1)
			break;

		clear_bit((void*) flags.f, flag);
	}
	va_end(ap);

	return BM_ERROR_NONE;
}
