/*
 * Copyright (c) 1999 Stephen Williams (steve@icarus.com)
 * Copyright (c) 2000 Stephan Boettcher <stephan@nevis.columbia.edu>
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
#ifdef HAVE_CVS_IDENT
#ident "$Id: sys_deposit.c,v 1.7 2007/03/14 04:05:51 steve Exp $"
#endif

# include "vpi_config.h"

# include  "vpi_user.h"
# include  <assert.h>

static PLI_INT32 sys_deposit_compiletf(PLI_BYTE8 *name)
{
      vpiHandle callh = vpi_handle(vpiSysTfCall, 0);
      vpiHandle argv = vpi_iterate(vpiArgument, callh);
      vpiHandle target, value;

      /* Check that there are arguments. */
      if (argv == 0) {
            vpi_printf("ERROR: %s requires two arguments.\n", name);
            vpi_control(vpiFinish, 1);
            return 0;
      }

      /* Check that there are at least two arguments. */
      target = vpi_scan(argv);  /* This should never be zero. */
      value = vpi_scan(argv);
      if (value == 0) {
            vpi_printf("ERROR: %s requires two arguments.\n", name);
            vpi_control(vpiFinish, 1);
            return 0;
      }

      assert(target);

      /* Check the targets type. It must be a net or a register. */
      switch (vpi_get(vpiType, target)) {
            case vpiNet:
            case vpiReg:
                  break;
            default:
                  vpi_printf("ERROR: invalid target type for %s.\n", name);
                  vpi_control(vpiFinish, 1);
                  return 0;
      }

      /* Check that there is at most two arguments. */
      target = vpi_scan(argv);
      if (target != 0) {
            vpi_printf("ERROR: %s takes at most two arguments.\n", name);
            vpi_control(vpiFinish, 1);
            return 0;
      }

      return 0;
}

static PLI_INT32 sys_deposit_calltf(PLI_BYTE8 *name)
{
      vpiHandle callh, argv, target, value;
      s_vpi_value val;

      callh = vpi_handle(vpiSysTfCall, 0);
      argv = vpi_iterate(vpiArgument, callh);
      target = vpi_scan(argv);
      assert(target);
      value = vpi_scan(argv);
      assert(value);

      val.format = vpiIntVal;
      vpi_get_value(value, &val);

      vpi_put_value(target, &val, 0, vpiNoDelay);

      vpi_free_object(argv);
      return 0;
}

void sys_deposit_register()
{
      s_vpi_systf_data tf_data;

      tf_data.type      = vpiSysTask;
      tf_data.tfname    = "$deposit";
      tf_data.calltf    = sys_deposit_calltf;
      tf_data.compiletf = sys_deposit_compiletf;
      tf_data.sizetf    = 0;
      tf_data.user_data = "$deposit";
      vpi_register_systf(&tf_data);
}


/*
 * $Log: sys_deposit.c,v $
 * Revision 1.7  2007/03/14 04:05:51  steve
 *  VPI tasks take PLI_BYTE* by the standard.
 *
 * Revision 1.6  2006/10/30 22:45:37  steve
 *  Updates for Cygwin portability (pr1585922)
 *
 * Revision 1.5  2004/10/04 01:10:58  steve
 *  Clean up spurious trailing white space.
 *
 * Revision 1.4  2004/01/21 01:22:53  steve
 *  Give the vip directory its own configure and vpi_config.h
 *
 * Revision 1.3  2002/08/12 01:35:04  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.2  2001/07/25 03:10:50  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.1  2001/04/26 00:01:33  steve
 *  Support $deposit to a wire or reg.
 *
 */

