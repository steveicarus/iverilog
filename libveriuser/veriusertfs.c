/* vi:sw=6
 * Copyright (c) 2002 Michael Ruff (mruff at chiaro.com)
 *                    Michael Runyan (mrunyan at chiaro.com)
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
#ident "$Id: veriusertfs.c,v 1.1 2002/05/30 02:37:26 steve Exp $"
#endif

/*
 * Contains the routines required to implement veriusertfs routines
 * via VPI. This is extremly ugly, so don't look after eating dinner.
 */

# include <stdio.h>
# include <string.h>
# include <stdlib.h>
# include "vpi_user.h"
# include "veriuser.h"


/*
 * local structure used to hold the persistent veriusertfs data
 * and anything else we decice to put it here like workarea data.
 */
typedef struct t_pli_data {
      short	data;			/* veriusertfs data */
      int	(*checktf)();		/* pointer to checktf routine */
      int	(*calltf)();		/* pointer to calltf routine */
      int	(*misctf)();		/* pointer to misctf routine */
} s_pli_data, *p_pli_data;

static int compiletf(char *);
static int calltf(char *);
static int callback(p_cb_data);

/*
 * Register veriusertfs routines/wrappers.
 */
void veriusertfs_register()
{
      p_tfcell usertf;
      s_vpi_systf_data tf_data;
      p_pli_data data;

      for (usertf = veriusertfs; usertf; usertf++) {
	    /* last element */
	    if (usertf->type == 0) break;

	    /* only forwref true */
	    if (!usertf->forwref) {
		  vpi_printf("  skipping %s, forwref != true\n",
			usertf->tfname);
		  continue;
	    }

	    /* squirrel away some of veriusertfs in persistent user_data */
	    data = (p_pli_data) calloc(1, sizeof(s_pli_data));
	    data->data = usertf->data;
	    data->checktf = usertf->checktf;
	    data->calltf = usertf->calltf;
	    data->misctf = usertf->misctf;

	    (void) memset(&tf_data, 0, sizeof(s_vpi_systf_data));
	    switch (usertf->type) {
		  case usertask:
			tf_data.type = vpiSysTask;
		  break;
		  case userfunction:
			tf_data.type = vpiSysFunc;
		  break;
		  default:
			vpi_printf("  skipping %s, unsupported type %d\n",
			      usertf->tfname, usertf->type);
			continue;
		  break;
	    }
	    tf_data.tfname = usertf->tfname;
	    tf_data.compiletf = compiletf;
	    tf_data.calltf = calltf;
	    tf_data.sizetf = usertf->sizetf;
	    tf_data.user_data = (char *)data;

	    /* register */
	    vpi_register_systf(&tf_data);
      }

      return;
}

/*
 * This function calls the veriusertfs checktf and sets up all the
 * callbacks misctf requires.
 */
static int compiletf(char *data)
{
      p_pli_data pli;
      vpiHandle call_h, arg_i, arg_h;
      s_cb_data cb_data;

      /* cast back from opaque */
      pli = (p_pli_data)data;

      /* get call handle */
      call_h = vpi_handle(vpiSysTfCall, NULL);

      /* default cb_data */
      (void) memset(&cb_data, 0, sizeof(s_cb_data));
      cb_data.cb_rtn = callback;
      cb_data.user_data = data;

      /* register EndOfSim misctf callback */
      cb_data.reason = cbEndOfSimulation;
      cb_data.obj = call_h;
      vpi_register_cb(&cb_data);

#if 0
      /* register callbacks on each argument */
      cb_data.reason = cbValueChange;
      arg_i = vpi_iterate(vpiArgument, call_h);
      if (arg_i != NULL) {
	    while ((arg_h = vpi_scan(arg_i)) != NULL) {
		  cb_data.obj = arg_h;
		  vpi_register_cb(&cb_data);
	    }
      }
#endif

      /* since we are at compiletf, misctf needs to fire */
      pli->misctf(pli->data, reason_endofcompile);

      /* similarly run checktf now */
      return pli->checktf(pli->data, 0);
}

/*
 * This function is the wrapper for the veriusertfs calltf routine.
 */
static int calltf(char *data)
{
      p_pli_data pli;

      /* cast back from opaque */
      pli = (p_pli_data)data;

      /* execute calltf */
      return pli->calltf(pli->data, 0);
}

/*
 * This function is the wrapper for all the misctf callbacks
 */
static int callback (p_cb_data data)
{
      p_pli_data pli;
      int reason;

      /* cast back from opaque */
      pli = (p_pli_data)data->user_data;

      switch (data->reason) {
	    case cbValueChange:
		  reason = reason_paramvc;
		  break;
	    case cbEndOfSimulation:
		  reason = reason_finish;
		  break;
	    default:
		  abort();
      }

      /* execute misctf */
      return pli->misctf(pli->data, reason);
}

/*
 * $Log: veriusertfs.c,v $
 * Revision 1.1  2002/05/30 02:37:26  steve
 *  Add the veriusertf_register funciton.
 *
 */
