#ifndef __table_mod_H
#define __table_mod_H
/*
 *  Copyright (C) 2011  Cary R. (cygcary@yahoo.com)
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

#include <stdio.h>
#include <vpi_user.h>

extern int parse_table_model(FILE *fp, const char *file_name, vpiHandle callh,
                             unsigned min_cols);

extern int tblmodlex();
extern void destroy_tblmod_lexor();
extern void init_tblmod_lexor(FILE *fp);

#endif
