/*
 * Copyright (c) 2022 Jevin Sweval (jevinsweval@gmail.com)
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

# include  "vpi_user.h"
# include  <assert.h>


static s_vpi_time cb_timerec = {vpiSimTime, 0, 0, 0};

static PLI_INT32 register_nextsimtime(struct t_cb_data* cb);

static PLI_INT32 nextsimtime_cb(struct t_cb_data* cb) {
      uint64_t nextsimtime = ((uint64_t)cb->time->high << 32) | cb->time->low;
      s_vpi_time timerec;
      timerec.type = vpiSimTime;
      vpi_get_time(NULL, &timerec);
      uint64_t time = ((uint64_t)timerec.high << 32) | timerec.low;
      vpi_printf("nextsimtime: %" PLI_UINT64_FMT " vpi_get_time: %" PLI_UINT64_FMT "\n",
            nextsimtime, time);
      register_nextsimtime(NULL);
      return 0;
}

static PLI_INT32 register_nextsimtime(struct t_cb_data* cb) {
      (void)cb; /* unused */
      s_cb_data cb_data;
      cb_data.time   = &cb_timerec;
      cb_data.reason = cbNextSimTime;
      cb_data.cb_rtn = nextsimtime_cb;
      vpiHandle hndl = NULL;
      assert((hndl = vpi_register_cb(&cb_data)) && vpi_free_object(hndl));
      return 0;
}

static void register_functions(void)
{
      s_cb_data cb_data;
      cb_data.reason = cbEndOfCompile;
      cb_data.cb_rtn = register_nextsimtime;
      vpi_register_cb(&cb_data);
}

void (*vlog_startup_routines[])(void) = {
      register_functions,
      NULL
};
