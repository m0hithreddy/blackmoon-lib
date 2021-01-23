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
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <unistd.h>

int (bm_socket_write)(int sockfd, struct bm_data *bm_data, struct bm_socket_write va_list) {
	if (sockfd < 0 || bm_data == NULL || \
			bm_data->data == NULL || bm_data->size <= 0) {
		va_list.status != NULL ? *(va_list.status) = 0 : 0;
		return BM_ERROR_INVAL;
	}

	/* Variables needed for end routine */

	int no_block = -1, sock_args = -1, sigfd = -1, return_status = BM_ERROR_NONE;
	long wr_counter = 0;

	/* Set the socket mode to non-blocking */

	sock_args = fcntl(sockfd, F_GETFL);

	if (sock_args < 0) {
		return_status = BM_ERROR_INVAL;
		goto write_return;
	}

	no_block = (sock_args & O_NONBLOCK) > 0;

	if (no_block == 0)
		if (fcntl(sockfd, F_SETFL, sock_args | O_NONBLOCK) < 0) {
			return_status = BM_ERROR_INVAL;
			goto write_return;
		}

	/* Timeout initializations */

	struct timeval wr_time, tp_time;

	if (va_list.io_timeout >= 0) {
		wr_time.tv_sec = va_list.io_timeout;
		wr_time.tv_usec  = 0;
	}

	/* Select initializations */

	fd_set wr_set, tw_set;
	FD_ZERO(&wr_set);
	FD_SET(sockfd, &wr_set);

	/* Signal mask initializations */

	struct signalfd_siginfo sigbuf;
	fd_set rd_set, tr_set;
	FD_ZERO(&rd_set);

	if (va_list.sigmask != NULL) {
		sigfd = signalfd(-1, va_list.sigmask, 0);

		if (sigfd < 0) {
			return_status = BM_ERROR_INVAL;
			goto write_return;
		}

		FD_SET(sigfd, &rd_set);
	}

	/* Write to socket or Timeout or Respond to signal */

	long wr_status = 0;
	int sl_status = 0, maxfds = sigfd > sockfd ? sigfd + 1 : sockfd + 1;

	for ( ; ; ) {
		/* Wait for an event to occur */

		tw_set = wr_set;
		sl_status = select(maxfds, va_list.sigmask == NULL ? NULL : (tr_set = rd_set, &tr_set), \
				&tw_set, NULL, va_list.io_timeout >= 0 ? (tp_time = wr_time, &tp_time) : NULL);

		/* Check select return status */

		if (sl_status < 0) {
			if (errno == EINTR) {
				if (isflag_set(va_list.flags, BM_MODE_AUTO_RETRY))
					continue;
				else {
					return_status = BM_ERROR_RETRY;
					goto write_return;
				}
			}
			else {
				return_status = BM_ERROR_FATAL;
				goto write_return;
			}
		}
		else if (sl_status == 0) {
			return_status = BM_ERROR_TIMEOUT;
			goto write_return;
		}

		/* Check if signal received */

		if (va_list.sigmask != NULL) {
			if (FD_ISSET(sigfd, &tr_set)) {
				read(sigfd, &sigbuf, sizeof(struct signalfd_siginfo));

				return_status = BM_ERROR_SIGRCVD;
				goto write_return;
			}
		}

		/* Check if socket is made writable */

		if (!FD_ISSET(sockfd, &tw_set)) {
			return_status = BM_ERROR_FATAL;
			goto write_return;
		}

		/* Commence the write operation */

		wr_status = write(sockfd, bm_data->data + wr_counter, bm_data->size - wr_counter);

		/* Check write return status */

		if (wr_status < 0) {
			if (errno == EINTR) {
				if (isflag_set(va_list.flags, BM_MODE_AUTO_RETRY))
					continue;
				else {
					return_status = BM_ERROR_RETRY;
					goto write_return;
				}
			}
			else if (errno == EWOULDBLOCK || errno == EAGAIN) {
				if (isflag_set(va_list.flags, BM_MODE_AUTO_RETRY) || no_block == 0)	// We made it non-blocking!
					continue;
				else {
					return_status = BM_ERROR_RETRY;
					goto write_return;
				}
			}
			else {
				return_status = BM_ERROR_FATAL;
				goto write_return;
			}
		}

		if (wr_status > 0)
			wr_counter = wr_counter + wr_status;

		/* If fewer bytes are transfered */

		if (wr_counter < bm_data->size) {
			if (isflag_set(va_list.flags, BM_MODE_AUTO_RETRY))
				continue;
			else {
				return_status = BM_ERROR_RETRY;
				goto write_return;
			}
		}

		/* All bytes are transfered */

		return_status = BM_ERROR_NONE;
		goto write_return;
	}

	/* Return procedures */

write_return:

	/* Revert back the socket mode */

	if (no_block == 0) {
		if (fcntl(sockfd, F_SETFL, sock_args) < 0)
			return_status = BM_ERROR_FATAL;
	}

	/* Close any signalfd if opened */

	if (va_list.sigmask != NULL && sigfd >= 0)
		close(sigfd);

	/* Set the write_status of the socket */

	va_list.status != NULL ? *(va_list.status) = wr_counter : 0;

	return return_status;
}

int (bm_socket_read)(int sockfd, struct bm_data *bm_data, struct bm_socket_read va_list) {
	if (sockfd < 0 || bm_data == NULL || \
			bm_data->data == NULL || bm_data->size <= 0) {
		va_list.status != NULL ? *(va_list.status) = 0 : 0;
		return BM_ERROR_INVAL;
	}

	/* Variables needed for end routine */

	int no_block = -1, sock_args = -1, sigfd = -1, return_status = BM_ERROR_NONE;
	long rd_counter = 0;

	/* Set the socket mode to non-blocking */

	sock_args = fcntl(sockfd, F_GETFL);

	if (sock_args < 0) {
		return_status = BM_ERROR_INVAL;
		goto read_return;
	}

	no_block = (sock_args & O_NONBLOCK) > 0;

	if (no_block == 0)
		if (fcntl(sockfd, F_SETFL, sock_args | O_NONBLOCK) < 0) {
			return_status = BM_ERROR_INVAL;
			goto read_return;
		}

	/* Timeout initializations */

	struct timeval rd_time, tp_time;

	if (va_list.io_timeout >= 0) {
		rd_time.tv_sec = va_list.io_timeout;
		rd_time.tv_usec  = 0;
	}

	/* Select initializations */

	fd_set rd_set, tr_set;
	FD_ZERO(&rd_set);
	FD_SET(sockfd, &rd_set);

	/* Signal mask initializations */

	struct signalfd_siginfo sigbuf;

	if (va_list.sigmask != NULL) {
		sigfd = signalfd(-1, va_list.sigmask, 0);

		if (sigfd < 0) {
			return_status = BM_ERROR_INVAL;
			goto read_return;
		}

		FD_SET(sigfd, &rd_set);
	}

	/* Read from socket or Timeout or Respond to signal */

	long rd_status = 0;
	int sl_status = 0, maxfds = sigfd > sockfd ? sigfd + 1 : sockfd + 1;

	for ( ; ; ) {
		/* Wait for an event occur */

		tr_set = rd_set;
		sl_status = select(maxfds, &tr_set, NULL, NULL, va_list.io_timeout >= 0 ? \
				(tp_time = rd_time, &tp_time) : NULL);

		/* Check for select return status */

		if (sl_status < 0) {
			if (errno == EINTR) {
				if (isflag_set(va_list.flags, BM_MODE_AUTO_RETRY))
					continue;
				else {
					return_status = BM_ERROR_RETRY;
					goto read_return;
				}
			}
			else {
				return_status = BM_ERROR_FATAL;
				goto read_return;
			}
		}
		else if (sl_status == 0) {
			return_status = BM_ERROR_TIMEOUT;
			goto read_return;
		}

		/* Check if signal is received */

		if (va_list.sigmask != NULL) {
			if (FD_ISSET(sigfd, &tr_set)) {
				read(sigfd, &sigbuf, sizeof(struct signalfd_siginfo));

				return_status = BM_ERROR_SIGRCVD;
				goto read_return;
			}
		}

		/* Check if socket is made writable */

		if (!FD_ISSET(sockfd, &tr_set)) {
			return_status = BM_ERROR_FATAL;
			goto read_return;
		}

		/* Commence the Read operation */

		rd_status = read(sockfd, bm_data->data + rd_counter, bm_data->size - rd_counter);

		/* Check for read return status */

		if (rd_status < 0) {
			if (errno == EINTR) {
				if (isflag_set(va_list.flags, BM_MODE_AUTO_RETRY))
					continue;
				else {
					return_status = BM_ERROR_RETRY;
					goto read_return;
				}
			}
			else if (errno == EWOULDBLOCK || errno == EAGAIN) {
				if (isflag_set(va_list.flags, BM_MODE_AUTO_RETRY) || no_block == 0)	// We made it non-blocking!
					continue;
				else {
					return_status = BM_ERROR_RETRY;
					goto read_return;
				}
			}
			else if (errno == EFAULT) {
				return_status = BM_ERROR_BUFFER_FULL;
				goto read_return;
			}
			else {
				return_status = BM_ERROR_FATAL;
				goto read_return;
			}
		}
		else if (rd_status > 0) {
			rd_counter = rd_counter + rd_status;

			if (rd_counter == bm_data->size) {
				return_status = BM_ERROR_BUFFER_FULL;
				goto read_return;
			}

			if (isflag_set(va_list.flags,  BM_MODE_AUTO_RETRY))
				continue;
			else {
				return_status = BM_ERROR_RETRY;
				goto read_return;
			}
		}
		else {
			return_status = BM_ERROR_NONE;
			goto read_return;
		}
	}

	/* Return procedures */

read_return:

	/* Revert back the socket mode */

	if (no_block == 0) {
		if (fcntl(sockfd, F_SETFL, sock_args) < 0)
			return_status = BM_ERROR_FATAL;
	}

	/* Close any opened signalfd */

	if (va_list.sigmask != NULL && sigfd >= 0)
		close(sigfd);

	/* Set the socket read status */

	va_list.status != NULL ? *(va_list.status) = rd_counter : 0;

	return return_status;
}
