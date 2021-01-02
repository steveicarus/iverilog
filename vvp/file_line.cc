/*
 * Copyright (C) 2011-2021 Cary R. (cygcary@yahoo.com)
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

class __vpiFileLine : public __vpiHandle {
    public:
      explicit __vpiFileLine(const char*desc_, long file_idx_, long lineno_);
      int get_type_code(void) const;
      int vpi_get(int code);
      char* vpi_get_str(int code);

      const char* get_description() const { return description; };
      unsigned get_file_idx() const { return file_idx; };
      unsigned get_lineno() const { return lineno; };

    private:
      const char *description;
      unsigned file_idx;
      unsigned lineno;
};

bool show_file_line = false;
bool code_is_instrumented = false;

static int file_line_get(int type, vpiHandle ref)
{
      __vpiFileLine*rfp = dynamic_cast<__vpiFileLine*>(ref);
      assert(rfp);

      switch (type) {
	case vpiLineNo:
	    return rfp->get_lineno();
	default:
	    return vpiUndefined;
      }
}

static char *file_line_get_str(int type, vpiHandle ref)
{
      __vpiFileLine*rfp = dynamic_cast<__vpiFileLine*>(ref);
      assert(rfp);

      switch (type) {
	case vpiFile:
	    assert(rfp->get_file_idx() < file_names.size());
	    return simple_set_rbuf_str(file_names[rfp->get_file_idx()]);
	case _vpiDescription:
	    if (rfp->get_description()) {
		  return simple_set_rbuf_str(rfp->get_description());
	    } else return simple_set_rbuf_str("Procedural tracing.");
	default:
	    return 0;
      }
}

inline __vpiFileLine::__vpiFileLine(const char*desc_, long file_idx_,
                                    long lineno_)
{
      if (desc_) description = vpip_name_string(desc_);
      else description = 0;
      file_idx = (unsigned) file_idx_;
      lineno = (unsigned) lineno_;
}

int __vpiFileLine::get_type_code(void) const
{ return _vpiFileLine; }

int __vpiFileLine::vpi_get(int code)
{ return file_line_get(code, this); }

char* __vpiFileLine::vpi_get_str(int code)
{ return file_line_get_str(code, this); }


vpiHandle vpip_build_file_line(char*description, long file_idx, long lineno)
{
      __vpiFileLine*obj = new __vpiFileLine(description, file_idx, lineno);

	/* You can turn on the diagnostic output if we find a %file_line. */
      code_is_instrumented = true;

      return obj;
}
