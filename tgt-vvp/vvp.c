/*
 * Copyright (c) 2001-2011 Stephen Williams (steve@icarus.com)
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

# include  "version_base.h"
# include  "version_tag.h"
# include  "vvp_priv.h"
# include  <string.h>
# include  <assert.h>
# include  <sys/types.h>
# include  <sys/stat.h>

static const char*version_string =
"Icarus Verilog VVP Code Generator " VERSION " (" VERSION_TAG ")\n\n"
"Copyright (c) 2001-2011 Stephen Williams (steve@icarus.com)\n\n"
"  This program is free software; you can redistribute it and/or modify\n"
"  it under the terms of the GNU General Public License as published by\n"
"  the Free Software Foundation; either version 2 of the License, or\n"
"  (at your option) any later version.\n"
"\n"
"  This program is distributed in the hope that it will be useful,\n"
"  but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
"  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n"
"  GNU General Public License for more details.\n"
"\n"
"  You should have received a copy of the GNU General Public License along\n"
"  with this program; if not, write to the Free Software Foundation, Inc.,\n"
"  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.\n"
;

FILE*vvp_out = 0;
int vvp_errors = 0;

__inline__ static void draw_execute_header(ivl_design_t des)
{
      const char*cp = ivl_design_flag(des, "VVP_EXECUTABLE");
      if (cp) {
	    fprintf(vvp_out, "#! %s\n", cp);
#if !defined(__MINGW32__)
	    fchmod(fileno(vvp_out), 0755);
#endif
      }
      fprintf(vvp_out, ":ivl_version \"" VERSION "\"");
	/* I am assuming that a base release will have a blank tag. */
      if (strcmp(VERSION_TAG, "") != 0) {
	    fprintf(vvp_out, " \"(" VERSION_TAG ")\"");
      }
      fprintf(vvp_out, ";\n");
}

__inline__ static void draw_module_declarations(ivl_design_t des)
{
      const char*cp = ivl_design_flag(des, "VPI_MODULE_LIST");

      while (*cp) {
	    char buffer[128];
	    const char*comma = strchr(cp, ',');

	    if (comma == 0)
		  comma = cp + strlen(cp);

	    strncpy(buffer, cp, comma-cp);
	    buffer[comma-cp] = 0;
	    fprintf(vvp_out, ":vpi_module \"%s\";\n", buffer);

	    cp = comma;
	    if (*cp) cp += 1;
      }
}


int target_design(ivl_design_t des)

{
      int rc;
      ivl_scope_t *roots;
      unsigned nroots, i;
      unsigned size;
      unsigned idx;
      const char*path = ivl_design_flag(des, "-o");
      assert(path);

#ifdef HAVE_FOPEN64
      vvp_out = fopen64(path, "w");
#else
      vvp_out = fopen(path, "w");
#endif
      if (vvp_out == 0) {
	    perror(path);
	    return -1;
      }

      vvp_errors = 0;

      draw_execute_header(des);

      { int pre = ivl_design_time_precision(des);
        char sign = '+';
        if (pre < 0) {
	      pre = -pre;
	      sign = '-';
	}
        fprintf(vvp_out, ":vpi_time_precision %c %d;\n", sign, pre);
      }

      draw_module_declarations(des);

        /* This causes all structural records to be drawn. */
      ivl_design_roots(des, &roots, &nroots);
      for (i = 0; i < nroots; i++)
	    draw_scope(roots[i], 0);

        /* Finish up any modpaths that are not yet emitted. */
      cleanup_modpath();

      rc = ivl_design_process(des, draw_process, 0);

        /* Dump the file name table. */
      size = ivl_file_table_size();
      fprintf(vvp_out, "# The file index is used to find the file name in "
                       "the following table.\n:file_names %u;\n", size);
      for (idx = 0; idx < size; idx++) {
	    fprintf(vvp_out, "    \"%s\";\n", ivl_file_table_item(idx));
      }

      fclose(vvp_out);
      EOC_cleanup_drivers();

      return rc + vvp_errors;
}


const char* target_query(const char*key)
{
      if (strcmp(key,"version") == 0)
	    return version_string;

      return 0;
}
