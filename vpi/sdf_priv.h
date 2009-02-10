#ifndef _sdf_priv_h
#define _sdf_priv_h
/*
 * Copyright (c) 2007-2009 Stephen Williams (steve@icarus.com)
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

# include  <stdio.h>

/*
 * Invoke the parser to parse the opened SDF file. The fd is the SDF
 * file already opened and ready for reading. The path is the path to
 * the file and is only used for error messages.
 */
extern void sdf_process_file(FILE*fd, const char*path);

extern int sdf_flag_warning;
extern int sdf_flag_inform;

/* ****
 * These functions are called by the parser to process the SDF file as
 * it is parsed.
 */

struct sdf_delay_s {
      int defined;
      double value;
};

struct sdf_delval_list_s {
      int count;
      struct sdf_delay_s val[12];
};

extern void sdf_select_instance(const char*celltype, const char*inst);
extern void sdf_iopath_delays(int vpi_edge, const char*src, const char*dst,
			      const struct sdf_delval_list_s*delval);

#endif
