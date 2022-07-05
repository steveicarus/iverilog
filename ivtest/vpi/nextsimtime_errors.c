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

#if defined(TEST_SCALED) || defined(TEST_SUPPRESS)
#if defined(TEST_SCALED)
#define TIME_TYPE vpiScaledRealTime
#elif defined(TEST_SUPPRESS)
#define TIME_TYPE vpiSuppressTime
#endif
static s_vpi_time cb_timerec = {TIME_TYPE, 0, 0, 0};
#endif

static PLI_INT32 dummy_cb(struct t_cb_data* cb) {
      (void)cb; /* unused */
      return 0;
}

static PLI_INT32 register_nextsimtime(struct t_cb_data* cb) {
      (void)cb; /* unused */
      s_cb_data cb_data;
#if defined(TEST_SCALED) || defined(TEST_SUPPRESS)
      cb_data.time   = &cb_timerec;
#elif defined(TEST_NULL)
      cb_data.time   = NULL;
#endif
      cb_data.reason = cbNextSimTime;
      cb_data.cb_rtn = dummy_cb;
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
