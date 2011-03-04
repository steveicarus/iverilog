/*
 * Copyright (C) 2011 Cary R. (cygcary@yahoo.com)
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

# include "compile.h"
# include "vpi_priv.h"

struct __vpiFileLine {
      struct __vpiHandle base;
      const char *description;
      unsigned file_idx;
      unsigned lineno;
};

bool show_file_line = false;
bool code_is_instrumented = false;

static int file_line_get(int type, vpiHandle ref)
{
      struct __vpiFileLine*rfp = (struct __vpiFileLine*)ref;

      assert(ref->vpi_type->type_code == _vpiFileLine);

      switch (type) {
	case vpiLineNo:
	    return rfp->lineno;
	default:
	    return vpiUndefined;
      }
}

static char *file_line_get_str(int type, vpiHandle ref)
{
      struct __vpiFileLine*rfp = (struct __vpiFileLine*)ref;

      assert(ref->vpi_type->type_code == _vpiFileLine);

      switch (type) {
	case vpiFile:
	    assert(rfp->file_idx < file_names.size());
	    return simple_set_rbuf_str(file_names[rfp->file_idx]);
	case _vpiDescription:
	    if (rfp->description) return simple_set_rbuf_str(rfp->description);
	    else return simple_set_rbuf_str("Procedural tracing.");
	default:
	    return 0;
      }
}

static const struct __vpirt vpip_file_line_rt = {
       _vpiFileLine,
       file_line_get,
       file_line_get_str,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0
};

vpiHandle vpip_build_file_line(char*description, long file_idx, long lineno)
{
      struct __vpiFileLine*obj = new struct __vpiFileLine;

	/* Turn on the diagnostic output when we find a %file_line. */
      show_file_line = true;
      code_is_instrumented = true;

      obj->base.vpi_type = &vpip_file_line_rt;
      if (description) obj->description = vpip_name_string(description);
      else obj->description = 0;
      obj->file_idx = (unsigned) file_idx;
      obj->lineno = (unsigned) lineno;

      return &obj->base;
}
