/*
 * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT)
#ident "$Id: vpi_time.cc,v 1.1 2001/03/31 19:00:44 steve Exp $"
#endif

# include  "vpi_priv.h"
# include  "schedule.h"
# include  <assert.h>

static struct __vpiSystemTime {
      struct __vpiHandle base;
      struct t_vpi_time value;
} time_handle;

static void timevar_get_value(vpiHandle ref, s_vpi_value*vp)
{
      assert(ref == &time_handle.base);

      switch (vp->format) {
	  case vpiObjTypeVal:
	  case vpiTimeVal:
	    vp->value.time = &time_handle.value;
	    vp->value.time->type = vpiSimTime;
	    vp->value.time->high = 0;
	    vp->value.time->low = schedule_simtime();
	    vp->format = vpiTimeVal;
	    break;

      }
}

static const struct __vpirt vpip_system_time_rt = {
      vpiTimeVar,
      0,
      0,
      timevar_get_value,
      0,
      0,
      0
};


vpiHandle vpip_sim_time(void)
{
      time_handle.base.vpi_type = &vpip_system_time_rt;
      return &time_handle.base;
}

/*
 * $Log: vpi_time.cc,v $
 * Revision 1.1  2001/03/31 19:00:44  steve
 *  Add VPI support for the simulation time.
 *
 */

