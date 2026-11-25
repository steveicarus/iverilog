#ifndef IVL_CmdExec_H
#define IVL_CmdExec_H
/*
 * Copyright (c) 2026 Lars-Peter Clausen <lars@metafoo.de>
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

# include  <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

int ivl_run_cmd(const char *cmd, int verbose);
FILE *ivl_run_cmd_pipe(const char *cmd);
int ivl_close_cmd_pipe(FILE *fp);

#ifdef __cplusplus
}
#endif

#endif /* IVL_CmdExec_H */
