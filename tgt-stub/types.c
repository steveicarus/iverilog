/*
 * Copyright (c) 2012  Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include  "config.h"
# include  "priv.h"
# include  <string.h>

static void show_net_type_darray(ivl_type_t net_type)
{
	/* Dynamic arrays have a single element type. */
      ivl_type_t element_type = ivl_type_element(net_type);

      fprintf(out, "darray of ");
      show_net_type(element_type);
}

static void show_net_type_queue(ivl_type_t net_type)
{
	/* Dynamic arrays have a single element type. */
      ivl_type_t element_type = ivl_type_element(net_type);

      fprintf(out, "queue of ");
      show_net_type(element_type);
}

void show_net_type(ivl_type_t net_type)
{
      ivl_variable_type_t data_type = ivl_type_base(net_type);

      switch (data_type) {
	  case IVL_VT_NO_TYPE:
	    fprintf(out, "<ERROR-no-type>");
	    stub_errors += 1;
	    break;
	  case IVL_VT_BOOL:
	    fprintf(out, "bool");
	    break;
	  case IVL_VT_LOGIC:
	    fprintf(out, "logic");
	    break;
	  case IVL_VT_REAL:
	    fprintf(out, "real");
	    break;
	  case IVL_VT_STRING:
	    fprintf(out, "string");
	    break;
	  case IVL_VT_DARRAY:
	    show_net_type_darray(net_type);
	    break;
	  case IVL_VT_CLASS:
	    fprintf(out, "class");
	    break;
	  case IVL_VT_QUEUE:
	    show_net_type_queue(net_type);
	    break;
	  case IVL_VT_VOID:
	    fprintf(out, "void");
	    break;
      }

      unsigned packed_dimensions = ivl_type_packed_dimensions(net_type);
      unsigned idx;
      for (idx = 0 ; idx < packed_dimensions ; idx += 1) {
	    fprintf(out, "[%d:%d]", ivl_type_packed_msb(net_type, idx),
		    ivl_type_packed_lsb(net_type, idx));
      }
}

void show_type_of_signal(ivl_signal_t net)
{
      unsigned dim;

	/* The data_type is the base type of the signal. This the the
	   starting point for the type. In the long run I think I want
	   to remove this in favor of the ivl_signal_net_type below. */
      ivl_variable_type_t data_type = ivl_signal_data_type(net);

	/* This gets the more general type description. This is a
	   newer form so doesn't yet handle all the cases. Newer
	   types, such DARRAY types, REQUIRE this method to get at the
	   type details. */
      ivl_type_t net_type = ivl_signal_net_type(net);

      if (net_type) {
	    show_net_type(net_type);
	    return;
      }

      switch (data_type) {
	  case IVL_VT_NO_TYPE:
	    fprintf(out, "<no-type>");
	    break;
	  case IVL_VT_BOOL:
	    fprintf(out, "bool");
	    break;
	  case IVL_VT_LOGIC:
	    fprintf(out, "logic");
	    break;
	  case IVL_VT_REAL:
	    fprintf(out, "real");
	    break;
	  case IVL_VT_STRING:
	    fprintf(out, "string");
	    break;
	  case IVL_VT_DARRAY:
	      /* The DARRAY type MUST be described by an
		 ivl_signal_net_type object. */
	    fprintf(out, "ERROR-DARRAY");
	    stub_errors += 1;
	    break;
	  case IVL_VT_QUEUE:
	      /* The QUEUE type MUST be described by an
		 ivl_signal_net_type object. */
	    fprintf(out, "ERROR-QUEUE");
	    stub_errors += 1;
	    break;
	  case IVL_VT_VOID:
	    fprintf(out, "void");
	    break;
	  case IVL_VT_CLASS:
	    fprintf(out, "class");
	    break;
      }

      for (dim = 0 ; dim < ivl_signal_packed_dimensions(net) ; dim += 1) {
	    fprintf(out, "[%d:%d]", ivl_signal_packed_msb(net,dim),
		    ivl_signal_packed_lsb(net,dim));
      }
}
