/*
 * Copyright (c) 2001-2010 Stephen Williams (steve@icarus.com)
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

# include "config.h"

# include  "priv.h"
# include  <stdio.h>
# include  <stdlib.h>
# include  <assert.h>
# include  "ivl_alloc.h"

static void draw_macrocell_modes(FILE*jfd)
{
      unsigned idx;
      unsigned cfuses;
      unsigned mode, mcnt;

      for (idx = 0 ;  idx < pins ;  idx += 1) {
	    char*str;
	    unsigned ffirst, flast, tmp;
	    struct pal_bind_s*cur = bind_pin + idx;
	    if (cur->sop == 0)
		  continue;

	    cfuses = pal_sop_cfuses(cur->sop);
	    mcnt = 1 << pal_sop_cfuses(cur->sop);
	    mode = 0;
	    for (mode = 0 ;  mode < mcnt ;  mode += 1) {

		  pal_sop_set_mode(cur->sop, mode);
		  if (cur->reg && !pal_sop_is_register(cur->sop))
			continue;

		  if (!cur->reg && pal_sop_is_register(cur->sop))
			continue;

		  if (cur->sop_inv && !pal_sop_is_invert(cur->sop))
			continue;

		  if (!cur->sop_inv && pal_sop_is_invert(cur->sop))
			continue;

		  break;
	    }

	    assert(mode < mcnt);

	    ffirst = pal_sop_cfuse(cur->sop, 0);
	    flast  = pal_sop_cfuse(cur->sop, 0);
	    for (tmp = 1 ;  tmp < cfuses ;  tmp += 1) {
		  unsigned f = pal_sop_cfuse(cur->sop, tmp);
		  if (f < ffirst)
			ffirst = f;
		  if (f > flast)
			flast = f;
	    }
	    assert(flast == (ffirst + cfuses - 1));

	    str = malloc(cfuses+1);
	    str[cfuses] = 0;
	    for (tmp = 0 ;  tmp < cfuses ;  tmp += 1) {
		  if (mode & (1 << (cfuses-tmp-1)))
			str[pal_sop_cfuse(cur->sop, tmp)-ffirst] = '1';
		  else
			str[pal_sop_cfuse(cur->sop, tmp)-ffirst] = '0';
	    }


	    fprintf(jfd, "L%05u %s* Note: ", ffirst, str);
	    if (cur->nexus)
		  fprintf(jfd, "%s ", ivl_nexus_name(cur->nexus));

	    { int pin = pal_sop_pin(cur->sop);
	      if (pin > 0)
		    fprintf(jfd, "pin %d: ", pin);
	    }
	    if (pal_sop_is_register(cur->sop))
		  fprintf(jfd, "<registered");
	    else
		  fprintf(jfd, "<unregistered");

	    if (pal_sop_is_invert(cur->sop))
		  fprintf(jfd, ", invert");

	    fprintf(jfd, "> *\n");

	    free(str);
      }
}

int emit_jedec(const char*path)
{
      FILE*jfd;

      jfd = fopen(path, "w");
      if (jfd == 0) {
	    fprintf(stderr, "unable to open ``%s'' for output.\n", path);
	    return -1;
      }

      fprintf(jfd, "\002This file created by Icarus Verilog/PAL\n");
      fprintf(jfd, "*\n");

      fprintf(jfd, "QF%u*  Number of fuses*\n", pal_fuses(pal));
      fprintf(jfd, "F0*  Note: Default fuse set to 0*\n");
      fprintf(jfd, "G0*  Note: Security fuse NOT blown.*\n");

      draw_macrocell_modes(jfd);

      fclose(jfd);
      return 0;
}
