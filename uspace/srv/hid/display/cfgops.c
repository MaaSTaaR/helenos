/*
 * Copyright (c) 2023 Jiri Svoboda
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 * - The name of the author may not be used to endorse or promote products
 *   derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/** @addtogroup display
 * @{
 */
/**
 * @file Display configuration ops implementation
 */

#include <errno.h>
#include <io/log.h>
#include <stdlib.h>
#include <str.h>
#include <dispcfg_srv.h>
#include "display.h"
#include "seat.h"
#include "cfgclient.h"

static errno_t dispc_get_seat_list(void *, dispcfg_seat_list_t **);
static errno_t dispc_get_seat_info(void *, sysarg_t, dispcfg_seat_info_t **);
static errno_t dispc_seat_create(void *, const char *, sysarg_t *);
static errno_t dispc_seat_delete(void *, sysarg_t);
static errno_t dispc_get_event(void *, dispcfg_ev_t *);

dispcfg_ops_t dispcfg_srv_ops = {
	.get_seat_list = dispc_get_seat_list,
	.get_seat_info = dispc_get_seat_info,
	.seat_create = dispc_seat_create,
	.seat_delete = dispc_seat_delete,
	.get_event = dispc_get_event,
};

/** Get seat list.
 *
 * @param arg Argument (CFG client)
 * @param rlist Place to store pointer to new list
 * @return EOK on success or an error code
 */
static errno_t dispc_get_seat_list(void *arg, dispcfg_seat_list_t **rlist)
{
	ds_cfgclient_t *cfgclient = (ds_cfgclient_t *)arg;
	dispcfg_seat_list_t *list;
	ds_seat_t *seat;
	unsigned i;

	log_msg(LOG_DEFAULT, LVL_DEBUG, "dispcfg_get_seat_list()");

	list = calloc(1, sizeof(dispcfg_seat_list_t));
	if (list == NULL)
		return ENOMEM;

	ds_display_lock(cfgclient->display);

	/* Count the number of seats */
	list->nseats = 0;
	seat = ds_display_first_seat(cfgclient->display);
	while (seat != NULL) {
		++list->nseats;
		seat = ds_display_next_seat(seat);
	}

	/* Allocate array for seat IDs */
	list->seats = calloc(list->nseats, sizeof(sysarg_t));
	if (list->seats == NULL) {
		ds_display_unlock(cfgclient->display);
		free(list);
		return ENOMEM;
	}

	/* Fill in seat IDs */
	i = 0;
	seat = ds_display_first_seat(cfgclient->display);
	while (seat != NULL) {
		list->seats[i++] = seat->id;
		seat = ds_display_next_seat(seat);
	}

	ds_display_unlock(cfgclient->display);
	*rlist = list;
	return EOK;
}

/** Get seat information.
 *
 * @param arg Argument (CFG client)
 * @param seat_id Seat ID
 * @param rinfo Place to store pointer to new seat information structure
 * @return EOK on success or an error code
 */
static errno_t dispc_get_seat_info(void *arg, sysarg_t seat_id,
    dispcfg_seat_info_t **rinfo)
{
	ds_cfgclient_t *cfgclient = (ds_cfgclient_t *)arg;
	ds_seat_t *seat;
	dispcfg_seat_info_t *info;

	log_msg(LOG_DEFAULT, LVL_DEBUG, "dispcfg_get_seat_info()");

	ds_display_lock(cfgclient->display);
	seat = ds_display_find_seat(cfgclient->display, seat_id);
	if (seat == NULL) {
		ds_display_unlock(cfgclient->display);
		return ENOENT;
	}

	info = calloc(1, sizeof(dispcfg_seat_info_t));
	if (info == NULL) {
		ds_display_unlock(cfgclient->display);
		return ENOMEM;
	}

	(void)seat;
	info->name = str_dup(seat->name);
	if (info->name == NULL) {
		ds_display_unlock(cfgclient->display);
		free(info);
		return ENOMEM;
	}

	ds_display_unlock(cfgclient->display);
	*rinfo = info;
	return EOK;
}

/** Create seat.
 *
 * @param arg Argument (CFG client)
 * @param name Seat name
 * @param rseat_id Place to store ID of the new seat
 * @return EOK on success or an error code
 */
static errno_t dispc_seat_create(void *arg, const char *name,
    sysarg_t *rseat_id)
{
	ds_cfgclient_t *cfgclient = (ds_cfgclient_t *)arg;
	ds_seat_t *seat;
	errno_t rc;

	log_msg(LOG_DEFAULT, LVL_DEBUG, "dispcfg_seat_create()");

	ds_display_lock(cfgclient->display);

	rc = ds_seat_create(cfgclient->display, name, &seat);
	if (rc != EOK) {
		ds_display_unlock(cfgclient->display);
		return rc;
	}

	(void) ds_display_paint(cfgclient->display, NULL);
	ds_display_unlock(cfgclient->display);

	*rseat_id = seat->id;
	return EOK;
}

/** Delete seat.
 *
 * @param arg Argument (CFG client)
 * @param seat_id Seat ID
 * @return EOK on success or an error code
 */
static errno_t dispc_seat_delete(void *arg, sysarg_t seat_id)
{
	ds_cfgclient_t *cfgclient = (ds_cfgclient_t *)arg;
	ds_seat_t *seat;

	log_msg(LOG_DEFAULT, LVL_DEBUG, "dispcfg_seat_delete()");

	ds_display_lock(cfgclient->display);
	seat = ds_display_find_seat(cfgclient->display, seat_id);
	if (seat == NULL) {
		ds_display_unlock(cfgclient->display);
		return ENOENT;
	}

	ds_seat_destroy(seat);

	(void) ds_display_paint(cfgclient->display, NULL);
	ds_display_unlock(cfgclient->display);
	return EOK;
}

/** Get display configuration event.
 *
 * @param arg Argument (CFG client)
 * @param ev Place to store event
 * @return EOK on success, ENOENT if there are no events
 */
static errno_t dispc_get_event(void *arg, dispcfg_ev_t *ev)
{
	ds_cfgclient_t *cfgclient = (ds_cfgclient_t *)arg;
	errno_t rc;

	log_msg(LOG_DEFAULT, LVL_DEBUG, "dispcfg_get_event()");

	ds_display_lock(cfgclient->display);
	rc = ds_cfgclient_get_event(cfgclient, ev);
	ds_display_unlock(cfgclient->display);
	return rc;
}

/** @}
 */
