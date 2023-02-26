/*
 * Copyright (c) 1999-2018 Stephen Williams (steve@icarus.com)
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

# include "vpi_config.h"
# include "vpi_user.h"
# include "vcd_priv.h"
# include <stdio.h>
# include <stdlib.h>
# include <string.h>

extern void sys_convert_register(void);
extern void sys_countdrivers_register(void);
extern void sys_darray_register(void);
extern void sys_fileio_register(void);
extern void sys_finish_register(void);
extern void sys_deposit_register(void);
extern void sys_display_register(void);
extern void sys_plusargs_register(void);
extern void sys_queue_register(void);
extern void sys_random_register(void);
extern void sys_random_mti_register(void);
extern void sys_readmem_register(void);
extern void sys_scanf_register(void);
extern void sys_sdf_register(void);
extern void sys_time_register(void);
extern void sys_vcd_register(void);
extern void sys_vcdoff_register(void);
extern void sys_special_register(void);
extern void table_model_register(void);
extern void vams_simparam_register(void);

#ifdef HAVE_LIBZ
#ifdef HAVE_LIBBZ2
extern void sys_lxt_register(void);
#else
static void sys_lxt_register(void) { fputs("LXT support disabled since libbzip2 not available\n",stderr); exit(1); }
#endif
extern void sys_lxt2_register(void);
extern void sys_fst_register(void);
#else
static void sys_lxt_register(void) { fputs("LXT support disabled since zlib not available\n",stderr); exit(1); }
static void sys_lxt2_register(void) { fputs("LXT2 support disabled since zlib not available\n",stderr); exit(1); }
static void sys_fst_register(void) { fputs("FST support disabled since zlib not available\n",stderr); exit(1); }
#endif

static void sys_lxt_or_vcd_register(void)
{
      int idx;
      struct t_vpi_vlog_info vlog_info;

      const char*dumper;

	/* Get the dumper of choice from the IVERILOG_DUMPER
	   environment variable. */
      dumper = getenv("IVERILOG_DUMPER");
      if (dumper) {
	    char*cp = strchr(dumper,'=');
	    if (cp != 0)
		  dumper = cp + 1;

      } else {
	    dumper = "vcd";
      }

	/* Scan the extended arguments, looking for flags that select
	   major features. This can override the environment variable
	   settings. */
      vpi_get_vlog_info(&vlog_info);

      for (idx = 0 ;  idx < vlog_info.argc ;  idx += 1) {

            if (strcmp(vlog_info.argv[idx],"-fst") == 0) {
		  dumper = "fst";

	    } else if (strcmp(vlog_info.argv[idx],"-fst-space") == 0) {
		  dumper = "fst";

	    } else if (strcmp(vlog_info.argv[idx],"-fst-speed") == 0) {
		  dumper = "fst";

	    } else if (strcmp(vlog_info.argv[idx],"-fst-space-speed") == 0) {
		  dumper = "fst";
	    } else if (strcmp(vlog_info.argv[idx],"-fst-speed-space") == 0) {
		  dumper = "fst";

	    } else if (strcmp(vlog_info.argv[idx],"-fst-none") == 0) {
		  dumper = "none";

	    } else if (strcmp(vlog_info.argv[idx],"-lxt") == 0) {
		  dumper = "lxt";

	    } else if (strcmp(vlog_info.argv[idx],"-lxt-space") == 0) {
		  dumper = "lxt";

	    } else if (strcmp(vlog_info.argv[idx],"-lxt-speed") == 0) {
		  dumper = "lxt";

	    } else if (strcmp(vlog_info.argv[idx],"-lxt-none") == 0) {
		  dumper = "none";

	    } else if (strcmp(vlog_info.argv[idx],"-lxt2") == 0) {
		  dumper = "lxt2";

	    } else if (strcmp(vlog_info.argv[idx],"-lxt2-space") == 0) {
		  dumper = "lxt2";

	    } else if (strcmp(vlog_info.argv[idx],"-lxt2-speed") == 0) {
		  dumper = "lxt2";

	    } else if (strcmp(vlog_info.argv[idx],"-lxt2-none") == 0) {
		  dumper = "none";

	    } else if (strcmp(vlog_info.argv[idx],"-lx2") == 0) {
		  dumper = "lxt2";

	    } else if (strcmp(vlog_info.argv[idx],"-lx2-space") == 0) {
		  dumper = "lxt2";

	    } else if (strcmp(vlog_info.argv[idx],"-lx2-speed") == 0) {
		  dumper = "lxt2";

	    } else if (strcmp(vlog_info.argv[idx],"-lx2-none") == 0) {
		  dumper = "none";

	    } else if (strcmp(vlog_info.argv[idx],"-vcd") == 0) {
		  dumper = "vcd";

	    } else if (strcmp(vlog_info.argv[idx],"-vcd-off") == 0) {
		  dumper = "none";

	    } else if (strcmp(vlog_info.argv[idx],"-vcd-none") == 0) {
		  dumper = "none";

	    } else if (strcmp(vlog_info.argv[idx],"-none") == 0) {
		  dumper = "none";

	    } else if (strncmp(vlog_info.argv[idx],"-dumpfile=",10) == 0) {
		  vcd_set_dump_path_default(vlog_info.argv[idx]+10);
	    }
      }

      if (strcmp(dumper, "vcd") == 0)
	    sys_vcd_register();

      else if (strcmp(dumper, "VCD") == 0)
	    sys_vcd_register();

      else if (strcmp(dumper, "fst") == 0)
	    sys_fst_register();

      else if (strcmp(dumper, "FST") == 0)
	    sys_fst_register();

      else if (strcmp(dumper, "lxt") == 0)
	    sys_lxt_register();

      else if (strcmp(dumper, "LXT") == 0)
	    sys_lxt_register();

      else if (strcmp(dumper, "lxt2") == 0)
	    sys_lxt2_register();

      else if (strcmp(dumper, "LXT2") == 0)
	    sys_lxt2_register();

      else if (strcmp(dumper, "lx2") == 0)
	    sys_lxt2_register();

      else if (strcmp(dumper, "LX2") == 0)
	    sys_lxt2_register();

      else if (strcmp(dumper, "none") == 0)
	    sys_vcdoff_register();

      else if (strcmp(dumper, "NONE") == 0)
	    sys_vcdoff_register();

      else {
	    vpi_mcd_printf(1, "system.vpi: Unknown dumper format: %s,"
		           " using VCD instead.\n", dumper);
	    sys_vcd_register();
      }
}

void (*vlog_startup_routines[])(void) = {
      sys_convert_register,
      sys_countdrivers_register,
      sys_darray_register,
      sys_fileio_register,
      sys_finish_register,
      sys_deposit_register,
      sys_display_register,
      sys_plusargs_register,
      sys_queue_register,
      sys_random_register,
      sys_random_mti_register,
      sys_readmem_register,
      sys_scanf_register,
      sys_time_register,
      sys_lxt_or_vcd_register,
      sys_sdf_register,
      sys_special_register,
      table_model_register,
      vams_simparam_register,
      0
};
