#define COPYRIGHT \
  "Copyright (c) 2001-2024 Stephen Williams (steve@icarus.com)"
/*
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

# include  "version_base.h"
# include  "version_tag.h"
# include  "vvp_priv.h"
# include  <string.h>
# include  <assert.h>
# include  <stdlib.h>
# include  <sys/types.h>
# include  <sys/stat.h>

static const char*version_string =
"Icarus Verilog VVP Code Generator " VERSION " (" VERSION_TAG ")\n\n"
COPYRIGHT "\n\n"
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
unsigned show_file_line = 0;

int debug_draw = 0;

/* This needs to match the actual flag count in the VVP thread. */
# define FLAGS_COUNT 512

static uint32_t allocate_flag_mask[FLAGS_COUNT / 32] = { 0x000000ff, 0 };


__inline__ static void draw_execute_header(ivl_design_t des)
{
      const char*cp = ivl_design_flag(des, "VVP_EXECUTABLE");
      if (cp) {
	    const char *extra_args = ivl_design_flag(des, "VVP_EXTRA_ARGS");
	    if (!extra_args)
		  extra_args = "";
	    fprintf(vvp_out, "#! %s%s\n", cp, extra_args);
#if !defined(__MINGW32__)
	    fchmod(fileno(vvp_out), 0755);
#endif
      }
      fprintf(vvp_out, ":ivl_version \"" VERSION "\"");
	/* I am assuming that a base release will have a blank tag. */
      if (*VERSION_TAG != 0) {
	    fprintf(vvp_out, " \"(" VERSION_TAG ")\"");
      }
      fprintf(vvp_out, ";\n");
}

__inline__ static void draw_module_declarations(ivl_design_t des)
{
      const char*cp = ivl_design_flag(des, "VPI_MODULE_LIST");

      while (*cp) {
	    const char*comma = strchr(cp, ',');

	    if (comma == 0)
		  comma = cp + strlen(cp);

	    char*buffer = malloc(comma - cp + 1);
	    strncpy(buffer, cp, comma-cp);
	    buffer[comma-cp] = 0;
	    fprintf(vvp_out, ":vpi_module \"%s\";\n", buffer);
	    free(buffer);

	    cp = comma;
	    if (*cp) cp += 1;
      }
}

int allocate_flag(void)
{
      int idx;
      for (idx = 0 ; idx < FLAGS_COUNT ; idx += 1) {
	    int word = idx / 32;
	    uint32_t mask = 1U << (idx%32);
	    if (allocate_flag_mask[word] & mask)
		  continue;

	    allocate_flag_mask[word] |= mask;
	    return idx;
      }

      fprintf(stderr, "vvp.tgt error: Exceeded the maximum flag count of "
                      "%d during VVP code generation.\n", FLAGS_COUNT);
      exit(1);
}

void clr_flag(int idx)
{
      if (idx < 8) return;
      assert(idx < FLAGS_COUNT);
      int word = idx / 32;
      uint32_t mask = 1 << (idx%32);

      assert(allocate_flag_mask[word] & mask);

      allocate_flag_mask[word] &= ~mask;
}

static void process_debug_string(const char*debug_string)
{
      const char*cp = debug_string;
      debug_draw = 0;

      while (*cp) {
	    const char*tail = strchr(cp, ',');
	    if (tail == 0)
		  tail = cp + strlen(cp);

	    size_t len = tail - cp;
	    if (len == 4 && strncmp(cp,"draw", 4)==0) {
		  debug_draw = 1;
	    }

	    while (*tail == ',')
		  tail += 1;

	    cp = tail;
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
	/* Use -pfileline to determine if file and line information is
	 * printed for procedural statements. (e.g. -pfileline=1).
	 * The default is no file/line information will be included. */
      const char*fileline = ivl_design_flag(des, "fileline");

      const char*debug_flags = ivl_design_flag(des, "debug_flags");
      process_debug_string(debug_flags);

      assert(path);

        /* Check to see if file/line information should be included. */
      if (strcmp(fileline, "") != 0) {
            char *eptr;
            long fl_value = strtol(fileline, &eptr, 0);
              /* Nothing usable in the file/line string. */
            if (fileline == eptr) {
                  fprintf(stderr, "vvp.tgt error: Unable to extract file/line "
                                  "information from string: %s\n", fileline);
                  return 1;
            }
              /* Extra stuff at the end. */
            if (*eptr != 0) {
                  fprintf(stderr, "vvp.tgt error: Extra characters '%s' "
                                  "included at end of file/line string: %s\n",
                                  eptr, fileline);
                  return 1;
            }
              /* The file/line flag must be positive. */
            if (fl_value < 0) {
                  fprintf(stderr, "vvp.tgt error: File/line flag (%ld) must "
                                  "be positive.\n", fl_value);
                  return 1;
            }
            show_file_line = fl_value > 0;
      }

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

      fprintf(vvp_out, ":ivl_delay_selection \"%s\";\n",
                       ivl_design_delay_sel(des));

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
