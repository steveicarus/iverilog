/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#ident "$Id: dump_final.c,v 1.4 2003/02/26 01:24:35 steve Exp $"
#endif

# include "config.h"

# include  "priv.h"
# include  <stdio.h>


void dump_final_design(FILE*out)
{
      unsigned idx;
      for (idx = 0 ;  idx < pins ;  idx += 1) {
	    struct pal_bind_s*pin = bind_pin + idx;

	    if (bind_pin[idx].sop) {
		  fprintf(out, "Output pin %u:\n", idx+1);
		  fprintf(out, "    pin nexus=%s\n",
			  pin->nexus? ivl_nexus_name(pin->nexus) : "");
		  fprintf(out, "    pin enable=%s\n",
			  pin->enable ? ivl_logic_name(pin->enable) : "1");

		  if (pin->reg)
			fprintf(out, "    pin ff=%s.%s.q%u\n",
				ivl_scope_name(ivl_lpm_scope(pin->reg)),
				ivl_lpm_basename(pin->reg),
				pin->reg_q);
		  else
			fprintf(out, "    pin ff=*.q%u\n", pin->reg_q);
	    } else {
		  fprintf(out, "Input pin %u:\n", idx+1);
		  fprintf(out, "    pin nexus=%s\n",
			  pin->nexus? ivl_nexus_name(pin->nexus) : "");
	    }
      }
}



/*
 * $Log: dump_final.c,v $
 * Revision 1.4  2003/02/26 01:24:35  steve
 *  ivl_lpm_name is obsolete.
 *
 * Revision 1.3  2002/08/12 01:35:03  steve
 *  conditional ident string using autoconfig.
 *
 * Revision 1.2  2001/07/25 03:10:50  steve
 *  Create a config.h.in file to hold all the config
 *  junk, and support gcc 3.0. (Stephan Boettcher)
 *
 * Revision 1.1  2000/12/09 03:42:52  steve
 *  Stuff registers into macrocells.
 *
 */

