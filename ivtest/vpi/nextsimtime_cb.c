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

#if     defined(TEST_SIM_TIME)
#define TIME_TYPE vpiSimTime
#elif   defined(TEST_SCALED_TIME)
#define TIME_TYPE vpiScaledRealTime
#else
#define TIME_TYPE vpiSuppressTime
#endif

#ifndef TEST_NULL_TIME
static s_vpi_time cb_timerec = {TIME_TYPE, 0, 0, 0};
#endif

static PLI_INT32 register_nextsimtime(struct t_cb_data* cb);


static PLI_INT32 nextsimtime_cb(struct t_cb_data* cb) {
      s_vpi_time timerec;

#ifdef TEST_NULL_TIME
      (void)cb;
#else
      assert(cb->time && (cb->time->type == TIME_TYPE));
#endif

#if defined(TEST_SCALED_TIME) || defined(TEST_SIM_TIME)
      timerec = *(cb->time);
#else
      timerec.type = vpiSimTime;
      vpi_get_time(NULL, &timerec);
#endif


#ifdef TEST_SCALED_TIME
      vpi_printf("nextsimtime: %f\n", timerec.real);
#else
      uint64_t time = ((uint64_t)timerec.high << 32) | timerec.low;
      vpi_printf("nextsimtime: %" PLI_UINT64_FMT "\n", time);
#endif
      register_nextsimtime(NULL);
      return 0;
}

static PLI_INT32 register_nextsimtime(struct t_cb_data* cb) {
      (void)cb; /* unused */
      s_cb_data cb_data;
#ifdef TEST_NULL_TIME
      cb_data.time   = 0;
#else
      cb_data.time   = &cb_timerec;
#endif
#ifdef TEST_SCALED_TIME
      cb_data.obj    = vpi_handle_by_name("main", NULL);
#else
      cb_data.obj    = 0;
#endif
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
